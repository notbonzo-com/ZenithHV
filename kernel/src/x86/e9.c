#include <x86/e9.h>
#include <x86/io.h>

void debugPutc(char c)
{
    outb(0xe9, c);
}

void debugPuts(const char* str)
{
    while (*str)
    {
        debugPutc(*str++);
    }
}