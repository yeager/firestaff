/*
 * ReDMCSB evidence (F0111 / D0C):
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor: open-door guard,
 *   G0074 temporary door copy, ornament/mask composition, partly-open zone
 *   math, Thieves-Eye mask, and final F0791 C10_COLOR_FLESH transparent blit.
 * - DUNVIEW.C:8164-8363 F0127_DUNGEONVIEW_DrawSquareD0C: D0C dispatch.
 *   DUNVIEW.C:8185-8216 handle C16_ELEMENT_DOOR_SIDE with the
 *   G0172_auc_Graphic558_Frame_DoorFrame_D0C and Thieves-Eye hole-in-frame
 *   blit.  The C17 door-front, C19 stairs-front, C02 pit, and C05
 *   teleporter branches are reached from F0127, but the F0111 call is
 *   shared with the F0124 D1C door-front path.
 * - DUNVIEW.C:8171 G0172_auc_Graphic558_Frame_DoorFrame_D0C
 *   (96, 127, 0, 122, 16, 123, 0, 0) anchors the D0C door frame box.
 * - DUNVIEW.C:594 G0163_aauc_Graphic558_Frame_Walls D0C row is
 *   (0, 223, 0, 135, 0, 0, 0, 0): D0C is the closest in-your-face wall and
 *   the wall set is the full viewport, with a no-blit sentinel for
 *   G0163 byte_width=0 / height=0.
 * - DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2088 C10_COLOR_FLESH;
 *   DEFS.H:2466 C15_DOOR_ORNAMENT_DESTROYED_MASK; DEFS.H:3508 C6_UNKNOWN;
 *   DEFS.H:3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR;
 *   DEFS.H:4036 C713_ZONE_WALL_D0C; DEFS.H:4055 C715_ZONE_WALL_D0C;
 *   DEFS.H:4067 C724_ZONE_DOOR_FRAME_D0C; DEFS.H:4086 C728_ZONE_DOOR_FRAME_D0C.
 * - DM1-PC34 form: M609_VIEW_SQUARE_D0C index, C728 zone for door frame
 *   (PC34 PC 3.4), C724 for MEDIA508.
 *
 * Non-overlap: this gate is the D0C F0111 door-panel source-lock and is
 * distinct from:
 *   - test_dm1_v1_viewport_f0111_door_panel_pc34_compat.c (which exercises
 *     the F0111 door-panel state machine and C10 blit on the
 *     D1C_ZONE_DOOR_D1C=3790 path);
 *   - test_dm1_v1_ceiling_pit_f0108_f0111_dispatch_source_lock_pc34_compat.c
 *     (which locks the F0108 floor-ornament and F0111 dispatch in the
 *     F0119/F0120/F0124 D2L/D2R/D1C stairs-and-pit routes);
 *   - test_csb_v1_viewport_d1c_f0111_door_pc34_compat.c and the other
 *     csb_v1_viewport_*_f0111_door_pc34_compat tests (which cover the
 *     CSB-lineage center-door dispatch for D1C/D2C/D0L/D0R/D1L/D1R/
 *     D1L2/D1R2/D2L2/D2R2/D3L2/D3R2).
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* F0111 line range anchored at DUNVIEW.C:4218-4337. */
#define DM1_V1_D0C_F0111_DOOR_PANEL_LINE_START_PC34 4218
#define DM1_V1_D0C_F0111_DOOR_PANEL_LINE_END_PC34   4337

/* F0127 D0C dispatch line range anchored at DUNVIEW.C:8164-8363. */
#define DM1_V1_D0C_F0127_DISPATCH_LINE_START_PC34   8164
#define DM1_V1_D0C_F0127_DISPATCH_LINE_END_PC34     8363

/* D0C G0163 frame row (DUNVIEW.C:594):
 *   { 0, 223, 0, 135, 0, 0, 0, 0 }  (full viewport, no-blit sentinel). */
#define DM1_V1_D0C_G0163_LEFT_X_PC34                  0
#define DM1_V1_D0C_G0163_RIGHT_X_PC34               223
#define DM1_V1_D0C_G0163_TOP_Y_PC34                   0
#define DM1_V1_D0C_G0163_BOTTOM_Y_PC34              135
#define DM1_V1_D0C_G0163_BYTE_WIDTH_PC34              0
#define DM1_V1_D0C_G0163_HEIGHT_PC34                  0
#define DM1_V1_D0C_G0163_BLIT_X_PC34                  0
#define DM1_V1_D0C_G0163_BLIT_Y_PC34                  0

/* G0172 D0C door frame (DUNVIEW.C:597). */
#define DM1_V1_D0C_G0172_DOOR_FRAME_LEFT_X_PC34      96
#define DM1_V1_D0C_G0172_DOOR_FRAME_RIGHT_X_PC34    127
#define DM1_V1_D0C_G0172_DOOR_FRAME_TOP_Y_PC34        0
#define DM1_V1_D0C_G0172_DOOR_FRAME_BOTTOM_Y_PC34   122
#define DM1_V1_D0C_G0172_DOOR_FRAME_BYTE_WIDTH_PC34  16
#define DM1_V1_D0C_G0172_DOOR_FRAME_HEIGHT_PC34     123

/* D0C zones from DEFS.H. */
#define DM1_V1_D0C_ZONE_WALL_MEDIA508_PC34          713
#define DM1_V1_D0C_ZONE_WALL_MEDIA720_PC34          715
#define DM1_V1_D0C_ZONE_DOOR_FRAME_MEDIA508_PC34    724
#define DM1_V1_D0C_ZONE_DOOR_FRAME_MEDIA720_PC34    728
#define DM1_V1_D0C_ZONE_FLOOR_PIT_MEDIA720_PC34     862
#define DM1_V1_D0C_ZONE_CEILING_PIT_MEDIA720_PC34   871

/* F0111 door state machine constants. */
#define DM1_V1_D0C_F0111_C10_COLOR_FLESH_PC34        10
#define DM1_V1_D0C_F0111_C15_DESTROYED_MASK_PC34     15
#define DM1_V1_D0C_F0111_C6_UNKNOWN_PC34              6
#define DM1_V1_D0C_F0111_MASK0X4000_PC34          0x4000
#define DM1_V1_D0C_F0111_DOOR_STATE_OPEN_PC34        0
#define DM1_V1_D0C_F0111_DOOR_STATE_CLOSED_PC34      4
#define DM1_V1_D0C_F0111_DOOR_STATE_DESTROYED_PC34   5
#define DM1_V1_D0C_F0111_VIEW_SQUARE_D0C_PC34        9  /* M609 */
#define DM1_V1_D0C_F0111_VIEW_DEPTH_D0_PC34          0
#define DM1_V1_D0C_F0111_VIEW_LANE_CENTER_PC34       0

typedef struct {
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool f0127_dispatches_d0c_door_side_c16;
    bool f0111_line_range_anchor_present;
    bool f0127_line_range_anchor_present;
    bool d0c_g0163_frame_row_full_viewport_no_blit;
    bool d0c_g0172_door_frame_anchor_present;
    bool c10_color_flesh_transparent_blit;
    bool c15_destroyed_mask_path;
    bool c6_unknown_partial_zone_shift;
    bool mask0x4000_unreadable_inscription_shift;
    bool open_state_skips_f0111;
    bool closed_state_uses_d0c_zone;
    bool destroyed_state_returns_zero_pixel;
    bool d0c_f0111_does_not_call_f0100;
    bool d0c_f0111_does_not_call_f0105;
    bool d0c_f0111_does_not_call_f0107;
    bool d0c_f0111_does_not_call_f0115;
    int f0111_line_start;
    int f0111_line_end;
    int f0127_line_start;
    int f0127_line_end;
    int d0c_view_square;
    int d0c_view_depth;
    int d0c_view_lane;
    int d0c_g0163_left_x;
    int d0c_g0163_right_x;
    int d0c_g0163_top_y;
    int d0c_g0163_bottom_y;
    int d0c_g0163_byte_width;
    int d0c_g0163_height;
    int d0c_g0163_blit_x;
    int d0c_g0163_blit_y;
    int d0c_g0172_door_frame_left_x;
    int d0c_g0172_door_frame_right_x;
    int d0c_g0172_door_frame_top_y;
    int d0c_g0172_door_frame_bottom_y;
    int d0c_g0172_door_frame_byte_width;
    int d0c_g0172_door_frame_height;
    int d0c_zone_wall_media508;
    int d0c_zone_wall_media720;
    int d0c_zone_door_frame_media508;
    int d0c_zone_door_frame_media720;
    int d0c_zone_floor_pit_media720;
    int d0c_zone_ceiling_pit_media720;
    int open_state;
    int closed_state;
    int destroyed_state;
    int c10_color_flesh;
    int c15_destroyed_mask;
    int c6_unknown;
    int mask0x4000_shift;
    /* M609 view-square-index for the D0C F0127 dispatch. */
    int view_square_d0c;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_d0c_g0163_anchor;
    const char *redmcsb_d0c_g0172_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_d0c_zones_anchor;
    const char *d0c_f0111_door_panel_source_evidence;
} DM1_V1_D0CF0111DoorPanelPc34Contract;

const DM1_V1_D0CF0111DoorPanelPc34Contract *
dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract(void);

const char *
dm1_v1_viewport_d0c_f0111_door_panel_pc34_source_evidence(void);

uint8_t
dm1_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H */
