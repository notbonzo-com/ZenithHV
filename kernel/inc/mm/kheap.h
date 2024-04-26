#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void initKheap(size_t maxSize);
extern uintptr_t kheap_base;

#ifdef __cplusplus
}
#endif