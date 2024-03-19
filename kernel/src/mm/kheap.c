#include <limine.h>
#include <mm/kheap.h>
#include <mm/vm.h>
#include <mem.h>
#include <string.h>
#include <binary.h>
#include <debug.h>
#include <mm/page.h>
#include <x86/int.h>
#include <hal/vfs.h>

#include <stdio.h>

uint64_t kernelHeapMaxSizePages = 0x0;
uintptr_t kernelHeapBaseAddress = 0x0;
uint8_t *kernelHeapBitmap = 0x0;

static k_lock malloc_lock;

void initKernelHeap(size_t max_heap_size_pages)
{
    kernelHeapBaseAddress = ALIGN_UP(pageHighestAddressMemmap + hhdm->offset, PAGE_SIZE);

    kernelHeapMaxSizePages = max_heap_size_pages;

    kernelHeapBitmap = pgmClaimPage(DIV_ROUNDUP(DIV_ROUNDUP(kernelHeapMaxSizePages, PAGE_SIZE), 8));
    if (!kernelHeapBitmap) {
        panic(NULL, 0, "no memory (for kernel heap bitmap)\n");
    }
    kernelHeapBitmap += hhdm->offset;
    memset(kernelHeapBitmap, 0x00, PAGE_SIZE);

    log_debug("Kernel Heap", "Base Address: 0x%x", kernelHeapBaseAddress);
    log_debug("Kernel Heap", "Max Size: %lu MiB", kernelHeapMaxSizePages/ (1024 * 1024));
}

void *get_page_at(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernelHeapBitmap, (address - kernelHeapBaseAddress) / PAGE_SIZE) != 0) {
        panic(NULL, 0, "trying to allocate heap page that was never freed\n");
    }
    void *new_page = pgmClaimPage(1);
    if (new_page == NULL) {
        return NULL;
    }
    vmMapSp(&kernelPmc, address, (uintptr_t)new_page, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE | PTE_BIT_READ_WRITE);
    BITMAP_SET_BIT(kernelHeapBitmap, (address - kernelHeapBaseAddress) / PAGE_SIZE);

    return (void *)address;
}

void *return_page_at(uintptr_t address)
{
    if (BITMAP_READ_BIT(kernelHeapBitmap, (address - kernelHeapBaseAddress) / PAGE_SIZE) == 0) {
        return 0;
    }
    BITMAP_UNSET_BIT(kernelHeapBitmap, (address - kernelHeapBaseAddress) / PAGE_SIZE);

    vmUnmapSp(&kernelPmc, address, 1);

    return 0;
}

extern int kalloc_lock() {
    acquire_lock(&malloc_lock);
    return 0;
}

extern int kalloc_unlock() {
    release_lock(&malloc_lock);
    return 0;
}

extern void* kalloc_alloc(int c) {
    size_t count = c;
    size_t idx = 0, pages_found = 0;
    while (idx < kernelHeapMaxSizePages) {
        if (BITMAP_READ_BIT(kernelHeapBitmap, idx) == 0) {
            pages_found++;
            if (pages_found == count) {
                for (size_t j = (idx + 1) - count; j <= idx; j++) {
                    if (!get_page_at(kernelHeapBaseAddress + (j * PAGE_SIZE))) {
                        return NULL;
                    }
                }
                return (void *)((((idx + 1ul) - count) * PAGE_SIZE) + kernelHeapBaseAddress);
            }
        } else {
            pages_found = 0;
        }   
        idx++;
    }

    log_crit("Kernel Heap", "Kernel heap requested pages: %lu not available", count); fprintf(VFS_FD_DEBUG, "\n");
    return NULL;
}

extern int kalloc_free(void* address, int count) {
    for (int i = 0; i < count; i++) {
        return_page_at(((uintptr_t)address + (i * PAGE_SIZE)));
    }
    return 1;
}