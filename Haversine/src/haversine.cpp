#include "haversine_common.h"
#include "haversine_ref0.h"

#ifndef UNITY_BUILD
#define UNITY_BUILD (0)
#endif // ifndef UNITY_BUILD

#if UNITY_BUILD
#include "haversine_ref0.cpp"
#endif // UNITY_BUILD

/*
 *  TODO:
 *      - [ ] Support random generation through clusters
 *      - [ ] Parse generated haversine input JSON
 */

int main(int ArgCount, const char* ArgValues[])
{
    constexpr int DefaultCount = 10000;
    constexpr int DefaultSeed = 156208;
    constexpr int DefaultClusterCount = 8;
    if (ArgCount == 2 && strcmp(ArgValues[1], "default") == 0)
    {
        constexpr int BufferSize = 64;
        char OutputFileName[BufferSize];

        HList PairList = Haversine_Ref0::GenerateDataClustered(DefaultCount, DefaultSeed, DefaultClusterCount);
        int OutputFileNameSize = sprintf_s(
                OutputFileName,
                "output_count%d_seed%d_clusters%d.json",
                DefaultCount,
                DefaultSeed,
                DefaultClusterCount);
        Haversine_Ref0::WriteDataAsJSON(PairList, OutputFileName);
    }
    else if (ArgCount < 4)
    {
        fprintf(stdout, "\tUsage: %s [PairCount] [Seed] [ClusterCount]\n",
                ArgValues[0]);
        fprintf(stdout, "\tExample: %s %d %d %d\n",
                ArgValues[0], DefaultCount, DefaultSeed, DefaultClusterCount);
    }
    else
    {
        int PairCount = strtol(ArgValues[1], nullptr, 10);
        int Seed = strtol(ArgValues[2], nullptr, 10);
        int ClusterCount = strtol(ArgValues[3], nullptr, 10);

        constexpr int BufferSize = 64;
        char OutputFileName[BufferSize];
        HList PairList = Haversine_Ref0::GenerateDataClustered(PairCount, Seed, ClusterCount);
        int OutputFileNameSize = sprintf_s(
                OutputFileName,
                "output_count%d_seed%d_clusters%d.json",
                DefaultCount,
                DefaultSeed,
                DefaultClusterCount);
        Haversine_Ref0::WriteDataAsJSON(PairList, OutputFileName);
    }
}

