#ifndef HAVERSINE_REF0_JSON_H
#define HAVERSINE_REF0_JSON_H

#include "haversine_common.h"
#include "haversine_ref0.h"

namespace Haversine_Ref0
{
    HList ParseJSON(FileContentsT& InputFile);
    void TraceTokens(char* JsonData, int DataLength);
}

#endif // HAVERSINE_REF0_JSON_H

