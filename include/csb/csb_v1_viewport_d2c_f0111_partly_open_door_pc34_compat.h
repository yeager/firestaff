#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 source-lock gate for the D2C F0111 partly-open
 * door-frame horizontal path. The D2C center-front square is a thin
 * single center-front square (M603_VIEW_SQUARE_D2C in PC34 form), and
 * the partly-open D2C F0111 dispatch is reached from F0121 (D2C body)
 * and F0128 (D2C post-dispatch). This gate is non-duplicative with the
 * existing CSB V1 F0111 partly-open coverage for D0L/D0R, D1C, D1L/D1R,
 * D1L2/D1R2, D2L2/D2R2, and D3L2/D3R2, and with the closed-state D2C
 * coverage in csb_v1_viewport_d2c_f0111_door_pc34_compat.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; partly-open horizontal
 *   path at 4311-4313 (LeftHorizontal/RightHorizontal selection), 4317-4318
 *   (P2084_i_ZoneIndex += state), 4320-4324 (first half blit through zone
 *   + C6_UNKNOWN), and 4325-4334 (state + 3 | MASK0x4000 then C10).
 * - DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C; the
 *   C17_ELEMENT_DOOR_FRONT branch at 7313-7341 dispatches F0111 for the
 *   D2C center door at zone M628_ZONE_DOOR_D2C with the D2C 64x61 native
 *   bitmap, after floor ornament, rear pass, and frame top/left/right.
 * - DUNVIEW.C:8508-8533 F0128_DUNGEONVIEW_Draw_CPSF; F0121 is called at
 *   line 8521 (D2L2/D2R2 at 8504/8508 when MEDIA720 is active, D2L/D2R at
 *   8513/8517, D2C at 8521, then D1L/D1R/D1C at 8525/8529/8533, and
 *   D0L/D0R/D0C follow). D0L/D0R-style F0100/F0105/F0107 fallback is
 *   NOT used for the D2C center.
 * - DUNVIEW.C:6837-6865 F0678_DrawD2L2 and 6868-6896 F0679_DrawD2R2 are
 *   D2L2/D2R2 wall anchors cited as the D2-side wall context, but the D2C
 *   center F0111 dispatch is reached from F0121 not from F0678/F0679.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2605-2606 C09/C10 D2L2/D2R2 squares; 3508
 *   C6_UNKNOWN; 3516 MASK0x4000; 4047-4048 C707/C708 D2L2/D2R2 wall zones
 *   (and 4029-4031 C707/C708/C709 D2C/D2L/D2R wall zones); 4256 M628
 *   D2C door zone (PC34 form).
 * - CSB-lineage Viewport.cpp:1903-1915 is the requested center-door
 *   dispatch anchor; local StdDrawF2DoorFacing at 1865-1879 binds F2.
 *
 * Note: the requested C2600_DOOR_PARTLY_OPEN_BITMAP symbol does not exist
 * in the available ReDMCSB Common/Source files. This gate cites the actual
 * F0111 partly-open bitmap-selection lines (4311-4313) and exposes a
 * source-anchor string documenting the missing literal symbol.
 */

#define CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34 0x4000

typedef enum {
    CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_OPEN_PC34 = 0,
    CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34 = 1,
    CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_CLOSED_PC34 = 2,
    CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_DESTROYED_PC34 = 3,
    CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34 = -1
} CSB_V1_D2CF0111PartlyOpenDoorBranchPc34;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square_d2c;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lane;
    int f0121_function_number;
    int wall_zone_d2c_binding;
    int wall_zone_d2l2_binding;
    int wall_zone_d2r2_binding;
    int f0678_f0679_d2l2_d2r2_direct_f0111_route_present;
    int wall_case_returns_before_f0111;
    int excludes_existing_front_clipped_gate;
    int excludes_existing_d2l2_d2r2_partly_open_gate;
    int excludes_existing_d2l2_d2r2_wall_gate;
    int excludes_existing_d1l2_d1r2_partly_open_gate;
    int excludes_existing_closed_d2c_gate;
    int excludes_d0l_d0r_f0100_f0105_f0107_fallback;
    int door_zone_d2c;
    int door_zone_d2l2;
    int door_zone_d2r2;
    int door_native_width;
    int door_native_height;
    int door_native_byte_count;
    int open_state;
    int partly_open_state_one;
    int partly_open_state_two;
    int partly_open_state_three;
    int closed_state;
    int destroyed_state;
    int decrements_state_before_frame_select;
    int first_half_zone_offset;
    int first_half_uses_f0635_zone_clip;
    int first_half_uses_f0654_blit;
    int first_half_zone_shift_x_is_half_bitmap_width;
    int first_half_transparent_color;
    int second_half_zone_offset;
    int second_half_zone_mask;
    int second_half_uses_f0791_drawbitmapxx;
    int second_half_transparent_color;
    int c2600_literal_symbol_present;
    int doorpass1_order;
    int doorpass2_order;
    const char *route_name;
    const char *left_horizontal_frame_bitmap;
    const char *right_horizontal_frame_bitmap;
    const char *f0111_anchor;
    const char *f0121_anchor;
    const char *f0128_anchor;
    const char *f0678_f0679_anchor;
    const char *defs_anchor;
    const char *lineage_anchor;
    const char *c2600_anchor;
} CSB_V1_D2CF0111PartlyOpenDoorSpecPc34;

typedef struct {
    int route_count;
    int assertions;
    int failures;
    int dispatch_order_ok;
    int branch_state_ok;
    int frame_selection_ok;
    int first_half_zone;
    int second_half_zone;
    int horizontal_mask_ok;
    int c10_transparency_ok;
    int copied_pixels;
    int c10_skipped_pixels;
    int no_real_asset_pixel_parity;
} CSB_V1_D2CF0111PartlyOpenDoorProbePc34;

size_t csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34(void);

const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2c_f0111_partly_open_door_spec_at_pc34(size_t index);

const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(int view_square);

int csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state);

int csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door);

const char *
csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half);

int csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *spec,
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

int csb_v1_viewport_d2c_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D2CF0111PartlyOpenDoorProbePc34 *out_probe);

const char *
csb_v1_viewport_d2c_f0111_partly_open_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
