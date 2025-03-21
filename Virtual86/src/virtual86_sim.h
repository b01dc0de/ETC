#ifndef VIRTUAL86_SIM_H
#define VIRTUAL86_SIM_H

#include "virtual86_common.h"

struct DataUnit
{
    u8* Ptr;
    bool bWide;
};

struct Sim86State
{
    u16 Registers[8];
    u16 IP;
    bool bFlagZero;
    bool bFlagSign;
    bool bEndStream;

    void Clear();
    void SetFlags8(u8 Result);
    void SetFlags16(u16 Result);
    void SetFlags(DataUnit Dst);
    void SimInst(VirtualInst* pInst);
    bool Sim(u8* InstStream, int Size, bool bPrint = true);
};

Sim86State Sim86(const char* FileName, bool bPrint);

#endif // VIRTUAL86_SIM_H

