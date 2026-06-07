#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only source-lock gate for the D1 side-lane stairs/pit bitmap
 * dispatch used by the D1L2/D1R2 parity work item.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0122 lines 7391-7557: D1L square aspect, F0104 bitmap
 *   dispatch, and F0115 tail at line 7536.
 * - DUNVIEW.C F0123 lines 7559-7725: D1R square aspect, F0105 flipped
 *   bitmap dispatch, and F0115 tail at line 7704.
 * - DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3218: native and
 *   flipped C10-transparent bitmap blit contracts.
 * - DUNVIEW.C F0128 lines 8524-8542: post-D1L/D1R draw order reaches D1C,
 *   D0L, D0R, and D0C; D0C line 8294 uses M609/C0x0021.
 * - DUNGEON.C F0163/F0164 lines 1769-1840 and F0172 lines 2466-2523:
 *   thing-list and square-aspect interaction.
 * - DEFS.H lines 2088, 2445-2452, 2596-2601, 2659-2666, 4147-4162,
 *   and 4205-4207 provide C10, bitmap slots, view squares, cell orders,
 *   stairs zones, and pit zones.
 */

enum {
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L = 4,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L = 11,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L = 54,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L = 60,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1L = 808,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1R = 810,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1L = 821,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1R = 823,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1L = 858,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1R = 860,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_VIEW_SQUARE_D1L = 4,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_VIEW_SQUARE_D1R = 5,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_VIEW_SQUARE_D0C = 0,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN = 0x0032,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN = 0x0041,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_CELL_ORDER_D0C_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_L2_PERSPECTIVE = 2,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_PC34_D1_VIEW_DEPTH = 1
};

typedef enum {
    DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34 = 0,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34 = 1
} DM1_V1_D1L2D1R2StairsPitSidePc34;

typedef enum {
    DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34 = 0,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 2,
    DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 = 3
} DM1_V1_D1L2D1R2StairsPitRoutePc34;

typedef struct {
    DM1_V1_D1L2D1R2StairsPitSidePc34 side;
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
    const char *lane_name;
    const char *draw_square_function;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    const char *dungeon_anchor;
    const char *f0115_tail_anchor;
    const char *f0128_followup_anchor;
    int l2_perspective_index;
    int redmcsb_view_depth;
    int relative_forward_step;
    int relative_lateral_step;
    int f0128_dispatch_order;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int first_stairs_graphic_index;
    int zone_index;
    int view_square_index;
    int cell_order;
    int transparent_color;
    bool uses_f0104_native;
    bool uses_f0105_flipped;
    bool uses_f0115_thing_pass_followup;
    bool follows_with_d1c_d0l_d0r_d0c;
    int followup_d0c_view_square_index;
    int followup_d0c_cell_order;
    bool followup_d0c_uses_f0115;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34;

typedef struct {
    DM1_V1_D1L2D1R2StairsPitSidePc34 side;
    int direction;
    int map_x;
    int map_y;
    int element_class;
    bool stairs_up;
    bool pit_or_teleporter_visible;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1L2D1R2StairsPitDispatchContextPc34;

typedef struct {
    bool ok;
    bool unsupported_element;
    bool contract_only;
    bool real_asset_claim;
    DM1_V1_D1L2D1R2StairsPitSidePc34 side;
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
    int direction;
    int map_x;
    int map_y;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int zone_index;
    int view_square_index;
    int cell_order;
    int l2_perspective_index;
    int redmcsb_view_depth;
    bool used_f0104_native;
    bool used_f0105_flipped;
    bool used_f0115_thing_pass_followup;
    bool follows_with_d1c_d0l_d0r_d0c;
    int followup_d0c_view_square_index;
    int followup_d0c_cell_order;
    bool followup_d0c_uses_f0115;
    const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *spec;
} DM1_V1_D1L2D1R2StairsPitDispatchResultPc34;

typedef struct {
    DM1_V1_D1L2D1R2StairsPitSidePc34 side;
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool used_f0104_native;
    bool used_f0105_flipped;
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
    const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *spec;
} DM1_V1_D1L2D1R2StairsPitPixelRunResultPc34;

void dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_init_context_pc34(
    DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D1L2D1R2StairsPitSidePc34 side);

const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_pc34(size_t *count);

const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
    DM1_V1_D1L2D1R2StairsPitSidePc34 side,
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route);

bool dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
    const DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D1L2D1R2StairsPitDispatchResultPc34 *out);

bool dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34 *input,
    DM1_V1_D1L2D1R2StairsPitPixelRunResultPc34 *out);

const char *dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_source_evidence_pc34(void);

const char *const *dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_anchor_table_pc34(
    size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
