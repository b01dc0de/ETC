#ifndef VIRTUAL86_PRINT_H
#define VIRTUAL86_PRINT_H

#include "virtual86_common.h"
#include "virtual86_sim.h"

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
    "shl", // "sal"
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
    "je", // "jz"
    "jl", // "jnge"
    "jle", // "jng"
    "jb", // "jnae"
    "jbe", // "jna"
    "jp", // "jpe"
    "jo",
    "js",
    "jne", // "jnz"
    "jnl", // "jge"
    "jnle", // "jg"
    "jnb", // "jae"
    "jnbe", // "ja"
    "jnp", // "jpo"
    "jno",
    "jns",
    "loop",
    "loopz", // "loope"
    "loopnz", // "loopne"
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

void PrintOperand(Operand* pOperand);
void PrintInst(VirtualInst* Inst);
void PrintInstStream(VirtualInstStream* pInstStream);
void PrintState(Sim86State* pSimState);

#endif // VIRTUAL86_PRINT_H
