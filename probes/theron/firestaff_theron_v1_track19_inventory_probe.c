#include "theron_v1_track19_inventory.h"

int main(void) {
    Theron_V1Track19InventoryReceipt receipt;

    if (!theron_v1_track19_inventory(
            "51b40a17b92a30339957ba564aa0015c",
            5983488u,
            &receipt)) {
        return 1;
    }
    if (!receipt.mode1_2352 || receipt.mode1_2048 ||
        receipt.sector_count != 2544u ||
        !receipt.container_format_unproven || receipt.startup_usable ||
        receipt.level_usable || receipt.bitmap_usable) {
        return 1;
    }
    if (!theron_v1_track19_inventory(
            "51b40a17b92a30339957ba564aa0015c",
            5984256u,
            &receipt) ||
        !receipt.mode1_2048 || receipt.mode1_2352 ||
        receipt.sector_count != 2922u || receipt.container_format_unproven ||
        receipt.startup_usable || receipt.level_usable || receipt.bitmap_usable ||
        !theron_v1_track19_inventory(
            "f9f069a5e489b91207f3156059b756f1", 6291456u, &receipt) ||
        !receipt.mode1_2048 || receipt.sector_count != 3072u ||
        theron_v1_track19_inventory("bad", 5983488u, &receipt)) {
        return 1;
    }
    return 0;
}
