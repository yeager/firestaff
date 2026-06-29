#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_DOOR_EDGE_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_DOOR_EDGE_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C door-edge-ornament source-lock probe.
 *
 * ReDMCSB anchors confirmed from the local checkout:
 * - DUNVIEW.C:8164-8311 F0127_DUNGEONVIEW_DrawSquareD0C is the D0C
 *   body. Its C16_ELEMENT_DOOR_SIDE branch at 8185-8236 is the
 *   door-edge-ornament draw path that is distinct from the F0111
 *   door-panel state machine at 4218-4337: this branch draws the
 *   door-frame border/edge using G0172_auc_Graphic558_Frame_DoorFrame_D0C
 *   and G2116_DoorFrameFrontD0C (or G0709_puc_Bitmap_WallSet_DoorFrameFront
 *   for legacy media), not the F0111 partly-open half-blit body. The
 *   door-edge-ornament is the wooden/metal frame border that wraps
 *   around the door opening in the D0C row; the door panel is the
 *   door itself. This gate pins the door-edge-ornament contract.
 *
 * - DUNVIEW.C:597 G0172_auc_Graphic558_Frame_DoorFrame_D0C is the
 *   door-frame stride in the wallset graphic source: { 96, 127, 0, 122,
 *   16, 123, 0, 0 } (X1, X2, Y1, Y2, ByteWidth, Height, X, Y). It is
 *   the door-edge-ornament stride and must remain byte-stable.
 *
 * - DUNVIEW.C:151/226/242/259 and 2162/2181/2196 declare and load
 *   G2116_DoorFrameFrontD0C as the door-edge-ornament native bitmap
 *   for the modern PC 3.4 (I34E) and F20E F20J X30J P20JA P20JB targets.
 *   The PC 3.4 Atari/Amiga targets use G0709_puc_Bitmap_WallSet_DoorFrameFront
 *   for the same role.
 *
 * - DUNVIEW.C:8197 (no-thieves-eye) and 8221 (no-thieves-eye too) draw
 *   the door-edge-ornament via F0100_DUNGEONVIEW_DrawWallSetBitmap at
 *   the G0172 stride. DUNVIEW.C:8216 (no-thieves-eye modern I34E)
 *   draws it via F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap with
 *   the C728_ZONE_DOOR_FRAME_D0C zone. DUNVIEW.C:8213 (F20E) uses
 *   C724_ZONE_DOOR_FRAME_D0C.
 *
 * - DUNVIEW.C:8185-8198 (thieves-eye path) copies G0709 into
 *   G0074_puc_Bitmap_Temporary at M075_BITMAP_BYTE_COUNT(32, 123) and
 *   then blits C041_GRAPHIC_HOLE_IN_WALL over it via
 *   G0108_auc_Graphic558_Box_ThievesEye_HoleInDoorFrame at the
 *   G0172[C0_X1] - M768_BOX_LEFT(G0106_visible_area) offset. Modern
 *   I34E targets use M711_NEGGRAPHIC_HOLE_IN_WALL with the
 *   C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME zone and
 *   C09_COLOR_GOLD transparency; F20E P20J targets use
 *   C732_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME.
 *
 * - DUNVIEW.C:8192 uses C048_BYTE_WIDTH / C016_BYTE_WIDTH for the
 *   legacy hole blit; DUNVIEW.C:8195 uses 95/123 height bytes for
 *   the Atari/Amiga clipped blit.
 *
 * - DUNVIEW.C:8294-8296 (post-frame F0112 ceiling-pit) is a no-op
 *   for the door-edge-ornament dispatch when the cell is
 *   C16_ELEMENT_DOOR_SIDE because the ceiling-pit code is reached
 *   only after the switch breaks for the door-side case; the
 *   F0112/C069/C871 ceiling-pit and F0115/M609/C0x0021_*
 *   post-frame contracts are stable for non-door-side D0C cells
 *   and are pinned here as the post-frame byte-stability surface.
 *
 * - DEFS.H:1039-1044 C0..C5 door states; 2088 C10_COLOR_FLESH;
 *   2090 C09_COLOR_GOLD; 4036 C713_ZONE_WALL_D0C (legacy);
 *   4055 C715_ZONE_WALL_D0C (modern I34E); 4067 C724_ZONE_DOOR_FRAME_D0C
 *   (F20E P20J); 4086 C728_ZONE_DOOR_FRAME_D0C (modern I34E);
 *   4074 C732_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME (F20E P20J);
 *   4095 C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME (modern I34E);
 *   2456 C00_THING_TYPE_DOOR; 2466 C15_DOOR_ORNAMENT_DESTROYED_MASK.
 *
 * Non-overlap marker: pass792-d0c-door-edge-ornament-source-lock.
 * This gate is intentionally distinct from the existing D0C F0111
 * door-panel and D0C F0111 partly-open-door source-locks: those
 * gate the F0111 state machine and the partly-open half-blit body,
 * while this gate pins the door-edge-ornament (door-frame border
 * + thieves-eye hole + post-frame F0112/F0115/F0113 stability)
 * contract that lives inside the F0127 C16_ELEMENT_DOOR_SIDE branch
 * but is never reached by F0111. The D0C F0108 floor+ceiling
 * ornament, D0C F0108 floor-ornament, D0C ceiling-pit, D0C
 * stairs-pit-dispatch, and the F0115 thing-pass source-locks for
 * other rows are also distinct from this contract.
 *
 * Synthetic 320x200 framebuffer with a 224x136 viewport; no real-asset
 * bitmap parity, no original-DOS pixel claim.
 */

#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C09_COLOR_GOLD_PC34 9
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C00_THING_TYPE_DOOR_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C15_DESTROYED_MASK_PC34 15
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C0_X1_INDEX_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C2_HEIGHT_INDEX_PC34 2
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C4_BYTE_WIDTH_INDEX_PC34 4
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C5_HEIGHT_INDEX_PC34 5
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C6_X_INDEX_PC34 6
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C7_Y_INDEX_PC34 7
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_STRIDE_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_LEFT_X_PC34 96
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_RIGHT_X_PC34 127
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_TOP_Y_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BOTTOM_Y_PC34 122
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BYTE_WIDTH_PC34 16
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_HEIGHT_PC34 123
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G2116_D0C_LOADER_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M075_BYTE_COUNT_PC34 32
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M075_HEIGHT_PC34 123
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TEMPORARY_COPY_BYTE_WIDTH_PC34 32
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TEMPORARY_COPY_HEIGHT_PC34 123
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C048_BYTE_WIDTH_PC34 48
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C016_BYTE_WIDTH_PC34 16
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HEIGHT_PC34 95
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_BOX_PC34 95
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HOLES_PC34 123
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_START_PC34 8164
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_END_PC34 8311
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_START_PC34 8185
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_END_PC34 8236
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0128_DOOR_SIDE_D0C_PC34 8542
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_SQUARE_D0C_PC34 9
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_DEPTH_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_LANE_PC34 0
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_CELL_ORDER_D0C_PC34 0x0021
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_CELL_ORDER_BACKLEFT_PC34 0x21
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_CELL_ORDER_BACKRIGHT_PC34 0x00
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G2116_D0C_LINE_PC34 2181
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C041_GRAPHIC_HOLE_PC34 41
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34 -711
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34 724
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C728_ZONE_DOOR_FRAME_PC34 728
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34 732
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C736_ZONE_THIEVES_EYE_PC34 736
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C713_ZONE_WALL_D0C_PC34 713
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C715_ZONE_WALL_D0C_PC34 715
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C068_CEILING_PIT_D0C_PC34 68
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C069_CEILING_PIT_D0C_PC34 69
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C869_ZONE_CEILING_PIT_PC34 869
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C871_ZONE_CEILING_PIT_PC34 871
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C16_DOOR_SIDE_PC34 16
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C19_STAIRS_FRONT_PC34 19
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C02_PIT_PC34 2
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C05_TELEPORTER_PC34 5
#define DM1_V1_D0C_DOOR_EDGE_ORNAMENT_HASH_PC34 0xfbe15fb3u

typedef enum {
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_LEGACY_PC34 = 0,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_F20E_PC34 = 1,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_I34E_PC34 = 2,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_LEGACY_PC34 = 3,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_F20E_PC34 = 4,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_I34E_PC34 = 5,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_BRANCH_INVALID_PC34 = -1
} DM1_V1_D0CDoorEdgeOrnamentBranchPc34;

typedef struct {
    int branch;
    int has_thieves_eye;
    int target_media;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int g0172_left_x;
    int g0172_right_x;
    int g0172_top_y;
    int g0172_bottom_y;
    int g0172_byte_width;
    int g0172_height;
    int g0172_blit_x;
    int g0172_blit_y;
    int g2116_door_frame_zone;
    int thieves_eye_zone;
    int thieves_eye_hole_native_bitmap;
    int thieves_eye_color;
    int frame_transparency_color;
    int f0127_line_start;
    int f0127_line_end;
    int f0127_door_side_branch_start;
    int f0127_door_side_branch_end;
    int f0128_d0c_call_line;
    int f0128_d0c_view_square;
    int f0128_d0c_view_depth;
    int f0128_d0c_view_lane;
    int f0128_d0c_cell_order;
    int f0127_dispatches_d0c_door_side;
    int g0172_strides_are_16x123;
    int g2116_used_for_modern_i34e;
    int g0709_used_for_legacy;
    int post_frame_f0112_ceiling_pit_present;
    int post_frame_f0115_thing_pass_present;
    int post_frame_f0113_field_blit_present;
    int c10_transparent_blit;
    int c09_gold_hole_blit;
    int thieves_eye_hole_blit_present;
    int no_thieves_eye_draws_frame_direct;
    int legacy_thieves_eye_copies_g0709_to_temporary;
    int legacy_thieves_eye_draws_temporary_via_f0100;
    int modern_thieves_eye_copies_g2116_to_temporary;
    int modern_thieves_eye_draws_temporary_via_f0656;
    int temporary_bitmap_byte_width;
    int temporary_bitmap_height;
    int half_clip_first_byte_width;
    int half_clip_second_byte_width;
    int half_clip_first_height;
    int half_clip_second_height;
    int frame_buffer_strip_origin;
    int frame_buffer_strip_byte_width;
    int frame_buffer_strip_destination_x;
    int frame_buffer_strip_destination_y;
    uint8_t first_probe_pixel;
    uint8_t second_probe_pixel;
    uint8_t third_probe_pixel;
} DM1_V1_D0CDoorEdgeOrnamentTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int no_thieves_eye_legacy_branch;
    int no_thieves_eye_f20e_branch;
    int no_thieves_eye_i34e_branch;
    int thieves_eye_legacy_branch;
    int thieves_eye_f20e_branch;
    int thieves_eye_i34e_branch;
    int invalid_branch;
    int g0172_stride_checks;
    int g2116_zone_checks;
    int thieves_eye_zone_checks;
    int transparency_color_checks;
    int thieves_eye_branch_gating_checks;
    int post_frame_f0112_checks;
    int post_frame_f0115_checks;
    int post_frame_f0113_checks;
    int non_overlap_checks;
    int bitmap_strip_byte_width_checks;
} DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34;

int dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(
    int has_thieves_eye,
    int target_media,
    DM1_V1_D0CDoorEdgeOrnamentTracePc34 *out_trace);

const char *
dm1_v1_viewport_d0c_door_edge_ornament_source_evidence_pc34(void);

int run_dm1_v1_viewport_d0c_door_edge_ornament_self_test(void);

const DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 *
dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
