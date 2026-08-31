#include "Common.h"
#include "Tests.h"

bool Helper_StringCompare(const char* Expected, char* String)
{
    if (Expected == String) { return true; }
    if (Expected && String) { return strcmp(Expected, String) == 0; }
    return false;
}

bool Helper_DumbFloatCompare(double Expected, double Value)
{
    // NOTE(CKA): This is not meant to be a _good_ float compare by any metric!
    constexpr double Epsilon = 0.000001;

    double Diff = Expected - Value;
    if (Diff < 0.0) { Diff = -Diff; }

    return Diff < Epsilon;
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
    Assert(Object);\
    Assert(Helper_StringCompare(InString, Object->GetString()))

#define JSON_INT_CHECK(Object, InInteger)\
    Assert(Object);\
    Assert(Object->GetInt() == InInteger)

#define JSON_FLOAT_CHECK(Object, InFloat)\
    Assert(Object);\
    Assert(Helper_DumbFloatCompare(InFloat, Object->GetFloat()));

#define JSON_BOOL_CHECK(Object, InBool)\
    Assert(Object);\
    Assert(Object->GetBool() == InBool)

#define JSON_NULL_CHECK(Object)\
    Assert(Object);\
    Assert(Object->IsNull())


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

void RunTests_ExampleSimpleLists(JSONObject* Root)
{
    Assert(Root->Key == nullptr);
    JSON_TYPE_SIZE_CHECK(Root, JSONType_Object, 2);

    JSONObject* ArrayOfArrays = Root->GetProperty("ArrayOfArrays");
    Assert(ArrayOfArrays);
    JSON_TYPE_SIZE_CHECK(ArrayOfArrays, JSONType_Array, 4);

    JSONObject* Arr_0 = ArrayOfArrays->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_0, nullptr, JSONType_Array, 5);
    JSONObject* Arr_0_0 = Arr_0->GetItem(0);
    JSONObject* Arr_0_1 = Arr_0->GetItem(1);
    JSONObject* Arr_0_2 = Arr_0->GetItem(2);
    JSONObject* Arr_0_3 = Arr_0->GetItem(3);
    JSONObject* Arr_0_4 = Arr_0->GetItem(4);
    JSON_INT_CHECK(Arr_0_0, 1);
    JSON_INT_CHECK(Arr_0_1, 2);
    JSON_INT_CHECK(Arr_0_2, 3);
    JSON_INT_CHECK(Arr_0_3, 4);
    JSON_INT_CHECK(Arr_0_4, 5);

    JSONObject* Arr_1 = ArrayOfArrays->GetItem(1);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_1, nullptr, JSONType_Array, 1);
    JSONObject* Arr_1_Nest = Arr_1->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_1_Nest, nullptr, JSONType_Array, 2);
    JSONObject* Arr_1_Nest_0 = Arr_1_Nest->GetItem(0);
    JSON_STRING_CHECK(Arr_1_Nest_0, "hello");
    JSONObject* Arr_1_Nest_1 = Arr_1_Nest->GetItem(1);
    JSON_STRING_CHECK(Arr_1_Nest_1, "goodbye");

    JSONObject* Arr_2 = ArrayOfArrays->GetItem(2);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_2, nullptr, JSONType_Array, 1);
    JSONObject* Arr_2_Nest = Arr_2->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_2_Nest, nullptr, JSONType_Object, 1);
    JSONObject* Arr_2_Nest_0 = Arr_2_Nest->GetProperty("testKey");
    JSON_STRING_CHECK(Arr_2_Nest_0, "testValue");

    JSONObject* Arr_3 = ArrayOfArrays->GetItem(3);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3, nullptr, JSONType_Array, 1);
    JSONObject* Arr_3_Nest0 = Arr_3->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3_Nest0, nullptr, JSONType_Array, 1);
    JSONObject* Arr_3_Nest1 = Arr_3_Nest0->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3_Nest1, nullptr, JSONType_Array, 1);
    JSONObject* Arr_3_Nest2 = Arr_3_Nest1->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3_Nest2, nullptr, JSONType_Array, 1);
    JSONObject* Arr_3_Nest3 = Arr_3_Nest2->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3_Nest3, nullptr, JSONType_Array, 1);
    JSONObject* Arr_3_Nest4 = Arr_3_Nest3->GetItem(0);
    JSON_KEY_TYPE_SIZE_CHECK(Arr_3_Nest4, nullptr, JSONType_Array, 0);


    JSONObject* ObjectOfArrays = Root->GetProperty("ObjectOfArrays");
    JSON_KEY_TYPE_SIZE_CHECK(ObjectOfArrays, "ObjectOfArrays", JSONType_Object, 3);

    JSONObject* Obj_0 = ObjectOfArrays->GetProperty("Array0");
    JSON_KEY_TYPE_SIZE_CHECK(Obj_0, "Array0", JSONType_Array, 3);
    JSONObject* Obj_0_0 = Obj_0->GetItem(0);
    JSON_STRING_CHECK(Obj_0_0, "100");
    JSONObject* Obj_0_1 = Obj_0->GetItem(1);
    JSON_STRING_CHECK(Obj_0_1, "200");
    JSONObject* Obj_0_2 = Obj_0->GetItem(2);
    JSON_STRING_CHECK(Obj_0_2, "300");

    JSONObject* Obj_1 = ObjectOfArrays->GetProperty("Array1");
    JSON_KEY_TYPE_SIZE_CHECK(Obj_1, "Array1", JSONType_Array, 3);
    JSONObject* Obj_1_0 = Obj_1->GetItem(0);
    JSON_BOOL_CHECK(Obj_1_0, true);
    JSONObject* Obj_1_1 = Obj_1->GetItem(1);
    JSON_BOOL_CHECK(Obj_1_1, false);
    JSONObject* Obj_1_2 = Obj_1->GetItem(2);
    JSON_NULL_CHECK(Obj_1_2);

    JSONObject* Obj_2 = ObjectOfArrays->GetProperty("Array2");
    JSON_KEY_TYPE_SIZE_CHECK(Obj_2, "Array2", JSONType_Array, 5);
    JSONObject* Obj_2_0 = Obj_2->GetItem(0);
    JSON_FLOAT_CHECK(Obj_2_0, 0.1);
    JSONObject* Obj_2_1 = Obj_2->GetItem(1);
    JSON_FLOAT_CHECK(Obj_2_1, 0.2);
    JSONObject* Obj_2_2 = Obj_2->GetItem(2);
    JSON_FLOAT_CHECK(Obj_2_2, 0.3);
    JSONObject* Obj_2_3 = Obj_2->GetItem(3);
    JSON_FLOAT_CHECK(Obj_2_3, 0.4);
    JSONObject* Obj_2_4 = Obj_2->GetItem(4);
    JSON_FLOAT_CHECK(Obj_2_4, 0.5);
}

