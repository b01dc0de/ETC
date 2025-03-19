#ifndef DECODE86_COMMON_H
#define DECODE86_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define ASSERT(Exp) do { if (!(Exp)) DebugBreak(); } while (0)
#define ARRAY_SIZE(Arr) (sizeof((Arr)) / sizeof((Arr)[0]))

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using byte = unsigned char;

struct FileContentsT
{
    const char* Name;
    u8* Contents;
    int Size; // in bytes
};

FileContentsT ReadFileContents(const char* FileName);

#endif // DECODE86_COMMON_H

