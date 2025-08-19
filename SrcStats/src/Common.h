#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void Outf(const char* Fmt, ...);
#define ASSERT(Exp) if (!(Exp)) { Outf("[error] ASSERT failed: %s\n", ##Exp); DebugBreak(); }

#endif // COMMON_H

