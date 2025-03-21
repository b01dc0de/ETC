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
    static constexpr s32 MemSpaceSize = 65536; //(1 << 16);
    u8 *Memory;
    bool bFlagZero;
    bool bFlagSign;
    bool bEndStream;

    void InitZero();
    DataUnit CalcEffAddr(EffAddrDesc* pAddrDesc);
    DataUnit GetDataUnit(Operand* Op);
    void SetFlags8(u8 Result);
    void SetFlags16(u16 Result);
    void SetFlags(DataUnit Dst);
    void SimInst(VirtualInst* pInst);
    bool Step(u8* InstStream, int Size, bool bPrint = true);
    void Sim86(const char* FileName, bool bPrint);
    void Sim86Dump(const char* FileName, const char* OutputFileName);
};


#endif // VIRTUAL86_SIM_H

