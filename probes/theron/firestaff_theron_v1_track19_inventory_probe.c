#include "theron_v1_track19_inventory.h"
#include "theron_v1_track19_item_names.h"
#include "theron_v1_track19_jp_item_names.h"
#include "theron_v1_track19_jp_level_labels.h"
#include "theron_v1_track19_level_labels.h"
#include "theron_v1_track19_record_window.h"
#include "theron_v1_track02_item_properties.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_real_us_item_table(void) {
    const char *path = getenv("THERON_TRACK19_US_ISO");
    FILE *file;
    long size;
    uint8_t *bytes;
    Theron_ItemPropertyRecord property;
    const Theron_ItemPropertyRecord *expected;
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
    /* Reload the unmodified source bytes so the level-label table is tested
     * independently of the item-table mutation above. */
    free(bytes);
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
    expected = theron_v1_track02_item_property(65u);
    if (!expected || !theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 0, 65u, &property) ||
        memcmp(&property, expected, sizeof(property)) != 0) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET] ^= 1u;
    if (theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 0, 0u, &property)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET] ^= 1u;
    for (i = 0u; i < THERON_TRACK19_US_LEVEL_LABEL_COUNT; ++i) {
        if (!theron_v1_track19_us_level_label_from_iso(
                bytes, (size_t)size, i, name, sizeof(name))) {
            free(bytes);
            return 0;
        }
    }
    bytes[THERON_TRACK19_US_LEVEL_LABEL_OFFSET] ^= 1u;
    if (theron_v1_track19_us_level_label_from_iso(
            bytes, (size_t)size, 0u, name, sizeof(name))) {
        free(bytes);
        return 0;
    }
    free(bytes);
    return 1;
}

static int verify_real_jp_item_table(void) {
    const char *path = getenv("THERON_TRACK19_JP_ISO");
    FILE *file;
    long size;
    uint8_t *bytes;
    uint8_t name[128];
    size_t name_size;
    uint8_t label[32];
    size_t label_size;
    Theron_ItemPropertyRecord property;
    const Theron_ItemPropertyRecord *expected;
    unsigned int i;

    if (!path || !path[0]) return 1;
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
    for (i = 0u; i < THERON_TRACK19_JP_ITEM_NAME_COUNT; ++i) {
        if (!theron_v1_track19_jp_item_name_from_iso(
                bytes, (size_t)size, i, name, sizeof(name), &name_size) ||
            name_size == 0u) {
            free(bytes);
            return 0;
        }
    }
    bytes[THERON_TRACK19_JP_ITEM_NAME_OFFSET] ^= 1u;
    if (theron_v1_track19_jp_item_name_from_iso(
            bytes, (size_t)size, 0u, name, sizeof(name), &name_size)) {
        free(bytes);
        return 0;
    }
    free(bytes);
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
    expected = theron_v1_track02_item_property(65u);
    if (!expected || !theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 1, 65u, &property) ||
        memcmp(&property, expected, sizeof(property)) != 0) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET] ^= 1u;
    if (theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 1, 0u, &property)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET] ^= 1u;
    for (i = 0u; i < THERON_TRACK19_JP_LEVEL_LABEL_COUNT; ++i) {
        if (!theron_v1_track19_jp_level_label_from_iso(
                bytes, (size_t)size, i, label, sizeof(label), &label_size) ||
            label_size != THERON_TRACK19_JP_LEVEL_LABEL_BYTES) {
            free(bytes);
            return 0;
        }
    }
    bytes[THERON_TRACK19_JP_LEVEL_LABEL_OFFSET] ^= 1u;
    if (theron_v1_track19_jp_level_label_from_iso(
            bytes, (size_t)size, 0u, label, sizeof(label), &label_size)) {
        free(bytes);
        return 0;
    }
    free(bytes);
    return 1;
}

int main(void) {
    Theron_V1Track19InventoryReceipt receipt;
    Theron_V1Track19InventoryReceipt file_receipt;
    const char *real_iso = getenv("THERON_TRACK19_US_ISO");
    const char *real_jp_iso = getenv("THERON_TRACK19_JP_ISO");

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
    if (!verify_real_us_item_table()) return 1;
    if (!verify_real_jp_item_table()) return 1;
    if (real_iso && real_iso[0] &&
        (!theron_v1_track19_inventory_file(real_iso, &file_receipt) ||
         !file_receipt.item_name_table_verified ||
         !file_receipt.level_label_table_verified ||
         !file_receipt.item_property_table_verified ||
         file_receipt.item_property_table_offset !=
             THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET ||
         file_receipt.item_property_table_bytes !=
             THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES ||
         !file_receipt.opaque_record_window_verified ||
         file_receipt.opaque_record_window_offset !=
             THERON_TRACK19_OPAQUE_RECORD_WINDOW_US_OFFSET ||
         file_receipt.opaque_record_window_bytes !=
             THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES ||
         file_receipt.source_md5[0] == '\0')) return 1;
    if (real_jp_iso && real_jp_iso[0] &&
        (!theron_v1_track19_inventory_file(real_jp_iso, &file_receipt) ||
         !file_receipt.item_name_table_verified ||
         !file_receipt.level_label_table_verified ||
         !file_receipt.item_property_table_verified ||
         file_receipt.item_property_table_offset !=
             THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET ||
         file_receipt.item_property_table_bytes !=
             THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES ||
         !file_receipt.opaque_record_window_verified ||
         file_receipt.opaque_record_window_offset !=
             THERON_TRACK19_OPAQUE_RECORD_WINDOW_JP_OFFSET ||
         file_receipt.opaque_record_window_bytes !=
             THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES ||
         file_receipt.source_md5[0] == '\0')) return 1;
    return 0;
}
