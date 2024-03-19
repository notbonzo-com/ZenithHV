#include <stdarg.h>
#include <hal/vfs.h>

#include <debug.h>
#include <stdio.h>

static const char* const g_LogSeverityColors[] =
{
    [LVL_DEBUG]        = "\033[2;37m",
    [LVL_INFO]         = "\033[37m",
    [LVL_WARN]         = "\033[1;33m",
    [LVL_ERROR]        = "\033[1;31m",
    [LVL_CRITICAL]     = "\033[1;37;41m",
    [LVL_PASS]         = "\033[1;32m"
};

static const char* const g_ColorReset = "\033[0m";

void logf(const char* module, DebugLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < MIN_LOG_LEVEL)
        return;

    debugs(g_LogSeverityColors[level]);
    debugs("[");
    debugs(module);
    debugs("] ");
    vfprintf(VFS_FD_DEBUG, fmt, args);
    debugs(g_ColorReset);
    debugs("\n");

    va_end(args);  
}