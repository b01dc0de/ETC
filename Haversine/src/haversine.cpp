#include "haversine_common.h"
#include "haversine_perf.h"
#include "haversine_ref0.h"

#ifndef UNITY_BUILD
#define UNITY_BUILD (0)
#endif // ifndef UNITY_BUILD

#if UNITY_BUILD
#include "haversine_perf.cpp"
#include "haversine_ref0.cpp"
#endif // UNITY_BUILD

constexpr int DefaultCount = 10000;
constexpr int DefaultSeed = 156208;

#include "haversine_cmdline.cpp"

int main(int ArgCount, const char* ArgValues[])
{
    MainExecParams ExecParams = ParseCmdLine(ArgCount, ArgValues);
    if (ExecParams.Type != MainExecType::Error)
    {
        PROFILING_BEGIN();
        Main_Exec(&ExecParams);
        PROFILING_END();
    }
    else
    {
        PrintProgramUsage(ArgValues[0]);
    }
}

#if ENABLE_PROFILER
static_assert(__COUNTER__ <= Perf::MaxAnchors, "Too many profile anchors!");
#endif // ENABLE_PROFILER
