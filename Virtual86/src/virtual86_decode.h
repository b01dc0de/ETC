#ifndef VIRTUAL86_DECODE_H
#define VIRTUAL86_DECODE_H

#include "virtual86_common.h"

VirtualInst DecodeInst(u8* pInst);
//VirtualInstStream DecodeFile86(const char* FileName, bool bPrint = true);

#endif // VIRTUAL86_DECODE_H
