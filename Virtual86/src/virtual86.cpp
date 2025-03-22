#include "virtual86_common.h"
#include "virtual86_decode.h"
#include "virtual86_estperf.h"
#include "virtual86_print.h"
#include "virtual86_sim.h"

#ifndef UNITY_BUILD
#define UNITY_BUILD (0)
#endif // UNITY_BUILD
#if UNITY_BUILD
#include "virtual86_common.cpp"
#include "virtual86_decode.cpp"
#include "virtual86_estperf.cpp"
#include "virtual86_print.cpp"
#include "virtual86_sim.cpp"
#endif // UNITY_BUILD

int main(int ArgCount, const char* ArgValues[])
{
    Sim86State ResultState = {};
    ResultState.InitZero();
    bool bPrint = true;

    if (ArgCount > 1)
    {
        for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
        {
            ResultState.Sim86(ArgValues[ArgIdx], bPrint);
        }
    }
    else
    {
        //ResultState.Sim86("input/listing_0043_immediate_movs", bPrint);
        //ResultState.Sim86("input/listing_0044_register_movs", bPrint);
        //ResultState.Sim86("input/listing_0045_challenge_register_movs", bPrint);
        //ResultState.Sim86("input/listing_0046_add_sub_cmp", bPrint);
        //ResultState.Sim86("input/listing_0048_ip_register", bPrint);
        //ResultState.Sim86("input/listing_0049_conditional_jumps", bPrint);

        //ResultState.Sim86("input/listing_0051_memory_mov", bPrint);
        //ResultState.Sim86("input/listing_0052_memory_add_loop", bPrint);

        //ResultState.Sim86Dump("input/listing_0054_draw_rectangle", "output_listing_0054_draw_rectangle.data");
        //ResultState.Sim86Dump("input/listing_0055_challenge_rectangle", "output_listing_0055_challenge_rectangle.data");

        Est86Cycles("input/listing_0056_estimating_cycles");
    }

    return 0;
}