#include "haversine_common.h"
#include "haversine_ref0.h"
#include "haversine_ref0_json.h"

#ifndef UNITY_BUILD
#define UNITY_BUILD (0)
#endif // ifndef UNITY_BUILD

#if UNITY_BUILD
#include "haversine_ref0.cpp"
#include "haversine_ref0_json.cpp"
#endif // UNITY_BUILD

/*
 *  TODO:
 *      - [ ] Parse generated haversine input JSON
 */

int main(int ArgCount, const char* ArgValues[])
{
#define ENABLE_JSON_TEST() (1)
#if ENABLE_JSON_TEST()
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/null.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/single-pair.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/simple-collection.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/nested-example.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/single-array.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/nested-arrays.json");
    //(void)Haversine_Ref0::ReadFileAsJSON("input/test/stress-test.json");
    (void)Haversine_Ref0::ReadFileAsJSON("output_count10000_seed156208_clusters8.json");
    return 0;
#endif // ENABLE_JSON_TEST()

    constexpr int DefaultCount = 10000;
    constexpr int DefaultSeed = 156208;
    constexpr int DefaultClusterCount = 8;

    constexpr int BufferSize = 64;
    if (ArgCount == 2 && strcmp(ArgValues[1], "default") == 0)
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

