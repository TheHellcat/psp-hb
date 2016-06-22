.set noreorder

#include "pspstub.s" //included in SDK

STUB_START "scePower_driver",0x40010000, 0x20005
STUB_FUNC 0x0442D852,scePower_0442D852
STUB_FUNC 0x79DB9421,scePowerRebootStart
STUB_END

