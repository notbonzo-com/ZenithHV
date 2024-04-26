#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE 0x1000ul

struct __attribute__((packed)) usable_entry {
    uintptr_t base;
    size_t length;
};
extern struct usable_entry *usable_entries;

extern struct limine_memmap_response *memmap;
extern struct limine_hhdm_response *hhdm;

extern uint8_t *page_bitmap;
extern uint64_t usable_pages;
extern uint64_t reserved_pages;
extern uint64_t total_pages;
extern uint64_t in_use_pages;

extern uint64_t highest_address_memmap;
extern size_t memmap_entry_count;
extern uint64_t total_pmm_struct_bytes;
extern uint64_t bitmap_size_bytes;
extern uint64_t highest_address_usable;

void initPgm(void);
void *allocate_pages(size_t count);
void free_pages(void *ptr, size_t count);

#ifdef __cplusplus
}
#endif