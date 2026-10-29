#ifndef CSB_V1_ATARI_F0115_CREATURE_PLAN_H
#define CSB_V1_ATARI_F0115_CREATURE_PLAN_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB DUNVIEW.C F0115 creature placement for MEDIA020 Atari ST.
 * This is a plan only: bitmap expansion, palette application, and blitting
 * remain owned by the platform renderer. C558 is the selected original
 * Atari GRAPHICS.DAT item 558 (4,720 bytes), never a generated substitute. */
enum {
    CSB_V1_ATARI_F0115_C558_SIZE = 4720,
    CSB_V1_ATARI_F0115_G0224_OFFSET = 264,
    CSB_V1_ATARI_F0115_G0219_OFFSET = 832,
    CSB_V1_ATARI_F0115_G0219_COUNT = 27,
    CSB_V1_ATARI_F0115_G0219_STRIDE = 12,
    CSB_V1_ATARI_F0115_G0224_SIZE = 330,
    CSB_V1_ATARI_F0115_GRAPHIC_FIRST_CREATURE = 446,
    CSB_V1_ATARI_F0115_SINGLE_CENTERED = 0xff
};

typedef struct CSB_V1_AtariF0115CreatureInput {
    const uint8_t *c558;
    size_t c558_size;
    int creature_type;
    /* ReDMCSB GROUP::Count is a zero-based count (0 means one creature). */
    int creature_count_minus_one;
    uint8_t cells;
    uint16_t directions;
    uint8_t aspect[4];
    int creature_size;       /* G0243 Attributes & 3: quarter=0, half=1, full=2 */
    uint16_t graphic_info;   /* Authentic G0243 GraphicInfo, supplied by caller */
    int party_direction;
    int view_square;         /* F0115 MEDIA020: D3C=0 ... D0C=9, D0L=10, D0R=11 */
    int view_cell;           /* F0115 relative cell ordinal 0..3 */
    int remaining_view_cells;
    int drawing_last_back_row_cell;
    int door_front_view_pass; /* 0 ordinary; 1 first pass; 2 second pass */
} CSB_V1_AtariF0115CreatureInput;

typedef struct CSB_V1_AtariF0115CreaturePlan {
    int creature_index;
    int absolute_cell;
    int coordinate_set;
    int coordinate_cell;
    int anchor_x;
    int anchor_y;
    int source_depth;         /* D3=2, D2=1, D1/D0=0 */
    int bitmap_index;         /* Atari native M618 index, base 446 */
    int source_byte_width;   /* G0219 ByteWidth*, before F0115 scaling */
    int height;
    int transparent_color;   /* raw G0219 M072, for M11 indexed blitter */
    int palette_transparent_color; /* F0115 G0221/G0222 result */
    int direction_delta;
    int mirrored;
    int attacking;
    int uses_side_bitmap;
    int uses_back_bitmap;
    int uses_attack_bitmap;
    int uses_special_d2_front;
    int flipped_special_d2_front;
    int requires_derived_depth_bitmap;
} CSB_V1_AtariF0115CreaturePlan;

/* ReDMCSB F0115 receives a packed OrderedViewCellOrdinals word from each
 * F0124 square script. The low nibble is consumed first; bit 3 on that first
 * nibble identifies a front-facing door pass and its low bit selects pass 1
 * or 2. Remaining nibbles are 1-based view-cell ordinals. */
typedef struct CSB_V1_AtariF0115CellOrder {
    uint8_t count;
    uint8_t door_front_pass; /* 0 ordinary, 1 frame-behind, 2 frame-front */
    uint8_t view_cell[4];    /* ReDMCSB M001_ORDINAL_TO_INDEX result */
    uint16_t remaining_after[4];
    uint8_t drawing_last_back_row_cell[4];
    uint8_t alcove_only;
} CSB_V1_AtariF0115CellOrder;

typedef enum CSB_V1_AtariF0115PassLayer {
    CSB_V1_ATARI_F0115_OBJECT_LAYER = 0,
    CSB_V1_ATARI_F0115_CREATURE_LAYER = 1,
    CSB_V1_ATARI_F0115_PROJECTILE_LAYER = 2,
    CSB_V1_ATARI_F0115_EXPLOSION_LAYER = 3,
    CSB_V1_ATARI_F0115_FLUXCAGE_LAYER = 4
} CSB_V1_AtariF0115PassLayer;

enum { CSB_V1_ATARI_F0115_POST_CELL = 0xffu };

typedef struct CSB_V1_AtariF0115PassEvent {
    uint8_t view_cell;
    uint8_t layer;
    uint16_t remaining_view_cells;
    uint8_t drawing_last_back_row_cell;
    uint8_t door_front_view_pass;
} CSB_V1_AtariF0115PassEvent;

/* Returns 1 for a visible source draw plan, 0 when F0115 has no creature to
 * draw on this source pass/coordinate, and -1 for invalid or unauthenticated
 * input. G0243 size and GraphicInfo must be supplied from the selected
 * platform's source-owned creature profile; C558 does not contain G0243. */
int csb_v1_atari_f0115_plan_creature(
    const CSB_V1_AtariF0115CreatureInput *input,
    CSB_V1_AtariF0115CreaturePlan *out_plan);

/* Decodes authentic MEDIA020 order words such as 0x0218 (door pass 1,
 * view cells 0 and 1), 0x0349 (door pass 2, cells 3 and 2), and 0x3421
 * (all four cells in source order). Returns -1 for malformed nibbles. */
int csb_v1_atari_f0115_decode_cell_order(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_AtariF0115CellOrder *out_order);

/* ReDMCSB DUNVIEW.C F0115 processes objects, creature and projectiles for
 * each view cell, then explosions and (except door pass 1) Fluxcage. Events
 * for the final two layers use CSB_V1_ATARI_F0115_POST_CELL. This plans source
 * dispatch only; the renderer must still filter against live runtime things,
 * source coordinates, and door-pass state. Returns zero for alcove-only. */
int csb_v1_atari_f0115_plan_pass_events(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_AtariF0115PassEvent *out_events,
    size_t event_capacity);

/* ReDMCSB F0115 seeds G0217 with (absolute map cell & 1) << 3, then advances
 * it for each object in the view cell. `view_cell` is relative to the party. */
int csb_v1_atari_f0115_object_shift_index(
    int party_direction, int view_cell, unsigned int pile_ordinal,
    unsigned int *out_shift_index);

/* ReDMCSB DEFS.H M078: round a source dimension using its 32ths scale. */
unsigned int csb_v1_atari_f0115_scaled_dimension(
    unsigned int dimension, unsigned int scale);

/* ReDMCSB DEFS.H C16_SCALE_D3/C20_SCALE_D2/C32_SCALE_D1. */
unsigned int csb_v1_atari_f0115_scale_for_depth(int source_depth);

#endif
