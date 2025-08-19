#include "Common.h"

void Outf(const char* Fmt, ...);
bool MatchStr(const char* A, const char* B);
void CopyFileName(char* Dst, const char* Src);

template <typename T>
struct DArray
{
    u64 Capacity;
    u64 Num;
    T* Data;

    void Init(u64 _Capacity)
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

    T& operator[](u64 Idx)
    {
        ASSERT(Idx < Num);
        return Data[Idx];
    }
};

struct FileTreeT
{
    char* BaseDirectory;
    DArray<char*> Files;
    DArray<FileTreeT*> Subdirs;
};

int main(int argc, const char* argv[])
{
    // TODO: Actually use argc, argv
    (void)argc;
    (void)argv;

    const char* DefaultTestingSrc = "W:/UBG/src";
    const char* SearchDirectory = DefaultTestingSrc;

    char SearchQuery[MAX_PATH] = {};
    s32 SearchQueryLength = sprintf_s(SearchQuery, MAX_PATH, "%s\\*", SearchDirectory);
    for (s32 Idx = 0; Idx < SearchQueryLength && SearchQuery[Idx]; Idx++)
    {
        if (SearchQuery[Idx] == '/') { SearchQuery[Idx] = '\\'; }
    }

    static constexpr u64 DefaultDArrayCapacity = 32;
    DArray<WIN32_FIND_DATAA> FileList = {};
    FileList.Init(DefaultDArrayCapacity);

    FileTreeT FileTree = {};
    FileTree.BaseDirectory = new char[MAX_PATH]{};
    CopyFileName(FileTree.BaseDirectory, SearchDirectory);
    FileTree.Files.Init(DefaultDArrayCapacity);
    FileTree.Subdirs.Init(DefaultDArrayCapacity);

    WIN32_FIND_DATAA FoundFile = {};
    HANDLE SearchHandle = FindFirstFileA(SearchQuery, &FoundFile);

    bool bDone = !SearchHandle;
    while (!bDone)
    {
        if (!MatchStr(".", FoundFile.cFileName) && !MatchStr("..", FoundFile.cFileName))
		{
			FileList.Add(FoundFile);
            bool bIsDirectory = FoundFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (bIsDirectory)
            {
                FileTreeT* NewSubdir = new FileTreeT{};
                NewSubdir->BaseDirectory = new char[MAX_PATH];
                CopyFileName(NewSubdir->BaseDirectory, FoundFile.cFileName);
                FileTree.Subdirs.Add(NewSubdir);
            }
            else
            {
                char* NewFileName = new char[MAX_PATH];
                CopyFileName(NewFileName, FoundFile.cFileName);
                FileTree.Files.Add(NewFileName);
            }
		}
		bDone = !FindNextFileA(SearchHandle, &FoundFile);
    }
    if (SearchHandle) { FindClose(SearchHandle); }
    
    Outf("Found %d files in directory %s:\n", FileTree.Files.Num, SearchDirectory);
    for (s32 Idx = 0; Idx < FileTree.Files.Num; Idx++)
    {
        Outf("\t[%d]: %s\n", Idx, FileTree.Files[Idx]);
    }
    Outf("Found %d subdirectories in directory %s:\n", FileTree.Subdirs.Num, SearchDirectory);
    for (s32 Idx = 0; Idx < FileTree.Subdirs.Num; Idx++)
    {
        Outf("\t[%d]: %s\n", Idx, FileTree.Subdirs[Idx]->BaseDirectory);
    }
}

void Outf(const char* Fmt, ...)
{
    constexpr size_t BufferSize = 1024;
    char MsgBuffer[BufferSize] = {};
    va_list Args;
    va_start(Args, Fmt);
    vsprintf_s(MsgBuffer, BufferSize, Fmt, Args);
    va_end(Args);
    OutputDebugStringA(MsgBuffer);
}

bool MatchStr(const char* A, const char* B)
{
    for (size_t Idx = 0;; Idx++)
    {
        if (!A[Idx] && !B[Idx])
        {
            return true;
        }
        if (A[Idx] != B[Idx])
        {
            return false;
        }
    }
    return true; // TODO: Is this correct?
}

void CopyFileName(char* Dst, const char* Src)
{
    for (s32 Idx = 0; Idx < MAX_PATH; Idx++)
    {
        Dst[Idx] = Src[Idx];
        if (!Src[Idx]) { break; }
    }
}

