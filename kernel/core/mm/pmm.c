//
// Created by notbonzo on 1/31/25.
//

#include <core/mm/pmm.h>
#include <core/constants.h>
#include <arch/x86_64/intr.h>
#include <stdbool.h>
#include <string.h>
#include <limine.h>
#ifdef DEBUG
#include <core/printf.h>
#endif

#define IS_ENTRY_USABLE( entry ) ( entry->type == LIMINE_MEMMAP_USABLE )
#define ALIGN_BLOCK( block ) ( ( struct free_block* )( ( uintptr_t )( block ) & ~( PAGE_SIZE - 1 ) ) )
#define TRANSLATE_PHYSICAL_ADDRESS( physical_address, hhdm_response ) ( physical_address + hhdm_response->offset )

struct limine_memmap_response *memmap = nullptr;
struct limine_hhdm_response *hhdm = nullptr;
struct pmm_info pmm_info = { 0 };

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

// block must be aligned (use ALIGN_BLOCK( block ))
static void insert_block_sorted( struct free_block *block ) {
    struct free_block *start = pmm_info.freelist;

    struct free_block *prev = { 0 };
    struct free_block *current = start;

    while ( true ) {
        // End of linked list
        if ( !current ) {
            break;
        }

        // The current address is higher than block
        if ( current >= block ) {
            break;
        }

        prev = current;
        current = current->next;
    }

    if ( prev ) {
        //   [ start ] ->
        //   [...] ->
        //   prev ->
        // * block ->
        //   current
        prev->next = block;
    } else {
        //   [ start ] ->
        // * block ->
        //   current

        pmm_info.freelist = block;
    }

    block->next = current;
}

static void coalesce_free_list( ) {
    struct free_block *current = pmm_info.freelist;
    while ( current && current->next ) {
        if ( (uintptr_t)current + current->size == (uintptr_t)current->next ) {
            current->size += current->next->size;
            current->next = current->next->next;
#ifdef DEBUG
            printf("[DEBUG] Merged adjacent free blocks at 0x%p\n", current);
#endif
        } else {
            current = current->next;
        }
    }
}

void init_pmm( ) {
    memmap = memmap_request.response;
    hhdm = hhdm_request.response;

    if ( !memmap || !hhdm || memmap->entry_count == 0 ) {
        kpanic( nullptr, "Memmap or HHDM request failed" );
    }

    pmm_info.freelist = nullptr;

#ifdef DEBUG
    printf("\nPhysical Memory Manager (PMM) Initialization\n");
    printf("===============================================================================================\n");
    printf("| Index | Base Address       | Length             | Type               | Pages      | Usable? |\n");
    printf("===============================================================================================\n");
#endif

    for ( size_t i = 0; i < memmap->entry_count; i++ ) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        bool is_usable = IS_ENTRY_USABLE( entry );
        size_t page_count = entry->length / PAGE_SIZE;
        uintptr_t entry_end = entry->base + entry->length;

        if ( is_usable && ( entry_end > pmm_info.highest_address_usable ) ) {
            pmm_info.highest_address_usable = entry_end;
        }


#ifdef DEBUG
        printf("| %5lu | 0x%016lx | 0x%016lx | %-18s | %10lu | %-7s |\n",
               i, entry->base, entry->length,
               is_usable ? "Usable" : "Reserved",
               page_count,
               is_usable ? "Yes" : "No");
#endif

        if ( is_usable ) {
            pmm_info.page_counters.pages_usable += page_count;

            struct free_block *block = ALIGN_BLOCK( TRANSLATE_PHYSICAL_ADDRESS( entry->base, hhdm ) );
            block->size = entry->length;
            block->next = nullptr;
            insert_block_sorted( block );
        } else {
            pmm_info.page_counters.pages_reserved += page_count;
        }
    }

#ifdef DEBUG
    printf("===============================================================================================\n");
#endif

    coalesce_free_list( );

    pmm_info.page_counters.pages_total = pmm_info.highest_address_usable / PAGE_SIZE;

#ifdef DEBUG
    printf("\n[DEBUG] PMM Finalized: Total Pages = %lu, Usable Pages = %lu, Reserved Pages = %lu\n",
           pmm_info.page_counters.pages_total,
           pmm_info.page_counters.pages_usable,
           pmm_info.page_counters.pages_reserved);
#endif
}


void *pmm_alloc_pages( size_t count ) {
    size_t size = count * PAGE_SIZE;
    struct free_block *best = nullptr;
    struct free_block **best_prev = nullptr;
    struct free_block **prev = &pmm_info.freelist;
    struct free_block *current = pmm_info.freelist;

    while ( current ) {
        if ( current->size >= size ) {
            if ( !best || current->size < best->size ) {
                best = current;
                best_prev = prev;
            }
        }
        prev = &current->next;
        current = current->next;
    }

    if ( !best ) {
#ifdef DEBUG
        printf("[ERROR] Out of physical memory: Requested %lu pages (0x%lx bytes)\n", count, size);
#endif
        return nullptr;
    }

    void *allocated_memory = ( void * ) best;

    if ( best->size > size ) {
        struct free_block *new_block = (struct free_block*)( (uintptr_t)best + size );
        new_block->size = best->size - size;
        new_block->next = best->next;
        *best_prev = new_block;
    } else {
        *best_prev = best->next;
    }

    pmm_info.page_counters.pages_in_use += count;

#ifdef DEBUG
    printf("[DEBUG] Allocated %lu pages (0x%lx bytes) at 0x%p\n", count, size, allocated_memory);
#endif

    return allocated_memory;
}

void pmm_free_pages( void *ptr, size_t count ) {
    if (!pmm_info.freelist || (uintptr_t)ptr < (uintptr_t)pmm_info.freelist) {
        kpanic(nullptr, "Attempted to free invalid memory!");
    }
    size_t size = count * PAGE_SIZE;
    struct free_block *block = ALIGN_BLOCK( ptr );
    block->size = size;
    block->next = nullptr;
    insert_block_sorted( block );
    coalesce_free_list( );
    pmm_info.page_counters.pages_in_use -= count;
#ifdef DEBUG
    printf("[DEBUG] Freed %lu pages (0x%lx bytes) at 0x%p\n", count, size, ptr);
#endif
}
