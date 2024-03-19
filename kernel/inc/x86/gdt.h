#pragma once

#include <stdint.h>

struct taskStateSegment;

typedef struct {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t accessByte;
    uint8_t limitHighAndFlags;
    uint8_t baseHigh;
} __attribute__((packed)) segmentDescriptor;

// TSS and LDT
typedef struct {
    segmentDescriptor descriptor;
    uint32_t base;
    uint32_t reserved;
} __attribute__((packed)) systemSegmentDescriptor;

// Task State Segment
typedef struct {
    uint32_t reserved0;
    uint64_t stackPointers[3]; // stackPointers
    uint64_t reserved1;
    uint64_t interruptStackTables[7]; // interruptStackTables
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t ioPermissionBitmap;
} __attribute__((packed)) tss;

int initGdt(void);
void reloadGdt();
void reloadTss(struct taskStateSegment *tssAddress);