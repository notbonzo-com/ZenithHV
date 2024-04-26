#include <x86/page.h>
#include <x86/idt.h>
#include <kprintf.h>
#include <mem.h>

struct usable_entry *usable_entries = NULL;

struct limine_memmap_response *memmap = NULL;
struct limine_hhdm_response *hhdm = NULL;

uint8_t *page_bitmap = NULL;
uint64_t usable_pages = 0;
uint64_t reserved_pages = 0;
uint64_t total_pages = 0;
uint64_t in_use_pages = 0;

uint64_t highest_address_memmap = 0;
size_t memmap_entry_count = 0;
uint64_t total_pmm_struct_bytes = 0;
uint64_t bitmap_size_bytes = 0;
uint64_t highest_address_usable = 0;

static size_t _allocator_last_index = 0;

struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

void initPgm(void)
{
    memmap = memmap_request.response;
    hhdm = hhdm_request.response;

    if (!memmap || !hhdm || memmap->entry_count <= 1) {
        debugf("Limine didn't supply a valid memmap or hhdm");
                kpanic();
    }

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        highest_address_memmap = MAX(highest_address_memmap, entry->base + entry->length);

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            memmap_entry_count++;
            usable_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            highest_address_usable = MAX(highest_address_usable, entry->base + entry->length);
        } else {
            reserved_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
        }
    }

    total_pages = highest_address_usable / PAGE_SIZE;
    bitmap_size_bytes = DIV_ROUNDUP(total_pages, 8);

        total_pmm_struct_bytes = ALIGN_UP(bitmap_size_bytes
        + memmap_entry_count * sizeof(struct usable_entry), PAGE_SIZE);
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= total_pmm_struct_bytes)
        {
            page_bitmap = (uint8_t*)(entry->base + hhdm->offset);
            memset(page_bitmap, 0xFF, bitmap_size_bytes);

            usable_entries = (struct usable_entry *)(entry->base + hhdm->offset + bitmap_size_bytes);

            entry->length -= ALIGN_UP(total_pmm_struct_bytes, PAGE_SIZE);
            entry->base += ALIGN_UP(total_pmm_struct_bytes, PAGE_SIZE);

            goto fill_bitmap;
        }
    }
    debugf("Failed to find usable memmap entry");
        kpanic();

fill_bitmap:
    size_t useable_entry_index = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t j = 0; j < entry->length / PAGE_SIZE; j++) {
                BITMAP_UNSET_BIT(page_bitmap, (entry->base / PAGE_SIZE) + j);
            }

            *((uint64_t *)usable_entries + useable_entry_index * 2) = entry->base;
            *((uint64_t *)usable_entries + useable_entry_index * 2 + 1) = entry->length;

            useable_entry_index++;
        }
    }

    debugf("Detected memory: %dMiB", (total_pages * PAGE_SIZE) / (1024 * 1024));
    debugf("Usable memory: %dMiB", ((usable_pages) * PAGE_SIZE) / (1024 * 1024));
    if ((total_pages * PAGE_SIZE) / (1024 * 1024) < 1024)
    {
        kprintf(" - [WARNING] -\n");
        kprintf("It is recommended to run this system with more than 1 GiB of RAM\n");
        kprintf("You may encounter unexpected issues!\n");
        kprintf(" - - - - - - -\n");
    }
}

static void *search_free(size_t *idx, size_t *pages_found, size_t count)
{
    if (BITMAP_READ_BIT(page_bitmap, *idx) == 0) {
        (*pages_found)++;
        if (*pages_found == count) {
            for (size_t j = (*idx + 1) - count; j <= *idx; j++) {
                BITMAP_SET_BIT(page_bitmap, j);
            }
            _allocator_last_index = *idx;
            in_use_pages += count;
            return (void *)(((*idx + 1ul) - count) * PAGE_SIZE);
        }
    } else {
        *pages_found = 0;
    }
    (*idx)++;
    return NULL;
}

void *allocate_pages(size_t count)
{
    size_t idx = _allocator_last_index, pages_found = 0;
    uint8_t can_retry = 1;

retry:
    for (size_t entry_idx = 0; entry_idx < memmap_entry_count; entry_idx++) {
        idx = MAX(idx, usable_entries[entry_idx].base / PAGE_SIZE);
        size_t end = (usable_entries[entry_idx].base + usable_entries[entry_idx].length) / PAGE_SIZE;
        while (idx < end) {
            void *ptr = search_free(&idx, &pages_found, count);
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

    debugf("Failed to find free pages as %lu are available; Requested pages: %lu", usable_pages - in_use_pages, count);
    return NULL;
}

void free_pages(void *ptr, size_t count)
{
    size_t starting_page = (uint64_t)ptr / PAGE_SIZE;
    for (size_t i = 0; i < count; i++) {
        BITMAP_UNSET_BIT(page_bitmap, starting_page + i);
    }
    in_use_pages -= count;
}