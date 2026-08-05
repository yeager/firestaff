#include "theron_v1_track19_inventory.h"
#include "theron_v1_track19_item_names.h"

#include <stdio.h>
#include <stdlib.h>

static int verify_real_us_item_table(void) {
    const char *path = getenv("THERON_TRACK19_US_ISO");
    FILE *file;
    long size;
    uint8_t *bytes;
    unsigned int i;
    char name[64];

    if (!path || !path[0]) return 1; /* CI remains data-free by default. */
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || size > 64L * 1024L * 1024L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    for (i = 0u; i < THERON_TRACK19_US_ITEM_NAME_COUNT; ++i) {
        if (!theron_v1_track19_us_item_name_from_iso(
                bytes, (size_t)size, i, name, sizeof(name))) {
            free(bytes);
            return 0;
        }
    }
    bytes[THERON_TRACK19_US_ITEM_NAME_OFFSET] ^= 1u;
    if (theron_v1_track19_us_item_name_from_iso(
            bytes, (size_t)size, 0u, name, sizeof(name))) {
        free(bytes);
        return 0;
    }
    free(bytes);
    return 1;
}

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
    return verify_real_us_item_table() ? 0 : 1;
}
