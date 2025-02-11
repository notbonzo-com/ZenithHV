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
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_BOLD_BLUE "\033[1;34m"
#define ANSI_BOLD_MAGENTA "\033[1;35m"
#define ANSI_BOLD_CYAN "\033[1;36m"
#define ANSI_BOLD_WHITE "\033[1;37m"

typedef struct 
{
    uint64_t type;
    const char *name;
    const char *color;
} memmap_entry_info;

static const memmap_entry_info memmap_types[] = 
{
    { LIMINE_MEMMAP_USABLE, "Usable", ANSI_BOLD_GREEN },
    { LIMINE_MEMMAP_RESERVED, "Reserved", ANSI_BOLD_RED },
    { LIMINE_MEMMAP_ACPI_RECLAIMABLE, "ACPI Reclaimable", ANSI_BOLD_YELLOW },
    { LIMINE_MEMMAP_ACPI_NVS, "ACPI NVS", ANSI_BOLD_MAGENTA },
    { LIMINE_MEMMAP_BAD_MEMORY, "Bad Memory", ANSI_BOLD_RED },
    { LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE, "Bootloader Reclaimable", ANSI_BOLD_CYAN },
    { LIMINE_MEMMAP_KERNEL_AND_MODULES, "Kernel & Modules", ANSI_BOLD_BLUE },
    { LIMINE_MEMMAP_FRAMEBUFFER, "Framebuffer", ANSI_BOLD_WHITE }
};

static const memmap_entry_info *get_memmap_info( uint64_t type ) 
{
    for ( size_t i = 0; i < sizeof( memmap_types ) / sizeof( memmap_types[ 0 ] ); i++ ) 
    {
        if ( memmap_types[ i ].type == type ) 
        {
            return &memmap_types[ i ];
        }
    }
    static const memmap_entry_info unknown = { 0, "Unknown", ANSI_BOLD_RED };
    return &unknown;
}

#endif

struct limine_memmap_response *memmap = nullptr;
struct limine_hhdm_response *hhdm = nullptr;
struct pmm_info pmm_info = { 0 };

struct limine_memmap_request memmap_request = 
{
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = nullptr
};

struct limine_hhdm_request hhdm_request = 
{
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = nullptr
};

static void insert_block_sorted( struct free_block *block ) 
{
    struct free_block **prev = &pmm_info.freelist;
    struct free_block *current = pmm_info.freelist;
    while ( current && current < block ) 
    {
        prev = &current->next;
        current = current->next;
    }
    block->next = current;
    *prev = block;
}

static void coalesce_free_list() 
{
    struct free_block *current = pmm_info.freelist;
    while ( current && current->next ) 
    {
        if ( ( uintptr_t ) current + current->size == ( uintptr_t ) current->next ) 
        {
            current->size += current->next->size;
            current->next = current->next->next;
#ifdef DEBUG
            printf( "[DEBUG] Merged adjacent free blocks at 0x%p\n", current );
#endif
        } 
        else 
        {
            current = current->next;
        }
    }
}

void init_pmm() 
{
    memmap = memmap_request.response;
    hhdm = hhdm_request.response;

    if ( !memmap || !hhdm || memmap->entry_count == 0 ) 
    {
        kpanic( nullptr, "Memmap or HHDM request failed" );
    }

    pmm_info.freelist = nullptr;

#ifdef DEBUG
    printf( "\nPhysical Memory Manager ( PMM ) Initialization\n" );
    printf( "=========================================================================================|\n" );
    printf( "| Index | Base Address       | Length             | Type                    | Pages      |\n" );
    printf( "=========================================================================================|\n" );
#endif

    for ( size_t i = 0; i < memmap->entry_count; i++ ) 
    {
        struct limine_memmap_entry *entry = memmap->entries[ i ];

        uintptr_t aligned_base = ( entry->base + PAGE_SIZE - 1 ) & ~( PAGE_SIZE - 1 );
        size_t aligned_length = ( entry->length - ( aligned_base - entry->base ) ) & ~( PAGE_SIZE - 1 );
        size_t page_count = aligned_length / PAGE_SIZE;

        if ( entry->type != LIMINE_MEMMAP_USABLE ) 
        {
            pmm_info.page_counters.pages_reserved += entry->length / PAGE_SIZE;
            goto debug_and_continue;
        }

        if ( aligned_length < PAGE_SIZE ) 
        {
            continue;
        }

        if ( aligned_base + aligned_length > pmm_info.highest_address_usable ) 
        {
            pmm_info.highest_address_usable = aligned_base + aligned_length;
        }

        pmm_info.page_counters.pages_usable += page_count;
        struct free_block *block = ( struct free_block * ) ( aligned_base + hhdm->offset );
        block->size = aligned_length;
        block->next = nullptr;

        insert_block_sorted( block );

debug_and_continue:

#ifdef DEBUG
        const memmap_entry_info *type_info = get_memmap_info( entry->type );
        printf( "| %5lu | 0x%016lx | 0x%016lx | %s%-23s%s | %10lu |\n",
            i, entry->base, entry->length,
            type_info->color, type_info->name, ANSI_RESET,
            page_count );
#endif
        continue;
    }

#ifdef DEBUG
    printf( "=========================================================================================|\n" );
#endif

    coalesce_free_list();

    pmm_info.page_counters.pages_total = pmm_info.highest_address_usable / PAGE_SIZE;

#ifdef DEBUG
    printf( "\n[DEBUG] PMM Finalized: Total Pages = %lu, Usable Pages = %lu, Reserved Pages = %lu\n",
           pmm_info.page_counters.pages_total,
           pmm_info.page_counters.pages_usable,
           pmm_info.page_counters.pages_reserved );
#endif
}

void *pmm_alloc_pages( size_t count ) 
{
    size_t size = count * PAGE_SIZE;
    struct free_block *best = nullptr;
    struct free_block **best_prev = nullptr;
    struct free_block **prev = &pmm_info.freelist;
    struct free_block *current = pmm_info.freelist;

    while ( current ) 
    {
        if ( current->size >= size ) 
        {
            if ( !best || current->size < best->size ) 
            {
                best = current;
                best_prev = prev;
            }
        }
        prev = &current->next;
        current = current->next;
    }

    if ( !best ) 
    {
#ifdef DEBUG
        printf( "[ERROR] Out of physical memory: Requested %lu pages ( 0x%lx bytes )\n", count, size );
#endif
        return nullptr;
    }

    void *allocated_memory = ( void * ) best;

    if ( best->size > size ) 
    {
        struct free_block *new_block = ( struct free_block * ) ( ( uintptr_t ) best + size );
        new_block->size = best->size - size;
        new_block->next = best->next;
        *best_prev = new_block;
    } 
    else 
    {
        *best_prev = best->next;
    }

    pmm_info.page_counters.pages_in_use += count;

#ifdef DEBUG
    printf( "[DEBUG] Allocated %lu pages ( 0x%lx bytes ) at 0x%p\n", count, size, allocated_memory );
#endif

    return allocated_memory;
}

void pmm_free_pages( void *ptr, size_t count ) 
{
    if ( __builtin_expect( !pmm_info.freelist || ( uintptr_t ) ptr < ( uintptr_t ) pmm_info.freelist, 0 ) ) 
    {
        kpanic( nullptr, "Attempted to free invalid memory!" );
    }
    if ( __builtin_expect( ( uintptr_t ) ptr % PAGE_SIZE != 0, 0 ) ) 
    {
        kpanic( nullptr, "Attempted to free unaligned memory!" );
    }

    size_t size = count * PAGE_SIZE;
    struct free_block *block = ( struct free_block * ) ptr;
    block->size = size;
    block->next = nullptr;

    insert_block_sorted( block );
    coalesce_free_list();
    pmm_info.page_counters.pages_in_use -= count;

#ifdef DEBUG
    printf( "[DEBUG] Freed %lu pages ( 0x%lx bytes ) at 0x%p\n", count, size, ptr );
#endif
}
