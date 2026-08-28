#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

FileContentsT ReadFileContents(const char* FileName, bool bAppendZero)
{
    FileContentsT Result = {};

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        fseek(FileHandle, 0, SEEK_END);
        Result.Size = (u64)ftell(FileHandle) + (bAppendZero ? 1 : 0);
        fseek(FileHandle, 0, SEEK_SET);

        if (Result.Size)
        {
            Result.Contents = new u8[Result.Size];
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

enum JSONType
{
    JSONType_Object,
    JSONType_Array,
    JSONType_String,
    JSONType_NumberInt,
    JSONType_NumberFloat,
    JSONType_Boolean,
    JSONType_Null,
    JSONType_Error,
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
};

enum JSONToken
{
    JSONToken_LeftCurly,
    JSONToken_RightCurly,
    JSONToken_LeftSquare,
    JSONToken_RightSquare,
    JSONToken_Colon,
    JSONToken_Comma,
    JSONToken_String,
    JSONToken_Number,
    JSONToken_LiteralBoolean,
    JSONToken_LiteralNull,
    JSONToken_Error,
    JSONToken_End,
    JSONToken_Other
};

struct JSONParseContext
{
    FileContentsT JSONContents;
    u64 ReadIdx;

    JSONObject Root;
    DynamicArray<JSONObject*> Stack;

    bool bError;
    bool bEnd;

#define ENABLE_DEBUG_STREAM() (_DEBUG && 1)
#if ENABLE_DEBUG_STREAM()
    DynamicArray<JSONToken> Debug_TokenStream;
    DynamicArray<JSONValue> Debug_ValueStream;

    void Debug_PrintToken(int& TokenIdx, int& ValueIdx);
    void Debug_PrintStream();
#endif // ENABLE_DEBUG_STREAM()

    bool IsCharValidNumber(char X);

    JSONToken PeekNextToken();
    JSONValue ParseString();
    JSONValue ParseNumber();
    JSONValue ParseLiteral();

    void ParseToken();
};

#if ENABLE_DEBUG_STREAM()
void JSONParseContext::Debug_PrintToken(int& TokenIdx, int& ValueIdx)
{
    if (TokenIdx < Debug_TokenStream.Size)
    {
        JSONToken Token = Debug_TokenStream[TokenIdx];
        switch (Token)
        {
            case JSONToken_LeftCurly: { printf("{\n"); } break;
            case JSONToken_RightCurly: { printf("}\n"); } break;
            case JSONToken_LeftSquare: { printf("[\n"); } break;
            case JSONToken_RightSquare: { printf("]\n"); } break;
            case JSONToken_Colon: { printf(":\n"); } break;
            case JSONToken_Comma: { printf(",\n"); } break;

            case JSONToken_String:
            {
                Assert(ValueIdx < Debug_ValueStream.Size);
                Assert(JSONType_String == Debug_ValueStream[ValueIdx].Type);
                char* ValueString = Debug_ValueStream[ValueIdx].String;
                printf("String -> \"%s\"\n", ValueString);
                ValueIdx++;
            } break;
            case JSONToken_Number:
            {
                Assert(ValueIdx < Debug_ValueStream.Size);
                JSONType Type = Debug_ValueStream[ValueIdx].Type;
                if (Type == JSONType_NumberInt)
                {
                    s64 ValueNumberInt = Debug_ValueStream[ValueIdx].NumberInt;
                    printf("NumberInt -> %lld\n", ValueNumberInt);
                }
                else if (Type == JSONType_NumberFloat)
                {
                    f64 ValueNumberFloat = Debug_ValueStream[ValueIdx].NumberFloat;
                    printf("NumberFloat -> %f\n", ValueNumberFloat);
                }
                else { Assert(false); }
                ValueIdx++;
            } break;
            case JSONToken_LiteralBoolean:
            {
                Assert(ValueIdx < Debug_ValueStream.Size);
                Assert(Debug_ValueStream[ValueIdx].Type == JSONType_Boolean);
                printf("Boolean -> %s\n", Debug_ValueStream[ValueIdx].Boolean ? "true" : "false");
                ValueIdx++;
            } break;
            case JSONToken_LiteralNull:
            {
                Assert(ValueIdx < Debug_ValueStream.Size);
                Assert(Debug_ValueStream[ValueIdx].Type == JSONType_Null);
                printf("Null\n");
                ValueIdx++;
            } break;

            case JSONToken_Error: { printf("Token_Error\n"); } break;
            case JSONToken_End: { printf("Token_End\n"); } break;

            case JSONToken_Other:
            default:
            {
                Assert(false);
            } break;
        }

        TokenIdx++;
    }
}
void JSONParseContext::Debug_PrintStream()
{
    int TokenIdx = 0;
    int ValueIdx = 0;
    while (TokenIdx < Debug_TokenStream.Size)
    {
        Debug_PrintToken(TokenIdx, ValueIdx);
    }
}
#endif // ENABLE_DEBUG_STREAM()

bool JSONParseContext::IsCharValidNumber(char X)
{
    // NOTE(CKA): This doesn't support scientific notation as-is
    return ('0' <= X && X <= '9') || X == '-' || X == '.';
}

JSONToken JSONParseContext::PeekNextToken()
{
    if (bError) { return JSONToken_Error; }
    if (bEnd || ReadIdx >= JSONContents.Size) { return JSONToken_End; }

    int PeekIdx = 0;
    JSONToken Token = JSONToken_Other;
    while ((ReadIdx + PeekIdx) < JSONContents.Size)
    {
        u8 CurrChar = JSONContents.Contents[ReadIdx + PeekIdx];
        switch (CurrChar)
        {
            case '{': { Token = JSONToken_LeftCurly; } break;
            case '}': { Token = JSONToken_RightCurly; } break;
            case '[': { Token = JSONToken_LeftSquare; } break;
            case ']': { Token = JSONToken_RightSquare; } break;
            case ':': { Token = JSONToken_Colon; } break;
            case ',': { Token = JSONToken_Comma; } break;
            case '"': { Token = JSONToken_String; } break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
            case '-': { Token = JSONToken_Number; } break;

            case 't':
            case 'f': { Token = JSONToken_LiteralBoolean; } break;

            case 'n': { Token = JSONToken_LiteralNull; } break;

            case '\0': { Token = JSONToken_End; } break;

            default: { } break;
        }

        if (Token != JSONToken_Other) { break; }
        else { PeekIdx++; }
    }

    if ((ReadIdx + PeekIdx) >= JSONContents.Size) { Token = JSONToken_End; }
    else { ReadIdx += PeekIdx; }

    return Token;
}

JSONValue JSONParseContext::ParseString()
{
    Assert(JSONContents.Contents[ReadIdx] == '"');
    u64 BeginQuoteIdx = ReadIdx;

    ReadIdx++;
    while (ReadIdx < JSONContents.Size && JSONContents.Contents[ReadIdx] != '"') { ReadIdx++; }
    Assert(JSONContents.Contents[ReadIdx] == '"');

    u64 EndQuoteIdx = ReadIdx;

    JSONValue Result = { JSONType_Error };
    if (ReadIdx < JSONContents.Size && 
        BeginQuoteIdx < EndQuoteIdx &&
        JSONContents.Contents[BeginQuoteIdx] == '"' &&
        JSONContents.Contents[EndQuoteIdx] == '"')
    {
        u64 BeginStringIdx = BeginQuoteIdx + 1;
        u64 EndStringIdx = EndQuoteIdx - 1;
        u64 StringLength = EndStringIdx - BeginStringIdx + 1;

        Result.Type = JSONType_String;
        Result.String = new char[StringLength + 1];
        for (int StringIdx = 0; StringIdx < StringLength; StringIdx++)
        {
            Result.String[StringIdx] = JSONContents.Contents[BeginStringIdx + StringIdx];
        }
        Result.String[StringLength] = '\0';

        ReadIdx++;
    }

    return Result;
}

JSONValue JSONParseContext::ParseNumber()
{
    JSONValue Result{ JSONType_Error };

    Assert(IsCharValidNumber(JSONContents.Contents[ReadIdx]));
    u64 BeginNumberIdx = ReadIdx;
    bool bDecimal = false;

    ReadIdx++;
    bool bValid = true;
    while (bValid && ReadIdx < JSONContents.Size && IsCharValidNumber(JSONContents.Contents[ReadIdx]))
    {
        if (JSONContents.Contents[ReadIdx] == '.')
        {
            if (bDecimal) { bValid = false; }
            else { bDecimal = true; }
        }
        ReadIdx++;
    }

    Assert(bValid);
    if (bValid && IsCharValidNumber(JSONContents.Contents[BeginNumberIdx]))
    {
        if (bDecimal)
        {
            char* FirstCharAfterNumber = nullptr;
            Result.Type = JSONType_NumberFloat;
            Result.NumberFloat = strtod((const char*)&JSONContents.Contents[BeginNumberIdx], &FirstCharAfterNumber);
            Assert((u8*)FirstCharAfterNumber == (JSONContents.Contents + ReadIdx));
        }
        else
        {
            char* FirstCharAfterNumber = nullptr;
            Result.Type = JSONType_NumberInt;
            Result.NumberInt = strtoll((const char*)&JSONContents.Contents[BeginNumberIdx], &FirstCharAfterNumber, 10);
            Assert((u8*)FirstCharAfterNumber == (JSONContents.Contents + ReadIdx));
        }
    }

    return Result;
}

JSONValue JSONParseContext::ParseLiteral()
{
    JSONValue Result{ JSONType_Error };

    auto IsValidLiteralString = [&](const char* Literal, int LiteralLength) -> bool
    {
        if ((ReadIdx + LiteralLength - 1) < JSONContents.Size)
        {
            int Idx = 0;
            while (Idx < LiteralLength && JSONContents.Contents[ReadIdx + Idx] == Literal[Idx]) { Idx++; }
            return Idx == LiteralLength;
        }
        return false;
    };

    constexpr const char* LiteralTrue= "true";
    constexpr size_t LiteralTrueLength = sizeof("true") - 1;
    constexpr const char* LiteralFalse = "false";
    constexpr size_t LiteralFalseLength = sizeof("false") - 1;
    constexpr const char* LiteralNull = "null";
    constexpr size_t LiteralNullLength = sizeof("null") - 1;

    switch (JSONContents.Contents[ReadIdx])
    {
        case 't':
        {
            if (IsValidLiteralString(LiteralTrue, LiteralTrueLength))
            {
                Result.Type = JSONType_Boolean;
                Result.Boolean = true;
                ReadIdx += LiteralTrueLength;
            }
            else { bError = true; }
        } break;
        case 'f':
        {
            if (IsValidLiteralString(LiteralFalse, LiteralFalseLength))
            {
                Result.Type = JSONType_Boolean;
                Result.Boolean = false;
                ReadIdx += LiteralFalseLength;
            }
            else { bError = true; }
        } break;
        case 'n':
        {
            if (IsValidLiteralString(LiteralNull, LiteralNullLength))
            {
                Result.Type = JSONType_Null;
                ReadIdx += LiteralNullLength;
            }
            else { bError = true; }
        } break;
        default:
        {
            bError = true;
        } break;
    }
    return Result;
}

void JSONParseContext::ParseToken()
{
    JSONToken Token = PeekNextToken();
    switch (Token)
    {
        case JSONToken_LeftCurly:
        {
            ReadIdx++;
        } break;

        case JSONToken_RightCurly:
        {
            ReadIdx++;
        } break;

        case JSONToken_LeftSquare:
        {
            ReadIdx++;
        } break;

        case JSONToken_RightSquare:
        {
            ReadIdx++;
        } break;

        case JSONToken_Colon:
        {
            ReadIdx++;
        } break;

        case JSONToken_Comma:
        {
            ReadIdx++;
        } break;

        case JSONToken_String:
        {
            int StringLength = 0;
            JSONValue Debug_NewString = ParseString();
        #if ENABLE_DEBUG_STREAM()
            Debug_ValueStream.Add(Debug_NewString);
        #endif ENABLE_DEBUG_STREAM()
        } break;

        case JSONToken_Number:
        {
            int NumberLength = 0;
            JSONValue NewNumber = ParseNumber();
        #if ENABLE_DEBUG_STREAM()
            Debug_ValueStream.Add(NewNumber);
        #endif ENABLE_DEBUG_STREAM()
        } break;

        case JSONToken_LiteralBoolean:
        case JSONToken_LiteralNull:
        {
            JSONValue NewLiteral = ParseLiteral();
        #if ENABLE_DEBUG_STREAM()
            Debug_ValueStream.Add(NewLiteral);
        #endif ENABLE_DEBUG_STREAM()
        } break;

        case JSONToken_End: { bEnd = true; } break;

        case JSONToken_Error:
        case JSONToken_Other:
        default: { bError = true; } break;
    }

#if ENABLE_DEBUG_STREAM()
    Debug_TokenStream.Add(Token);
    if (bError) { Debug_TokenStream.Add(JSONToken_Error); }
#endif // ENABLE_DEBUG_PARSE_STREAM()
}

JSONObject JSONParse(FileContentsT JSONContents)
{
    if (JSONContents.Size == 0 || !JSONContents.Contents) { return JSONObject{}; }

    JSONParseContext Context;
    Context.JSONContents = JSONContents;
    Context.ReadIdx = 0;
    Context.Root = {};
    Context.Root.Value.Type = JSONType_Object;
    Context.Root.Value.List = new DynamicArray<JSONObject*>;
    Context.bError = false;
    Context.bEnd = false;

    while (!Context.bError && !Context.bEnd)
    {
        Context.ParseToken();
    }

#if ENABLE_DEBUG_STREAM()
    Context.Debug_PrintStream();
#endif // ENABLE_DEBUG_STREAM()

    return Context.Root;
}

int main()
{
    FileContentsT JSONText = ReadFileContents("test/example.json", true);
    //FileContentsT JSONText = ReadFileContents("test/example_literals.json", true);
    //FileContentsT JSONText = ReadFileContents("test/example_numbers.json", true);
    JSONObject Root = JSONParse(JSONText);

    return 0;
}

