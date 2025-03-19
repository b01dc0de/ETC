#include "decode86_common.h"
#define UNITY_BUILD() (0)
#if UNITY_BUILD()
#include "decode86_common.cpp"
#endif // UNITY_BUILD

enum RegisterType : u8
{
    Reg_Invalid,
    Reg_a,
    Reg_c,
    Reg_d,
    Reg_b,
    Reg_sp,
    Reg_bp,
    Reg_si,
    Reg_di,
    Reg_Count
};

RegisterType GetReg(u8 Val)
{
    RegisterType Result = Reg_Invalid;
    if (0 <= Val && Val < 7)
    {
        Result = (RegisterType)(Val + 1);
    }
    return Result;
}

struct RegisterDesc
{
    RegisterType Type;
    bool bWide;
    bool bHigh;
};

enum EffAddrType : u8
{
    EffAddr_Invalid,
    EffAddr_bx_si,
    EffAddr_bx_di,
    EffAddr_bp_si,
    EffAddr_si,
    EffAddr_di,
    EffAddr_bp,
    EffAddr_bx,
    EffAddr_Count
};

EffAddrType GetEffAddr(u8 Val)
{
    EffAddrType Result = EffAddr_Invalid;
    if (0 <= Val && Val < 7)
    {
        Result = (EffAddrType)(Val + 1);
    }
    return Result;
}

struct DataDesc
{
    bool bPresent;
    bool bWide;
    union
    {
        u8 Data8;
        u16 Data16;
    };
};

struct EffAddrDesc
{
    EffAddrType Type;
    DataDesc Disp;
};

enum OperandType
{
    Operand_Invalid,
    Operand_Reg,
    Operand_EffAddr,
    Operand_EffAddrDisp,
    Operand_Imm,
    OperandType_Count
};

struct OperandDesc
{
    OperandType Type;
    union
    {
        RegisterDesc Reg;
        EffAddrDesc EffAddr;
        DataDesc Imm;
    };
};

enum OpCodeType : u8
{
    OpCode_Invalid,
    // Data Transfer:
    OpCode_Mov,
    OpCode_Push,
    OpCode_Pop,
    OpCode_Xchg,
    OpCode_In,
    OpCode_Out,
    OpCode_Xlat,
    OpCode_Lea,
    OpCode_Lds,
    OpCode_Les,
    OpCode_Lahf,
    OpCode_Sahf,
    OpCode_Pushf,
    OpCode_Popf,
    // Arithmetic:
    OpCode_Add,
    OpCode_Adc,
    OpCode_Inc,
    OpCode_Aaa,
    OpCode_Daa,
    OpCode_Sub,
    OpCode_Sbb,
    OpCode_Dec,
    OpCode_Neg,
    OpCode_Cmp,
    OpCode_Aas,
    OpCode_Das,
    OpCode_Mul,
    OpCode_Imul,
    OpCode_Aam,
    OpCode_Div,
    OpCode_Idiv,
    OpCode_Aad,
    OpCode_Cbw,
    OpCode_Cwd,
    // Logic:
    OpCode_Not,
    OpCode_ShlSal,
    OpCode_Shr,
    OpCode_Sar,
    OpCode_Rol,
    OpCode_Ror,
    OpCode_Rcl,
    OpCode_Rcr,
    OpCode_And,
    OpCode_Test,
    OpCode_Or,
    OpCode_Xor,
    // String Manipulation:
    OpCode_Rep,
    OpCode_Movs,
    OpCode_Cmps,
    OpCode_Scas,
    OpCode_Lods,
    OpCode_Stds,
    // Control Transfer:
    OpCode_Call,
    OpCode_Jmp,
    OpCode_Ret,
    OpCode_JeJz,
    OpCode_JlJnge,
    OpCode_JleJng,
    OpCode_JbJnae,
    OpCode_JbeJna,
    OpCode_JbJpe,
    OpCode_Jo,
    OpCode_Js,
    OpCode_JneJnz,
    OpCode_JnlJge,
    OpCode_JnleJg,
    OpCode_JnbJae,
    OpCode_JnbeJa,
    OpCode_JnpJpo,
    OpCode_Jno,
    OpCode_Jns,
    OpCode_Loop,
    OpCode_LoopzLoope,
    OpCode_LoopnzLoopne,
    OpCode_Jcxz,
    OpCode_Int,
    OpCode_Into,
    OpCode_Iret,
    // Processor Control:
    OpCode_Clc,
    OpCode_Cmc,
    OpCode_Stc,
    OpCode_Cld,
    OpCode_Std,
    OpCode_Cli,
    OpCode_Sti,
    OpCode_Hlt,
    OpCode_Wait,
    OpCode_Esc,
    OpCode_Lock,
    OpCode_Segment,
    OpCode_Count
};

enum InstDescFlags
{
    InstF_None = 0,
    InstF_BitD = 1 << 0,
    InstF_BitW = 1 << 1,
    InstF_BitS = 1 << 2,
    InstF_Mode = 1 << 3,
    InstF_Reg = 1 << 4,
    InstF_CompareReg = 1 << 5,
    InstF_RM = 1 << 6,
    InstF_Disp = 1 << 7,
    InstF_Data = 1 << 8,
    InstF_DstAcc = 1 << 9,
    InstF_SrcAcc = 1 << 10
};

struct InstructionFormatDesc
{
    OpCodeType OpType;
    u8 EncodeValue;
    u8 EncodeBitCount;
    int Flags;

    u8 OptRegEncode;
};

InstructionFormatDesc InstDescTable[] =
{
    // Invalid parse
    { OpCode_Invalid, 0b0, 0, InstF_None },

    // mov - Register/memory to/from register
    { OpCode_Mov, 0b100010, 6, InstF_BitD|InstF_BitW|InstF_Mode|InstF_Reg|InstF_RM|InstF_Disp },
    // mov - Immediate to register/memory
    { OpCode_Mov, 0b1100011, 7, InstF_BitW|InstF_Mode|InstF_RM|InstF_Disp|InstF_Data },
    // mov - Immediate to register
    { OpCode_Mov, 0b1011, 4, InstF_BitW|InstF_Reg|InstF_Data },
    // mov - Memory to accumulator
    { OpCode_Mov, 0b1010000, 7, InstF_Data|InstF_DstAcc },
    // mov - Accumulator to memory
    { OpCode_Mov, 0b1010001, 7, InstF_Data|InstF_SrcAcc },

    // add - Reg/memory with register to either
    { OpCode_Add, 0b000000, 6, InstF_BitD|InstF_BitW|InstF_Mode|InstF_Reg|InstF_RM|InstF_Disp },
    // add - Immediate to register/memory
    { OpCode_Add, 0b100000, 6, InstF_BitS|InstF_BitW|InstF_Mode|InstF_CompareReg|InstF_Disp|InstF_Data, 0b000 },
    // add - Immediate to accumulator
    { OpCode_Add, 0b0000010, 7, InstF_BitW|InstF_Data|InstF_DstAcc },

    // sub - Reg/memory and register to either
    { OpCode_Sub, 0b001010, 6, InstF_BitD|InstF_BitW|InstF_Mode|InstF_Reg|InstF_RM|InstF_Disp },
    // sub - Immediate from register/memory
    { OpCode_Sub, 0b100000, 6, InstF_BitS|InstF_BitW|InstF_Mode|InstF_CompareReg|InstF_RM|InstF_Disp|InstF_Data },
    // sub - Immediate from accumulator
    { OpCode_Sub, 0b0010110, 7, InstF_BitW|InstF_Data|InstF_DstAcc },

    // cmp - Register/memory and register
    { OpCode_Cmp, 0b001110, 6, InstF_BitD|InstF_BitW|InstF_Mode|InstF_Reg|InstF_RM|InstF_Disp },
    // cmp - Immediate with register/memory
    { OpCode_Cmp, 0b100000, 6, InstF_BitS|InstF_BitW|InstF_Mode|InstF_CompareReg|InstF_RM|InstF_Disp|InstF_Data, 0x111 },
    // cmp - Immediate with accumulator
    { OpCode_Cmp, 0b0011110, 7, InstF_BitW|InstF_Data|InstF_DstAcc },

    // TODO: Jumps
};

InstructionFormatDesc FetchInstFmtDesc(u8 Byte0, u8 Byte1)
{
    InstructionFormatDesc Result = InstDescTable[0];
    bool bFound = false;
    for (int InstDescIdx = 1; InstDescIdx < ARRAY_SIZE(InstDescTable); InstDescIdx++)
    {
        InstructionFormatDesc& CurrInstDesc = InstDescTable[InstDescIdx];

        u8 CurrEncodeValue = CurrInstDesc.EncodeValue;
        u8 CurrEncodeBitCount = CurrInstDesc.EncodeBitCount;
        u8 CurrEncodeShift = 8 - CurrEncodeBitCount;

        if ((Byte0 >> CurrEncodeShift) == CurrInstDesc.EncodeValue)
        {
            if (CurrInstDesc.Flags & InstF_CompareReg)
            {
                constexpr u8 CompareRegMask = 0b00111000;
                constexpr u8 CompareRegShift = 3;
                bFound = ((Byte1 & CompareRegMask) >> CompareRegShift) == CurrInstDesc.OptRegEncode;
            }
            else
            {
                bFound = true;
            }

            if (bFound) { Result = CurrInstDesc; break; }
        }
    }
    return Result;
}

struct DecodedInst
{
    OpCodeType Op;
    u8 ByteWidth;
    OperandDesc Operands[2];
};

struct DecodeStateT
{
    bool bFlagDirection;
    bool bFlagWide;
    bool bFlagS;

    bool bMode;
    int ModeValue;

    bool bReg;
    int RegValue;

    bool bRM;
    int RMValue;

    bool bDisp;
    bool bDispWide;
    u16 DispValue;

    bool bData;
    bool bDataWide;
    u16 DataValue;
};

DecodeStateT InitDecodeState()
{
    DecodeStateT Result;
    {
        Result.bFlagDirection = false;
        Result.bFlagWide = false;
        Result.bFlagS = false;
        Result.bMode = false;
        Result.ModeValue = 0;
        Result.bReg = false;
        Result.RegValue = 0;
        Result.bRM = false;
        Result.RMValue = 0;
        Result.bDisp = false;
        Result.bDispWide = false;
        Result.DispValue = 0;
        Result.bData = false;
        Result.bDataWide = false;
        Result.DataValue = 0;
    }
    return Result;
}

OperandDesc GetOperandDesc(DecodeStateT* pDecodeState, bool bSrc)
{
    OperandDesc Result = {};
    if (pDecodeState->bDisp)
    {
        Result.Type = Operand_EffAddrDisp;
    }
    else if (pDecodeState->bData)
    {
        Result.Type = Operand_Imm;
    }
    else if (pDecodeState->bMode)
    {
        switch (pDecodeState->ModeValue)
        {
            case 0: Result.Type = Operand_EffAddr; break;
            case 3: Result.Type = Operand_Reg; break;
            case 1: case 2: default: DebugBreak(); break;
        }
    }
    switch (Result.Type)
    {
        case Operand_Reg:
        {
            Result.Reg.bWide = pDecodeState->bFlagWide;
            constexpr u8 bHighMin = 4;
            if (pDecodeState->bFlagDirection == bSrc)
            {
                ASSERT(pDecodeState->bRM);
                Result.Reg.Type = GetReg(pDecodeState->RMValue);
                Result.Reg.bHigh = !pDecodeState->bFlagWide && pDecodeState->RMValue >= bHighMin;
            }
            else
            {
                ASSERT(pDecodeState->bReg);
                Result.Reg.Type = GetReg(pDecodeState->RegValue);
                Result.Reg.bHigh = !pDecodeState->bFlagWide && pDecodeState->RegValue >= bHighMin;
            }
        } break;
        case Operand_EffAddr:
        case Operand_EffAddrDisp:
        {
            if (pDecodeState->bFlagDirection == bSrc)
            {
                ASSERT(pDecodeState->bRM);
                Result.EffAddr.Type = GetEffAddr(pDecodeState->RMValue);
            }
            else
            {
                ASSERT(pDecodeState->bReg);
                Result.EffAddr.Type = GetEffAddr(pDecodeState->RegValue);
            }
            Result.EffAddr.Disp = {};
            Result.EffAddr.Disp.bPresent = Result.Type == Operand_EffAddrDisp;
            if (Result.EffAddr.Disp.bPresent)
            {
                Result.EffAddr.Disp.bWide = pDecodeState->bFlagWide;
                if (Result.EffAddr.Disp.bWide) { Result.EffAddr.Disp.Data16 = pDecodeState->DispValue; }
                else { Result.EffAddr.Disp.Data8 = pDecodeState->DispValue; }
            }
        } break;
        case Operand_Imm:
        {
            Result.Imm.bPresent = true;
            Result.Imm.bWide = pDecodeState->bFlagWide;
            if (Result.Imm.bWide) { Result.Imm.Data16 = pDecodeState->DataValue; }
            else { Result.Imm.Data8 = pDecodeState->DataValue; }
        } break;
        case Operand_Invalid: default: { DebugBreak(); } break;
    }
    return Result;
}

DecodedInst DecodeInst(InstructionFormatDesc InstDesc, u8* Inst)
{
    DecodedInst Result = {};
    Result.Op = InstDesc.OpType;
    Result.ByteWidth = 0;

    DecodeStateT DecodeState = InitDecodeState();

    Result.ByteWidth += 1;
    {
        if (InstDesc.Flags & InstF_BitD)
        {
            constexpr u8 BitDMask = 0b00000010;
            DecodeState.bFlagDirection = (*Inst) & BitDMask;
        }
        else if (InstDesc.Flags & InstF_BitS) // TODO: Assert that BitD and BitS are not both set
        {
            constexpr u8 BitSMask = 0b00000010;
            DecodeState.bFlagS = (*Inst) & BitSMask;
        }

        if (InstDesc.Flags & InstF_BitW)
        {
            if (InstDesc.EncodeBitCount == 4) // mov - immediate to register special case
            {
                constexpr u8 BitWMask = 0b00001000;
                DecodeState.bFlagWide = (*Inst) & BitWMask;
            }
            else
            {
                constexpr u8 BitWMask = 0b00000001;
                DecodeState.bFlagWide = (*Inst) & BitWMask;
            }
        }
        if (InstDesc.Flags & InstF_Mode)
        {
            constexpr u8 ModeMask = 0b11000000;
            DecodeState.bMode = true;
            DecodeState.ModeValue = ((*(Inst + 1)) & ModeMask) >> 6;
            Result.ByteWidth += 1;
        }
        if (InstDesc.Flags & InstF_Reg)
        {
            // If present, reg is in one of a few places:
            //      - Byte1: 00REG000
            //      - Byte0: 00000REG
            //      - Byte0: 000SR000 segment register for push/pop - we don't handle this right now
            DecodeState.bReg = true;
            if (InstDesc.EncodeBitCount == 4) // mov - immediate to register special case
            {
                constexpr u8 RegMask = 0b00000111;
                DecodeState.RegValue = (*Inst) & RegMask >> 0;
            }
            else
            {
                constexpr u8 RegMask = 0b00111000;
                DecodeState.RegValue = ((*(Inst + 1)) & RegMask) >> 3;
            }
        }
        //if (InstDesc.Flags & InstF_CompareReg) { }
        if (InstDesc.Flags & InstF_RM)
        {
            // RM, if present, is always in the second byte: 0000 0111
            constexpr u8 RMMask = 0b00000111;
            DecodeState.bRM = true;
            DecodeState.RMValue = ((*(Inst + 1)) & RMMask) >> 0;
        }
        if (InstDesc.Flags & InstF_Disp)
        {
            ASSERT(DecodeState.bMode);
            if (DecodeState.ModeValue == 0)
            {
                if (DecodeState.bRM && DecodeState.RMValue == 0b110)
                {
                    DecodeState.bDisp = true;
                    DecodeState.bDispWide = true;
                }
                else { DecodeState.bDisp = false; }
            }
            else if (DecodeState.ModeValue == 1)
            {
                DecodeState.bDisp = true;
                DecodeState.bDispWide = false;
            }
            else if (DecodeState.ModeValue == 2)
            {
                DecodeState.bDisp = true;
                DecodeState.bDispWide = true;
            }
            else if (DecodeState.ModeValue == 3)
            {
                DecodeState.bDisp = false;
            }

            if (DecodeState.bDisp)
            {
                if (DecodeState.bDispWide)
                {
                    DecodeState.DispValue = *(u16*)(Inst + 2);
                    Result.ByteWidth += 2;
                }
                else
                {
                    DecodeState.DispValue = (u16)(*(Inst + 2));
                    Result.ByteWidth += 1;
                }
            }
        }
        if (InstDesc.Flags & InstF_Data)
        {
            DecodeState.bData = true;
            if (DecodeState.bFlagS) { DecodeState.bDataWide = false; }
            else { DecodeState.bDataWide = DecodeState.bFlagWide; }

            if (DecodeState.bDataWide)
            {
                DecodeState.DataValue = *(u16*)(Inst + Result.ByteWidth);
                Result.ByteWidth += 2;
            }
            else
            {
                DecodeState.DataValue = *(Inst + 2 + Result.ByteWidth);
                Result.ByteWidth += 1;
            }
        }
    }

    {
        // NOTE:
        // OperandDesc: OperandType Type; ( RegisterDesc Reg; EffAddrDesc EffAddr; DataDesc Imm; )

        if (InstDesc.Flags & InstF_DstAcc)
        {
            Result.Operands[0].Type = Operand_Reg;
            Result.Operands[0].Reg.Type = Reg_a;
            Result.Operands[0].Reg.bHigh = false; // TODO: Is this always false?
            Result.Operands[0].Reg.bWide = DecodeState.bDataWide;
            Result.Operands[1] = GetOperandDesc(&DecodeState, 1);
        }
        else if (InstDesc.Flags & InstF_SrcAcc)
        {
            Result.Operands[0] = GetOperandDesc(&DecodeState, 0);
            Result.Operands[1].Type = Operand_Reg;
            Result.Operands[1].Reg.Type = Reg_a;
            Result.Operands[1].Reg.bHigh = false; // TODO: Is this always false?
            Result.Operands[1].Reg.bWide = DecodeState.bDataWide;
        }
        else
        {
            if (DecodeState.bFlagDirection)
            {
                Result.Operands[1] = GetOperandDesc(&DecodeState, 0);
                Result.Operands[0] = GetOperandDesc(&DecodeState, 1);
            }
            else
            {
                Result.Operands[0] = GetOperandDesc(&DecodeState, 0);
                Result.Operands[1] = GetOperandDesc(&DecodeState, 1);
            }
        }
    }

    return Result;
}

void CleanDecode86(const char* FileName)
{
    FileContentsT Asm86 = ReadFileContents(FileName);
    if (nullptr == Asm86.Contents) { return; }

    int InstCount = 0;
    DecodedInst InstStream[1000];

    int InstReadIdx = 0;
    while (InstReadIdx < Asm86.Size)
    {
        int InstByteCount = 0;

        u8 Inst0 = Asm86.Contents[InstReadIdx];
        u8 Inst1 = Asm86.Contents[InstReadIdx + 1];

        InstructionFormatDesc InstFmt = FetchInstFmtDesc(Inst0, Inst1);
        if (InstFmt.OpType != OpCode_Invalid)
        {
            printf("[Found]: %d\n", InstFmt.EncodeValue);
            DecodedInst OutInst = DecodeInst(InstFmt, Asm86.Contents + InstReadIdx);
            InstStream[InstCount++] = OutInst;
            InstReadIdx += OutInst.ByteWidth;
        }
        else
        {
            printf("[Invalid]\n");
            DebugBreak();
        }

        InstReadIdx += InstByteCount;
    }
}

int main(int ArgCount, const char* ArgValues[])
{
#define USE_CMD_LINE() (0)
#if USE_CMD_LINE()
    for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
    {
        CleanDecode86(ArgValues[ArgIdx]);
    }
#else
    {
        CleanDecode86("input/listing_0037_single_register_mov");
        //CleanDecode86("input/listing_0038_many_register_movs");
        //CleanDecode86("input/listing_0039_more_movs");
        //CleanDecode86("input/listing_0040_challenge_movs");
        //CleanDecode86("input/listing_0041_add_sub_cmp_jnz");
    }
#endif
}

