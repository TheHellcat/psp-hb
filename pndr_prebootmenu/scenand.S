.set noreorder

#include "pspimport.s"

IMPORT_START	"sceNand_driver",0x00090000
IMPORT_FUNC	"sceNand_driver",0xB2B021E5,sceNandWriteBlockWithVerify
IMPORT_FUNC "sceNand_driver",0x8933B2E0,sceNandEraseBlockWithRetry
IMPORT_FUNC "sceNand_driver",0xAE4438C7,sceNandLock
IMPORT_FUNC "sceNand_driver",0x41FFA822,sceNandUnlock


//#include "pspstub.s"

//STUB_START	"sceNand_driver",0x00010000,0x00020005
//STUB_FUNC	0xB2B021E5,sceNandWriteBlockWithVerify
//STUB_FUNC	0x8933B2E0,sceNandEraseBlockWithRetry
//STUB_FUNC	0xAE4438C7,sceNandLock
//STUB_FUNC	0x41FFA822,sceNandUnlock
//STUB_END
