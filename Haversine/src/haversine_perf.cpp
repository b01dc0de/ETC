#include "haversine_perf.h"

namespace Perf
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

    static u64 TotalBegin = 0;
    static u64 TotalEnd = 0;
    static bool bProfiling = false;

#if ENABLE_PROFILER
    static constexpr int MaxEntries = 4096;

    static int GlobalCurrentIndex = 0;
    static ProfileEntry Entries[MaxEntries] = {};

    ScopedTiming::ScopedTiming(const char* InName, int Index)
    {
        static constexpr int StartIdxOffset = 1;
        EntryIdx = Index + StartIdxOffset;
        if (!Entries[EntryIdx].bActive)
        {
            Entries[EntryIdx].bActive = true;
            Entries[EntryIdx].Name = InName;
            EntryIdx = Index + StartIdxOffset;
            ParentIdx = GlobalCurrentIndex;
            GlobalCurrentIndex = EntryIdx;
            Begin = ReadCPUTimer();
        }
        else
        {
            EntryIdx = 0;
        }
    }
    ScopedTiming::~ScopedTiming()
    {
        if (EntryIdx)
        {
            u64 Delta = ReadCPUTimer() - Begin;

            if (ParentIdx)
            {
                ProfileEntry* Parent = Entries + ParentIdx;
                Parent->TimeChildren += Delta;
            }
            GlobalCurrentIndex = ParentIdx;

            ProfileEntry* ThisEntry = Entries + EntryIdx;
            ++ThisEntry->HitCount;
            ThisEntry->Time += Delta;
            ThisEntry->bActive = false;
        }
    }

    void PrintTimings(ProfileEntry* Entries)
    {
        // TODO: Figure out a better way of doing this check
        if (__COUNTER__ >= MaxEntries)
        {
            fprintf(stdout, "[error] Too many timings (%d) for timing buffer size (%d)!\n",
                    __COUNTER__, MaxEntries);
        }
        else
        {
            u64 CPUFreq = EstimateCPUFreq();

            f64 TotalTime = (f64)(TotalEnd - TotalBegin) / (f64)CPUFreq * 1000.0;

            fprintf(stdout, "BEGIN PRINT TIMINGS:\n");
            for (int Idx = 1; Idx < MaxEntries; Idx++)
            {
                if (!Entries[Idx].Name || !Entries[Idx].HitCount) { continue; }

                f64 EntryTime = (f64)(Entries[Idx].Time - Entries[Idx].TimeChildren) / (f64)CPUFreq * 1000.0f;
                f64 EntryPercent = (f64)EntryTime / TotalTime * 100.0;

                fprintf(stdout, "\t%s[%u]: %.04f ms (%.02f%%)\n",
                        Entries[Idx].Name, Entries[Idx].HitCount,
                        EntryTime, EntryPercent);
                if (Entries[Idx].TimeChildren)
                {
                    f64 ChildrenTime = (f64)Entries[Idx].TimeChildren / (f64)CPUFreq * 1000.0f;
                    f64 ChildrenPercent = (f64)ChildrenTime / TotalTime * 100.0;
                    fprintf(stdout, "\t\tChildren: %.04f ms (%.02f%%)\n",
                            ChildrenTime, ChildrenPercent);

                }
            }
            fprintf(stdout, ":END PRINT TIMINGS\n");
        }
    }
#endif // ENABLE_PROFILER

    void BeginProfiling()
    {
        if (!bProfiling)
        {
            TotalBegin = ReadCPUTimer();
            bProfiling = true;
        }
    }

    void EndProfiling()
    {
        if (bProfiling)
        {
            TotalEnd = ReadCPUTimer();
            u64 CPUFreq = EstimateCPUFreq();
            f64 TotalTime = (f64)(TotalEnd - TotalBegin) / (f64)CPUFreq * 1000.0;
            fprintf(stdout, "\t[Total]: %.04f ms\n", TotalTime);
#if ENABLE_PROFILER
            PrintTimings(Entries);
#endif // ENABLE_PROFILER
            bProfiling = false;
        }
    }

}
