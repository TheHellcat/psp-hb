
/*
    #########################################################
    ##                                                     ##
    ##   R e c o v e r y   F l a s h e r                   ##
    ##   by ---==> HELLCAT <==---                          ##
    ##                                                     ##
    #########################################################

    ====  Kernel Helper Module  ====
    --------------------------------
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "silvercode.h"
#include "ictypes.h"


PSP_MODULE_INFO("InterCom_driver", 0x1006, 1, 1);
PSP_MAIN_THREAD_ATTR(0);


//////////////////////////////
//                          //
//   Declares and Defines   //
//                          //
//////////////////////////////

// int w/e
// int memlmd_E706CCBC(u8 *buf, int bufSize, int *outSize);


//////////////////////////////
//                          //
//   Main Code              //
//   (exported functions)   //
//                          //
//////////////////////////////

int module_start(SceSize args, void *argp)
{
  return 0;
}
int module_stop( void )
{
  return 0;
}


int intercomEncryptBuffer( intercomDataHeader* dataHeader, void* dataBuffer )
{
	void* p;
	int i;
	int j;
	int c;
	int r;
	int seedmod;
	int round;
	int seedmask;
	int seedbase;
	
	enablekirk();
	r = 0;
	seedmod = 0;
	
	if( dataHeader->mseedcount == 0 )
  {
  	if( ((dataHeader->dataSeed & 0x200) == 0x200) && (dataHeader->dataSize < 512) )
    {
      // KIRK bug (?) with smaller buffers and custom IV.... no go here
      return 0x80CA9001;
    }
  		  
  	if( dataHeader->usecustomkey == 1 )
  	{
  		r = encryptky( (u8*)dataBuffer, dataHeader->dataSize, dataHeader->dataSeed, (u8*)dataHeader->customkey );
  	} else {
  		r = encrypt( (u8*)dataBuffer, dataHeader->dataSize, dataHeader->dataSeed );
  	}
  } else {
  	// two rounds, first one already affecting seedmod for block 1 of round 2
  	for( round=0; round<=1; round++ )  // 0 1
  	{
  	  p = dataBuffer;
  	  c = 0;
  	  for( i=1; i<=((dataHeader->dataSize/dataHeader->mchunkSize)-round); i++)
    	{
    		// set up a "mask" for the seed + mod to ensure seed being within propper range
  		  seedmask = 0xFFFF; seedbase = 0x0000;
  		  if( (dataHeader->mseeds[c] & 0x100) == 0x100 ) { seedmask = 0x1FF; seedbase = 0x100; }
  		  if( (dataHeader->mseeds[c] & 0x200) == 0x200 ) { seedmask = 0x2FF; seedbase = 0x200; }
  		  if( dataHeader->mseeds[c] <= 0x3F ) { seedmask = 0x3F; seedbase = 0x00; }
  		  	
  		  if( (seedbase == 0x200) && (dataHeader->mchunkSize < 512) )
  		  {
  		  	// KIRK bug (?) with smaller buffers and custom IV.... no go here
  		  	return 0x80CA9002;
  		  }
  		  
  		  // pre crypt work:
  		  // scramble current plaintext block based on EVERYTHING around it, preceding crypted as well as following plain text
  		  for( j=0; j<((int)p)-((int)dataBuffer); j++ )  // preceding already crypted
  		  {
  		  	*((char*)(p + (j % dataHeader->mchunkSize))) ^= *((char*)(dataBuffer + j));
  		  }
  		  
  		  // the actual KIRK'ing
  	  	if( dataHeader->usecustomkey == 1 )
  	    {
  	  	  r = encryptky( (u8*)p, dataHeader->mchunkSize, ((dataHeader->mseeds[c] ^ seedmod) & seedmask) | seedbase, (u8*)dataHeader->customkey );
  	    } else {
  		    r = encrypt( (u8*)p, dataHeader->mchunkSize, ((dataHeader->mseeds[c] ^ seedmod) & seedmask) | seedbase );
  		  }
  		  
  		  // post crypt work:
  		  // generade modification value for next block based on current one
  		  seedmod = 0;
  		  for( j=0; j<dataHeader->mchunkSize; j++)
  		  {
  			  seedmod ^= *(char*)(p+j);
  		  }
  		  
  		  p += dataHeader->mchunkSize;
  		  c++;
  		  if( c == dataHeader->mseedcount ) { c = 0; }
  	  }
    }
  }
  
  disablekirk();
  
  return r;
}


int intercomDecryptBuffer( intercomDataHeader* dataHeader, void* dataBuffer )
{
	void* p;
	int i;
	int j;
	int c;
	int r;
	int seedmod;
	int numchunks;
	int round;
	int seedmask;
	int seedbase;
	
	////////////////////////////////////////////////////////////////////
	
	/* enablekirk();
	c = pspSdkSetK1( 0 );
	i = 0;
	r = memlmd_E706CCBC((u8*) dataBuffer, dataHeader->dataSize, &i);
	pspSdkSetK1( c );
	if( i==0 )
	[
	  return r;
	} else {
		return i;
	} */
	
	////////////////////////////////////////////////////////////////////
	
	enablekirk();
	
	r = 0;
	seedmod = 0;
	numchunks = dataHeader->dataSize/dataHeader->mchunkSize;
	if( dataHeader->mseedcount == 0 )
  {
  	if( ((dataHeader->dataSeed & 0x200) == 0x200) && (dataHeader->dataSize < 512) )
    {
      // KIRK bug (?) with smaller buffers and custom IV.... no go here
      return 0x80CA9001;
    }
    
  	if( dataHeader->usecustomkey == 1 )
  	{
  		r = decryptky( (u8*)dataBuffer, dataHeader->dataSize, dataHeader->dataSeed, (u8*)dataHeader->customkey );
  	} else {
  	  r = decrypt( (u8*)dataBuffer, dataHeader->dataSize, dataHeader->dataSeed );
  	}
  } else {
  	for( round=1; round>=0; round-- )  // 1 0
    {
  	  p = dataBuffer + ((numchunks-1-round) * dataHeader->mchunkSize);  // get propper end of buffer in case it's not propperly aligned / padded (buffer size not multiple of chunksize)
  	  c = (numchunks % dataHeader->mseedcount) - 1 - round;
  	  if( c == -2 ) { c = dataHeader->mseedcount - 2; }
  	  if( c == -1 ) { c = dataHeader->mseedcount - 1; }
  	  for( i=numchunks-round; i>=1; i--)
  	  {
  		  seedmod = 0;
  		  if( (round==1) && (i==1) )
  		  {  // in the first round, for block 1 of the buffer, generate seedmod from LAST block
  		  	for( j=0; j<dataHeader->mchunkSize; j++)
  		    {
  			    seedmod ^= *(char*)((dataBuffer + ((numchunks-1) * dataHeader->mchunkSize))+j);
  		    }
  		  } else if( (round==0) && (i==1) ) {
  		    // in the second round, for block 1 of the buffer, set seedmod to zero
  		  	seedmod = 0;
  		  } else {
  		  	// generate seedmod from preceeding block
  		    for( j=0; j<dataHeader->mchunkSize; j++)
  		    {
  			    seedmod ^= *(char*)(p-dataHeader->mchunkSize+j);
  		    }
  		  }
  		  
  		  // set up a "mask" for the seed + mod to ensure seed being within propper range
  		  seedmask = 0xFFFF; seedbase = 0x0000;
  		  if( (dataHeader->mseeds[c] & 0x100) == 0x100 ) { seedmask = 0x1FF; seedbase = 0x100; }
  		  if( (dataHeader->mseeds[c] & 0x200) == 0x200 ) { seedmask = 0x2FF; seedbase = 0x200; }
  		  if( dataHeader->mseeds[c] <= 0x3F ) { seedmask = 0x3F; seedbase = 0x00; }
  		  	
  		  if( (seedbase == 0x200) && (dataHeader->mchunkSize < 512) )
  		  {
  		  	// KIRK bug (?) with smaller buffers and custom IV.... no go here
  		  	return 0x80CA9002;
  		  }
  		  
  		  if( dataHeader->usecustomkey == 1 )
  	    {
  		    r = decryptky( (u8*)p, dataHeader->mchunkSize, ((dataHeader->mseeds[c] ^ seedmod) & seedmask) | seedbase, (u8*)dataHeader->customkey );
  		  } else {
  			  r = decrypt( (u8*)p, dataHeader->mchunkSize, ((dataHeader->mseeds[c] ^ seedmod) & seedmask) | seedbase );
  		  }
  		  
  		  for( j=((int)p)-((int)dataBuffer)-1; j>=0; j-- )
  		  {
  		  	*((char*)(p + (j % dataHeader->mchunkSize))) ^= *((char*)(dataBuffer + j));
  		  }
  		  
  		  p -= dataHeader->mchunkSize;
  		  c--;
  		  if( c < 0  ) { c = dataHeader->mseedcount - 1; }
  	  }
  	}
  }
  
  disablekirk();
  
  return r;
}


void intercomSetKirkWorkbuffer( void* buffer )
{
	setkirkworkbuffer( buffer );
}
