#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void Outf(const char* Fmt, ...);
#define ASSERT(Exp) if (!(Exp)) { Outf("[error] ASSERT failed: %s\n", ##Exp); DebugBreak(); }

template <typename T>
struct DArray
{
    static constexpr u64 DefaultInitCapacity = 32;

    u64 Capacity;
    u64 Num;
    T* Data;

    void Init(u64 _Capacity = DefaultInitCapacity)
    {
        Capacity = _Capacity;
        Num = 0;
        Data = new T[Capacity];
    }
    void Term()
    {
        if (Data)
        {
            delete[] Data;
        }
    }
    void Resize(u64 NewCapacity)
    {
        ASSERT(NewCapacity > Capacity);
        T* OldData = Data;
        Capacity = NewCapacity;
        Data = new T[NewCapacity];
        memcpy_s(Data, sizeof(T) * Num, OldData, sizeof(T) * Num);
        delete[] OldData;
    }
    void Add(T Item)
    {
        if (Num + 1 > Capacity)
        {
            Resize(Capacity * 2);
        }
        Data[Num++] = Item;
    }
    T& AddGetRef()
    {
        if (Num + 1 > Capacity)
        {
            Resize(Capacity * 2);
        }
        Data[Num] = {};
        T& NewItem = Data[Num];
        Num++;
        return NewItem;
    }
    void RemoveLast()
    {
        if (Num)
        {
            Data[Num - 1] = {};
            Num--;
        }
    }

    T& operator[](u64 Idx)
    {
        ASSERT(Idx < Num);
        return Data[Idx];
    }
};


#endif // COMMON_H

