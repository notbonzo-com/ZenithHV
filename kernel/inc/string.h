#pragma once
#include <stdint.h>
#include <stddef.h>

// string functions
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
size_t strlen(const char* str);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);
char* strtok(char* str, const char* delim);
char* strdup(const char* str);
char* strndup(const char* str, size_t n);
void strrev(char* str);
char* strlower(char* str);
char* strupper(char* str);
void strtrim(char* str);
void strtriml(char* str);
void strtrimr(char* str);
void strtrimc(char* str, char c);
void strtrimcl(char* str, char c);
void strtrimcr(char* str, char c);
void strtrimw(char* str);
void strtrimwl(char* str);
void strtrimwr(char* str);