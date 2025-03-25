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
        while (CharIsNumeric(Begin[ReadIdx]) || Begin[ReadIdx] == '.')
        {
            if (Begin[ReadIdx] == '.')
            {
                // Multiple .'s in literal -> report error
                if (NumberType == JsonType_NumberFloat) { return false; }
                else { NumberType = JsonType_NumberFloat; }
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

    bool TryReadString(char* Begin, char** End, char** ParsedString)
    {
        if (Begin[0] != '"' || !ParsedString) { return false; }
        int ReadIdx = 1;
        while (Begin[ReadIdx] != '"')
        {
            if (Begin[ReadIdx] == '\0') { return false; }
            // TODO: Make this _way_ more robust
            ReadIdx++;
        }
        if (ReadIdx > 1)
        {
            *ParsedString = new char[ReadIdx];
            memcpy(*ParsedString, Begin + 1, ReadIdx - 1);
            (*ParsedString)[ReadIdx-1] = '\0';
        }
        else
        {
            *ParsedString = nullptr;
        }
        if (End) { *End = Begin + ReadIdx; }
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

    struct ParseJsonStateMachine
    {
        DEF_LITERAL_VAL(null);
        DEF_LITERAL_VAL(true);
        DEF_LITERAL_VAL(false);

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
        int Advance(char* JsonData, int StartIdx);
    };
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
    bool bThisKey = true;
    while (State == ParseState_Key && bThisKey)
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
            } break;
            case '\0':
            {
                State = ParseState_Error;
            } break;
        }
        ReadIdx++;
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Value(char* JsonData, int StartIdx)
{
    if (nullptr == Stack.Top().Last()->Key)
    {
        // If we haven't read a key yet, then this indicates a bug in Parse_Key
        State = ParseState_Error;
        return StartIdx;
    }

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
    // TODO: Make this more robust, try to verify that Stack.Top() was an ArrayType
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

        if (State == ParseState_Array)
        {
            ReadIdx++;
        }
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Advance(char* JsonData, int StartIdx)
{
    StateType BeginState = State;
    int ReadIdx = StartIdx;
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
            while (JsonData[ReadIdx] != '\0')
            {
                ReadIdx++;
            }
            ReadIdx++;
        } break;
    }
    constexpr bool bDebugTracePrint = false;
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
        fprintf(stdout, "\t\tBeginState: %s, StartIdx: %d '%c' (0x%x)\n",
                GetStateName(BeginState), StartIdx, GetPrintableChar(JsonData[StartIdx]), JsonData[StartIdx]);
        fprintf(stdout, "\t\tEndState: %s, EndIdx: %d '%c' (0x%x)\n\n",
                GetStateName(State), ReadIdx, GetPrintableChar(JsonData[ReadIdx]), JsonData[ReadIdx]);
        fprintf(stdout, "\t\tJSON Parsed:%.*s\n\n", ReadIdx - StartIdx, JsonData + StartIdx);
    }
    return ReadIdx;
}

HList Haversine_Ref0::ParseJSON(FileContentsT& InputFile)
{
    HList Result = {};
    if (nullptr == InputFile.Data) { return Result; }

    JsonRoot Root = {};
    ParseJsonStateMachine StateMachine = {};
    StateMachine.Init(&Root);

    bool bError = false;
    char* pRead = (char*)InputFile.Data;
    int ReadIdx = 0;
    while (!bError && ReadIdx < InputFile.Size)
    {
        ReadIdx = StateMachine.Advance(pRead, ReadIdx);
        bError = StateMachine.Error();
    }

    if (bError)
    {
        fprintf(stdout, "[json]: Error while parsing!\n");
    }
    else
    {
        fprintf(stdout, "[json]: Parse success! no errors :)\n");
    }

    return Result;
}

