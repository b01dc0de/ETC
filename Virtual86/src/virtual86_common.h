#ifndef VIRTUAL86_COMMON_H
#define VIRTUAL86_COMMON_H

#include <stdint.h>
#include <stdio.h>

#include <windows.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

#define ASSERT(Exp) do { if (!(Exp)) DebugBreak(); } while (0)
#define ARRAY_SIZE(Arr) (sizeof((Arr)) / sizeof((Arr)[0]))

struct FileContentsT
{
    const char* Name;
    u8* Data;
    size_t Size;
};

FileContentsT ReadFileContents(const char* FileName);

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

static const char* OpCodeMnemonicTable[] = 
{
    "----",
    "mov",
    "push",
    "pop",
    "xchg",
    "in",
    "out",
    "xlat",
    "lea",
    "lds",
    "les",
    "lahf",
    "sahf",
    "pushf",
    "popf",
    "add",
    "adc",
    "inc",
    "aaa",
    "daa",
    "sub",
    "sbb",
    "dec",
    "neg",
    "cmp",
    "aas",
    "das",
    "mul",
    "imul",
    "aam",
    "div",
    "idiv",
    "aad",
    "cbw",
    "cwd",
    "not",
    "shlsal",
    "shr",
    "sar",
    "rol",
    "ror",
    "rcl",
    "rcr",
    "and",
    "test",
    "or",
    "xor",
    "rep",
    "movs",
    "cmps",
    "scas",
    "lods",
    "stds",
    "call",
    "jmp",
    "ret",
    "jejz",
    "jljnge",
    "jlejng",
    "jbjnae",
    "jbejna",
    "jbjpe",
    "jo",
    "js",
    "jnejnz",
    "jnljge",
    "jnlejg",
    "jnbjae",
    "jnbeja",
    "jnpjpo",
    "jno",
    "jns",
    "loop",
    "loopzloope",
    "loopnzloopne",
    "jcxz",
    "int",
    "into",
    "iret",
    "clc",
    "cmc",
    "stc",
    "cld",
    "std",
    "cli",
    "sti",
    "hlt",
    "wait",
    "esc",
    "lock",
    "segment",
    "++++"
};

#endif // VIRTUAL86_COMMON_H
