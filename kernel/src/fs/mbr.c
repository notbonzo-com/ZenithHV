#include <fs/mbr.h>
#include <mm/kalloc.h>
#include <mem.h>
#include <debug.h>
#include <x86/int.h>

mbr_t *gMbr;
partition_entry_t gParts[4];

void initMbr(void)
{
    gMbr = kmalloc(sizeof(mbr_t));
    ATARead((uint8_t*)gMbr, 0, 1, BOOT_DRIVE);
    if (gMbr->signature[0] != 0x55 || gMbr->signature[1] != 0xAA)
    {
        panic(NULL, 0, "MBR signature not found\n");
    }
    
    for (int i = 0; i < NUM_PARTITIONS; i++)
    {
        partition_entry_t *entry = (partition_entry_t *)&gMbr->partitionTable[i * PARTITION_ENTRY_SIZE];
        if (entry->type != 0)
        {
            gParts[i] = *entry;
            log_debug("MBR", "Partition %d: LBA Offset: %d, Length: %d\n", i, entry->lbaStart, entry->numSectors);
            continue;
        }
        log_debug("MBR", "Partition %d: Empty\n", i);
    }
}

