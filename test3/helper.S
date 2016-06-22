.set noreorder

#include "pspstub.s"
#                                       v
STUB_START "hcRfHelper",0x40090000,0x00130005
STUB_FUNC 0xA9A001B0,hcRfHelperGetMoboVersions
STUB_FUNC 0x09ABEFA7,hcRfHelperLoadStartModule
STUB_FUNC 0x05FFD724,hcRfHelperPspFindProc
STUB_FUNC 0x0E24BA59,hcRfHelperIoBridgeUnassign
STUB_FUNC 0x1640F99E,hcRfHelperRebootToUpdater
STUB_FUNC 0x9A05670E,hcRfHelperIoBridgeLFatfmt
STUB_FUNC 0xC669BC22,hcRfHelperInitMangleSyscall
STUB_FUNC 0xFFCF887A,hcRfHelperPspEncrypt
STUB_FUNC 0xC4E140CD,hcRfHelperPatchLFatEncryption
STUB_FUNC 0x8B8C76B3,hcRfHelperIoBridgeWriteFile
STUB_FUNC 0x97378BFE,hcRfHelperIoBridgeUnloadModule
STUB_FUNC 0x9FF9A864,hfRfHelperPoweroff
STUB_FUNC 0xF376B327,hcRfHelperLaunchEBOOT
STUB_FUNC 0x27B970A6,hcRfHelperPspDecryptSC
STUB_FUNC 0x97C0083D,hcRfHelperDumpNandIplToBuffer
STUB_FUNC 0xF93F02FF,hcRfHelperBatSetSerial
STUB_FUNC 0x4F4035A5,hcRfHelperBatGetSerial
STUB_FUNC 0x57FC8C22,hcRfHelperDumpNandBlockRangeToBuffer
STUB_FUNC 0xFDC95CD5,hcRfHelperWriteNandBlockRange
STUB_END
