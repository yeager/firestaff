#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 source-lock gate for the D1L2/D1R2 F0111
 * partly-open horizontal door dispatch. This does not load game data and
 * does not claim real-asset pixel parity.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0111 lines 4218-4337; partly-open horizontal path at
 *   4311-4313 (LeftHorizontal/RightHorizontal selection), 4317-4318
 *   (P2084_i_ZoneIndex + state), 4320-4324 (first half blit through
 *   zone + C6_UNKNOWN), and 4325-4334 (state + 3 | MASK0x4000 then C10).
 * - DUNVIEW.C F0122 lines 7391-7557 and F0123 lines 7559-7725 bind the
 *   side door-front bodies to M630_ZONE_DOOR_D1L/M632_ZONE_DOOR_D1R.
 * - DUNVIEW.C F0128 lines 8524-8542 dispatch D1L, D1R, D1C, D0L, D0R,
 *   then F0127; F0127 line 8294 anchors the final center object pass.
 * - DEFS.H lines 2088, 2605-2606, 4047-4048 are retained as requested
 *   existing D2 partly-open/zone anchors; D1-specific bindings are
 *   DEFS.H lines 2600-2601, 4053-4054, and 4258/4260.
 * - CSB-lineage Viewport.cpp lines 1903-1915 bind the CSB F1 door-facing
 *   dispatch through StdDrawDoor between rear/front room-object passes.
 */

#define CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34 0x4000

typedef enum {
    CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34 = 1,
    CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34 = 2
} CSB_V1_D1L2D1R2F0111PartlyOpenDoorSidePc34;

typedef enum {
    CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34 = 0,
    CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34 = 1,
    CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34 = 2,
    CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34 = 3,
    CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34 = -1
} CSB_V1_D1L2D1R2F0111DoorBranchPc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square;
    int f0122_f0123_function_number;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0128_d1c_followup_order;
    int f0128_d0l_followup_order;
    int f0128_d0r_followup_order;
    int f0127_followup_order;
    int f0127_object_pass_line;
    int door_zone_base;
    int door_frame_top_zone;
    int wall_zone;
    int open_state;
    int partly_open_state_one;
    int partly_open_state_two;
    int partly_open_state_three;
    int closed_state;
    int destroyed_state;
    int decrements_state_before_frame_select;
    int horizontal_door_selects_left_horizontal;
    int horizontal_door_selects_right_horizontal;
    const char *left_horizontal_frame_bitmap;
    const char *right_horizontal_frame_bitmap;
    int first_half_source_zone_offset;
    int first_half_dest_zone_offset;
    int first_half_uses_f0635_zone_clip;
    int first_half_uses_f0654_blit;
    int first_half_zone_shift_x_is_half_bitmap_width;
    int first_half_transparent_color;
    int second_half_zone_offset;
    int second_half_zone_mask;
    int second_half_uses_f0791_drawbitmapxx;
    int second_half_transparent_color;
    const char *f0111_anchor;
    const char *d1_body_anchor;
    const char *f0128_anchor;
    const char *f0127_anchor;
    const char *defs_anchor;
    const char *lineage_anchor;
} CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34;

typedef struct {
    int route_count;
    int dispatch_order_ok;
    int branch_state_ok;
    int frame_selection_ok;
    int first_half_zone;
    int second_half_zone;
    int f0128_followup_ok;
    int copied_pixels;
    int c10_skipped_pixels;
    int no_real_asset_pixel_parity;
} CSB_V1_D1L2D1R2F0111PartlyOpenDoorProbePc34;

size_t csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34(void);

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(size_t index);

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(int side);

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square);

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

const char *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half);

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    int *out_c10_skipped);

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D1L2D1R2F0111PartlyOpenDoorProbePc34 *out_probe);

const char *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
