#ifndef VIRTUAL86_ESTPERF_H
#define VIRTUAL86_ESTPERF_H

#include "virtual86_common.h"

int EstEACalcCycles(Operand* pOp);
int EstInstCycles(VirtualInst* pInst);
void Est86Cycles(const char* FileName);

#endif // VIRTUAL86_ESTPERF_H