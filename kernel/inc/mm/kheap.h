#pragma once

#include <stddef.h>
#include <stdint.h>

extern uintptr_t kernelHeapBaseAddress;

void initKernelHeap(size_t max_heap_size_pages);