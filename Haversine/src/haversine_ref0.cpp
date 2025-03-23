#include "haversine_ref0.h"

constexpr double CoordinateMin = -180.0;
constexpr double CoordinateMax = +180.0;
constexpr double DegreesPerRadian = 0.01745329251994329577;
constexpr double EarthRadius = 6372.8;

namespace Ref0_Helpers
{
    // NOTE:
    //      These functions are taken directly from
    //      https://github.com/cmuratori/computer_enhance/blob/main/perfaware/part2/listing_0065_haversine_formula.cpp
    //      as a good baseline for the Haversine formula

    f64 Square(f64 A)
    {
        f64 Result = (A*A);
        return Result;
    }

    f64 RadiansFromDegrees(f64 Degrees)
    {
        f64 Result = DegreesPerRadian * Degrees;
        return Result;
    }

    f64 Haversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
    {
        f64 Lat1 = Y0;
        f64 Lat2 = Y1;
        f64 Lon1 = X0;
        f64 Lon2 = X1;

        f64 dLat = RadiansFromDegrees(Lat2 - Lat1);
        f64 dLon = RadiansFromDegrees(Lon2 - Lon1);
        Lat1 = RadiansFromDegrees(Lat1);
        Lat2 = RadiansFromDegrees(Lat2);

        f64 a = Square(sin(dLat/2.0)) + cos(Lat1)*cos(Lat2)*Square(sin(dLon/2));
        f64 c = 2.0*asin(sqrt(a));

        f64 Result = EarthRadius * c;
        return Result;
    }
}

namespace Haversine_Ref0
{
    struct FileContentsT
    {
        u8* Data;
        int Size;
        void Release();
        void Read(const char* FileName);
        void Write(const char* FileName);
    };

    void FileContentsT::Release()
    {
        if (Data)
        {
            delete[] Data;
            Data = nullptr;
        }
        Size = 0;
    }

    void FileContentsT::Read(const char* FileName)
    {
        if (Data) { Release(); }

        FILE* FileHandle = nullptr;
        fopen_s(&FileHandle, FileName, "rb");
        if (FileHandle)
        {
            fseek(FileHandle, 0, SEEK_END);
            long FileSize = ftell(FileHandle);
            fseek(FileHandle, 0, SEEK_SET);

            if (FileSize > 0)
            {
                Size = FileSize;
                Data = new u8[FileSize];
                fread_s(Data, Size, Size, 1, FileHandle);
            }
            fclose(FileHandle);
        }
        else
        {
            fprintf(stdout, "ERROR: Can't open file %s for read!\n", FileName);
        }
    }
    void FileContentsT::Write(const char* FileName)
    {
        if (!Data)
        {
            fprintf(stdout, "ERROR: File contents are empty during a call to write! (%s)\n", FileName);
            return;
        }

        FILE* FileHandle = nullptr;
        fopen_s(&FileHandle, FileName, "wb");
        if (FileHandle)
        {
            fwrite (Data, Size, 1, FileHandle);
            fclose(FileHandle);
        }
        else
        {
            fprintf(stdout, "ERROR: Can't open file %s for write!\n", FileName);
        }
    }
}

double Haversine_Ref0::CalculateHaversine(HPair Pair)
{
    return Ref0_Helpers::Haversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
}

double Haversine_Ref0::CalculateAverage(HList List)
{
    double Sum = 0.0f;
    for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
    {
        double fHaversine = CalculateHaversine(List.Data[PairIdx]);
        Sum += fHaversine;
    }
    double Average = Sum / List.Count;
    return Average;
}

using RandomEngineT = std::default_random_engine;
using UniformRealDistT = std::uniform_real_distribution<double>;
using UniformIntDistT = std::uniform_int_distribution<int>;

HList Haversine_Ref0::GenerateDataUniform(int PairCount, int Seed)
{
    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordinate_dist(CoordinateMin, CoordinateMax);

    HList Result = {PairCount, new HPair[PairCount]};
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        Result.Data[PairIdx].X0 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = coordinate_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = coordinate_dist(default_rand_engine);
    }

    fprintf(stdout, "Generated Pair data - Uniform - Count: %d, Seed: %d\n",
            PairCount, Seed);
    PrintData(Result);

    double Average = CalculateAverage(Result);
    printf("Average: %f\n", Average);

    return Result;
}

HList Haversine_Ref0::GenerateDataClustered(int PairCount, int Seed, int ClusterCount)
{
    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordinate_dist(CoordinateMin, CoordinateMax);

    HPair* Clusters = new HPair[ClusterCount];
    for (int ClusterIdx = 0; ClusterIdx < ClusterCount; ClusterIdx++)
    {
        Clusters[ClusterIdx].X0 = coordinate_dist(default_rand_engine);
        Clusters[ClusterIdx].Y0 = coordinate_dist(default_rand_engine);
        Clusters[ClusterIdx].X1 = coordinate_dist(default_rand_engine);
        Clusters[ClusterIdx].Y1 = coordinate_dist(default_rand_engine);
    }

    double MaxClusterOffset = CoordinateMax / (ClusterCount * 2);
    int MaxClusterIdx = ClusterCount - 1;
    UniformIntDistT clusteridx_dist(0, MaxClusterIdx);
    UniformRealDistT clusteroffset_dist(-MaxClusterOffset, +MaxClusterOffset);

    HList Result = {PairCount, new HPair[PairCount]};
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        int ClusterIdx = clusteridx_dist(default_rand_engine);
        Result.Data[PairIdx].X0 = Clusters[ClusterIdx].X0 + clusteroffset_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = Clusters[ClusterIdx].X0 + clusteroffset_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = Clusters[ClusterIdx].X1 + clusteroffset_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = Clusters[ClusterIdx].X1 + clusteroffset_dist(default_rand_engine);
    }

    fprintf(stdout, "Generated Pair data - Clustered - Count: %d, Seed: %d\n",
            PairCount, Seed);
    PrintData(Result);

    double Average = CalculateAverage(Result);
    printf("Average: %f\n", Average);

    return Result;
}

void Haversine_Ref0::PrintData(HList List)
{
    for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
    {
        fprintf(stdout, "    { X0:%.8f Y0:%.8f X1:%.8f Y1:%.8f }\n",
                List.Data[PairIdx].X0,
                List.Data[PairIdx].Y0,
                List.Data[PairIdx].X1,
                List.Data[PairIdx].Y1);
    }
}

void Haversine_Ref0::WriteDataAsBinary(HList List, const char* FileName)
{
    FILE* OutputFileHandle = nullptr;
    fopen_s(&OutputFileHandle, FileName, "wb");

    if (OutputFileHandle)
    {
        size_t WriteCount = fwrite(List.Data, sizeof(HPair), List.Count, OutputFileHandle);
        if (WriteCount != List.Count)
        {
            fprintf(stdout, "Warning: List has %d pairs but only %zd were written\n", List.Count, WriteCount);
        }

        fclose(OutputFileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Could not open file %s for write!\n", FileName);
    }
}

void Haversine_Ref0::WriteDataAsJSON(HList List, const char* FileName)
{
    FILE* OutputFileHandle = nullptr;
    fopen_s(&OutputFileHandle, FileName, "wt");

    if (OutputFileHandle)
    {
        fprintf(OutputFileHandle, "{\n");
        fprintf(OutputFileHandle, "    \"Pairs\": [\n");
        for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
        {
            fprintf(OutputFileHandle,
                    "        { \"X0\": %f, \"Y0\": %f, \"X1\": %f, \"Y1\": %f }%s\n",
                    List.Data[PairIdx].X0,
                    List.Data[PairIdx].Y0,
                    List.Data[PairIdx].X1,
                    List.Data[PairIdx].Y1,
                    PairIdx == List.Count - 1 ? " ]" : ",");
        }
        fprintf(OutputFileHandle, "}\n");
        fclose(OutputFileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Could not open file %s for write!\n", FileName);
    }
}

HList Haversine_Ref0::ReadFileAsBinary(const char* FileName)
{
    return {}; // TODO: implement
}
HList Haversine_Ref0::ReadFileAsJSON(const char* FileName)
{
    return {}; // TODO: implement
}
