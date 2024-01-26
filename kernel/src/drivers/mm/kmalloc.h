#pragma once

#include <stddef.h>
#include <stdint.h>

void x86_64_KHeap_Init(size_t max_heap_size_pages);

int KAlloc_GetLock();
void KAlloc_ReleaseLock();
void* KAlloc_Alloc(size_t size);
int KAlloc_Free(void* ptr, size_t size);
void* KAlloc_Ralloc(void* ptr, size_t size);


#define kalloc(size) KAlloc_Alloc(size)
#define kfree(ptr) KAlloc_Free(ptr, 0)
#define krealloc(ptr, size) KAlloc_Ralloc(ptr, size)