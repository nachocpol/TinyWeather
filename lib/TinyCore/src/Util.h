#ifndef UTIL_H
#define UTIL_H

#include "stdint.h"

// Returns current system time in miliseconds
uint64_t GetSystemMS();

void StrToArray(const char* input, uint8_t* output, uint8_t outputMaxLen);

void ArrayToStr(uint8_t* input, char* output, uint8_t outputMaxLen);

#endif