#ifndef DM1_V1_IIGS_METADATA_H
#define DM1_V1_IIGS_METADATA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_IigsProgramVersion {
    DM1_V1_IIGS_PROGRAM_UNKNOWN=0,
    DM1_V1_IIGS_PROGRAM_G20=20,
    DM1_V1_IIGS_PROGRAM_G21=21
} DM1_V1_IigsProgramVersion;

/* Copied native GS program-owned startup/UI and ornament/object metadata.
 * Palettes are RGB4 words; palette-change entries are native indices 0..15,
 * not another platform's ten-times-index representation. Frames contain
 * X1,X2,Y1,Y2,packed-byte-width,height; aspects contain the six native bytes.
 * Palette-change pairs use D3 then D2 order. No input pointers are retained.
 * This set covers DATA.C G0000..G0021 and DUNVIEW.C G0190..G0218 tables
 * used here; it does not claim complete executable/runtime parity. */
typedef struct DM1_V1_IigsMetadata {
    DM1_V1_IigsProgramVersion program_version;
    uint16_t credits_palette[16];
    uint16_t entrance_palette[16];
    uint16_t interface_palette[16];
    uint16_t dungeon_palettes[6][16];
    uint16_t ui_boxes[17][4];
    uint8_t palette_no_changes[16];
    uint16_t mandatory_graphics[70];
    uint8_t object_aspects[85][6];
    uint8_t object_coordinates[3][10][5][2];
    uint8_t object_palette_changes[2][16];
    uint8_t wall_frames[8][13][6];
    uint8_t floor_frames[3][9][6];
    uint8_t door_frames[4][3][6];
    uint8_t door_button_frames[4][6];
    uint8_t wall_palette_changes[2][16];
    uint8_t door_palette_changes[2][16];
    uint8_t wall_coordinate_sets[60];
    uint8_t floor_coordinate_sets[9];
    uint8_t door_coordinate_sets[12];
    uint8_t wall_bitmap_increments[12];
    uint8_t floor_bitmap_increments[9];
    uint8_t unreadable_inscription_y2[15];
    uint8_t inscription_line_y[4];
    uint8_t wall_inscription_patch[4];
} DM1_V1_IigsMetadata;

/* Require exact original G20E or G21E DUNGEON.MASTER identity before
 * reading their independently verified native table offsets. Returns 1 on
 * success; 0 on failure, with a zeroed output. This authenticates metadata's
 * program owner only: it does not admit a complete game/platform package. */
int dm1_v1_iigs_metadata_read(const uint8_t *program, size_t size,
                             DM1_V1_IigsMetadata *out);

#ifdef __cplusplus
}
#endif
#endif
