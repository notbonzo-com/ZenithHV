#include <hal/vfs.h>
#include <x86/e9.h>
#include <vga/font.h>

#include <binary.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;
    case VFS_FD_STDOUT:
        for (size_t i = 0; i < size; i++)
            VISUAL_putc(data[i]);
        return size;

    case VFS_FD_STDERR:
    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
            debugPutc(data[i]);
        return size;

    default:
        return -1;
    }
}