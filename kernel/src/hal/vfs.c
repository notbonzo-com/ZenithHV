#include "vfs.h"
#include <drivers/debug/e9.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;
    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
            //VGA_putc(data[i]); Maybe one day
            debug_putc(data[i]);
        return size;

    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
            debug_putc(data[i]);
        return size;

    default:
        return -1;
    }
}