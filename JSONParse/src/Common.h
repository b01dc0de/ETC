#ifndef COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <crtdbg.h>
#include <Windows.h>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#define Assert(Exp) if (!(Exp)) { __debugbreak(); }

struct FileContentsT
{
    u64 Size;
    u8* Contents;
};

FileContentsT ReadFileContents(const char* FileName, bool bAppendZero);

template <typename T>
struct DynamicArray
{
    u64 Capacity;
    u64 Size;
    T* Data;

    static constexpr u64 DefaultCapacity = 16;

    DynamicArray()
    {
        Capacity = DefaultCapacity;
        Size = 0;
        Data = new T[Capacity];
    }

    ~DynamicArray()
    {
        if (Data) { delete[] Data; }
    }

    T& operator[](int Idx)
    {
        return Data[Idx];
    }

    void Grow()
    {
        u64 OldCapacity = Capacity;
        T* OldData = Data;

        Capacity = Capacity * 2;
        Data = new T[Capacity];
        memcpy(Data, OldData, sizeof(T) * Size);
        delete[] OldData;
    }

    void Add(T Item)
    {
        if (Size == Capacity) { Grow(); }

        Data[Size] = Item;
        Size++;
    }

    T& Last()
    {
        return Data[Size - 1];
    }
};

enum JSONType
{
    JSONType_Unspecified,
    JSONType_Object,
    JSONType_Array,
    JSONType_String,
    JSONType_NumberInt,
    JSONType_NumberFloat,
    JSONType_Boolean,
    JSONType_Null,
    JSONType_Error
};

struct JSONObject;

struct JSONValue
{
    JSONType Type;
    union
    {
        DynamicArray<JSONObject*>* List;
        char* String;
        s64 NumberInt;
        f64 NumberFloat;
        u64 Boolean;
    };
};

struct JSONObject
{
    char* Key = nullptr;
    JSONValue Value;

    JSONObject* GetProperty(const char* Key);
    JSONObject* GetItem(int Idx);

    DynamicArray<JSONObject*>& GetList();
    char*& GetString();
    s64& GetInt();
    f64& GetFloat();
    u64& GetBool();
    bool IsNull();
};

#endif // COMMON_H

