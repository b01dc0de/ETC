#ifndef HAVERSINE_PERF_H
#define HAVERSINE_PERF_H

#include "haversine_common.h"

namespace PerfTimings
{
    u64 ReadOSTimer();
    u64 GetOSFreq();
    u64 ReadCPUTimer();
    u64 EstimateCPUFreq();

    struct PerfTimingEntry
    {
        const char* FuncName;
        u64 Begin;
        u64 End;
        u64 GetDelta();
    };

    static constexpr int MaxTimingsCount = 1000;

    extern PerfTimingEntry Total;
    extern int PerfTimingsCount;
    extern PerfTimingEntry PerfEntries[MaxTimingsCount];

    struct ScopedPerfTiming
    {
        const char* FuncName;
        int EntryIdx;
        u64 Begin;
        u64 End;

        void Start(const char *InFuncName);
        void Stop();
        ScopedPerfTiming(const char* InFuncName);
        ~ScopedPerfTiming();
    };

    void PrintTimings(PerfTimingEntry* Entries, int NumTimings);
}

#define PROFILING_BEGIN()\
    PerfTimings::PerfTimingsCount = 0;\
    PerfTimings::Total.FuncName = "[Total]";\
    PerfTimings::Total.Begin = PerfTimings::ReadCPUTimer();\
    PerfTimings::Total.End = 0u;

#define PROFILING_END()\
    PerfTimings::Total.End = PerfTimings::ReadCPUTimer();\
    PerfTimings::PrintTimings(PerfTimings::PerfEntries, PerfTimings::PerfTimingsCount);

#define TIME_FUNC()\
    PerfTimings::ScopedPerfTiming _SPF_##__func__(__func__);
#define TIME_BLOCK(name)\
    PerfTimings::ScopedPerfTiming _SPF_##name(__func__ #name);

#endif // HAVERSINE_PERF_H

