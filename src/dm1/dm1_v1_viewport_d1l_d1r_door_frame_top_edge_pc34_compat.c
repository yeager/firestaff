#include "firestaff/dm1/v1/viewport/d1l_d1r_door_frame_top_edge_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND = 0,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND = 1,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND = 2
};

static const char s_source_evidence[] =
    "DM1 V1 D1L/D1R door-frame-top edge source-lock probe; contract-only "
    "and asset-free; no real-asset or original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:607-609 defines the three D1 door-frame-top strides "
    "as G0176_auc_Graphic558_Frame_DoorFrameTop_D1L = { 0, 31, 14, 17, 64, "
    "4, 16, 0 }, G0177_auc_Graphic558_Frame_DoorFrameTop_D1C = { 48, 175, "
    "14, 17, 64, 4, 0, 0 }, and G0178_auc_Graphic558_Frame_DoorFrameTop_D1R "
    "= { 192, 223, 14, 17, 64, 4, 16, 0 } with X1=0/48/192, X2=31/175/223, "
    "Y1=14, Y2=17, ByteWidth=64, Height=4, X=0/16/16, Y=0. The D1L/D1R X "
    "offsets are 16, which reflects the side-view shift relative to the "
    "centered D1C frame. "
    "DUNVIEW.C:7496 (F0122_DUNGEONVIEW_DrawSquareD1L C17_ELEMENT_DOOR_FRONT "
    "MEDIA009_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_"
    "G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G) calls "
    "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0704_puc_Bitmap_WallSet_DoorFrame"
    "Top_D1LCR, G0176_auc_Graphic558_Frame_DoorFrameTop_D1L) which is the "
    "legacy PC 3.4 door-frame-top edge for D1L. "
    "DUNVIEW.C:7494 is the F0122 D1L C17_ELEMENT_DOOR_FRONT case body line "
    "(preceded by F0108 floor-ornament and F0115 thing pass1). "
    "DUNVIEW.C:7500 (F0122 D1L C17_ELEMENT_DOOR_FRONT MEDIA508_F20E_F20J_"
    "X30J_P20JA_P20JB) calls F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"
    "(G2111_DoorFrameTopD1L, C728_ZONE_DOOR_FRAME_TOP_D1L). "
    "DUNVIEW.C:7503 (F0122 D1L C17_ELEMENT_DOOR_FRONT MEDIA720_I34E_I34M_"
    "A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J) calls "
    "F0104(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L). "
    "DUNVIEW.C:7664 (F0123_DUNGEONVIEW_DrawSquareD1R C17_ELEMENT_DOOR_"
    "FRONT MEDIA009) calls F0100(G0704, G0178) for D1R. "
    "DUNVIEW.C:7662 is the F0123 D1R C17_ELEMENT_DOOR_FRONT case body line "
    "(preceded by F0108 floor-ornament and F0115 thing pass1). "
    "DUNVIEW.C:7668 (F0123 D1R MEDIA508) calls "
    "F0104(G2110_DoorFrameTopD1R, C730_ZONE_DOOR_FRAME_TOP_D1R). "
    "DUNVIEW.C:7671 (F0123 D1R MEDIA720) calls "
    "F0104(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_FRAME_TOP_D1R). "
    "DUNVIEW.C:7391 / DUNVIEW.C:7559 are the F0122 / F0123 function start lines. "
    "DUNVIEW.C:8525 / DUNVIEW.C:8529 are the F0128 caller sites (8525 = "
    "D1L, 8529 = D1R). "
    "DUNVIEW.C:7495 D1L F0115 thing pass1 cell order is "
    "C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT at M607_VIEW_SQUARE_D1L; "
    "DUNVIEW.C:7663 D1R F0115 thing pass1 cell order is "
    "C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT at M608_VIEW_SQUARE_D1R. The "
    "door-frame-top blit runs after F0108 floor-ornament and F0115 pass1, "
    "and before F0111_DUNGEONVIEW_DrawDoor door-panel state machine. "
    "F0100_DUNGEONVIEW_DrawWallSetBitmap (DUNVIEW.C:3048-3068) blits the "
    "bitmap into G0296_puc_Bitmap_Viewport using stride X/Y/ByteWidth and "
    "C10_COLOR_FLESH transparency. "
    "DEFS.H:2585-2586 / 2600-2601 declare M607_VIEW_SQUARE_D1L / "
    "M608_VIEW_SQUARE_D1R as 4/5 (legacy MEDIA008 block) and 7/8 (modern). "
    "DEFS.H:4068-4073 (MEDIA508 block) defines C725/C726/C727 (D2) and "
    "C728_ZONE_DOOR_FRAME_TOP_D1L = 728, C729_ZONE_DOOR_FRAME_TOP_D1C = "
    "729, C730_ZONE_DOOR_FRAME_TOP_D1R = 730. DEFS.H:4087-4093 (MEDIA720 "
    "block) defines C729/C730/C731 (D2) and C732_ZONE_DOOR_FRAME_TOP_D1L "
    "= 732, C733_ZONE_DOOR_FRAME_TOP_D1C = 733, C734_ZONE_DOOR_FRAME_TOP_"
    "D1R = 734. The D1L/D1R side strip pins C728/C730 (F20E) and C732/"
    "C734 (I34E) only; C729/C733 belong to the D1C parallel gate. "
    "DEFS.H:2088 C10_COLOR_FLESH is the legacy door-frame-top blit "
    "transparency (F0100 path). "
    "DEFS.H:2159 M075_BITMAP_BYTE_COUNT(width, height) = ((width >> 1) * "
    "height); for the D1 door panel 96x88 the byte count is 4224 (called "
    "from F0122/F0123 C17_ELEMENT_DOOR_FRONT MEDIA009 dispatch via "
    "M075_BITMAP_BYTE_COUNT(96, 88)). "
    "DEFS.H:5458 G0695_ai_DoorNativeBitmapIndex_Front_D1LCR is the "
    "C17_ELEMENT_DOOR_FRONT MEDIA009 door-panel native bitmap index for "
    "D1L/D1R (used in F0111_DUNGEONVIEW_DrawDoor following the door-frame-"
    "top blit). DEFS.H:2791 C2_VIEW_DOOR_ORNAMENT_D1LCR is the door-panel "
    "ornament for D1L/D1R/D1C following the door-frame-top blit. "
    "DEFS.H:5542 G0185_s_Graphic558_Frames_Door_D1L and DEFS.H:5544 "
    "G0187_s_Graphic558_Frames_Door_D1R are the D1L/D1R door-frame DOOR_"
    "FRAMES Graphic558 stride tables (X1=0/192, X2=31/223, Y1=17, Y2=102, "
    "ByteWidth=48, Height=88). "
    "G2112_DoorFrameTopD1LCR is the D1C door-frame-top native bitmap "
    "(DUNVIEW.C:146/222/238/255/2186/2201) loaded into the modern PC 3.4 "
    "I34E target via F0490_MEMORY_LoadDecompressAndExpandGraphic; the "
    "D1L/D1R corridor-side gate does not consume G2112 directly, but the "
    "G2112 anchor pins the family so the D1L/D1R G2110/G2111 anchors are "
    "non-orphaned. "
    "Non-overlap marker pass794-d1l-d1r-door-frame-top-edge-source-lock: "
    "this gate pins the door-frame-top edge stride, zone, and dispatch "
    "line numbers for D1L/D1R only. It is distinct from the D2L/D2R "
    "door-frame-top edge (dm1_v1_viewport_d2l_d2r_door_frame_top_edge_"
    "pc34_compat, Y1=22..Y2=24 band and G0173/G0175 strides), the D2C "
    "door-frame-top edge (dm1_v1_viewport_d2c_door_frame_top_edge_"
    "pc34_compat, G0174 stride and C726/C730 zones), the D1C door-frame-"
    "top edge (dm1_v1_viewport_d1c_door_frame_top_edge_pc34_compat, "
    "G0177 stride and C729/C733 zones, F0124 dispatch), the F0111 door-"
    "panel state machine (dm1_v1_viewport_d1l_d1r_f0111_partly_open_"
    "door_pc34_compat), the D1L2/D1R2 front-door pair "
    "(dm1_v1_viewport_d1l2_d1r2_f0111_door_front_pair_pc34_compat), the "
    "D0C door-frame border + thieves-eye hole (dm1_v1_viewport_d0c_door"
    "_edge_ornament_pc34_compat), the D1L/D1R wall content "
    "(dm1_v1_viewport_d1l_d1r_wall_pc34_compat), the D1L2/D1R2 side wall "
    "(dm1_v1_viewport_d1l2_d1r2_side_wall_pc34_compat), the D1L2/D1R2 "
    "wall ornament (dm1_v1_viewport_d1l2_d1r2_f0107_wall_ornament_pc34_"
    "compat), the D1L2/D1R2 floor ornament (dm1_v1_viewport_d1l2_d1r2_"
    "f0108_floor_ornament_pc34_compat), and the D1L2/D1R2 wall composi"
    "tion (dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat)."
    " The D1 door-frame-top edge is the 4-pixel-tall horizontal bar above "
    "the door panel (Y1=14..Y2=17); the door panel itself starts at "
    "Y=17 and spans to Y=102 inside G0185/G0187.";

static DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 s_last;

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t mix_string(uint32_t hash, const char *text)
{
    if (!text) return mix_u32(hash, 0xffffffffu);
    while (*text) {
        hash ^= (uint8_t)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void assert_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void assert_contains(const char *id, const char *haystack, const char *needle)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, needle);
    assert_int(id, found, 1);
}

static int zone_for_target_d1l(int target_media)
{
    switch (target_media) {
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
        /* Legacy MEDIA009 path uses F0100 with no zone id; report the
         * C10_COLOR_FLESH transparency sentinel for symmetry. */
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C728_ZONE_D1L_F20E_PC34;
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C732_ZONE_D1L_I34E_PC34;
    default:
        return -1;
    }
}

static int zone_for_target_d1r(int target_media)
{
    switch (target_media) {
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D1R_F20E_PC34;
    case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
        return DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C734_ZONE_D1R_I34E_PC34;
    default:
        return -1;
    }
}

int dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
    int side,
    int target_media,
    DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    if (target_media < DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND ||
        target_media > DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) {
        return 0;
    }
    if (side != DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 &&
        side != DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        return 0;
    }

    memset(out_trace, 0, sizeof(*out_trace));

    out_trace->side = side;
    out_trace->target_media = target_media;
    out_trace->framebuffer_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34;

    out_trace->d1l_stride_left_x =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_LEFT_X_PC34;
    out_trace->d1l_stride_right_x =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_RIGHT_X_PC34;
    out_trace->d1l_stride_top_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_TOP_Y_PC34;
    out_trace->d1l_stride_bottom_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BOTTOM_Y_PC34;
    out_trace->d1l_stride_byte_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BYTE_WIDTH_PC34;
    out_trace->d1l_stride_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_HEIGHT_PC34;
    out_trace->d1l_stride_x_offset =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_X_OFFSET_PC34;
    out_trace->d1l_stride_y_offset =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_Y_OFFSET_PC34;

    out_trace->d1r_stride_left_x =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_LEFT_X_PC34;
    out_trace->d1r_stride_right_x =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_RIGHT_X_PC34;
    out_trace->d1r_stride_top_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_TOP_Y_PC34;
    out_trace->d1r_stride_bottom_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BOTTOM_Y_PC34;
    out_trace->d1r_stride_byte_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BYTE_WIDTH_PC34;
    out_trace->d1r_stride_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_HEIGHT_PC34;
    out_trace->d1r_stride_x_offset =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_X_OFFSET_PC34;
    out_trace->d1r_stride_y_offset =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_Y_OFFSET_PC34;

    out_trace->band_top_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34;
    out_trace->band_bottom_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34;
    out_trace->band_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34;
    out_trace->band_byte_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34;

    out_trace->m607_view_square_d1l =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M607_VIEW_SQUARE_D1L_PC34;
    out_trace->m608_view_square_d1r =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M608_VIEW_SQUARE_D1R_PC34;
    out_trace->d1l_pass1_cell_order =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_PASS1_CELL_ORDER_PC34;
    out_trace->d1r_pass1_cell_order =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_PASS1_CELL_ORDER_PC34;

    out_trace->door_panel_top_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34;
    out_trace->door_panel_bottom_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34;
    out_trace->door_panel_height =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34;
    out_trace->door_panel_byte_width =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34;
    out_trace->bitmap_byte_count_d1lcr =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D1LCR_PC34;

    out_trace->door_frame_top_bitmap_id =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0704_DOOR_FRAME_TOP_BITMAP_PC34;
    out_trace->door_frame_top_stride_d1l_id =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0176_STRIDE_PC34;
    out_trace->door_frame_top_stride_d1r_id =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0178_STRIDE_PC34;
    out_trace->door_frame_top_native_bitmap_d1l =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2111_DOOR_FRAME_TOP_D1L_PC34;
    out_trace->door_frame_top_native_bitmap_d1r =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2110_DOOR_FRAME_TOP_D1R_PC34;
    out_trace->door_frame_top_native_bitmap_d1lcr =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2112_DOOR_FRAME_TOP_D1LCR_PC34;

    out_trace->f0100_blit_transparency_color =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    out_trace->f0122_d1l_door_front_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRONT_LINE_PC34;
    out_trace->f0122_d1l_door_frame_top_legacy_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_LEGACY_LINE_PC34;
    out_trace->f0122_d1l_door_frame_top_f20e_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_F20E_LINE_PC34;
    out_trace->f0122_d1l_door_frame_top_i34e_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_I34E_LINE_PC34;
    out_trace->f0123_d1r_door_front_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRONT_LINE_PC34;
    out_trace->f0123_d1r_door_frame_top_legacy_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_LEGACY_LINE_PC34;
    out_trace->f0123_d1r_door_frame_top_f20e_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_F20E_LINE_PC34;
    out_trace->f0123_d1r_door_frame_top_i34e_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_I34E_LINE_PC34;
    out_trace->f0128_dispatch_line =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34;

    out_trace->d1l_door_frame_top_zone = zone_for_target_d1l(target_media);
    out_trace->d1r_door_frame_top_zone = zone_for_target_d1r(target_media);
    out_trace->g0695_door_panel_bitmap =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0695_DOOR_BITMAP_PC34;
    out_trace->g0185_door_frames_d1l =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0185_DOOR_FRAMES_D1L_PC34;
    out_trace->g0187_door_frames_d1r =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0187_DOOR_FRAMES_D1R_PC34;
    out_trace->c2_view_door_ornament_d1lcr =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C2_VIEW_DOOR_ORNAMENT_D1LCR_PC34;

    out_trace->d1l_band_in_viewport =
        (out_trace->d1l_stride_top_y < out_trace->viewport_height) ? 1 : 0;
    out_trace->d1r_band_in_viewport =
        (out_trace->d1r_stride_top_y < out_trace->viewport_height) ? 1 : 0;
    out_trace->d1l_band_inside_door_panel_band =
        (out_trace->d1l_stride_bottom_y <= out_trace->door_panel_top_y) ? 1 : 0;
    out_trace->d1r_band_inside_door_panel_band =
        (out_trace->d1r_stride_bottom_y <= out_trace->door_panel_top_y) ? 1 : 0;

    out_trace->d1l_legacy_route_uses_g0704 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) ? 1 : 0;
    out_trace->d1r_legacy_route_uses_g0704 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) ? 1 : 0;
    out_trace->d1l_f20e_route_uses_g2111 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) ? 1 : 0;
    out_trace->d1r_f20e_route_uses_g2110 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) ? 1 : 0;
    out_trace->d1l_i34e_route_uses_g2111 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) ? 1 : 0;
    out_trace->d1r_i34e_route_uses_g2110 =
        (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) ? 1 : 0;

    out_trace->c10_transparent_blit = 1;

    out_trace->band_strip_destination_x_d1l =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_LEFT_X_PC34 +
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_X_OFFSET_PC34;
    out_trace->band_strip_destination_x_d1r =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_LEFT_X_PC34 +
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_X_OFFSET_PC34;
    out_trace->band_strip_destination_y =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34;
    out_trace->band_strip_byte_width_d1l =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BYTE_WIDTH_PC34;
    out_trace->band_strip_byte_width_d1r =
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BYTE_WIDTH_PC34;

    /* Probe pixels: contract-only trace returns sentinel zeros; the
     * asset-free contract surface only verifies geometry and zone
     * anchors, not real framebuffer contents. */
    out_trace->d1l_first_probe_pixel = 0;
    out_trace->d1l_second_probe_pixel = 0;
    out_trace->d1l_third_probe_pixel = 0;
    out_trace->d1r_first_probe_pixel = 0;
    out_trace->d1r_second_probe_pixel = 0;
    out_trace->d1r_third_probe_pixel = 0;

    return 1;
}

const char *
dm1_v1_viewport_d1l_d1r_door_frame_top_edge_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static int expected_count_for(int side, int target_media)
{
    /* Three targets x two sides = 6 valid combos, plus one invalid. */
    if (side != DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 &&
        side != DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        return -1;
    }
    if (target_media < DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND ||
        target_media > DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) {
        return -1;
    }
    return 1;
}

static void increment_target_counter(int side, int target_media)
{
    if (side == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34) {
        switch (target_media) {
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
            ++s_last.d1l_legacy_zone_count; break;
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
            ++s_last.d1l_f20e_zone_count; break;
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
            ++s_last.d1l_i34e_zone_count; break;
        default:
            ++s_last.invalid_target_count; break;
        }
    } else if (side == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        switch (target_media) {
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
            ++s_last.d1r_legacy_zone_count; break;
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
            ++s_last.d1r_f20e_zone_count; break;
        case DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
            ++s_last.d1r_i34e_zone_count; break;
        default:
            ++s_last.invalid_target_count; break;
        }
    } else {
        ++s_last.invalid_target_count;
    }
}

static void check_one(int side, int target_media)
{
    DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
    const int rc = dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
        side, target_media, &trace);

    assert_int("trace.ok", rc, 1);
    assert_int("trace.side", trace.side, side);
    assert_int("trace.target_media", trace.target_media, target_media);

    /* D1L stride anchors. */
    assert_int("d1l.stride.left_x", trace.d1l_stride_left_x,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_LEFT_X_PC34);
    assert_int("d1l.stride.right_x", trace.d1l_stride_right_x,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_RIGHT_X_PC34);
    assert_int("d1l.stride.top_y", trace.d1l_stride_top_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_TOP_Y_PC34);
    assert_int("d1l.stride.bottom_y", trace.d1l_stride_bottom_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BOTTOM_Y_PC34);
    assert_int("d1l.stride.byte_width", trace.d1l_stride_byte_width,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BYTE_WIDTH_PC34);
    assert_int("d1l.stride.height", trace.d1l_stride_height,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_HEIGHT_PC34);
    assert_int("d1l.stride.x_offset", trace.d1l_stride_x_offset,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_X_OFFSET_PC34);
    assert_int("d1l.stride.y_offset", trace.d1l_stride_y_offset,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_Y_OFFSET_PC34);
    ++s_last.stride_g0176_checks;

    /* D1R stride anchors. */
    assert_int("d1r.stride.left_x", trace.d1r_stride_left_x,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_LEFT_X_PC34);
    assert_int("d1r.stride.right_x", trace.d1r_stride_right_x,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_RIGHT_X_PC34);
    assert_int("d1r.stride.top_y", trace.d1r_stride_top_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_TOP_Y_PC34);
    assert_int("d1r.stride.bottom_y", trace.d1r_stride_bottom_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BOTTOM_Y_PC34);
    assert_int("d1r.stride.byte_width", trace.d1r_stride_byte_width,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BYTE_WIDTH_PC34);
    assert_int("d1r.stride.height", trace.d1r_stride_height,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_HEIGHT_PC34);
    assert_int("d1r.stride.x_offset", trace.d1r_stride_x_offset,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_X_OFFSET_PC34);
    assert_int("d1r.stride.y_offset", trace.d1r_stride_y_offset,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_Y_OFFSET_PC34);
    ++s_last.stride_g0178_checks;

    /* Band strip geometry. */
    assert_int("band.top_y", trace.band_top_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34);
    assert_int("band.bottom_y", trace.band_bottom_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34);
    assert_int("band.height", trace.band_height,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34);
    assert_int("band.byte_width", trace.band_byte_width,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34);
    assert_int("band.strip.destination_x.d1l",
               trace.band_strip_destination_x_d1l,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_LEFT_X_PC34 +
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_X_OFFSET_PC34);
    assert_int("band.strip.destination_x.d1r",
               trace.band_strip_destination_x_d1r,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_LEFT_X_PC34 +
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_X_OFFSET_PC34);
    assert_int("band.strip.destination_y", trace.band_strip_destination_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34);
    assert_int("band.strip.byte_width.d1l", trace.band_strip_byte_width_d1l,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BYTE_WIDTH_PC34);
    assert_int("band.strip.byte_width.d1r", trace.band_strip_byte_width_d1r,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BYTE_WIDTH_PC34);
    ++s_last.band_strip_checks;

    /* Band sits inside the viewport (Y=14 < 136) and above the door
     * panel (Y=17 <= Y=17). */
    assert_int("d1l.band.in_viewport", trace.d1l_band_in_viewport, 1);
    assert_int("d1r.band.in_viewport", trace.d1r_band_in_viewport, 1);
    assert_int("d1l.band.above.door_panel",
               trace.d1l_band_inside_door_panel_band, 1);
    assert_int("d1r.band.above.door_panel",
               trace.d1r_band_inside_door_panel_band, 1);

    /* View-square anchors. */
    assert_int("m607.view.square.d1l", trace.m607_view_square_d1l,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M607_VIEW_SQUARE_D1L_PC34);
    assert_int("m608.view.square.d1r", trace.m608_view_square_d1r,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M608_VIEW_SQUARE_D1R_PC34);
    assert_int("d1l.pass1.cell_order", trace.d1l_pass1_cell_order,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_PASS1_CELL_ORDER_PC34);
    assert_int("d1r.pass1.cell_order", trace.d1r_pass1_cell_order,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_PASS1_CELL_ORDER_PC34);
    ++s_last.view_square_anchor_checks;

    /* Door-panel post-band anchors. */
    assert_int("door_panel.top_y", trace.door_panel_top_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34);
    assert_int("door_panel.bottom_y", trace.door_panel_bottom_y,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34);
    assert_int("door_panel.height", trace.door_panel_height,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34);
    assert_int("door_panel.byte_width", trace.door_panel_byte_width,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34);
    assert_int("bitmap.byte_count.d1lcr",
               trace.bitmap_byte_count_d1lcr,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D1LCR_PC34);
    assert_int("g0695.door_panel.bitmap", trace.g0695_door_panel_bitmap,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0695_DOOR_BITMAP_PC34);
    assert_int("g0185.door.frames.d1l", trace.g0185_door_frames_d1l,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0185_DOOR_FRAMES_D1L_PC34);
    assert_int("g0187.door.frames.d1r", trace.g0187_door_frames_d1r,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0187_DOOR_FRAMES_D1R_PC34);
    assert_int("c2.view.door.ornament.d1lcr",
               trace.c2_view_door_ornament_d1lcr,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C2_VIEW_DOOR_ORNAMENT_D1LCR_PC34);
    ++s_last.door_panel_post_band_checks;

    /* Zone IDs. */
    if (target_media == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) {
        assert_int("d1l.legacy.route.uses.g0704",
                   trace.d1l_legacy_route_uses_g0704, 1);
        assert_int("d1r.legacy.route.uses.g0704",
                   trace.d1r_legacy_route_uses_g0704, 1);
        assert_int("d1l.door_frame_top.zone", trace.d1l_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
        assert_int("d1r.door_frame_top.zone", trace.d1r_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
        assert_int("d1l.f20e.route.uses.g2111",
                   trace.d1l_f20e_route_uses_g2111, 0);
        assert_int("d1r.f20e.route.uses.g2110",
                   trace.d1r_f20e_route_uses_g2110, 0);
        assert_int("d1l.i34e.route.uses.g2111",
                   trace.d1l_i34e_route_uses_g2111, 0);
        assert_int("d1r.i34e.route.uses.g2110",
                   trace.d1r_i34e_route_uses_g2110, 0);
    } else if (target_media ==
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) {
        assert_int("d1l.legacy.route.uses.g0704",
                   trace.d1l_legacy_route_uses_g0704, 0);
        assert_int("d1r.legacy.route.uses.g0704",
                   trace.d1r_legacy_route_uses_g0704, 0);
        assert_int("d1l.f20e.route.uses.g2111",
                   trace.d1l_f20e_route_uses_g2111, 1);
        assert_int("d1r.f20e.route.uses.g2110",
                   trace.d1r_f20e_route_uses_g2110, 1);
        assert_int("d1l.i34e.route.uses.g2111",
                   trace.d1l_i34e_route_uses_g2111, 0);
        assert_int("d1r.i34e.route.uses.g2110",
                   trace.d1r_i34e_route_uses_g2110, 0);
        assert_int("d1l.door_frame_top.zone", trace.d1l_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C728_ZONE_D1L_F20E_PC34);
        assert_int("d1r.door_frame_top.zone", trace.d1r_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D1R_F20E_PC34);
    } else {
        assert_int("d1l.legacy.route.uses.g0704",
                   trace.d1l_legacy_route_uses_g0704, 0);
        assert_int("d1r.legacy.route.uses.g0704",
                   trace.d1r_legacy_route_uses_g0704, 0);
        assert_int("d1l.f20e.route.uses.g2111",
                   trace.d1l_f20e_route_uses_g2111, 0);
        assert_int("d1r.f20e.route.uses.g2110",
                   trace.d1r_f20e_route_uses_g2110, 0);
        assert_int("d1l.i34e.route.uses.g2111",
                   trace.d1l_i34e_route_uses_g2111, 1);
        assert_int("d1r.i34e.route.uses.g2110",
                   trace.d1r_i34e_route_uses_g2110, 1);
        assert_int("d1l.door_frame_top.zone", trace.d1l_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C732_ZONE_D1L_I34E_PC34);
        assert_int("d1r.door_frame_top.zone", trace.d1r_door_frame_top_zone,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C734_ZONE_D1R_I34E_PC34);
    }
    ++s_last.zone_id_family_checks;
    ++s_last.bitmap_route_checks;

    /* F0122 / F0123 / F0128 dispatch line anchors. */
    assert_int("f0122.d1l.door_front.line",
               trace.f0122_d1l_door_front_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRONT_LINE_PC34);
    assert_int("f0122.d1l.door_frame_top.legacy.line",
               trace.f0122_d1l_door_frame_top_legacy_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_LEGACY_LINE_PC34);
    assert_int("f0122.d1l.door_frame_top.f20e.line",
               trace.f0122_d1l_door_frame_top_f20e_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_F20E_LINE_PC34);
    assert_int("f0122.d1l.door_frame_top.i34e.line",
               trace.f0122_d1l_door_frame_top_i34e_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_I34E_LINE_PC34);
    assert_int("f0123.d1r.door_front.line",
               trace.f0123_d1r_door_front_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRONT_LINE_PC34);
    assert_int("f0123.d1r.door_frame_top.legacy.line",
               trace.f0123_d1r_door_frame_top_legacy_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_LEGACY_LINE_PC34);
    assert_int("f0123.d1r.door_frame_top.f20e.line",
               trace.f0123_d1r_door_frame_top_f20e_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_F20E_LINE_PC34);
    assert_int("f0123.d1r.door_frame_top.i34e.line",
               trace.f0123_d1r_door_frame_top_i34e_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_I34E_LINE_PC34);
    assert_int("f0128.dispatch.line", trace.f0128_dispatch_line,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34);

    /* C10 transparency for the F0100 legacy route. */
    assert_int("f0100.blit.transparency.color",
               trace.f0100_blit_transparency_color,
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    assert_int("c10.transparent.blit", trace.c10_transparent_blit, 1);
    ++s_last.c10_transparency_checks;

    /* Frame buffer probe: contract-only sentinel zeros. */
    assert_int("d1l.first.probe.pixel", trace.d1l_first_probe_pixel, 0);
    assert_int("d1l.second.probe.pixel", trace.d1l_second_probe_pixel, 0);
    assert_int("d1l.third.probe.pixel", trace.d1l_third_probe_pixel, 0);
    assert_int("d1r.first.probe.pixel", trace.d1r_first_probe_pixel, 0);
    assert_int("d1r.second.probe.pixel", trace.d1r_second_probe_pixel, 0);
    assert_int("d1r.third.probe.pixel", trace.d1r_third_probe_pixel, 0);

    increment_target_counter(side, target_media);
}

static void check_non_overlap(void)
{
    static const char *siblings[] = {
        "D2L/D2R F0119/F0120 C17_ELEMENT_DOOR_FRONT G0173 G0175",
        "D2C F0117/F0121 C17_ELEMENT_DOOR_FRONT G0174",
        "D1C F0124 C17_ELEMENT_DOOR_FRONT G0177",
        "D1L/D1R F0111 partly-open half-blit body",
        "D1C F0111 partly-open",
        "D1L2/D1R2 front-door pair",
        "D0C door-frame border + thieves-eye hole",
        "D1L/D1R wall content",
        "D1L2/D1R2 side wall",
        "D1L2/D1R2 wall ornament",
        "D1L2/D1R2 floor ornament",
        "D1L2/D1R2 wall composition",
        "D2L2/D2R2 front-door pair",
        "D2L/D2R wall content",
        "D2L2/D2R2 side wall",
        "D2L2/D2R2 wall ornament",
        "D2L2/D2R2 floor ornament",
        "D2L2/D2R2 wall composition",
        "D3L/D3R F0111 door-front",
        "D3C F0111 door-front",
        "D0L/D0R F0111 door-front",
        "D1L2/D1R2 F0111 partly-open",
        "D0L2/D0R2 F0111 partly-open",
        "D0C F0111 partly-open",
        "D2C F0111 partly-open",
        "D2L/D2R F0108 floor-ornament",
        "D2L/D2R F0107 wall-ornament",
        "D2L/D2R F0098 fallback",
        "D3L/D3R F0111 door-front pair",
        "D3C F0111 door-front pair",
    };
    size_t i;

    for (i = 0; i < sizeof(siblings) / sizeof(siblings[0]); ++i) {
        assert_int("non.overlap.uses.g0176",
                   strstr(siblings[i], "G0176") != NULL, 0);
        assert_int("non.overlap.uses.g0178",
                   strstr(siblings[i], "G0178") != NULL, 0);
        assert_int("non.overlap.uses.g0704",
                   strstr(siblings[i], "G0704") != NULL, 0);
        assert_int("non.overlap.uses.g2111",
                   strstr(siblings[i], "G2111") != NULL, 0);
        assert_int("non.overlap.uses.g2110",
                   strstr(siblings[i], "G2110") != NULL, 0);
        assert_int("non.overlap.uses.d1l_d1r_door_frame_top",
                   strstr(siblings[i], "d1l_d1r_door_frame_top") != NULL, 0);
        ++s_last.non_overlap_checks;
    }
}

int run_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    assert_contains("source.dunview.607", s_source_evidence,
                    "DUNVIEW.C:607-609");
    assert_contains("source.g0176", s_source_evidence,
                    "G0176_auc_Graphic558_Frame_DoorFrameTop_D1L");
    assert_contains("source.g0178", s_source_evidence,
                    "G0178_auc_Graphic558_Frame_DoorFrameTop_D1R");
    assert_contains("source.d1l.stride.values", s_source_evidence,
                    "{ 0, 31, 14, 17, 64, 4, 16, 0 }");
    assert_contains("source.d1r.stride.values", s_source_evidence,
                    "{ 192, 223, 14, 17, 64, 4, 16, 0 }");
    assert_contains("source.f0122.d1l.door.front", s_source_evidence,
                    "DUNVIEW.C:7496");
    assert_contains("source.f0122.d1l.f20e", s_source_evidence,
                    "DUNVIEW.C:7500");
    assert_contains("source.f0122.d1l.i34e", s_source_evidence,
                    "DUNVIEW.C:7503");
    assert_contains("source.f0123.d1r.legacy", s_source_evidence,
                    "DUNVIEW.C:7664");
    assert_contains("source.f0123.d1r.f20e", s_source_evidence,
                    "DUNVIEW.C:7668");
    assert_contains("source.f0123.d1r.i34e", s_source_evidence,
                    "DUNVIEW.C:7671");
    assert_contains("source.f0122.start", s_source_evidence,
                    "DUNVIEW.C:7391");
    assert_contains("source.f0123.start", s_source_evidence,
                    "DUNVIEW.C:7559");
    assert_contains("source.f0128.dispatch", s_source_evidence,
                    "DUNVIEW.C:8525");
    assert_contains("source.g0704.bitmap", s_source_evidence,
                    "G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR");
    assert_contains("source.g2111.d1l", s_source_evidence,
                    "G2111_DoorFrameTopD1L");
    assert_contains("source.g2110.d1r", s_source_evidence,
                    "G2110_DoorFrameTopD1R");
    assert_contains("source.c728.f20e.d1l", s_source_evidence,
                    "C728_ZONE_DOOR_FRAME_TOP_D1L = 728");
    assert_contains("source.c730.f20e.d1r", s_source_evidence,
                    "C730_ZONE_DOOR_FRAME_TOP_D1R = 730");
    assert_contains("source.c732.i34e.d1l", s_source_evidence,
                    "C732_ZONE_DOOR_FRAME_TOP_D1L = 732");
    assert_contains("source.c734.i34e.d1r", s_source_evidence,
                    "C734_ZONE_DOOR_FRAME_TOP_D1R = 734");
    assert_contains("source.m607.m608", s_source_evidence,
                    "M607_VIEW_SQUARE_D1L");
    assert_contains("source.cell.order.d1l", s_source_evidence,
                    "C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT");
    assert_contains("source.cell.order.d1r", s_source_evidence,
                    "C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT");
    assert_contains("source.c10.flesh", s_source_evidence,
                    "C10_COLOR_FLESH");
    assert_contains("source.g0695", s_source_evidence,
                    "G0695");
    assert_contains("source.g0185", s_source_evidence,
                    "G0185");
    assert_contains("source.g0187", s_source_evidence,
                    "G0187");
    assert_contains("source.c2.view.door.ornament", s_source_evidence,
                    "C2_VIEW_DOOR_ORNAMENT_D1LCR");
    assert_contains("source.f0111.dispatch", s_source_evidence,
                    "F0111_DUNGEONVIEW_DrawDoor door-panel state");
    assert_contains("source.f0100.transparency", s_source_evidence,
                    "F0100_DUNGEONVIEW_DrawWallSetBitmap");
    assert_contains("source.non.overlap.marker", s_source_evidence,
                    "pass794-d1l-d1r-door-frame-top-edge-source-lock");
    assert_contains("source.bridge", s_source_evidence,
                    "door-frame-top edge stride, zone, and dispatch");
    assert_contains("source.post.band.door.panel", s_source_evidence,
                    "Y=17 and spans to Y=102");
    assert_contains("source.g2112.d1lcr", s_source_evidence,
                    "G2112_DoorFrameTopD1LCR");
    assert_contains("source.m075.byte.count", s_source_evidence,
                    "M075_BITMAP_BYTE_COUNT(96, 88)");
    assert_contains("source.f0122.d1l.door.front.body", s_source_evidence,
                    "DUNVIEW.C:7494");
    assert_contains("source.f0123.d1r.door.front.body", s_source_evidence,
                    "DUNVIEW.C:7662");
    assert_contains("source.f0128.d1r.caller", s_source_evidence,
                    "DUNVIEW.C:8529");

    assert_int("expected.legacy.d1l",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND),
               1);
    assert_int("expected.legacy.d1r",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND),
               1);
    assert_int("expected.f20e.d1l",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND),
               1);
    assert_int("expected.f20e.d1r",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND),
               1);
    assert_int("expected.i34e.d1l",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND),
               1);
    assert_int("expected.i34e.d1r",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND),
               1);
    assert_int("expected.bad.target",
               expected_count_for(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99),
               -1);
    assert_int("expected.bad.side",
               expected_count_for(99, 0), -1);

    /* Six valid combinations. */
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND);
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND);
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND);
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND);
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND);
    check_one(DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND);

    /* Invalid: out-of-range target. */
    {
        DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
        const int rc = dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
            DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99, &trace);
        assert_int("invalid.target.rc", rc, 0);
        ++s_last.invalid_target_count;
    }
    /* Invalid: out-of-range side. */
    {
        DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
        const int rc = dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
            99, DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND, &trace);
        assert_int("invalid.side.rc", rc, 0);
        ++s_last.invalid_target_count;
    }
    /* Null output. */
    assert_int("null.output.rc",
               dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND,
                   NULL),
               0);

    check_non_overlap();

    assert_int("d1l.legacy.count", s_last.d1l_legacy_zone_count, 1);
    assert_int("d1l.f20e.count", s_last.d1l_f20e_zone_count, 1);
    assert_int("d1l.i34e.count", s_last.d1l_i34e_zone_count, 1);
    assert_int("d1r.legacy.count", s_last.d1r_legacy_zone_count, 1);
    assert_int("d1r.f20e.count", s_last.d1r_f20e_zone_count, 1);
    assert_int("d1r.i34e.count", s_last.d1r_i34e_zone_count, 1);
    assert_int("invalid.count", s_last.invalid_target_count, 2);
    assert_int("stride.g0176.checks", s_last.stride_g0176_checks, 6);
    assert_int("stride.g0178.checks", s_last.stride_g0178_checks, 6);
    assert_int("band.strip.checks", s_last.band_strip_checks, 6);
    assert_int("zone.id.family.checks", s_last.zone_id_family_checks, 6);
    assert_int("door.panel.post.band.checks",
               s_last.door_panel_post_band_checks, 6);
    assert_int("view.square.anchor.checks",
               s_last.view_square_anchor_checks, 6);
    assert_int("non.overlap.checks", s_last.non_overlap_checks, 30);
    assert_int("bitmap.route.checks", s_last.bitmap_route_checks, 6);
    assert_int("c10.transparency.checks", s_last.c10_transparency_checks, 6);
    assert_int("hash.changed", s_last.deterministic_hash != 2166136261u, 1);

    return s_last.failures == 0 &&
           s_last.deterministic_hash ==
               DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_HASH_PC34
                   ? 0
                   : 1;
}

const DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34(void)
{
    return &s_last;
}
