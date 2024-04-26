#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define DIV_ROUNDUP(x, y) (((x) + (y) - 1) / (y))
#define DIV_ROUNDDOWN(x, y) (x / y)
#define BITMAP_SET_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] |= (1 << ((bit_index) % 8)))
#define BITMAP_UNSET_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] &= ~(1 << ((bit_index) % 8)))
#define BITMAP_READ_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] & (1 << ((bit_index) % 8)))
#define ALIGN_UP(x, base) (((x) + (base) - 1) & ~((base) - 1))
#define ALIGN_DOWN(x, base) ((x) & ~((base) - 1))
#define EXTRACT_BITS(value, start_index, end_index) (((value) & (((1ul << ((end_index) - (start_index) + 1)) - 1) << (start_index))) >> (start_index))


#ifdef __cplusplus
}
#endif