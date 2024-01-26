#include "kmalloc.h"
#include "pg.h"
#include <limine.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <binary.h>
#include <string.h>
#include <mem.h>
#include <drivers/mm/vm.h>
#include <drivers/debug/e9.h>

int KAlloc_GetLock()
{
    asm ("cli");
    return 0;
}
void KAlloc_ReleaseLock()
{
    asm ("sti");
}

uint64_t kernel_heap_max_size_pages = 0x0;
uintptr_t kernel_heap_base_address = 0x0;
uint8_t *kernel_heap_bitmap = 0x0;

void x86_64_KHeap_Init(size_t max_heap_size_pages)
{
    kernel_heap_base_address = ALIGN_UP(pgm_highest_address_memmap + hhdm->offset, PAGE_SIZE);
    kernel_heap_max_size_pages = max_heap_size_pages;

    kernel_heap_bitmap = x86_64_pgm_claim_page(DIV_ROUNDUP(DIV_ROUNDUP(kernel_heap_max_size_pages, PAGE_SIZE), 8));
    if (!kernel_heap_bitmap) {
        log_crit("Kernel Heap", "Failed to allocate kernel heap bitmap");
        asm volatile("cli\n hlt");
    }
    kernel_heap_bitmap += hhdm->offset;
    memset(kernel_heap_bitmap, 0x00, PAGE_SIZE);

    log_debug("Kernel Heap", "Kernel heap initialized at 0x%lx with size 0x%lx", kernel_heap_base_address, kernel_heap_max_size_pages * PAGE_SIZE);
}

void *getPageAt(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernel_heap_bitmap, (address - kernel_heap_base_address) / PAGE_SIZE) != 0) {
        log_crit("Kernel Heap", "Attempted to get page at 0x%lx, but it is free", address);
        asm volatile("cli\n hlt");
    }

    void *new_page = x86_64_pgm_claim_page(1);
    if (!new_page) {
        log_crit("Kernel Heap", "Failed to claim page for kernel heap");
        asm volatile("cli\n hlt");
    }
    x86_64_VM_MapSP(&kernel_pmc, address, (uintptr_t)new_page, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE | PTE_BIT_READ_WRITE);
    BITMAP_SET_BIT(kernel_heap_bitmap, (address - kernel_heap_base_address) / PAGE_SIZE);
    
    return (void *)address;
}

void *returnPageAt(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernel_heap_bitmap, (address - kernel_heap_base_address) / PAGE_SIZE) == 0) {
        return 0;
    }

    BITMAP_UNSET_BIT(kernel_heap_bitmap, (address - kernel_heap_base_address) / PAGE_SIZE);
    x86_64_VM_UnmapSP(&kernel_pmc, address, 1);

    return 0;
}

void* KAlloc_Alloc(size_t count) {
    size_t idx = 0, pages_found = 0;
    while (idx < kernel_heap_max_size_pages) {
        // log_debug("Kernel Heap", "Checking page %lu", idx);
        if (BITMAP_READ_BIT(kernel_heap_bitmap, idx) == 0) {
            pages_found++;
            if (pages_found == count) {
                for (size_t j = (idx + 1) - count; j <= idx; j++) {
                    if (!getPageAt(kernel_heap_base_address + (j * PAGE_SIZE))) {
                        return NULL;
                    }
                }
                return (void *)((((idx + 1ul) - count) * PAGE_SIZE) + kernel_heap_base_address);
            }
        } else {
            pages_found = 0;
        }
        idx++;
    }
    log_err("Kernel Heap", "Requested %lu pages, but not enough available", count);
    return NULL;
}

int KAlloc_Free(void* ptr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        returnPageAt(((uintptr_t)ptr + (i * PAGE_SIZE)));
    }
    log_debug("Kernel Heap", "Freed %lu pages from kernel heap at 0x%016lX", size, (uintptr_t)ptr);

    return 1;
}

void* KAlloc_Ralloc(void* ptr, size_t size)
{
    log_debug("Kernel Heap", "Reallocating %lu pages at 0x%016lX", size, (uintptr_t)ptr);
    uintptr_t new_ptr = (uintptr_t)KAlloc_Alloc(size);
    if (!new_ptr) {
        log_err("Kernel Heap", "Failed to reallocate %lu pages at 0x%016lX", size, (uintptr_t)ptr);
        return NULL;
    }
    memcpy((void *)new_ptr, ptr, size * PAGE_SIZE);
    KAlloc_Free(ptr, size);
    return (void *)new_ptr;
}