#include "e9.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <hal/vfs.h>

static const char* const g_LogSeverityColors[] =
{
    [LVL_DEBUG]        = "\033[2;37m",
    [LVL_INFO]         = "\033[37m",
    [LVL_WARN]         = "\033[1;33m",
    [LVL_ERROR]        = "\033[1;31m",
    [LVL_CRITICAL]     = "\033[1;37;41m",
};

static const char* const g_ColorReset = "\033[0m";

void debug_putc(char c)
{
    asm volatile (
        "mov $0xe9, %%dx\n"      // Load port number into dx
        "movb %0, %%al\n"        // Move the character into al
        "outb %%al, %%dx\n"      // Output the byte in al to the port in dx
        :
        : "r"(c)
        : "rax", "rdx"
    );
}


void debug_puts(const char *s)
{
    while (*s) {
        debug_putc(*s);
        s++;
    }
}

void logf(const char* module, DebugLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < MIN_LOG_LEVEL)
        return;

    debug_puts(g_LogSeverityColors[level]);
    debug_puts("[");
    debug_puts(module);
    debug_puts("] ");
    vfprintf(VFS_FD_DEBUG, fmt, args);
    debug_puts(g_ColorReset);
    debug_puts("\n");

    va_end(args);  
}