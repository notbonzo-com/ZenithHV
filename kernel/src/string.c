#include <stddef.h>
#include <stdint.h>
#include "string.h"

#include <mem.h>
//#include <mm/kalloc.h>

int strcmp(const char* str1, const char* str2)
{
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;

}
int strncmp(const char* str1, const char* str2, size_t n)
{
    while (n-- && *str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;

}
char* strcpy(char* dest, const char* src)
{
    char* ret = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;

}
char* strncpy(char* dest, const char* src, size_t n)
{
    char* ret = dest;
    while (n-- && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}
size_t strlen(const char* str)
{
    size_t ret = 0;
    while (*str++) {
        ret++;
    }
    return ret;
}
char* strcat(char* dest, const char* src)
{
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}
char* strncat(char* dest, const char* src, size_t n)
{
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while (n-- && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}
char* strchr(const char* str, int c)
{
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;

}
char* strrchr(const char* str, int c)
{
    char* ret = NULL;
    while (*str) {
        if (*str == c) {
            ret = (char*)str;
        }
        str++;
    }
    return ret;
}
char* strstr(const char* haystack, const char* needle)
{
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) {
            return (char*)haystack;
        }
        haystack++;
    }
    return NULL;
}
char* strtok(char* str, const char* delim)
{
    static char* last = NULL;
    if (str) {
        last = str;
    }
    if (!last) {
        return NULL;
    }
    char* ret = last;
    while (*last) {
        const char* d = delim;
        while (*d) {
            if (*last == *d) {
                *last = '\0';
                last++;
                return ret;
            }
            d++;
        }
        last++;
    }
    last = NULL;
    return ret;
}
char* strdup(const char* str)
{
    // size_t len = strlen(str);
    // char* ret = kalloc(len + 1);
    // // Check for null
    // if (!ret) {
    //     return NULL;
    // }
    // strcpy(ret, str);
    // return ret;
    (void)str;
    return NULL;
}
char* strndup(const char* str, size_t n)
{
    // size_t len = strlen(str);
    // if (len > n) {
    //     len = n;
    // }
    // char* ret = kalloc(len + 1);
    // // Check for null
    // if (!ret) {
    //     return NULL;
    // }
    // strncpy(ret, str, n);
    // return ret;
    (void)str; (void)n;
    return NULL;
}
void strrev(char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}
char* strlower(char* str)
{
    char* ret = str;
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str += 'a' - 'A';
        }
        str++;
    }
    return ret;
}
char* strupper(char* str)
{
    char* ret = str;
    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str -= 'a' - 'A';
        }
        str++;
    }
    return ret;
}
void strtrim(char* str)
{
    strtriml(str);
    strtrimr(str);
}
void strtriml(char* str)
{
    size_t len = strlen(str);
    size_t i = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }
    if (i) {
        memmove(str, str + i, len - i + 1);
    }
}
void strtrimr(char* str)
{
    size_t len = strlen(str);
    size_t i = len - 1;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i--;
    }
    str[i + 1] = '\0';
}
void strtrimc(char* str, char c)
{
    strtrimcl(str, c);
    strtrimcr(str, c);
}
void strtrimcl(char* str, char c)
{
    size_t len = strlen(str);
    size_t i = 0;
    while (str[i] == c) {
        i++;
    }
    if (i) {
        memmove(str, str + i, len - i + 1);
    }
}
void strtrimcr(char* str, char c)
{
    size_t len = strlen(str);
    size_t i = len - 1;
    while (str[i] == c) {
        i--;
    }
    str[i + 1] = '\0';
}
void strtrimw(char* str) 
{
    strtrimwl(str);
    strtrimwr(str);
}
void strtrimwl(char* str)
{
    size_t len = strlen(str);
    size_t i = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }
    if (i) {
        memmove(str, str + i, len - i + 1);
    }
    i = 0;
    while (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
        i++;
    }
    str[i] = '\0';
}
void strtrimwr(char* str)
{
    size_t len = strlen(str);
    size_t i = len - 1;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i--;
    }
    str[i + 1] = '\0';
    i = len - 1;
    while (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
        i--;
    }
    str[i + 1] = '\0';
}