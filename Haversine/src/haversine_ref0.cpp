#include "haversine_ref0.h"

constexpr double CoordXMin = -180.0;
constexpr double CoordXMax = +180.0;
constexpr double CoordYMin = -90.0;
constexpr double CoordYMax = +90.0;
constexpr double DegreesPerRadian = 0.01745329251994329577;
constexpr double EarthRadius = 6372.8;

namespace Haversine_Ref0_Helpers
{
    // NOTE:
    //      These functions are taken directly from
    //      https://github.com/cmuratori/computer_enhance/blob/main/perfaware/part2/listing_0065_haversine_formula.cpp
    //      as a good baseline for the Haversine formula

    f64 Square(f64 A)
    {
        f64 Result = (A*A);
        return Result;
    }

    f64 RadiansFromDegrees(f64 Degrees)
    {
        f64 Result = DegreesPerRadian * Degrees;
        return Result;
    }

    f64 Haversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
    {
        f64 Lat1 = Y0;
        f64 Lat2 = Y1;
        f64 Lon1 = X0;
        f64 Lon2 = X1;

        f64 dLat = RadiansFromDegrees(Lat2 - Lat1);
        f64 dLon = RadiansFromDegrees(Lon2 - Lon1);
        Lat1 = RadiansFromDegrees(Lat1);
        Lat2 = RadiansFromDegrees(Lat2);

        f64 a = Square(sin(dLat/2.0)) + cos(Lat1)*cos(Lat2)*Square(sin(dLon/2));
        f64 c = 2.0*asin(sqrt(a));

        f64 Result = EarthRadius * c;
        return Result;
    }

    u64 GetOSTimer();
    u64 GetOSFreq();
    u64 GetCPUTimer();

    f64 GetElapsedTimeSeconds(u64 Delta, u64 Freq)
    {
        return (f64)Delta / (f64)Freq;
    }
}

#if _WIN32
#include <intrin.h>
#include <windows.h>
u64 Haversine_Ref0_Helpers::GetOSTimer()
{
    LARGE_INTEGER PerfCount;
    QueryPerformanceCounter(&PerfCount);
    return PerfCount.QuadPart;
}
u64 Haversine_Ref0_Helpers::GetOSFreq()
{
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}
#else // NOT _WIN32
#include <x86intrin.h>
#include <sys/time.h>
// TODO: These are untested currently until I build/run these on non-Windows platforms!!
u64 Haversine_Ref0_Helpers::GetOSTimer()
{
    timeval tval;
    gettimeofday(&tval, 0);
    u64 Result = GetOSTimerFreq()*(u64)tval.tv_sex + (u64)tval.tv_usec;
    return Result;
}
u64 Haversine_Ref0_Helpers::GetOSFreq() { return 1000000u; }
#endif // _WIN32
inline u64 Haversine_Ref0_Helpers::GetCPUTimer()
{
    return __rdtsc();
}

namespace Haversine_Ref0
{
    using namespace Haversine_Ref0_Helpers;

    using RandomEngineT = std::default_random_engine;
    using UniformRealDistT = std::uniform_real_distribution<double>;
    using UniformIntDistT = std::uniform_int_distribution<int>;
}

double Haversine_Ref0::CalculateHaversine(HPair Pair)
{
    return Haversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
}

double Haversine_Ref0::CalculateAverage(HList List)
{
    double Sum = 0.0f;
    for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
    {
        double fHaversine = CalculateHaversine(List.Data[PairIdx]);
        Sum += fHaversine;
    }
    double Average = Sum / List.Count;
    return Average;
}

HList Haversine_Ref0::GenerateDataUniform(int PairCount, int Seed)
{
    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordx_dist(CoordXMin, CoordXMax);
    UniformRealDistT coordy_dist(CoordYMin, CoordYMax);

    HList Result = {PairCount, new HPair[PairCount]};
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        Result.Data[PairIdx].X0 = coordx_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = coordy_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = coordx_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = coordy_dist(default_rand_engine);
    }

    fprintf(stdout, "Generated Pair data - Uniform - Count: %d, Seed: %d\n",
            PairCount, Seed);
    PrintData(Result);

    double Average = CalculateAverage(Result);
    printf("Average: %f\n", Average);

    return Result;
}

HList Haversine_Ref0::GenerateDataClustered(int PairCount, int Seed, int ClusterCount)
{
    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordx_dist(CoordXMin, CoordXMax);
    UniformRealDistT coordy_dist(CoordYMin, CoordYMax);

    HPair* Clusters = new HPair[ClusterCount];
    for (int ClusterIdx = 0; ClusterIdx < ClusterCount; ClusterIdx++)
    {
        Clusters[ClusterIdx].X0 = coordx_dist(default_rand_engine);
        Clusters[ClusterIdx].Y0 = coordy_dist(default_rand_engine);
        Clusters[ClusterIdx].X1 = coordx_dist(default_rand_engine);
        Clusters[ClusterIdx].Y1 = coordy_dist(default_rand_engine);
    }

    double MaxClusterXOffset = CoordXMax / (ClusterCount * 2);
    double MaxClusterYOffset = CoordYMax / (ClusterCount * 2);
    int MaxClusterIdx = ClusterCount - 1;
    UniformIntDistT clusteridx_dist(0, MaxClusterIdx);
    UniformRealDistT clusteroffsetx_dist(-MaxClusterXOffset, +MaxClusterXOffset);
    UniformRealDistT clusteroffsety_dist(-MaxClusterYOffset, +MaxClusterYOffset);

    HList Result = {PairCount, new HPair[PairCount]};
    for (int PairIdx = 0; PairIdx < PairCount; PairIdx++)
    {
        int ClusterIdx = clusteridx_dist(default_rand_engine);
        Result.Data[PairIdx].X0 = Clusters[ClusterIdx].X0 + clusteroffsetx_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = Clusters[ClusterIdx].Y0 + clusteroffsety_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = Clusters[ClusterIdx].X1 + clusteroffsetx_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = Clusters[ClusterIdx].Y1 + clusteroffsety_dist(default_rand_engine);
    }

    fprintf(stdout, "Generated Pair data - Clustered - Count: %d, Seed: %d\n",
            PairCount, Seed);
    constexpr bool bPrintGeneratedData = false;
    if (bPrintGeneratedData)
    {
        PrintData(Result);
    }

    double Average = CalculateAverage(Result);
    printf("Average: %f\n", Average);

    return Result;
}

void Haversine_Ref0::PrintData(HList List)
{
    for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
    {
        fprintf(stdout, "    { X0:%.8f Y0:%.8f X1:%.8f Y1:%.8f }\n",
                List.Data[PairIdx].X0,
                List.Data[PairIdx].Y0,
                List.Data[PairIdx].X1,
                List.Data[PairIdx].Y1);
    }
}

void Haversine_Ref0::WriteDataAsBinary(HList List, const char* FileName)
{
    FileContentsT OutputContents = { (int)sizeof(HPair)*List.Count, (u8*)List.Data };
    OutputContents.Write(FileName);
}

void Haversine_Ref0::WriteDataAsJSON(HList List, const char* FileName)
{
    FILE* OutputFileHandle = nullptr;
    fopen_s(&OutputFileHandle, FileName, "wt");

    if (OutputFileHandle)
    {
        fprintf(OutputFileHandle, "{\n");
        fprintf(OutputFileHandle, "    \"pairs\": [\n");
        for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
        {
            fprintf(OutputFileHandle,
                    "        { \"X0\": %f, \"Y0\": %f, \"X1\": %f, \"Y1\": %f }%s\n",
                    List.Data[PairIdx].X0,
                    List.Data[PairIdx].Y0,
                    List.Data[PairIdx].X1,
                    List.Data[PairIdx].Y1,
                    PairIdx == List.Count - 1 ? " ]" : ",");
        }
        fprintf(OutputFileHandle, "}\n");
        fclose(OutputFileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Could not open file %s for write!\n", FileName);
    }
}

HList Haversine_Ref0::ReadFileAsBinary(const char* FileName)
{
    FileContentsT InputFile = {};
    InputFile.Read(FileName);

    HList Result = {};

    if (InputFile.Data)
    {
        if (InputFile.Size % sizeof(HPair) == 0)
        {
            Result.Count = InputFile.Size / sizeof(HPair);
            Result.Data = (HPair*)InputFile.Data;
            InputFile = {};
        }
        else
        {
            fprintf(stdout, "ERROR: When reading file %s, file data not formatted correctly!\n", FileName);
            InputFile.Release();
        }
    }
    else
    {
        fprintf(stdout, "ERROR: Could not open file %s!\n", FileName);
    }
    return Result;
}

HList Haversine_Ref0::ReadFileAsJSON(const char* FileName)
{
    FileContentsT Input = {};
    Input.Read(FileName, true);
    HList Result = ParseJSON(Input);
    Input.Release();

    return Result;
}

void Haversine_Ref0::FileContentsT::Release()
{
    Size = 0;
    if (Data)
    {
        delete[] Data;
        Data = nullptr;
    }
}

void Haversine_Ref0::FileContentsT::Read(const char* FileName, bool bAppendNull)
{
    if (Data) { Release(); }

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        fseek(FileHandle, 0, SEEK_END);
        long FileSize = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        if (FileSize > 0)
        {
            Size = FileSize + (bAppendNull ? 1 : 0);
            Data = new u8[Size];
            fread_s(Data, FileSize, FileSize, 1, FileHandle);
            if (bAppendNull) { Data[Size - 1] = '\0'; }
        }
        fclose(FileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Can't open file %s for read!\n", FileName);
    }
}
void Haversine_Ref0::FileContentsT::Write(const char* FileName)
{
    if (!Data)
    {
        fprintf(stdout, "ERROR: File contents are empty during a call to write! (%s)\n", FileName);
        return;
    }

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "wb");
    if (FileHandle)
    {
        fwrite (Data, Size, 1, FileHandle);
        fclose(FileHandle);
    }
    else
    {
        fprintf(stdout, "ERROR: Can't open file %s for write!\n", FileName);
    }
}

namespace Haversine_Ref0
{
    // Forward type decls:
    struct JsonObject;

    enum JsonType
    {
        JsonType_Null,
        JsonType_NumberInt,
        JsonType_NumberFloat,
        JsonType_Bool,
        JsonType_String,
        JsonType_Object,
        JsonType_Array,
    };

    struct JsonValue
    {
        using b64 = u64;
        JsonType Type;
        union
        {
            s64 NumberInt;
            f64 NumberFloat;
            b64 Bool;
            char* String;
            DynamicArray<JsonObject*>* List;
        };
    };

    enum JsonToken
    {
        Token_LeftCurly,
        Token_RightCurly,
        Token_LeftSquare,
        Token_RightSquare,
        Token_Colon,
        Token_Comma,
        Token_String,
        Token_Number,
        Token_Null,
        Token_True,
        Token_False,
        Token_Error,
        Token_End
    };

    struct JsonObject
    {
        char* Key;
        JsonValue Value;
    };

    struct JsonTreeStack
    {
        int Depth;
        using ItemT = JsonObject*;
        using RefT = JsonObject&;
        using ListT = DynamicArray<JsonObject*>*;
        DynamicArray<ItemT> Stack;

        void Init(ItemT Root)
        {
            Stack.Add(Root);
            Depth = 0;
        }
        void Push(ItemT Item) { Stack.Add(Item); Depth++; }
        void Pop() { if (Depth > 0) { Stack.RemoveLast(); Depth--; } }
        RefT Top() { return *Stack[Depth]; }
        ListT TopList() { return Top().Value.List; }
        JsonType TopType() { return Top().Value.Type; }
        int GetDepth() { return Depth; }
        bool Root() { return Depth == 0; };
    };

    namespace JsonHelpers
    {
        bool CharIsNumeric(char C)
        {
            return ('0' <= C && C <= '9');
        }
        bool CharIsAlpha(char C)
        {
            return ('a' <= C && C <= 'z') ||
                ('A' <= C && C <= 'Z');
        }
        bool CharIsAlphanumeric(char C)
        {
            return CharIsAlpha(C) ||
                CharIsNumeric(C);
        }
        bool CharIsWhiteSpace(char C)
        {
            return C == ' ' ||
                C == '\n' ||
                C == '\r' ||
                C == '\t';
        }
        bool CharIsBeginNumber(char C)
        {
            return CharIsNumeric(C) ||
                C == '+' ||
                C == '-';
        }
        bool CharIsHexDigit(char C)
        {
            return CharIsNumeric(C) ||
                ('A' <= C && C <= 'F') ||
                ('a' <= C && C <= 'f');
        }
        int IsValidStringValue(char *C)
        {
            if (C[0] == '"') { return 0; }
            if (C[0] == '\\')
            {
                switch (C[1])
                {
                    case '"':
                    case '\\':
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                        {
                            return 2;
                        } break;
                    case 'u':
                        {
                            if (CharIsHexDigit(C[2]) &&
                                    CharIsHexDigit(C[3]) &&
                                    CharIsHexDigit(C[4]) &&
                                    CharIsHexDigit(C[5]))
                            {
                                return 6;
                            }
                        } break;
                }
                return 0;
            }
            // TODO: Need to exempt control characters here
            return 1;
        }
        bool TryMatchLiteral(char* Begin, const char* Literal)
        {
            bool bMatch = true;
            int ReadIdx = 0;
            while (bMatch)
            {
                if (0 == Literal[ReadIdx]) { break; } // Match!
                else if (0 == Begin[ReadIdx]) { bMatch = false; }
                else if (Literal[ReadIdx] != Begin[ReadIdx]) { bMatch = false; }
                ReadIdx++;
            }
            return bMatch;
        }
        bool TryReadNumber(char* Begin, char** End, JsonValue* ParsedNumber)
        {
            if (!Begin || !End || !ParsedNumber) { return false; }

            JsonType NumberType = JsonType_NumberInt;
            int ReadIdx = 0;
            bool bReadPeriod = false;
            bool bReadSign = false;
            while (CharIsNumeric(Begin[ReadIdx]) ||
                    Begin[ReadIdx] == '.' ||
                    Begin[ReadIdx] == '-')
            {
                if (Begin[ReadIdx] == '.')
                {
                    // Multiple periods in literal => Error
                    if (bReadPeriod) { return false; }
                    else { bReadPeriod = true; NumberType = JsonType_NumberFloat; }
                }
                else if (Begin[ReadIdx] == '-')
                {
                    // Multiple signs or sign after period => Error
                    if (bReadSign || bReadPeriod) { return false; }
                    else { bReadSign = true; }
                }
                ReadIdx++;
            }

            ParsedNumber->Type = NumberType;
            char* FirstCharAfterNumber = 0;
            if (JsonType_NumberInt == NumberType)
            {
                ParsedNumber->NumberInt = strtol(Begin, &FirstCharAfterNumber, 10);
            }
            else if (JsonType_NumberFloat == NumberType)
            {
                ParsedNumber->NumberFloat = strtod(Begin, &FirstCharAfterNumber);
            }
            if (!FirstCharAfterNumber) { return false; }
            *End = FirstCharAfterNumber;
            return true;
        }
        bool TryReadString(char* Begin, char** CharAfterCloseQuote, char** ParsedString)
        {
            if (Begin[0] != '"' || !CharAfterCloseQuote || !ParsedString) { return false; }
            int ReadIdx = 1;
            int NumReadChars = 0;
            while (NumReadChars = IsValidStringValue(Begin+ReadIdx))
            {
                ReadIdx += NumReadChars;
            }
            if (ReadIdx > 1)
            {
                if (Begin[ReadIdx] == '"')
                {
                    *ParsedString = new char[ReadIdx];
                    memcpy(*ParsedString, Begin + 1, ReadIdx - 1);
                    (*ParsedString)[ReadIdx-1] = '\0';
                }
                else
                {
                    return false;
                }
            }
            else
            {
                *ParsedString = nullptr;
            }
            if (CharAfterCloseQuote) { *CharAfterCloseQuote = Begin + ReadIdx + 1; }
            return true;
        }
    } // namespace JsonHelpers
    using namespace JsonHelpers;

#define DEF_LITERAL_VAL(lit_val) \
        static constexpr const char* Literal_##lit_val = #lit_val; \
        static constexpr const int Literal_##lit_val##_Length = sizeof(#lit_val)-1;
#define LITERAL_VAL(lit_val) (Literal_##lit_val)
#define LITERAL_VAL_LENGTH(lit_val) (Literal_##lit_val##_Length)

    DEF_LITERAL_VAL(null);
    DEF_LITERAL_VAL(true);
    DEF_LITERAL_VAL(false);

    void DebugPrintStep(char* JsonData, JsonToken InToken, JsonValue InValue, int BeginIdx, int EndIdx);

    JsonToken ParseNextToken(char* Begin, JsonValue* OutValue, char** NextTokenBegin);
    JsonObject Parse(char* JsonData, int Size);

    JsonObject* Query(JsonObject* Root, const char* Key);
    HList ParsePairsArray(JsonObject* Pairs);
}

Haversine_Ref0::JsonToken Haversine_Ref0::ParseNextToken(char* Begin, JsonValue* OutValue, char** NextTokenBegin)
{
    if (!Begin || !OutValue || !NextTokenBegin) { return Token_Error; }
    JsonToken Result = Token_Error;
    int ReadIdx = 0;
    while (Begin[ReadIdx])
    {
        if (CharIsWhiteSpace(Begin[ReadIdx]))
        {
            ReadIdx++;
            continue;
        }
        switch (Begin[ReadIdx])
        {
            case '{': *NextTokenBegin = Begin + ReadIdx + 1; return Token_LeftCurly;
            case '}': *NextTokenBegin = Begin + ReadIdx + 1; return Token_RightCurly;
            case '[': *NextTokenBegin = Begin + ReadIdx + 1; return Token_LeftSquare;
            case ']': *NextTokenBegin = Begin + ReadIdx + 1; return Token_RightSquare;
            case ':': *NextTokenBegin = Begin + ReadIdx + 1; return Token_Colon;
            case ',': *NextTokenBegin = Begin + ReadIdx + 1; return Token_Comma;
            case 'f':
            {
                if (TryMatchLiteral(Begin + ReadIdx, LITERAL_VAL(false)))
                {
                    OutValue->Type = JsonType_Bool;
                    OutValue->Bool = false;
                    *NextTokenBegin = Begin + ReadIdx + LITERAL_VAL_LENGTH(false);
                    return Token_False;
                }
                else { return Token_Error; }

            } break;
            case 't':
            {
                if (TryMatchLiteral(Begin + ReadIdx, LITERAL_VAL(true)))
                {
                    OutValue->Type = JsonType_Bool;
                    OutValue->Bool = true;
                    *NextTokenBegin = Begin + ReadIdx + LITERAL_VAL_LENGTH(true);
                    return Token_True;
                }
                else { return Token_Error; }
            } break;
            case 'n':
            {
                if (TryMatchLiteral(Begin + ReadIdx, LITERAL_VAL(null)))
                {
                    OutValue->Type = JsonType_Null;
                    *NextTokenBegin = Begin + ReadIdx + LITERAL_VAL_LENGTH(null);
                    return Token_Null;
                }
                else { return Token_Error; }
            } break;
            case '"':
            {
                char* CharAfterStringEnd = nullptr;
                if (TryReadString(Begin + ReadIdx, &CharAfterStringEnd, &OutValue->String))  
                {
                    OutValue->Type = JsonType_String;
                    *NextTokenBegin = CharAfterStringEnd;
                    return Token_String;
                }
                else
                {
                    return Token_Error; // Not valid string
                }
            } break;
        }
        if (CharIsBeginNumber(Begin[ReadIdx]))
        {
            char* CharAfterNumberEnd = nullptr;
            if (TryReadNumber(Begin + ReadIdx, &CharAfterNumberEnd, OutValue))
            {
                *NextTokenBegin = CharAfterNumberEnd;
                return Token_Number;
            }
            else
            {
                return Token_Error;
            }
        }
    }
    if (Begin[ReadIdx] == '\0') { *NextTokenBegin = nullptr; return Token_End; }
    return Result;
}

void Haversine_Ref0::DebugPrintStep(char* JsonData, Haversine_Ref0::JsonToken InToken, Haversine_Ref0::JsonValue InValue, int BeginIdx, int EndIdx)
{
    static constexpr const char* TokenNamesTable[] = 
    {
        "LeftCurly", "RightCurly",
        "LeftBrace", "RightBrace",
        "Colon", "Comma",
        "String", "Number",
        "Null", "True", "False",
        "Error",
        "End"
    };

    if (InToken == Token_Error)
    {
        fprintf(stdout, "[json][token] ERROR encountered at Idx %d!\n", BeginIdx);
    }
    else
    {
        fprintf(stdout, "[json][token] Read next token: %s", TokenNamesTable[InToken]);
        if (InToken == Token_String) { fprintf(stdout, ": \"%s\"\n", InValue.String); }
        else if (InToken == Token_Number && InValue.Type == JsonType_NumberInt) { fprintf(stdout, ": %lld\n", InValue.NumberInt); }
        else if (InToken == Token_Number && InValue.Type == JsonType_NumberFloat) { fprintf(stdout, ": %f\n", InValue.NumberFloat); }
        else { fprintf(stdout, "\n"); }
        fprintf(stdout, "\n\tBeginIdx: %d\tEndIdx:%d\n", BeginIdx, EndIdx);
    }
}

Haversine_Ref0::JsonObject Haversine_Ref0::Parse(char* JsonData, int Size)
{
    JsonObject Root = {};
    Root.Value.Type = JsonType_Object;
    Root.Value.List = new DynamicArray<JsonObject*>{};

    JsonTreeStack Stack;
    bool bStackInit = false;

    bool bEnd = false;
    bool bError = false;
    int ReadIdx = 0;

    bool bAfterComma = false;
    bool bAfterColon = false;
    while (!bError && !bEnd && ReadIdx < Size)
    {
        int BeginIdx = ReadIdx;
        if (bStackInit && (Stack.TopType() != JsonType_Object) &&
                (Stack.TopType() != JsonType_Array))
        {
            bError = true;
            break;
        }
        char* NextTokenBegin = nullptr;
        JsonValue OutValue = {};
        JsonToken CurrToken = ParseNextToken(JsonData + ReadIdx, &OutValue, &NextTokenBegin);
        switch (CurrToken)
        {
            case Token_LeftCurly:
            {
                if (!bStackInit) { Stack.Init(&Root); bStackInit = true; }
                else if (Stack.TopType() == JsonType_Object && bAfterColon)
                {
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>{};
                    auto LastVal = Stack.TopList()->Last();
                    Stack.TopList()->Last()->Value.Type = JsonType_Object;
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else if (Stack.TopType() == JsonType_Array && 
                        (bAfterComma || Stack.TopList()->Num == 0))
                {
                    Stack.TopList()->Add(new JsonObject{});
                    Stack.TopList()->Last()->Key = nullptr;
                    Stack.TopList()->Last()->Value.Type = JsonType_Object;
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>{};
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else { bError = true; }
            } break;
            case Token_RightCurly:
            {
                if (Stack.TopType() == JsonType_Object)
                {
                    if (Stack.Root()) { bEnd = true; }
                    else { Stack.Pop(); }
                }
                else { bError = true; }
            } break;
            case Token_LeftSquare:
            {
                if (Stack.TopType() == JsonType_Object && bAfterColon)
                {
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>{};
                    Stack.TopList()->Last()->Value.Type = JsonType_Array;
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else if (Stack.TopType() == JsonType_Array && 
                        (bAfterComma || Stack.TopList()->Num == 0))
                {
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>{};
                    Stack.TopList()->Add(new JsonObject{});
                    Stack.TopList()->Last()->Value.Type = JsonType_Array;
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else { bError = true; }
            } break;
            case Token_RightSquare:
            {
                if (Stack.TopType() == JsonType_Array) { Stack.Pop(); }
                else { bError = true; }
            } break;
            case Token_Colon:
            case Token_Comma: { if (bAfterColon || bAfterComma) { bError = true; } } break;
            case Token_String:
            case Token_Number:
            case Token_Null:
            case Token_True:
            case Token_False:
            {
                bool bKey = false;
                JsonObject* ObjToSetValue = nullptr;
                if (Stack.TopType() == JsonType_Array)
                {
                    if (Stack.TopList()->Num == 0 || bAfterComma)
                    {
                        ObjToSetValue = *Stack.TopList()->Add_RetPtr(new JsonObject{});
                    }
                    else { bError = true; }
                }
                else if (Stack.TopType() == JsonType_Object)
                {
                    if (bAfterColon)
                    {
                        ObjToSetValue = Stack.TopList()->Last();
                    }
                    else if ((CurrToken == Token_String &&
                            OutValue.Type == JsonType_String) &&
                            (Stack.TopList()->Num == 0 || bAfterComma))
                    {
                        JsonObject* NewObj_Key = *Stack.TopList()->Add_RetPtr(new JsonObject{});
                        NewObj_Key->Key = OutValue.String;
                        bKey = true;
                    }
                    else { bError = true; }
                }
                if (ObjToSetValue)
                {
                    switch (CurrToken)
                    {
                        case Token_String:
                        {
                            ObjToSetValue->Value.Type = JsonType_String;
                            ObjToSetValue->Value.String = OutValue.String;
                        } break;
                        case Token_Number:
                        {
                            if (OutValue.Type == JsonType_NumberInt)
                            {
                                ObjToSetValue->Value.Type = JsonType_NumberInt;
                                ObjToSetValue->Value.NumberInt = OutValue.NumberInt;
                            }
                            else if (OutValue.Type == JsonType_NumberFloat)
                            {
                                ObjToSetValue->Value.Type = JsonType_NumberFloat;
                                ObjToSetValue->Value.NumberFloat = OutValue.NumberFloat;
                            }
                            else { bError = true; }
                        } break;
                        case Token_Null:
                        {
                            ObjToSetValue->Value.Type = JsonType_Null;
                        } break;
                        case Token_True:
                        {
                            ObjToSetValue->Value.Type = JsonType_Bool;
                            ObjToSetValue->Value.Bool = true;
                        } break;
                        case Token_False:
                        {
                            ObjToSetValue->Value.Type = JsonType_Bool;
                            ObjToSetValue->Value.Bool = false;
                        } break;
                        default: { bError = true; } break;
                    }
                }
                if (!bKey && !ObjToSetValue) { bError = true; }
            } break;
            case Token_Error: { bError = true; } break;
            case Token_End: { bEnd = true; } break;
        }
        if (!(CurrToken == Token_Error || CurrToken == Token_End) && NextTokenBegin)
        {
            ReadIdx = NextTokenBegin - JsonData;
        }
        bAfterComma = CurrToken == Token_Comma;
        bAfterColon = CurrToken == Token_Colon;
        constexpr bool bDebugPrint = false;
        if (bDebugPrint)
        {
            DebugPrintStep(JsonData, CurrToken, OutValue, BeginIdx, ReadIdx);
        }
    }
    return Root;
}

Haversine_Ref0::JsonObject* Haversine_Ref0::Query(JsonObject* Root, const char* Key)
{
    if (!Root) { return nullptr; }

    JsonObject* Result = nullptr;

    for (int RootObjIdx = 0; RootObjIdx < Root->Value.List->Num; RootObjIdx++)
    {
        JsonObject* CurrObject = (*Root->Value.List)[RootObjIdx];
        if (strcmp(CurrObject->Key, Key) == 0)
        {
            Result = CurrObject;
            break;
        }
        else if (CurrObject->Value.Type == JsonType_Object ||
                CurrObject->Value.Type == JsonType_Array)
        {
            JsonObject* QueryResult = Query(CurrObject, Key);
            if (QueryResult)
            {
                Result = QueryResult;
                break;
            }
        }
    }

    return Result;
}

HList Haversine_Ref0::ParsePairsArray(JsonObject* Pairs)
{
    if (!Pairs || !Pairs->Key || strcmp(Pairs->Key, "pairs") != 0) { return HList{}; }
    if (Pairs->Value.Type != JsonType_Array || !Pairs->Value.List || Pairs->Value.List->Num == 0)
    { return HList{}; }

    int ListSize = Pairs->Value.List->Num;
    DynamicArray<JsonObject*>* JsonPairArray = Pairs->Value.List;
    HList Result = { ListSize, new CoordPair[ListSize] };

    int PairIdx = 0;
    bool bError = false;
    while (PairIdx < Pairs->Value.List->Num)
    {
        JsonObject* CurrPair = (*JsonPairArray)[PairIdx];
        if (!CurrPair) { bError = true; break; }
        else
        {
            JsonObject* JsonX0 = Query(CurrPair, "X0");
            JsonObject* JsonY0 = Query(CurrPair, "Y0");
            JsonObject* JsonX1 = Query(CurrPair, "X1");
            JsonObject* JsonY1 = Query(CurrPair, "Y1");
            if (JsonX0 && JsonX0->Value.Type == JsonType_NumberFloat &&
                    JsonY0 && JsonY0->Value.Type == JsonType_NumberFloat &&
                    JsonX1 && JsonX1->Value.Type == JsonType_NumberFloat &&
                    JsonY1 && JsonY1->Value.Type == JsonType_NumberFloat)
            {
                Result.Data[PairIdx].X0 = JsonX0->Value.NumberFloat;
                Result.Data[PairIdx].Y0 = JsonY0->Value.NumberFloat;
                Result.Data[PairIdx].X1 = JsonX1->Value.NumberFloat;
                Result.Data[PairIdx].Y1 = JsonY1->Value.NumberFloat;
            }
            else { bError = true; break; }
            PairIdx++;
        }
    }
    if (bError) { fprintf(stdout, "ERROR encounted at Idx: %d (Size: %d) in ParsePairsArray\n",
            PairIdx, ListSize); delete Result.Data; return HList{}; }
    return Result;
}

HList Haversine_Ref0::ParseJSON(FileContentsT& InputFile)
{
    HList Result = {};
    if (nullptr == InputFile.Data) { return Result; }

    //fprintf(stdout, "PARSE BEGIN:\n");
    JsonObject Root = Haversine_Ref0::Parse((char*)InputFile.Data, InputFile.Size);
    //fprintf(stdout, "PARSE END\n");
    JsonObject* Pairs = Query(&Root, "pairs");
    if (Pairs)
    {
        Result = ParsePairsArray(Pairs);
    }
    return Result;
}

void Haversine_Ref0::DemoPipeline(int Seed, int Count, bool bClustered)
{
    using namespace Haversine_Ref0_Helpers;

    static constexpr int FileNameMaxSize = 64;
    static constexpr int DefaultClusterCount = 8;

    u64 OS_Freq = GetOSFreq();
    u64 OS_Timer0 = GetOSTimer();
    u64 CPU_Timer0 = GetCPUTimer();

    u64 Debug_OSFreq = GetOSFreq();
    u64 Debug_CPUStart = GetCPUTimer();
    u64 Debug_OSStart = GetOSTimer();

    u64 OS_Timer1 = 0u, CPU_Timer1 = 0u,
        OS_Timer2 = 0u, CPU_Timer2 = 0u,
        OS_Timer3 = 0u, CPU_Timer3 = 0u,
        OS_Timer4 = 0u, CPU_Timer4 = 0u,
        OS_Timer5 = 0u, CPU_Timer5 = 0u,
        OS_Timer6 = 0u, CPU_Timer6 = 0u;

    char JSONFileName[FileNameMaxSize];
    HList PairList = {};

    OS_Timer1 = GetOSTimer();
    CPU_Timer1 = GetCPUTimer();

    if (bClustered)
    {
        PairList = Haversine_Ref0::GenerateDataClustered(Count, Count, DefaultClusterCount);
        (void)sprintf_s(JSONFileName, "output_seed%d_count%d_clusters%d.json",
                Seed, Count, DefaultClusterCount);
    }
    else
    {
        PairList = Haversine_Ref0::GenerateDataUniform(Count, Seed);
        (void)sprintf_s(JSONFileName, "output_seed%d_count%d_u.json",
                Seed, Count);
    }

    OS_Timer2 = GetOSTimer();
    CPU_Timer2 = GetCPUTimer();

    Haversine_Ref0::WriteDataAsJSON(PairList, JSONFileName);
    fprintf(stdout, "Wrote data to file %s\n", JSONFileName); 

    OS_Timer3 = GetOSTimer();
    CPU_Timer3 = GetCPUTimer();

    HList ParsedPairs = Haversine_Ref0::ReadFileAsJSON(JSONFileName);
    fprintf(stdout, "Calculating Haversine Average on parsed JSON file (%s) list of size %d:\n", JSONFileName, ParsedPairs.Count);

    OS_Timer4 = GetOSTimer();
    CPU_Timer4 = GetCPUTimer();

    f64 HvAvg = Haversine_Ref0::CalculateAverage(ParsedPairs);
    fprintf(stdout, "\tAverage: %f\n", HvAvg);

    OS_Timer5 = GetOSTimer();
    CPU_Timer5 = GetCPUTimer();

    delete[] PairList.Data;
    delete[] ParsedPairs.Data;

    OS_Timer6 = GetOSTimer();
    CPU_Timer6 = GetCPUTimer();


    // NOTE: Following block is imported from perfaware/part2/listing_0073
    {
        u64 Debug_OSEnd = GetOSTimer();
        u64 Debug_OSElapsed = Debug_OSEnd - Debug_OSStart;
        u64 Debug_CPUEnd = GetCPUTimer();
        u64 Debug_CPUElapsed = Debug_CPUEnd - Debug_CPUStart;
        u64 Debug_CPUFreq = 0;
        if (Debug_OSElapsed) { Debug_CPUFreq = Debug_OSFreq * Debug_CPUElapsed / Debug_OSElapsed; }

        printf("[debug][ref] OS Timer: %llu -> %llu = %llu elapsed\n", Debug_OSStart, Debug_OSEnd, Debug_OSElapsed);
        printf("[debug][ref] OS Seconds: %.4f\n", (f64)Debug_OSElapsed/(f64)Debug_OSFreq);
        printf("[debug][ref] CPU Timer: %llu -> %llu = %llu elapsed\n", Debug_CPUStart, Debug_CPUEnd, Debug_CPUElapsed);
        printf("[debug][ref] CPU Freq: %llu (guessed)\n\n", Debug_CPUFreq);
        printf("[debug][ref] CPU Seconds: %.4f (guessed)\n\n", (f64)Debug_CPUElapsed / (f64)Debug_CPUFreq);
    }

    auto PrintDebugPerfTimeStep = [](u64 TimeEnd, u64 TimeBegin, f64 TotalSeconds, u64 Freq, const char* StepName)
    {
        u64 Delta = TimeEnd - TimeBegin;
        f64 TimeSeconds = (f64)Delta / (f64)Freq;
        fprintf(stdout, "%s: Took %.06f seconds\t(%.02f %% overall)\n", StepName, TimeSeconds, TimeSeconds / TotalSeconds * 100.0f);
    };

    {
        f64 TotalSeconds = (f64)(OS_Timer6 - OS_Timer0) / (f64)OS_Freq;

        PrintDebugPerfTimeStep(OS_Timer6, OS_Timer0, TotalSeconds, OS_Freq, "Total");
        PrintDebugPerfTimeStep(OS_Timer1, OS_Timer0, TotalSeconds, OS_Freq, "Startup");
        PrintDebugPerfTimeStep(OS_Timer2, OS_Timer1, TotalSeconds, OS_Freq, "Generation");
        PrintDebugPerfTimeStep(OS_Timer3, OS_Timer2, TotalSeconds, OS_Freq, "Write JSON");
        PrintDebugPerfTimeStep(OS_Timer4, OS_Timer3, TotalSeconds, OS_Freq, "Read JSON");
        PrintDebugPerfTimeStep(OS_Timer5, OS_Timer4, TotalSeconds, OS_Freq, "Calculate Haversine Average");
        PrintDebugPerfTimeStep(OS_Timer6, OS_Timer5, TotalSeconds, OS_Freq, "Cleanup");
    }

}

