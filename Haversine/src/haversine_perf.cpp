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
        constexpr u64 ms_to_wait = 1000;

        u64 OSFreq = GetOSFreq();

        u64 CPUStart = ReadCPUTimer();
        u64 OSBegin = ReadOSTimer();

        u64 OSEnd = 0;
        u64 OSElapsed = 0;
        u64 OSWaitTime = OSFreq * ms_to_wait / 1000;
        while (OSElapsed < OSWaitTime)
        {
            OSEnd = ReadOSTimer();
            OSElapsed = OSEnd - OSBegin;
        }

        u64 CPUEnd = ReadCPUTimer();
        u64 CPUElapsed = CPUEnd - CPUStart;
        u64 CPUFreq = 0;

        if (OSElapsed) { CPUFreq = OSFreq * CPUElapsed / OSElapsed; }
        return CPUFreq;
    }

    f64 GetElapsedTimeSeconds(u64 Delta, u64 Freq)
    {
        return (f64)Delta / (f64)Freq;
    }

    static u64 TotalBegin = 0;
    static u64 TotalEnd = 0;

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
        fprintf(stdout, "    %s[%llu]: ", Anchor->Name, Anchor->HitCount);
        constexpr bool bPrintMilliseconds = true;
        if (bPrintMilliseconds)
        {
            f64 ExclusiveMs = 1000.0 * (f64)Anchor->TimeElapsedExclusive / (f64)Freq;
            fprintf(stdout, "%0.4f ms ", ExclusiveMs);
        }
        else
        {
            fprintf(stdout, "%llu cycles ", Anchor->TimeElapsedExclusive);
        }

        f64 Percent = 100.0 * ((f64)Anchor->TimeElapsedExclusive / (f64)TotalTime);
        if (Anchor->TimeElapsedInclusive != Anchor->TimeElapsedExclusive)
        {
            f64 PercentWithChildren = 100.0 * ((f64)Anchor->TimeElapsedInclusive / (f64)TotalTime);
            fprintf(stdout, "(%.2f%%, %.2f%% w/children)", Percent, PercentWithChildren);
        }
        else
        {
            fprintf(stdout, "(%.2f%%)", Percent);
        }

        if (Anchor->BytesProcessed)
        {
            constexpr f64 Megabyte = 1024.0 * 1024.0;
            constexpr f64 Gigabyte = Megabyte * 1024.0;

            f64 Seconds = (f64)Anchor->TimeElapsedInclusive / (f64)Freq;
            f64 BytesPerSecond = (f64)Anchor->BytesProcessed / Seconds;
            f64 MegabytesProcessed = (f64)Anchor->BytesProcessed / (f64)Megabyte;
            f64 GigabytesProcessedPerSecond = BytesPerSecond / Gigabyte;

            f64 MilliSeconds = (f64)Anchor->TimeElapsedInclusive / (f64)Freq * 1000.0;
            fprintf(stdout, "    %.3fmb at %.2fgb/s\n", MegabytesProcessed, GigabytesProcessedPerSecond);
        }
        fprintf(stdout, "\n");
    }

#endif // ENABLE_PROFILER

    void BeginProfiling()
    {
        TotalBegin = ReadCPUTimer();
    }

    void EndProfiling()
    {
        TotalEnd = ReadCPUTimer();

        u64 CPUFreq = EstimateCPUFreq();
        u64 TotalTime = TotalEnd - TotalBegin;
        if (CPUFreq)
        {
            f64 CPUFreq_GHz = (f64)CPUFreq / (1000.0 * 1000.0 * 1000.0);
            fprintf(stdout, "\nTotal time: %0.4fms (CPU freq: %llu / ~%.2f GHz)\n", 1000.0 * (f64)TotalTime / (f64)CPUFreq, CPUFreq, CPUFreq_GHz);
        }
#if ENABLE_PROFILER
        for (int Idx = 0; Idx < MaxAnchors; Idx++)
        {
            ProfileAnchor* Anchor = Anchors + Idx;
            if (Anchor->TimeElapsedInclusive)
            {
                PrintAnchor(TotalTime, CPUFreq, Anchor);
            }
        }
#endif // ENABLE_PROFILER
    }

}
