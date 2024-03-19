#pragma once
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))
int abs(int value);
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define DIV_ROUNDUP(x, y) (((x) + (y) - 1) / (y))
#define DIV_ROUNDDOWN(x, y) (x / y)
#define BITMAP_SET_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] |= (1 << ((bit_index) % 8)))
#define BITMAP_UNSET_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] &= ~(1 << ((bit_index) % 8)))
#define BITMAP_READ_BIT(bitmap, bit_index) ((bitmap)[(bit_index) / 8] & (1 << ((bit_index) % 8)))
#define ALIGN_UP(x, base) ((x + base - 1) & ~(base - 1))
#define ALIGN_DOWN(x, base) (x & ~(base - 1))
#define EXTRACT_BITS(value, start_index, end_index) (((value) & (((1ul << ((end_index) - (start_index) + 1)) - 1) << (start_index))) >> (start_index))
bool islower(char chr);
char toupper(char chr);


typedef struct {
    atomic_flag lock;
} k_lock;

void acquire_lock(k_lock* lock);
void release_lock(k_lock* lock);