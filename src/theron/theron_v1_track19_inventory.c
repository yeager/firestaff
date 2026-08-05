#include "theron_v1_track19_inventory.h"
#include "asset_status_m12.h"
#include "theron_v1_track19_item_names.h"
#include "theron_v1_track19_jp_item_names.h"
#include "theron_v1_track19_jp_level_labels.h"
#include "theron_v1_track19_level_labels.h"
#include "theron_v1_track19_record_window.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out) {
    const char *variant = NULL;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    size_t sector_bytes;

    if (!out || !md5 || bytes == 0u) {
        return 0;
    }

    if (strcmp(md5, "51b40a17b92a30339957ba564aa0015c") == 0) {
        variant = "us";
    } else if (strcmp(md5, "f9f069a5e489b91207f3156059b756f1") == 0) {
        variant = "jp";
    }
    if (!variant) {
        return 0;
    }

    /* Track 19 is authored as MODE1/2048 in the supplied CUE/ISO corpus.
     * Raw MODE1/2352 is also retained as an accepted transport form, but
     * neither format alone proves the game-record grammar. */
    if (bytes % 2048u == 0u &&
        ((strcmp(md5, "51b40a17b92a30339957ba564aa0015c") == 0 &&
          bytes == 5984256u) ||
         (strcmp(md5, "f9f069a5e489b91207f3156059b756f1") == 0 &&
          bytes == 6291456u))) {
        sector_bytes = 2048u;
    } else if (bytes % 2352u == 0u) {
        sector_bytes = 2352u;
    } else {
        return 0;
    }

    out->valid = 1;
    out->sector_aligned = 1;
    out->container_format_unproven = sector_bytes == 2352u;
    out->startup_usable = 0;
    out->level_usable = 0;
    out->bitmap_usable = 0;
    out->mode1_2048 = sector_bytes == 2048u;
    out->mode1_2352 = sector_bytes == 2352u;
    out->sector_count = bytes / sector_bytes;
    out->bytes = bytes;
    out->source_format = sector_bytes == 2048u ? "MODE1/2048-ISO" :
        "MODE1/2352-RAW";
    out->variant = variant;
    return 1;
}

int theron_v1_track19_inventory_file(
        const char *path, Theron_V1Track19InventoryReceipt *out) {
    FILE *file;
    long file_size;
    size_t bytes;
    uint8_t *data;
    char md5[33];
    char text[64];
    unsigned int i;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !path[0] || !out || !m12_file_md5_hex(path, md5) ||
        !(file = fopen(path, "rb"))) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || file_size > 64L * 1024L * 1024L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (size_t)file_size;
    if (!theron_v1_track19_inventory(md5, bytes, out) ||
        !out->mode1_2048 || out->mode1_2352) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc(bytes);
    if (!data || fread(data, 1u, bytes, file) != bytes) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    snprintf(out->source_md5, sizeof(out->source_md5), "%s", md5);
    if (strcmp(out->variant, "us") == 0) {
        for (i = 0u; i < THERON_TRACK19_US_ITEM_NAME_COUNT; ++i) {
            if (!theron_v1_track19_us_item_name_from_iso(
                    data, bytes, i, text, sizeof(text))) {
                free(data);
                return 0;
            }
        }
        out->item_name_table_verified = 1;
        for (i = 0u; i < THERON_TRACK19_US_LEVEL_LABEL_COUNT; ++i) {
            if (!theron_v1_track19_us_level_label_from_iso(
                    data, bytes, i, text, sizeof(text))) {
                free(data);
                return 0;
            }
        }
        out->level_label_table_verified = 1;
    } else if (strcmp(out->variant, "jp") == 0) {
        uint8_t raw_name[128];
        size_t raw_name_size;
        uint8_t raw_label[32];
        size_t raw_label_size;

        for (i = 0u; i < THERON_TRACK19_JP_ITEM_NAME_COUNT; ++i) {
            if (!theron_v1_track19_jp_item_name_from_iso(
                    data, bytes, i, raw_name, sizeof(raw_name),
                    &raw_name_size) || raw_name_size == 0u) {
                free(data);
                return 0;
            }
        }
        out->item_name_table_verified = 1;
        for (i = 0u; i < THERON_TRACK19_JP_LEVEL_LABEL_COUNT; ++i) {
            if (!theron_v1_track19_jp_level_label_from_iso(
                    data, bytes, i, raw_label, sizeof(raw_label),
                    &raw_label_size) || raw_label_size == 0u) {
                free(data);
                return 0;
            }
        }
        out->level_label_table_verified = 1;
    }
    if (!theron_v1_track19_opaque_record_window_validate(
            data, bytes, strcmp(out->variant, "jp") == 0,
            &out->opaque_record_window_offset,
            &out->opaque_record_window_bytes)) {
        free(data);
        return 0;
    }
    if (!theron_v1_track19_item_property_table_validate(
            data, bytes, strcmp(out->variant, "jp") == 0,
            &out->item_property_table_offset,
            &out->item_property_table_bytes)) {
        free(data);
        return 0;
    }
    if (!theron_v1_track19_startup_level_envelope_validate(
            data, bytes, &out->startup_level_envelope_offset,
            &out->startup_level_envelope_bytes,
            &out->startup_level_envelope_fnv1a)) {
        free(data);
        return 0;
    }
    out->item_property_table_verified = 1;
    out->opaque_record_window_verified = 1;
    out->startup_level_envelope_verified = 1;
    free(data);
    return 1;
}
