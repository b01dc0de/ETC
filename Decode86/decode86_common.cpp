#include "decode86_common.h"

FileContentsT ReadFileContents(const char* FileName)
{
    FileContentsT Result = {};
    Result.Name = FileName;

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        fseek(FileHandle, 0, SEEK_END);
        Result.Size = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (Result.Size > 0)
        {
            Result.Contents = new u8[Result.Size];
            fread_s(Result.Contents, Result.Size, Result.Size, 1, FileHandle);
        }

        fclose(FileHandle);
    }

    return Result;
}

