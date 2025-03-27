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
 *      - [ ] Parse generated haversine input JSON
 */

int main(int ArgCount, const char* ArgValues[])
{
    constexpr int DefaultCount = 10000;
    constexpr int DefaultSeed = 156208;
    constexpr int DefaultClusterCount = 8;

    constexpr int BufferSize = 64;
    if (ArgCount == 2 && strcmp(ArgValues[1], "defaultall") == 0)
    {
        char JSONFileName[BufferSize];
        HList PairList = Haversine_Ref0::GenerateDataClustered(DefaultCount, DefaultSeed, DefaultClusterCount);
        (void)sprintf_s(JSONFileName, "output_count%d_seed%d_clusters%d.json",
                DefaultCount, DefaultSeed, DefaultClusterCount);
        Haversine_Ref0::WriteDataAsJSON(PairList, JSONFileName);
        fprintf(stdout, "Wrote data to file %s\n", JSONFileName); 

        HList ParsedPairs = Haversine_Ref0::ReadFileAsJSON(JSONFileName);
        fprintf(stdout, "Calculating Haversine Average on parsed JSON file (%s) list of size %d:\n", JSONFileName, ParsedPairs.Count);
        f64 HvAvg = Haversine_Ref0::CalculateAverage(ParsedPairs);
        fprintf(stdout, "\tAverage: %f\n", HvAvg);
    }
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

        char JSONFileName[BufferSize];
        char BinaryFileName[BufferSize];

        HList PairList = Haversine_Ref0::GenerateDataClustered(PairCount, Seed, ClusterCount);
        (void)sprintf_s(JSONFileName, "output_count%d_seed%d_clusters%d.json",
                PairCount, Seed, ClusterCount);
        (void)sprintf_s(BinaryFileName, "output_count%d_seed%d_clusters%d.f64",
                PairCount, Seed, ClusterCount);
        Haversine_Ref0::WriteDataAsJSON(PairList, JSONFileName);
        Haversine_Ref0::WriteDataAsBinary(PairList, BinaryFileName);
        fprintf(stdout, "Wrote data to file %s\n", JSONFileName); 
        fprintf(stdout, "Wrote data to file %s\n", BinaryFileName); 
    }
}

