#include "virtual86_common.h"

#define UNITY_BUILD() (0)
#if UNITY_BUILD()
#include "virtual86_common.cpp"
#endif // UNITY_BUILD()

enum RegisterType
{
    Reg_Invalid,
    Reg_a,
    Reg_b,
    Reg_c,
    Reg_d,
    Reg_sp,
    Reg_bp,
    Reg_si,
    Reg_di,
};

enum EffAddrType
{
    EffAddr_Invalid,
    EffAddr_bx_si,
    EffAddr_bx_di,
    EffAddr_bp_si,
    EffAddr_bp_di,
    EffAddr_si,
    EffAddr_di,
    EffAddr_bp,
    EffAddr_bx,
    EffAddr_Direct,
};

struct DataDesc
{
    bool bWide;
    union { u8 Data8; u16 Data16; };
};

struct RegisterDesc
{
    RegisterType Type;
    bool bWide;
    bool bHigh;
};

struct EffAddrDesc
{
    EffAddrType Type;
    bool bDisp;
    DataDesc Disp;
};

RegisterDesc GetRegisterDesc(u8 Val, bool bWide)
{
    ASSERT(0 <= Val && Val <= 7);

    // NOTE: RegisterDesc: { RegisterType; bWide; bHigh; }
    RegisterDesc RegisterDescTable[][2] =
    {
        { { Reg_a, false, false }, { Reg_a, true, false } },
        { { Reg_c, false, false }, { Reg_c, true, false } },
        { { Reg_d, false, false }, { Reg_d, true, false } },
        { { Reg_b, false, false }, { Reg_b, true, false } },

        { { Reg_a, false, true }, { Reg_sp, true, false } },
        { { Reg_c, false, true }, { Reg_bp, true, false } },
        { { Reg_d, false, true }, { Reg_si, true, false } },
        { { Reg_b, false, true }, { Reg_di, true, false } },
    };

    RegisterDesc Result = {};
    if (0 <= Val && Val <= 7)
    {
        Result = RegisterDescTable[Val][bWide];
    }
    return Result;
}

EffAddrDesc GetEffAddrDesc(u8 Mode, u8 Val)
{
    ASSERT(0 <= Mode && Mode <= 2);
    ASSERT(0 <= Val && Val <= 7);
    EffAddrDesc Result = {};

    // NOTE: EffAddrDesc { EffAddrType; bool bDisp; Disp; }
    EffAddrDesc EffAddrTable[][3] =
    {
        { { EffAddr_bx_si, 0 }, { EffAddr_bx_si, 1, {0} }, { EffAddr_bx_si, 1, {1} } },
        { { EffAddr_bx_di, 0 }, { EffAddr_bx_di, 1, {0} }, { EffAddr_bx_di, 1, {1} }, },
        { { EffAddr_bp_si, 0 }, { EffAddr_bp_si, 1, {0} }, { EffAddr_bp_si, 1, {1} }, },
        { { EffAddr_bp_di, 0 }, { EffAddr_bp_di, 1, {0} }, { EffAddr_bp_di, 1, {1} }, },
        { { EffAddr_si, 0 }, { EffAddr_si, 1, {0} }, { EffAddr_si, 1, {1} }, },
        { { EffAddr_di, 0 }, { EffAddr_di, 1, {0} }, { EffAddr_di, 1, {1} }, },
        { { EffAddr_Direct, 1, {1} }, { EffAddr_bp, 0, {0} }, { EffAddr_bp, 1, {1} }, },
        { { EffAddr_bx, 1, {1} }, { EffAddr_bx, 0, {0} }, { EffAddr_bx, 1, {1} }, },
    };

    if (0 <= Val && Val <= 7 && 0 <= Mode && Mode <= 2)
    {
        Result = EffAddrTable[Val][Mode];
    }
    return Result;
}

//enum InstFormatFlags : u32 { Flags_None = 0 << 0, };

enum InstArgCaseType : u32
{
    Args_None,
    Args_BothRegMem,
    Args_DstRegMem_SrcImm,
    Args_DstReg_SrcImm,
    Args_DstAcc_SrcImm,
    Args_DstImm_SrcAcc,
};

struct InstEncodeFormat
{
    OpCodeType Type;
    u8 EncodeValue;
    u8 EncodeBitCount;
    //InstFormatFlags Flags;
    InstArgCaseType ArgCase;
    u8 OptEncodeValue;
    u8 OptEncodeBitCount; // always 3?
};

#define INSTFMT_ENCODE(Bits) 0b##Bits, sizeof(#Bits)-1

InstEncodeFormat EncodeFormatTable[] =
{
    { OpCode_Invalid },
    // mov:
    { OpCode_Mov, INSTFMT_ENCODE(100010), /*Flags_None,*/ Args_BothRegMem}, // Register/memory to/from register
    { OpCode_Mov, INSTFMT_ENCODE(1100011), /*Flags_None,*/ Args_DstRegMem_SrcImm }, // Immediate to register/memory
    { OpCode_Mov, INSTFMT_ENCODE(1011), /*Flags_None,*/ Args_DstReg_SrcImm }, // Immediate to register
    { OpCode_Mov, INSTFMT_ENCODE(1010000), /*Flags_None,*/ Args_DstAcc_SrcImm }, // Memory to accumulator
    { OpCode_Mov, INSTFMT_ENCODE(1010001), /*Flags_None,*/ Args_DstImm_SrcAcc }, // Accumulator to memory
    // add:
    { OpCode_Add, INSTFMT_ENCODE(000000), /*Flags_None,*/ Args_BothRegMem }, // Reg/memory with register to either
    { OpCode_Add, INSTFMT_ENCODE(100000), /*Flags_None,*/ Args_DstRegMem_SrcImm, INSTFMT_ENCODE(000) }, // Immediate to register/memory
    { OpCode_Add, INSTFMT_ENCODE(0000010), /*Flags_None,*/ Args_DstAcc_SrcImm }, // Immediate to accumulator
    // sub:
    { OpCode_Sub, INSTFMT_ENCODE(001010), /*Flags_None,*/ Args_BothRegMem }, // Reg/memory and register to either
    { OpCode_Sub, INSTFMT_ENCODE(100000), /*Flags_None,*/ Args_DstRegMem_SrcImm, INSTFMT_ENCODE(101) }, // Immediate from register/memory
    { OpCode_Sub, INSTFMT_ENCODE(0010110), /*Flags_None,*/ Args_DstAcc_SrcImm }, // Immediate from accumulator
    // cmp:
    { OpCode_Cmp, INSTFMT_ENCODE(001110), /*Flags_None,*/ Args_BothRegMem }, // Register/memory and register
    { OpCode_Cmp, INSTFMT_ENCODE(100000), /*Flags_None,*/ Args_DstRegMem_SrcImm, INSTFMT_ENCODE(111) }, // Immediate with register/memory
    { OpCode_Cmp, INSTFMT_ENCODE(0011110), /*Flags_None,*/ Args_DstAcc_SrcImm }, // Immediate with accumulator
};

enum OperandType
{
    OperandType_Invalid,
    OperandType_Reg,
    OperandType_EffAddr,
    OperandType_Imm,
};

struct Operand
{
    OperandType Type;
    union
    {
        RegisterDesc RegDesc;
        EffAddrDesc AddrDesc;
        DataDesc ImmDesc;
    };
};

// NOTE: This only supports Reg and EffAddr operands as currently designed
Operand GetOperand(u8 Mode, u8 Val, bool bWide)
{
    ASSERT(0 <= Mode && Mode <= 3);
    ASSERT(0 <= Val && Val <= 7);
    Operand Result = {};
    switch (Mode)
    {
        case 0b00:
        case 0b01:
        case 0b10:
        {
            Result.Type = OperandType_EffAddr;
            Result.AddrDesc = GetEffAddrDesc(Mode, Val);
        } break;
        case 0b11:
        {
            Result.Type = OperandType_Reg;
            Result.RegDesc = GetRegisterDesc(Val, bWide);
        } break;
    }
    return Result;
}

struct ParsedInst
{
    OpCodeType Code;
    Operand Ops[2];
};

ParsedInst ParseInst(InstEncodeFormat* EncodeFmt, u8* pInst, int* OutNumBytesRead)
{
    ASSERT(EncodeFmt);
    ASSERT(EncodeFmt->Type != OpCode_Invalid);

    ParsedInst Result = {};
    Result.Code = EncodeFmt->Type;
    switch (EncodeFmt->ArgCase)
    {
        case Args_BothRegMem:
        {
            bool bDirection = *pInst & 0b00000010;
            bool bWide = *pInst & 0b00000001;
            u8 Mode = (*(pInst+1) & 0b11000000) >> 6;
            u8 Reg = (*(pInst+1) & 0b00111000) >> 3;
            u8 RM = *(pInst+1) & 0b00000111;

            // Dst == Reg, Src == RM
            int IdxRM = bDirection ? 1 : 0;
            int IdxReg = bDirection ? 0 : 1;

            Result.Ops[IdxRM] = GetOperand(Mode, RM, bWide);
            Result.Ops[IdxReg] = GetOperand(Mode, Reg, bWide);
            if (Result.Ops[IdxRM].Type == OperandType_EffAddr && Result.Ops[IdxRM].AddrDesc.bDisp)
            {
                if (Result.Ops[IdxRM].AddrDesc.Disp.bWide)
                {
                    Result.Ops[IdxRM].AddrDesc.Disp.Data16 = *(u16*)(pInst + 2);
                    *OutNumBytesRead = 4;
                }
                else
                {
                    Result.Ops[IdxRM].AddrDesc.Disp.Data8 = *(pInst + 2);
                    *OutNumBytesRead = 3;
                }
            }
            else { *OutNumBytesRead = 2; }
        } break;
        case Args_DstRegMem_SrcImm:
        {
            bool bWide = *pInst & 0b00000001;
            u8 Mode = (*(pInst+1) & 0b11000000) >> 6;
            u8 RM = *(pInst+1) & 0b00000111;

            bool bDisp = false;
            bool bWideDisp = false;

            Result.Ops[0] = GetOperand(Mode, RM, bWide);
            if (Result.Ops[0].Type == OperandType_EffAddr && Result.Ops[0].AddrDesc.bDisp)
            {
                bDisp = true;
                if (Result.Ops[0].AddrDesc.Disp.bWide)
                {
                    bWideDisp = true;
                    Result.Ops[0].AddrDesc.Disp.Data16 = *(u16*)(pInst + 2);
                }
                else
                {
                    Result.Ops[0].AddrDesc.Disp.Data8 = *(pInst + 2);
                }
            }
            Result.Ops[1].Type = OperandType_Imm;
            int ImmDataOffset = (bDisp && bWideDisp) ? 4 : (bDisp ? 3 : 2);
            if (bWide)
            {
                Result.Ops[1].ImmDesc.bWide = true;
                Result.Ops[1].ImmDesc.Data16 = *(u16*)(pInst + ImmDataOffset);
            }
            else
            {
                Result.Ops[1].ImmDesc.bWide = false;
                Result.Ops[1].ImmDesc.Data16 = *(u16*)(pInst + ImmDataOffset);
            }
            *OutNumBytesRead = ImmDataOffset + (bWide ? 2 : 1);
        } break;
        case Args_DstReg_SrcImm:
        {
            bool bWide = *pInst & 0b00001000;
            u8 Reg = *pInst & 0b00000111;
            Result.Ops[0].Type = OperandType_Reg;
            Result.Ops[0].RegDesc = GetRegisterDesc(Reg, bWide);
            Result.Ops[1].Type = OperandType_Imm;
            if (bWide)
            {
                Result.Ops[1].ImmDesc.bWide = true;
                Result.Ops[1].ImmDesc.Data16 = *(u16*)(pInst + 1);
            }
            else
            {
                Result.Ops[1].ImmDesc.bWide = false;
                Result.Ops[1].ImmDesc.Data16 = *(u16*)(pInst + 1);
            }
            *OutNumBytesRead = bWide ? 3 : 2;
        } break;
        case Args_DstAcc_SrcImm:
        case Args_DstImm_SrcAcc:
        {
            bool bWide = *pInst & 0b00000001;

            int AccIdx = (EncodeFmt->ArgCase == Args_DstAcc_SrcImm) ? 0 : 1;
            int ImmIdx = (EncodeFmt->ArgCase == Args_DstAcc_SrcImm) ? 1 : 0;

            Result.Ops[AccIdx].Type = OperandType_Reg;
            Result.Ops[AccIdx].RegDesc.Type = Reg_a;
            Result.Ops[AccIdx].RegDesc.bWide = true; // TODO: is it always ax ?
            Result.Ops[AccIdx].RegDesc.bHigh = false;

            Result.Ops[ImmIdx].Type = OperandType_Imm;
            if (bWide)
            {
                Result.Ops[ImmIdx].ImmDesc.bWide = true;
                Result.Ops[ImmIdx].ImmDesc.Data16 = *(u16*)(pInst + 1);
            }
            else
            {
                Result.Ops[ImmIdx].ImmDesc.bWide = false;
                Result.Ops[ImmIdx].ImmDesc.Data8 = *(pInst + 1);
            }
            *OutNumBytesRead = bWide ? 3 : 2;
        } break;
    }
    return Result;
}

ParsedInst DecodeInst(u8* pInst, int* OutNumBytesRead)
{
    *OutNumBytesRead = 0;

    InstEncodeFormat* pMatch = nullptr;
    for (int InstFmtIdx = 1; InstFmtIdx < ARRAY_SIZE(EncodeFormatTable); InstFmtIdx++)
    {
        InstEncodeFormat& CurrFmt = EncodeFormatTable[InstFmtIdx];
        if (((*pInst) >> (8-CurrFmt.EncodeBitCount)) == CurrFmt.EncodeValue )
        {
            if (CurrFmt.OptEncodeBitCount == 0)
            {
                pMatch = &CurrFmt; break;
            }
            else if (((*(pInst + 1)) >> 3) == CurrFmt.OptEncodeValue)
            {
                pMatch = &CurrFmt; break;
            }
        }
    }

    ParsedInst Result = {};
    if (pMatch) { Result = ParseInst(pMatch, pInst, OutNumBytesRead); }
    else { DebugBreak(); }
    return Result;
}

const char* GetRegisterName(RegisterDesc* pReg)
{
    const char* RegisterNames[][2] = {
        { "REGNAMEERROR", "REGNAMEERRORWIDE" },
        { "al", "ax" },
        { "cl", "cx" },
        { "dl", "dx" },
        { "bl", "bx" },
        { "ah", "sp" },
        { "ch", "bp" },
        { "dh", "si" },
        { "bh", "di" },
    };
    if (pReg)
    {
        // TODO: Cleanup, this _feels_ messy
        return RegisterNames[pReg->Type + (pReg->bHigh ? 4 : 0)][pReg->bWide];
    }
    else { return nullptr; }
}

void PrintOperand(Operand* pOperand)
{
    const char* RegisterNames[][2] = {
        { "REGNAMEERROR", "REGNAMEERRORWIDE" },
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
        { "EFFADDRNAMEERROR", "EFFADDRNAMEERRORDISP" },
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
            sprintf_s(OperandBuffer, "%s", RegisterNames[pOperand->RegDesc.Type + (pOperand->RegDesc.bHigh ? 4 : 0)][pOperand->RegDesc.bWide]);
        } break;
        case OperandType_EffAddr:
        {
            if (pOperand->AddrDesc.bDisp)
            {
                sprintf_s(OperandBuffer, EffAddrNameFmtTable[pOperand->AddrDesc.Type][1],
                    pOperand->AddrDesc.Disp.bWide ? pOperand->AddrDesc.Disp.Data16 : pOperand->AddrDesc.Disp.Data8);
            }
            else
            {
                sprintf_s(OperandBuffer, EffAddrNameFmtTable[pOperand->AddrDesc.Type][0]);
            }
        } break;
        case OperandType_Imm:
        {
            if (pOperand->ImmDesc.bWide)
            {
                short sData16 = pOperand->ImmDesc.Data16;
                sprintf_s(OperandBuffer, "%d", sData16);
            }
            else
            {
                char sData8 = pOperand->ImmDesc.Data16;
                sprintf_s(OperandBuffer, "%d", sData8);
            }
        } break;
    }
    printf("%s", OperandBuffer);
}

void PrintInst(ParsedInst* Inst)
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

void DecodeFile86(const char* FileName)
{
    FileContentsT FileContents = ReadFileContents(FileName);
    if (FileContents.Data == nullptr) { DebugBreak(); return; }

    ParsedInst InstStream[1000];

    int InstReadIdx = 0;
    int InstWriteIdx = 0;
    while (InstReadIdx < FileContents.Size)
    {
        int NumBytesRead = 0;
        InstStream[InstWriteIdx] = DecodeInst(FileContents.Data + InstReadIdx, &NumBytesRead);
        PrintInst(&InstStream[InstWriteIdx]);

        InstReadIdx += NumBytesRead;
        InstWriteIdx++;
    }
}

int main(int ArgCount, const char* ArgValues[])
{
    /*
    for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
    {
        printf("\tArgValues[%d]: %s\n", ArgIdx, ArgValues[ArgIdx]);
    }
    */
    DecodeFile86("input/listing_0037_single_register_mov");
    //DecodeFile86("input/listing_0038_many_register_mov");
    //DecodeFile86("input/listing_0039_more_movs");
    //DecodeFile86("input/listing_0040_challenge_movs");
    //DecodeFile86("input/listing_0041_add_sub_cmp_jnz");

    return 0;
}