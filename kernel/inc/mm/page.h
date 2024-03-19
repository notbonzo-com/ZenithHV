#pragma once

#include <limine.h>
#include <stdint.h>
#include <stddef.h>

// 4096 byte pages
#define PAGE_SIZE 0x1000ul

// keep track of where on the bitmap are usable areas,
// so time is saved when looking for free pages
struct __attribute__((packed)) pageUsableEntries {
    uintptr_t base;
    size_t length;
};
extern struct pageUsableEntries *pageUsableEntries; 

extern struct limine_memmap_response *memmap;
extern struct limine_hhdm_response *hhdm;

// keep track of free and used pages: 0 = unused
extern uint8_t *pageBitmap;
extern uint64_t pageUsable;
extern uint64_t pageRestored;
extern uint64_t pageTotalPages;
extern uint64_t pageInUse;

extern uint64_t pageHighestAddressMemmap;
extern uint64_t pageMemmapUsableEntryCount;
extern uint64_t pageTotalBytesPageStructures;
extern uint64_t pageBitmapSizeBytes;
extern uint64_t pageHighestAddressUsable;

void initPgm(void);
void* pgmClaimPage(size_t count);
void pgmFreePage(void* addr, size_t count);