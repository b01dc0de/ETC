#include "virtual86_common.h"

FileContentsT ReadFileContents(const char* FileName)
{
    FileContentsT Result = {};

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        Result.Name = FileName;

        fseek(FileHandle, 0, SEEK_END);
        Result.Size = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (Result.Size > 0)
        {
            Result.Data = new u8[Result.Size];
            ASSERT(fread_s(Result.Data, Result.Size, Result.Size, 1, FileHandle) == 1);
        }

        fclose(FileHandle);
    }

    return Result;
}

bool WriteFileContents(const char* FileName, FileContentsT& FileContents)
{
    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "wb");

    if (FileHandle)
    {
        ASSERT(FileContents.Data && FileContents.Size);
        ASSERT(fwrite(FileContents.Data, 1, FileContents.Size, FileHandle) == FileContents.Size);
        fclose(FileHandle);
    }

    return FileHandle != nullptr;
}

size_t ReadFileDirect(const char* FileName, u8* Dst, size_t BufferSize)
{
    size_t BytesWritten = 0;
    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");

    if (FileHandle)
    {
        size_t FileSize = 0;
        
        fseek(FileHandle, 0, SEEK_END);
        FileSize = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (FileSize > 0)
        {
            BytesWritten = fread_s(Dst, BufferSize, 1, FileSize, FileHandle);
        }

        fclose(FileHandle);
    }

    return BytesWritten;

}
