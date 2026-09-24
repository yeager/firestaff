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

/* Japanese Rev 1 CUE: Track 19 is MODE1/2352 with INDEX 01 at 00:02:74.
 * The raw file therefore retains 224 pregap sectors before the same
 * 3072-sector MODE1/2048 payload authenticated by the ISO receipt. */
static size_t theron_v1_track19_pregap_sectors(const char *md5,
                                                size_t bytes)
{
    if (md5 && strcmp(md5, THERON_V1_TRACK19_JP_REV1_RAW_MD5) == 0 &&
        bytes == THERON_V1_TRACK19_JP_REV1_RAW_BYTES) {
        return THERON_V1_TRACK19_JP_REV1_PREGAP_SECTORS;
    }
    return 0u;
}

int theron_v1_track19_item_type_codes_from_iso(
        const uint8_t *iso, size_t iso_size, int japanese_variant,
        uint8_t out_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT],
        size_t *out_offset, uint32_t *out_fnv1a) {
    const size_t offset = japanese_variant
        ? THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET
        : THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET;
    const uint32_t expected = japanese_variant
        ? THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_FNV1A
        : THERON_V1_TRACK19_ITEM_TYPE_CODE_US_FNV1A;
    uint32_t hash = 2166136261u;
    size_t i;

    if (!iso || !out_codes ||
        iso_size < offset + THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT) return 0;
    for (i = 0u; i < THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT; ++i) {
        hash ^= iso[offset + i];
        hash *= 16777619u;
    }
    if (hash != expected) return 0;
    memcpy(out_codes, iso + offset, THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT);
    if (out_offset) *out_offset = offset;
    if (out_fnv1a) *out_fnv1a = hash;
    return 1;
}

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
    } else if (strcmp(md5, "f9f069a5e489b91207f3156059b756f1") == 0 ||
               strcmp(md5, THERON_V1_TRACK19_JP_REV1_RAW_MD5) == 0) {
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
    size_t normalized_bytes;
    size_t sector_bytes;
    size_t sector_count;
    size_t pregap_sectors;
    size_t payload_sector_count;
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
        (!out->mode1_2048 && !out->mode1_2352)) {
        fclose(file);
        return 0;
    }
    sector_bytes = out->mode1_2352 ? 2352u : 2048u;
    sector_count = bytes / sector_bytes;
    pregap_sectors = out->mode1_2352
        ? theron_v1_track19_pregap_sectors(md5, bytes) : 0u;
    if (pregap_sectors > sector_count) {
        fclose(file);
        return 0;
    }
    payload_sector_count = sector_count - pregap_sectors;
    normalized_bytes = payload_sector_count * 2048u;
    data = (uint8_t *)malloc(normalized_bytes);
    if (!data) {
        free(data);
        fclose(file);
        return 0;
    }
    if (out->mode1_2048) {
        if (fread(data, 1u, normalized_bytes, file) != normalized_bytes) {
            free(data);
            fclose(file);
            return 0;
        }
    } else {
        /* Track 19 records use MODE1/2048 offsets even when the transport is
         * a raw 2352-byte image. Strip only the 16-byte sector header; keep
         * the authenticated raw hash and transport identity in `out`. */
        for (i = 0u; i < payload_sector_count; ++i) {
            if (fseek(file, (long)((i + pregap_sectors) * 2352u + 16u), SEEK_SET) != 0 ||
                fread(data + i * 2048u, 1u, 2048u, file) != 2048u) {
                free(data);
                fclose(file);
                return 0;
            }
        }
    }
    fclose(file);
    snprintf(out->source_md5, sizeof(out->source_md5), "%s", md5);
    if (strcmp(out->variant, "us") == 0) {
        for (i = 0u; i < THERON_TRACK19_US_ITEM_NAME_COUNT; ++i) {
            if (!theron_v1_track19_us_item_name_from_iso(
                    data, normalized_bytes, i, text, sizeof(text))) {
                free(data);
                return 0;
            }
        }
        out->item_name_table_verified = 1;
        for (i = 0u; i < THERON_TRACK19_US_LEVEL_LABEL_COUNT; ++i) {
            if (!theron_v1_track19_us_level_label_from_iso(
                    data, normalized_bytes, i, text, sizeof(text))) {
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
                    data, normalized_bytes, i, raw_name, sizeof(raw_name),
                    &raw_name_size) || raw_name_size == 0u) {
                free(data);
                return 0;
            }
        }
        out->item_name_table_verified = 1;
        for (i = 0u; i < THERON_TRACK19_JP_LEVEL_LABEL_COUNT; ++i) {
            if (!theron_v1_track19_jp_level_label_from_iso(
                    data, normalized_bytes, i, raw_label, sizeof(raw_label),
                    &raw_label_size) || raw_label_size == 0u) {
                free(data);
                return 0;
            }
        }
        out->level_label_table_verified = 1;
    }
    {
        uint8_t type_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT];
        if (!theron_v1_track19_item_type_codes_from_iso(
                data, normalized_bytes, strcmp(out->variant, "jp") == 0,
                type_codes, &out->item_type_code_table_offset,
                &out->item_type_code_table_fnv1a)) {
            free(data);
            return 0;
        }
        out->item_type_code_table_verified = 1;
    }
    if (!theron_v1_track19_opaque_record_window_validate(
            data, normalized_bytes, strcmp(out->variant, "jp") == 0,
            &out->opaque_record_window_offset,
            &out->opaque_record_window_bytes)) {
        free(data);
        return 0;
    }
    if (!theron_v1_track19_item_property_table_validate(
            data, normalized_bytes, strcmp(out->variant, "jp") == 0,
            &out->item_property_table_offset,
            &out->item_property_table_bytes)) {
        free(data);
        return 0;
    }
    Theron_Track19LevelEnvelope envelope;
    if (!theron_v1_track19_startup_level_envelope_read(data, normalized_bytes,
                                                        &envelope)) {
        free(data);
        return 0;
    }
    out->item_property_table_verified = 1;
    out->opaque_record_window_verified = 1;
    out->startup_level_envelope_verified = 1;
    out->startup_level_envelope_offset = envelope.envelope_offset;
    out->startup_level_envelope_bytes = envelope.envelope_bytes;
    out->startup_level_envelope_fnv1a = envelope.envelope_fnv1a;
    out->startup_level_width = envelope.width;
    out->startup_level_height = envelope.height;
    memcpy(out->startup_level_header_words, envelope.header_words,
           sizeof(out->startup_level_header_words));
    out->startup_level_payload_bytes = envelope.payload_bytes;
    out->startup_level_nonzero_payload_bytes = envelope.nonzero_payload_bytes;
    out->startup_level_payload_fnv1a = envelope.payload_fnv1a;
    free(data);
    return 1;
}

int theron_v1_track19_item_name_bank_file(
        const char *path, Theron_V1Track19ItemNameBank *out) {
    Theron_V1Track19InventoryReceipt inventory;
    FILE *file;
    long file_size;
    size_t source_bytes, sector_bytes, sector_count, pregap_sectors;
    size_t payload_sector_count, normalized_bytes;
    uint8_t *data;
    unsigned int i;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !out ||
        !theron_v1_track19_inventory_file(path, &inventory) ||
        !inventory.item_name_table_verified ||
        !(file = fopen(path, "rb"))) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || file_size > 64L * 1024L * 1024L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file); return 0;
    }
    source_bytes = (size_t)file_size;
    sector_bytes = inventory.mode1_2352 ? 2352u : 2048u;
    sector_count = source_bytes / sector_bytes;
    /* Match the inventory reader's hash-bound CUE pregap handling so every
     * ISO-addressed Track 19 table has the same source-relative offsets. */
    pregap_sectors = inventory.mode1_2352
        ? theron_v1_track19_pregap_sectors(inventory.source_md5, source_bytes)
        : 0u;
    if (pregap_sectors > sector_count) {
        fclose(file);
        return 0;
    }
    payload_sector_count = sector_count - pregap_sectors;
    normalized_bytes = payload_sector_count * 2048u;
    data = (uint8_t *)malloc(normalized_bytes);
    if (!data) { fclose(file); return 0; }
    if (inventory.mode1_2048) {
        if (fread(data, 1u, normalized_bytes, file) != normalized_bytes) {
            free(data); fclose(file); return 0;
        }
    } else {
        for (i = 0u; i < payload_sector_count; ++i) {
            if (fseek(file,
                      (long)((i + pregap_sectors) * 2352u + 16u),
                      SEEK_SET) != 0 ||
                fread(data + i * 2048u, 1u, 2048u, file) != 2048u) {
                free(data); fclose(file); return 0;
            }
        }
    }
    fclose(file);
    if (strcmp(inventory.variant, "us") == 0) {
        char name[THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY];
        out->variant = 2;
        out->source_span_fnv1a = 0x5be5602du;
        out->type_code_source_offset =
            THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET;
        out->type_code_source_fnv1a =
            THERON_V1_TRACK19_ITEM_TYPE_CODE_US_FNV1A;
        for (i = 0u; i < THERON_V1_TRACK19_ITEM_NAME_COUNT; ++i) {
            size_t length;
            if (!theron_v1_track19_us_item_name_from_iso(
                    data, normalized_bytes, i, name, sizeof(name)) ||
                (length = strlen(name)) == 0u ||
                length >= THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY) {
                free(data); memset(out, 0, sizeof(*out)); return 0;
            }
            memcpy(out->raw_names[i], name, length);
            out->raw_name_sizes[i] = (uint8_t)length;
        }
    } else if (strcmp(inventory.variant, "jp") == 0) {
        out->variant = 1;
        out->source_span_fnv1a = 0x1020ac88u;
        out->type_code_source_offset =
            THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET;
        out->type_code_source_fnv1a =
            THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_FNV1A;
        for (i = 0u; i < THERON_V1_TRACK19_ITEM_NAME_COUNT; ++i) {
            size_t length = 0u;
            if (!theron_v1_track19_jp_item_name_from_iso(
                    data, normalized_bytes, i, out->raw_names[i],
                    THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY, &length) ||
                length == 0u ||
                length >= THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY) {
                free(data); memset(out, 0, sizeof(*out)); return 0;
            }
            out->raw_name_sizes[i] = (uint8_t)length;
        }
    } else {
        free(data); return 0;
    }
    if (!theron_v1_track19_item_type_codes_from_iso(
            data, normalized_bytes, out->variant == 1,
            out->raw_type_codes, NULL, NULL)) {
        free(data); memset(out, 0, sizeof(*out)); return 0;
    }
    {
        const size_t property_offset = out->variant == 1
            ? THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET
            : THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET;
        memcpy(out->raw_properties, data + property_offset,
               THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES);
        out->property_source_fnv1a =
            THERON_TRACK19_ITEM_PROPERTY_TABLE_FNV1A;
    }
    free(data);
    out->valid = 1;
    out->count = THERON_V1_TRACK19_ITEM_NAME_COUNT;
    snprintf(out->source_md5, sizeof(out->source_md5), "%s",
             inventory.source_md5);
    out->item_mapping_proven = 0;
    out->mapped_track02_dungeon_mask = 0u;
    out->host_text_rendering_proven = 0;
    return 1;
}
