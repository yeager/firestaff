#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0L2/D0R2 stairs-side and pit dispatch gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0125 lines 7960-8062: D0L C18 stairs-side, C02 pit,
 *   ceiling-pit/F0115 tail, and C00 wall-return separation.
 * - DUNVIEW.C F0126 lines 8064-8162: D0R C18 stairs-side, C02 pit,
 *   ceiling-pit/F0115 tail, and C00 wall-return separation.
 * - DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247: native
 *   and flipped C10-transparent bitmap blits.
 * - DUNVIEW.C F0128 lines 8534-8542: near D0L, D0R, then D0C ordering.
 * - DUNGEON.C F0172 lines 2466-2523: square-aspect source for C18/C02.
 * - DEFS.H lines 2088, 2443-2452, 2596-2598, 2658-2663, 4140-4170,
 *   and 4209-4221: C10, bitmap slots, view squares, cell orders, zones.
 */

typedef enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34 = 0,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34 = 1
} DM1_V1_D0L2D0R2StairsPitSidePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34 = 0,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 = 2
} DM1_V1_D0L2D0R2StairsPitRoutePc34;

enum {
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_TRANSPARENT_COLOR_PC34 = 10,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0L_PC34 = 1,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_VIEW_SQUARE_D0R_PC34 = 2,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0L_PC34 = 0x0002,
    DM1_V1_D0L2_D0R2_STAIRS_PIT_CELL_ORDER_D0R_PC34 = 0x0001
};

typedef struct {
    DM1_V1_D0L2D0R2StairsPitSidePc34 side;
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
    const char *route_name;
    const char *draw_square_function;
    const char *dispatch_anchor;
    const char *bitmap_anchor;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int zone_index;
    int ceiling_pit_graphic;
    int ceiling_pit_zone;
    int view_square_index;
    int cell_order;
    int f0128_dispatch_order;
    int redmcsb_depth;
    int redmcsb_lateral;
    int transparent_color;
    bool uses_f0104_native;
    bool uses_f0105_flipped;
    bool returns_before_ceiling_pit;
    bool uses_ceiling_pit_tail;
    bool uses_f0115_thing_pass_tail;
    bool excludes_wall_return;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34;

typedef struct {
    DM1_V1_D0L2D0R2StairsPitSidePc34 side;
    int element_class;
    bool pit_or_teleporter_visible;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D0L2D0R2StairsPitDispatchContextPc34;

typedef struct {
    bool ok;
    bool unsupported_element;
    bool contract_only;
    bool real_asset_claim;
    DM1_V1_D0L2D0R2StairsPitSidePc34 side;
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int zone_index;
    int ceiling_pit_graphic;
    int ceiling_pit_zone;
    int view_square_index;
    int cell_order;
    bool used_f0104_native;
    bool used_f0105_flipped;
    bool returned_before_ceiling_pit;
    bool used_ceiling_pit_tail;
    bool used_f0115_thing_pass_tail;
    const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *spec;
} DM1_V1_D0L2D0R2StairsPitDispatchResultPc34;

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
} DM1_V1_D0L2D0R2StairsPitPixelRunInputPc34;

typedef struct {
    bool ok;
    bool used_f0104_native;
    bool used_f0105_flipped;
    bool wrote_any;
    bool transparent_skip_seen;
    size_t writes;
    size_t transparent_skips;
    uint8_t first_destination_byte;
    uint8_t last_destination_byte;
    const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *spec;
} DM1_V1_D0L2D0R2StairsPitPixelRunResultPc34;

void dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_init_context_pc34(
    DM1_V1_D0L2D0R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D0L2D0R2StairsPitSidePc34 side);

const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_specs_pc34(size_t *count);

const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_for_pc34(
    DM1_V1_D0L2D0R2StairsPitSidePc34 side,
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route);

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_resolve_pc34(
    const DM1_V1_D0L2D0R2StairsPitDispatchContextPc34 *context,
    DM1_V1_D0L2D0R2StairsPitDispatchResultPc34 *out);

bool dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
    const DM1_V1_D0L2D0R2StairsPitPixelRunInputPc34 *input,
    DM1_V1_D0L2D0R2StairsPitPixelRunResultPc34 *out);

const char *dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
