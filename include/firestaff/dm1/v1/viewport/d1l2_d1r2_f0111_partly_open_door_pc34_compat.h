/*
 * DM1 V1 PC 3.4 contract-only source-lock gate for the F0111 partly-open
 * corridor-side D1L2/D1R2 lateral door fronts (door-on-side / depth-1
 * side cells of the D1 row, not the D1L/D1R primary view squares).
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; line 4248 skips
 *   fully-open doors, line 4308 decrements partly-open states, lines
 *   4312-4313 select LeftHorizontal/RightHorizontal frame bitmaps,
 *   lines 4317-4324 apply the state-adjusted P2084_i_ZoneIndex plus
 *   C6_UNKNOWN and F0654 C10_COLOR_FLESH first-half blit, and lines
 *   4325-4334 add 3 | MASK0x4000 before F0791_DUNGEONVIEW_DrawBitmapXX
 *   with C10.
 * - DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L and 7559-7725
 *   F0123_DUNGEONVIEW_DrawSquareD1R are the D1L/D1R body callers; the
 *   D1L2/D1R2 lateral door fronts ride these bodies when the
 *   L2464_ai_SquareAspect[M556_DOOR_STATE] is in 1..3.
 * - DUNVIEW.C:8524-8542 F0128_DUNGEONVIEW_Draw_CPSF dispatches D1L
 *   then D1R; D1C, D0L, D0R, and D0C follow; D0C is at 8549 with the
 *   F0127 object-pass boundary at 8294.
 * - DEFS.H:1039-1043 C0..C4 door states; 2088 C10_COLOR_FLESH;
 *   2599-2601 M606/M607/M608 view squares; 2600-2601 M607_D1L and
 *   M608_D1R are the *primary* D1L/D1R view squares (NOT the lateral
 *   cells); 2670-2676 C0x0218..C0x0439 DoorPass anchors; 2791
 *   C2_VIEW_DOOR_ORNAMENT_D1LCR; 3508 C6_UNKNOWN; 3516 MASK0x4000;
 *   4053-4054 C713_ZONE_WALL_D1L / C714_ZONE_WALL_D1R; 4258/4260
 *   M630_ZONE_DOOR_D1L / M632_ZONE_DOOR_D1R; 5458 G0695 extern;
 *   5543/5545 G0186/G0188 externs.
 * - CSB counterpart: csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat
 *   is the CSB V1 version of this same lateral door pair; this DM1 V1
 *   file is the non-overlap DM1 source-lock sibling.
 *
 * This gate is synthetic and asset-free. It makes no real-asset or
 * original-DOS pixel-parity claim. The DM1 V1 D1C F0111 partly-open
 * sibling is test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat,
 * and the DM1 V1 D2L/D2R F0111 partly-open sibling is
 * test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34 6
#define DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_MASK0X4000_PC34 0x4000

typedef enum {
    DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34 = 2
} DM1_V1_D1L2D1R2F0111PartlyOpenDoorSidePc34;

typedef enum {
    DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34 = 0,
    DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34 = 1,
    DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34 = 2,
    DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34 = 3,
    DM1_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34 = -1
} DM1_V1_D1L2D1R2F0111DoorBranchPc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square_m606;
    int view_square_m607;
    int view_square_m608;
    int f0122_f0123_function_number;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0128_d1c_followup_order;
    int f0128_d0l_followup_order;
    int f0128_d0r_followup_order;
    int f0127_followup_order;
    int f0127_object_pass_line;
    int door_zone_m630_d1l;
    int door_zone_m632_d1r;
    int door_frame_top_zone_d1l;
    int door_frame_top_zone_d1r;
    int wall_zone_c713_d1l;
    int wall_zone_c714_d1r;
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
    int first_half_dest_zone_offset;
    int second_half_zone_offset;
    int second_half_zone_mask;
    int first_half_transparent_color;
    int second_half_transparent_color;
    const char *f0111_anchor;
    const char *d1_body_anchor;
    const char *f0128_anchor;
    const char *f0127_anchor;
    const char *defs_anchor;
} DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d1l2_partly_one;
    int d1l2_partly_two;
    int d1l2_partly_three;
    int d1r2_partly_one;
    int d1r2_partly_two;
    int d1r2_partly_three;
    int d1l2_partly;
    int d1r2_partly;
    int closed_rejections;
    int open_rejections;
    int unknown_rejections;
    int c10_first_half_skips;
    int c10_second_half_skips;
    int f0128_followup_anchors;
} DM1_V1_D1L2D1R2F0111PartlyOpenDoorSelfTestResultPc34;

size_t dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34(void);

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(size_t index);

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(int side);

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square);

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

const char *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half);

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
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

int dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_trace_pc34(
    int side,
    int door_state,
    DM1_V1_D1L2D1R2F0111DoorBranchPc34 *out_branch,
    int *out_decremented_state,
    int *out_first_half_zone,
    int *out_second_half_zone,
    int *out_first_half_c10_skips,
    int *out_second_half_c10_skips,
    int *out_first_half_writes,
    int *out_second_half_writes,
    uint8_t *out_probe_a,
    uint8_t *out_probe_b);

const char *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34(void);

int run_dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_self_test(void);

const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
