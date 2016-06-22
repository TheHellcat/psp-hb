.set noreorder

#include "pspstub.s"

STUB_START "sceSyscon_driver",0x00010000,0x00030005 
# STUB _FUNC 0x1165C864,sceSyscon_driver_1165C864 
# STUB _FUNC 0x68EF0BEF,sceSyscon_driver_68EF0BEF 
STUB_FUNC 0x40CBBD46,sceSyscon_driver_40CBBD46
STUB_FUNC 0xB9C1B0AC,sceSyscon_driver_B9C1B0AC
STUB_FUNC 0x5B9ACC97,sceSysconCmdExec
STUB_END
