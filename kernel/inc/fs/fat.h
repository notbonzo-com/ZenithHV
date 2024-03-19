#pragma once

#include <stdint.h>

typedef struct {
    uint8_t reserved1[3];           // First 3 bytes are a short jump and a nop
    uint8_t oemName[8];             // OEM name ("EFI PART" most likely)
    uint16_t bytesPerSector;        // Bytes per sector (Usually 512)
    uint8_t sectorsPerCluster;      // How many sectors (512b) are in a cluster
    uint16_t reservedSectors;       // Number of reserved sectors 
    uint8_t fatCount;               // Number of FAT's (usually two)
    uint16_t rootDirCount;          // Number of root directory entries
} __attribute__((packed)) fat32_t;