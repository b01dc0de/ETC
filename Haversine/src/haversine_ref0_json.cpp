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
            DynamicArray<JsonObject*>* pObjects;
            DynamicArray<JsonObject*>* pArray;
        };
    };

    enum JsonToken
    {
        Token_LeftBracket,
        Token_RightBracket,
        Token_LeftBrace,
        Token_RightBrace,
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
        using ItemT = DynamicArray<JsonObject*>*;
        using RefT = DynamicArray<JsonObject*>&;
        int Depth;
        DynamicArray<JsonType> Types;
        DynamicArray<ItemT> Stack;

        void Init(ItemT Root)
        {
            Types.Add(JsonType_Object);
            Stack.Add(Root);
            Depth = 0;
        }
        void Push(JsonType Type, ItemT Item) { Types.Add(Type); Stack.Add(Item); Depth++; }
        void Pop() { if (Depth > 0) { Types.RemoveLast(); Stack.RemoveLast(); Depth--; } }
        RefT Top() { return *Stack[Depth]; }
        JsonType TopType() { return Types[Depth]; }
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

    struct ParseJsonStateMachine
    {
        enum StateType
        {
            ParseState_Root,
            ParseState_Key,
            ParseState_Value,
            ParseState_Array,
            ParseState_End,
            ParseState_Error
        };

        StateType State;
        int Depth;
        JsonRoot* Root;
        JsonTreeStack Stack;

        void Init(JsonRoot* InRoot)
        {
            State = ParseState_Root;
            Depth = 0;
            Root = InRoot;
            Stack.Init(&InRoot->Objects);
        }
        bool Error() { return State == ParseState_Error; }
        int Parse_Root(char* JsonData, int StartIdx);
        int Parse_Key(char* JsonData, int StartIdx);
        int Parse_Value(char* JsonData, int StartIdx);
        int Parse_Array(char* JsonData, int StartIdx);
        bool Advance(char* JsonData, int* Idx);
    };
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
            case '{': *NextTokenBegin = Begin + ReadIdx + 1; return Token_LeftBracket;
            case '}': *NextTokenBegin = Begin + ReadIdx + 1; return Token_RightBracket;
            case '[': *NextTokenBegin = Begin + ReadIdx + 1; return Token_LeftBrace;
            case ']': *NextTokenBegin = Begin + ReadIdx + 1; return Token_RightBrace;
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

void Haversine_Ref0::TraceTokens(char* JsonData, int DataLength)
{
    auto GetPrintableTokenName = [](JsonToken InToken) -> const char*
    {
        switch (InToken)
        {
            case Token_LeftBracket: return "LeftBracket";
            case Token_RightBracket: return "RightBracket";
            case Token_LeftBrace: return "LeftBrace";
            case Token_RightBrace: return "RightBrace";
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
    };
    int ReadIdx = 0;
    while (ReadIdx < DataLength)
    {
        char* NextTokenBegin = 0;
        JsonValue ParsedValue = {};
        JsonToken CurrToken = ParseNextToken(JsonData + ReadIdx, &ParsedValue, &NextTokenBegin);
        if (CurrToken == Token_Error)
        {
            fprintf(stdout, "[json][token] ERROR encountered at Idx %d!\n", ReadIdx);
            break;
        }
        else
        {
            fprintf(stdout, "[json][token] Read next token: %s", GetPrintableTokenName(CurrToken));
            if (CurrToken == Token_String) { fprintf(stdout, ": \"%s\"\n", ParsedValue.String); }
            else if (CurrToken == Token_Number && ParsedValue.Type == JsonType_NumberInt) { fprintf(stdout, ": %lld\n", ParsedValue.NumberInt); }
            else if (CurrToken == Token_Number && ParsedValue.Type == JsonType_NumberFloat) { fprintf(stdout, ": %f\n", ParsedValue.NumberFloat); }
            else { fprintf(stdout, "\n"); }
            fprintf(stdout, "\n\tBeginIdx: %d\tEndIdx:%d\n",
                    ReadIdx, NextTokenBegin ? (int)(NextTokenBegin - JsonData) : 0);
            ReadIdx = NextTokenBegin - JsonData;
        }
    }
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Root(char* JsonData, int StartIdx)
{
    int ReadIdx = StartIdx;
    while (State == ParseState_Root)
    {
        switch (JsonData[ReadIdx])
        {
            case '{': { State = ParseState_Key; } break;
            case '}': { State = ParseState_End; } break;
            case '\0': { State = ParseState_Error; } break;
            default: { } break;
        }
        ReadIdx++;
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Key(char* JsonData, int StartIdx)
{
    int ReadIdx = StartIdx;
    bool bStartKey = true;
    while (State == ParseState_Key && bStartKey)
    {
        switch (JsonData[ReadIdx])
        {
            case '"':
            {
                Stack.Top().Add(new JsonObject);
                char* RightQuote = nullptr;
                if (TryReadString(JsonData + ReadIdx, &RightQuote, &Stack.Top().Last()->Key))
                {
                    ReadIdx = RightQuote - JsonData + 1;

                    char* CharAfterColon = nullptr;
                    if (TryReadUntilValueBegin(JsonData + ReadIdx, &CharAfterColon))
                    {
                        State = ParseState_Value;
                        ReadIdx = CharAfterColon - JsonData - 1;
                    }
                    else
                    {
                        State = ParseState_Error;
                    }
                }
                else
                {
                    State = ParseState_Error;
                }
            } break;
            case '}':
            {
                if (Stack.Root())
                {
                    State = ParseState_End;
                }
                else
                {
                    Stack.Pop();
                    if (Stack.TopType() == JsonType_Array)
                    {
                        State = ParseState_Array;
                    }
                }
                ReadIdx++;
                bStartKey = false;
                return ReadIdx;
            } break;
            case '\0':
            {
                State = ParseState_Error;
            } break;
        }
        if (State == ParseState_Key) { ReadIdx++; }
    }
    bool bAdvToNextSigChar = true;
    while (bAdvToNextSigChar)
    {
        switch (JsonData[ReadIdx])
        {
            // Valid
            case ':':
            {
                bAdvToNextSigChar = false;
                ReadIdx++;
            } break;
            case '"':
            case ',':
            case '[':
            case '{':
            {
                bAdvToNextSigChar = false;
            } break;
            case '}':
            case ']':
            case '\0':
            {
                State = ParseState_Error;
                bAdvToNextSigChar = false;
            } break;
        }
        if (bAdvToNextSigChar) { ReadIdx++; }
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Value(char* JsonData, int StartIdx)
{
    /*
    // TODO: Commented this out due to false positives triggering when we did read a key, but it was null (""), how should this be handled/marked?
    if (nullptr == Stack.Top().Last()->Key)
    {
        // If we haven't read a key yet, then this indicates a bug in Parse_Key
        State = ParseState_Error;
        return StartIdx;
    }
    */

    int ReadIdx = StartIdx;
    bool bDone = false;
    while (!bDone && State == ParseState_Value)
    {
        // Try to parse value type
        switch (JsonData[ReadIdx])
        {
            case 'n': // Type is null
            {
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(null)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(null);
                    bDone = true;
                }
                else { State = ParseState_Error; }
            } break;
            case 't': // Type is bool (true)
            {
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(true)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(true);
                    Stack.Top().Last()->Value.Type = JsonType_Bool;
                    Stack.Top().Last()->Value.Bool = true;
                    bDone = true;
                }
                else { State = ParseState_Error; }
            } break;
            case 'f': // Type is bool (false)
            {
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(false)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(false);
                    Stack.Top().Last()->Value.Type = JsonType_Bool;
                    Stack.Top().Last()->Value.Bool = false;
                    bDone = true;
                }
                else { State = ParseState_Error; }
            } break;
            case '"': // Type is string
            {
                Stack.Top().Last()->Value.Type = JsonType_String;
                char* RightQuote = nullptr;
                if (TryReadString(JsonData + ReadIdx, &RightQuote, &Stack.Top().Last()->Value.String))
                {
                    ReadIdx = RightQuote - JsonData + 1;
                    bDone = true;
                }
                else
                {
                    State = ParseState_Error;
                }
            } break;
            case '{': // Type is Object
            {
                Stack.Top().Last()->Value.Type = JsonType_Object;
                DynamicArray<JsonObject*>* NewObjs = new DynamicArray<JsonObject*>;
                Stack.Top().Last()->Value.pObjects = NewObjs; 
                Stack.Push(JsonType_Object, NewObjs);
                ReadIdx++;
                State = ParseState_Key;
            } break;
            case '[': // Type is Array
            {
                Stack.Top().Last()->Value.Type = JsonType_Array;
                DynamicArray<JsonObject*>* NewArray = new DynamicArray<JsonObject*>;
                Stack.Top().Last()->Value.pArray = NewArray; 
                ReadIdx++;
                Stack.Push(JsonType_Array, NewArray);
                State = ParseState_Array;
            } break;
            case '}':
            case ']':
            {
                State = ParseState_Error;
            } break;
            case ':':
            case ',':
            case '\0':
            {
                State = ParseState_Error;
            } break;
        }
        if (!bDone && State == ParseState_Value && CharIsBeginNumber(JsonData[ReadIdx]))
        {
            char* CharAfterNumber = nullptr;
            if (TryReadNumber(JsonData + ReadIdx, &CharAfterNumber, &Stack.Top().Last()->Value))
            {
                ReadIdx = CharAfterNumber - JsonData;
                bDone = true;
            }
            else
            {
                State = ParseState_Error;
            }
            break;
        }
        else if (!bDone && State == ParseState_Value)
        {
            ReadIdx++;
        }
    }
    // If we haven't hit an error, try to read chars until the next key begin (or root end)
    while (State == ParseState_Value)
    {
        switch (JsonData[ReadIdx])
        {
            case ',': // There is another object
            {
                State = ParseState_Key;
            } break;
            case '}': // End of object
            {
                if (Stack.Root()) { State = ParseState_End; }
                else
                {
                    Stack.Pop();
                    if (Stack.TopType() == JsonType_Array)
                    {
                        State = ParseState_Array;
                    }
                    else if (Stack.TopType() == JsonType_Object)
                    {
                    }
                    else
                    {
                        State = ParseState_Error;
                    }
                }
            } break;
            case '\0': // End of file
            {
                State = ParseState_Error;
            } break;
        }
        ReadIdx++;
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Array(char* JsonData, int StartIdx)
{
    DynamicArray<JsonObject*>* pArray = &Stack.Top();
    if (!pArray) { State = ParseState_Error; }
    int ReadIdx = StartIdx;
    bool bStartArray = true;
    bool bExpectCommaOrEnd = true;
    while (State == ParseState_Array && bStartArray)
    {
        switch (JsonData[ReadIdx])
        {
            case ',':
            {
                if (!bExpectCommaOrEnd)
                {
                    State = ParseState_Error;
                }
                bExpectCommaOrEnd = false;
            } break;
            case 'n': // Type is null
            {
                pArray->Add(new JsonObject{});
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(null)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(null);
                }
                else { State = ParseState_Error; }
                bExpectCommaOrEnd = true;
            } break;
            case 't': // Type is bool (true)
            {
                pArray->Add(new JsonObject{});
                pArray->Last()->Value.Type = JsonType_Bool;
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(true)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(true);
                    Stack.Top().Last()->Value.Type = JsonType_Bool;
                    Stack.Top().Last()->Value.Bool = true;
                }
                else { State = ParseState_Error; }
                bExpectCommaOrEnd = true;
            } break;
            case 'f': // Type is bool (false)
            {
                pArray->Add(new JsonObject{});
                pArray->Last()->Value.Type = JsonType_Bool;
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(false)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(false);
                    Stack.Top().Last()->Value.Type = JsonType_Bool;
                    Stack.Top().Last()->Value.Bool = false;
                }
                else { State = ParseState_Error; }
                bExpectCommaOrEnd = true;
            } break;
            case '"': // Type is string
            {
                pArray->Add(new JsonObject{});
                pArray->Last()->Value.Type = JsonType_String;
                char* RightQuote = nullptr;
                if (TryReadString(JsonData + ReadIdx, &RightQuote, &Stack.Top().Last()->Value.String))
                {
                    ReadIdx = RightQuote - JsonData;
                }
                else { State = ParseState_Error; }
                bExpectCommaOrEnd = true;
            } break;
            case '{': // Type is Object
            {
                pArray->Add(new JsonObject{});
                pArray->Last()->Value.Type = JsonType_Object;
                DynamicArray<JsonObject*>* NewObjs = new DynamicArray<JsonObject*>;
                pArray->Last()->Value.pObjects = NewObjs;
                ReadIdx++;
                Stack.Push(JsonType_Object, NewObjs);
                State = ParseState_Key;
            } break;
            case '[': // Type is Array
            {
                pArray->Add(new JsonObject{});
                pArray->Last()->Value.Type = JsonType_Array;
                DynamicArray<JsonObject*>* NewArray = new DynamicArray<JsonObject*>;
                pArray->Last()->Value.pArray = NewArray;
                ReadIdx++;
                Stack.Push(JsonType_Array, NewArray);
                State = ParseState_Array;
                bStartArray = false;
            } break;
            case ']':
            {
                // TODO: Add in check that comma wasn't read before this without a value,
                // But for now, just accept this as the end of the array
                if (Stack.Root()) { State = ParseState_Error; }
                else
                {
                    Stack.Pop();
                    if (Stack.TopType() == JsonType_Object)
                    {
                        State = ParseState_Key;
                    }
                    else if (Stack.TopType() == JsonType_Array)
                    {
                    }
                    bStartArray = false;
                    ReadIdx++;
                }
            } break;
            case '}':
            {
                State = ParseState_Error;
            } break;
        }
        if (CharIsBeginNumber(JsonData[ReadIdx]))
        {
            pArray->Add(new JsonObject{});
            char* CharAfterNumber = nullptr;
            if (TryReadNumber(JsonData + ReadIdx, &CharAfterNumber, &
            pArray->Last()->Value))
            {
                ReadIdx = CharAfterNumber - JsonData - 1;
            }
            else { State = ParseState_Error; }
            bExpectCommaOrEnd = true;
        }

        if (State == ParseState_Array && bStartArray)
        {
            ReadIdx++;
        }
    }
    return ReadIdx;
}

bool Haversine_Ref0::ParseJsonStateMachine::Advance(char* JsonData, int* Idx)
{
    StateType BeginState = State;
    int BeginIdx = *Idx;
    int ReadIdx = *Idx;
    switch (State)
    {
        case ParseState_Root:
        {
            ReadIdx = Parse_Root(JsonData, ReadIdx);
        } break;
        case ParseState_Key:
        {
            ReadIdx = Parse_Key(JsonData, ReadIdx);
        } break;
        case ParseState_Array:
        {
            ReadIdx = Parse_Array(JsonData, ReadIdx);
        } break;
        case ParseState_Value:
        {
            ReadIdx = Parse_Value(JsonData, ReadIdx);
        } break;
        case ParseState_End:
        {
            // TODO: Error handling(?)
            /*
             * while (JsonData[ReadIdx] != '\0') { ReadIdx++; }
             */

        } break;
    }
    constexpr bool bDebugTracePrint = true;
    if (bDebugTracePrint)
    {
        auto GetStateName = [](StateType InState) -> const char*
        {
            switch (InState)
            {
                case ParseState_Root: return "Root";
                case ParseState_Key: return "Key";
                case ParseState_Value: return "Value";
                case ParseState_Array: return "Array";
                case ParseState_End: return "End";
                case ParseState_Error: return "Error";
                default: return "";
            }
        };
        auto GetPrintableChar = [](char C) -> char
        {
            switch (C)
            {
                case '\n':
                case '\t':
                case '\r':
                    return '\\';
                default:
                    return C;
            }
        };

        fprintf(stdout, "[json][debug] Advance:\n");
        fprintf(stdout, "\t\tBeginState: %s, BeginIdx: %d '%c' (0x%x)\n",
                GetStateName(BeginState), BeginIdx, GetPrintableChar(JsonData[BeginIdx]), JsonData[BeginIdx]);
        fprintf(stdout, "\t\tEndState: %s, EndIdx: %d '%c' (0x%x)\n\n",
                GetStateName(State), ReadIdx, GetPrintableChar(JsonData[ReadIdx]), JsonData[ReadIdx]);
        fprintf(stdout, "\t\tJSON Parsed:%.*s\n\n", ReadIdx - BeginIdx, JsonData + BeginIdx);
    }
    *Idx = ReadIdx;
    return (State != ParseState_End && State != ParseState_Error);
}

HList Haversine_Ref0::ParseJSON(FileContentsT& InputFile)
{
    HList Result = {};
    if (nullptr == InputFile.Data) { return Result; }

#define ENABLE_TRACE_DEBUG() (1)
#if ENABLE_TRACE_DEBUG()
    TraceTokens((char*)InputFile.Data, InputFile.Size);
    return Result;
#endif // ENABLE_TRACE_DEBUG()

    JsonRoot Root = {};
    ParseJsonStateMachine StateMachine = {};
    StateMachine.Init(&Root);

    char* pRead = (char*)InputFile.Data;
    int ReadIdx = 0;
    while (ReadIdx < InputFile.Size &&
            StateMachine.Advance(pRead, &ReadIdx))
    {
    }

    if (StateMachine.Error())
    {
        fprintf(stdout, "[json]: Error while parsing!\n");
    }
    else
    {
        fprintf(stdout, "[json]: Parse success! no errors :)\n");
    }

    return Result;
}

