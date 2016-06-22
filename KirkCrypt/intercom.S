.set noreorder

#include "pspstub.s"
#                                     v
STUB_START "intercom",0x40090000,0x00030005
STUB_FUNC 0xF53DD2F9,intercomSetKirkWorkbuffer
STUB_FUNC 0x446F9F8E,intercomEncryptBuffer
STUB_FUNC 0x36CA07FF,intercomDecryptBuffer
STUB_END
