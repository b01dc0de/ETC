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

    struct ProfileEntry
    {
        const char* Name;
        u32 HitCount;
        bool bActive;
        u64 Time;
        u64 TimeChildren;
    };

    struct ScopedTiming
    {
        int EntryIdx;
        int ParentIdx;
        u64 Begin;

        ScopedTiming(const char* InName, int Index);
        ~ScopedTiming();
    };

    void BeginProfiling();
    void EndProfiling();
}

#define PROFILING_BEGIN() Perf::BeginProfiling()
#define PROFILING_END() Perf::EndProfiling()
#if ENABLE_PROFILER
#define TIME_FUNC() Perf::ScopedTiming _ST_##__func__(__func__, __COUNTER__);
#define TIME_BLOCK(name) Perf::ScopedTiming _ST_##name(#name, __COUNTER__);
#else
#define TIME_FUNC() (void)0
#define TIME_BLOCK(name) (void)0
#endif // ENABLE_PROFILER

#endif // HAVERSINE_PERF_H

