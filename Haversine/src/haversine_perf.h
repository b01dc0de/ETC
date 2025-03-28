#ifndef HAVERSINE_PERF_H
#define HAVERSINE_PERF_H

#include "haversine_common.h"

namespace _PerfTimings
{
    u64 ReadOSTimer();
    u64 GetOSFreq();
    u64 ReadCPUTimer();
    u64 EstimateCPUFreq();

    struct _PerfTimingEntry
    {
        const char* FuncName;
        u64 Begin;
        u64 End;
        u64 GetDelta();
    };

    static constexpr int _MaxTimingsCount = 1000;

    extern _PerfTimingEntry _Total;
    extern int _PerfTimingsCount;
    extern _PerfTimingEntry _PerfEntries[_MaxTimingsCount];

    struct _ScopedPerfTiming
    {
        const char* FuncName;
        int EntryIdx;
        u64 Begin;
        u64 End;

        void Start(const char *InFuncName);
        void Stop();
        _ScopedPerfTiming(const char* InFuncName);
        ~_ScopedPerfTiming();
    };

    void PrintTimings(_PerfTimingEntry* Entries, int NumTimings);
}

#define PROFILING_BEGIN()\
    _PerfTimings::_PerfTimingsCount = 0;\
    _PerfTimings::_Total.FuncName = "[Total]";\
    _PerfTimings::_Total.Begin = _PerfTimings::ReadCPUTimer();\
    _PerfTimings::_Total.End = 0u;

#define PROFILING_END()\
    _PerfTimings::_Total.End = _PerfTimings::ReadCPUTimer();\
    _PerfTimings::PrintTimings(_PerfTimings::_PerfEntries, _PerfTimings::_PerfTimingsCount);

#define TIME_FUNC()\
    _PerfTimings::_ScopedPerfTiming _SPF_##__func__(__func__);
#define TIME_BLOCK(name)\
    _PerfTimings::_ScopedPerfTiming _SPF_##name(__func__ #name);

#endif // HAVERSINE_PERF_H

