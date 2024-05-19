#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include <util>

namespace pmm {

struct __pack UsableEntry {
    uintptr_t base;
    size_t length;
};
extern UsableEntry* usableEntries; 

extern struct limine_memmap_response *memmap;
extern struct limine_hhdm_response *hhdm;

// keep track of free and used pages: 0 = unused
extern uint8_t* pageBitmap;
extern uint64_t pagesUsable;
extern uint64_t pagesReserved;
extern uint64_t pagesTotal;
extern uint64_t pagesInUse;

extern uint64_t highestAddressMemmap;
extern uint64_t memmapUsableEntryCount;
extern uint64_t totalBytesPmmStructures;
extern uint64_t bitmapSizeBytes;
extern uint64_t highestAddressUsable;

void init(void);

}