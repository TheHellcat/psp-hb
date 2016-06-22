#ifndef __GS_SDK__
#define __GS_SDK__

#define PSP_O_RDONLY 0x0001
#define PSP_O_WRONLY 0x0002
#define PSP_O_RDWR (PSP_O_RDONLY | PSP_O_WRONLY)
#define PSP_O_NBLOCK 0x0004
#define PSP_O_DIROPEN 0x0008
#define PSP_O_APPEND 0x0100
#define PSP_O_CREAT 0x0200
#define PSP_O_TRUNC 0x0400
#define PSP_O_EXCL 0x0800
#define PSP_O_NOWAIT 0x8000
#define PSP_SEEK_SET 0
#define PSP_SEEK_CUR 1
#define PSP_SEEK_END 2

enum PspCtrlButtons
{
	PSP_CTRL_SELECT     = 0x000001,
	PSP_CTRL_START      = 0x000008,
	PSP_CTRL_UP	 		= 0x000010,
	PSP_CTRL_RIGHT      = 0x000020,
	PSP_CTRL_DOWN	   	= 0x000040,
	PSP_CTRL_LEFT	   	= 0x000080,
	PSP_CTRL_LTRIGGER   = 0x000100,
	PSP_CTRL_RTRIGGER   = 0x000200,
	PSP_CTRL_TRIANGLE   = 0x001000,
	PSP_CTRL_CIRCLE     = 0x002000,
	PSP_CTRL_CROSS      = 0x004000,
	PSP_CTRL_SQUARE     = 0x008000,
	PSP_CTRL_HOME       = 0x010000,
	PSP_CTRL_HOLD       = 0x020000,
	PSP_CTRL_NOTE       = 0x800000,
	PSP_CTRL_SCREEN     = 0x400000,
	PSP_CTRL_VOLUP      = 0x100000,
	PSP_CTRL_VOLDOWN    = 0x200000,
	PSP_CTRL_WLAN_UP    = 0x040000,
	PSP_CTRL_REMOTE     = 0x080000, 
	PSP_CTRL_DISC       = 0x1000000,
	PSP_CTRL_MS	 		= 0x2000000,
};

enum PspCtrlMode
{
	/* Digitial. */
	PSP_CTRL_MODE_DIGITAL = 0,
	/* Analog. */
	PSP_CTRL_MODE_ANALOG
};

enum PspDisplayPixelFormats {
	PSP_DISPLAY_PIXEL_FORMAT_565 = 0,
	PSP_DISPLAY_PIXEL_FORMAT_5551,
	/* 16-bit RGBA 4:4:4:4. */
	PSP_DISPLAY_PIXEL_FORMAT_4444,
	/* 32-bit RGBA 8:8:8:8. */
	PSP_DISPLAY_PIXEL_FORMAT_8888
};

enum PspDisplaySetBufSync {
	PSP_DISPLAY_SETBUF_IMMEDIATE = 0, 
	PSP_DISPLAY_SETBUF_NEXTFRAME = 1
};


enum PspDisplayErrorCodes
{
   SCE_DISPLAY_ERROR_OK    = 0,   
   SCE_DISPLAY_ERROR_POINTER    = 0x80000103,   
   SCE_DISPLAY_ERROR_ARGUMENT   = 0x800  
};

typedef int SceUID;
typedef int SceMode;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long int u64;
typedef unsigned int SceSize;
typedef unsigned int SceUInt;
typedef unsigned long clock_t;
typedef unsigned long time_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef long int int64_t;
typedef int32_t SceInt32;
typedef int64_t SceInt64;
typedef uint32_t SceUInt32;
typedef SceInt64 SceOff;

typedef int (* SceKernelThreadEntry) (SceSize args, void *argp);
typedef void(* PspGeCallback)(int id, void *arg) ;

typedef struct
{
	SceSize size;
	SceUID mpidtext;
	SceUID mpiddata;
	unsigned int flags;
	char position;
	char access;
	char creserved [2];
} SceKernelLMOption;

typedef struct
{
	int unknown;
	char product [9];
} productStruct;

typedef struct
{
	SceSize size;
	SceUID stackMpid;
} SceKernelThreadOptParam;

typedef struct
{
	unsigned int context[512];
} PspGeContext;

typedef struct
{
	unsigned int size;
	PspGeContext *context;
} PspGeListArgs;

typedef struct
{
	PspGeCallback signal_func;
	void * signal_arg;
	PspGeCallback finish_func;
	void * finish_arg;
} PspGeCallbackData;

typedef struct
{
	unsigned int TimeStamp;
	unsigned int Buttons;
	unsigned char Lx;
	unsigned char Ly;
	unsigned char Rsrv [6];
} SceCtrlData;

typedef struct
{
	SceSize size;
} SceKernelSemaOptParam;

typedef struct
{
	SceSize size;
} SceKernelEventFlagOptParam;

typedef struct
{
	SceSize size;
	char name [32];
	SceUInt attr;
	SceUInt initPattern;
	SceUInt currentPattern;
	int numWaitThreads;
} SceKernelEventFlagInfo;

typedef int (*SceKernelCallbackFunction)(int arg1, int arg2, void *arg);

typedef struct
{
	SceSize size;
	char name [32];
	SceUID threadId;
	SceKernelCallbackFunction callback;
	void * common;
	int notifyCount;
	int notifyArg;

} SceKernelCallbackInfo;

typedef struct
{
	SceUInt32 low;
	SceUInt32 hi;
} SceKernelSysClock;

typedef struct
{
	SceSize size;
	char name [32];
	SceUInt attr;
	int status;
	SceKernelThreadEntry entry;
	void * stack;
	int stackSize;
	void * gpReg;
	int initPriority;
	int currentPriority;
	int waitType;
	SceUID waitId;
	int wakeupCount;
	int exitStatus;
	SceKernelSysClock runClocks;
	SceUInt intrPreemptCount;
	SceUInt threadPreemptCount;
	SceUInt releaseCount;
} SceKernelThreadInfo;

typedef struct
{
	SceSize size;
	SceUID mpidstack;
	SceSize stacksize;
	int priority;
	unsigned int attribute;
} SceKernelSMOption;

typedef struct
{
	unsigned short year;
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
	unsigned int microsecond;
} ScePspDateTime;

typedef struct
{
	SceMode st_mode;
	unsigned int st_attr;
	SceOff st_size;
	ScePspDateTime st_ctime;
	ScePspDateTime st_atime;
	ScePspDateTime st_mtime;
	unsigned int st_private [6];
} SceIoStat;

typedef struct
{
	SceIoStat d_stat;
	char d_name [256];
	void * d_private;
	int dummy;
} SceIoDirent;

typedef struct
{
	u16 year;
	u16 month;
	u16 day;
	u16 hour;
	u16 minutes;
	u16 seconds;
	u32 microseconds;
} pspTime;

typedef void * ScePVoid;
typedef ScePVoid SceMpeg;

typedef struct
{
	SceUInt32 iUnk0;
	SceInt32 iTimestamp;
	SceUInt32 iUnk1;
	SceUInt32 iUnk2;
	SceUInt32 iUnk3;
	SceUInt32 iUnk4;
} SceMpegAu;

typedef void SceVoid;

typedef SceInt32 (* sceMpegRingbufferCB)(ScePVoid pData, SceInt32 iNumPackets, ScePVoid pParam);

typedef struct
{
	SceInt32 iPackets;
	SceUInt32 iUnk0;
	SceUInt32 iUnk1;
	SceUInt32 iUnk2;
	SceUInt32 iUnk3;
	ScePVoid pData;
	sceMpegRingbufferCB Callback;
	ScePVoid pCBparam;
	SceUInt32 iUnk4;
	SceUInt32 iUnk5;
	SceMpeg pSceMpeg;
} SceMpegRingbuffer;

typedef SceVoid SceMpegStream;

typedef struct
{
	unsigned int size;
	int language;
	int buttonSwap;
	int graphicsThread;
	int accessThread;
	int fontThread;
	int soundThread;
	int result;
	int reserved [4];
} pspUtilityDialogCommon;

typedef enum
{
	PSP_UTILITY_SAVEDATA_AUTOLOAD = 0,
	PSP_UTILITY_SAVEDATA_AUTOSAVE,
	PSP_UTILITY_SAVEDATA_LOAD,
	PSP_UTILITY_SAVEDATA_SAVE,
	PSP_UTILITY_SAVEDATA_LISTLOAD,
	PSP_UTILITY_SAVEDATA_LISTSAVE,
	PSP_UTILITY_SAVEDATA_LISTDELETE,
	PSP_UTILITY_SAVEDATADELETE,

} PspUtilitySavedataMode;

typedef struct
{
	char title [0x80];
	char savedataTitle [0x80];
	char detail [0x400];
	unsigned char parentalLevel;
	unsigned char unknown [3];
} PspUtilitySavedataSFOParam;

typedef struct
{
	void * buf;
	SceSize bufSize;
	SceSize size;
	int unknown;
} PspUtilitySavedataFileData;

typedef struct
{
	PspUtilitySavedataFileData icon0;
	char * title;
} PspUtilitySavedataListSaveNewData;

typedef enum
{
	PSP_UTILITY_SAVEDATA_FOCUS_UNKNOWN = 0,
	PSP_UTILITY_SAVEDATA_FOCUS_FIRSTLIST,	/* First in list */
	PSP_UTILITY_SAVEDATA_FOCUS_LASTLIST,	/* Last in list */
	PSP_UTILITY_SAVEDATA_FOCUS_LATEST,	/* Most recent date */
	PSP_UTILITY_SAVEDATA_FOCUS_OLDEST,	/* Oldest date */
	PSP_UTILITY_SAVEDATA_FOCUS_UNKNOWN2,
	PSP_UTILITY_SAVEDATA_FOCUS_UNKNOWN3,
	PSP_UTILITY_SAVEDATA_FOCUS_FIRSTEMPTY, /* First empty slot */
	PSP_UTILITY_SAVEDATA_FOCUS_LASTEMPTY,	/*Last empty slot */
	
} PspUtilitySavedataFocus;

typedef struct
{
	pspUtilityDialogCommon base;
	PspUtilitySavedataMode mode;
	int unknown1;
	int overwrite;
	char gameName [13];
	char reserved [3];
	char saveName [20];
	char(* saveNameList )[20];
	char fileName [13];
	char reserved1 [3];
	void * dataBuf;
	SceSize dataBufSize;
	SceSize dataSize;
	PspUtilitySavedataSFOParam sfoParam;
	PspUtilitySavedataFileData icon0FileData;
	PspUtilitySavedataFileData icon1FileData;
	PspUtilitySavedataFileData pic1FileData;
	PspUtilitySavedataFileData snd0FileData;
	PspUtilitySavedataListSaveNewData * newData;
	PspUtilitySavedataFocus focus;
	int unknown2 [4];
} SceUtilitySavedataParam;

typedef void (*sceNetAdhocctlHandler)(int flag, int error, void *unknown);

void (* sceKernelExitGame) (void) = (void *) 0x08A69404;
int (* sceKernelRegisterExitCallback) (int cbid) = (void *) 0x08A6940C;
int (* sceAtracReleaseAtracID) (int atracID) = (void *) 0x08A69414;
void (* sceAtracSetData) (void *buf, SceSize bufsize) = (void *) 0x08A6941C;
int (* sceAtracSetDataAndGetID) (void *buf, SceSize bufsize) = (void *) 0x08A69424;
int (* sceAtracDecodeData) (int atracID, u16 *outSamples, int *outN, int *outEnd, int *outRemainFrame) = (void *) 0x08A6942C;
int (* sceAtracGetRemainFrame) (int atracID, int *outRemainFrame) = (void *) 0x08A69434;
int (* sceAtracGetStreamDataInfo) (int atracID, u8 **writePointer, u32 *availableBytes, u32 *readOffset) = (void *) 0x08A6943C;
int (* sceAtracAddStreamData) (int atracID, unsigned int bytesToAdd) = (void *) 0x08A69444;
void (* sceAtracGetSoundSample) (void) = (void *) 0x08A6944C;
void (* sceAtracGetChannel) (void) = (void *) 0x08A69454;
int (* sceAtracGetBitrate) (int atracID, int *outBitrate) = (void *) 0x08A6945C;
int (* sceAtracSetLoopNum) (int atracID, int nloops) = (void *) 0x08A69464;
int (* __sceSasCore) (int sasCore, int outBufPtr) = (void *) 0x08A6946C;
int (* __sceSasGetEndFlag) (int sasCore) = (void *) 0x08A69474;
int (* __sceSasSetVolume) (int sasCore, int num, int l, int el, int r, int er) = (void *) 0x08A6947C;
int (* __sceSasSetPitch) (int sasCore, int num, int pitch) = (void *) 0x08A69484;
int (* __sceSasSetVoice) (int sasCore, int num, int vagPtr, int size, int loopmode) = (void *) 0x08A6948C;
int (* __sceSasSetNoise) (void) = (void *) 0x08A69494;
int (* __sceSasSetADSR) (int sasCore, int num, int flag, int a, int d, int s, int r) = (void *) 0x08A6949C;
int (* __sceSasSetADSRmode) (int sasCore, int num, int aMode, int dMode, int sMode, int rMode) = (void *) 0x08A694A4;
int (* __sceSasSetSL) (void) = (void *) 0x08A694AC;
int (* __sceSasGetEnvelopeHeight) (int sasCore, int num) = (void *) 0x08A694B4;
int (* __sceSasSetSimpleADSR) (void) = (void *) 0x08A694BC;
int (* __sceSasInit) (void) = (void *) 0x08A694C4;
int (* __sceSasSetKeyOff) (int sasCore, int num) = (void *) 0x08A694CC;
int (* __sceSasSetKeyOn) (int sasCore, int num) = (void *) 0x08A694D4;
int (* __sceSasRevVON) (void) = (void *) 0x08A694DC;
int (* __sceSasRevEVOL) (void) = (void *) 0x08A694E4;
int (* __sceSasRevType) (void) = (void *) 0x08A694EC;
int (*__sceSasRevParam) (void) = (void *) 0x08A694F4;
int (* sceNetAdhocctlInit) (int unk1, int unk2, struct productStruct *product) = (void *) 0x08A694FC;
int (* sceNetAdhocctlTerm) (void) = (void *) 0x08A69504;
int (* sceNetAdhocctlConnect) (int *unk1) = (void *) 0x08A6950C;
int (* sceNetAdhocctlDisconnect) (void) = (void *) 0x08A69514;
int (* sceNetAdhocctlAddHandler) (sceNetAdhocctlHandler handler, void *unknown) = (void *) 0x08A6951C;
int (* sceNetAdhocctlDelHandler) (int id) = (void *) 0x08A69524;
int (* sceNetAdhocInit) (void) = (void *) 0x08A6952C;
int (* sceNetAdhocTerm) (void) = (void *) 0x08A69534;
int (* sceNetAdhocPollSocket) (void) = (void *) 0x08A6953C;
int (* sceNetAdhocPdpCreate) (unsigned char *mac, unsigned short port, unsigned int bufsize, int unk1) = (void *) 0x08A69544;
int (* sceNetAdhocPdpSend) (int id, unsigned char *destMacAddr, unsigned short port, void *data, unsigned int len, unsigned int timeout, int nonblock) = (void *) 0x08A6954C;
int (* sceNetAdhocPdpRecv) (int id, unsigned char *srcMacAddr, unsigned short *port, void *data, void *dataLength, unsigned int timeout, int nonblock) = (void *) 0x08A69554;
int (* sceNetAdhocPdpDelete) (int id, int unk1) = (void *) 0x08A6955C;
void * (* sceGeEdramGetAddr) (void) = (void *) 0x08A69564;
int (* sceGeListEnQueue) (const void *list, void *stall, int cbid, PspGeListArgs *arg) = (void *) 0x08A6956C;
int (* sceGeListEnQueueHead) (const void *list, void *stall, int cbid, PspGeListArgs *arg) = (void *) 0x08A69574;
int (* sceGeListUpdateStallAddr) (int qid, void *stall) = (void *) 0x08A6957C;
int (* sceGeListSync) (int qid, int syncType) = (void *) 0x08A69584;
int (* sceGeDrawSync) (int syncType) = (void *) 0x08A6958C;
void (* sceGeBreak) (int unk1) = (void *) 0x08A69594;
void (* sceGeContinue) (int unk1) = (void *) 0x08A6959C;
int (* sceGeSetCallback) (PspGeCallbackData *cb) = (void *) 0x08A695A4;
int (* sceGeUnsetCallback) (int cbid) = (void *) 0x08A695AC;
unsigned int (* sceGeEdramGetSize) (void) = (void *) 0x08A695B4;
int (* sceDisplaySetMode) (int mode, int width, int height) = (void *) 0x08A695BC;
int (* sceDisplayGetMode) (int *pmode, int *pwidth, int *pheight) = (void *) 0x08A695C4;
float (* sceDisplayGetFramePerSec) (void) = (void *) 0x08A695CC;
int (* sceDisplaySetFrameBuf) (void *topaddr, int bufferwidth, int pixelformat, int sync) = (void *) 0x08A695D4;
int (* sceDisplayGetFrameBuf) (void **topaddr, int *bufferwidth, int *pixelformat, int sync) = (void *) 0x08A695DC;
unsigned int (* sceDisplayGetVcount) (void) = (void *) 0x08A695E4;
int (* sceDisplayWaitVblankStart) (void) = (void *) 0x08A695EC;
int (* sceDisplayWaitVblankStartCB) = (void *) 0x08A695F4;
int (* sceCtrlReadBufferPositive) (SceCtrlData *pad_data, int count) = (void *) 0x08A695FC;
int (* sceCtrlSetSamplingCycle) (int cycle) = (void *) 0x08A69604;
int (* sceCtrlSetSamplingMode) (int mode) = (void *) 0x08A6960C;
void (* sceKernelPowerTick) (void) = (void *) 0x08A69614;
int (* sceKernelVolatileMemTryLock) (int unk, int ptr, int size) = (void *) 0x08A6961C;
int (* sceKernelVolatileMemUnlock) (void) = (void *) 0x08A69624;
void (* sceKernelSetGPO)(void) = (void *) 0x08A6962C;
clock_t (* sceKernelLibcClock) (void) = (void *) 0x08A69634;
time_t (* sceKernelLibcTime) (time_t *t) = (void *) 0x08A6963C;
int (* sceKernelLibcGettimeofday) (struct timeval *tp, struct timezone *tzp) = (void *) 0x08A69644;
void (* sceKernelDcacheInvalidateRange) (const void *p, unsigned int size) = (void *) 0x08A6965C;
void (* sceKernelDcacheWritebackAll) (void) = (void *) 0x08A6964C;
void (* sceKernelDcacheWritebackRange) (const void *p, unsigned int size) = (void *) 0x08A69654;
void (* sceKernelDcacheWritebackInvalidateRange) (const void *p, unsigned int size) = (void *) 0x08A6965C;
int (* sceKernelCheckCallback) (void) = (void *) 0x08A69664;
SceUID (* sceKernelCreateThread) (const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam *option) = (void *) 0x08A6966C;
int (* sceKernelDeleteThread) (SceUID thid) = (void *) 0x08A69674;
int (* sceKernelStartThread) (SceUID thid, SceSize arglen, void *argp) = (void *) 0x08A6967C;
int (* sceKernelExitThread) (int status) = (void *) 0x08A69684;
int (* sceKernelExitDeleteThread) (int status) = (void *) 0x08A6968C;
int (* sceKernelTerminateDeleteThread) (SceUID thid) = (void *) 0x08A69694;
int (* sceKernelChangeCurrentThreadAttr) (int unknown, SceUInt attr) = (void *) 0x08A6969C;
int (* sceKernelChangeThreadPriority) (SceUID thid, int priority) = (void *) 0x08A696A4;
int (* sceKernelGetThreadId) (void) = (void *) 0x08A696AC;
int (* sceKernelGetThreadCurrentPriority) (void) = (void *) 0x08A696B4;
int (* sceKernelWaitThreadEnd) (SceUID thid, SceUInt *timeout) = (void *) 0x08A696BC;
int (* sceKernelWaitThreadEndCB) (SceUID thid, SceUInt *timeout) = (void *) 0x08A696C4;
int (* sceKernelDelayThread) (SceUInt delay) = (void *) 0x08A696CC;
int (* sceKernelDelayThreadCB) (SceUInt delay) = (void *) 0x08A696D4;
SceUID (* sceKernelCreateSema) (const char *name, SceUInt attr, int initVal, int maxVal, SceKernelSemaOptParam *option) = (void *) 0x08A696DC;
int (* sceKernelDeleteSema) (SceUID semaid) = (void *) 0x08A696E4;
int (* sceKernelSignalSema) (SceUID semaid, int signal) = (void *) 0x08A696EC;
int (* sceKernelWaitSema) (SceUID semaid, int signal, SceUInt *timeout) = (void *) 0x08A696F4;
int (* sceKernelWaitSemaCB) (SceUID semaid, int signal, SceUInt *timeout) = (void *) 0x08A696FC;
SceUID (* sceKernelCreateEventFlag) (const char *name, int attr, int bits, SceKernelEventFlagOptParam *opt) = (void *) 0x08A69704;
int (* sceKernelDeleteEventFlag) (int evid) = (void *) 0x08A6970C;
int (* sceKernelSetEventFlag) (SceUID evid, u32 bits) = (void *) 0x08A69714;
int (* sceKernelClearEventFlag) (SceUID evid, u32 bits) = (void *) 0x08A6971C;
int (* sceKernelWaitEventFlag) (int evid, u32 bits, u32 wait, u32 *outBits, SceUInt *timeout) = (void *) 0x08A69724;
int (* sceKernelReferEventFlagStatus) (SceUID event, SceKernelEventFlagInfo *status) = (void *) 0x08A6972C;
int (* sceKernelCreateCallback) (const char *name, SceKernelCallbackFunction func, void *arg) = (void *) 0x08A69734;
int (* sceKernelDeleteCallback) (SceUID cb) = (void *) 0x08A6973C;
unsigned int (* sceKernelGetSystemTimeLow) (void) = (void *) 0x08A69744;
int (* sceKernelReferThreadStatus) (SceUID thid, SceKernelThreadInfo *info) = (void *) 0x08A6974C;
int (* sceKernelSleepThread) (void) = (void *) 0x08A69754;
int (* sceKernelWakeupThread) (SceUID thid) = (void *) 0x08A6975C;
int (* sceKernelSuspendThread) (SceUID thid) = (void *) 0x08A69764;
int (* sceKernelResumeThread) (SceUID thid) = (void *) 0x08A6976C;
int (* sceKernelPollSema) (SceUID semaid, int signal) = (void *) 0x08A69774;
int (* sceKernelCreateFpl) (const char *name, int part, int attr, unsigned int size, unsigned int blocks, struct SceKernelFplOptParam *opt) = (void *) 0x08A6977C;
int (* sceKernelAllocateFpl) (SceUID uid, void **data, unsigned int *timeout) = (void *) 0x08A69784;
SceInt64 (* sceKernelGetSystemTimeWide) (void) = (void *) 0x08A6978C;
SceUID (* sceKernelAllocPartitionMemory) (SceUID partitionid, const char *name, int type, SceSize size, void *addr) = (void *) 0x08A69794;
int (* sceKernelFreePartitionMemory) (SceUID blockid) = (void *) 0x08A6979C;
void * (* sceKernelGetBlockHeadAddr) (SceUID blockid) = (void *) 0x08A697A4;
SceSize (* sceKernelMaxFreeMemSize) (void) = (void *) 0x08A697AC;
SceUID (* sceKernelStdin) (void) = (void *) 0x08A697B4;
SceUID (* sceKernelStdout) (void) = (void *) 0x08A697BC;
SceUID (* sceKernelStderr) (void) = (void *) 0x08A697C4;
int (* sceKernelSelfStopUnloadModule) (int unknown, SceSize argsize, void *argp) = (void *) 0x08A697CC;
SceUID (* sceKernelLoadModule) (const char *path, int flags, SceKernelLMOption *option);
int (* sceKernelStartModule) (SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option) = (void *) 0x08A697DC;
int (* sceKernelStopModule) (SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option) = (void *) 0x08A697E4;
int (* sceKernelUnloadModule) (SceUID modid) = (void *) 0x08A697EC;
SceUID (* sceIoOpenAsync) (const char *file, int flags, SceMode mode) = (void *) 0x08A697F4;
int (* sceIoRead) (SceUID fd, void *data, SceSize size) = (void *) 0x08A697FC;
int (* sceIoReadAsync) (SceUID fd, void *data, SceSize size) = (void *) 0x08A69804;
int (* sceIoWrite) (SceUID fd, const void *data, SceSize size) = (void *) 0x08A6980C;
SceOff (* sceIoLseek) (SceUID fd, SceOff offset, int whence) = (void *) 0x08A69814;
int (* sceIoLseekAsync) (SceUID fd, SceOff offset, int whence) = (void *) 0x08A6981C;
SceUID (* sceIoDopen) (const char *dirname) = (void *) 0x08A69824;
int (* sceIoDclose) (SceUID fd) = (void *) 0x08A6982C;
int (* sceIoGetstat) (const char *file, SceIoStat *stat) = (void *) 0x08A69834;
int (* sceIoWaitAsyncCB) (SceUID fd, SceInt64 *res) = (void *) 0x08A6983C;
int (* sceIoRename) (const char *oldname, const char *newname) = (void *) 0x08A69844;
int (* sceIoDevctl) (const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) = (void *) 0x08A6984C;
int (* sceIoClose) (SceUID fd) = (void *) 0x08A69854;
SceUID (* sceIoOpen) (const char *file, int flags, SceMode mode) = (void *) 0x08A6985C;
int (* sceIoWriteAsync) (SceUID fd, const void *data, SceSize size) = (void *) 0x08A69864;
int (* sceIoDread) (SceUID fd, SceIoDirent *dir) = (void *) 0x08A6986C;
int (* sceIoRemove) (const char *file) = (void *) 0x08A69874;
int (* sceIoMkdir) (const char *dir, SceMode mode) = (void *) 0x08A6987C;
int (* sceIoRmdir) (const char *path) = (void *) 0x08A69884;
int (* sceIoCloseAsync) (SceUID fd) = (void *) 0x08A6988C;
void * (* sceFontNewLib)(void *params, int *error) = (void *) 0x08A69894;
void (* sceFontGetNumFontList) (void) = (void *) 0x08A6989C;
void (* sceFontGetFontList) (void) = (void *) 0x08A698A4;
void (* sceFontOpen) (void) = (void *) 0x08A698AC;
void (* sceFontGetFontInfo) (void) = (void *) 0x08A698B4;
void (* sceFontGetCharGlyphImage) (void) = (void *) 0x08A698BC;
int (* sceRtcGetTick) (const pspTime *date, u64 *tick) = (void *) 0x08A698C4;
int (* scePowerRegisterCallback) (int slot, SceUID cbid) = (void *) 0x08A698CC;
int (* scePowerUnregisterCallback) (int slot) = (void *) 0x08A698D4;
SceInt32 (* sceMpegQueryStreamOffset) (SceMpeg *Mpeg, ScePVoid pBuffer, SceInt32 *iOffset) = (void *) 0x08A698DC;
SceInt32 (* sceMpegQueryStreamSize) (ScePVoid pBuffer, SceInt32 *iSize) = (void *) 0x08A698E4;
SceInt32 (* sceMpegInit) (void) = (void *) 0x08A698EC;
SceVoid (* sceMpegFinish) () = (void *) 0x08A698F4;
SceInt32 (* sceMpegQueryMemSize) (int iUnk) = (void *) 0x08A698FC;
SceInt32 (* sceMpegCreate) (SceMpeg *Mpeg, ScePVoid pData, SceInt32 iSize, SceMpegRingbuffer *Ringbuffer, SceInt32 iFrameWidth, SceInt32 iUnk1, SceInt32 iUnk2) = (void *) 0x08A69904;
SceVoid (* sceMpegDelete) (SceMpeg *Mpeg) = (void *) 0x08A6990C;
SceMpegStream * (* sceMpegRegistStream) (SceMpeg *Mpeg, SceInt32 iStreamID, SceInt32 iUnk) = (void *) 0x08A69914;
SceVoid (* sceMpegUnRegistStream) (SceMpeg Mpeg, SceMpegStream *pStream) = (void *) 0x08A6991C;
ScePVoid (* sceMpegMallocAvcEsBuf) (SceMpeg *Mpeg) = (void *) 0x08A69924;
SceVoid (* sceMpegFreeAvcEsBuf) (SceMpeg *Mpeg, ScePVoid pBuf) = (void *) 0x08A6992C;
SceInt32 (* sceMpegQueryAtracEsSize) (SceMpeg *Mpeg, SceInt32 *iEsSize, SceInt32 *iOutSize) = (void *) 0x08A69934;
SceInt32 (* sceMpegInitAu) (SceMpeg *Mpeg, ScePVoid pEsBuffer, SceMpegAu *pAu) = (void *) 0x08A6993C;
SceInt32 (* sceMpegGetAvcAu) (SceMpeg *Mpeg, SceMpegStream *pStream, SceMpegAu *pAu, SceInt32 *iUnk) = (void *) 0x08A69944;
SceInt32 (* sceMpegGetAtracAu) (SceMpeg *Mpeg, SceMpegStream *pStream, SceMpegAu *pAu, ScePVoid pUnk) = (void *) 0x08A6994C;
SceInt32 (* sceMpegFlushAllStream) (SceMpeg *Mpeg) = (void *) 0x08A69954;
SceInt32 (* sceMpegAvcDecode) (SceMpeg *Mpeg, SceMpegAu *pAu, SceInt32 iFrameWidth, ScePVoid pBuffer, SceInt32 *iInit) = (void *) 0x08A6995C;
SceInt32 (* sceMpegAvcDecodeStop) (SceMpeg *Mpeg, SceInt32 iFrameWidth, ScePVoid pBuffer, SceInt32 *iStatus) = (void *) 0x08A69964;
int (* sceMpegAvcDecodeFlush) (void) = (void *) 0x08A6996C;
SceInt32 (* sceMpegAtracDecode) (SceMpeg *Mpeg, SceMpegAu *pAu, ScePVoid pBuffer, SceInt32 iInit) = (void *) 0x08A69974;
SceInt32 (* sceMpegRingbufferQueryMemSize) (SceInt32 iPackets) = (void *) 0x08A6997C;
SceInt32 (* sceMpegRingbufferConstruct) (SceMpegRingbuffer *Ringbuffer, SceInt32 iPackets, ScePVoid pData, SceInt32 iSize, sceMpegRingbufferCB Callback, ScePVoid pCBparam) = (void *) 0x08A69984;
SceVoid (* sceMpegRingbufferDestruct) (SceMpegRingbuffer *Ringbuffer) = (void *) 0x08A6998C;
SceInt32 (* sceMpegRingbufferPut) (SceMpegRingbuffer *Ringbuffer, SceInt32 iNumPackets, SceInt32 iAvailable) = (void *) 0x08A69994;
SceInt32 (* sceMpegRingbufferAvailableSize) (SceMpegRingbuffer *Ringbuffer) = (void *) 0x08A6999C;
int (* sceWlanGetSwitchState) (void) = (void *) 0x08A699A4;
int (* sceWlanGetEtherAddr) (u8 *etherAddr) = (void *) 0x08A699AC;
int (* sceUtilitySavedataInitStart) (SceUtilitySavedataParam *params) = (void *) 0x08A699B4;
int (* sceUtilitySavedataShutdownStart) (void) = (void *) 0x08A699BC;
void (* sceUtilitySavedataUpdate) (int unknown)  = (void *) 0x08A699C4;
int (* sceUtilitySavedataGetStatus) (void) = (void *) 0x08A699CC;
int (* sceUtilityGetSystemParamInt) (int id, int *value) = (void *) 0x08A699D4;
int (* sceUtilityGetSystemParamString) (int id, char *str, int len) = (void *) 0x08A699DC;
int (* sceDmacMemcpy) (void *dest, const void *source, unsigned int size) = (void *) 0x08A699E4;
int (* sceAudioOutputBlocking) (int channel, int vol, void *buf) = (void *) 0x08A699EC;
int (* sceAudioOutputPanned) (int channel, int leftvol, int rightvol, void *buf) = (void *) 0x08A699F4;
int (* sceAudioOutputPannedBlocking) (int channel, int leftvol, int rightvol, void *buf) = (void *) 0x08A699FC;
int (* sceAudioChReserve) (int channel, int samplecount, int format) = (void *) 0x08A69A04;
int (* sceAudioChRelease) (int channel) = (void *) 0x08A69A0C;
int (* sceAudioGetChannelRestLen) (int channel) = (void *) 0x08A69A14;
int (* sceAudioSetChannelDataLen) (int channel, int samplecount) = (void *) 0x08A69A1C;
int (* sceAudioChangeChannelConfig) (int channel, int format) = (void *) 0x08A69A24;
int (* sceAudioChangeChannelVolume) (int channel, int leftvol, int rightvol) = (void *) 0x08A69A2C;
int (* sceUmdWaitDriveStat) (int stat) = (void *) 0x08A69A34;
int (* sceUmdWaitDriveStatCB) (int stat, unsigned int timeout) = (void *) 0x08A69A3C;
int (* sceUmdCancelWaitDriveStat) (void) = (void *) 0x08A69A44;
int (* sceUmdGetDriveStat) (void) = (void *) 0x08A69A4C;
int (* sceUmdRegisterUMDCallBack) (int cbid) = (void *) 0x08A69A54;
int (* sceUmdUnRegisterUMDCallBack) (int cbid) = (void *) 0x08A69A5C;
int (* sceUmdCheckMedium) (void) = (void *) 0x08A69A64;
int (* sceUmdActivate) (int unit, const char *drive) = (void *) 0x08A69A6C;
int (* sceNetEtherNtostr) (unsigned char *mac, char *name) = (void *) 0x08A69A74;
int (* sceNetInit) (int unk1, int unk2, int unk3, int unk4, int unk5) = (void *) 0x08A69A7C;
int (* sceNetTerm) (void) = (void *) 0x08A69A84;
void (* sceNetEtherStrton) (char *name, unsigned char *mac) = (void *) 0x08A69A8C;
int (* sceNetGetLocalEtherAddr) (unsigned char *mac) = (void *) 0x08A69A94;

void *memset(void *s, int c, int count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

void io_write_hex(SceUID fd, unsigned int i)
{
	sceIoWrite(fd, "0x", 2);
	u8 digits[8];
	digits[0] = i && 0x0000000F;
	digits[1] = (i && 0x000000F0) >> 1;
	digits[2] = (i && 0x00000F00) >> 2;
	digits[3] = (i && 0x0000F000) >> 3;
	digits[4] = (i && 0x000F0000) >> 4;
	digits[5] = (i && 0x00F00000) >> 5;
	digits[6] = (i && 0x0F000000) >> 6;
	digits[7] = (i && 0xF0000000) >> 7;
	for(i = 0; i<8; i++)
		sceIoWrite(fd, "0123456789ABCDEF"+digits[i], 1);
	return;
}

void io_write_hex64(SceUID fd, unsigned long int i)
{
	sceIoWrite(fd, "0x", 2);
	u8 digits[16];
	digits[0] = i && 0x0000000F;
	digits[1] = (i && 0x000000F0) >> 1;
	digits[2] = (i && 0x00000F00) >> 2;
	digits[3] = (i && 0x0000F000) >> 3;
	digits[4] = (i && 0x000F0000) >> 4;
	digits[5] = (i && 0x00F00000) >> 5;
	digits[6] = (i && 0x0F000000) >> 6;
	digits[7] = (i && 0xF0000000) >> 7;
	digits[8] = (i && 0xF00000000) >> 8;
	digits[9] = (i && 0xF000000000) >> 9;
	digits[10] = (i && 0xF0000000000) >> 10;
	digits[11] = (i && 0xF00000000000) >> 11;
	digits[12] = (i && 0xF000000000000) >> 12;
	digits[13] = (i && 0xF0000000000000) >> 13;
	digits[14] = (i && 0xF00000000000000) >> 14;
	digits[15] = (i && 0xF000000000000000) >> 15;
	for(i = 0; i<15; i++)
		sceIoWrite(fd, "0123456789ABCDEF"+digits[i], 1);
	return;
}
#endif
