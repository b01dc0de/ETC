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

    ProfileEntry Total;
    int EntriesCount = 0;
    ProfileEntry Entries[MaxEntries] = {};

    void ScopedTiming::Start(const char *InName)
    {
        Name = InName;
        EntryIdx = EntriesCount++;
        Begin = ReadCPUTimer();
    }
    void ScopedTiming::Stop()
    {
        u64 End = ReadCPUTimer();

        ProfileEntry* WriteEntry = &Entries[EntryIdx];
        WriteEntry->Name = Name;
        ++WriteEntry->HitCount;
        WriteEntry->Time += End - Begin;
    }
    ScopedTiming::ScopedTiming(const char* InName) { Start(InName); }
    ScopedTiming::~ScopedTiming() { Stop(); }

    void PrintTimings(ProfileEntry* Entries, int NumEntries)
    {
        if (NumEntries >= MaxEntries)
        {
            fprintf(stdout, "[error] Too many timings (%d) for timing buffer size (%d)!\n",
                    NumEntries, MaxEntries);
        }
        else
        {
            u64 CPUFreq = EstimateCPUFreq();

            f64 TotalTime = (f64)Total.Time / (f64)CPUFreq * 1000.0;

            fprintf(stdout, "BEGIN PRINT TIMINGS:\n");
            fprintf(stdout, "\t%s : %.04f ms\n",
                    Total.Name,
                    TotalTime);
            for (int Idx = 0; Idx < NumEntries; Idx++)
            {
                f64 EntryTime = (f64)Entries[Idx].Time / (f64)CPUFreq * 1000.0f;
                f64 EntryPercent = (f64)EntryTime / TotalTime * 100.0;
                fprintf(stdout, "\t%s[%llu]: %.04f ms (%.02f%%)\n",
                        Entries[Idx].Name, Entries[Idx].HitCount,
                        EntryTime, EntryPercent);
            }
            fprintf(stdout, ":END PRINT TIMINGS\n");
        }
    }
}
