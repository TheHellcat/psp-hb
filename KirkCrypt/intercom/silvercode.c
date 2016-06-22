
/*
 *  ============================================================
 *    the code in here is almost completely by SilverSpring
 *    all credits for KIRK usage must go to him at this point
 *
 *    only slight modifications were made by me to fit it more
 *    into the app and what I wanted from it
 *  ============================================================
 */
 
#include <pspkernel.h>
#include <pspsdk.h>
#include <string.h>
#include <stdio.h>
 
 
#define KIRK_BUS_CLOCK      (1 << 7)

#define BUS_ENABLE          (1)
#define BUS_DISABLE         (0)

#define SYSREG_GET_BUS_CLOCK()        (_lw(0xBC100050))
#define SYSREG_SET_BUS_CLOCK(_bus)    (_sw(_bus, 0xBC100050))

__inline__ void BusClockEnable(int bus, int mode)
{
    u32 bus_mask = SYSREG_GET_BUS_CLOCK();

    if (mode == BUS_ENABLE)
        bus_mask |= bus;
    else
        bus_mask &= ~bus;

    SYSREG_SET_BUS_CLOCK(bus_mask);
}

typedef volatile struct
{
    u32 signature;
    u32 version;
    u32 error;
    u32 proc_phase;
    u32 command;
    u32 result;
    u32 unk_18;
    u32 status;
    u32 status_async;
    u32 status_async_end;
    u32 status_end;
    u32 src_addr;
    u32 dst_addr;

} PspKirkRegs;

#define MAKE_PHYS_ADDR(_addr)    (((u32)_addr) & 0x1FFFFFFF)
#define SYNC()                   asm("sync")

#define KIRK_HW_REGISTER_ADDR    ((PspKirkRegs *)0xBDE00000)


__inline__ u32 crypt(void *dst, int dstSize, const void *src, int srcSize, u32 cmd)
{
    sceKernelDcacheWritebackInvalidateRange(dst, dstSize);
    sceKernelDcacheWritebackInvalidateRange(src, srcSize);

    PspKirkRegs *const crypt = KIRK_HW_REGISTER_ADDR;

    crypt->command = cmd;
    crypt->src_addr = MAKE_PHYS_ADDR(src);
    crypt->dst_addr = MAKE_PHYS_ADDR(dst);

    crypt->proc_phase = 1;

    while((crypt->status & 0x11) == 0);

    if (crypt->status & 0x10) // error
    {
        crypt->proc_phase = 2;

        while((crypt->status & 2) == 0);

        crypt->status_end = crypt->status;
        SYNC();
        return crypt->result; //-1;
    }

    crypt->status_end = crypt->status;
    SYNC();
    return(crypt->result);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//u8 g_buf[512] __attribute__((aligned(64)));
u8* g_buf;

#define KIRK_COMMAND_4    (4)
#define KIRK_COMMAND_7    (7)

#define KIRK_COMMAND_5    (5)
#define KIRK_COMMAND_8    (8)

#define KIRK_COMMAND_6    (6)
#define KIRK_COMMAND_9    (9)


void setkirkworkbuffer( void* buffer )
{
	g_buf = (u8*)buffer;
}

// seed: 0x00-0x3F
int encrypt(u8 *buf, int size, int seed)
{
    u32 *p = (u32*)g_buf;
    
    p[0] = 4; // KIRK Block Cipher Encrypt operation
    p[1] = 0;
    p[2] = 0;
    p[3] = seed; // 0x00-0x3F only
    p[4] = size;

    int i;
    for(i=0; i<size; i++)
        g_buf[0x14+i] = buf[i]; // save 0x14 Bytes for KIRK header

    if (crypt(g_buf, size+0x14, g_buf, size+0x14, KIRK_COMMAND_4) != 0)
	    return -1;

    for(i=0; i<size; i++)
        buf[i] = g_buf[0x14+i];

    return 0;
}

// seed: 0x00-0x3F
// (can also be 0x40-0x7F which has unknown relationship with cmd4 seed 0x00-0x3F)
int decrypt(u8 *buf, int size, int seed)
{
    u32 *p = (u32*)g_buf;
    
    p[0] = 5; // KIRK Block Cipher Decrypt operation
    p[1] = 0;
    p[2] = 0;
    p[3] = seed; // 0x00-0x3F only (don't use 0x40-0x7F yet)
    p[4] = size;

    int i;
    for(i=0; i<size; i++)
        g_buf[0x14+i] = buf[i]; // save 0x14 Bytes for KIRK header

    if (crypt(g_buf, size+0x14, g_buf, size+0x14, KIRK_COMMAND_7) != 0)
	    return -1;

    for(i=0; i<size; i++)
        buf[i] = g_buf[i];

    return 0;
}

// copies of the above func's but with using custom key
int encryptky(u8 *buf, int size, int seed, u8* key)
{
	int r;
	int kcmd;
	int o;
	
	if( (seed & 0x100) == 0x100 ) { kcmd = KIRK_COMMAND_5; }
	if( (seed & 0x200) == 0x200 ) { kcmd = KIRK_COMMAND_6; }
		
    u32 *p = (u32*)g_buf;
    
    p[0] = 4; // KIRK Block Cipher Encrypt operation
    p[1] = 0;
    p[2] = 0;
    p[3] = seed; // 0x100-0x1FF for IV=fuseid, 0x200-0x2FF for IV=custom
    p[4] = size;
    
    if( (seed & 0x100) == 0x100 ) { o=0x14; }
    if( (seed & 0x200) == 0x200 ) { memcpy( (void*)p+0x14, (void*)key, 16 ); o=0x24; }
    
    int i;
    for(i=0; i<size; i++)
        g_buf[o+i] = buf[i]; // save 0x24 Bytes for KIRK header

    r = crypt(g_buf, size+o, g_buf, size+o, kcmd);
    if (r != 0)
	    return r;

    for(i=0; i<size; i++)
        buf[i] = g_buf[o+i];

    return 0;
}

int decryptky(u8 *buf, int size, int seed, u8* key)
{
	int r;
	int kcmd;
	int o;
	
	if( (seed & 0x100) == 0x100 ) { kcmd = KIRK_COMMAND_8; }
	if( (seed & 0x200) == 0x200 ) { kcmd = KIRK_COMMAND_9; }
		
    u32 *p = (u32*)g_buf;
    
    p[0] = 5; // KIRK Block Cipher Decrypt operation
    p[1] = 0;
    p[2] = 0;
    p[3] = seed;
    p[4] = size;

    if( (seed & 0x100) == 0x100 ) { o=0x14; }
    if( (seed & 0x200) == 0x200 ) { memcpy( (void*)p+0x14, (void*)key, 16 ); o=0x24; }
    
    int i;
    for(i=0; i<size; i++)
        g_buf[o+i] = buf[i]; // save 0x24 Bytes for KIRK header
    
    r = crypt(g_buf, size+o, g_buf, size+o, kcmd);
    if (r != 0)
	    return r;

    for(i=0; i<size; i++)
        buf[i] = g_buf[i];

    return 0;
}


void enablekirk( void )
{
	BusClockEnable(KIRK_BUS_CLOCK, BUS_ENABLE);
}

void disablekirk( void )
{
	BusClockEnable(KIRK_BUS_CLOCK, BUS_DISABLE);
}
