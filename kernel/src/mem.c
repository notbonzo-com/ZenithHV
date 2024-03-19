#include <mem.h>

#include <stdint.h>
#include <stddef.h>

void *memset(void *s, int c, size_t n)
{
	unsigned int i;
	for ( i = 0; i < n ; i++)
		((char*)s)[i] = c;

	return s;
}
void *memcpy(void *s1, const void *s2, size_t n)
{
    char *cdest;
    char *csrc;
    unsigned int *ldest = (unsigned int*)s1;
    unsigned int *lsrc  = (unsigned int*)s2;

    while ( n >= sizeof(unsigned int) )
    {
        *ldest++ = *lsrc++;
        n -= sizeof(unsigned int);
    }

    cdest = (char*)ldest;
    csrc  = (char*)lsrc;

    while ( n > 0 )
    {
        *cdest++ = *csrc++;
        n -= 1;
    }

    return s1;
}
void *memmove(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;
    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else if (d > s) {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dest;
}
int memcmp(const void *s1, const void *s2, size_t n)
{
    const char *p1 = s1;
    const char *p2 = s2;
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}
void *memchr(const void *s, int c, size_t n)
{
    const char *p = s;
    while (n--) {
        if (*p == c)
            return (void *)p;
        p++;
    }
    return NULL;
}