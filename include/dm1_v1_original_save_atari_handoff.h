#ifndef DM1_V1_ORIGINAL_SAVE_ATARI_HANDOFF_H
#define DM1_V1_ORIGINAL_SAVE_ATARI_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_original_save_classifier.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_ATARI_SAVE_F0435_HEADER_BYTES = 512,
    DM1_V1_ATARI_SAVE_PART_COUNT = 5,
    DM1_V1_ATARI_SAVE_GLOBAL_BYTES = 128,
    DM1_V1_ATARI_SAVE_ACTIVE_GROUP_BYTES = 16,
    DM1_V1_ATARI_SAVE_CHAMPION_BYTES = 800,
    DM1_V1_ATARI_SAVE_PARTY_INFO_BYTES = 128,
    DM1_V1_ATARI_SAVE_PARTY_BYTES =
        4 * DM1_V1_ATARI_SAVE_CHAMPION_BYTES +
        DM1_V1_ATARI_SAVE_PARTY_INFO_BYTES,
    DM1_V1_ATARI_SAVE_EVENT_BYTES = 10,
    DM1_V1_ATARI_SAVE_TIMELINE_BYTES = 2,
    DM1_V1_ATARI_DUNGEON_HEADER_BYTES = 44,
    DM1_V1_ATARI_DUNGEON_MAP_BYTES = 16,
    DM1_V1_ATARI_MAX_DUNGEON_MAPS = 32,
    DM1_V1_ATARI_MAX_ACTIVE_GROUPS = 2048,
    DM1_V1_ATARI_MAX_EVENTS = 4096
};

typedef enum {
    DM1_V1_ATARI_SAVE_OK = 0,
    DM1_V1_ATARI_SAVE_ERR_ARGUMENT = -1,
    DM1_V1_ATARI_SAVE_ERR_NOT_FORMAT1 = -2,
    DM1_V1_ATARI_SAVE_ERR_HEADER = -3,
    DM1_V1_ATARI_SAVE_ERR_PART = -4,
    DM1_V1_ATARI_SAVE_ERR_COUNTS = -5,
    DM1_V1_ATARI_SAVE_ERR_DUNGEON = -6
} Dm1V1AtariSaveResult;

/* Read-only admission receipt for legacy FormatID 1 Dungeon Master Atari ST
 * saves. The v1.0 on-disk champion records include 464-byte portraits; they
 * are not Amiga format-5's 320-byte champion records. The receipt authenticates
 * each keyed F0435 part and accounts for the complete unchecksummed F0434
 * dungeon tail. It does not materialize or publish runtime state. */
typedef struct {
    DM1OriginalSaveClassifyResult classify;
    uint16_t expected_checksums[DM1_V1_ATARI_SAVE_PART_COUNT];
    uint16_t actual_checksums[DM1_V1_ATARI_SAVE_PART_COUNT];
    uint32_t part_offsets[DM1_V1_ATARI_SAVE_PART_COUNT];
    uint32_t part_byte_counts[DM1_V1_ATARI_SAVE_PART_COUNT];
    uint32_t dungeon_offset;
    uint32_t dungeon_byte_count;
    uint32_t dungeon_column_count;
    uint16_t dungeon_ornament_seed;
    uint16_t dungeon_raw_map_byte_count;
    uint16_t dungeon_text_word_count;
    uint16_t dungeon_square_first_thing_count;
    uint16_t party_champion_count;
    uint16_t event_count;
    uint16_t event_capacity;
    uint16_t current_active_group_count;
    uint16_t maximum_active_group_count;
    uint8_t dungeon_map_count;
    uint8_t parts_authenticated;
    int header_authenticated;
    int body_authenticated;
    int dungeon_layout_authenticated;
} Dm1V1AtariSaveF0435Receipt;

int dm1_v1_original_save_atari_f0435_receipt_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AtariSaveF0435Receipt *out_receipt);

const char *dm1_v1_original_save_atari_f0435_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif
