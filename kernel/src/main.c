#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <mem.h>
#include <drivers/debug/e9.h>
#include <drivers/visual/main.h>
#include <drivers/visual/font.h>
#include <drivers/visual/graphic.h>
#include <hal/hal.h>
#include <string.h>
#include <stdio.h>

#define forever(a) forever_start: a ; goto forever_start

/**
 * @brief The entry point of the kernel
 * 
 */
void _start(void) {
    VISUAL_Init();
    HAL_Init(); 

    // Test case 1: Reallocating smaller size
    size_t size1 = 4;
    void* ptr1 = kalloc(size1);
    memcpy(ptr1, "abcd", size1);
    log_info("Test", "Before reallocation: %s\n", (char*)ptr1);
    ptr1 = krealloc(ptr1, 2);
    log_info("Test", "After reallocation: %s\n", (char*)ptr1);
    kfree(ptr1);

    // Test case 2: Reallocating larger size
    size_t size2 = 4;
    void* ptr2 = kalloc(size2);
    memcpy(ptr2, "abcd", size2);
    log_info("Test", "Before reallocation: %s\n", (char*)ptr2);
    ptr2 = krealloc(ptr2, 8);
    log_info("Test", "After reallocation: %s\n", (char*)ptr2);
    kfree(ptr2);

    // Test case 3: Reallocating same size
    size_t size3 = 4;
    void* ptr3 = kalloc(size3);
    memcpy(ptr3, "abcd", size3);
    log_info("Test", "Before reallocation: %s\n", (char*)ptr3);
    ptr3 = krealloc(ptr3, 4);
    log_info("Test", "After reallocation: %s\n", (char*)ptr3);
    kfree(ptr3);



    forever(asm ("hlt"));
}