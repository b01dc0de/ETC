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

struct HaversinePairData
{
    float X0;
    float Y0;
    float X1;
    float Y1;
};

struct HaversinePairList
{
    int Count;
    HaversinePairData* Data;
};

using PairType = HaversinePairData;
using ListType = HaversinePairList;

#endif // HAVERSINE_COMMON_H

