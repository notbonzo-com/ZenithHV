#pragma once

#include <limine.h>
#include <stdint.h>
#include <stddef.h>

// 4096 byte pages
#define PAGE_SIZE 0x1000ul

// keep track of where on the bitmap are usable areas,
// so time is saved when looking for free pages
struct __attribute__((packed)) pgm_usable_entry {
    uintptr_t base;
    size_t length;
};
extern struct pgm_usable_entry *pgm_usable_entries; 

extern struct limine_memmap_response *memmap;
extern struct limine_hhdm_response *hhdm;

// keep track of free and used pages: 0 = unused
extern uint8_t *pgm_page_bitmap;
extern uint64_t pgm_pages_usable;
extern uint64_t pgm_pages_reserved;
extern uint64_t pgm_pages_total;
extern uint64_t pgm_pages_in_use;

extern uint64_t pgm_highest_address_memmap;
extern uint64_t pgm_memmap_usable_entry_count;
extern uint64_t pgm_total_bytes_pgm_structures;
extern uint64_t pgm_bitmap_size_bytes;
extern uint64_t pgm_highest_address_usable;

void x86_64_pgm_init(void);
void* x86_64_pgm_claim_page(size_t count);
void x86_64_pgm_free_page(void* addr, size_t count);