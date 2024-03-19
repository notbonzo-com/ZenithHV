#pragma once

/**
 * @file mem.h
 * @brief Memory management
 *
*/

#include <stdint.h>
#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);