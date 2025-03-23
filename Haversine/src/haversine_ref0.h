#ifndef HAVERSINE_REF0_H
#define HAVERSINE_REF0_H

/*
 * NOTE:
 *      These functions are supposed to be unchanged after they
 *      are implemented for the first time, and future iterations
 *      should be in separate files, this is in order to show
 *      progress related to performance/optimization from following
 *      Casey Muratori's Performance-Aware Programming course
 */

#include "haversine_common.h"

namespace Haversine_Ref0
{
    ListType GenerateDataBinary(int PairCount, int Seed);
    void WriteDataAsJSON(ListType PairList, const char* OutputFileName);
}

#endif // HAVERSINE_REF0_H

