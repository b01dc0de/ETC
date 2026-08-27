#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

FileContentsT ReadFileContents(const char* FileName, bool bAppendZero = false)
{
    FileContentsT Result = {};

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        fseek(FileHandle, 0, SEEK_END);
        Result.Size = (u64)ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (Result.Size)
        {
            Result.Contents = new u8[Result.Size + (bAppendZero ? 1 : 0)];
            fread_s(Result.Contents, Result.Size, Result.Size, 1, FileHandle);
            if (bAppendZero) { Result.Contents[Result.Size - 1] = '\0'; }
        }

        fclose(FileHandle);
    }

    return Result;
}

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
};

int main()
{
    FileContentsT JSONText = ReadFileContents("test/example.json", true);

    return 0;
}

