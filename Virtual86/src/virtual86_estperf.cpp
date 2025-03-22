#include "virtual86_estperf.h"
#include "virtual86_decode.h"
#include "virtual86_print.h"

int EstEACalcCycles(Operand* pOp)
{
    ASSERT(pOp && pOp->Type == OperandType_EffAddr);
    int Result = 0;
    auto H_DispIsNonZero = [](EffAddrDesc& Desc) -> bool
    {
        if (Desc.bDisp)
        {
            if (Desc.Disp.bWide) { return Desc.Disp.Data16 != 0; }
            else { return Desc.Disp.Data8 != 0; }
        }
        return false;
    };
    switch (pOp->AddrDesc.Type)
    {
        case EffAddr_bp_di:
        case EffAddr_bx_si:
        {
            if (pOp->AddrDesc.bDisp && H_DispIsNonZero(pOp->AddrDesc)) { Result = 11; }
            else { Result = 7; }
        } break;
        case EffAddr_bp_si:
        case EffAddr_bx_di:
        {
            if (pOp->AddrDesc.bDisp && H_DispIsNonZero(pOp->AddrDesc)) { Result = 12; }
            else { Result = 8; }
        } break;
        case EffAddr_si:
        case EffAddr_di:
        case EffAddr_bp:
        case EffAddr_bx:
        {
            if (pOp->AddrDesc.bDisp && H_DispIsNonZero(pOp->AddrDesc)) { Result = 9; }
            else { Result = 5; }
        } break;
        case EffAddr_Direct: { Result = 6; } break;
        default: { DebugBreak(); } break;
    }
    return Result;
}

int EstInstCycles(VirtualInst* pInst)
{
    ASSERT(pInst);
    int Result = 0;

    // Helpers for common op type checks
    auto H_OpIsReg = [](Operand& Op) -> bool { return Op.Type == OperandType_Reg; };
    auto H_OpIsAcc = [](Operand& Op) -> bool { return Op.Type == OperandType_Reg && Op.RegDesc.Type == Reg_a; };
    auto H_OpIsEA = [](Operand& Op) -> bool { return Op.Type == OperandType_EffAddr; };
    auto H_OpIsDirMem = [](Operand& Op) -> bool { return Op.Type == OperandType_EffAddr && Op.AddrDesc.Type == EffAddr_Direct; };
    auto H_OpIsImm = [](Operand& Op) -> bool { return Op.Type == OperandType_Imm; };

    Operand& Dst = pInst->Ops[0];
    Operand& Src = pInst->Ops[1];
    // NOTE: int EstEACalcCycles(Operand * pOp)
    switch (pInst->Code)
    {
        case OpCode_Mov:
        {
            // Memory, accumulator => 10
            // TODO: These 'DirMem's might just have to also handle EA in general(?)
            if (H_OpIsDirMem(Dst) && H_OpIsAcc(Src)) { Result = 10; }
            // Accumulator, memory => 10
            else if (H_OpIsAcc(Dst) && H_OpIsDirMem(Src)) { Result = 10; }
            // Register, register => 2
            else if (H_OpIsReg(Dst) && H_OpIsReg(Src)) { Result = 2; }
            // Register, memory => 8 + EA
            else if (H_OpIsReg(Dst) && H_OpIsEA(Src)) { Result = 8 + EstEACalcCycles(&Src); }
            // Memory, register => 9 + EA
            else if (H_OpIsEA(Dst) && H_OpIsReg(Src)) { Result = 9 + EstEACalcCycles(&Dst); }
            // Register, immediate => 4
            else if (H_OpIsReg(Dst) && H_OpIsImm(Src)) { Result = 4; }
            // Memory, immediate => 10 + EA
            else if (H_OpIsEA(Dst) && H_OpIsImm(Src)) { Result = 10 + EstEACalcCycles(&Dst); }
        } break;
        case OpCode_Add:
        case OpCode_Sub:
        case OpCode_Cmp:
        {
            // Register, register => 3
            if (H_OpIsReg(Dst) && H_OpIsReg(Src)) { Result = 3; }
            // Register, memory => 9 + EA
            else if (H_OpIsReg(Dst) && H_OpIsEA(Src)) { Result = 9 + EstEACalcCycles(&Src); }
            // Add/Sub: Memory, register => 16 + EA
            // Cmp: Memory, register => 9 + EA
            else if (H_OpIsEA(Dst) && H_OpIsReg(Src))
            {
                if (pInst->Code == OpCode_Cmp) { Result = 9 + EstEACalcCycles(&Dst); }
                else { Result = 16 + EstEACalcCycles(&Dst); }
            }
            // Register, immediate => 4
            else if (H_OpIsReg(Dst) && H_OpIsImm(Src)) { Result = 4; }
            // Add/Sub: Memory, immediate => 17 + EA
            // Cmp:Memory, immediate => 10 + EA
            else if (H_OpIsEA(Dst) && H_OpIsImm(Src))
            {
                if (pInst->Code == OpCode_Cmp) { Result = 10 + EstEACalcCycles(&Dst); }
                else { Result = 17 + EstEACalcCycles(&Dst); }
            }
            // Accumulator, immediate => 4
            else if (H_OpIsAcc(Dst) && H_OpIsImm(Src)) { Result = 4; }
        } break;
        default: { DebugBreak(); } break;
    }
    ASSERT(Result != 0);
    return Result;
}

void Est86Cycles(const char* FileName)
{
    FileContentsT FileContents = ReadFileContents(FileName);
    if (FileContents.Data == nullptr) { DebugBreak(); return; }

    printf("; EstCycles - %s :\n", FileName);

    size_t FakeIP = 0;
    int EstTotalCycles = 0;

    while (FakeIP < FileContents.Size)
    {
        VirtualInst Inst = DecodeInst(FileContents.Data + FakeIP);
        ASSERT(Inst.Code != OpCode_Invalid);
        int InstCycles = EstInstCycles(&Inst);
        EstTotalCycles += InstCycles;
        PrintInst(&Inst);
        printf("\tEstCycles: %d -- Total: %d\n", InstCycles, EstTotalCycles);
        FakeIP += Inst.ByteWidth;
    }
}
