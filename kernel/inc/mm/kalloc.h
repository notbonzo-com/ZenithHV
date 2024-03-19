#pragma once

#include <stddef.h>
#include <stdint.h>

extern int kalloc_lock();
extern int kalloc_unlock();
extern void* kalloc_alloc(size_t);
extern int kalloc_free(void*,size_t);

extern void *kmalloc(size_t);
extern void *krealloc(void *, size_t);
extern void *kcalloc(size_t, size_t);
extern void kfree(void *ptr);
