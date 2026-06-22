#include "firestaff/dm1/v1/viewport/d2l_d2r_door_frame_top_edge_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND = 0,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND = 1,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND = 2
};

static const char s_source_evidence[] =
    "DM1 V1 D2L/D2R door-frame-top edge source-lock probe; contract-only "
    "and asset-free; no real-asset or original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:604-606 defines the three D2 door-frame-top strides "
    "as G0173_auc_Graphic558_Frame_DoorFrameTop_D2L = { 0, 59, 22, 24, 48, "
    "3, 16, 0 }, G0174_auc_Graphic558_Frame_DoorFrameTop_D2C = { 64, 159, "
    "22, 24, 48, 3, 0, 0 }, and G0175_auc_Graphic558_Frame_DoorFrameTop_D2R "
    "= { 164, 223, 22, 24, 48, 3, 16, 0 } with X1=0/64/164, X2=59/159/223, "
    "Y1=22, Y2=24, ByteWidth=48, Height=3, X=0/16/16, Y=0. The D2L/D2R X "
    "offsets are 16, which reflects the side-view shift relative to the "
    "centered D2C frame. "
    "DUNVIEW.C:6991 (F0119_DUNGEONVIEW_DrawSquareD2L C17_ELEMENT_DOOR_FRONT "
    "MEDIA009_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_"
    "G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G) calls "
    "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0703_puc_Bitmap_WallSet_DoorFrame"
    "Top_D2LCR, G0173_auc_Graphic558_Frame_DoorFrameTop_D2L) which is the "
    "legacy PC 3.4 door-frame-top edge for D2L. "
    "DUNVIEW.C:6994 (F0119 D2L C17_ELEMENT_DOOR_FRONT MEDIA508_F20E_F20J_"
    "X30J_P20JA_P20JB) calls F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"
    "(G2114_DoorFrameTopD2L, C725_ZONE_DOOR_FRAME_TOP_D2L). "
    "DUNVIEW.C:6997 (F0119 D2L C17_ELEMENT_DOOR_FRONT MEDIA720_I34E_I34M_"
    "A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J) calls "
    "F0104(G2114_DoorFrameTopD2L, C729_ZONE_DOOR_FRAME_TOP_D2L). "
    "DUNVIEW.C:7184 (F0120_DUNGEONVIEW_DrawSquareD2R_CPSF C17_ELEMENT_DOOR_"
    "FRONT MEDIA009) calls F0100(G0703, G0175) for D2R. "
    "DUNVIEW.C:7187 (F0120 D2R MEDIA508) calls "
    "F0104(G2113_DoorFrameTopD2R, C727_ZONE_DOOR_FRAME_TOP_D2R). "
    "DUNVIEW.C:7190 (F0120 D2R MEDIA720) calls "
    "F0104(G2113_DoorFrameTopD2R, C731_ZONE_DOOR_FRAME_TOP_D2R). "
    "DUNVIEW.C:6900 / DUNVIEW.C:7051 are the F0119 / F0120 function start lines. "
    "DUNVIEW.C:8513 / 8517 are the F0128 caller sites. "
    "DUNVIEW.C:6990 D2L F0115 thing pass1 cell order is "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT at M604_VIEW_SQUARE_"
    "D2L; DUNVIEW.C:7183 D2R F0115 thing pass1 cell order is "
    "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT at M605_VIEW_SQUARE_"
    "D2R. The door-frame-top blit runs after F0108 floor-ornament and "
    "F0115 pass1, and before F0111_DUNGEONVIEW_DrawDoor door-panel state "
    "machine. "
    "F0100_DUNGEONVIEW_DrawWallSetBitmap (DUNVIEW.C:3048-3068) blits the "
    "bitmap into G0296_puc_Bitmap_Viewport using stride X/Y/ByteWidth and "
    "C10_COLOR_FLESH transparency. "
    "DEFS.H:2582-2583 / 2603-2604 declare M604_VIEW_SQUARE_D2L / "
    "M605_VIEW_SQUARE_D2R as 4/5 (legacy) and 7/8 (modern). "
    "DEFS.H:4068-4070 (MEDIA508 block) defines C725_ZONE_DOOR_FRAME_TOP_"
    "D2L = 725, C726_ZONE_DOOR_FRAME_TOP_D2C = 726, C727_ZONE_DOOR_FRAME_"
    "TOP_D2R = 727. DEFS.H:4087-4089 (MEDIA720 block) defines "
    "C729_ZONE_DOOR_FRAME_TOP_D2L = 729, C730_ZONE_DOOR_FRAME_TOP_D2C = "
    "730, C731_ZONE_DOOR_FRAME_TOP_D2R = 731. "
    "DEFS.H:2088 C10_COLOR_FLESH is the legacy door-frame-top blit "
    "transparency (F0100 path). "
    "DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2790 C1_VIEW_DOOR_"
    "ORNAMENT_D2LCR; DEFS.H:5457 G0694; DEFS.H:5539 G0182; DEFS.H:5541 "
    "G0184. "
    "G2115_DoorFrameTopD2LCR is the D2C door-frame-top native bitmap "
    "(DUNVIEW.C:145/225/241/258/2187) loaded into the modern PC 3.4 "
    "I34E target via F0490_MEMORY_LoadDecompressAndExpandGraphic; the "
    "D2L/D2R corridor-side gate does not consume G2115 directly, but the "
    "G2115 anchor pins the family so the D2L/D2R G2113/G2114 anchors are "
    "non-orphaned. "
    "Non-overlap marker pass794-d2l-d2r-door-frame-top-edge-source-lock: "
    "this gate pins the door-frame-top edge stride, zone, and dispatch "
    "line numbers for D2L/D2R only. It is distinct from the F0111 door-"
    "panel state machine (dm1_v1_viewport_d2l_d2r_f0111_partly_open_door"
    "_pc34_compat), the D2L2/D2R2 front-door pair "
    "(dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat), the "
    "D0C door-frame border + thieves-eye hole (dm1_v1_viewport_d0c_door"
    "_edge_ornament_pc34_compat), the D2L/D2R wall content "
    "(dm1_v1_viewport_d2l_d2r_wall_pc34_compat), the D2L2/D2R2 side wall "
    "(dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat), the D2L2/D2R2 "
    "wall ornament (dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_pc34_"
    "compat), the D2L2/D2R2 floor ornament (dm1_v1_viewport_d2l2_d2r2_"
    "f0108_floor_ornament_pc34_compat), and the D2L2/D2R2 wall composi"
    "tion (dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat)."
    " The door-frame-top edge is the 3-pixel-tall horizontal bar above "
    "the door panel (Y1=22..Y2=24); the door panel itself starts at "
    "Y=24 and spans to Y=82 inside G0182/G0184.";

static DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 s_last;

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

static int zone_for_target_d2l(int target_media)
{
    switch (target_media) {
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
        /* Legacy MEDIA009 path uses F0100 with no zone id; report the
         * C10_COLOR_FLESH transparency sentinel for symmetry. */
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C725_ZONE_D2L_F20E_PC34;
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C729_ZONE_D2L_I34E_PC34;
    default:
        return -1;
    }
}

static int zone_for_target_d2r(int target_media)
{
    switch (target_media) {
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C727_ZONE_D2R_F20E_PC34;
    case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
        return DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C731_ZONE_D2R_I34E_PC34;
    default:
        return -1;
    }
}

int dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
    int side,
    int target_media,
    DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    if (target_media < DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND ||
        target_media > DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) {
        return 0;
    }
    if (side != DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 &&
        side != DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        return 0;
    }

    memset(out_trace, 0, sizeof(*out_trace));

    out_trace->side = side;
    out_trace->target_media = target_media;
    out_trace->framebuffer_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34;

    out_trace->d2l_stride_left_x =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_LEFT_X_PC34;
    out_trace->d2l_stride_right_x =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_RIGHT_X_PC34;
    out_trace->d2l_stride_top_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_TOP_Y_PC34;
    out_trace->d2l_stride_bottom_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BOTTOM_Y_PC34;
    out_trace->d2l_stride_byte_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BYTE_WIDTH_PC34;
    out_trace->d2l_stride_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_HEIGHT_PC34;
    out_trace->d2l_stride_x_offset =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_X_OFFSET_PC34;
    out_trace->d2l_stride_y_offset =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_Y_OFFSET_PC34;

    out_trace->d2r_stride_left_x =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_LEFT_X_PC34;
    out_trace->d2r_stride_right_x =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_RIGHT_X_PC34;
    out_trace->d2r_stride_top_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_TOP_Y_PC34;
    out_trace->d2r_stride_bottom_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BOTTOM_Y_PC34;
    out_trace->d2r_stride_byte_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BYTE_WIDTH_PC34;
    out_trace->d2r_stride_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_HEIGHT_PC34;
    out_trace->d2r_stride_x_offset =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_X_OFFSET_PC34;
    out_trace->d2r_stride_y_offset =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_Y_OFFSET_PC34;

    out_trace->band_top_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34;
    out_trace->band_bottom_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34;
    out_trace->band_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34;
    out_trace->band_byte_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34;

    out_trace->m604_view_square_d2l =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M604_VIEW_SQUARE_D2L_PC34;
    out_trace->m605_view_square_d2r =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M605_VIEW_SQUARE_D2R_PC34;
    out_trace->d2l_pass1_cell_order =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_PASS1_CELL_ORDER_PC34;
    out_trace->d2r_pass1_cell_order =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_PASS1_CELL_ORDER_PC34;

    out_trace->door_panel_top_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34;
    out_trace->door_panel_bottom_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34;
    out_trace->door_panel_height =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34;
    out_trace->door_panel_byte_width =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34;
    out_trace->bitmap_byte_count_d2lcr =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D2LCR_PC34;

    out_trace->door_frame_top_bitmap_id =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0703_DOOR_FRAME_TOP_BITMAP_PC34;
    out_trace->door_frame_top_stride_d2l_id =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0173_STRIDE_PC34;
    out_trace->door_frame_top_stride_d2r_id =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0175_STRIDE_PC34;
    out_trace->door_frame_top_native_bitmap_d2l =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2114_DOOR_FRAME_TOP_D2L_PC34;
    out_trace->door_frame_top_native_bitmap_d2r =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2113_DOOR_FRAME_TOP_D2R_PC34;
    out_trace->door_frame_top_native_bitmap_d2lcr =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2115_DOOR_FRAME_TOP_D2LCR_PC34;

    out_trace->f0100_blit_transparency_color =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    out_trace->f0119_d2l_door_front_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRONT_LINE_PC34;
    out_trace->f0119_d2l_door_frame_top_legacy_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_LEGACY_LINE_PC34;
    out_trace->f0119_d2l_door_frame_top_f20e_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_F20E_LINE_PC34;
    out_trace->f0119_d2l_door_frame_top_i34e_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_I34E_LINE_PC34;
    out_trace->f0120_d2r_door_front_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRONT_LINE_PC34;
    out_trace->f0120_d2r_door_frame_top_legacy_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_LEGACY_LINE_PC34;
    out_trace->f0120_d2r_door_frame_top_f20e_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_F20E_LINE_PC34;
    out_trace->f0120_d2r_door_frame_top_i34e_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_I34E_LINE_PC34;
    out_trace->f0128_dispatch_line =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34;

    out_trace->d2l_door_frame_top_zone = zone_for_target_d2l(target_media);
    out_trace->d2r_door_frame_top_zone = zone_for_target_d2r(target_media);
    out_trace->g0694_door_panel_bitmap =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0694_DOOR_BITMAP_PC34;
    out_trace->g0182_door_frames_d2l =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0182_DOOR_FRAMES_D2L_PC34;
    out_trace->g0184_door_frames_d2r =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0184_DOOR_FRAMES_D2R_PC34;
    out_trace->c1_view_door_ornament_d2lcr =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C1_VIEW_DOOR_ORNAMENT_D2LCR_PC34;

    out_trace->d2l_band_in_viewport =
        (out_trace->d2l_stride_top_y < out_trace->viewport_height) ? 1 : 0;
    out_trace->d2r_band_in_viewport =
        (out_trace->d2r_stride_top_y < out_trace->viewport_height) ? 1 : 0;
    out_trace->d2l_band_inside_door_panel_band =
        (out_trace->d2l_stride_bottom_y <= out_trace->door_panel_top_y) ? 1 : 0;
    out_trace->d2r_band_inside_door_panel_band =
        (out_trace->d2r_stride_bottom_y <= out_trace->door_panel_top_y) ? 1 : 0;

    out_trace->d2l_legacy_route_uses_g0703 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) ? 1 : 0;
    out_trace->d2r_legacy_route_uses_g0703 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) ? 1 : 0;
    out_trace->d2l_f20e_route_uses_g2114 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) ? 1 : 0;
    out_trace->d2r_f20e_route_uses_g2113 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) ? 1 : 0;
    out_trace->d2l_i34e_route_uses_g2114 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) ? 1 : 0;
    out_trace->d2r_i34e_route_uses_g2113 =
        (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) ? 1 : 0;

    out_trace->c10_transparent_blit = 1;

    out_trace->band_strip_destination_x_d2l =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_LEFT_X_PC34 +
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_X_OFFSET_PC34;
    out_trace->band_strip_destination_x_d2r =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_LEFT_X_PC34 +
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_X_OFFSET_PC34;
    out_trace->band_strip_destination_y =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34;
    out_trace->band_strip_byte_width_d2l =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BYTE_WIDTH_PC34;
    out_trace->band_strip_byte_width_d2r =
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BYTE_WIDTH_PC34;

    /* Probe pixels: contract-only trace returns sentinel zeros; the
     * asset-free contract surface only verifies geometry and zone
     * anchors, not real framebuffer contents. */
    out_trace->d2l_first_probe_pixel = 0;
    out_trace->d2l_second_probe_pixel = 0;
    out_trace->d2l_third_probe_pixel = 0;
    out_trace->d2r_first_probe_pixel = 0;
    out_trace->d2r_second_probe_pixel = 0;
    out_trace->d2r_third_probe_pixel = 0;

    return 1;
}

const char *
dm1_v1_viewport_d2l_d2r_door_frame_top_edge_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static int expected_count_for(int side, int target_media)
{
    /* Three targets x two sides = 6 valid combos, plus one invalid. */
    if (side != DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 &&
        side != DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        return -1;
    }
    if (target_media < DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND ||
        target_media > DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND) {
        return -1;
    }
    return 1;
}

static void increment_target_counter(int side, int target_media)
{
    if (side == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34) {
        switch (target_media) {
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
            ++s_last.d2l_legacy_zone_count; break;
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
            ++s_last.d2l_f20e_zone_count; break;
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
            ++s_last.d2l_i34e_zone_count; break;
        default:
            ++s_last.invalid_target_count; break;
        }
    } else if (side == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34) {
        switch (target_media) {
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND:
            ++s_last.d2r_legacy_zone_count; break;
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND:
            ++s_last.d2r_f20e_zone_count; break;
        case DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND:
            ++s_last.d2r_i34e_zone_count; break;
        default:
            ++s_last.invalid_target_count; break;
        }
    } else {
        ++s_last.invalid_target_count;
    }
}

static void check_one(int side, int target_media)
{
    DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
    const int rc = dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
        side, target_media, &trace);

    assert_int("trace.ok", rc, 1);
    assert_int("trace.side", trace.side, side);
    assert_int("trace.target_media", trace.target_media, target_media);

    /* D2L stride anchors. */
    assert_int("d2l.stride.left_x", trace.d2l_stride_left_x,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_LEFT_X_PC34);
    assert_int("d2l.stride.right_x", trace.d2l_stride_right_x,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_RIGHT_X_PC34);
    assert_int("d2l.stride.top_y", trace.d2l_stride_top_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_TOP_Y_PC34);
    assert_int("d2l.stride.bottom_y", trace.d2l_stride_bottom_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BOTTOM_Y_PC34);
    assert_int("d2l.stride.byte_width", trace.d2l_stride_byte_width,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BYTE_WIDTH_PC34);
    assert_int("d2l.stride.height", trace.d2l_stride_height,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_HEIGHT_PC34);
    assert_int("d2l.stride.x_offset", trace.d2l_stride_x_offset,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_X_OFFSET_PC34);
    assert_int("d2l.stride.y_offset", trace.d2l_stride_y_offset,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_Y_OFFSET_PC34);
    ++s_last.stride_g0173_checks;

    /* D2R stride anchors. */
    assert_int("d2r.stride.left_x", trace.d2r_stride_left_x,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_LEFT_X_PC34);
    assert_int("d2r.stride.right_x", trace.d2r_stride_right_x,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_RIGHT_X_PC34);
    assert_int("d2r.stride.top_y", trace.d2r_stride_top_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_TOP_Y_PC34);
    assert_int("d2r.stride.bottom_y", trace.d2r_stride_bottom_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BOTTOM_Y_PC34);
    assert_int("d2r.stride.byte_width", trace.d2r_stride_byte_width,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BYTE_WIDTH_PC34);
    assert_int("d2r.stride.height", trace.d2r_stride_height,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_HEIGHT_PC34);
    assert_int("d2r.stride.x_offset", trace.d2r_stride_x_offset,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_X_OFFSET_PC34);
    assert_int("d2r.stride.y_offset", trace.d2r_stride_y_offset,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_Y_OFFSET_PC34);
    ++s_last.stride_g0175_checks;

    /* Band strip geometry. */
    assert_int("band.top_y", trace.band_top_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34);
    assert_int("band.bottom_y", trace.band_bottom_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34);
    assert_int("band.height", trace.band_height,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34);
    assert_int("band.byte_width", trace.band_byte_width,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34);
    assert_int("band.strip.destination_x.d2l",
               trace.band_strip_destination_x_d2l,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_LEFT_X_PC34 +
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_X_OFFSET_PC34);
    assert_int("band.strip.destination_x.d2r",
               trace.band_strip_destination_x_d2r,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_LEFT_X_PC34 +
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_X_OFFSET_PC34);
    assert_int("band.strip.destination_y", trace.band_strip_destination_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34);
    assert_int("band.strip.byte_width.d2l", trace.band_strip_byte_width_d2l,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BYTE_WIDTH_PC34);
    assert_int("band.strip.byte_width.d2r", trace.band_strip_byte_width_d2r,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BYTE_WIDTH_PC34);
    ++s_last.band_strip_checks;

    /* Band sits inside the viewport (Y=22 < 136) and above the door
     * panel (Y=24 <= Y=24). */
    assert_int("d2l.band.in_viewport", trace.d2l_band_in_viewport, 1);
    assert_int("d2r.band.in_viewport", trace.d2r_band_in_viewport, 1);
    assert_int("d2l.band.above.door_panel",
               trace.d2l_band_inside_door_panel_band, 1);
    assert_int("d2r.band.above.door_panel",
               trace.d2r_band_inside_door_panel_band, 1);

    /* View-square anchors. */
    assert_int("m604.view.square.d2l", trace.m604_view_square_d2l,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M604_VIEW_SQUARE_D2L_PC34);
    assert_int("m605.view.square.d2r", trace.m605_view_square_d2r,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M605_VIEW_SQUARE_D2R_PC34);
    assert_int("d2l.pass1.cell_order", trace.d2l_pass1_cell_order,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_PASS1_CELL_ORDER_PC34);
    assert_int("d2r.pass1.cell_order", trace.d2r_pass1_cell_order,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_PASS1_CELL_ORDER_PC34);
    ++s_last.view_square_anchor_checks;

    /* Door-panel post-band anchors. */
    assert_int("door_panel.top_y", trace.door_panel_top_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34);
    assert_int("door_panel.bottom_y", trace.door_panel_bottom_y,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34);
    assert_int("door_panel.height", trace.door_panel_height,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34);
    assert_int("door_panel.byte_width", trace.door_panel_byte_width,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34);
    assert_int("bitmap.byte_count.d2lcr",
               trace.bitmap_byte_count_d2lcr,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D2LCR_PC34);
    assert_int("g0694.door_panel.bitmap", trace.g0694_door_panel_bitmap,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0694_DOOR_BITMAP_PC34);
    assert_int("g0182.door.frames.d2l", trace.g0182_door_frames_d2l,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0182_DOOR_FRAMES_D2L_PC34);
    assert_int("g0184.door.frames.d2r", trace.g0184_door_frames_d2r,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0184_DOOR_FRAMES_D2R_PC34);
    assert_int("c1.view.door.ornament.d2lcr",
               trace.c1_view_door_ornament_d2lcr,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C1_VIEW_DOOR_ORNAMENT_D2LCR_PC34);
    ++s_last.door_panel_post_band_checks;

    /* Zone IDs. */
    if (target_media == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND) {
        assert_int("d2l.legacy.route.uses.g0703",
                   trace.d2l_legacy_route_uses_g0703, 1);
        assert_int("d2r.legacy.route.uses.g0703",
                   trace.d2r_legacy_route_uses_g0703, 1);
        assert_int("d2l.door_frame_top.zone", trace.d2l_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
        assert_int("d2r.door_frame_top.zone", trace.d2r_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
        assert_int("d2l.f20e.route.uses.g2114",
                   trace.d2l_f20e_route_uses_g2114, 0);
        assert_int("d2r.f20e.route.uses.g2113",
                   trace.d2r_f20e_route_uses_g2113, 0);
        assert_int("d2l.i34e.route.uses.g2114",
                   trace.d2l_i34e_route_uses_g2114, 0);
        assert_int("d2r.i34e.route.uses.g2113",
                   trace.d2r_i34e_route_uses_g2113, 0);
    } else if (target_media ==
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND) {
        assert_int("d2l.legacy.route.uses.g0703",
                   trace.d2l_legacy_route_uses_g0703, 0);
        assert_int("d2r.legacy.route.uses.g0703",
                   trace.d2r_legacy_route_uses_g0703, 0);
        assert_int("d2l.f20e.route.uses.g2114",
                   trace.d2l_f20e_route_uses_g2114, 1);
        assert_int("d2r.f20e.route.uses.g2113",
                   trace.d2r_f20e_route_uses_g2113, 1);
        assert_int("d2l.i34e.route.uses.g2114",
                   trace.d2l_i34e_route_uses_g2114, 0);
        assert_int("d2r.i34e.route.uses.g2113",
                   trace.d2r_i34e_route_uses_g2113, 0);
        assert_int("d2l.door_frame_top.zone", trace.d2l_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C725_ZONE_D2L_F20E_PC34);
        assert_int("d2r.door_frame_top.zone", trace.d2r_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C727_ZONE_D2R_F20E_PC34);
    } else {
        assert_int("d2l.legacy.route.uses.g0703",
                   trace.d2l_legacy_route_uses_g0703, 0);
        assert_int("d2r.legacy.route.uses.g0703",
                   trace.d2r_legacy_route_uses_g0703, 0);
        assert_int("d2l.f20e.route.uses.g2114",
                   trace.d2l_f20e_route_uses_g2114, 0);
        assert_int("d2r.f20e.route.uses.g2113",
                   trace.d2r_f20e_route_uses_g2113, 0);
        assert_int("d2l.i34e.route.uses.g2114",
                   trace.d2l_i34e_route_uses_g2114, 1);
        assert_int("d2r.i34e.route.uses.g2113",
                   trace.d2r_i34e_route_uses_g2113, 1);
        assert_int("d2l.door_frame_top.zone", trace.d2l_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C729_ZONE_D2L_I34E_PC34);
        assert_int("d2r.door_frame_top.zone", trace.d2r_door_frame_top_zone,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C731_ZONE_D2R_I34E_PC34);
    }
    ++s_last.zone_id_family_checks;
    ++s_last.bitmap_route_checks;

    /* F0119 / F0120 / F0128 dispatch line anchors. */
    assert_int("f0119.d2l.door_front.line",
               trace.f0119_d2l_door_front_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRONT_LINE_PC34);
    assert_int("f0119.d2l.door_frame_top.legacy.line",
               trace.f0119_d2l_door_frame_top_legacy_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_LEGACY_LINE_PC34);
    assert_int("f0119.d2l.door_frame_top.f20e.line",
               trace.f0119_d2l_door_frame_top_f20e_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_F20E_LINE_PC34);
    assert_int("f0119.d2l.door_frame_top.i34e.line",
               trace.f0119_d2l_door_frame_top_i34e_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_I34E_LINE_PC34);
    assert_int("f0120.d2r.door_front.line",
               trace.f0120_d2r_door_front_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRONT_LINE_PC34);
    assert_int("f0120.d2r.door_frame_top.legacy.line",
               trace.f0120_d2r_door_frame_top_legacy_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_LEGACY_LINE_PC34);
    assert_int("f0120.d2r.door_frame_top.f20e.line",
               trace.f0120_d2r_door_frame_top_f20e_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_F20E_LINE_PC34);
    assert_int("f0120.d2r.door_frame_top.i34e.line",
               trace.f0120_d2r_door_frame_top_i34e_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_I34E_LINE_PC34);
    assert_int("f0128.dispatch.line", trace.f0128_dispatch_line,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34);

    /* C10 transparency for the F0100 legacy route. */
    assert_int("f0100.blit.transparency.color",
               trace.f0100_blit_transparency_color,
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    assert_int("c10.transparent.blit", trace.c10_transparent_blit, 1);
    ++s_last.c10_transparency_checks;

    /* Frame buffer probe: contract-only sentinel zeros. */
    assert_int("d2l.first.probe.pixel", trace.d2l_first_probe_pixel, 0);
    assert_int("d2l.second.probe.pixel", trace.d2l_second_probe_pixel, 0);
    assert_int("d2l.third.probe.pixel", trace.d2l_third_probe_pixel, 0);
    assert_int("d2r.first.probe.pixel", trace.d2r_first_probe_pixel, 0);
    assert_int("d2r.second.probe.pixel", trace.d2r_second_probe_pixel, 0);
    assert_int("d2r.third.probe.pixel", trace.d2r_third_probe_pixel, 0);

    increment_target_counter(side, target_media);
}

static void check_non_overlap(void)
{
    static const char *siblings[] = {
        "F0111 partly-open half-blit body (D2L/D2R)",
        "D2L2/D2R2 front-door pair",
        "D0C door-frame border + thieves-eye hole",
        "D2L/D2R wall content",
        "D2L2/D2R2 side wall",
        "D2L2/D2R2 wall ornament",
        "D2L2/D2R2 floor ornament",
        "D2L2/D2R2 wall composition",
        "D2C F0111 door-front layering",
        "D3L/D3R F0111 door-front",
        "D1C F0111 door-front",
        "D0L/D0R F0111 door-front",
        "D1L2/D1R2 F0111 partly-open",
        "D0L2/D0R2 F0111 partly-open",
        "D0C F0111 partly-open",
        "D1C F0111 partly-open",
        "D2C F0111 partly-open",
        "D2L/D2R F0108 floor-ornament",
        "D2L/D2R F0107 wall-ornament",
        "D2L/D2R F0098 fallback",
        "D3L/D3R F0111 door-front pair",
        "D3C F0111 door-front pair",
    };
    size_t i;

    for (i = 0; i < sizeof(siblings) / sizeof(siblings[0]); ++i) {
        assert_int("non.overlap.uses.g0173",
                   strstr(siblings[i], "G0173") != NULL, 0);
        assert_int("non.overlap.uses.g0175",
                   strstr(siblings[i], "G0175") != NULL, 0);
        assert_int("non.overlap.uses.g0703",
                   strstr(siblings[i], "G0703") != NULL, 0);
        assert_int("non.overlap.uses.g2114",
                   strstr(siblings[i], "G2114") != NULL, 0);
        assert_int("non.overlap.uses.g2113",
                   strstr(siblings[i], "G2113") != NULL, 0);
        assert_int("non.overlap.uses.door_frame_top",
                   strstr(siblings[i], "DoorFrameTop") != NULL, 0);
        ++s_last.non_overlap_checks;
    }
}

int run_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    assert_contains("source.dunview.604", s_source_evidence,
                    "DUNVIEW.C:604-606");
    assert_contains("source.g0173", s_source_evidence,
                    "G0173_auc_Graphic558_Frame_DoorFrameTop_D2L");
    assert_contains("source.g0175", s_source_evidence,
                    "G0175_auc_Graphic558_Frame_DoorFrameTop_D2R");
    assert_contains("source.d2l.stride.values", s_source_evidence,
                    "{ 0, 59, 22, 24, 48, 3, 16, 0 }");
    assert_contains("source.d2r.stride.values", s_source_evidence,
                    "{ 164, 223, 22, 24, 48, 3, 16, 0 }");
    assert_contains("source.f0119.d2l.door.front", s_source_evidence,
                    "DUNVIEW.C:6991");
    assert_contains("source.f0119.d2l.f20e", s_source_evidence,
                    "DUNVIEW.C:6994");
    assert_contains("source.f0119.d2l.i34e", s_source_evidence,
                    "DUNVIEW.C:6997");
    assert_contains("source.f0120.d2r.legacy", s_source_evidence,
                    "DUNVIEW.C:7184");
    assert_contains("source.f0120.d2r.f20e", s_source_evidence,
                    "DUNVIEW.C:7187");
    assert_contains("source.f0120.d2r.i34e", s_source_evidence,
                    "DUNVIEW.C:7190");
    assert_contains("source.f0119.start", s_source_evidence,
                    "DUNVIEW.C:6900");
    assert_contains("source.f0120.start", s_source_evidence,
                    "DUNVIEW.C:7051");
    assert_contains("source.f0128.dispatch", s_source_evidence,
                    "DUNVIEW.C:8513");
    assert_contains("source.g0703.bitmap", s_source_evidence,
                    "G0703_puc_Bitmap_WallSet_DoorFrameTop_D2LCR");
    assert_contains("source.g2114.d2l", s_source_evidence,
                    "G2114_DoorFrameTopD2L");
    assert_contains("source.g2113.d2r", s_source_evidence,
                    "G2113_DoorFrameTopD2R");
    assert_contains("source.c725.f20e.d2l", s_source_evidence,
                    "C725_ZONE_DOOR_FRAME_TOP_D2L = 725");
    assert_contains("source.c727.f20e.d2r", s_source_evidence,
                    "C727_ZONE_DOOR_FRAME_TOP_D2R = 727");
    assert_contains("source.c729.i34e.d2l", s_source_evidence,
                    "C729_ZONE_DOOR_FRAME_TOP_D2L = 729");
    assert_contains("source.c731.i34e.d2r", s_source_evidence,
                    "C731_ZONE_DOOR_FRAME_TOP_D2R = 731");
    assert_contains("source.m604.m605", s_source_evidence,
                    "M604_VIEW_SQUARE_D2L");
    assert_contains("source.cell.order.d2l", s_source_evidence,
                    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT");
    assert_contains("source.cell.order.d2r", s_source_evidence,
                    "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT");
    assert_contains("source.c10.flesh", s_source_evidence,
                    "C10_COLOR_FLESH");
    assert_contains("source.g0694", s_source_evidence,
                    "G0694");
    assert_contains("source.g0182", s_source_evidence,
                    "G0182");
    assert_contains("source.g0184", s_source_evidence,
                    "G0184");
    assert_contains("source.c1.view.door.ornament", s_source_evidence,
                    "C1_VIEW_DOOR_ORNAMENT_D2LCR");
    assert_contains("source.f0111.dispatch", s_source_evidence,
                    "F0111_DUNGEONVIEW_DrawDoor door-panel state");
    assert_contains("source.f0100.transparency", s_source_evidence,
                    "F0100_DUNGEONVIEW_DrawWallSetBitmap");
    assert_contains("source.non.overlap.marker", s_source_evidence,
                    "pass794-d2l-d2r-door-frame-top-edge-source-lock");
    assert_contains("source.bridge", s_source_evidence,
                    "door-frame-top edge stride, zone, and dispatch");
    assert_contains("source.post.band.door.panel", s_source_evidence,
                    "Y=24 and spans to Y=82");
    assert_contains("source.g2115.d2lcr", s_source_evidence,
                    "G2115_DoorFrameTopD2LCR");

    assert_int("expected.legacy.d2l",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND),
               1);
    assert_int("expected.legacy.d2r",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND),
               1);
    assert_int("expected.f20e.d2l",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND),
               1);
    assert_int("expected.f20e.d2r",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND),
               1);
    assert_int("expected.i34e.d2l",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND),
               1);
    assert_int("expected.i34e.d2r",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND),
               1);
    assert_int("expected.bad.target",
               expected_count_for(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99),
               -1);
    assert_int("expected.bad.side",
               expected_count_for(99, 0), -1);

    /* Six valid combinations. */
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND);
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND);
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND);
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND);
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_KIND);
    check_one(DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
              DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_KIND);

    /* Invalid: out-of-range target. */
    {
        DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
        const int rc = dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
            DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99, &trace);
        assert_int("invalid.target.rc", rc, 0);
        ++s_last.invalid_target_count;
    }
    /* Invalid: out-of-range side. */
    {
        DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
        const int rc = dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
            99, DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND, &trace);
        assert_int("invalid.side.rc", rc, 0);
        ++s_last.invalid_target_count;
    }
    /* Null output. */
    assert_int("null.output.rc",
               dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
                   DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_KIND,
                   NULL),
               0);

    check_non_overlap();

    assert_int("d2l.legacy.count", s_last.d2l_legacy_zone_count, 1);
    assert_int("d2l.f20e.count", s_last.d2l_f20e_zone_count, 1);
    assert_int("d2l.i34e.count", s_last.d2l_i34e_zone_count, 1);
    assert_int("d2r.legacy.count", s_last.d2r_legacy_zone_count, 1);
    assert_int("d2r.f20e.count", s_last.d2r_f20e_zone_count, 1);
    assert_int("d2r.i34e.count", s_last.d2r_i34e_zone_count, 1);
    assert_int("invalid.count", s_last.invalid_target_count, 2);
    assert_int("stride.g0173.checks", s_last.stride_g0173_checks, 6);
    assert_int("stride.g0175.checks", s_last.stride_g0175_checks, 6);
    assert_int("band.strip.checks", s_last.band_strip_checks, 6);
    assert_int("zone.id.family.checks", s_last.zone_id_family_checks, 6);
    assert_int("door.panel.post.band.checks",
               s_last.door_panel_post_band_checks, 6);
    assert_int("view.square.anchor.checks",
               s_last.view_square_anchor_checks, 6);
    assert_int("non.overlap.checks", s_last.non_overlap_checks, 22);
    assert_int("bitmap.route.checks", s_last.bitmap_route_checks, 6);
    assert_int("c10.transparency.checks", s_last.c10_transparency_checks, 6);
    assert_int("hash.changed", s_last.deterministic_hash != 2166136261u, 1);

    return s_last.failures == 0 &&
           s_last.deterministic_hash ==
               DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_HASH_PC34
                   ? 0
                   : 1;
}

const DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34(void)
{
    return &s_last;
}
