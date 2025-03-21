#ifndef VIRTUAL86_SIM_H
#define VIRTUAL86_SIM_H

#include "virtual86_common.h"

struct Sim86_State
{
    u16 Registers[8];
};

Sim86_State Sim86(VirtualInstStream *pInstStream, bool bPrint = true);

#endif // VIRTUAL86_SIM_H
