#include "dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.h"

enum {
    DM1_V1_D0C_F0111_PRESENT = 1,
    DM1_V1_D0C_F0111_ABSENT = 0
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; contract_only=1, no real-asset "
    "bitmap parity, and no game-data load. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor is the door-panel state machine. "
    "DUNVIEW.C:4248 C0_DOOR_STATE_OPEN guard returns without drawing. "
    "DUNVIEW.C:4260 copies the native panel into G0074_puc_Bitmap_Temporary "
    "and DUNVIEW.C:4262 draws the base ornament with F0109. "
    "DUNVIEW.C:4273-4285 resolve the MASK0x0001_FLIP_HORIZONTAL and "
    "MASK0x0002_FLIP_VERTICAL animated flip flags. DUNVIEW.C:4292-4294 "
    "apply the C16_DOOR_ORNAMENT_THIEVES_EYE_MASK on the M631_ZONE_DOOR_D1C "
    "path; the D0C frame uses G0172_auc_Graphic558_Frame_DoorFrame_D0C at "
    "DUNVIEW.C:8185-8216. DUNVIEW.C:4297-4298 draw the C4 closed frame and "
    "DUNVIEW.C:4301-4304 apply the C15_DOOR_ORNAMENT_DESTROYED_MASK on the "
    "C5 destroyed branch. DUNVIEW.C:4317-4325 shift the PC34 zone with "
    "C6_UNKNOWN and 3|MASK0x4000 on the horizontal partly-open path. "
    "DUNVIEW.C:4334 final F0791 blit writes with C10_COLOR_FLESH "
    "transparency. DUNVIEW.C:8164-8363 F0127_DUNGEONVIEW_DrawSquareD0C is "
    "the D0C dispatch: DUNVIEW.C:8185-8216 C16_ELEMENT_DOOR_SIDE for the "
    "G0172 door frame, DUNVIEW.C:8241-8273 C19_ELEMENT_STAIRS_FRONT, "
    "DUNVIEW.C:8274-8294 C02_ELEMENT_PIT, DUNVIEW.C:8294 F0112 ceiling "
    "pit, DUNVIEW.C:8294 F0115 thing pass with C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT, and "
    "DUNVIEW.C:8302-8308 C05_ELEMENT_TELEPORTER F0113 field blit using "
    "G0163_aauc_Graphic558_Frame_Walls[M609_VIEW_SQUARE_D0C]. DUNVIEW.C:594 "
    "anchors the D0C G0163 row as { 0, 223, 0, 135, 0, 0, 0, 0 } which is "
    "the closest in-your-face full-viewport no-blit sentinel. DUNVIEW.C:597 "
    "anchors G0172 door frame D0C as { 96, 127, 0, 122, 16, 123, 0, 0 }. "
    "DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2088 C10_COLOR_FLESH; "
    "DEFS.H:2466 C15_DOOR_ORNAMENT_DESTROYED_MASK; DEFS.H:3508 C6_UNKNOWN; "
    "DEFS.H:3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR; "
    "DEFS.H:4036 C713_ZONE_WALL_D0C; DEFS.H:4055 C715_ZONE_WALL_D0C; "
    "DEFS.H:4067 C724_ZONE_DOOR_FRAME_D0C; DEFS.H:4086 C728_ZONE_DOOR_FRAME_D0C. "
    "Non-overlap: this gate is the D0C F0111 door-panel source-lock; it is "
    "distinct from the F0108/F0111 stairs-pit dispatch lock and the D1C F0111 "
    "door-panel state-machine lock.";

static const DM1_V1_D0CF0111DoorPanelPc34Contract s_contract = {
    /* booleans */
    DM1_V1_D0C_F0111_PRESENT,            /* source_locked_contract_only */
    DM1_V1_D0C_F0111_PRESENT,            /* no_real_asset_bitmap_parity */
    DM1_V1_D0C_F0111_PRESENT,            /* no_game_data_load */
    DM1_V1_D0C_F0111_PRESENT,            /* f0127_dispatches_d0c_door_side_c16 */
    DM1_V1_D0C_F0111_PRESENT,            /* f0111_line_range_anchor_present */
    DM1_V1_D0C_F0111_PRESENT,            /* f0127_line_range_anchor_present */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_g0163_full_viewport_no_blit */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_g0172_door_frame_anchor_present */
    DM1_V1_D0C_F0111_PRESENT,            /* c10_color_flesh_transparent_blit */
    DM1_V1_D0C_F0111_PRESENT,            /* c15_destroyed_mask_path */
    DM1_V1_D0C_F0111_PRESENT,            /* c6_unknown_partial_zone_shift */
    DM1_V1_D0C_F0111_PRESENT,            /* mask0x4000_unreadable_inscription_shift */
    DM1_V1_D0C_F0111_PRESENT,            /* open_state_skips_f0111 */
    DM1_V1_D0C_F0111_PRESENT,            /* closed_state_uses_d0c_zone */
    DM1_V1_D0C_F0111_PRESENT,            /* destroyed_state_returns_zero_pixel */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_f0111_does_not_call_f0100 */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_f0111_does_not_call_f0105 */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_f0111_does_not_call_f0107 */
    DM1_V1_D0C_F0111_PRESENT,            /* d0c_f0111_does_not_call_f0115 */
    /* integers */
    DM1_V1_D0C_F0111_DOOR_PANEL_LINE_START_PC34,
    DM1_V1_D0C_F0111_DOOR_PANEL_LINE_END_PC34,
    DM1_V1_D0C_F0127_DISPATCH_LINE_START_PC34,
    DM1_V1_D0C_F0127_DISPATCH_LINE_END_PC34,
    DM1_V1_D0C_F0111_VIEW_SQUARE_D0C_PC34,
    DM1_V1_D0C_F0111_VIEW_DEPTH_D0_PC34,
    DM1_V1_D0C_F0111_VIEW_LANE_CENTER_PC34,
    DM1_V1_D0C_G0163_LEFT_X_PC34,
    DM1_V1_D0C_G0163_RIGHT_X_PC34,
    DM1_V1_D0C_G0163_TOP_Y_PC34,
    DM1_V1_D0C_G0163_BOTTOM_Y_PC34,
    DM1_V1_D0C_G0163_BYTE_WIDTH_PC34,
    DM1_V1_D0C_G0163_HEIGHT_PC34,
    DM1_V1_D0C_G0163_BLIT_X_PC34,
    DM1_V1_D0C_G0163_BLIT_Y_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_LEFT_X_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_RIGHT_X_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_TOP_Y_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_BOTTOM_Y_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_BYTE_WIDTH_PC34,
    DM1_V1_D0C_G0172_DOOR_FRAME_HEIGHT_PC34,
    DM1_V1_D0C_ZONE_WALL_MEDIA508_PC34,
    DM1_V1_D0C_ZONE_WALL_MEDIA720_PC34,
    DM1_V1_D0C_ZONE_DOOR_FRAME_MEDIA508_PC34,
    DM1_V1_D0C_ZONE_DOOR_FRAME_MEDIA720_PC34,
    DM1_V1_D0C_ZONE_FLOOR_PIT_MEDIA720_PC34,
    DM1_V1_D0C_ZONE_CEILING_PIT_MEDIA720_PC34,
    DM1_V1_D0C_F0111_DOOR_STATE_OPEN_PC34,
    DM1_V1_D0C_F0111_DOOR_STATE_CLOSED_PC34,
    DM1_V1_D0C_F0111_DOOR_STATE_DESTROYED_PC34,
    DM1_V1_D0C_F0111_C10_COLOR_FLESH_PC34,
    DM1_V1_D0C_F0111_C15_DESTROYED_MASK_PC34,
    DM1_V1_D0C_F0111_C6_UNKNOWN_PC34,
    DM1_V1_D0C_F0111_MASK0X4000_PC34,
    DM1_V1_D0C_F0111_VIEW_SQUARE_D0C_PC34,
    /* strings */
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
    "ReDMCSB DUNVIEW.C:8164-8363 F0127_DUNGEONVIEW_DrawSquareD0C",
    "ReDMCSB DUNVIEW.C:594 G0163_aauc_Graphic558_Frame_Walls[M609] "
    "= { 0, 223, 0, 135, 0, 0, 0, 0 }",
    "ReDMCSB DUNVIEW.C:597 G0172_auc_Graphic558_Frame_DoorFrame_D0C "
    "= { 96, 127, 0, 122, 16, 123, 0, 0 }",
    "ReDMCSB DEFS.H:1039-1044,2088,2466,3508,3516",
    "ReDMCSB DEFS.H:4036 C713_ZONE_WALL_D0C; DEFS.H:4055 C715_ZONE_WALL_D0C; DEFS.H:4067 C724_ZONE_DOOR_FRAME_D0C; DEFS.H:4086 C728_ZONE_DOOR_FRAME_D0C",
    s_source_evidence
};

const DM1_V1_D0CF0111DoorPanelPc34Contract *
dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract(void)
{
    return &s_contract;
}

const char *
dm1_v1_viewport_d0c_f0111_door_panel_pc34_source_evidence(void)
{
    return s_source_evidence;
}

uint8_t
dm1_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    /* ReDMCSB DUNVIEW.C:4334 F0791 C10_COLOR_FLESH transparent blit. */
    return source_pixel == transparent_color
        ? destination_pixel
        : source_pixel;
}
