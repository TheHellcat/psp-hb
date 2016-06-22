.set noreorder

#include "pspstub.s"

STUB_START "KernelStuff",0x40090000,0x00070005
STUB_FUNC 0x9F4FF075,kstuffInitBuffers
STUB_FUNC 0xAC575805,kstuffMsInstallFW
STUB_FUNC 0x1878D543,kstuffMsInstallAdditional
STUB_FUNC 0x06D08C9B,kstuffBatMakeService
STUB_FUNC 0xF447A181,kstuffBatMakeNormal
STUB_FUNC 0x681A0518,kstuffSetUserFileWriteFunc
STUB_FUNC 0x8F643C1F,kstuff__end
STUB_END
