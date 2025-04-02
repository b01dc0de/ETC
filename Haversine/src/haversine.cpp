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

/*
 *  TODO:
 *      - [ ] Parse generated haversine input JSON
 */

int main(int ArgCount, const char* ArgValues[])
{
    constexpr int DefaultCount = 10000;
    constexpr int DefaultSeed = 156208;

    PROFILING_BEGIN();

    constexpr int BufferSize = 64;
    if (ArgCount == 2 && strcmp(ArgValues[1], "defaultall") == 0)
    {
        Haversine_Ref0::DemoPipeline(DefaultSeed, DefaultCount, true);
        //Haversine_Ref0::DemoPipeline(DefaultSeed, DefaultCount, false);
    }
    /*
    else if (ArgCount == 2 && strcmp(ArgValues[1], "default") == 0)
    {
        char JSONFileName[BufferSize];
        char BinaryFileName[BufferSize];

        HList PairList = Haversine_Ref0::GenerateDataClustered(DefaultCount, DefaultSeed, DefaultClusterCount);
        (void)sprintf_s(JSONFileName, "output_count%d_seed%d_clusters%d.json",
                DefaultCount, DefaultSeed, DefaultClusterCount);
        (void)sprintf_s(BinaryFileName, "output_count%d_seed%d_clusters%d.f64",
                DefaultCount, DefaultSeed, DefaultClusterCount);
        Haversine_Ref0::WriteDataAsJSON(PairList, JSONFileName);
        Haversine_Ref0::WriteDataAsBinary(PairList, BinaryFileName);
        fprintf(stdout, "Wrote data to file %s\n", JSONFileName); 
        fprintf(stdout, "Wrote data to file %s\n", BinaryFileName); 
    }
    */
    else if (ArgCount < 3)
    {
        fprintf(stdout, "\tUsage: %s [PairCount] [Seed]\n",
                ArgValues[0]);
        fprintf(stdout, "\tExample: %s %d %d \n",
                ArgValues[0], DefaultCount, DefaultSeed);
    }
    else
    {
        int Count = strtol(ArgValues[1], nullptr, 10);
        int Seed = strtol(ArgValues[2], nullptr, 10);

        Haversine_Ref0::DemoPipeline(Seed, Count, true);
    }

    PROFILING_END();
}

#if ENABLE_PROFILER
static_assert(__COUNTER__ <= Perf::MaxAnchors, "Too many profile anchors!");
#endif // ENABLE_PROFILER
