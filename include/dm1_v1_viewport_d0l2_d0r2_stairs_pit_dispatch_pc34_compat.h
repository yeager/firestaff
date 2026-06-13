#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0L2/D0R2 stairs/pit dispatch gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0116 lines 6361-6480 and F0117 lines 6500-6622:
 *   D3L/D3R side-lane bodies used by this near D0L2/D0R2 parity alias.
 * - DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247:
 *   native and flipped C10-transparent bitmap blit contracts.
 * - DUNVIEW.C F0115 lines 4547-4581 and 5668-5671: object/creature/
 *   projectile/explosion follow-up reached by the F0116/F0117 tails.
 * - DUNVIEW.C F0128 lines 8478-8508: D3L/D3R dispatch order around
 *   the D3L2/D3R2 and D2L2/D2R2 neighbors.
 * - DUNGEON.C F0163/F0164 lines 1769-1840 and F0172 lines 2466-2523:
 *   thing-list and square-aspect map-coordinate handoff.
 * - DEFS.H lines 2088, 2443, 2450, 2582-2583, 2603-2604, 2610-2611,
 *   2662, 2676-2677, 4139-4153, 4197-4198, plus the exact F0116/F0117
 *   D3L/D3R PC34 constants at lines 2441/2448, 2608-2609, 2676-2677,
 *   4141/4143/4154/4156, and 4199/4201.
 */

enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D3L = 0,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D3L = 7,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D3L = 50,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_NO_VISIBLE_PIT_BITMAP = -1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3L = 802,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3R = 804,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3L = 815,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3R = 817,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3L = 852,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3R = 854,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D3L = 1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D3R = 2,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_PC34_VIEW_SQUARE_D3L = 12,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_PC34_VIEW_SQUARE_D3R = 13,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_CELL_ORDER_D3L_OPEN = 0x3421,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_CELL_ORDER_D3R_OPEN = 0x4312,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10
};

typedef enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34 = 0,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34 = 1
} DM1_V1_D0L2D0R2StairsPitSidePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34 = 0,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 2,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34 = 3
} DM1_V1_D0L2D0R2StairsPitRoutePc34;

typedef struct {
    const char *label;
    const char *file;
    const char *function_name;
    int first_line;
    int last_line;
    const char *contract_note;
} DM1_V1_D0L2D0R2StairsPitAnchorPc34;

typedef struct {
    DM1_V1_D0L2D0R2StairsPitSidePc34 side;
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
    const char *role_name;
    const char *draw_square_function;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    const char *dungeon_anchor;
    int perspective_depth;
    int relative_forward;
    int relative_lateral;
    int f0128_dispatch_order;
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
    bool uses_f0115_thing_pass_followup;
    bool d0l2_d0r2_tail_dispatch;
    bool no_door_front_helper_anchor;
    bool no_d2l2_d0r2_mixed_route;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D0L2D0R2StairsPitSpecPc34;

typedef struct {
    DM1_V1_D0L2D0R2StairsPitSidePc34 side;
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D0L2D0R2StairsPitPixelInputPc34;

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
    const DM1_V1_D0L2D0R2StairsPitSpecPc34 *spec;
} DM1_V1_D0L2D0R2StairsPitPixelResultPc34;

size_t dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_count_pc34(void);

const DM1_V1_D0L2D0R2StairsPitSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_at_pc34(size_t index);

const DM1_V1_D0L2D0R2StairsPitSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_pc34(
    DM1_V1_D0L2D0R2StairsPitSidePc34 side,
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route);

const DM1_V1_D0L2D0R2StairsPitAnchorPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_anchor_citations_pc34(
    size_t *count);

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D0L2D0R2StairsPitPixelInputPc34 *input,
    DM1_V1_D0L2D0R2StairsPitPixelResultPc34 *out);

const char *dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
