#pragma once
#include <stdbool.h>

void cpuid_getCpuVendor(char str[static 13]);
bool cpuid_isApicSupported(void);