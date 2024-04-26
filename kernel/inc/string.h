#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#error "string.h not implemented"

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strpbrk(const char *s, const char *accept);
char *strtok(char *str, const char *delim);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *strerror(int errnum);

#ifdef __cplusplus
}
#endif