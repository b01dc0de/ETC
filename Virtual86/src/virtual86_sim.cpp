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

void Sim_OpMov(Sim86State* pState, VirtualInst* pInst)
{

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

