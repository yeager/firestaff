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
    int item_name_table_verified;
    int level_label_table_verified;
    int item_property_table_verified;
    size_t item_property_table_offset;
    size_t item_property_table_bytes;
    int opaque_record_window_verified;
    size_t opaque_record_window_offset;
    size_t opaque_record_window_bytes;
    char source_md5[33];
} Theron_V1Track19InventoryReceipt;

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out);

/* Read a real Track 19 ISO, authenticate its known hash/size, and validate
 * the source-owned US item and level-label spans when applicable. */
int theron_v1_track19_inventory_file(
    const char *path, Theron_V1Track19InventoryReceipt *out);

#endif
