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
 *      - [ ] Generate haversine input JSON
 *          - [x] Generate haversine data
 *          - [ ] Output data as JSON
 *      - [ ] Parse generated haversine input JSON
 *      - [ ] Implement reference haversine calculation
 */

int main(int ArgCount, const char* ArgValues[])
{
    if (ArgCount < 3)
    {
        fprintf(stdout, "\tUsage: %s [PairCount] [Seed]\n",
                ArgValues[0]);
        fprintf(stdout, "\tExample: %s 10000 156208\n",
                ArgValues[0]);
    }
    else
    {
        int PairCount = strtol(ArgValues[1], nullptr, 10);
        int Seed = strtol(ArgValues[2], nullptr, 10);
        constexpr int OutputFileNameBufferSize = 64;
        char OutputFileName[OutputFileNameBufferSize];
        ListType PairList = Haversine_Ref0::GenerateDataBinary(PairCount, Seed);
        int OutputFileNameSize = sprintf_s(
                OutputFileName,
                "output_count%d_seed%d.json",
                PairCount,
                Seed);
        Haversine_Ref0::WriteDataAsJSON(PairList, OutputFileName);
    }
}

