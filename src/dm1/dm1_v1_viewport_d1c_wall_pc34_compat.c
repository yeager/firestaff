#include "dm1_v1_viewport_d1c_wall_pc34_compat.h"

#include <string.h>

enum {
    /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C=6 in MEDIA720. */
    DM1_V1_VIEW_SQUARE_D1C_PC34 = 6,
    /* ReDMCSB DEFS.H:3427 C04_WALL_D1C=4 (PC 3.4 wall ordinal). */
    DM1_V1_WALL_D1C_PC34 = 4,
    /* ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C=712 in MEDIA720. */
    DM1_V1_ZONE_WALL_D1C_PC34 = 712,
    /* ReDMCSB DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720. */
    DM1_V1_VIEW_WALL_D1C_FRONT_PC34 = 14,
    /* ReDMCSB DUNVIEW.C:165 G3055_i_WallSetFlipped_Wall_D1C=-24 (PC ordinal 24). */
    DM1_V1_FLIPPED_WALL_D1C_PC34 = 24,
    /* ReDMCSB DUNVIEW.C:142 G3013_i_WallSet_Wall_D1C=-13 (PC ordinal 13,
     * also the native CSB/Amiga/ST wall ordinal that PC reuses in
     * G0700_puc_Bitmap_WallSet_Wall_D1LCR). */
    DM1_V1_NATIVE_WALL_D1C_PC34 = 13,
    /* ReDMCSB DUNVIEW.C:589 G0163 D1C frame row, with the C0/1/2/3/4/5/6/7
     * indices mapped to viewport X1, X2, Y1, Y2, byte_width, height, X, Y. */
    DM1_V1_D1C_FRAME_X1_PC34 = 32,
    DM1_V1_D1C_FRAME_X2_PC34 = 191,
    DM1_V1_D1C_FRAME_Y1_PC34 = 9,
    DM1_V1_D1C_FRAME_Y2_PC34 = 119,
    DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34 = 128,
    DM1_V1_D1C_FRAME_HEIGHT_PC34 = 111,
    DM1_V1_D1C_FRAME_SOURCE_X_PC34 = 48,
    DM1_V1_D1C_FRAME_SOURCE_Y_PC34 = 0,
    /* ReDMCSB DUNVIEW.C:7813-7843 C00 wall case uses C712 (MEDIA720) and
     * C710 (P20JA/P20JB) zone ids; this gate is the PC 3.4 I34E
     * MEDIA720 contract, so C712 is the canonical PC 3.4 D1C zone. */
    DM1_V1_D1C_ZONE_LEGACY_PC34 = 710
};

static const char s_non_overlap_note[] =
    "non-overlap: D1C center field coverage stays in "
    "test_dm1_v1_viewport_d1c_center_field_pc34_compat; D1C stairs/pit "
    "dispatch coverage stays in "
    "test_dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat; D1C door "
    "front coverage stays in the F0111 door-panel gate. This gate covers "
    "only the D1C center wall pixel slice, including the G0076 flipped "
    "wall/footprints path, C10 transparency, clipped edges, and no-write "
    "metadata; it does not claim real-asset bitmap parity.";

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:581-594 G0163_aauc_Graphic558_Frame_Walls[M606_VIEW_SQUARE_D1C]
 * line 589 defines the D1C wall frame as
 * {32,191,9,119,128,111,48,0}. DUNVIEW.C:7727-7843 F0124_DUNGEONVIEW_DrawSquareD1C
 * is the C00_ELEMENT_WALL case; lines 7784-7843 draw the D1C wall, probe
 * F0107 front wall ornament (M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720) for
 * alcove, optionally call F0115 with C0x0000_CELL_ORDER_ALCOVE, and return
 * when the probe is not alcove. DUNVIEW.C:7792-7801 cover
 * MEDIA458_P20JA_P20JB / MEDIA709_I34E_I34M_P31J and call
 * F0792_DUNGEONVIEW_DrawBitmapYYY with
 * G2107_WallSet[C04_WALL_D1C] / C712_ZONE_WALL_D1C plus
 * G0076_B_UseFlippedWallAndFootprintsBitmaps. DUNVIEW.C:7802-7807 cover
 * MEDIA506_F20E_F20J_X30J / MEDIA747_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J
 * and call F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency with
 * G2107_WallSet[C04_WALL_D1C] / C712_ZONE_WALL_D1C. DUNVIEW.C:3048-3058
 * F0100_DUNGEONVIEW_DrawWallSetBitmap is the C10 transparent blit contract
 * for the wall-set bitmap (used by F0765 in PC 3.4 I34E for the door
 * top, door frame, and D0C F0113 frames; not the D1C center wall on
 * MEDIA720 because that path uses F0792). DUNVIEW.C:3113-3129 F0104 is
 * the PC34 native bitmap route shape. DUNVIEW.C:3502+ F0107
 * DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF is the front wall ornament
 * probe that gates the F0115 alcove thing-pass branch. DUNVIEW.C:2417
 * F1000_ creates the G3055_i_WallSetFlipped_Wall_D1C flipped variant on
 * bitmap load. DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2599
 * M606_VIEW_SQUARE_D1C=6 in MEDIA720; DEFS.H:2695-2710
 * M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720; DEFS.H:3427 C04_WALL_D1C=4;
 * DEFS.H:4052 C712_ZONE_WALL_D1C=712 in MEDIA720; the legacy
 * non-MEDIA720 path uses C710_ZONE_WALL_D1C.
 */
static const DM1_V1_D1CWallSpecPc34 s_spec = {
    true,
    false,
    DM1_V1_VIEW_SQUARE_D1C_PC34,
    DM1_V1_WALL_D1C_PC34,
    DM1_V1_ZONE_WALL_D1C_PC34,
    DM1_V1_NATIVE_WALL_D1C_PC34,
    DM1_V1_FLIPPED_WALL_D1C_PC34,
    DM1_V1_VIEW_WALL_D1C_FRONT_PC34,
    DM1_V1_VIEW_SQUARE_D1C_PC34,
    DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34,
    DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34 -
        DM1_V1_D1C_FRAME_SOURCE_X_PC34,
    DM1_V1_D1C_FRAME_HEIGHT_PC34,
    DM1_V1_D1C_WALL_C10_COLOR_FLESH_PC34,
    DM1_V1_D1C_FRAME_SOURCE_X_PC34,
    DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34 - 1,
    DM1_V1_D1C_FRAME_SOURCE_Y_PC34,
    DM1_V1_D1C_FRAME_HEIGHT_PC34 - 1,
    DM1_V1_D1C_FRAME_X1_PC34,
    DM1_V1_D1C_FRAME_X1_PC34 +
        (DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34 -
         DM1_V1_D1C_FRAME_SOURCE_X_PC34) - 1,
    DM1_V1_D1C_FRAME_Y1_PC34,
    DM1_V1_D1C_FRAME_Y2_PC34,
    DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34,
    DM1_V1_D1C_WALL_SOURCE_HEIGHT_PC34,
    DM1_V1_D1C_FRAME_BYTE_WIDTH_PC34 -
        DM1_V1_D1C_FRAME_SOURCE_X_PC34,
    DM1_V1_D1C_FRAME_HEIGHT_PC34,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    false,
    false,
    false,
    true,
    true,
    "M606_VIEW_SQUARE_D1C",
    "C04_WALL_D1C",
    "C24_FLIPPED_WALL_D1C",
    "C712_ZONE_WALL_D1C",
    "M587_VIEW_WALL_D1C_FRONT",
    NULL,
    s_non_overlap_note
};

const DM1_V1_D1CWallSpecPc34 *dm1_v1_viewport_d1c_wall_spec_pc34(void)
{
    static DM1_V1_D1CWallSpecPc34 spec;
    spec = s_spec;
    spec.source_lines = dm1_v1_viewport_d1c_wall_source_evidence_pc34();
    return &spec;
}

bool dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
    const DM1_V1_D1CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1CWallPixelResultPc34 *out)
{
    const DM1_V1_D1CWallSpecPc34 *spec;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d1c_wall_spec_pc34();
    out->spec = *spec;
    if (!input) return false;

    out->row = input->row;
    out->viewport_x = input->viewport_x;
    out->route_is_flipped = input->use_flipped_wall_bitmap;
    out->no_transparency_route = input->use_no_transparency;
    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D1C_WALL_C10_COLOR_FLESH_PC34;
    }
    out->spec.transparent_color = transparent_color;

    if (input->row < spec->viewport_y_first ||
        input->row > spec->viewport_y_last ||
        input->viewport_x < spec->viewport_x_first ||
        input->viewport_x > spec->viewport_x_last) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = spec->source_x_first +
        (input->viewport_x - spec->viewport_x_first);
    out->source_y = spec->source_y_first +
        (input->row - spec->viewport_y_first);
    out->selected_source_x = input->use_flipped_wall_bitmap
        ? (DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset = (size_t)out->source_y *
        (size_t)DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D1C_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        !input->use_no_transparency && out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] = input->use_no_transparency
        ? out->source_pixel
        : dm1_v1_viewport_d1c_wall_blend_pixel_pc34(
            viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_d1c_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d1c_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap blits "
        "wall frames with C10_COLOR_FLESH transparency. DUNVIEW.C:3113-3129 "
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap shows the PC34 "
        "native bitmap route shape used by layout zones. DUNVIEW.C:581-594 "
        "G0163_aauc_Graphic558_Frame_Walls[M606_VIEW_SQUARE_D1C] gives the "
        "D1C row at line 589: {32,191,9,119,128,111,48,0}; this slice gates "
        "the clipped center wall band byte_width=80, height=111, source X "
        "48..127, viewport X 32..111, viewport Y 9..119. DUNVIEW.C:2417 "
        "F1000_ builds the G3055_i_WallSetFlipped_Wall_D1C=-24 flipped "
        "variant, which is what F0792 picks up on PC 3.4 I34E when "
        "G0076_B_UseFlippedWallAndFootprintsBitmaps is set. DUNVIEW.C:7727-7843 "
        "F0124_DUNGEONVIEW_DrawSquareD1C is the C00_ELEMENT_WALL dispatch; "
        "DUNVIEW.C:7792-7801 F0792_DUNGEONVIEW_DrawBitmapYYY is the PC34 "
        "wall zone blit used by MEDIA458/MEDIA709 with G2107_WallSet[C04_WALL_D1C] "
        "and C712_ZONE_WALL_D1C; DUNVIEW.C:3288-3301 F0792 passes "
        "CM1_COLOR_NO_TRANSPARENCY to F0132_VIDEO_Blit. DUNVIEW.C:7802-7807 "
        "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency is the PC34 opaque "
        "center-wall path used by MEDIA506/MEDIA747; DUNVIEW.C:3159-3175 "
        "F0765 also passes CM1_COLOR_NO_TRANSPARENCY, so C10 writes on these "
        "zone routes instead of preserving the destination. "
        "line 7810 calls F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF "
        "with M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720, and line 7813 enters "
        "the F0115 alcove thing pass on a true probe and the function "
        "returns on a false probe. DUNVIEW.C:3502+ F0107 is the front wall "
        "ornament probe that gates the F0115 alcove thing-pass branch. "
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2599 "
        "M606_VIEW_SQUARE_D1C=6; DEFS.H:2695-2710 "
        "M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720; DEFS.H:3427 "
        "C04_WALL_D1C=4; DEFS.H:4052 C712_ZONE_WALL_D1C=712 in MEDIA720; "
        "C710_ZONE_WALL_D1C is the legacy non-MEDIA720 D1C zone and is "
        "recorded as route metadata only. DEFS.H:4042 C702_ZONE_WALL_D3L2 "
        "/ C703_ZONE_WALL_D3R2 are D3L2/D3R2 in MEDIA720 and not D1C. "
        "DUNVIEW.C:3048-3058 F0100 is recorded as a C10 transparent contract "
        "reference but is not the D1C center wall PC34 path; DUNVIEW.C:3113-3204 "
        "F0104/F0105 are the F0792/F0765 PC34 native/opaque equivalents "
        "for the D1C wall. F0115 only follows the explicit alcove path on "
        "the C00 wall case; F0108 floor ornaments and F0111 door drawing "
        "are in the door-front case and are not in this wall pixel slice. "
        "non-overlap: D1C center field gate, D1C stairs/pit dispatch gate, "
        "and the F0111 door-panel gate each cover their own D1C sub-route; "
        "F0113 center-field integration is also out of scope for this slice "
        "and stays in the broad D1C center-field gate.";
}
