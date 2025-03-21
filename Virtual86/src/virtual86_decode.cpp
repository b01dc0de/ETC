#include "virtual86_decode.h"
#include "virtual86_print.h"

enum InstFormatFlags : u32
{
    Flags_None = 0 << 0,
    Flags_BitS = 1 << 1,
};

enum InstArgCaseType : u32
{
    Args_None,
    Args_BothRegMem,
    Args_DstRegMem_SrcImm,
    Args_DstReg_SrcImm,
    Args_DstAcc_SrcImm,
    Args_DstImm_SrcAcc,
    Args_JmpOffset,
};

struct InstEncodeFormat
{
    OpCodeType Type;
    u8 EncodeValue;
    u8 EncodeBitCount;
    InstFormatFlags Flags;
    InstArgCaseType ArgCase;
    u8 OptEncodeValue;
    u8 OptEncodeBitCount; // always 3?
};

#define INSTFMT_ENCODE(Bits) 0b##Bits, sizeof(#Bits)-1

InstEncodeFormat EncodeFormatTable[] =
{
    { OpCode_Invalid },
    // mov:
    { OpCode_Mov, INSTFMT_ENCODE(100010), Flags_None, Args_BothRegMem}, // Register/memory to/from register
    { OpCode_Mov, INSTFMT_ENCODE(1100011), Flags_None, Args_DstRegMem_SrcImm }, // Immediate to register/memory
    { OpCode_Mov, INSTFMT_ENCODE(1011), Flags_None, Args_DstReg_SrcImm }, // Immediate to register
    { OpCode_Mov, INSTFMT_ENCODE(1010000), Flags_None, Args_DstAcc_SrcImm }, // Memory to accumulator
    { OpCode_Mov, INSTFMT_ENCODE(1010001), Flags_None, Args_DstImm_SrcAcc }, // Accumulator to memory
    // add:
    { OpCode_Add, INSTFMT_ENCODE(000000), Flags_None, Args_BothRegMem }, // Reg/memory with register to either
    { OpCode_Add, INSTFMT_ENCODE(100000), Flags_BitS, Args_DstRegMem_SrcImm, INSTFMT_ENCODE(000) }, // Immediate to register/memory
    { OpCode_Add, INSTFMT_ENCODE(0000010), Flags_None, Args_DstAcc_SrcImm }, // Immediate to accumulator
    // sub:
    { OpCode_Sub, INSTFMT_ENCODE(001010), Flags_None, Args_BothRegMem }, // Reg/memory and register to either
    { OpCode_Sub, INSTFMT_ENCODE(100000), Flags_BitS, Args_DstRegMem_SrcImm, INSTFMT_ENCODE(101) }, // Immediate from register/memory
    { OpCode_Sub, INSTFMT_ENCODE(0010110), Flags_None, Args_DstAcc_SrcImm }, // Immediate from accumulator
    // cmp:
    { OpCode_Cmp, INSTFMT_ENCODE(001110), Flags_None, Args_BothRegMem }, // Register/memory and register
    { OpCode_Cmp, INSTFMT_ENCODE(100000), Flags_BitS, Args_DstRegMem_SrcImm, INSTFMT_ENCODE(111) }, // Immediate with register/memory
    { OpCode_Cmp, INSTFMT_ENCODE(0011110), Flags_None, Args_DstAcc_SrcImm }, // Immediate with accumulator
    // conditional jumps:
    { OpCode_JeJz, INSTFMT_ENCODE(01110100), Flags_None, Args_JmpOffset }, // Jump on equal/zero
    { OpCode_JlJnge, INSTFMT_ENCODE(01111100), Flags_None, Args_JmpOffset }, // Jump on less/not greater or equal
    { OpCode_JleJng, INSTFMT_ENCODE(01111110), Flags_None, Args_JmpOffset }, // Jump on less or equal/not greater
    { OpCode_JbJnae, INSTFMT_ENCODE(01110010), Flags_None, Args_JmpOffset }, // Jump on below/not above or equal
    { OpCode_JbeJna, INSTFMT_ENCODE(01110110), Flags_None, Args_JmpOffset }, // Jump on below or equal/not above
    { OpCode_JpJpe, INSTFMT_ENCODE(01111010), Flags_None, Args_JmpOffset }, // Jump on parity/parity even
    { OpCode_Jo, INSTFMT_ENCODE(01110000), Flags_None, Args_JmpOffset }, // Jump on overflow
    { OpCode_Js, INSTFMT_ENCODE(01111000), Flags_None, Args_JmpOffset }, // Jump on sign
    { OpCode_JneJnz, INSTFMT_ENCODE(01110101), Flags_None, Args_JmpOffset }, // Jump on not equal/not zero
    { OpCode_JnlJge, INSTFMT_ENCODE(01111101), Flags_None, Args_JmpOffset }, // Jump on not less/greater or equal
    { OpCode_JnleJg, INSTFMT_ENCODE(01111111), Flags_None, Args_JmpOffset }, // Jump on not less or equal/greater
    { OpCode_JnbJae, INSTFMT_ENCODE(01110011), Flags_None, Args_JmpOffset }, // Jump on not below/above or equal
    { OpCode_JnbeJa, INSTFMT_ENCODE(01110111), Flags_None, Args_JmpOffset }, // Jump on not below or equal/above
    { OpCode_JnpJpo, INSTFMT_ENCODE(01111011), Flags_None, Args_JmpOffset }, // Jump on not par/par odd
    { OpCode_Jno, INSTFMT_ENCODE(01110001), Flags_None, Args_JmpOffset }, // Jump on not overflow
    { OpCode_Jns, INSTFMT_ENCODE(01111001), Flags_None, Args_JmpOffset }, // Jump on not sign
    { OpCode_Loop, INSTFMT_ENCODE(11100010), Flags_None, Args_JmpOffset }, // Loop CX times
    { OpCode_LoopzLoope, INSTFMT_ENCODE(11100001), Flags_None, Args_JmpOffset }, // Loop while zero/equal
    { OpCode_LoopnzLoopne, INSTFMT_ENCODE(11100000), Flags_None, Args_JmpOffset }, // Loop while not zero/equal
    { OpCode_Jcxz, INSTFMT_ENCODE(11100011), Flags_None, Args_JmpOffset }, // Jump on CX zero
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
        { { EffAddr_Direct, 1, {1} }, { EffAddr_bp, 1, {0} }, { EffAddr_bp, 1, {1} }, },
        { { EffAddr_bx, 0 }, { EffAddr_bx, 1, {0} }, { EffAddr_bx, 1, {1} }, },
    };

    if (0 <= Val && Val <= 7 && 0 <= Mode && Mode <= 2)
    {
        Result = EffAddrTable[Val][Mode];
    }
    return Result;
}

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

VirtualInst ParseInst(InstEncodeFormat* EncodeFmt, u8* pInst)
{
    ASSERT(EncodeFmt);
    ASSERT(EncodeFmt->Type != OpCode_Invalid);

    VirtualInst Result = {};
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
            Result.Ops[IdxReg] = GetOperand(0b11, Reg, bWide);
            if (Result.Ops[IdxRM].Type == OperandType_EffAddr && Result.Ops[IdxRM].AddrDesc.bDisp)
            {
                if (Result.Ops[IdxRM].AddrDesc.Disp.bWide)
                {
                    Result.Ops[IdxRM].AddrDesc.Disp.Data16 = *(u16*)(pInst + 2);
                    Result.EncodedByteWidth = 4;
                }
                else
                {
                    Result.Ops[IdxRM].AddrDesc.Disp.Data8 = *(pInst + 2);
                    Result.EncodedByteWidth = 3;
                }
            }
            else { Result.EncodedByteWidth = 2; }
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
            bWide = bWide && (!(EncodeFmt->Flags & Flags_BitS) || !(*pInst & 0b00000010));
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
            Result.EncodedByteWidth = ImmDataOffset + (bWide ? 2 : 1);
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
            Result.EncodedByteWidth = bWide ? 3 : 2;
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
            Result.EncodedByteWidth = bWide ? 3 : 2;
        } break;
        case Args_JmpOffset:
        {
            Result.Ops[0].Type = OperandType_RelOffset;
            Result.Ops[0].ImmDesc.bWide = false;
            Result.Ops[0].ImmDesc.Data8 = *(pInst + 1) + 2; // TODO: Why does this 2 make the values correct?
            // NOTE: type of data according to manual is IP-INC8
            Result.EncodedByteWidth = 2;
        } break;
    }
    return Result;
}

VirtualInst DecodeInst(u8* pInst)
{
    InstEncodeFormat* pMatch = nullptr;
    for (int InstFmtIdx = 1; InstFmtIdx < ARRAY_SIZE(EncodeFormatTable); InstFmtIdx++)
    {
        InstEncodeFormat& CurrFmt = EncodeFormatTable[InstFmtIdx];
        u8 ShiftedEncodeValue = (*pInst) >> (8 - CurrFmt.EncodeBitCount);
        if (ShiftedEncodeValue == CurrFmt.EncodeValue )
        {
            if (CurrFmt.OptEncodeBitCount == 0)
            {
                pMatch = &CurrFmt; break;
            }
            else 
            {
                u8 ShiftedOptEncodeValue = (*(pInst + 1)&0b00111000) >> 3;
                if (ShiftedOptEncodeValue == CurrFmt.OptEncodeValue)
                {
                    pMatch = &CurrFmt; break;
                }
            }
        }
    }

    VirtualInst Result = {};
    if (pMatch) { Result = ParseInst(pMatch, pInst); }
    else { DebugBreak(); }
    return Result;
}

VirtualInstStream DecodeFile86(const char* FileName, bool bPrint)
{
    constexpr int DefaultCapacity = 1024;
    VirtualInstStream Result = { new VirtualInst[DefaultCapacity], 0, DefaultCapacity };

    FileContentsT FileContents = ReadFileContents(FileName);
    if (FileContents.Data == nullptr) { DebugBreak(); return Result; }


    int InstReadIdx = 0;
    while (InstReadIdx < FileContents.Size)
    {
        Result.Data[Result.Num] = DecodeInst(FileContents.Data + InstReadIdx);

        InstReadIdx += Result.Data[Result.Num].EncodedByteWidth;
        Result.Num++;
    }

    if (bPrint)
    {
        printf("; %s:\n", FileName);
        PrintInstStream(&Result);
    }

    return Result;
}

