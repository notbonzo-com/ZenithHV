#include "pg.h"
#include <stdint.h>
#include <stddef.h>
#include <drivers/debug/e9.h>
#include <binary.h>
#include <mem.h>

// usable entries are page aligned
struct limine_memmap_response *memmap = NULL;
struct limine_hhdm_response *hhdm = NULL;

struct pgm_usable_entry *pgm_usable_entries = NULL;

// keep track of free and used pages: 0 = unused
uint8_t *pgm_page_bitmap = NULL;
uint64_t pgm_pages_usable = 0;
uint64_t pgm_pages_reserved = 0;
uint64_t pgm_pages_total = 0;
uint64_t pgm_pages_in_use = 0;

uint64_t pgm_highest_address_memmap = 0;
uint64_t pgm_memmap_usable_entry_count = 0;
uint64_t pgm_total_bytes_pgm_structures = 0;
uint64_t pgm_highest_address_usable = 0;
uint64_t pgm_bitmap_size_bytes = 0;

static size_t _allocator_last_index = 0;

// memmap and hhdm
struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static void init_bitmap(void)
{
    // upwards page aligned
    pgm_total_bytes_pgm_structures = ALIGN_UP(pgm_bitmap_size_bytes
        + pgm_memmap_usable_entry_count * sizeof(struct pgm_usable_entry), PAGE_SIZE);
    uint8_t success = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= pgm_total_bytes_pgm_structures)
        {
            // put bitmap at the start of fitting entry
            pgm_page_bitmap = (uint8_t*)(entry->base + hhdm->offset);
            memset(pgm_page_bitmap, 0xFF, pgm_bitmap_size_bytes);

            // put usable_entry_array right after it
            pgm_usable_entries = (struct pgm_usable_entry *)(entry->base + hhdm->offset + pgm_bitmap_size_bytes);

            // wipe bitmap space from memmap entry (page aligned size)
            entry->length -= ALIGN_UP(pgm_total_bytes_pgm_structures, PAGE_SIZE);
            entry->base += ALIGN_UP(pgm_total_bytes_pgm_structures, PAGE_SIZE);

            success = 1;
            break;
        }
    }
    if (!success) {
        log_crit("pgm", "Failed to find a suitable memory region for the page bitmap");
        __asm__ volatile ("cli\n hlt");
    }
}

static void fill_bitmap(void)
{
    size_t useable_entry_index = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            // free usable entries in the page map
            for (size_t j = 0; j < entry->length / PAGE_SIZE; j++) {
                BITMAP_UNSET_BIT(pgm_page_bitmap, (entry->base / PAGE_SIZE) + j);
            }

            // fill usable entry tracking array
            *((uint64_t *)pgm_usable_entries + useable_entry_index * 2) = entry->base;
            *((uint64_t *)pgm_usable_entries + useable_entry_index * 2 + 1) = entry->length;

            useable_entry_index++;
        }
    }
}

void x86_64_pgm_init(void)
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

        pgm_highest_address_memmap = MAX(pgm_highest_address_memmap, entry->base + entry->length);

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            pgm_memmap_usable_entry_count++;
            pgm_pages_usable += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            pgm_highest_address_usable = MAX(pgm_highest_address_usable, entry->base + entry->length);
        } else {
            pgm_pages_reserved += DIV_ROUNDUP(entry->length, PAGE_SIZE);
        }
    }

    pgm_pages_total = pgm_highest_address_usable / PAGE_SIZE;
    pgm_bitmap_size_bytes = DIV_ROUNDUP(pgm_pages_total, 8);

    init_bitmap();
    fill_bitmap();

    log_debug("pgm", "pgm_pages_usable: %lu", pgm_pages_usable);

    if ((pgm_pages_usable * PAGE_SIZE) / (1024 * 1024) < 1000) {
        log_crit("pgm", "Not enough usable memory");
        __asm__ volatile ("cli\n hlt");
    }
}

static void *search_free(size_t *idx, size_t *pages_found, size_t count)
{
    // if free page at idx
    if (BITMAP_READ_BIT(pgm_page_bitmap, *idx) == 0) {
        (*pages_found)++;
        // found enough contiguous pages
        if (*pages_found == count) {
            // alloc them
            for (size_t j = (*idx + 1) - count; j <= *idx; j++) {
                BITMAP_SET_BIT(pgm_page_bitmap, j);
            }
            _allocator_last_index = *idx;
            pgm_pages_in_use += count;
            return (void *)(((*idx + 1ul) - count) * PAGE_SIZE);
        }
    // the region we are at doesn't contain enough pages
    } else {
        *pages_found = 0;
    }
    (*idx)++;
    return NULL;
}

void *x86_64_pgm_claim_page(size_t count)
{
    size_t idx = _allocator_last_index, pages_found = 0;
    uint8_t can_retry = 1;

retry:
    for (size_t entry_idx = 0; entry_idx < pgm_memmap_usable_entry_count; entry_idx++) {
        // while not at last page (last idx = total - 1)
        idx = MAX(idx, pgm_usable_entries[entry_idx].base / PAGE_SIZE);
        size_t end = (pgm_usable_entries[entry_idx].base + pgm_usable_entries[entry_idx].length) / PAGE_SIZE;
        while (idx < end) {
            void *ptr = search_free(&idx, &pages_found, count);
            if (ptr != NULL)
                return ptr;
        }
    }
    // chance to look again from the beginning
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

// ptr needs to be the original pointer memory was allocated from
void x86_64_pgm_free_page(void *ptr, size_t count)
{
    size_t starting_page = (uint64_t)ptr / PAGE_SIZE;
    for (size_t i = 0; i < count; i++) {
        BITMAP_UNSET_BIT(pgm_page_bitmap, starting_page + i);
    }
    pgm_pages_in_use -= count;
}