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

void DecodeAndSim(const char* FileName, bool bPrint)
{
    VirtualInstStream DecodedStream = DecodeFile86(FileName, bPrint);
    Sim86State ResultState = Sim86(&DecodedStream, bPrint);
    if (bPrint) { printf("\n"); }
    delete[] DecodedStream.Data;
}

int main(int ArgCount, const char* ArgValues[])
{
    if (ArgCount > 1)
    {
        for (int ArgIdx = 1; ArgIdx < ArgCount; ArgIdx++)
        {
            DecodeFile86(ArgValues[ArgIdx]);
        }
    }
    else
    {
        /*
        DecodeFile86("input/listing_0037_single_register_mov");
        DecodeFile86("input/listing_0038_many_register_mov");
        DecodeFile86("input/listing_0039_more_movs");
        DecodeFile86("input/listing_0040_challenge_movs");
        DecodeFile86("input/listing_0041_add_sub_cmp_jnz");
        */

        DecodeAndSim("input/listing_0043_immediate_movs", true);
        DecodeAndSim("input/listing_0044_register_movs", true);
    }

    return 0;
}