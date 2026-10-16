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

/* Native CREATURE_INFO, DEFS.H:1575–1594. Both unreferenced bytes are
 * retained so the original 26-byte record can be reconstructed exactly. */
typedef struct DM1_V1_IigsCreatureInfo {
    uint8_t creature_aspect_index;
    uint8_t attack_sound_ordinal;
    uint16_t attributes;
    uint16_t graphic_info;
    uint8_t movement_ticks;
    uint8_t attack_ticks;
    uint8_t defense;
    uint8_t base_health;
    uint8_t attack;
    uint8_t poison_attack;
    uint8_t dexterity;
    uint8_t a_unreferenced;
    uint16_t ranges;
    uint16_t properties;
    uint16_t resistances;
    uint16_t animation_ticks;
    uint16_t wound_probabilities;
    uint8_t attack_type;
    uint8_t b_unreferenced;
} DM1_V1_IigsCreatureInfo;

/* Native DEFS.H structures: decode fields explicitly, never fread host structs. */
typedef struct DM1_V1_IigsCreatureAspect {
    uint16_t first_native_bitmap_relative_index;
    uint16_t first_derived_bitmap_index;
    uint8_t byte_width_front, height_front;
    uint8_t byte_width_side, height_side;
    uint8_t byte_width_attack, height_attack;
    uint8_t coordinate_set_transparent_color;
    uint8_t replacement_color_set_ordinals;
} DM1_V1_IigsCreatureAspect;

typedef struct DM1_V1_IigsProjectileAspect {
    uint8_t first_native_bitmap_relative_index;
    uint8_t first_derived_bitmap_relative_index;
    uint8_t byte_width, height;
    uint16_t graphic_info;
} DM1_V1_IigsProjectileAspect;

typedef struct DM1_V1_IigsCreatureReplacementColorSet {
    uint16_t rgb[6];
    uint8_t d2_replacement_color;
    uint8_t d3_replacement_color;
} DM1_V1_IigsCreatureReplacementColorSet;

/* Copied native GS program-owned startup/UI and ornament/object metadata.
 * Palettes are RGB4 words; palette-change entries are native indices 0..15,
 * not another platform's ten-times-index representation. Frames contain
 * X1,X2,Y1,Y2,packed-byte-width,height; aspects contain the six native bytes.
 * Palette-change pairs use D3 then D2 order. No input pointers are retained.
 * This set covers DATA.C G0000..G0021 and DUNVIEW.C G0190..G0218 tables
 * used here, plus native creature/projectile/explosion inputs G0210..G0228.
 * These are data helpers; native runtime integration remains unfinished. */
typedef struct DM1_V1_IigsMetadata {
    DM1_V1_IigsProgramVersion program_version;
    uint16_t credits_palette[16];
    uint16_t entrance_palette[16];
    uint16_t interface_palette[16];
    uint16_t dungeon_palettes[6][16];
    uint16_t ui_boxes[17][4];
    int16_t slot_boxes[46][3]; /* G0030: x,y,cached icon; viewport for8+ */
    int16_t inventory_mouse[38][6]; /* G0449: command, inclusive box, button */
    uint16_t icon_first[7];
    uint16_t slot_masks[38];
    uint16_t food_amounts[8]; /* Native G0242, apple through dragon steak */
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
    DM1_V1_IigsCreatureAspect creature_aspects[27];
    DM1_V1_IigsProjectileAspect projectile_aspects[14];
    DM1_V1_IigsCreatureReplacementColorSet creature_replacement_colors[13];
    uint8_t creature_palette_changes[2][16];
    int16_t shift_sets[3][8];
    uint8_t creature_coordinates[3][11][5][2];
    uint8_t explosion_aspects[4][2];
    uint8_t smoke_palette_changes[16];
    uint8_t projectile_scales[7];
    uint8_t explosion_base_scales[5];
    uint8_t object_pile_shift_sets[16][2];
    int16_t centered_explosion_coordinates[15][2];
    int16_t explosion_coordinates[15][2][2];
    int16_t rebirth_step2_coordinates[7][3];
    int16_t rebirth_step1_coordinates[7][3];
    DM1_V1_IigsCreatureInfo creature_info[27];
} DM1_V1_IigsMetadata;

/* Require exact original G20E or G21E DUNGEON.MASTER identity before
 * reading their independently verified native table offsets. Returns 1 on
 * success; 0 on failure, with a zeroed output. This authenticates metadata's
 * program owner only: it does not admit a complete game/platform package. */
int dm1_v1_iigs_metadata_read(const uint8_t *program, size_t size,
                             DM1_V1_IigsMetadata *out);

typedef struct DM1_V1_IigsCreaturePalette {
    uint16_t dungeon_palettes[6][16];
    uint8_t creature_palette_changes[2][16];
} DM1_V1_IigsCreaturePalette;

/* DUNVIEW.C F0095:2805–2815 and F0093:2008–2014. Start with copied native
 * palettes, restore color 9/set 8 and color 10/set 12, then apply allowed
 * creatures in map order. Packed ordinals are zero=no change, otherwise
 * set index=ordinal-1; the last request for each color wins. Count<=15 is
 * the native map's four-bit CreatureTypeCount limit. Empty lists are valid.
 * Returns 1 on success, 0 with zeroed output on invalid input. No input
 * metadata is changed; no native runtime integration is implied. */
int dm1_v1_iigs_metadata_creature_palette(const DM1_V1_IigsMetadata *metadata,
    const uint8_t *allowed_creatures, size_t count,
    DM1_V1_IigsCreaturePalette *out);

#ifdef __cplusplus
}
#endif
#endif
