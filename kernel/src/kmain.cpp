#include <stdint.h>

#include <gdt.hpp>
#include <pmm.hpp>

#include <gtest>
#include <string>

#include <kmalloc>
#include <vector>

extern "C" uint8_t kmain(void)
{
    return 0;
}