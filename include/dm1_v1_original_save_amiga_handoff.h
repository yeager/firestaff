#ifndef DM1_V1_ORIGINAL_SAVE_AMIGA_HANDOFF_H
#define DM1_V1_ORIGINAL_SAVE_AMIGA_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_original_save_classifier.h"

struct GameWorld_Compat;

#ifdef __cplusplus
extern "C" {
#endif

/* Read-only, big-endian F0435 receipt for original DM 2.x Amiga saves.
 *
 * This is intentionally distinct from the PC 3.4 importer.  The source
 * order below is taken from ReDMCSB's Amiga MEDIA A20 branch:
 *   DEFS.H:468-480, 503-517, 538-587, 838-920
 *   READWRIT.C:F0417/F0419 (191-242)
 *   LOADSAVE.C:F0435 (2665-2827)
 *
 * The reader authenticates the original header, five F0435 save parts, and
 * the following F0434 dungeon stream in memory.  It does not turn those
 * big-endian source bytes into a PC34 world; a later runtime adapter must
 * remain format-specific.
 */
enum {
    DM1_V1_AMIGA_SAVE_F0435_PART_COUNT = 5,
    DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES = 512,
    DM1_V1_AMIGA_SAVE_F0435_GLOBAL_DATA_BYTES = 128,
    DM1_V1_AMIGA_SAVE_F0435_ACTIVE_GROUP_BYTES = 16,
    /* DEFS.H GAME_HINT documents 128 + 4*320 = 1408 on Amiga.  The
     * PC3.4 compact record is 319 bytes and is deliberately not reused. */
    DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES = 1408,
    DM1_V1_AMIGA_SAVE_F0435_EVENT_BYTES = 10,
    DM1_V1_AMIGA_SAVE_PORTRAIT_BYTES = 464,
    DM1_V1_AMIGA_SAVE_PORTRAIT_COUNT = 4,
    DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES = 44,
    DM1_V1_AMIGA_SAVE_DUNGEON_MAP_BYTES = 16
};

typedef enum {
    DM1_V1_AMIGA_SAVE_F0435_OK = 0,
    DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT = -1,
    DM1_V1_AMIGA_SAVE_F0435_ERR_NOT_AMIGA_FORMAT5 = -2,
    DM1_V1_AMIGA_SAVE_F0435_ERR_HEADER = -3,
    DM1_V1_AMIGA_SAVE_F0435_ERR_BODY = -4,
    DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY = -5,
    DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL = -6
} Dm1V1AmigaSaveF0435Result;

typedef struct {
    DM1OriginalSaveClassifyResult classify;
    uint16_t expected_checksums[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    uint16_t actual_checksums[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    uint32_t part_offsets[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    uint32_t part_byte_counts[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    uint32_t authenticated_body_end_offset;
    uint32_t trailing_source_byte_count;
    uint32_t portrait_byte_count;
    uint32_t dungeon_offset;
    uint32_t dungeon_byte_count;
    uint32_t dungeon_checksum_offset;
    uint16_t dungeon_expected_checksum;
    uint16_t dungeon_actual_checksum;
    uint16_t dungeon_raw_map_byte_count;
    uint16_t dungeon_text_word_count;
    uint16_t dungeon_square_first_thing_count;
    uint16_t dungeon_column_count;
    uint8_t dungeon_map_count;
    uint32_t game_time;
    uint16_t party_champion_count;
    int16_t party_map_x;
    int16_t party_map_y;
    int16_t party_direction;
    int16_t party_map_index;
    int16_t leader_index;
    uint16_t event_count;
    uint16_t first_unused_event_index;
    uint16_t event_maximum_count;
    uint16_t current_active_group_count;
    uint16_t maximum_active_group_count;
    uint16_t parts_authenticated;
    int header_authenticated;
    int body_authenticated;
    int tail_authenticated;
} Dm1V1AmigaSaveF0435Receipt;

/* Plaintext GLOBAL_DATA after the full original F0435 admission.  This is a
 * format-specific big-endian view, not a PC34 PARTY_INFO substitute.  The
 * later world adapter may consume it only together with its separately
 * authenticated C2/C3/C4 parts and F0434 tail. */
typedef struct {
    uint32_t game_time;
    uint16_t party_champion_count;
    int16_t party_map_x;
    int16_t party_map_y;
    int16_t party_direction;
    int16_t party_map_index;
    int16_t leader_index;
    uint16_t event_count;
    uint16_t first_unused_event_index;
    uint16_t event_maximum_count;
    uint16_t current_active_group_count;
    uint16_t maximum_active_group_count;
} Dm1V1AmigaSaveF0435GlobalData;

int dm1_v1_original_save_amiga_f0435_receipt_bytes(
    const uint8_t *bytes,
    size_t size,
    Dm1V1AmigaSaveF0435Receipt *out_receipt);

/* Decodes only GLOBAL_DATA after authenticating the complete original save.
 * The source byte buffer is never modified or written to disk. */
int dm1_v1_original_save_amiga_f0435_global_data_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AmigaSaveF0435GlobalData *out_global,
    Dm1V1AmigaSaveF0435Receipt *out_receipt);

/* Copies the original decrypted C2 party part (four 320-byte Amiga
 * CHAMPION_EXCLUDING_PORTRAIT records plus 128-byte PARTY_INFO) after the
 * complete F0435 admission.  This intentionally exposes bytes, not a PC34
 * champion conversion: the A20-only record padding remains source-owned. */
int dm1_v1_original_save_amiga_f0435_party_part_bytes(
    const uint8_t *bytes, size_t size,
    uint8_t out_party[DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES],
    Dm1V1AmigaSaveF0435Receipt *out_receipt);

/* Copies the authenticated F0434 Amiga dungeon tail, including its original
 * big-endian checksum word.  The caller provides storage and receives the
 * exact required byte count through out_tail_size.  No PC34 loader or endian
 * coercion is involved; this is the input boundary for the Amiga world
 * adapter. */
int dm1_v1_original_save_amiga_f0435_dungeon_tail_bytes(
    const uint8_t *bytes, size_t size,
    uint8_t *out_tail, size_t out_tail_capacity, size_t *out_tail_size,
    Dm1V1AmigaSaveF0435Receipt *out_receipt);

/* Materializes only the authenticated F0434 dungeon into an otherwise-empty
 * GameWorld_Compat.  The destination must be zero-initialized: this function
 * is deliberately transactional and will not replace a live session until
 * the source-specific C2/C3/C4 state adapter exists.  `originalSaveTailBytes`
 * retains an exact in-memory copy of the supplied Amiga bytes for a later
 * A20 serializer; it is never converted to a PC34 source stream. */
int dm1_v1_original_save_amiga_f0435_materialize_dungeon_world_bytes(
    const uint8_t *bytes, size_t size,
    struct GameWorld_Compat *out_world,
    Dm1V1AmigaSaveF0435Receipt *out_receipt);

const char *dm1_v1_original_save_amiga_f0435_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_ORIGINAL_SAVE_AMIGA_HANDOFF_H */
