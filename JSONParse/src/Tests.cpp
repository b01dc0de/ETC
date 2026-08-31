#include "Common.h"
#include "Tests.h"

bool Helper_StringCompare(const char* Expected, char* String)
{
    if (Expected == String) { return true; }
    if (Expected && String) { return strcmp(Expected, String) == 0; }
    return false;
}

#define JSON_KEY_CHECK(Object, InKey) Assert(Helper_StringCompare(InKey, Object->Key))
#define JSON_TYPE_CHECK(Object, InType) Assert(Object->Value.Type == InType)
#define JSON_SIZE_CHECK(Object, InSize) Assert(Object->Value.List->Size == InSize)

#define JSON_TYPE_SIZE_CHECK(Object, InType, InSize)\
    JSON_TYPE_CHECK(Object, InType);\
    JSON_SIZE_CHECK(Object, InSize)

#define JSON_KEY_TYPE_SIZE_CHECK(Object, InKey, InType, InSize)\
    JSON_KEY_CHECK(Object, InKey);\
    JSON_TYPE_SIZE_CHECK(Object, InType, InSize)

#define JSON_STRING_CHECK(Object, InString)\
    JSON_TYPE_CHECK(Object, JSONType_String);\
    Assert(Helper_StringCompare(InString, Object->Value.String))

void RunTests_ExampleGlossary(JSONObject* Root)
{
    Assert(Root->Key == nullptr);
    JSON_TYPE_SIZE_CHECK(Root, JSONType_Object, 1);
    
    JSONObject* Glossary = Root->Value.List->Data[0];
    JSON_KEY_CHECK(Glossary, "glossary");
    JSON_TYPE_SIZE_CHECK(Glossary, JSONType_Object, 2);

    JSONObject* Title = Glossary->Value.List->Data[0];
    JSON_KEY_CHECK(Title, "title");
    JSON_STRING_CHECK(Title, "example glossary");

    JSONObject* GlossDiv = Glossary->Value.List->Data[1];
    JSON_KEY_CHECK(GlossDiv, "GlossDiv");
    JSON_TYPE_SIZE_CHECK(GlossDiv, JSONType_Object, 2);

    JSONObject* GlossDiv_Title = GlossDiv->Value.List->Data[0];
    JSON_KEY_CHECK(GlossDiv_Title, "title");
    JSON_STRING_CHECK(GlossDiv_Title, "S");

    JSONObject* GlossList = GlossDiv->Value.List->Data[1];
    JSON_KEY_CHECK(GlossList, "GlossList");
    JSON_TYPE_SIZE_CHECK(GlossList, JSONType_Object, 1);

    JSONObject* GlossEntry = GlossList->Value.List->Data[0];
    JSON_KEY_CHECK(GlossEntry, "GlossEntry");
    JSON_TYPE_SIZE_CHECK(GlossEntry, JSONType_Object, 7);

    JSONObject* ID = GlossEntry->Value.List->Data[0];
    JSON_KEY_CHECK(ID, "ID");
    JSON_STRING_CHECK(ID, "SGML");

    JSONObject* SortAs = GlossEntry->Value.List->Data[1];
    JSON_KEY_CHECK(SortAs , "SortAs");
    JSON_STRING_CHECK(SortAs , "SGML");

    JSONObject* GlossTerm = GlossEntry->Value.List->Data[2];
    JSON_KEY_CHECK(GlossTerm, "GlossTerm");
    JSON_STRING_CHECK(GlossTerm, "Standard Generalized Markup Language");

    JSONObject* Acronym = GlossEntry->Value.List->Data[3];
    JSON_KEY_CHECK(Acronym, "Acronym");
    JSON_STRING_CHECK(Acronym, "SGML");

    JSONObject* Abbrev = GlossEntry->Value.List->Data[4];
    JSON_KEY_CHECK(Abbrev, "Abbrev");
    JSON_STRING_CHECK(Abbrev, "ISO 8879:1986");

    JSONObject* GlossDef = GlossEntry->Value.List->Data[5];
    JSON_KEY_TYPE_SIZE_CHECK(GlossDef, "GlossDef", JSONType_Object, 2);

    JSONObject* Para = GlossDef->Value.List->Data[0];
    JSON_KEY_CHECK(Para, "para");
    JSON_STRING_CHECK(Para, "A meta-markup language, used to...");

    JSONObject* GlossSeeAlso = GlossDef->Value.List->Data[1];
    JSON_KEY_TYPE_SIZE_CHECK(GlossSeeAlso, "GlossSeeAlso", JSONType_Array, 2);

    JSONObject* GML = GlossSeeAlso->Value.List->Data[0];
    JSON_KEY_CHECK(GML, nullptr);
    JSON_STRING_CHECK(GML, "GML");

    JSONObject* XML = GlossSeeAlso->Value.List->Data[1];
    JSON_KEY_CHECK(XML, nullptr);
    JSON_STRING_CHECK(XML, "XML");

    JSONObject* GlossSee = GlossEntry->Value.List->Data[6];
    JSON_KEY_CHECK(GlossSee, "GlossSee");
    JSON_STRING_CHECK(GlossSee, "markup");
}

