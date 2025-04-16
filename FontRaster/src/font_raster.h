#ifndef FONT_RASTER_H
#define FONT_RASTER_H

// Std lib headers
#include <stdint.h>
#include <stdio.h>

// Platform headers
#include <windows.h>

using byte = unsigned char;
using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using ullong = unsigned long long;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

#if DEBUG_BUILD()
    #define DEBUG_BREAK() DebugBreak()
#else // DEBUG_BUILD()
    #define DEBUG_BREAK() (void)0
#endif // DEBUG_BUILD()

#define MAIN_ERRCHK(Exp, FuncName) if ((Exp)) { fprintf(stdout, "[error] %s failed!\n", #FuncName); DEBUG_BREAK(); return 1; }

#endif // FONT_RASTER_H
