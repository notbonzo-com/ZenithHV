#include <fs/ata.h>
#include <fs/mbr.h>
#include <mem.h>

#include <kalloc.h>
#include <kprintf.h>
#include <x86/io.h>
#include <x86/smp.h>

mbr_t g_Mbr;

void parse_mbr()
{
    uint8_t buf[512];
    ata_read(buf, 0, 1);

    memcpy(&g_Mbr, buf, sizeof(mbr_t));

    if (g_Mbr.signature[0] == 0x55 && g_Mbr.signature[1] == 0xAA) {
        debugf("MBR signature found");
        for (int i = 0; i < 4; i++) {
            // This check seems to not work ¯\_(ツ)_/¯
            // if (g_Mbr.partitions[i].status == 0x80) {
                // debugf("Partition #%d: @%d+%d", i+1, g_Mbr.partitions[i].lba_first_sector, g_Mbr.partitions[i].sector_count);
            // } else {
                // debugf("Partition #%d: inactive", i+1);
            // }
            if (g_Mbr.partitions[i].lba_first_sector != 0 && g_Mbr.partitions[i].sector_count != 0) {
                debugf("Partition #%d: @%d+%d", i+1, g_Mbr.partitions[i].lba_first_sector, g_Mbr.partitions[i].sector_count);
            } else {
                debugf("Partition #%d: inactive", i+1);
            }
        }
        return;
    } else {
        debugf("MBR signature not found");
        debugf("Signature was 0x%x 0x%x instead of 0x55 0xAA", g_Mbr.signature[0], g_Mbr.signature[1]);

        debugf("Attempting to parse non MBR partition yields");

        for (int i = 0; i < 4; i++) {
            if (g_Mbr.partitions[i].status & 0x80) {
                debugf("Partition #%d: @%d+%d", i+1, g_Mbr.partitions[i].lba_first_sector, g_Mbr.partitions[i].sector_count);
            } else {
                debugf("Partition #%d: inactive", i+1);
            }
        }
    }
    return;
}