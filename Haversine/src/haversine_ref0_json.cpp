#include "haversine_ref0_json.h"

namespace Haversine_Ref0
{
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
    int TryReadString(char* Begin)
    {
        int ReadIdx = 0;
        while (Begin[ReadIdx] != '"')
        {
            // TODO: Make this _way_ more robust
            if (Begin[ReadIdx] == '\0') { return -1; }
            ReadIdx++;
        }
        return ReadIdx;
    }

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
                CurrObject = Root->Objects.Add_Ref(JsonObject{});
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
                ReadIdx++;
                { // Read key:
                    int KeyNameIdx = ReadIdx;
                    while (JsonData[KeyNameIdx] != '"')
                    {
                        KeyNameIdx++;
                    }
                    int KeyNameLength = KeyNameIdx - ReadIdx;
                    CurrObject->Key = new char[KeyNameLength+1];
                    memcpy(CurrObject->Key, JsonData + ReadIdx, KeyNameLength);
                    CurrObject->Key[KeyNameLength] = '\0';
                    ReadIdx = KeyNameIdx + 1;
                }
                { // Read until ':' or ','
                    int ValueIdx = ReadIdx;
                    bool bValueStart = false;
                    while (!bValueStart)
                    {
                        switch (JsonData[ValueIdx])
                        {
                            case ':':
                            {
                                State = ParseState_Value;
                                bValueStart = true;
                            } break;
                            case ',':
                            {
                                // We were actually reading the string value here, _not_ the key, so swap them
                                CurrObject->Value.Type = JsonType_String;
                                CurrObject->Value.String = CurrObject->Key;
                                CurrObject->Key = nullptr;
                                // TODO: This will require adjusting when we start adding depth traversal
                                CurrObject = Root->Objects.Add_Ref(JsonObject{});
                                bValueStart = true;
                                // Keep state the same, try to read the next objects' key
                            } break;
                            default: { } break;
                        }
                        ValueIdx++;
                    }
                    ReadIdx = ValueIdx;
                    bDone = true;
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
                    ReadIdx++;
                    CurrObject->Value.Type = JsonType_String;
                    int ValueStringLength = TryReadString(JsonData + ReadIdx);
                    if (ValueStringLength <= -1) { State = ParseState_Error; }
                    else if (ValueStringLength == 0) // Empty string
                    {
                        CurrObject->Value.String = "";
                        ReadIdx += ValueStringLength + 1;
                    }
                    else // Valid value read
                    {
                        CurrObject->Value.String = new char[ValueStringLength + 1];
                        memcpy(CurrObject->Value.String, JsonData + ReadIdx, ValueStringLength);
                        CurrObject->Value.String[ValueStringLength] = '\0';
                        ReadIdx += ValueStringLength + 1;
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
                    CurrObject = &Root->Objects[Root->Objects.Add(JsonObject{})];
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

