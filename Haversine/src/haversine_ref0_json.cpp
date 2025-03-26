#include "haversine_ref0_json.h"

namespace Haversine_Ref0
{
    using b64 = u64;
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

    struct JsonRoot
    {
        DynamicArray<JsonObject*> Objects;
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
}

namespace Haversine_Ref0_JsonHelpers
{
    /*
    using Haversine_Ref0::JsonType;
    using Haversine_Ref0::JsonValue;
    */
    using namespace Haversine_Ref0;

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
    bool TryReadUntilValueBegin(char* Begin, char** End)
    {
        if (!End) { return false; }
        int ReadIdx = 0;
        while (true)
        {
            switch (Begin[ReadIdx])
            {
                case ':':
                {
                    *End = Begin + ReadIdx + 1;
                    return true;
                } break;
                case '"':
                case '\0':
                {
                    return false;
                } break;
            }
            ReadIdx++;
        }
        return false;
    }
}

namespace Haversine_Ref0
{
    using namespace Haversine_Ref0_JsonHelpers;
#define DEF_LITERAL_VAL(lit_val) \
        static constexpr const char* Literal_##lit_val = #lit_val; \
        static constexpr const int Literal_##lit_val##_Length = sizeof(#lit_val)-1;
#define LITERAL_VAL(lit_val) (Literal_##lit_val)
#define LITERAL_VAL_LENGTH(lit_val) (Literal_##lit_val##_Length)

    DEF_LITERAL_VAL(null);
    DEF_LITERAL_VAL(true);
    DEF_LITERAL_VAL(false);

    JsonToken ParseNextToken(char* Begin, JsonValue* OutValue, char** NextTokenBegin);

    JsonObject Parse(char* JsonData, int Size);
    const char* GetPrintableTokenName(JsonToken InToken);
    void PrintTokenStep(char* JsonData, Haversine_Ref0::JsonToken InToken, Haversine_Ref0::JsonValue InValue, int BeginIdx, int EndIdx);
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

const char* Haversine_Ref0::GetPrintableTokenName(JsonToken InToken)
{
    switch (InToken)
    {
        case Token_LeftCurly: return "LeftBracket";
        case Token_RightCurly: return "RightBracket";
        case Token_LeftSquare: return "LeftBrace";
        case Token_RightSquare: return "RightBrace";
        case Token_Colon: return "Colon";
        case Token_Comma: return "Comma";
        case Token_String: return "String";
        case Token_Number: return "Number";
        case Token_Null: return "Null";
        case Token_True: return "True";
        case Token_False: return "False";
        case Token_Error: return "Error";
        case Token_End: return "End";
    }
    return "";
}

void Haversine_Ref0::PrintTokenStep(char* JsonData, Haversine_Ref0::JsonToken InToken, Haversine_Ref0::JsonValue InValue, int BeginIdx, int EndIdx)
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

void Haversine_Ref0::TraceTokens(char* JsonData, int DataLength)
{
    int ReadIdx = 0;
    while (ReadIdx < DataLength)
    {
        int BeginIdx = ReadIdx;
        char* NextTokenBegin = 0;
        JsonValue ParsedValue = {};
        JsonToken CurrToken = ParseNextToken(JsonData + ReadIdx, &ParsedValue, &NextTokenBegin);

        PrintTokenStep(JsonData, CurrToken, ParsedValue, BeginIdx, NextTokenBegin ? NextTokenBegin - JsonData : 0);
        if (CurrToken != Token_Error)
        {
            ReadIdx = NextTokenBegin - JsonData;
        }
    }
}

Haversine_Ref0::JsonObject Haversine_Ref0::Parse(char* JsonData, int Size)
{

    JsonObject Root = {};
    Root.Value.Type = JsonType_Object;
    Root.Value.List = new DynamicArray<JsonObject*>;

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
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>;
                    auto LastVal = Stack.TopList()->Last();
                    Stack.TopList()->Last()->Value.Type = JsonType_Object;
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else if (Stack.TopType() == JsonType_Array && 
                        (bAfterComma || Stack.TopList()->Num == 0))
                {
                    Stack.TopList()->Add(new JsonObject);
                    Stack.TopList()->Last()->Key = nullptr;
                    Stack.TopList()->Last()->Value.Type = JsonType_Object;
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>;
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
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>;
                    Stack.TopList()->Last()->Value.Type = JsonType_Array;
                    Stack.TopList()->Last()->Value.List = NewList;
                    Stack.Push(Stack.TopList()->Last());
                }
                else if (Stack.TopType() == JsonType_Array && 
                        (bAfterComma || Stack.TopList()->Num == 0))
                {
                    DynamicArray<JsonObject*>* NewList = new DynamicArray<JsonObject*>;
                    Stack.TopList()->Add(new JsonObject);
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
                        ObjToSetValue = *Stack.TopList()->Add_RetPtr(new JsonObject);
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
                        JsonObject* NewObj_Key = *Stack.TopList()->Add_RetPtr(new JsonObject);
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
        PrintTokenStep(JsonData, CurrToken, OutValue, BeginIdx, ReadIdx);
    }
    return Root;
}

HList Haversine_Ref0::ParseJSON(FileContentsT& InputFile)
{
    HList Result = {};
    if (nullptr == InputFile.Data) { return Result; }

    fprintf(stdout, "PARSE BEGIN:\n");
    JsonObject Root = Haversine_Ref0::Parse((char*)InputFile.Data, InputFile.Size);
    fprintf(stdout, "PARSE END\n");
    return Result;
}

