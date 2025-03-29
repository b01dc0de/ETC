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
        u64 Begin;
        u64 End;
        u64 GetDelta();
    };

    static constexpr int MaxEntries = 4096;

    extern ProfileEntry Total;
    extern int EntriesCount;
    extern ProfileEntry Entries[MaxEntries];

    struct ScopedTiming
    {
        const char* Name;
        int EntryIdx;
        u64 Begin;
        u64 End;

        void Start(const char *InName);
        void Stop();
        ScopedTiming(const char* InName);
        ~ScopedTiming();
    };

    void PrintTimings(ProfileEntry* Entries, int NumEntries);
}

#define PROFILING_BEGIN()\
    Perf::EntriesCount= 0;\
    Perf::Total.Name = "[Total]";\
    Perf::Total.Begin = Perf::ReadCPUTimer();\
    Perf::Total.End = 0u;

#define PROFILING_END()\
    Perf::Total.End = Perf::ReadCPUTimer();\
    Perf::PrintTimings(Perf::Entries, Perf::EntriesCount);

#define TIME_FUNC()\
    Perf::ScopedTiming _ST_##__func__(__func__);
#define TIME_BLOCK(name)\
    Perf::ScopedTiming _ST_##name(__func__ #name);

#endif // HAVERSINE_PERF_H

