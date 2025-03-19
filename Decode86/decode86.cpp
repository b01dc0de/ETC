#include <stdio.h>
#include <windows.h>
#include <stdint.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using byte = unsigned char;

#define ASSERT(Exp) do { if (!(Exp)) DebugBreak(); } while (0)
#define ARRAY_SIZE(Arr) (sizeof((Arr)) / sizeof((Arr)[0]))

byte* ReadFile(const char* FileName, int* OutSize)
{
    ASSERT(FileName);
    ASSERT(OutSize);
    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");

    byte* Result = nullptr;
    *OutSize = 0;

    if (FileHandle)
    {
        int Size = 0;
        fseek(FileHandle, 0, SEEK_END);
        Size = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (Size > 0)
        {
            byte* Data = new byte[Size];
            fread_s(Data, Size, Size, 1, FileHandle);

            Result = Data;
            *OutSize = Size;
        }

        fclose(FileHandle);
    }

    return Result;
}

void WriteFile(const char* FileName, byte* Data, int Size)
{
    ASSERT(FileName);
    ASSERT(Data);
    ASSERT(Size > 0);
    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "wb");

    if (FileHandle)
    {
        fwrite(Data, Size, 1, FileHandle);
        fclose(FileHandle);
    }
}

int WriteStringToBuffer(char* Buffer, const char* String)
{
    ASSERT(Buffer && String);
    int WriteIdx = 0;
    int ReadIdx = 0;
    while (String[ReadIdx] != 0)
    {
        Buffer[WriteIdx++] = String[ReadIdx++];
    }
    return WriteIdx;
}

void WriteStringToBuffer(char* Buffer, const char* String, int Count)
{
    ASSERT(Buffer && String && Count > 0);
    for (int Idx = 0; Idx < Count; Idx++)
    {
        Buffer[Idx] = String[Idx];
    }
}

enum InstCaseType
{
    RegMem_To_RegMem, // Register/memory to/from register
    Imm_To_RegMem, // Immediate to register/memory
    Imm_To_Reg, // Immediate to register
    Mem_To_Acc, // Memory to accumulator
    Acc_To_Mem, // Accumulator to memory
    Imm_With_Acc, // Immediate with accumulator
    Rel_Jmp, // Relative jmp condition
    InstCaseType_Invalid
};

enum OpCodeType
{
    Op_Mov,
    Op_Add,
    Op_Sub,
    Op_Cmp,
    Op_Jmp,
    Op_Invalid
};

struct InstState
{
    int OpCodeID;
    int ByteCount; // Number of bytes in memory
    int Ops[2]; // Dst == [0], Src == [1]

    bool bMode;
    bool bReg;
    bool bRM;

    int Mode; // 2 bits, if present
    int Reg; // 3 bits, if present
    int RM; // 3 bits, if present

    bool bFlagD;
    bool bFlagW;
    // TODO: Other single-bit flags here;

    bool bData;
    bool bDataIsWide;
    int Data;

    bool bDisp;
    bool bDispIsWide;
    int Disp;
};

constexpr int BufferSize = 32;

InstCaseType ParseInstCase(byte Inst0)
{
    constexpr int ImmToRegMask = 0xF0; // 1111 0000
    constexpr int ImmToRegValue = 0xB0; // 1011 0000

    constexpr int RegMemToRegMemMask = 0xFC; // 1111 1100
    constexpr int RegMemToRegMemValue = 0x88; // 1000 0100

    constexpr int ImmToRegMemMask = 0xFE; // 1111 1110
    constexpr int ImmToRegMemValue = 0xC6; // 1100 0110

    constexpr int MemToAccMask = ImmToRegMemMask;
    constexpr int MemToAccValue = 0xA0; // 1010 0000

    constexpr int AccToMemMask = ImmToRegMemMask;
    constexpr int AccToMemValue = 0xA2; // 1010 0010

    InstCaseType Result = InstCaseType_Invalid;

    if ((Inst0 & ImmToRegMask) == ImmToRegValue) { Result = Imm_To_Reg; }
    else if ((Inst0 & RegMemToRegMemMask) == RegMemToRegMemValue) { Result = RegMem_To_RegMem; }
    else if ((Inst0 & ImmToRegMemMask) == ImmToRegMemValue) { Result = Imm_To_RegMem; }
    else if ((Inst0 & MemToAccMask) == MemToAccValue) { Result = Mem_To_Acc; }
    else if ((Inst0 & AccToMemMask) == AccToMemValue) { Result = Acc_To_Mem; }
    else { DebugBreak(); }
    return Result;
}

int ParseAndWriteInstNameAndType(byte* NextInst, char* OutInst, InstCaseType* OutCaseType, OpCodeType* OutOpType)
{
    ASSERT(NextInst);
    ASSERT(OutInst);
    ASSERT(OutCaseType);
    ASSERT(OutOpType);

    static const char* InstAddStr = "add";
    static const char* InstSubStr = "sub";
    static const char* InstCmpStr = "cmp";

    byte Byte0 = NextInst[0];

    constexpr int CommonAddSubCmpMask = 0xFC; // 1111 1100
    constexpr int CommonAddSubCmpValue = 0x80; // 1000 0000
    if ((Byte0 & CommonAddSubCmpMask) == CommonAddSubCmpValue)
    {
        // NOTE: ADD/SUB/CMP all share the same Immediate from register/memory value
        //       so we must distinguish based off of where the 'reg' field is normally
        byte Byte1 = NextInst[1];
        constexpr int RegInstFieldMask = 0x38; // 0011 1000
        constexpr int AddValue = 0x0; // 000
        constexpr int SubValue = 0x5; // 101
        constexpr int CmpValue = 0x7; // 111

        *OutCaseType = Imm_To_RegMem;

        int RegInstValue = (Byte1 & RegInstFieldMask) >> 3;
        if (RegInstValue == AddValue)
        {
            *OutOpType = Op_Add;
            return WriteStringToBuffer(OutInst, InstAddStr);
        }
        else if (RegInstValue == SubValue)
        {
            *OutOpType = Op_Sub;
            return WriteStringToBuffer(OutInst, InstSubStr);
        }
        else if (RegInstValue == CmpValue)
        {
            *OutOpType = Op_Cmp;
            return WriteStringToBuffer(OutInst, InstCmpStr);
        }
        else
        {
            // Not sure what instructions can get here
            DebugBreak();
            return 0;
        }
    }

    constexpr int First6BitsMask = 0xFC; // 1111 1100
    constexpr int Add_RegMemToRegMemValue = 0x00; // 0000 0000
    constexpr int Sub_RegMemToRegMemValue = 0x28; // 0010 1000
    constexpr int Cmp_RegMemAndRegValue = 0x38; // 0011 1000
    if ((Byte0 & First6BitsMask) == Add_RegMemToRegMemValue)
    {
        *OutCaseType = RegMem_To_RegMem;
        *OutOpType = Op_Add;
        return WriteStringToBuffer(OutInst, InstAddStr);
    }
    else if ((Byte0 & First6BitsMask) == Sub_RegMemToRegMemValue)
    {
        *OutCaseType = RegMem_To_RegMem;
        *OutOpType = Op_Sub;
        return WriteStringToBuffer(OutInst, InstSubStr);
    }
    else if ((Byte0 & First6BitsMask) == Cmp_RegMemAndRegValue)
    {
        *OutCaseType = RegMem_To_RegMem;
        *OutOpType = Op_Cmp;
        return WriteStringToBuffer(OutInst, InstCmpStr);
    }

    // TODO: This can be cleaned up A LOT, by actually utilizing the common 3 bit patterns
    constexpr int Left7BitsMask = 0xFE;
    constexpr int Add_ImmWithAccValue = 0x04; // 0000 0100
    constexpr int Sub_ImmWithAccValue = 0x2C; // 0010 1100
    constexpr int Cmp_ImmWithAccValue = 0x3C; // 0011 1100
    if ((Byte0 & Left7BitsMask) == Add_ImmWithAccValue)
    {
        *OutCaseType = Imm_With_Acc;
        *OutOpType = Op_Add;
        return WriteStringToBuffer(OutInst, InstAddStr);
    }
    else if ((Byte0 & Left7BitsMask) == Sub_ImmWithAccValue)
    {
        *OutCaseType = Imm_With_Acc;
        *OutOpType = Op_Sub;
        return WriteStringToBuffer(OutInst, InstSubStr);
    }
    else if ((Byte0 & Left7BitsMask) == Cmp_ImmWithAccValue)
    {
        *OutCaseType = Imm_With_Acc;
        *OutOpType = Op_Cmp;
        return WriteStringToBuffer(OutInst, InstCmpStr);
    }

    constexpr byte JumpInstValuesCommon[] =
    {
        0x70, // 0111 0000 // Jump on overflow
        0x78, // 0111 1000 // Jump on sign
        0x71, // 0111 0001 // Jump on not overflow
        0x79, // 0111 1001 // Jump on not sign
        0xE2, // 1110 0010 // Loop CX times
        0xE1, // 1110 0001 // Loop while zero/equal
        0xE0, // 1110 0000 // Loop while not zero/equal
        0xE3, // 1110 0011 // Jump on CX zero
    };
    const char* JumpInstNameValuesCommon[] =
    {
        "jo", // jump on overflow
        "js", // jump on sign
        "jno", // jump on not overflow
        "jns", // jump on not sign
        "loop", // loop cx times
        "loopz", // loop while zero
        "loopnz", // loo while not zero
        "jcxz" // jump on cx zero
    };
    constexpr byte JumpInstValues[] =
    {
        0x74, // 0111 0100 // Jump on equal/zero
        0x7C, // 0111 1100 // Jump on less/not greater or equal
        0x7E, // 0111 1110 // Jump on less or equal/not greater
        0x72, // 0111 0010 // Jump on below/not above or equal
        0x76, // 0111 0110 // Jump on below or equal/not above
        0x7A, // 0111 1010 // Jump on parity/parity even
        0x75, // 0111 0101 // Jump on not equal/not zero
        0x7D, // 0111 1101 // Jump on not less/greater or equal
        0x7F, // 0111 1111 // Jump on not less or equal/greater
        0x73, // 0111 0011 // Jump on not below/above or equal
        0x77, // 0111 0111 // Jump on not below or equal/above
        0x7B, // 0111 1011 // Jump on not par/par odd
    };
    const char* JumpInstNameValues[] =
    {
        "je", // jump on equal
        "jl", // jump on less
        "jle", // jump on less or equal
        "jb", // jump on below
        "jbe", // jump below or equal
        "jp", // jump on parity
        "jne", // jump on not equal
        "jnl", // jump on not less
        "jnle", // jump on not less or equal
        "jnb", // jump on not below
        "jnbe", // jump on not below or equal
        "jnp", // jump on not par
    };
    /*
    const char* JumpInstNameValuesAlt[] =
    {
        "jz", // jump on zero
        "jnge", // jump on not greater or equal
        "jng", // jump on not greater
        "jnae", // jump on not above or equal
        "jna", // jump on not above
        "jpe", // jump on parity even
        "jnz", // jump on not zero
        "jge", // jump on greater or equal
        "jg", // jump on greater
        "jae", // jump on above or equal
        "ja", // jump on above
        "jpo", // jump on par odd
    };
    */
    static_assert(ARRAY_SIZE(JumpInstValuesCommon) == ARRAY_SIZE(JumpInstNameValuesCommon), "Should be equal!");
    static_assert(ARRAY_SIZE(JumpInstValues) == ARRAY_SIZE(JumpInstNameValues), "Should be equal!");
    for (int CommonIdx = 0; CommonIdx < ARRAY_SIZE(JumpInstValuesCommon); CommonIdx++)
    {
        if (Byte0 == JumpInstValuesCommon[CommonIdx])
        {
            *OutCaseType = Rel_Jmp;
            *OutOpType = Op_Jmp;
            return WriteStringToBuffer(OutInst, JumpInstNameValuesCommon[CommonIdx]);
        }
    }
    for (int JumpValueIdx = 0; JumpValueIdx < ARRAY_SIZE(JumpInstValues); JumpValueIdx++)
    {
        if (Byte0 == JumpInstValues[JumpValueIdx])
        {
            *OutCaseType = Rel_Jmp;
            *OutOpType = Op_Jmp;
            return WriteStringToBuffer(OutInst, JumpInstNameValues[JumpValueIdx]);
        }
    }

    // mov fallback -- for now
    {
        *OutCaseType = ParseInstCase(Byte0);
        *OutOpType = Op_Mov;
        return WriteStringToBuffer(OutInst, "mov");
    }
}

void WriteRegToBuffer(char* Buffer, int Reg, bool bWide)
{
    static const char* WideRegTable[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
    static const char* NonWideRegTable[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
    ASSERT(0 <= Reg && Reg <= 7); // NOTE: Reg must be within [0, 7]
    if (Reg < 0 || 7 < Reg) { Buffer[0] = '?'; Buffer[1] = '?'; }
    else if (bWide)
    {
        Buffer[0] = WideRegTable[Reg][0];
        Buffer[1] = WideRegTable[Reg][1];
    }
    else
    {
        Buffer[0] = NonWideRegTable[Reg][0];
        Buffer[1] = NonWideRegTable[Reg][1];
    }
}

int WriteEffAddrToBuffer(char* Buffer, int Mode, int R_M, bool bWide, byte* pOptDisp, int DispSize)
{
    static const char* EffAddrCalcTable[] = {
        "[bx + si]",
        "[bx + di]",
        "[bp + si]",
        "[bp + di]",
        "[si]",
        "[di]",
        "[bp]",
        "[bx]"
    };
    static const char* EffAddrCalcTableDisp[] = {
        "[bx + si + %hd]",
        "[bx + di + %hd]",
        "[bp + si + %hd]",
        "[bp + di + %hd]",
        "[si + %hd]",
        "[di + %hd]",
        "[bp + %hd]",
        "[bx + %hd]"
    };
    ASSERT(0 <= R_M && R_M <= 7);
    switch (Mode)
    {
        case 0: // Memory Mode, no displacement follows (except when R_M == 110, then 16-bit displacement)
        {
            if (R_M == 6)
            {
                ASSERT(pOptDisp);
                ASSERT(DispSize == sizeof(short));
                short Disp = *(short*)pOptDisp;
                return sprintf_s(Buffer, BufferSize, "[%d]", Disp);
            }
            else
            {
                return WriteStringToBuffer(Buffer, EffAddrCalcTable[R_M]);
            }
        } break;
        case 1: // Memory Mode, 8-bit displacement follows
        {
            ASSERT(pOptDisp);
            ASSERT(DispSize == sizeof(char));
            ASSERT(!bWide);
            char Disp = *(char*)pOptDisp;
            if (Disp == 0)
            {
                return WriteStringToBuffer(Buffer, EffAddrCalcTable[R_M]);
            }
            else
            {
                return sprintf_s(Buffer, BufferSize, EffAddrCalcTableDisp[R_M], Disp);
            }
        } break;
        case 2: // Memory Mode, 16-bit displacement follows
        {
            ASSERT(pOptDisp);
            ASSERT(DispSize == sizeof(short));
            short Disp = *(short*)pOptDisp;
            return sprintf_s(Buffer, BufferSize, EffAddrCalcTableDisp[R_M], Disp);
        } break;
        case 3: // Register Mode (no displacement)
        {
            WriteRegToBuffer(Buffer, R_M, bWide);
            return 2;
        } break;
        default:
        {
            DebugBreak();
        } break;
    }
}

void DecodeAsm(const char* FileName)
{
    int Size = 0;
    byte* Data = ReadFile(FileName, &Size);
    if (!Data) { return; }

    printf("; %s:\n", FileName);

    char Dst[BufferSize];
    char Src[BufferSize];
    char InstName[BufferSize];

    int ReadIdx = 0;
    int InstCount = 0;
    while (ReadIdx < Size)
    {
        int DstWriteIdx = 0;
        int SrcWriteIdx = 0;
        int InstWriteIdx = 0;

        byte Inst0 = Data[ReadIdx];
        InstCaseType InstCase = InstCaseType_Invalid;
        OpCodeType OpCode = Op_Invalid;
        InstWriteIdx = ParseAndWriteInstNameAndType(Data + ReadIdx, InstName, &InstCase, &OpCode);

        bool bHandledFlag = false; // For debugging

        switch (OpCode)
        {
            case Op_Mov: {} break;
            case Op_Add:
            case Op_Sub:
            case Op_Cmp:
            {
                if (InstCase == Imm_With_Acc)
                {
                    bHandledFlag = true;
                    constexpr int W_ParamMask = 0x01; // 0000 0001
                    bool bWide = Inst0 & W_ParamMask;
                    Dst[0] = 'a';
                    Dst[1] = 'x';
                    DstWriteIdx = 2;
                    if (bWide)
                    {
                        short Imm = *(short*)(Data + ReadIdx + 1);
                        SrcWriteIdx = sprintf_s(Src, BufferSize, "%d", Imm);
                        ReadIdx += 3;
                    }
                    elsehttps://www.computerenhance.com/
                    {
                        char Imm = Data[ReadIdx + 1];
                        SrcWriteIdx = sprintf_s(Src, BufferSize, "%d", Imm);
                        ReadIdx += 2;
                    }
                }
            } break;
            case Op_Jmp:
            {
                ASSERT(InstCase == Rel_Jmp);
                bHandledFlag = true;
                char JmpOffset = Data[ReadIdx + 1] + 2;
                // TODO: No idea why the above +2 makes the values correct -- two's complement / something signed that i'm not getting?
                DstWriteIdx = sprintf_s(Dst, BufferSize, "$%+d", JmpOffset);
                ReadIdx += 2;
            } break;
            default:
            case Op_Invalid: DebugBreak(); break;
        }

        if (!bHandledFlag)
        {
            bHandledFlag = true;
            switch (InstCase)
            {
                case RegMem_To_RegMem:
                {
                    constexpr int D_ParamMask = 0x02; // Direction
                    constexpr int W_ParamMask = 0x01; // Wide
                    constexpr int ModMask = 0xC0; // 1100 0000
                    constexpr int RegMask = 0x38; // 0011 1000
                    constexpr int RMemMask = 0x07;  // 0000 0111

                    bool bRegIsDst = (Inst0 & D_ParamMask);
                    bool bW = (Inst0 & W_ParamMask);

                    char* OutMem = Dst;
                    char* OutReg = Src;
                    if (bRegIsDst)
                    {
                        OutReg = Dst;
                        OutMem = Src;
                    }

                    byte Byte1 = Data[ReadIdx + 1];

                    int ModValue = (Byte1 & ModMask) >> 6;
                    int RegValue = (Byte1 & RegMask) >> 3;
                    int R_M = (Byte1 & RMemMask);

                    switch (ModValue)
                    {
                        case 0: // Memory mode, no displacement (except when R/M == 110)
                        {
                            if (R_M == 6)
                            {
                                DstWriteIdx = WriteEffAddrToBuffer(OutMem, ModValue, R_M, bW, Data + ReadIdx + 2, 2);
                                ReadIdx += 4;
                            }
                            else
                            {
                                DstWriteIdx = WriteEffAddrToBuffer(OutMem, ModValue, R_M, bW, nullptr, 0);
                                ReadIdx += 2;
                            }
                            WriteRegToBuffer(OutReg, RegValue, bW);
                            SrcWriteIdx = 2;
                        } break;
                        case 1: // Memory mode, 8-bit displacement
                        {
                            DstWriteIdx = WriteEffAddrToBuffer(OutMem, ModValue, R_M, false, Data + ReadIdx + 2, 1);
                            WriteRegToBuffer(OutReg, RegValue, bW);
                            SrcWriteIdx = 2;
                            ReadIdx += 3;
                        } break;
                        case 2: // Memory mode, 16-bit displacement
                        {
                            DstWriteIdx = WriteEffAddrToBuffer(OutMem, ModValue, R_M, true, Data + ReadIdx + 2, 2);
                            WriteRegToBuffer(OutReg, RegValue, bW);
                            SrcWriteIdx = 2;
                            ReadIdx += 4;
                        } break;
                        case 3: // Register Mode (no displacement)
                        {
                            WriteRegToBuffer(OutReg, RegValue, bW);
                            WriteRegToBuffer(OutMem, R_M, bW);
                            DstWriteIdx = 2;
                            SrcWriteIdx = 2;
                            ReadIdx += 2;
                        } break;
                    }

                    if (bRegIsDst)
                    {
                        int TmpWriteIdx = DstWriteIdx;
                        DstWriteIdx = SrcWriteIdx;
                        SrcWriteIdx = TmpWriteIdx;
                    }
                } break;
                case Imm_To_RegMem:
                {
                    constexpr int W_ParamMask = 0x01; // 0000 0001
                    constexpr int S_ParamMask = 0x02; // 0000 0010
                    constexpr int ModMask = 0xC0; // 1100 0000
                    constexpr int MemMask = 0x07; // 0000 0111
                    bool bW = Inst0 & W_ParamMask;
                    bool bS = false;

                    if (OpCode != Op_Mov)
                    {
                        bS = Inst0 & S_ParamMask;
                    }

                    byte Byte1 = Data[ReadIdx + 1];
                    int ModValue = (Byte1 & ModMask) >> 6;
                    int R_M = (Byte1 & MemMask);

                    //int WriteEffAddrToBuffer(char* Buffer, int Mode, int R_M, bool bWide, byte * pOptDisp, int DispSize)
                    switch (ModValue)
                    {
                        case 0: // Memory mode, no displacement (except when R/M == 110)
                        {
                            if (R_M == 6)
                            {
                                DstWriteIdx = WriteEffAddrToBuffer(Dst, ModValue, R_M, bW, Data + ReadIdx + 2, 2);
                                ReadIdx += 4;
                            }
                            else
                            {
                                DstWriteIdx = WriteEffAddrToBuffer(Dst, ModValue, R_M, bW, nullptr, 0);
                                ReadIdx += 2;
                            }

                            if (bW && !bS)
                            {
                                short ImmVal = *(short*)(Data + ReadIdx);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "word" : "", ImmVal);
                                ReadIdx += 2;
                            }
                            else
                            {
                                char ImmVal = *(char*)(Data + ReadIdx);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "byte" : "", ImmVal);
                                ReadIdx += 1;
                            }

                            //if (R_M == 6) { if (bW) ReadIdx += 6; else ReadIdx += 5; }
                            //else { if (bW) ReadIdx += 4; else ReadIdx += 3; }
                        } break;
                        case 1: // Memory mode, 8-bit displacement
                        {
                            DstWriteIdx = WriteEffAddrToBuffer(Dst, ModValue, R_M, bW, Data + ReadIdx + 2, 1);
                            if (bW && !bS)
                            {
                                short ImmVal = *(short*)(Data + ReadIdx + 3);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "word" : "", ImmVal);
                                ReadIdx += 5;
                            }
                            else
                            {
                                char ImmVal = *(char*)(Data + ReadIdx + 3);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "byte" : "", ImmVal);
                                ReadIdx += 4;
                            }
                        } break;
                        case 2: // Memory mode, 16-bit displacement
                        {
                            DstWriteIdx = WriteEffAddrToBuffer(Dst, ModValue, R_M, bW, Data + ReadIdx + 2, 2);
                            if (bW && !bS)
                            {
                                short ImmVal = *(short*)(Data + ReadIdx + 4);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "word" : "", ImmVal);
                                ReadIdx += 6;
                            }
                            else
                            {
                                char ImmVal = *(char*)(Data + ReadIdx + 4);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "byte" : "", ImmVal);
                                ReadIdx += 5;
                            }
                        } break;
                        case 3: // Register Mode (no displacement)
                        {
                            WriteRegToBuffer(Dst, R_M, bW);
                            DstWriteIdx = 2;
                            if (bW && !bS)
                            {
                                short ImmVal = *(short*)(Data + ReadIdx + 2);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "word" : "", ImmVal);
                                ReadIdx += 4;
                            }
                            else
                            {
                                char ImmVal = *(char*)(Data + ReadIdx + 2);
                                SrcWriteIdx = sprintf_s(Src, BufferSize, "%s %d", Dst[0] == '[' ? "byte" : "", ImmVal);
                                ReadIdx += 3;
                            }
                        } break;
                    }
                } break;
                case Imm_To_Reg:
                {
                    constexpr int W_ParamMask = 0x08; // 0000 1000
                    constexpr int RegMask = 0x07; // 0000 0111
                    bool bW = Inst0 & W_ParamMask;
                    int RegValue = (Inst0 & RegMask);
                    WriteRegToBuffer(Dst, RegValue, bW);
                    DstWriteIdx = 2;
                    if (bW)
                    {
                        short Byte12 = *(short*)(Data + ReadIdx + 1);
                        SrcWriteIdx = sprintf_s(Src, "[%d]", Byte12);
                        ReadIdx += 3;
                    }
                    else
                    {
                        char Byte1 = Data[ReadIdx + 1];
                        SrcWriteIdx = sprintf_s(Src, "[%d]", Byte1);
                        ReadIdx += 2;
                    }
                } break;
                case Mem_To_Acc:
                {
                    constexpr int W_ParamMask = 0x01; // 0000 0001
                    Dst[0] = 'a';
                    Dst[1] = 'x';
                    DstWriteIdx = 2;

                    bool bW = Inst0 & W_ParamMask;
                    if (bW)
                    {
                        short Bytes12 = *(short*)(Data + ReadIdx + 1);
                        SrcWriteIdx = sprintf_s(Src, "[%d]", Bytes12);
                        ReadIdx += 3;
                    }
                    else
                    {
                        char Byte1 = Data[ReadIdx + 1];
                        SrcWriteIdx = sprintf_s(Src, "[%d]", Byte1);
                        ReadIdx += 2;
                    }
                } break;
                case Acc_To_Mem:
                {
                    constexpr int W_ParamMask = 0x01; // 0000 0001
                    Src[0] = 'a';
                    Src[1] = 'x';
                    SrcWriteIdx = 2;

                    bool bW = Inst0 & W_ParamMask;
                    if (bW)
                    {
                        short Bytes12 = *(short*)(Data + ReadIdx + 1);
                        DstWriteIdx = sprintf_s(Dst, "[%d]", Bytes12);
                        ReadIdx += 3;
                    }
                    else
                    {
                        char Byte1 = Data[ReadIdx + 1];
                        DstWriteIdx = sprintf_s(Dst, "[%d]", Byte1);
                        ReadIdx += 2;
                    }
                } break;
            }

        }

        char AsmOutput[BufferSize];
        int WriteIdx = 0;
        WriteStringToBuffer(AsmOutput + WriteIdx, InstName, InstWriteIdx);
        WriteIdx += InstWriteIdx;

        AsmOutput[WriteIdx++] = ' ';

        WriteStringToBuffer(AsmOutput + WriteIdx, Dst, DstWriteIdx);
        WriteIdx += DstWriteIdx;

        if (SrcWriteIdx > 0)
        {
            AsmOutput[WriteIdx++] = ',';
            AsmOutput[WriteIdx++] = ' ';
            WriteStringToBuffer(AsmOutput + WriteIdx, Src, SrcWriteIdx);
            WriteIdx += SrcWriteIdx;
        }
        AsmOutput[WriteIdx++] = '\0';

        printf("%s\n", AsmOutput);

        InstCount++;
    }
    printf("\n");
}

int main(int ArgCount, const char* ArgValues[])
{
    if (ArgCount > 1)
    {
        int ArgIdx = 1;
        while (ArgIdx < ArgCount)
        {
            DecodeAsm(ArgValues[ArgIdx++]);
        }
    }
    return 0;
}
