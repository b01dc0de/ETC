#ifndef HAVERSINE_REF0_H
#define HAVERSINE_REF0_H

/*
 * NOTE:
 *      These functions are supposed to be unchanged after they
 *      are implemented for the first time, and future iterations
 *      should be in separate files, this is in order to show
 *      progress related to performance/optimization from following
 *      Casey Muratori's Performance-Aware Programming course
 */

#include "haversine_common.h"

namespace Haversine_Ref0
{
    f64 CalculateHaversine(HPair Pair);
    f64 CalculateAverage(HList List);
    HList GenerateDataUniform(int PairCount, int Seed);
    HList GenerateDataClustered(int PairCount, int Seed, int ClusterCount);
    void PrintData(HList List);
    void WriteDataAsBinary(HList List, const char* FileName);
    void WriteDataAsJSON(HList List, const char* FileName);
    HList ReadFileAsBinary(const char* FileName);
    HList ReadFileAsJSON(const char* FileName);

    struct FileContentsT
    {
        int Size;
        u8* Data;

        void Release();
        void Read(const char* FileName, bool bAppendNull = false);
        void Write(const char* FileName);
    };
}

#include "haversine_ref0.inl"

#endif // HAVERSINE_REF0_H

