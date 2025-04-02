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

    template <typename T>
    struct DynamicArray
    {
        int Capacity;
        int Num;
        T* Data;

        static constexpr int DefaultCapacity = 32;
        static constexpr int GrowRate = 2;
        DynamicArray()
            : Capacity(DefaultCapacity)
            , Num(0)
            , Data(new T[DefaultCapacity])
        {
        }
        DynamicArray(int InitCapacity)
            : Capacity(InitCapacity)
            , Num(0)
            , Data(new T[DefaultCapacity])
        {
        }
        ~DynamicArray()
        {
            if (Data)
            {
                delete[] Data;
            }
        }
        int Resize(int NewCapacity)
        {
            if (NewCapacity > Capacity)
            {
                int OldCapacity = Capacity;
                T* OldData = Data;

                Capacity = NewCapacity;
                Data = new T[NewCapacity];
                (void)memcpy(Data, OldData, sizeof(T)*Num);

                delete[] OldData;
            }

            return Capacity;
        }
        int Add(T Item)
        {
            if (Num >= Capacity)
            {
                Resize(Capacity * GrowRate);
            }
            int NewItemIdx = Num;
            Data[Num++] = Item;
            return NewItemIdx;
        }
        T* Add_RetPtr(T Item)
        {
            int NewItemIdx = Add(Item);
            return &Data[NewItemIdx];
        }
        void RemoveLast()
        {
            // NOTE: We aren't doing _any_ cleanup here for now
            Num--;
        }
        T& operator[](int Idx)
        {
            return Data[Idx];
        }
        T& Last()
        {
            if (Num > 0) return Data[Num-1];
            else return Data[0];
        }
    };
}

#endif // HAVERSINE_REF0_H

