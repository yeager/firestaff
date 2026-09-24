#include "theron_v1_track19_inventory.h"
#include "theron_v1_track19_item_names.h"
#include "theron_v1_track19_jp_item_names.h"
#include "theron_v1_track19_jp_level_labels.h"
#include "theron_v1_track19_level_labels.h"
#include "theron_v1_track19_record_window.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int find_track19_at_root(const char *root, const char *file_name,
                                const char *raw_file_name, char *fallback,
                                size_t fallback_capacity) {
    int length;
    FILE *file;

    if (!root || !root[0]) return 0;
    length = snprintf(fallback, fallback_capacity, "%s/%s", root, file_name);
    if (length >= 0 && (size_t)length < fallback_capacity &&
        (file = fopen(fallback, "rb")) != NULL) {
        fclose(file);
        return 1;
    }
    if (!raw_file_name) return 0;
    length = snprintf(fallback, fallback_capacity, "%s/%s", root,
                      raw_file_name);
    if (length >= 0 && (size_t)length < fallback_capacity &&
        (file = fopen(fallback, "rb")) != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

static const char *resolve_track19_iso(const char *env_name,
                                       const char *file_name,
                                       const char *raw_file_name,
                                       char *fallback,
                                       size_t fallback_capacity) {
    const char *configured = getenv(env_name);
    const char *theron_root = getenv("FIRESTAFF_THERON_DATA_DIR");
    const char *root = getenv("FIRESTAFF_DATA_DIR");
    const char *home;
    int length;

    if (configured && configured[0]) return configured;
    if (find_track19_at_root(theron_root, file_name, raw_file_name, fallback,
                             fallback_capacity)) return fallback;
    if (root && root[0]) {
        char root_theron[512];
        length = snprintf(root_theron, sizeof(root_theron), "%s/theron", root);
        if (length >= 0 && (size_t)length < sizeof(root_theron))
            if (find_track19_at_root(root_theron, file_name, raw_file_name,
                                     fallback, fallback_capacity)) return fallback;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    {
        char home_theron[512];
        length = snprintf(home_theron, sizeof(home_theron),
                          "%s/.firestaff/data/theron", home);
        if (length >= 0 && (size_t)length < sizeof(home_theron))
            if (find_track19_at_root(home_theron, file_name, raw_file_name,
                                     fallback, fallback_capacity)) return fallback;
    }
    return NULL;
}

static int verify_real_us_item_table(void) {
    char fallback[512];
    const char *path = resolve_track19_iso(
        "THERON_TRACK19_US_ISO", "TQUS19.iso", NULL, fallback, sizeof(fallback));
    FILE *file;
    long size;
    uint8_t *bytes;
    Theron_ItemPropertyRecord property;
    size_t envelope_offset;
    size_t envelope_bytes;
    uint32_t envelope_fnv1a;
    Theron_Track19LevelEnvelope envelope;
    unsigned int i;
    char name[64];
    uint8_t type_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT];

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
    if (!theron_v1_track19_startup_level_envelope_validate(
            bytes, (size_t)size, &envelope_offset, &envelope_bytes,
            &envelope_fnv1a) ||
        envelope_offset != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET ||
        envelope_bytes != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES ||
        envelope_fnv1a != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A) {
        free(bytes);
        return 0;
    }
    if (!theron_v1_track19_startup_level_envelope_read(
            bytes, (size_t)size, &envelope) || envelope.width != 32u ||
        envelope.height != 27u || envelope.payload_bytes != 864u ||
        envelope.payload != bytes + envelope_offset + 12u ||
        envelope.nonzero_payload_bytes == 0u) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET + 12u] ^= 1u;
    if (!theron_v1_track19_item_type_codes_from_iso(
            bytes, (size_t)size, 0, type_codes, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET] ^= 1u;
    if (theron_v1_track19_item_type_codes_from_iso(
            bytes, (size_t)size, 0, type_codes, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET] ^= 1u;
    if (theron_v1_track19_startup_level_envelope_validate(
            bytes, (size_t)size, NULL, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET + 12u] ^= 1u;
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
    if (!theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 0, 65u, &property) ||
        memcmp(&property,
               bytes + THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET +
                   65u * THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES,
               sizeof(property)) != 0) {
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
    char fallback[512];
    const char *path = resolve_track19_iso(
        "THERON_TRACK19_JP_ISO", "TQJP19.iso",
        "Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 19).bin",
        fallback, sizeof(fallback));
    FILE *file;
    long size;
    uint8_t *bytes;
    uint8_t name[128];
    size_t name_size;
    uint8_t label[32];
    size_t label_size;
    Theron_ItemPropertyRecord property;
    size_t envelope_offset;
    size_t envelope_bytes;
    uint32_t envelope_fnv1a;
    Theron_Track19LevelEnvelope envelope;
    unsigned int i;
    uint8_t type_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT];

    if (!path || !path[0]) return 1;
    /* The supplied Japanese Rev 1 image is a raw MODE1/2352 track with a
     * CUE-defined pregap.  The product inventory performs that lossless,
     * in-memory normalization before validating ISO-addressed game records.
     * Direct mutation checks below intentionally remain ISO-only. */
    if (strstr(path, "(Track 19).bin") != NULL) {
        Theron_V1Track19InventoryReceipt receipt;
        int loaded = theron_v1_track19_inventory_file(path, &receipt);
        if (!loaded || !receipt.mode1_2352 || receipt.sector_count != 3296u ||
            !receipt.item_name_table_verified ||
            !receipt.level_label_table_verified ||
            !receipt.item_property_table_verified ||
            !receipt.startup_level_envelope_verified) {
            fprintf(stderr,
                    "FAIL: authentic JP Track 19 raw intake: loaded=%d "
                    "mode1_2352=%d sectors=%zu names=%d labels=%d "
                    "properties=%d startup_envelope=%d md5=%s\n",
                    loaded, receipt.mode1_2352, receipt.sector_count,
                    receipt.item_name_table_verified,
                    receipt.level_label_table_verified,
                    receipt.item_property_table_verified,
                    receipt.startup_level_envelope_verified,
                    receipt.source_md5);
            return 0;
        }
        return 1;
    }
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
    if (!theron_v1_track19_startup_level_envelope_validate(
            bytes, (size_t)size, &envelope_offset, &envelope_bytes,
            &envelope_fnv1a) ||
        envelope_offset != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET ||
        envelope_bytes != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES ||
        envelope_fnv1a != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A) {
        free(bytes);
        return 0;
    }
    if (!theron_v1_track19_startup_level_envelope_read(
            bytes, (size_t)size, &envelope) || envelope.width != 32u ||
        envelope.height != 27u || envelope.payload_bytes != 864u ||
        envelope.payload != bytes + envelope_offset + 12u ||
        envelope.nonzero_payload_bytes == 0u) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET + 12u] ^= 1u;
    if (!theron_v1_track19_item_type_codes_from_iso(
            bytes, (size_t)size, 1, type_codes, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET] ^= 1u;
    if (theron_v1_track19_item_type_codes_from_iso(
            bytes, (size_t)size, 1, type_codes, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET] ^= 1u;
    if (theron_v1_track19_startup_level_envelope_validate(
            bytes, (size_t)size, NULL, NULL, NULL)) {
        free(bytes);
        return 0;
    }
    bytes[THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET + 12u] ^= 1u;
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
    if (!theron_v1_track19_item_property_from_iso(
            bytes, (size_t)size, 1, 65u, &property) ||
        memcmp(&property,
               bytes + THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET +
                   65u * THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES,
               sizeof(property)) != 0) {
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
    Theron_V1Track19ItemNameBank name_bank;
    unsigned int name_index;
    char us_fallback[512];
    char jp_fallback[512];
    const char *real_iso = resolve_track19_iso(
        "THERON_TRACK19_US_ISO", "TQUS19.iso", NULL, us_fallback,
        sizeof(us_fallback));
    const char *real_jp_iso = resolve_track19_iso(
        "THERON_TRACK19_JP_ISO", "TQJP19.iso",
        "Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 19).bin", jp_fallback,
        sizeof(jp_fallback));

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
    if (!theron_v1_track19_inventory(
            "27d54f58154662885bb67d5967e5111e", 7752192u, &receipt) ||
        !receipt.mode1_2352 || receipt.mode1_2048 ||
        receipt.sector_count != 3296u || !receipt.container_format_unproven ||
        strcmp(receipt.variant, "jp") != 0) {
        return 1;
    }
    if (!verify_real_us_item_table()) return 1;
    if (!verify_real_jp_item_table()) return 1;
    if (real_iso && real_iso[0] &&
        (!theron_v1_track19_inventory_file(real_iso, &file_receipt) ||
         !file_receipt.item_name_table_verified ||
         !file_receipt.item_type_code_table_verified ||
         file_receipt.item_type_code_table_offset !=
             THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET ||
         file_receipt.item_type_code_table_fnv1a !=
             THERON_V1_TRACK19_ITEM_TYPE_CODE_US_FNV1A ||
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
         !file_receipt.startup_level_envelope_verified ||
         file_receipt.startup_level_envelope_offset !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET ||
         file_receipt.startup_level_envelope_bytes !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES ||
         file_receipt.startup_level_envelope_fnv1a !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A ||
         file_receipt.startup_level_width != 32u ||
         file_receipt.startup_level_height != 27u ||
         file_receipt.startup_level_payload_bytes != 864u ||
         file_receipt.startup_level_nonzero_payload_bytes == 0u ||
         file_receipt.startup_level_payload_fnv1a == 0u ||
         file_receipt.startup_usable || file_receipt.level_usable ||
         file_receipt.bitmap_usable ||
         file_receipt.source_md5[0] == '\0')) {
        fprintf(stderr,
                "FAIL: US Track 19 file receipt: names=%d types=%d "
                "type_offset=0x%zx type_hash=%08x labels=%d properties=%d "
                "property_offset=0x%zx property_bytes=%zu opaque=%d "
                "opaque_offset=0x%zx opaque_bytes=%zu envelope=%d "
                "envelope_offset=0x%zx envelope_bytes=%zu "
                "envelope_hash=%08x dimensions=%ux%u payload=%zu "
                "nonzero=%zu payload_hash=%08x source_md5=%s\n",
                file_receipt.item_name_table_verified,
                file_receipt.item_type_code_table_verified,
                file_receipt.item_type_code_table_offset,
                file_receipt.item_type_code_table_fnv1a,
                file_receipt.level_label_table_verified,
                file_receipt.item_property_table_verified,
                file_receipt.item_property_table_offset,
                file_receipt.item_property_table_bytes,
                file_receipt.opaque_record_window_verified,
                file_receipt.opaque_record_window_offset,
                file_receipt.opaque_record_window_bytes,
                file_receipt.startup_level_envelope_verified,
                file_receipt.startup_level_envelope_offset,
                file_receipt.startup_level_envelope_bytes,
                file_receipt.startup_level_envelope_fnv1a,
                (unsigned int)file_receipt.startup_level_width,
                (unsigned int)file_receipt.startup_level_height,
                file_receipt.startup_level_payload_bytes,
                file_receipt.startup_level_nonzero_payload_bytes,
                file_receipt.startup_level_payload_fnv1a,
                file_receipt.source_md5);
        return 1;
    }
    if (real_jp_iso && real_jp_iso[0] &&
        (!theron_v1_track19_inventory_file(real_jp_iso, &file_receipt) ||
         !file_receipt.item_name_table_verified ||
         !file_receipt.item_type_code_table_verified ||
         file_receipt.item_type_code_table_offset !=
             THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET ||
         file_receipt.item_type_code_table_fnv1a !=
             THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_FNV1A ||
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
         !file_receipt.startup_level_envelope_verified ||
         file_receipt.startup_level_envelope_offset !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET ||
         file_receipt.startup_level_envelope_bytes !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES ||
         file_receipt.startup_level_envelope_fnv1a !=
             THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A ||
         file_receipt.startup_level_width != 32u ||
         file_receipt.startup_level_height != 27u ||
         file_receipt.startup_level_payload_bytes != 864u ||
         file_receipt.startup_level_nonzero_payload_bytes == 0u ||
         file_receipt.startup_level_payload_fnv1a == 0u ||
         file_receipt.startup_usable || file_receipt.level_usable ||
         file_receipt.bitmap_usable ||
         file_receipt.source_md5[0] == '\0')) {
        fprintf(stderr,
                "FAIL: JP Track 19 file receipt: names=%d types=%d "
                "type_offset=0x%zx type_hash=%08x labels=%d properties=%d "
                "property_offset=0x%zx property_bytes=%zu opaque=%d "
                "opaque_offset=0x%zx opaque_bytes=%zu envelope=%d "
                "envelope_offset=0x%zx envelope_bytes=%zu "
                "envelope_hash=%08x dimensions=%ux%u payload=%zu "
                "nonzero=%zu payload_hash=%08x source_md5=%s\n",
                file_receipt.item_name_table_verified,
                file_receipt.item_type_code_table_verified,
                file_receipt.item_type_code_table_offset,
                file_receipt.item_type_code_table_fnv1a,
                file_receipt.level_label_table_verified,
                file_receipt.item_property_table_verified,
                file_receipt.item_property_table_offset,
                file_receipt.item_property_table_bytes,
                file_receipt.opaque_record_window_verified,
                file_receipt.opaque_record_window_offset,
                file_receipt.opaque_record_window_bytes,
                file_receipt.startup_level_envelope_verified,
                file_receipt.startup_level_envelope_offset,
                file_receipt.startup_level_envelope_bytes,
                file_receipt.startup_level_envelope_fnv1a,
                (unsigned int)file_receipt.startup_level_width,
                (unsigned int)file_receipt.startup_level_height,
                file_receipt.startup_level_payload_bytes,
                file_receipt.startup_level_nonzero_payload_bytes,
                file_receipt.startup_level_payload_fnv1a,
                file_receipt.source_md5);
        return 1;
    }
    if (real_iso && real_iso[0]) {
        if (!theron_v1_track19_item_name_bank_file(real_iso, &name_bank) ||
            !name_bank.valid || name_bank.variant != 2 ||
            name_bank.count != THERON_V1_TRACK19_ITEM_NAME_COUNT ||
            name_bank.source_span_fnv1a != 0x5be5602du ||
            name_bank.type_code_source_offset !=
                THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET ||
            name_bank.type_code_source_fnv1a !=
                THERON_V1_TRACK19_ITEM_TYPE_CODE_US_FNV1A ||
            strcmp(name_bank.source_md5,
                   "51b40a17b92a30339957ba564aa0015c") != 0 ||
            name_bank.item_mapping_proven ||
            name_bank.host_text_rendering_proven ||
            name_bank.raw_name_sizes[0] != 7u ||
            memcmp(name_bank.raw_names[0], "COMPASS", 7u) != 0)
            return 1;
        for (name_index = 0u; name_index < name_bank.count; ++name_index)
            if (name_bank.raw_name_sizes[name_index] == 0u) return 1;
    }
    if (real_jp_iso && real_jp_iso[0]) {
        static const unsigned char jp_first_name[8] = {
            0x83u, 0x52u, 0x83u, 0x93u,
            0x83u, 0x70u, 0x83u, 0x58u
        };
        if (!theron_v1_track19_item_name_bank_file(real_jp_iso, &name_bank) ||
            !name_bank.valid || name_bank.variant != 1 ||
            name_bank.count != THERON_V1_TRACK19_ITEM_NAME_COUNT ||
            name_bank.source_span_fnv1a != 0x1020ac88u ||
            name_bank.type_code_source_offset !=
                THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET ||
            name_bank.type_code_source_fnv1a !=
                THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_FNV1A ||
            strcmp(name_bank.source_md5, file_receipt.source_md5) != 0 ||
            name_bank.item_mapping_proven ||
            name_bank.host_text_rendering_proven ||
            name_bank.raw_name_sizes[0] != sizeof(jp_first_name) ||
            memcmp(name_bank.raw_names[0], jp_first_name,
                   sizeof(jp_first_name)) != 0) {
            fprintf(stderr,
                    "FAIL: JP Track 19 item-name bank: valid=%d variant=%d "
                    "count=%zu span=%08x type_offset=0x%zx type_hash=%08x "
                    "source_md5=%s mapping=%d host_text=%d first_size=%u "
                    "first_bytes=%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    name_bank.valid, name_bank.variant, name_bank.count,
                    name_bank.source_span_fnv1a,
                    name_bank.type_code_source_offset,
                    name_bank.type_code_source_fnv1a, name_bank.source_md5,
                    name_bank.item_mapping_proven,
                    name_bank.host_text_rendering_proven,
                    (unsigned int)name_bank.raw_name_sizes[0],
                    (unsigned int)name_bank.raw_names[0][0],
                    (unsigned int)name_bank.raw_names[0][1],
                    (unsigned int)name_bank.raw_names[0][2],
                    (unsigned int)name_bank.raw_names[0][3],
                    (unsigned int)name_bank.raw_names[0][4],
                    (unsigned int)name_bank.raw_names[0][5],
                    (unsigned int)name_bank.raw_names[0][6],
                    (unsigned int)name_bank.raw_names[0][7]);
            return 1;
        }
        for (name_index = 0u; name_index < name_bank.count; ++name_index)
            if (name_bank.raw_name_sizes[name_index] == 0u) return 1;
        {
            Theron_V1_World *world =
                (Theron_V1_World *)calloc(1u, sizeof(*world));
            const uint8_t *raw_name = NULL;
            size_t raw_name_size = 0u;
            if (!world ||
                !theron_v1_world_bind_track19_item_name_bank(
                    world, &name_bank, 1) ||
                !theron_v1_world_track19_item_name_raw(
                    world, 0u, &raw_name, &raw_name_size) ||
                raw_name_size != sizeof(jp_first_name) ||
                memcmp(raw_name, jp_first_name, sizeof(jp_first_name)) != 0 ||
                theron_v1_world_track19_item_name_raw(
                    world, THERON_V1_TRACK19_ITEM_NAME_COUNT,
                    &raw_name, &raw_name_size) ||
                theron_v1_world_bind_track19_item_name_bank(
                    world, &name_bank, 2) ||
                world->track19_item_names.valid) {
                free(world);
                return 1;
            }
            free(world);
        }
    }
    return 0;
}
