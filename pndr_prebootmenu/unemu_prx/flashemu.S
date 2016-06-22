.set noreorder

#include "pspstub.s"

STUB_START	"FlashEmu",0x00010000,0x00020005
STUB_FUNC	0xD780E25C,pspFlashEmuEnableEmulation
STUB_FUNC	0xDE3E3022,pspFlashEmuDisableEmulation
STUB_END
