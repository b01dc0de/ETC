#include "haversine_perf.h"

namespace PerfTimings
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

    u64 PerfTimingEntry::GetDelta()
    {
        if (End > Begin) { return End - Begin; }
        else { return 0u; }
    }

    PerfTimingEntry Total;
    int PerfTimingsCount = 0;
    PerfTimingEntry PerfEntries[MaxTimingsCount] = {};

    void ScopedPerfTiming::Start(const char *InFuncName)
    {
        FuncName = InFuncName;
        EntryIdx = PerfTimingsCount++;
        Begin = ReadCPUTimer();
    }
    void ScopedPerfTiming::Stop()
    {
        End = ReadCPUTimer();
        PerfEntries[EntryIdx] = {FuncName, Begin, End};
    }
    ScopedPerfTiming::ScopedPerfTiming(const char* InFuncName) { Start(InFuncName); }
    ScopedPerfTiming::~ScopedPerfTiming() { Stop(); }

    void PrintTimings(PerfTimingEntry* Entries, int NumTimings)
    {
        if (PerfTimings::PerfTimingsCount >= PerfTimings::MaxTimingsCount)
        {
            fprintf(stdout, "[error] Too many timings (%d) for timing buffer size (%d)!\n",
                    PerfTimings::PerfTimingsCount, PerfTimings::MaxTimingsCount);
            return;
        }
        else
        {
            u64 CPUFreq = EstimateCPUFreq();

            f64 TotalTime = (f64)PerfTimings::Total.GetDelta() / (f64)CPUFreq * 1000.0;

            fprintf(stdout, "BEGIN PRINT TIMINGS:\n");
            fprintf(stdout, "\t%s : %.04f ms\n",
                    PerfTimings::Total.FuncName,
                    TotalTime);
            for (int Idx = 0; Idx < PerfTimings::PerfTimingsCount; Idx++)
            {
                f64 EntryTime = (f64)PerfEntries[Idx].GetDelta() / (f64)CPUFreq * 1000.0f;
                f64 EntryPercent = (f64)EntryTime / TotalTime * 100.0;
                fprintf(stdout, "\t[%d] : %s : %.04f ms (%.02f%%)\n",
                        Idx, PerfEntries[Idx].FuncName,
                        EntryTime, EntryPercent);
            }
            fprintf(stdout, ":END PRINT TIMINGS\n");
        }
    }
}
