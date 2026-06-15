#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 viewport source-lock gate for the D2L2/D2R2
 * F0111 partly-open door-frame route.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor open/closed/destroyed
 *   and partly-open branches. The partly-open horizontal path decrements the
 *   door state, uses LeftHorizontal/RightHorizontal frame bitmaps at
 *   4311-4313, adds P2084_i_ZoneIndex + state at 4317-4318, performs the
 *   first half blit through zone + C6_UNKNOWN at 4320-4324, then shifts the
 *   final half by 3 | MASK0x4000 at 4325 and blits with C10 at 4334.
 * - DUNVIEW.C:8503-8508 F0128_DUNGEONVIEW_Draw_CPSF dispatches D2L2 at
 *   relative depth 2/lane -2 through F0678 and D2R2 at depth 2/lane +2
 *   through F0679.
 * - DUNVIEW.C:6837-6896 F0678_DrawD2L2/F0679_DrawD2R2 bind D2L2/D2R2 wall
 *   and teleporter drawing to C707_ZONE_WALL_D2L2/C708_ZONE_WALL_D2R2, with
 *   wall cases returning before any F0111 door-front route.
 * - DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2605-2606 C09/C10 D2L2/D2R2 view
 *   squares; DEFS.H:3508 C6_UNKNOWN; DEFS.H:3516 MASK0x4000; DEFS.H:4047
 *   C707_ZONE_WALL_D2L2; DEFS.H:4048 C708_ZONE_WALL_D2R2; DEFS.H:4228-4230
 *   C2500_ZONE_/C2900_ZONE_ thing/projectile bases outside this F0111 gate.
 * - COORD.C:788-797 C3700 per-state door records and COORD.C:1556-1559 C03
 *   48x40 clip record 126 through parent 129 at x=24,y=28.
 * - CSB Viewport.cpp:1903-1906 CSB-lineage room-object overlay binding is
 *   lineage evidence only; this file does not implement CSB-lineage logic.
 *
 * Note: the requested C2600_DOOR_PARTLY_OPEN_BITMAP symbol does not exist in
 * the available ReDMCSB Common/Source files. This gate therefore cites the
 * actual F0111 partly-open bitmap-selection lines above and exposes a
 * source-anchor string documenting the missing literal symbol.
 */

#define CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH 10
#define CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000 0x4000

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lane;
    int wall_zone_binding;
    int f0678_f0679_direct_f0111_route_present;
    int wall_case_returns_before_f0111;
    int excludes_existing_front_clipped_gate;
    int excludes_existing_partly_open_gate;
    int excludes_existing_wall_gate;
    int excludes_c2500_object_base;
    int excludes_c2900_projectile_base;
    int door_zone_base;
    int open_state;
    int partly_open_state_one;
    int partly_open_state_two;
    int partly_open_state_three;
    int closed_state;
    int destroyed_state;
    int first_half_zone_offset;
    int first_half_source_mask_zone_offset;
    int final_half_zone_offset;
    int final_half_mask;
    int transparent_color;
    int viewport_width;
    int viewport_height;
    int framebuffer_width;
    int framebuffer_height;
    int clipped_width;
    int clipped_height;
    int coord_parent_record;
    int coord_clip_record;
    int coord_frame_x;
    int coord_frame_y;
    int c2600_literal_symbol_present;
    const char *route_name;
    const char *f0111_anchor;
    const char *f0128_anchor;
    const char *d2_lane_anchor;
    const char *defs_anchor;
    const char *coord_anchor;
    const char *lineage_anchor;
    const char *c2600_anchor;
} CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34;

typedef struct {
    int route_count;
    int assertions;
    int failures;
    int c10_skipped_pixels;
    int copied_pixels;
    int left_edge_writes;
    int right_edge_writes;
    int first_half_zone;
    int final_half_zone;
    int clipped_write_inside_224x136;
    int no_real_asset_pixel_parity;
} CSB_V1_ViewportD2L2D2R2F0111PartlyOpenDoorProbePc34;

size_t csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34(void);

const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_at_pc34(size_t index);

const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int dest_x,
    int dest_y,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int *out_c10_skipped,
    int *out_left_edge_writes,
    int *out_right_edge_writes);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_ViewportD2L2D2R2F0111PartlyOpenDoorProbePc34 *out_probe);

const char *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
