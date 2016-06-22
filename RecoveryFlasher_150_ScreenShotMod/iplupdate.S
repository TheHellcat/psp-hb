.set noreorder

#include "pspstub.s"

STUB_START "pspIplUpdate",0x40090000,0x00020005
STUB_FUNC 0xF7EF8E3F,pspIplUpdate_F7EF8E3F
STUB_FUNC 0x282EDFEC,pspIplUpdate_282EDFEC
STUB_END
