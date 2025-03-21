#include "virtual86_print.h"

void PrintOperand(Operand* pOperand)
{
    const char* RegisterNames[][2] = {
        { "al", "ax" },
        { "cl", "cx" },
        { "dl", "dx" },
        { "bl", "bx" },
        { "ah", "sp" },
        { "ch", "bp" },
        { "dh", "si" },
        { "bh", "di" },
    };
    const char* EffAddrNameFmtTable[][2] =
    {
        { "[bx + si]", "[bx + si + %d]" },
        { "[bx + di]", "[bx + di + %d]" },
        { "[bp + si]", "[bp + si + %d]" },
        { "[bp + di]", "[bp + di + %d]" },
        { "[si]", "[si + %d]" },
        { "[di]", "[di + %d]" },
        { "[bp]", "[bp + %d]" },
        { "[bx]", "[bx + %d]" },
        { "[%d]", "[%d]" },
    };

    ASSERT(pOperand);
    char OperandBuffer[32];
    OperandBuffer[0] = '\0';
    if (!pOperand) { return; }
    switch (pOperand->Type)
    {
        case OperandType_Invalid: { DebugBreak(); } break;
        case OperandType_Reg:
        {
            ASSERT(pOperand->RegDesc.Type != Reg_Invalid);
            // TODO: Cleanup, this _feels_ messy
            int RegIdx = pOperand->RegDesc.Type + (pOperand->RegDesc.bHigh ? 4 : 0) - 1;
            sprintf_s(OperandBuffer, "%s", RegisterNames[RegIdx][pOperand->RegDesc.bWide]);
        } break;
        case OperandType_EffAddr:
        {
            ASSERT(pOperand->AddrDesc.Type != EffAddr_Invalid);
            int EffAddrIdx = pOperand->AddrDesc.Type - 1;
            if (pOperand->AddrDesc.bDisp && pOperand->AddrDesc.Disp.bWide)
            {
                short sData16 = pOperand->AddrDesc.Disp.Data16;
                sprintf_s(OperandBuffer, EffAddrNameFmtTable[EffAddrIdx][1], sData16);

            }
            else if (pOperand->AddrDesc.bDisp && !pOperand->AddrDesc.Disp.bWide)
            {
                char sData8 = pOperand->AddrDesc.Disp.Data8;
                sprintf_s(OperandBuffer, EffAddrNameFmtTable[EffAddrIdx][1], sData8);
            }
            else
            {
                sprintf_s(OperandBuffer, EffAddrNameFmtTable[EffAddrIdx][0]);
            }
        } break;
        case OperandType_Imm:
        {
            if (pOperand->ImmDesc.bWide)
            {
                short sData16 = pOperand->ImmDesc.Data16;
                sprintf_s(OperandBuffer, "word %d", sData16);
            }
            else
            {
                char sData8 = pOperand->ImmDesc.Data8;
                sprintf_s(OperandBuffer, "byte %d", sData8);
            }
        } break;
        case OperandType_RelOffset:
        {
            char sData8 = pOperand->ImmDesc.Data8;
            sprintf_s(OperandBuffer, "$%+d", sData8);
        } break;
    }
    printf("%s", OperandBuffer);
}

void PrintInst(VirtualInst* Inst)
{
    ASSERT(Inst);
    ASSERT(Inst->Code != OpCode_Invalid);

    printf("%s ", OpCodeMnemonicTable[Inst->Code]);
    if (Inst->Ops[0].Type != OperandType_Invalid)
    {
        PrintOperand(&Inst->Ops[0]);
    }
    if (Inst->Ops[1].Type != OperandType_Invalid)
    {
        printf(", ");
        PrintOperand(&Inst->Ops[1]);
    }
    printf("\n");
}

void PrintInstStream(VirtualInstStream* pInstStream)
{
    ASSERT(pInstStream);
    if (pInstStream)
    {
        for (int InstIdx = 0; InstIdx < pInstStream->Num; InstIdx++)
        {
            PrintInst(&pInstStream->Data[InstIdx]);
        }
    }
    printf("\n");
}

void PrintState(Sim86State* pSimState)
{
    ASSERT(pSimState);
    if (pSimState)
    {
        u16 ax = pSimState->Registers[Reg_a-1];
        u16 bx = pSimState->Registers[Reg_b-1];
        u16 cx = pSimState->Registers[Reg_c-1];
        u16 dx = pSimState->Registers[Reg_d-1];
        u16 sp = pSimState->Registers[Reg_sp-1];
        u16 bp = pSimState->Registers[Reg_bp-1];
        u16 si = pSimState->Registers[Reg_si-1];
        u16 di = pSimState->Registers[Reg_di-1];
        
        u16 ip = pSimState->IP;

        printf("RESULT:\n");
        printf("\tax: 0x%04x  (%d)\n", ax, ax);
        printf("\tbx: 0x%04x  (%d)\n", bx, bx);
        printf("\tcx: 0x%04x  (%d)\n", cx, cx);
        printf("\tdx: 0x%04x  (%d)\n", dx, dx);

        printf("\tsp: 0x%04x  (%d)\n", sp, sp);
        printf("\tbp: 0x%04x  (%d)\n", bp, bp);
        printf("\tsi: 0x%04x  (%d)\n", si, si);
        printf("\tdi: 0x%04x  (%d)\n", di, di);

        printf("\n\tIP: 0x%04x  (%d)\n", ip, ip);

        printf("\n\tFlags:\n");
        printf("\tSign: %c\n", pSimState->bFlagSign ? '1' : '0');
        printf("\tZero: %c\n", pSimState->bFlagZero ? '1' : '0');
    }
}

