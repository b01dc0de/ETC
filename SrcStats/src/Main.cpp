#include "Common.h"

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

struct FileTreeT
{
    FileTreeT* Parent;
    char* BaseDirectory;
    DArray<char*> Files;
    DArray<FileTreeT*> Subdirs;
};

struct SrcStatsT
{
    int NumFiles;
    int NumLines;
};

struct FileContentsT
{
    u64 Size;
    u8* Data;
};

void LoadFileContents(FileContentsT* FileContents, const char* AbsoluteFilePath);
u64 CountLines(FileContentsT* FileContents);
void GetStats(FileTreeT* Tree, SrcStatsT* Stats, DArray<char*>* Dirs);
char* ConstructFullPath(DArray<char*>* Dirs, const char* File);
char* ConstructFullPathSearchQuery(FileTreeT* Tree);
void PopulateFileTree(FileTreeT* Tree, const char* BaseDirectory);
void PopulateSubdir(FileTreeT* Tree);
void PrintFullFileTree(FileTreeT* Tree);
void PrintSubdir(FileTreeT* Tree, DArray<s32>* IndexList);
void Outf(const char* Fmt, ...);
bool MatchStr(const char* A, const char* B);
void CopyFileName(char* Dst, const char* Src);

int main(int argc, const char* argv[])
{
    // TODO: Actually use argc, argv
    (void)argc;
    (void)argv;

    const char* DefaultTestingSrc = "W:/UBG/src";
    const char* SearchDirectory = DefaultTestingSrc;

    static constexpr bool bPrintFileTree = false;

    FileTreeT Tree = {};
    PopulateFileTree(&Tree, SearchDirectory);
    SrcStatsT Stats = {};
    GetStats(&Tree, &Stats, nullptr);
    if (bPrintFileTree)
    {
        PrintFullFileTree(&Tree);
    }
}

void LoadFileContents(FileContentsT* FileContents, const char* AbsoluteFilePath)
{
    ASSERT(FileContents);
    if (!FileContents) { return; }

    long Size = 0;
    u8* Data = nullptr;

    FILE* File = nullptr;
    fopen_s(&File, AbsoluteFilePath, "rb");

    if (File)
    {

        // Seek to end of file to get file size
        fseek(File, 0, SEEK_END);
        Size = ftell(File);

        if (Size)
        {
            u8* Data = new u8[Size];
            fread(Data, sizeof(u8), Size, File);
        }
        fclose(File);
    }
    FileContents->Size = Size;
    FileContents->Data = Data;
}

u64 CountLines(FileContentsT* FileContents)
{
    ASSERT(FileContents->Data && FileContents->Size);
    u64 Result = 0;
    for (u64 Idx = 0; Idx < FileContents->Size; Idx++)
    {
        // TODO: We probably want to handle the different OS new line formats here
        if (FileContents->Data[Idx] == '\n')
        {
            Result++;
        }
    }
    return Result;
}

void GetStats(FileTreeT* Tree, SrcStatsT* Stats, DArray<char*>* Dirs)
{
    if (!Dirs)
    {
        DArray<char*> RootDirs = {};
        RootDirs.Init();
        RootDirs.Add(Tree->BaseDirectory);
        GetStats(Tree, Stats, &RootDirs);
        RootDirs.Term();
    }
    else
    {
        Stats->NumFiles += Tree->Files.Num;
        for (u64 FileIdx = 0; FileIdx < Tree->Files.Num; FileIdx++)
        {
            // Construct absolute path for file
            char* FullPath = ConstructFullPath(Dirs, Tree->Files[FileIdx]);

            // Read file contents
            FileContentsT LoadedFile = {};
            LoadFileContents(&LoadedFile, FullPath);
            //ASSERT(LoadedFile.Data);

            // Count lines
            if (LoadedFile.Data)
            {
                Stats->NumLines += CountLines(&LoadedFile);
            }

            // Release file contents
            delete[] LoadedFile.Data;
        }
        u64 BeforeCount = Dirs->Num;
        if (Dirs->Num > 1) { Dirs->Add(Tree->BaseDirectory); }
        for (u64 SubdirIdx = 0; SubdirIdx < Tree->Subdirs.Num; SubdirIdx++)
        {
            GetStats(Tree->Subdirs[SubdirIdx], Stats, Dirs);
        }
        if (Dirs->Num > 1) { Dirs->RemoveLast(); }
        u64 AfterCount = Dirs->Num;
        ASSERT(BeforeCount == AfterCount);
    }
}

char* ConstructFullPath(DArray<char*>* Dirs, const char* File)
{
    char* Result = nullptr;
    if (Dirs && File)
    {
        Result = new char[MAX_PATH] {};
        int CharsWritten = 0;
        for (u64 DirIdx = 0; DirIdx < Dirs->Num; DirIdx++)
        {
            int WriteCount = sprintf_s(Result + CharsWritten, MAX_PATH - CharsWritten, "%s/", (*Dirs)[DirIdx]);
            ASSERT(WriteCount > 0);
            CharsWritten += WriteCount;
        }
        int WriteCount = sprintf_s(Result + CharsWritten, MAX_PATH - CharsWritten, "%s", File);
        ASSERT(WriteCount > 0);
        CharsWritten += WriteCount;
        ASSERT(CharsWritten < MAX_PATH);
    }
    return Result;
}

char* ConstructFullPathSearchQuery(FileTreeT* Tree)
{
    char* Result = nullptr;
    if (Tree)
    {
        DArray<char*> ReverseTree = {};
        ReverseTree.Init();
        FileTreeT* Cond = Tree;
        while (Cond)
        {
            ReverseTree.Add(Cond->BaseDirectory);
            Cond = Cond->Parent;
        }

        Result = new char[MAX_PATH] {};
        int CharsWritten = 0;
        for (s64 Idx = ReverseTree.Num - 1; Idx >= 0; Idx--)
        {
            int WriteCount = 0;
            if (!Idx)
            {
                WriteCount = sprintf_s(Result + CharsWritten, MAX_PATH - CharsWritten, "%s\\*", ReverseTree[Idx]);
            }
            else
            {
                WriteCount = sprintf_s(Result + CharsWritten, MAX_PATH - CharsWritten, "%s\\", ReverseTree[Idx]);
            }
            ASSERT(WriteCount > 0);
            CharsWritten += WriteCount;
        }
        ASSERT(CharsWritten < MAX_PATH);

        for (int Idx = 0; Idx < CharsWritten; Idx++)
        {
            if (Result[Idx] == '/') { Result[Idx] = '\\'; }
        }

        ReverseTree.Term();
    }
    return Result;
}

void PopulateFileTree(FileTreeT* Tree, const char* BaseDirectory)
{
    if (!Tree || !BaseDirectory) { return; }

    char SearchQuery[MAX_PATH] = {};
    s32 SearchQueryLength = sprintf_s(SearchQuery, MAX_PATH, "%s\\*", BaseDirectory);
    for (s32 Idx = 0; Idx < SearchQueryLength && SearchQuery[Idx]; Idx++)
    {
        if (SearchQuery[Idx] == '/') { SearchQuery[Idx] = '\\'; }
    }

    DArray<WIN32_FIND_DATAA> FileList = {};
    FileList.Init();

    bool bHasParent = Tree->Parent;

    Tree->Parent = nullptr;
    Tree->BaseDirectory = new char[MAX_PATH]{};
    CopyFileName(Tree->BaseDirectory, BaseDirectory);
    Tree->Files.Init();
    Tree->Subdirs.Init();

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
                NewSubdir->Parent = Tree;
                NewSubdir->BaseDirectory = new char[MAX_PATH];
                NewSubdir->Files.Init();
                NewSubdir->Subdirs.Init();
                CopyFileName(NewSubdir->BaseDirectory, FoundFile.cFileName);
                Tree->Subdirs.Add(NewSubdir);
                PopulateSubdir(NewSubdir);
            }
            else
            {
                char* NewFileName = new char[MAX_PATH];
                CopyFileName(NewFileName, FoundFile.cFileName);
                Tree->Files.Add(NewFileName);
            }
		}
		bDone = !FindNextFileA(SearchHandle, &FoundFile);
    }
    if (SearchHandle) { FindClose(SearchHandle); }
}

void PopulateSubdir(FileTreeT* Tree)
{
    char* FullPathSearchQuery = ConstructFullPathSearchQuery(Tree);
    if (!Tree || !FullPathSearchQuery) { return; }

    WIN32_FIND_DATAA FoundFile = {};
    HANDLE SearchHandle = FindFirstFileA(FullPathSearchQuery, &FoundFile);

    bool bDone = !SearchHandle;
    while (!bDone)
    {
        if (!MatchStr(".", FoundFile.cFileName) && !MatchStr("..", FoundFile.cFileName))
		{
            bool bIsDirectory = FoundFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (bIsDirectory)
            {
                FileTreeT* NewSubdir = new FileTreeT{};
                NewSubdir->Parent = Tree;
                NewSubdir->BaseDirectory = new char[MAX_PATH];
                NewSubdir->Files.Init();
                NewSubdir->Subdirs.Init();
                CopyFileName(NewSubdir->BaseDirectory, FoundFile.cFileName);
                Tree->Subdirs.Add(NewSubdir);
                PopulateSubdir(NewSubdir);
            }
            else
            {
                char* NewFileName = new char[MAX_PATH];
                CopyFileName(NewFileName, FoundFile.cFileName);
                Tree->Files.Add(NewFileName);
            }
		}
		bDone = !FindNextFileA(SearchHandle, &FoundFile);
    }
    if (SearchHandle) { FindClose(SearchHandle); }
    delete[] FullPathSearchQuery;
}

void PrintFullFileTree(FileTreeT* Tree)
{
    DArray<s32> IndexList = {};
    IndexList.Init();

    Outf("Found %d files in directory %s:\n", Tree->Files.Num, Tree->BaseDirectory);
    for (s32 Idx = 0; Idx < Tree->Files.Num; Idx++)
    {
        Outf("\t[%d]: %s\n", Idx, Tree->Files[Idx]);
    }
    Outf("Found %d subdirectories in directory %s:\n", Tree->Subdirs.Num, Tree->BaseDirectory);
    for (s32 Idx = 0; Idx < Tree->Subdirs.Num; Idx++)
    {
        FileTreeT* Subdir = Tree->Subdirs[Idx];
        Outf("\t[%d]: %s\n", Idx, Subdir->BaseDirectory);
        u64 BeforeCount = IndexList.Num;
        IndexList.Add(Idx);
        PrintSubdir(Subdir, &IndexList);
        IndexList.RemoveLast();
        u64 AfterCount = IndexList.Num;
        ASSERT(BeforeCount == AfterCount);
    }

    IndexList.Term();
}

void PrintSubdir(FileTreeT* Tree, DArray<s32>* IndexList)
{
    // TODO: Figure out the right way to print a variable amount of tabs
    //       Likely use '*' width-specifier?
    auto PrintTabs = [](int Num)
    {
        for (s32 TabIdx = 0; TabIdx < Num; TabIdx++) { Outf("\t"); }
    };

    ASSERT(IndexList->Num);
    if (Tree->Files.Num)
    {
        PrintTabs(IndexList->Num);
        Outf("  Files:\n");
        for (s32 Idx = 0; Idx < Tree->Files.Num; Idx++)
        {
            PrintTabs(IndexList->Num + 1);
            Outf("[%d]: %s\n", Idx, Tree->Files[Idx]);
        }
    }
    if (Tree->Subdirs.Num)
    {
        for (s32 Idx = 0; Idx < IndexList->Num; Idx++)
        {
            PrintTabs(IndexList->Num);
            Outf("  Subdirs:\n");
            for (s32 Idx = 0; Idx < Tree->Subdirs.Num; Idx++)
            {
                FileTreeT* Subdir = Tree->Subdirs[Idx];
                PrintTabs(IndexList->Num + 1);
                Outf("[%d]: %s\n", Idx, Subdir->BaseDirectory);
                u64 BeforeCount = IndexList->Num; // TODO: Remove after impl'd
                IndexList->Add(Idx);
                PrintSubdir(Subdir, IndexList);
                IndexList->RemoveLast();
                u64 AfterCount = IndexList->Num; // TODO: Remove after impl'd
                ASSERT(BeforeCount == AfterCount); // TODO: Remove after impl'd


            }
        }
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
    if (!A || !B) { return false; }
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
    return false; // TODO: Is this correct?
}

void CopyFileName(char* Dst, const char* Src)
{
    for (s32 Idx = 0; Idx < MAX_PATH; Idx++)
    {
        Dst[Idx] = Src[Idx];
        if (!Src[Idx]) { break; }
    }
}

