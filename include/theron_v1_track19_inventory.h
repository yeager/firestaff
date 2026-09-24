#ifndef THERON_V1_TRACK19_INVENTORY_H
#define THERON_V1_TRACK19_INVENTORY_H

#include <stddef.h>
#include <stdint.h>

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
    int item_type_code_table_verified;
    size_t item_type_code_table_offset;
    uint32_t item_type_code_table_fnv1a;
    int level_label_table_verified;
    int item_property_table_verified;
    size_t item_property_table_offset;
    size_t item_property_table_bytes;
    int opaque_record_window_verified;
    size_t opaque_record_window_offset;
    size_t opaque_record_window_bytes;
    int startup_level_envelope_verified;
    size_t startup_level_envelope_offset;
    size_t startup_level_envelope_bytes;
    uint32_t startup_level_envelope_fnv1a;
    uint16_t startup_level_width;
    uint16_t startup_level_height;
    uint16_t startup_level_header_words[6];
    size_t startup_level_payload_bytes;
    size_t startup_level_nonzero_payload_bytes;
    uint32_t startup_level_payload_fnv1a;
    char source_md5[33];
} Theron_V1Track19InventoryReceipt;

#define THERON_V1_TRACK19_ITEM_NAME_COUNT 69u
#define THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY 128u
#define THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT 69u
#define THERON_V1_TRACK19_ITEM_TYPE_CODE_US_OFFSET 0x0E9226u
#define THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_OFFSET 0x0E9266u
#define THERON_V1_TRACK19_ITEM_TYPE_CODE_US_FNV1A 0x21533BB5u
#define THERON_V1_TRACK19_ITEM_TYPE_CODE_JP_FNV1A 0xF9C3EABBu
#define THERON_V1_TRACK19_JP_REV1_RAW_MD5 \
    "27d54f58154662885bb67d5967e5111e"
#define THERON_V1_TRACK19_JP_REV1_RAW_BYTES 7752192u
#define THERON_V1_TRACK19_JP_REV1_PREGAP_SECTORS 224u

/* Lossless runtime bank for the authenticated Track 19 name/type/property
 * tables. JP names remain Shift-JIS bytes. The file reader asserts no Track
 * 02 mapping; the world may prove dungeon 4 only after comparing both banks. */
typedef struct {
    int valid;
    int variant; /* 1 = JP, 2 = US */
    size_t count;
    uint32_t source_span_fnv1a;
    size_t type_code_source_offset;
    uint32_t type_code_source_fnv1a;
    char source_md5[33];
    uint8_t raw_names[THERON_V1_TRACK19_ITEM_NAME_COUNT]
                     [THERON_V1_TRACK19_ITEM_NAME_RAW_CAPACITY];
    uint8_t raw_name_sizes[THERON_V1_TRACK19_ITEM_NAME_COUNT];
    uint8_t raw_type_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT];
    uint32_t property_source_fnv1a;
    uint8_t raw_properties[66u][6u];
    int item_mapping_proven;
    unsigned int mapped_track02_dungeon_mask;
    int host_text_rendering_proven;
} Theron_V1Track19ItemNameBank;

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out);

/* Read a real Track 19 ISO, authenticate its known hash/size, and validate
 * the source-owned US item and level-label spans when applicable. */
int theron_v1_track19_inventory_file(
    const char *path, Theron_V1Track19InventoryReceipt *out);

int theron_v1_track19_item_name_bank_file(
    const char *path, Theron_V1Track19ItemNameBank *out);

/* Authenticate and copy the complete source-owned type-code table that
 * immediately precedes the regional 69-name span. */
int theron_v1_track19_item_type_codes_from_iso(
    const uint8_t *iso, size_t iso_size, int japanese_variant,
    uint8_t out_codes[THERON_V1_TRACK19_ITEM_TYPE_CODE_COUNT],
    size_t *out_offset, uint32_t *out_fnv1a);

#endif
