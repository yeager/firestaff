#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2L/D2R stairs/pit dispatch gate.
 * ReDMCSB anchors verified in:
 * DUNVIEW.C F0119 lines 6900-7049, F0120_CPSF lines 7051-7220,
 * F0104 lines 3113-3156, F0105 lines 3185-3247, F0115 lines
 * 4547-4581, F0128 lines 8503-8517; DUNGEON.C F0163 lines
 * 1769-1838, F0164 lines 1840-1905+, F0172 lines 2466-2523;
 * DEFS.H lines 2088, 2443-2452, 2582-2583, 2596-2604, 2662,
 * 4144-4162, and 4202-4207.
 */

enum {
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D2L = 2,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D2L = 9,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D2L = 52,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D2L = 58,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D2L = 805,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D2R = 807,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D2L = 818,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D2R = 820,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D2L = 855,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D2R = 857,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2L = 4,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2R = 5,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2L_PC34 = 7,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2R_PC34 = 8,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10
};

typedef enum {
    DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34 = 0,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34 = 1
} DM1_V1_D2L2D2R2StairsPitSidePc34;

typedef enum {
    DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34 = 0,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 2,
    DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 = 3
} DM1_V1_D2L2D2R2StairsPitRoutePc34;

typedef struct {
    const char *label;
    const char *file;
    const char *function_name;
    int first_line;
    int last_line;
    const char *contract_note;
} DM1_V1_D2L2D2R2StairsPitAnchorPc34;

typedef struct {
    DM1_V1_D2L2D2R2StairsPitSidePc34 side;
    DM1_V1_D2L2D2R2StairsPitRoutePc34 route;
    const char *role_name;
    const char *draw_square_function;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    const char *dungeon_anchor;
    int perspective_depth;
    int relative_forward;
    int relative_lateral;
    int f0128_preceding_wall_lateral;
    int f0128_preceding_wall_order;
    int f0128_draw_order;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int first_stairs_graphic_index;
    int zone_index;
    int legacy_view_square_index;
    int pc34_view_square_index;
    int cell_order_backleft_backright;
    int cell_order_open_followup;
    int transparent_color;
    bool uses_f0104_native;
    bool uses_f0105_flipped;
    bool uses_f0108_floor_followup;
    bool uses_f0112_ceiling_followup;
    bool uses_f0115_thing_pass_followup;
    bool uses_f0128_post_d2l2_d2r2_wall_followup;
    bool no_f0111_door_dispatch;
    bool f0120_cpsf_callout;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2L2D2R2StairsPitSpecPc34;

typedef struct {
    DM1_V1_D2L2D2R2StairsPitSidePc34 side;
    DM1_V1_D2L2D2R2StairsPitRoutePc34 route;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2L2D2R2StairsPitPixelInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool flipped_horizontally;
    bool wrote_any;
    bool transparent_skip_seen;
    size_t row_width;
    size_t height;
    size_t byte_count;
    size_t destination_stride;
    size_t writes;
    size_t transparent_skips;
    uint8_t first_source_byte;
    uint8_t last_source_byte;
    uint8_t first_destination_byte;
    uint8_t last_destination_byte;
    const DM1_V1_D2L2D2R2StairsPitSpecPc34 *spec;
} DM1_V1_D2L2D2R2StairsPitPixelResultPc34;

size_t dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_count_pc34(void);

const DM1_V1_D2L2D2R2StairsPitSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_at_pc34(size_t index);

const DM1_V1_D2L2D2R2StairsPitSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D2L2D2R2StairsPitSidePc34 side,
    DM1_V1_D2L2D2R2StairsPitRoutePc34 route);

const DM1_V1_D2L2D2R2StairsPitAnchorPc34 *
dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count);

bool dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D2L2D2R2StairsPitPixelInputPc34 *input,
    DM1_V1_D2L2D2R2StairsPitPixelResultPc34 *out);

const char *dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
