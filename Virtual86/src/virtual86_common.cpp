#include "virtual86_common.h"

FileContentsT ReadFileContents(const char* FileName)
{
    FileContentsT Result;

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
            //ASSERT(fread_s(Result.Data, Result.Size, Result.Size, 1, FileHandle) == Result.Size);
            fread_s(Result.Data, Result.Size, Result.Size, 1, FileHandle);
        }

        fclose(FileHandle);
    }

    return Result;
}
