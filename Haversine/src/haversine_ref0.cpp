#include "haversine_ref0.h"
#include "haversine_perf.h"

constexpr f64 CoordXMin = -180.0;
constexpr f64 CoordXMax = +180.0;
constexpr double CoordYMin = -90.0;
constexpr double CoordYMax = +90.0;
constexpr f64 DegreesPerRadian = 0.01745329251994329577;
constexpr f64 EarthRadius = 6372.8;

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
}

namespace Haversine_Ref0
{
    using namespace Haversine_Ref0_Helpers;

    using RandomEngineT = std::default_random_engine;
    using UniformRealDistT = std::uniform_real_distribution<f64>;
    using UniformIntDistT = std::uniform_int_distribution<int>;

    static constexpr int DefaultClusterCount = 8;
}

f64 Haversine_Ref0::CalculateHaversine(HPair Pair)
{
    return Haversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
}

f64 Haversine_Ref0::CalculateAverage(HList List)
{
    TIME_FUNC();

    f64 Sum = 0.0f;
    for (int PairIdx = 0; PairIdx < List.Count; PairIdx++)
    {
        f64 fHaversine = CalculateHaversine(List.Data[PairIdx]);
        Sum += fHaversine;
    }
    f64 Average = Sum / (f64)List.Count;
    return Average;
}

HList Haversine_Ref0::GenerateDataUniform(int Seed, int Count)
{
    TIME_FUNC();

    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordx_dist(CoordXMin, CoordXMax);
    UniformRealDistT coordy_dist(CoordYMin, CoordYMax);

    HList Result = {Count, new HPair[Count]};
    for (int PairIdx = 0; PairIdx < Count; PairIdx++)
    {
        Result.Data[PairIdx].X0 = coordx_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = coordy_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = coordx_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = coordy_dist(default_rand_engine);
    }

    PrintData(Result);

    constexpr bool bVerboseDebugPrint = false;
    if (bVerboseDebugPrint)
    {
        fprintf(stdout, "Generated Pair data - Uniform - Count: %d, Seed: %d\n",
                Count, Seed);
        f64 Average = CalculateAverage(Result);
        fprintf(stdout, "Average: %f\n", Average);
    }

    return Result;
}

HList Haversine_Ref0::GenerateDataClustered(int Seed, int Count)
{
    TIME_FUNC();

    RandomEngineT default_rand_engine(Seed);
    UniformRealDistT coordx_dist(CoordXMin, CoordXMax);
    UniformRealDistT coordy_dist(CoordYMin, CoordYMax);

    HPair* Clusters = new HPair[DefaultClusterCount];
    for (int ClusterIdx = 0; ClusterIdx < DefaultClusterCount; ClusterIdx++)
    {
        Clusters[ClusterIdx].X0 = coordx_dist(default_rand_engine);
        Clusters[ClusterIdx].Y0 = coordy_dist(default_rand_engine);
        Clusters[ClusterIdx].X1 = coordx_dist(default_rand_engine);
        Clusters[ClusterIdx].Y1 = coordy_dist(default_rand_engine);
    }

    f64 MaxClusterXOffset = CoordXMax / (DefaultClusterCount * 2);
    f64 MaxClusterYOffset = CoordYMax / (DefaultClusterCount * 2);
    int MaxClusterIdx = DefaultClusterCount - 1;
    UniformIntDistT clusteridx_dist(0, MaxClusterIdx);
    UniformRealDistT clusteroffsetx_dist(-MaxClusterXOffset, +MaxClusterXOffset);
    UniformRealDistT clusteroffsety_dist(-MaxClusterYOffset, +MaxClusterYOffset);

    HList Result = {Count, new HPair[Count]};
    for (int PairIdx = 0; PairIdx < Count; PairIdx++)
    {
        int ClusterIdx = clusteridx_dist(default_rand_engine);
        Result.Data[PairIdx].X0 = Clusters[ClusterIdx].X0 + clusteroffsetx_dist(default_rand_engine);
        Result.Data[PairIdx].Y0 = Clusters[ClusterIdx].Y0 + clusteroffsety_dist(default_rand_engine);
        Result.Data[PairIdx].X1 = Clusters[ClusterIdx].X1 + clusteroffsetx_dist(default_rand_engine);
        Result.Data[PairIdx].Y1 = Clusters[ClusterIdx].Y1 + clusteroffsety_dist(default_rand_engine);
    }

    constexpr bool bVerboseDebugPrint = false;
    if (bVerboseDebugPrint)
    {
        fprintf(stdout, "Generated Pair data - Clustered - Count: %d, Seed: %d\n",
                Count, Seed);
        constexpr bool bPrintGeneratedData = false;
        if (bPrintGeneratedData) { PrintData(Result); }

        double Average = CalculateAverage(Result);
        printf("Average: %f\n", Average);
    }

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
    TIME_FUNC();

    FileContentsT OutputContents = { (int)sizeof(HPair)*List.Count, (u8*)List.Data };
    OutputContents.Write(FileName);
}

void Haversine_Ref0::WriteDataAsJSON(HList List, const char* FileName)
{
    TIME_FUNC();

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
    TIME_FUNC();

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
            TIME_BLOCK_DATA(fread_s, FileSize);
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

    void Release(JsonObject* Object, bool bRoot);

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

void Haversine_Ref0::Release(JsonObject* Object, bool bRoot)
{
    if (!Object) { return; }

    switch (Object->Value.Type)
    {
        case JsonType_Null:
        case JsonType_NumberInt:
        case JsonType_NumberFloat:
        case JsonType_Bool:
        {
        } break;
        case JsonType_String:
        {
            delete Object->Value.String;
        } break;
        case JsonType_Object:
        case JsonType_Array:
        {
            DynamicArray<JsonObject*>* List = Object->Value.List;
            if (List)
            {
                for (int ItemIdx = 0; ItemIdx < List->Num; ItemIdx++)
                {
                    Release((*List)[ItemIdx], false);
                }
                delete List;
            }
        } break;
    }
    if (!bRoot)
    {
        delete Object;
    }
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

    JsonObject Root = Haversine_Ref0::Parse((char*)InputFile.Data, InputFile.Size);
    JsonObject* Pairs = Query(&Root, "pairs");
    if (Pairs)
    {
        Result = ParsePairsArray(Pairs);
    }
    Release(&Root, true);
    return Result;
}

void PrintDebugPerfTimeStep(u64 TimeEnd, u64 TimeBegin, f64 TotalSeconds, u64 Freq, const char* StepName)
{
    u64 Delta = TimeEnd - TimeBegin;
    f64 TimeS = (f64)Delta / (f64)Freq;
    fprintf(stdout, "%s: Took %.06f ms\t(%.02f %% overall)\n", StepName, TimeS * 1000.0f, TimeS / TotalSeconds * 100.0f);
};

void Haversine_Ref0::GetInputDataFileName(char* Buffer, int BufferSize, int Seed, int Count, bool bClustered)
{
    if (bClustered)
    {
        (void)sprintf_s(Buffer, BufferSize, "hvpairs_seed%d_count%d_clustered.json", Seed, Count);
    }
    else
    {
        (void)sprintf_s(Buffer, BufferSize, "hvpairs_seed%d_count%d_uniform.json", Seed, Count);
    }
}

void Haversine_Ref0::Gen(int Seed, int Count, bool bClustered)
{
    TIME_FUNC();

    static constexpr int FileNameMaxSize = 96;
    char JSONFileName[FileNameMaxSize];
    GetInputDataFileName(JSONFileName, FileNameMaxSize, Seed, Count, bClustered);

    HList PairList = {};

    if (bClustered) { PairList = Haversine_Ref0::GenerateDataClustered(Seed, Count); }
    else { PairList = Haversine_Ref0::GenerateDataUniform(Seed, Count); }

    Haversine_Ref0::WriteDataAsJSON(PairList, JSONFileName);
    fprintf(stdout, "Wrote JSON data to file %s\n", JSONFileName); 

    f64 HvAvg = Haversine_Ref0::CalculateAverage(PairList);
    fprintf(stdout, "\tAverage for generated data: %f\n", HvAvg);

    {
        TIME_BLOCK(Gen_Cleanup);
        delete[] PairList.Data;
    }
}

void Haversine_Ref0::Calc(int Seed, int Count, bool bClustered)
{
    using namespace Haversine_Ref0_Helpers;

    TIME_FUNC();

    static constexpr int FileNameMaxSize = 96;
    char JSONFileName[FileNameMaxSize];
    GetInputDataFileName(JSONFileName, FileNameMaxSize, Seed, Count, bClustered);

    HList PairList = ReadFileAsJSON(JSONFileName);
    //fprintf(stdout, "Calculating Haversine Average on parsed JSON file (%s) list of size %d:\n", JSONFileName, PairList.Count);

    f64 HvAvg = CalculateAverage(PairList);
    fprintf(stdout, "\tAverage: %f\n", HvAvg);

    {
        TIME_BLOCK(Calc_Cleanup);
        delete[] PairList.Data;
    }
}

void Haversine_Ref0::Full(int Seed, int Count, bool bClustered)
{
    Gen(Seed, Count, bClustered);

    Calc(Seed, Count, bClustered);
}

