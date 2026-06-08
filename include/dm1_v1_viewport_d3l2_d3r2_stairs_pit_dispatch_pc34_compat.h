#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3L2/D3R2 stairs/pit dispatch gate.
 * ReDMCSB anchors verified in:
 * DUNVIEW.C F0676 lines 6226-6291, F0677 lines 6293-6358,
 * F0116 lines 6361-6480, F0117 lines 6500-6622, F0678 lines
 * 6837-6866, F0104 lines 3113-3156, F0105 lines 3185-3247,
 * F0115 lines 4547-4581, F0128 lines 8478-8508, and F0127
 * line 8294; DUNGEON.C F0163 lines 1769-1838, F0164 lines
 * 1840-1905+, F0172 lines 2466-2523; DEFS.H lines 2088,
 * 2443/2450, 2596-2611, 2662/2676, 3674-3677, 4139-4153,
 * and 4197-4198.
 */

enum {
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_NEGGRAPHIC_D3L2 = -18,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_NEGGRAPHIC_D3L2 = -20,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_NEGGRAPHIC_D3R2 = -19,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_NEGGRAPHIC_D3R2 = -21,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D3L2 = 49,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_FLOOR_PIT_VISIBLE_FOLLOWUP_GRAPHIC_D3L2 = -1,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3L2 = 800,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3R2 = 801,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3L2 = 813,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3R2 = 814,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3L2 = 850,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3R2 = 851,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_VIEW_SQUARE_D3L2 = 14,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_VIEW_SQUARE_D3R2 = 15,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_VIEW_SQUARE_D3L2_PC34 = 14,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_VIEW_SQUARE_D3R2_PC34 = 15,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_CELL_ORDER_OPEN_MIRRORED = 0x4312,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10
};

typedef enum {
    DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_SIDE_D3R2_PC34 = 1
} DM1_V1_D3L2D3R2StairsPitSidePc34;

typedef enum {
    DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34 = 0,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 2,
    DM1_V1_D3L2_D3R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34 = 3
} DM1_V1_D3L2D3R2StairsPitRoutePc34;

typedef struct {
    const char *label;
    const char *file;
    const char *function_name;
    int first_line;
    int last_line;
    const char *contract_note;
} DM1_V1_D3L2D3R2StairsPitAnchorPc34;

typedef struct {
    DM1_V1_D3L2D3R2StairsPitSidePc34 side;
    DM1_V1_D3L2D3R2StairsPitRoutePc34 route;
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
    bool uses_f0128_post_d3l2_d3r2_wall_followup;
    bool no_f0111_door_dispatch;
    bool f0120_cpsf_callout;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D3L2D3R2StairsPitSpecPc34;

typedef struct {
    DM1_V1_D3L2D3R2StairsPitSidePc34 side;
    DM1_V1_D3L2D3R2StairsPitRoutePc34 route;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D3L2D3R2StairsPitPixelInputPc34;

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
    const DM1_V1_D3L2D3R2StairsPitSpecPc34 *spec;
} DM1_V1_D3L2D3R2StairsPitPixelResultPc34;

size_t dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_count_pc34(void);

const DM1_V1_D3L2D3R2StairsPitSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_at_pc34(size_t index);

const DM1_V1_D3L2D3R2StairsPitSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D3L2D3R2StairsPitSidePc34 side,
    DM1_V1_D3L2D3R2StairsPitRoutePc34 route);

const DM1_V1_D3L2D3R2StairsPitAnchorPc34 *
dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count);

bool dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D3L2D3R2StairsPitPixelInputPc34 *input,
    DM1_V1_D3L2D3R2StairsPitPixelResultPc34 *out);

const char *dm1_v1_viewport_d3l2_d3r2_stairs_pit_dispatch_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
