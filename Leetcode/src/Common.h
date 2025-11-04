#ifndef COMMON_H
#define COMMON_H

// Platform headers:
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// C std lib:
#include <stdio.h>

// C++ std lib:
#include <string>
#include <vector>

void Outf(const char* Fmt, ...);

#define Assert(Exp) if (!(Exp)) { __debugbreak(); }

#endif // COMMON_H
