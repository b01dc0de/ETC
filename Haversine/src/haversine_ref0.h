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
    struct FileContentsT
    {
        int Size;
        u8* Data;

        void Release();
        void Read(const char* FileName, bool bAppendNull = false);
        void Write(const char* FileName);
    };

    f64 CalculateHaversine(HPair Pair);
    f64 CalculateAverage(HList List);
    HList GenerateDataUniform(int Seed, int Count);
    HList GenerateDataClustered(int Seed, int Count);
    void PrintData(HList List);
    void WriteDataAsBinary(HList List, const char* FileName);
    void WriteDataAsJSON(HList List, const char* FileName);
    HList ReadFileAsBinary(const char* FileName);
    HList ReadFileAsJSON(const char* FileName);
    HList ParseJSON(FileContentsT& InputFile);

    void DemoPipeline(int Seed, int Count, bool bClustered);
}

#include "haversine_ref0.inl"

#endif // HAVERSINE_REF0_H

