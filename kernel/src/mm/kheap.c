#include <mm/kheap.h>
#include <mm/vmm.h>
#include <x86/idt.h>
#include <x86/page.h>
#include <x86/smp.h>
#include <mem.h>
#include <kprintf.h>

uint64_t kernel_heap_max_size_pages = 0x0;
uintptr_t kheap_base = 0x0;
uint8_t *kernel_heap_bitmap = 0x0;

static klockc alloc_lock;

void initKheap(size_t maxSize)
{
    kheap_base = ALIGN_UP(highest_address_memmap + hhdm->offset, PAGE_SIZE);

    kernel_heap_max_size_pages = maxSize;

    kernel_heap_bitmap = allocate_pages(DIV_ROUNDUP(DIV_ROUNDUP(kernel_heap_max_size_pages, PAGE_SIZE), 8));

    if (!kernel_heap_bitmap) {
        debugf("Failed to allocate kernel heap bitmap");
                kpanic();
    }
    kernel_heap_bitmap += hhdm->offset;
    memset(kernel_heap_bitmap, 0x00, PAGE_SIZE);

    debugf("Kernel heap initialized at 0x%p", kheap_base);
    debugf("Available dynamic memory: %lu MiB", (kernel_heap_max_size_pages * PAGE_SIZE) / (1024 * 1024));
}

void *get_page_at(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernel_heap_bitmap, (address - kheap_base) / PAGE_SIZE) != 0) {
        debugf("Page already in use");
    }
    void *new_page = allocate_pages(1);
    if (new_page == NULL) {
        return NULL;
    }
    mapPage(&kernelPmc, address, (uintptr_t)new_page, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE | PTE_BIT_READ_WRITE);
    BITMAP_SET_BIT(kernel_heap_bitmap, (address - kheap_base) / PAGE_SIZE);

    return (void *)address;
}

void *return_page_at(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernel_heap_bitmap, (address - kheap_base) / PAGE_SIZE) == 0) {
        return 0;
    }
    BITMAP_UNSET_BIT(kernel_heap_bitmap, (address - kheap_base) / PAGE_SIZE);

    unmapPage(&kernelPmc, address, 1);

    return 0;
}


// Liballoc functions
extern int liballoc_lock() {
    acquire_lock(&alloc_lock);
    return 0;
}

extern int liballoc_unlock() {
    release_lock(&alloc_lock);
    return 0;
}

extern void* liballoc_alloc(int c) {
    size_t count = c;
    size_t idx = 0, pages_found = 0;
    while (idx < kernel_heap_max_size_pages) {
        if (BITMAP_READ_BIT(kernel_heap_bitmap, idx) == 0) {
            pages_found++;
            if (pages_found == count) {
                for (size_t j = (idx + 1) - count; j <= idx; j++) {
                    if (!get_page_at(kheap_base + (j * PAGE_SIZE))) {
                        return NULL;
                    }
                }
                return (void *)((((idx + 1ul) - count) * PAGE_SIZE) + kheap_base);
            }
        } else {
            pages_found = 0;
        }
        idx++;
    }

    debugf("Kernel heap requested pages: %lu not available\n", count);
    return NULL;
}

extern int liballoc_free(void* address, int count) {
    for (int i = 0; i < count; i++) {
        return_page_at(((uintptr_t)address + (i * PAGE_SIZE)));
    }
    return 1;
}