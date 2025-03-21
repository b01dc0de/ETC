#include "virtual86_sim.h"
#include "virtual86_print.h"

Sim86State ZeroState()
{
    Sim86State Result = {};
    for (int RegIdx = 0; RegIdx < ARRAY_SIZE(Result.Registers); RegIdx++)
    {
        Result.Registers[RegIdx] = 0;
    }
    return Result;
}

struct OpUnit
{
    u8* Ptr;
    bool bWide;
};

OpUnit GetOpUnit(Sim86State* pState, Operand* Op)
{
    ASSERT(Op);
    ASSERT(pState);

    OpUnit Result = {};
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

void Sim_OpMov(Sim86State* pState, VirtualInst* pInst)
{
    OpUnit Dst = GetOpUnit(pState, &pInst->Ops[0]);
    OpUnit Src = GetOpUnit(pState, &pInst->Ops[1]);
    ASSERT(Dst.bWide == Src.bWide);
    if (Dst.bWide) { *(u16*)Dst.Ptr = *(u16*)Src.Ptr; }
    else { *Dst.Ptr = *Src.Ptr; }
}

void SimInst(Sim86State* pState, VirtualInst* pInst)
{
    ASSERT(pState);
    ASSERT(pInst);
    switch (pInst->Code)
    {
        case OpCode_Mov:
        {
            Sim_OpMov(pState, pInst);
        } break;
        default:
        {
            // UNHANDLED INST
            DebugBreak();
        } break;
    }
}

Sim86State Sim86(VirtualInstStream *pInstStream, bool bPrint)
{
    ASSERT(pInstStream && pInstStream->Data && pInstStream->Num > 0);

    Sim86State Result = ZeroState();

    for (int InstIdx = 0; InstIdx < pInstStream->Num; InstIdx++)
    {
        SimInst(&Result, &pInstStream->Data[InstIdx]);
    }

    if (bPrint) { PrintState(&Result); }
    return {};
}

