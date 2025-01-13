#include <sys/mm/pmm.hh>
#include <limine.h>
#include <sys/idt.hh>
#include <cstring>
#include <macro.hh>
#include <kprintf>

namespace pmm {

    struct limine_memmap_response *memmap = nullptr;
    struct limine_hhdm_response *hhdm = nullptr;

    struct UsableEntry *usableEntries = nullptr;

    uint8_t* pageBitmap;
    uint64_t pagesUsable;
    uint64_t pagesReserved;
    uint64_t pagesTotal;
    uint64_t pagesInUse;

    uint64_t highestAddressMemmap;
    uint64_t memmapUsableEntryCount;
    uint64_t totalBytesPmmStructures;
    uint64_t bitmapSizeBytes;
    uint64_t highestAddressUsable;

    static size_t _allocator_last_index = 0;

    struct limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = nullptr
    };

    struct limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = nullptr
    };

    void init()
    {
        kprintf(" -> PMM initialization started\n");
        memmap = memmap_request.response;
        hhdm = hhdm_request.response;

        if (!memmap || !hhdm || memmap->entry_count <= 1) {
            intr::kpanic(nullptr, "Memmap or HHDM request failed");
        }

        kprintf(" -> Memmap and HHDM requests are valid with entry count: %zu\n", memmap->entry_count);

        for (size_t i = 0; i < memmap->entry_count; i++) {
            struct limine_memmap_entry *entry = memmap->entries[i];
            kprintf(" -> Examining memmap entry %zu: type %d, base %p, length %llu\n", i, entry->type, entry->base, entry->length);

            highestAddressMemmap = MAX(highestAddressMemmap, entry->base + entry->length);

            if (entry->type == LIMINE_MEMMAP_USABLE) {
                memmapUsableEntryCount++;
                pagesUsable += DIV_ROUNDUP(entry->length, PAGE_SIZE);
                highestAddressUsable = MAX(highestAddressUsable, entry->base + entry->length);
            } else {
                pagesReserved += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            }
        }

        pagesTotal = highestAddressUsable / PAGE_SIZE;
        bitmapSizeBytes = DIV_ROUNDUP(pagesTotal, 8);
        kprintf(" -> Total pages: %llu, Bitmap size: %llu bytes\n", pagesTotal, bitmapSizeBytes);

        totalBytesPmmStructures = ALIGN_UP(bitmapSizeBytes + memmapUsableEntryCount * sizeof(struct UsableEntry), PAGE_SIZE);
        kprintf(" -> Total bytes for PMM structures: %llu\n", totalBytesPmmStructures);
        
        bool found = false;
        for (size_t i = memmap->entry_count; i > 0; i--) {
            size_t index = i - 1;
            struct limine_memmap_entry *entry = memmap->entries[index];
            if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= totalBytesPmmStructures) {
                uintptr_t aligned_top = ALIGN_DOWN(entry->base + entry->length - totalBytesPmmStructures, PAGE_SIZE);

                kprintf(" -> Suitable location found at entry %zu, top-aligned base %p\n", index, aligned_top);

                pageBitmap = (uint8_t*)(aligned_top + hhdm->offset);
                std::memset(pageBitmap, 0xFF, bitmapSizeBytes);

                usableEntries = (UsableEntry*)(aligned_top + hhdm->offset);

                entry->length = aligned_top - entry->base;
                
                found = true;
                break;
            }
        }
        if (!found) {
            intr::kpanic(nullptr, "Failed to find suitable location for bitmap!");
            __builtin_unreachable();
        }

        size_t usable_entry_index = 0;
        for (size_t i = 0; i < memmap->entry_count; i++) {
            struct limine_memmap_entry *entry = memmap->entries[i];
            if (entry->type == LIMINE_MEMMAP_USABLE) {
                for (size_t j = 0; j < entry->length / PAGE_SIZE; j++) {
                    BITMAP_UNSET_BIT(pageBitmap, (entry->base / PAGE_SIZE) + j);
                }
                kprintf(" -> Setting usable memory entry %zu: base %p, length %llu\n", usable_entry_index, entry->base, entry->length);
                *((uint64_t *)usableEntries + usable_entry_index * 2) = entry->base;
                *((uint64_t *)usableEntries + usable_entry_index * 2 + 1) = entry->length;

                usable_entry_index++;
            }
        }

        if ((pagesUsable * PAGE_SIZE) / (1024 * 1024) < 1000) {
            debugf("! ! ! ! ! ! ! !");
            kprintf(" -> You are running with less than one gibbibyte!\n");
            kprintf("    You are not going to have a lot of memory for the guest!\n");
        }
        kprintf(" -> PMM initialization complete\n");
    }

    static void *searchFree(size_t *idx, size_t *pages_found, size_t count)
    {
        if (BITMAP_READ_BIT(pageBitmap, *idx) == 0) {
            (*pages_found)++;
            if (*pages_found == count) {
                for (size_t j = (*idx + 1) - count; j <= *idx; j++) {
                    BITMAP_SET_BIT(pageBitmap, j);
                }
                _allocator_last_index = *idx;
                pagesInUse += count;
                return (void *)(((*idx + 1ul) - count) * PAGE_SIZE);
            }
        } else {
            *pages_found = 0;
        }
        (*idx)++;
        return nullptr;
    }

    void* claim(size_t count) {
        size_t idx = _allocator_last_index;
        size_t pagesFound = 0;
        bool canRetry = true;

        while (canRetry) {
            for (size_t entryIdx = 0; entryIdx < memmapUsableEntryCount; entryIdx++) {
                idx = MAX(idx, usableEntries[entryIdx].base / PAGE_SIZE);
                size_t end = (usableEntries[entryIdx].base + usableEntries[entryIdx].length) / PAGE_SIZE;

                while (idx < end) {
                    void* ptr = searchFree(&idx, &pagesFound, count);
                    if (ptr != nullptr)
                        return ptr;
                }
            }

            if (canRetry) {
                _allocator_last_index = 0;
                idx = 0;
                pagesFound = 0;
                canRetry = false;
            } else {
                kprintf(" -> NO PAGES FOUND; Free pages: %lu; Requested pages: %lu\n", pagesUsable - pagesInUse, count);
                return nullptr;
            }
        }

        __builtin_unreachable();
    }

    void free(void *ptr, size_t count) {

        size_t starting_page = (uint64_t)ptr / PAGE_SIZE;
        for (size_t i = 0; i < count; i++) {
            BITMAP_UNSET_BIT(pageBitmap, starting_page + i);
        }
        pagesInUse -= count;
    }
    
}
