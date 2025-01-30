#pragma once

#include <stddef.h>
#include <stdint.h>

namespace kheap {

    extern uintptr_t base;

    void init(size_t max_heap_size_pages);
    struct heap_bounds {
        uintptr_t base;
        size_t size;
    };

    extern heap_bounds heap_bounds;

}