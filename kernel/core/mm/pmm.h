//
// Created by notbonzo on 1/31/25.
//

#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000ul

struct free_block {
    size_t size;
    struct free_block *next;
};

struct pmm_info {
    struct free_block *freelist;
    uint64_t highest_address_usable;
    struct {
        uint64_t pages_usable;
        uint64_t pages_reserved;
        uint64_t pages_total;
        uint64_t pages_in_use;
    } page_counters;
};

extern struct pmm_info pmm_info;
extern struct limine_hhdm_response *hhdm;
extern struct limine_memmap_response *memmap;

void init_pmm( );
void *pmm_alloc_pages( size_t count );
void pmm_free_pages( void *ptr, size_t count );

#endif //PMM_H
