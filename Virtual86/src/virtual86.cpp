#include "virtual86_common.h"
#include "virtual86_decode.h"
#include "virtual86_print.h"
#include "virtual86_sim.h"

#ifndef UNITY_BUILD
#define UNITY_BUILD (0)
#endif // UNITY_BUILD
#if UNITY_BUILD
#include "virtual86_common.cpp"
#include "virtual86_decode.cpp"
#include "virtual86_print.cpp"
#include "virtual86_sim.cpp"
#endif // UNITY_BUILD

int main(int ArgCount, const char* ArgValues[])
{
    if (ArgCount > 1)
    {
        Sim86State Result = {}; Result.Clear();
        for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
        {
            Result = Sim86(ArgValues[ArgIdx], true);
        }
    }
    else
    {
        Sim86State Result = {};
        //Result = Sim86("input/listing_0043_immediate_movs", true);
        //Result = Sim86("input/listing_0044_register_movs", true);
        //Result = Sim86("input/listing_0045_challenge_register_movs", true);
        //Result = Sim86("input/listing_0046_add_sub_cmp", true);
        Result = Sim86("input/listing_0048_ip_register", true);
        Result = Sim86("input/listing_0049_conditional_jumps", true);
    }

    return 0;
}