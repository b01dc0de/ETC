#include "virtual86_sim.h"
#include "virtual86_print.h"
#include "virtual86_decode.h"

DataUnit GetOpUnit(Sim86State* pState, Operand* Op)
{
    ASSERT(Op);
    ASSERT(pState);

    DataUnit Result = {};
    switch (Op->Type)
    {
        case OperandType_Reg:
        {
            ASSERT(!(Op->RegDesc.bWide && Op->RegDesc.bHigh));
            ASSERT(Op->RegDesc.Type != Reg_Invalid);
            Result.Ptr = (u8*)&pState->Registers[Op->RegDesc.Type-1];
            Result.bWide = Op->RegDesc.bWide;
            if (Op->RegDesc.bHigh) { Result.Ptr++; }
        } break;
        case OperandType_Imm:
        {
            Result.Ptr = &Op->ImmDesc.Data8;
            Result.bWide = Op->ImmDesc.bWide;
        } break;
        case OperandType_EffAddr:
        case OperandType_RelOffset:
        case OperandType_Invalid:
        default:
        {
            // We shouldn't be here (yet)
            ASSERT(false);
        } break;
    }
    return Result;
}

void Sim86State::Clear()
{
    for (int RegIdx = 0; RegIdx < ARRAY_SIZE(Registers); RegIdx++)
    {
        Registers[RegIdx] = 0;
    }
    IP = 0;
    bFlagZero = false;
    bFlagSign = false;
}
void Sim86State::SetFlags8(u8 Result)
{
    bFlagSign = Result & 0x80;
    bFlagZero = Result == 0;
}
void Sim86State::SetFlags16(u16 Result)
{
    bFlagSign = Result & 0x8000;
    bFlagZero = Result == 0;
}
void Sim86State::SetFlags(DataUnit Dst)
{
    if (Dst.bWide) { SetFlags16(*(u16*)Dst.Ptr); }
    else { SetFlags8(*Dst.Ptr); }
}

void Sim_OpMov(Sim86State* pState, VirtualInst* pInst)
{
    DataUnit Dst = GetOpUnit(pState, &pInst->Ops[0]);
    DataUnit Src = GetOpUnit(pState, &pInst->Ops[1]);
    ASSERT(Dst.bWide >= Src.bWide);
    if (Dst.bWide)
    {
        if (Src.bWide) { *(u16*)Dst.Ptr = *(u16*)Src.Ptr; }
        else { *(u16*)Dst.Ptr = *Src.Ptr; }
    }
    else { *Dst.Ptr = *Src.Ptr; }
}
void Sim_OpAdd(Sim86State* pState, VirtualInst* pInst)
{
    DataUnit Dst = GetOpUnit(pState, &pInst->Ops[0]);
    DataUnit Src = GetOpUnit(pState, &pInst->Ops[1]);
    ASSERT(Dst.bWide >= Src.bWide);
    if (Dst.bWide)
    {
        if (Src.bWide) { *(u16*)Dst.Ptr += *(u16*)Src.Ptr; }
        else { *(u16*)Dst.Ptr += *Src.Ptr; }
    }
    else { *Dst.Ptr += *Src.Ptr; }
    pState->SetFlags(Dst);
}
void Sim_OpSub(Sim86State* pState, VirtualInst* pInst)
{
    DataUnit Dst = GetOpUnit(pState, &pInst->Ops[0]);
    DataUnit Src = GetOpUnit(pState, &pInst->Ops[1]);
    ASSERT(Dst.bWide >= Src.bWide);
    if (Dst.bWide)
    {
        if (Src.bWide) { *(u16*)Dst.Ptr -= *(u16*)Src.Ptr; }
        else { *(u16*)Dst.Ptr -= *Src.Ptr; }
    }
    pState->SetFlags(Dst);
}
void Sim_OpCmp(Sim86State* pState, VirtualInst* pInst)
{
    DataUnit Dst = GetOpUnit(pState, &pInst->Ops[0]);
    DataUnit Src = GetOpUnit(pState, &pInst->Ops[1]);
    ASSERT(Dst.bWide >= Src.bWide);
    if (Dst.bWide)
    {
        u16 Tmp = *(u16*)Dst.Ptr;
        if (Src.bWide) { Tmp -= *(u16*)Src.Ptr; }
        else { Tmp -= *Src.Ptr; }
        pState->SetFlags16(Tmp);
    }
    else
    {
        u8 Tmp = *Dst.Ptr;
        Tmp -= *Src.Ptr;
        pState->SetFlags8(Tmp);
    }
}
bool Sim_OpJmp(Sim86State* pState, VirtualInst* pInst)
{
    ASSERT(pInst->Ops[0].Type == OperandType_RelOffset);

    bool bJmp = false;

    switch (pInst->Code)
    {
        case OpCode_JeJz: { bJmp = pState->bFlagZero; } break;
        case OpCode_JneJnz: { bJmp = !pState->bFlagZero; } break;
        case OpCode_Js: { bJmp = pState->bFlagSign; } break;
        case OpCode_Jns: { bJmp = !pState->bFlagSign; } break;
        /*
        case OpCode_JlJnge: case OpCode_JleJng: case OpCode_JbJnae: case OpCode_JbeJna:
        case OpCode_JpJpe: case OpCode_Jo: case OpCode_JnlJge: case OpCode_JnleJg:
        case OpCode_JnbJae: case OpCode_JnbeJa: case OpCode_JnpJpo: case OpCode_Jno:
        case OpCode_Loop: case OpCode_LoopzLoope: case OpCode_LoopnzLoopne: case OpCode_Jcxz:
        */
        default: { DebugBreak(); } break;
    }

    if (bJmp)
    {
        s8 Offset = pInst->Ops[0].ImmDesc.Data8;
        pState->IP += Offset;
    }
    return bJmp;
}

void Sim86State::SimInst(VirtualInst* pInst)
{
    ASSERT(pInst);
    bool bJmp = false;
    switch (pInst->Code)
    {
        case OpCode_Mov: { Sim_OpMov(this, pInst); } break;
        case OpCode_Add: { Sim_OpAdd(this, pInst); } break;
        case OpCode_Sub: { Sim_OpSub(this, pInst); } break;
        case OpCode_Cmp: { Sim_OpCmp(this, pInst); } break;
        case OpCode_JeJz:
        case OpCode_JneJnz:
        case OpCode_Js:
        case OpCode_Jns: { bJmp = Sim_OpJmp(this, pInst); } break;
        default:
        {
            // UNHANDLED INST
            DebugBreak();
        } break;
    }
    if (!bJmp) { IP += pInst->ByteWidth; }
}

bool Sim86State::Sim(u8* InstStream, int Size, bool bPrint)
{
    ASSERT(IP < Size);
    if (IP >= Size) { DebugBreak(); return true; }
    else
    {
        VirtualInst Inst = DecodeInst(InstStream + IP);
        if (bPrint) { PrintInst(&Inst); }
        SimInst(&Inst);
    }
    return IP < Size;
}

Sim86State Sim86(const char* FileName, bool bPrint)
{
    Sim86State Result = {}; Result.Clear();

    FileContentsT FileContents = ReadFileContents(FileName);
    if (FileContents.Data == nullptr) { DebugBreak(); return Result; }

    printf("; %s:\n", FileName);

    while (Result.Sim(FileContents.Data, FileContents.Size, bPrint)) { }

    if (bPrint)
    {
        PrintState(&Result);
    }

    delete[] FileContents.Data;

    return Result;
}

// NOTE: Commenting this version out because it can't simulate the instruction pointer properly
/*
Sim86State Sim86(VirtualInstStream *pInstStream, bool bPrint = true);
Sim86State Sim86(VirtualInstStream *pInstStream, bool bPrint)
{
    ASSERT(pInstStream && pInstStream->Data && pInstStream->Num > 0);

    Sim86State Result = {};
    Result.Clear();

    for (int InstIdx = 0; InstIdx < pInstStream->Num; InstIdx++)
    {
        SimInst(&Result, &pInstStream->Data[InstIdx]);
    }

    if (bPrint) { PrintState(&Result); }
    return {};
}
*/

