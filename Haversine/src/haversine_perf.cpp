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
    static constexpr int MaxAnchors = 4096;
    static ProfileAnchor Anchors[MaxAnchors] = {};
    static u32 GlobalParentIndex = 0;

    ScopedTiming::ScopedTiming(const char* Name_, u32 Index_, u64 Bytes_)
    {
        ParentIndex = GlobalParentIndex;

        Index = Index_;
        Name = Name_;

        ProfileAnchor* Anchor = Anchors + Index;
        OldTimeElapsedInclusive = Anchor->TimeElapsedInclusive;
        Anchor->BytesProcessed += Bytes_;

        GlobalParentIndex = Index;
        StartTime = ReadCPUTimer();
    }
    ScopedTiming::~ScopedTiming()
    {
        u64 TimeElapsed = ReadCPUTimer() - StartTime;
        GlobalParentIndex = ParentIndex;

        ProfileAnchor* Parent = Anchors + ParentIndex;
        ProfileAnchor* Anchor = Anchors + Index;

        Parent->TimeElapsedExclusive -= TimeElapsed;
        Anchor->TimeElapsedExclusive += TimeElapsed;
        Anchor->TimeElapsedInclusive = OldTimeElapsedInclusive + TimeElapsed;
        ++Anchor->HitCount;

        Anchor->Name = Name;
    }

    void PrintAnchor(u64 TotalTime, u64 Freq, ProfileAnchor* Anchor)
    {
        f64 Percent = 100.0 * ((f64)Anchor->TimeElapsedExclusive / (f64)TotalTime);
        printf("    %s[%llu]: %llu (%.2f%%", Anchor->Name, Anchor->HitCount, Anchor->TimeElapsedExclusive, Percent);
        if (Anchor->TimeElapsedInclusive != Anchor->TimeElapsedExclusive)
        {
            f64 PercentWithChildren = 100.0 * ((f64)Anchor->TimeElapsedInclusive / (f64)TotalTime);
            printf(", %.2f%% w/children", PercentWithChildren);
        }
        printf(")");

        if (Anchor->BytesProcessed)
        {
            constexpr f64 Megabyte = 1024.0 * 1024.0;
            constexpr f64 Gigabyte = Megabyte * 1024.0;

            f64 Seconds = (f64)Anchor->TimeElapsedInclusive / (f64)Freq;
            f64 BytesPerSecond = (f64)Anchor->BytesProcessed / Seconds;
            f64 MegabytesProcessed = (f64)Anchor->BytesProcessed / (f64)Megabyte;
            f64 GigabytesProcessedPerSecond = BytesPerSecond / Gigabyte;
            printf("    %.3fmb at %.2fgb/s\n", MegabytesProcessed, GigabytesProcessedPerSecond);
        }
        printf("\n");
    }

    void PrintTimings()
    {
        u64 CPUFreq = EstimateCPUFreq();

        u64 TotalTime = TotalEnd - TotalBegin;

        if (CPUFreq)
        {
            printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalTime / (f64)CPUFreq, CPUFreq);
        }

        for (int Idx = 0; Idx < MaxAnchors; Idx++)
        {
            ProfileAnchor* Anchor = Anchors + Idx;
            if (Anchor->TimeElapsedInclusive)
            {
                PrintAnchor(TotalTime, CPUFreq, Anchor);
            }
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
#if ENABLE_PROFILER
            PrintTimings();
#else
            u64 CPUFreq = EstimateCPUFreq();
            f64 TotalTime = (f64)(TotalEnd - TotalBegin) / (f64)CPUFreq * 1000.0;
            fprintf(stdout, "\t[Total]: %.04f ms\n", TotalTime);
#endif // ENABLE_PROFILER

            bProfiling = false;
        }
    }

}
