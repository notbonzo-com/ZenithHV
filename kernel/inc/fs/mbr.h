#include <fs/ata.h>
#include <vector.h>

#define SECTOR_SIZE 512
#define PARTITION_ENTRY_SIZE 16
#define NUM_PARTITIONS 4
#define BOOT_DRIVE (ATA_PRIMARY << 1 | ATA_MASTER)

typedef struct {
    uint8_t bootCode[446];
    uint8_t partitionTable[64];
    uint8_t signature[2];
} __attribute__((packed)) mbr_t;

typedef struct {
    uint8_t status;              // Bootable flag
    uint8_t chsStart[3];         // CHS address of first absolute sector in partition
    uint8_t type;                // Partition type
    uint8_t chsEnd[3];           // CHS address of last absolute sector in partition
    uint32_t lbaStart;           // LBA of first absolute sector in the partition
    uint32_t numSectors;         // Number of sectors in partition
} __attribute__((packed)) partition_entry_t;

extern mbr_t *gMbr;
extern partition_entry_t gParts[4];

void initMbr(void);