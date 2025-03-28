#include "haversine_perf.h"

namespace _PerfTimings
{
#if _WIN32
#include <intrin.h>
#include <windows.h>
    u64 ReadOSTimer()
    {
        LARGE_INTEGER PerfCount;
        QueryPerformanceCounter(&PerfCount);
        return PerfCount.QuadPart;
    }
    u64 GetOSFreq()
    {
        LARGE_INTEGER Freq;
        QueryPerformanceFrequency(&Freq);
        return Freq.QuadPart;
    }
#else // NOT _WIN32
#include <x86intrin.h>
#include <sys/time.h>
    // TODO: These are untested currently until I build/run these on non-Windows platforms!!
    u64 ReadOSTimer()
    {
        timeval tval;
        gettimeofday(&tval, 0);
        u64 Result = GetOSTimerFreq()*(u64)tval.tv_sex + (u64)tval.tv_usec;
        return Result;
    }
    u64 GetOSFreq() { return 1000000u; }
#endif // _WIN32
    u64 ReadCPUTimer()
    {
        return __rdtsc();
    }

    u64 EstimateCPUFreq()
    {
        u64 CPUStart = ReadCPUTimer();
        u64 OSFreq = GetOSFreq();
        u64 OSBegin = ReadOSTimer();
        u64 OSEnd = 0;
        u64 OSElapsed = 0;
        while (OSElapsed < OSFreq)
        {
            OSEnd = ReadOSTimer();
            OSElapsed = OSEnd - OSBegin;
        }

        u64 CPUElapsed = ReadCPUTimer();
        if (OSElapsed)
        {
            return OSFreq * CPUElapsed / OSElapsed;
        }
        return 0;
    }
    f64 GetElapsedTimeSeconds(u64 Delta, u64 Freq)
    {
        return (f64)Delta / (f64)Freq;
    }



    u64 _PerfTimingEntry::GetDelta()
    {
        if (End > Begin) { return End - Begin; }
        else { return 0u; }
    }

    _PerfTimingEntry _Total;
    int _PerfTimingsCount = 0;
    _PerfTimingEntry _PerfEntries[_MaxTimingsCount] = {};

    void _ScopedPerfTiming::Start(const char *InFuncName)
    {
        FuncName = InFuncName;
        EntryIdx = _PerfTimingsCount++;
        Begin = ReadCPUTimer();
    }
    void _ScopedPerfTiming::Stop()
    {
        End = ReadCPUTimer();
        _PerfEntries[EntryIdx] = {FuncName, Begin, End};
    }
    _ScopedPerfTiming::_ScopedPerfTiming(const char* InFuncName) { Start(InFuncName); }
    _ScopedPerfTiming::~_ScopedPerfTiming() { Stop(); }

    void PrintTimings(_PerfTimingEntry* Entries, int NumTimings)
    {
        if (_PerfTimings::_PerfTimingsCount >= _PerfTimings::_MaxTimingsCount)
        {
            fprintf(stdout, "[error] Too many timings (%d) for timing buffer size (%d)!\n",
                    _PerfTimings::_PerfTimingsCount, _PerfTimings::_MaxTimingsCount);
            return;
        }
        else
        {
            u64 CPUFreq = EstimateCPUFreq();

            f64 TotalTime = (f64)_PerfTimings::_Total.GetDelta() / (f64)CPUFreq * 1000.0;

            fprintf(stdout, "BEGIN PRINT TIMINGS:\n");
            fprintf(stdout, "\t%s : %.04f ms\n",
                    _PerfTimings::_Total.FuncName,
                    TotalTime);
            for (int Idx = 0; Idx < _PerfTimings::_PerfTimingsCount; Idx++)
            {
                f64 EntryTime = (f64)_PerfEntries[Idx].GetDelta() / (f64)CPUFreq * 1000.0f;
                f64 EntryPercent = (f64)EntryTime / TotalTime * 100.0;
                fprintf(stdout, "\t[%d] : %s : %.04f ms (%.02f%%)\n",
                        Idx, _PerfEntries[Idx].FuncName,
                        EntryTime, EntryPercent);
            }
            fprintf(stdout, ":END PRINT TIMINGS\n");
        }
    }
}
