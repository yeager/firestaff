#ifndef THERON_V1_TRACK19_INVENTORY_H
#define THERON_V1_TRACK19_INVENTORY_H

#include <stddef.h>

typedef struct {
    int valid;
    int sector_aligned;
    int container_format_unproven;
    int startup_usable;
    int level_usable;
    int bitmap_usable;
    int mode1_2048;
    int mode1_2352;
    size_t sector_count;
    size_t bytes;
    const char *source_format;
    const char *variant;
} Theron_V1Track19InventoryReceipt;

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out);

#endif
