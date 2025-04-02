#ifndef HAVERSINE_PERF_H
#define HAVERSINE_PERF_H

#include "haversine_common.h"

#ifndef ENABLE_PROFILER
#define ENABLE_PROFILER (0)
#endif // ENABLE_PROFILER

namespace Perf
{
    u64 ReadOSTimer();
    u64 GetOSFreq();
    u64 ReadCPUTimer();
    u64 EstimateCPUFreq();

    struct ProfileAnchor
    {
        const char* Name;
        u64 HitCount;
        u64 TimeElapsedExclusive; // Does NOT include children
        u64 TimeElapsedInclusive; // Does include children
    };

    struct ScopedTiming
    {
        const char* Name;
        u64 OldTimeElapsedInclusive;
        u64 StartTime;
        u32 ParentIndex;
        u32 Index;

        ScopedTiming(const char* Name_, int Index_);
        ~ScopedTiming();
    };

    void BeginProfiling();
    void EndProfiling();
}

#define PROFILING_BEGIN() Perf::BeginProfiling()
#define PROFILING_END() Perf::EndProfiling()
#if ENABLE_PROFILER
#define TIME_FUNC() Perf::ScopedTiming _ST_##__func__(__func__, __COUNTER__ + 1);
#define TIME_BLOCK(name) Perf::ScopedTiming _ST_##name(#name, __COUNTER__ + 1);
#else
#define TIME_FUNC() (void)0
#define TIME_BLOCK(name) (void)0
#endif // ENABLE_PROFILER

#endif // HAVERSINE_PERF_H

