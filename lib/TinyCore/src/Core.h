
#ifndef CORE_H
#define CORE_H

#include "stdint.h"
#include "stdbool.h"

void DelayMS(uint32_t ms);

void DelayUS(uint32_t us);

// Init core subsystems like TCP stack, or non-volatile storage
bool InitializeSubSystems();

bool HandleOutput(int errorCode, const char* file, const char* function, int line);

#define HANDLE_OUTPUT(er) HandleOutput(er, __FILE__, __func__,  __LINE__)

#endif