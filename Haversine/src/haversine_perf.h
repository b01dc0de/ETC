#ifndef HAVERSINE_PERF_H
#define HAVERSINE_PERF_H

#include "haversine_common.h"

namespace Perf
{
    u64 ReadOSTimer();
    u64 GetOSFreq();
    u64 ReadCPUTimer();
    u64 EstimateCPUFreq();

    struct ProfileEntry
    {
        const char* Name;
        u64 HitCount;
        u64 Time;
        //bool bActive;
    };

    static constexpr int MaxEntries = 4096;

    extern u64 TotalBegin;
    extern u64 TotalEnd;
    extern ProfileEntry Entries[MaxEntries];

    struct ScopedTiming
    {
        const char* Name;
        int EntryIdx;
        int ParentIdx;
        u64 Begin;

        ScopedTiming(const char* InName, int Index);
        ~ScopedTiming();
        void Start(const char *InName, int Index);
        void Stop();
    };

    void PrintTimings(ProfileEntry* Entries);
}

#define PROFILING_BEGIN()\
    Perf::TotalBegin = Perf::ReadCPUTimer();

#define PROFILING_END()\
    Perf::TotalEnd = Perf::ReadCPUTimer();\
    Perf::PrintTimings(Perf::Entries);

#define TIME_FUNC()\
    Perf::ScopedTiming _ST_##__func__(__func__, __COUNTER__);
#define TIME_BLOCK(name)\
    Perf::ScopedTiming _ST_##name(__func__, __COUNTER__);

#endif // HAVERSINE_PERF_H

