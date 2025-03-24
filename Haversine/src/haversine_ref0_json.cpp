#include "haversine_ref0_json.h"

namespace Haversine_Ref0_JsonHelpers
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
    bool TryReadString(char* Begin, char** End, char** ParsedString)
    {
        if (Begin[0] != '"' || !ParsedString) { return false; }
        int ReadIdx = 1;
        while (Begin[ReadIdx] != '"')
        {
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
    using b64 = u64;
    // Forward type decls:
    struct JsonObject;
    struct JsonArray;

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
            JsonObject* Object;
            JsonArray* Array;
        };
    };

    struct JsonObject
    {
        char* Key;
        JsonValue Value;
    };

    struct JsonArray
    {
        char* Key;
        DynamicArray<JsonObject> Values;
    };

    struct JsonRoot
    {
        DynamicArray<JsonObject> Objects;
    };


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
            ParseState_End,
            ParseState_Error
        };

        StateType State;
        int Depth;
        JsonRoot* Root;
        JsonObject* CurrObject;

        void Init(JsonRoot* InRoot)
        {
            State = ParseState_Root;
            Depth = 0;
            Root = InRoot;
            CurrObject = nullptr;
        }
        bool Error() { return State == ParseState_Error; }
        int Parse_Root(char* JsonData, int StartIdx);
        int Parse_Key(char* JsonData, int StartIdx);
        int Parse_Value(char* JsonData, int StartIdx);
        int Advance(char* JsonData, int StartIdx);
    };
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Root(char* JsonData, int StartIdx)
{
    int ReadIdx = StartIdx;
    bool bDone = false;
    while (!bDone)
    {
        switch (JsonData[ReadIdx])
        {
            case '{':
            {
                State = ParseState_Key;
                // TODO: Clean up this syntax:
                CurrObject = Root->Objects.Add_RetPtr(JsonObject{});
                bDone = true;
            } break;
            case '}': { State = ParseState_End; bDone = true; } break;
            case '\0': { State = ParseState_Error; bDone = true; } break;
            default: { } break;
        }
        ReadIdx++;
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Key(char* JsonData, int StartIdx)
{
    int ReadIdx = StartIdx;
    bool bDone = false;
    while (!bDone)
    {
        switch (JsonData[ReadIdx])
        {
            case '"':
            {
                char* RightQuote = nullptr;
                if (TryReadString(JsonData + ReadIdx, &RightQuote, &CurrObject->Key))
                {
                    ReadIdx = RightQuote - JsonData + 1;

                    char* CharAfterColon = nullptr;
                    if (TryReadUntilValueBegin(JsonData + ReadIdx, &CharAfterColon))
                    {
                        State = ParseState_Value;
                        ReadIdx = CharAfterColon - JsonData;
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
                // TODO: Require adjusting when we start adding depth
                State = ParseState_End;
                bDone = true;
            } break;
            default:
            {
            } break;
        }
        ReadIdx++;
    }
    return ReadIdx;
}

int Haversine_Ref0::ParseJsonStateMachine::Parse_Value(char* JsonData, int StartIdx)
{
    int ReadIdx = StartIdx;
    bool bDone = false;
    while (!bDone)
    {
        switch (JsonData[ReadIdx])
        {
            // Try to parse value type
            case 'n': // Type is null
            {

                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(null)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(null);
                }
                else { State = ParseState_Error; }
                bDone = true;
            } break;
            case 't': // Type is bool
            {
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(true)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(true);
                    CurrObject->Value.Type = JsonType_Bool;
                    CurrObject->Value.Bool = true;
                }
                else { State = ParseState_Error; }
                bDone = true;
            } break;
            case 'f': // Type is bool
            {
                if (TryMatchLiteral(JsonData + ReadIdx, LITERAL_VAL(false)))
                {
                    ReadIdx += LITERAL_VAL_LENGTH(false);
                    CurrObject->Value.Type = JsonType_Bool;
                    CurrObject->Value.Bool = false;
                }
                else { State = ParseState_Error; }
                bDone = true;
            } break;
            case '"': // Type is string
            {
                if (nullptr == CurrObject->Key)
                {
                    // If we haven't read a key yet, then this indicates a bug in Parse_Key
                    State = ParseState_Error;
                }
                else 
                {
                    CurrObject->Value.Type = JsonType_String;
                    char* RightQuote = nullptr;
                    if (TryReadString(JsonData + ReadIdx, &RightQuote, &CurrObject->Value.String))
                    {
                        ReadIdx = RightQuote - JsonData + 1;
                    }
                    else
                    {
                        State = ParseState_Error;
                    }
                }
                bDone = true;
            } break;
            case ':':
            case ',':
            case '\0':
            {
                State = ParseState_Error;
                bDone = true;
            } break;
        }
        if (CharIsNumeric(JsonData[ReadIdx]) ||
                JsonData[ReadIdx] == '+' ||
                JsonData[ReadIdx] == '-')
        {
            // Default number type is int
            CurrObject->Value.Type = JsonType_NumberInt;
            int BeginNumberIdx = ReadIdx++;
            while (CharIsNumeric(JsonData[ReadIdx]) ||
                    JsonData[ReadIdx] == '.')
            {
                if (JsonData[ReadIdx] == '.')
                {
                    if (JsonType_NumberFloat == CurrObject->Value.Type)
                    {
                        // Multiple .'s in literal -> report error
                        State = ParseState_Error;
                        bDone = true;
                        return ReadIdx;
                    }
                    else
                    {
                        CurrObject->Value.Type = JsonType_NumberFloat;
                    }
                }
                ReadIdx++;
            }
            char* FirstCharAfterNumber = 0;
            if (JsonType_NumberInt == CurrObject->Value.Type)
            {
                CurrObject->Value.NumberInt = strtol(JsonData + BeginNumberIdx, &FirstCharAfterNumber, 10);
            }
            else if (JsonType_NumberFloat == CurrObject->Value.Type)
            {
                char* EndFloatReadIdx = 0;
                CurrObject->Value.NumberFloat = strtod(JsonData + BeginNumberIdx, &FirstCharAfterNumber);
            }
            if (FirstCharAfterNumber)
            {
                ReadIdx = FirstCharAfterNumber - JsonData;
            }
            else
            {
                State = ParseState_Error;
            }
            bDone = true;
            break;
        }
    }
    // If we haven't hit an error, try to read chars until the next key begin (or root end)
    if (State != ParseState_Error)
    {
        bool bReading = true;
        while (bReading)
        {
            switch (JsonData[ReadIdx])
            {
                case ',': // There is another object
                {
                    CurrObject = Root->Objects.Add_RetPtr(JsonObject{});
                    State = ParseState_Key;
                    bReading = false;
                } break;
                case '}': // End of root
                {
                    // TODO: Will have to handle this once we start traversing Json objects with depth
                    State = ParseState_End;
                    bReading = false;
                } break;
                case '\0': // End of file
                {
                    State = ParseState_Error;
                    bReading = false;
                } break;
            }
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
    constexpr bool bDebugPrint = true;
    if (bDebugPrint)
    {
        auto GetStateName = [](StateType InState) -> const char*
        {
            switch (InState)
            {
                case ParseState_Root: return "Root";
                case ParseState_Key: return "Key";
                case ParseState_Value: return "Value";
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
        fprintf(stdout, "\t\tEndState: %s, EndIdx: %d '%c' (0x%x)\n",
                GetStateName(State), ReadIdx, GetPrintableChar(JsonData[ReadIdx]), JsonData[ReadIdx]);
        fprintf(stdout, "JSON Parsed:%.*s\n", ReadIdx - StartIdx, JsonData + StartIdx);
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

