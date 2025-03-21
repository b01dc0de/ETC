#ifndef VIRTUAL86_SIM_H
#define VIRTUAL86_SIM_H

#include "virtual86_common.h"

Sim86State Sim86(VirtualInstStream *pInstStream, bool bPrint = true);

#endif // VIRTUAL86_SIM_H
