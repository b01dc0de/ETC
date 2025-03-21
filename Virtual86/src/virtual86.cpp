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
    Sim86State ResultState = {};
    ResultState.InitZero();

    if (ArgCount > 1)
    {
        for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
        {
            ResultState.Sim86(ArgValues[ArgIdx], true);
        }
    }
    else
    {
        //ResultState.Sim86("input/listing_0043_immediate_movs", true);
        //ResultState.Sim86("input/listing_0044_register_movs", true);
        //ResultState.Sim86("input/listing_0045_challenge_register_movs", true);
        //ResultState.Sim86("input/listing_0046_add_sub_cmp", true);
        //ResultState.Sim86("input/listing_0048_ip_register", true);
        //ResultState.Sim86("input/listing_0049_conditional_jumps", true);

        ResultState.Sim86("input/listing_0051_memory_mov", true);
        ResultState.Sim86("input/listing_0052_memory_add_loop", true);
    }

    return 0;
}