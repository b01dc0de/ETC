#include "haversine_ref0.h"

constexpr float CoordinateMinF = -180.0f;
constexpr float CoordinateMaxF = +180.0f;

ListType Haversine_Ref0::GenerateDataBinary(int PairCount, int Seed)
{
    std::default_random_engine default_rand_engine(Seed);
    std::uniform_real_distribution<float> coordinate_dist(CoordinateMinF, CoordinateMaxF);

    ListType Result = {};
    Result.Count = PairCount;
    Result.Data = new HaversinePairData[PairCount];
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        Result.Data[PairIdx].X0 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = coordinate_dist(default_rand_engine);
    }

    fprintf(stdout, "Generated pair data - Count: %d, Seed: %d\n",
            PairCount, Seed);
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        fprintf(stdout, "    {X0:%f\tY0:%f\tX1:%f\tY1:%f}\n",
                Result.Data[PairIdx].X0,
                Result.Data[PairIdx].Y0,
                Result.Data[PairIdx].X1,
                Result.Data[PairIdx].Y1);
    }

    return Result;
}

void Haversine_Ref0::WriteDataAsJSON(ListType PairList, const char* OutputFileName)
{
    FILE* OutputFileHandle = nullptr;
    fopen_s(&OutputFileHandle, OutputFileName, "wt");

    if (OutputFileHandle)
    {
        fprintf(OutputFileHandle, "{\n");
        fprintf(OutputFileHandle, "    \"Pairs\": [\n");
        for (int PairIdx = 0; PairIdx < PairList.Count; PairIdx++)
        {
            fprintf(OutputFileHandle,
                    "        { \"X0\": %f, \"Y0\": %f, \"X1\": %f, \"Y1\": %f }%s\n",
                    PairList.Data[PairIdx].X0,
                    PairList.Data[PairIdx].Y0,
                    PairList.Data[PairIdx].X1,
                    PairList.Data[PairIdx].Y1,
                    PairIdx == PairList.Count - 1 ? " ]" : ",");
        }
        fprintf(OutputFileHandle, "}\n");
        fclose(OutputFileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Could not open file %s for write!\n", OutputFileName);
    }
}

