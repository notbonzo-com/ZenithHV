#include <mm/page.h>
#include <stdint.h>
#include <stddef.h>
#include <debug.h>
#include <binary.h>
#include <mem.h>

#include <int.h>

struct limine_memmap_response *memmap = NULL;
struct limine_hhdm_response *hhdm = NULL;

struct pageUsableEntries *pageUsableEntries = NULL;

uint8_t *pageBitmap = NULL;
uint64_t pageUsable = 0;
uint64_t pageRestored = 0;
uint64_t pageTotalPages = 0;
uint64_t pageInUse = 0;

uint64_t pageHighestAddressMemmap = 0;
uint64_t pageMemmapUsableEntryCount = 0;
uint64_t pageTotalBytesPageStructures = 0;
uint64_t pageBitmapSizeBytes = 0;
uint64_t pageHighestAddressUsable = 0;

static size_t _allocator_last_index = 0;

struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static void initBitmap(void)
{
    pageTotalBytesPageStructures = ALIGN_UP(pageHighestAddressUsable
        + pageMemmapUsableEntryCount * sizeof(struct pageUsableEntries), PAGE_SIZE);
    uint8_t success = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= pageTotalBytesPageStructures)
        {
            pageBitmap = (uint8_t*)(entry->base + hhdm->offset);
            memset(pageBitmap, 0xFF, pageHighestAddressUsable);

            pageUsableEntries = (struct pageUsableEntries *)(entry->base + hhdm->offset + pageHighestAddressUsable);

            entry->length -= ALIGN_UP(pageTotalBytesPageStructures, PAGE_SIZE);
            entry->base += ALIGN_UP(pageTotalBytesPageStructures, PAGE_SIZE);

            success = 1;
            break;
        }
    }
    if (!success) {
        log_crit("pgm", "Failed to find a suitable memory region for the page bitmap");
        __asm__ volatile ("cli\n hlt");
    }
}

static void fillBitmap(void)
{
    size_t useable_entry_index = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            // free usable entries in the page map
            for (size_t j = 0; j < entry->length / PAGE_SIZE; j++) {
                BITMAP_UNSET_BIT(pageBitmap, (entry->base / PAGE_SIZE) + j);
            }

            // fill usable entry tracking array
            *((uint64_t *)pageUsableEntries + useable_entry_index * 2) = entry->base;
            *((uint64_t *)pageUsableEntries + useable_entry_index * 2 + 1) = entry->length;

            useable_entry_index++;
        }
    }
}

void initPgm(void)
{
    memmap = memmap_request.response;
    hhdm = hhdm_request.response;

    if (!memmap || !hhdm || memmap->entry_count <= 1) {
        log_crit("pgm", "limine memory map request not answered");
        __asm__ volatile ("cli\n hlt");
    }

    // calc page_bitmap size
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        pageHighestAddressMemmap = MAX(pageHighestAddressMemmap, entry->base + entry->length);

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            pageMemmapUsableEntryCount++;
            pageUsable += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            pageBitmapSizeBytes = MAX(pageBitmapSizeBytes, entry->base + entry->length);
        } else {
            pageRestored += DIV_ROUNDUP(entry->length, PAGE_SIZE);
        }
    }

    pageTotalPages = pageBitmapSizeBytes / PAGE_SIZE;
    pageHighestAddressUsable = DIV_ROUNDUP(pageTotalPages, 8);

    initBitmap();
    fillBitmap();

    log_debug("pgm", "pageUsable: %lu", pageUsable);

    if ((pageUsable * PAGE_SIZE) / (1024 * 1024) < 1000) {
        log_crit("pgm", "Not enough usable memory");
        __asm__ volatile ("cli\n hlt");
    }
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
            pageInUse += count;
            return (void *)(((*idx + 1ul) - count) * PAGE_SIZE);
        }
    } else {
        *pages_found = 0;
    }
    (*idx)++;
    return NULL;
}

void *pgmClaimPage(size_t count)
{
    size_t idx = _allocator_last_index, pages_found = 0;
    uint8_t can_retry = 1;

retry:
    for (size_t entry_idx = 0; entry_idx < pageMemmapUsableEntryCount; entry_idx++) {
        idx = MAX(idx, pageUsableEntries[entry_idx].base / PAGE_SIZE);
        size_t end = (pageUsableEntries[entry_idx].base + pageUsableEntries[entry_idx].length) / PAGE_SIZE;
        while (idx < end) {
            void *ptr = searchFree(&idx, &pages_found, count);
            if (ptr != NULL)
                return ptr;
        }
    }
    if (can_retry) {
        _allocator_last_index = 0;
        can_retry = 0;
        idx = 0;
        pages_found = 0;
        goto retry;
    }

    log_crit("pgm", "Failed to allocate %lu contiguous pages", count);
    return NULL;
}

void pgmFreePage(void *ptr, size_t count)
{
    size_t starting_page = (uint64_t)ptr / PAGE_SIZE;
    for (size_t i = 0; i < count; i++) {
        BITMAP_UNSET_BIT(pageBitmap, starting_page + i);
    }
    pageInUse -= count;
}