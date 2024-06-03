#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000ul

namespace pmm
{
void *claim(size_t count);
void free(void *ptr, size_t count);


}