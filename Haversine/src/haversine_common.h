#ifndef HAVERSINE_COMMON_H
#define HAVERSINE_COMMON_H

// C stdlib headers:
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// C++ stdlib headers:
#include <random>

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

struct CoordPair
{
    f64 X0;
    f64 Y0;
    f64 X1;
    f64 Y1;
};

struct CoordPairList
{
    int Count;
    CoordPair* Data;
};

using HPair = CoordPair;
using HList = CoordPairList;

struct PerfTiming
{
    const char* FuncName;
    u64 Begin;
    u64 End;
};

#endif // HAVERSINE_COMMON_H

