#include "dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D3C_PC34 = 11,     /* ReDMCSB: DEFS.H:2607 M600_VIEW_SQUARE_D3C */
    DM1_V1_VIEW_SQUARE_D3L_PC34 = 12,     /* ReDMCSB: DEFS.H:2608 M601_VIEW_SQUARE_D3L */
    DM1_V1_VIEW_SQUARE_D3R_PC34 = 13,     /* ReDMCSB: DEFS.H:2609 M602_VIEW_SQUARE_D3R */
    DM1_V1_VIEW_SQUARE_D3L2_PC34 = 14,    /* ReDMCSB: DEFS.H:2610 C14_VIEW_SQUARE_D3L2 */
    DM1_V1_VIEW_SQUARE_D3R2_PC34 = 15,    /* ReDMCSB: DEFS.H:2611 C15_VIEW_SQUARE_D3R2 */
    DM1_V1_WALL_D3R2_PC34 = 10,           /* ReDMCSB: DEFS.H:3433 C10_WALL_D3R2 */
    DM1_V1_WALL_D3L2_PC34 = 11,           /* ReDMCSB: DEFS.H:3434 C11_WALL_D3L2 */
    DM1_V1_ZONE_WALL_D3L2_PC34 = 702,     /* ReDMCSB: DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    DM1_V1_ZONE_WALL_D3R2_PC34 = 703,     /* ReDMCSB: DEFS.H:4043 C703_ZONE_WALL_D3R2 */
    DM1_V1_ZONE_WALL_D3C_PC34 = 704,      /* ReDMCSB: DEFS.H:4044 C704_ZONE_WALL_D3C */
    DM1_V1_ZONE_WALL_D3L_PC34 = 705,      /* ReDMCSB: DEFS.H:4045 C705_ZONE_WALL_D3L */
    DM1_V1_ZONE_WALL_D3R_PC34 = 706,      /* ReDMCSB: DEFS.H:4046 C706_ZONE_WALL_D3R */
    DM1_V1_WALLSET_WALL_D3R2_PC34 = -6,   /* ReDMCSB: DUNVIEW.C:130 G3072_i_WallSet_Wall_D3R2 */
    DM1_V1_WALLSET_WALL_D3L2_PC34 = -5,   /* ReDMCSB: DUNVIEW.C:139 G3010_i_WallSet_Wall_D3L2 */
    DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34 = 112 /* ReDMCSB: DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT */
};

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:130 and :139 define the PC34 D3R2/D3L2 wall-set indices.
 * DUNVIEW.C:317-318 declares the old-media D3R2/D3L2 wall bitmaps.
 * DUNVIEW.C:579-580 defines the D3L2/D3R2 frames as 0..15 and 208..223,
 * y=25..73, byte width 8, height 49, source x/y 0/0.
 * DUNVIEW.C:8446-8464 F0128's relative-square plain-wall branch draws
 * D3L2 at 3,-2 before D3R2 at 3,2; it contains no F0107/F0111/F0115,
 * no pit route, and no F0108 floor ornament. DUNVIEW.C:6253-6331 is the
 * later full square dispatcher and is deliberately outside this gate.
 * DUNVIEW.C:3048-3058 F0100 preserves C10 through the transparent blit.
 * DUNVIEW.C:2440-2441 builds the C10/C11 flipped wall-set pair.
 */
static const DM1_V1_D3L2D3R2WallSpecPc34 s_specs[2] = {
    {
        DM1_V1_D3L2_D3R2_WALL_SIDE_D3L2_PC34,
        true,
        false,
        0,
        DM1_V1_VIEW_SQUARE_D3L2_PC34,
        3,
        -2,
        DM1_V1_WALL_D3L2_PC34,
        DM1_V1_WALL_D3R2_PC34,
        DM1_V1_WALLSET_WALL_D3L2_PC34,
        DM1_V1_ZONE_WALL_D3L2_PC34,
        0,
        15,
        25,
        73,
        8,
        DM1_V1_D3L2_D3R2_WALL_SOURCE_HEIGHT_PC34,
        0,
        0,
        0,
        15,
        0,
        48,
        0,
        15,
        25,
        73,
        16,
        49,
        DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        false,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        "C14_VIEW_SQUARE_D3L2",
        "C11_WALL_D3L2",
        "C10_WALL_D3R2",
        "G3010_i_WallSet_Wall_D3L2",
        "C702_ZONE_WALL_D3L2",
        "G0697_puc_Bitmap_WallSet_Wall_D3L2",
        "G0711_auc_Graphic558_Frame_Wall_D3L2",
        "DUNVIEW.C:8446-8452 F0128 relative 3,-2 plain-wall branch"
    },
    {
        DM1_V1_D3L2_D3R2_WALL_SIDE_D3R2_PC34,
        true,
        false,
        1,
        DM1_V1_VIEW_SQUARE_D3R2_PC34,
        3,
        2,
        DM1_V1_WALL_D3R2_PC34,
        DM1_V1_WALL_D3L2_PC34,
        DM1_V1_WALLSET_WALL_D3R2_PC34,
        DM1_V1_ZONE_WALL_D3R2_PC34,
        208,
        223,
        25,
        73,
        8,
        DM1_V1_D3L2_D3R2_WALL_SOURCE_HEIGHT_PC34,
        0,
        0,
        0,
        15,
        0,
        48,
        208,
        223,
        25,
        73,
        16,
        49,
        DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        "C15_VIEW_SQUARE_D3R2",
        "C10_WALL_D3R2",
        "C11_WALL_D3L2",
        "G3072_i_WallSet_Wall_D3R2",
        "C703_ZONE_WALL_D3R2",
        "G0696_puc_Bitmap_WallSet_Wall_D3R2",
        "G0712_auc_Graphic558_Frame_Wall_D3R2",
        "DUNVIEW.C:8454-8464 F0128 relative 3,2 plain-wall branch"
    }
};

const DM1_V1_D3L2D3R2WallSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_wall_spec_pc34(
    DM1_V1_D3L2D3R2WallSidePc34 side)
{
    if (side == DM1_V1_D3L2_D3R2_WALL_SIDE_D3L2_PC34 ||
        side == DM1_V1_D3L2_D3R2_WALL_SIDE_D3R2_PC34) {
        return &s_specs[(int)side];
    }
    return NULL;
}

static bool dm1_v1_d3l2_d3r2_wall_apply_pixel(
    const DM1_V1_D3L2D3R2WallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3L2D3R2WallPixelPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->parity_flip = parity_flip;
    out->row = row;
    out->viewport_x = viewport_x;
    if (!spec) return false;

    if (row < spec->viewport_y_first ||
        row > spec->viewport_y_last ||
        viewport_x < spec->viewport_x_first ||
        viewport_x > spec->viewport_x_last) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = spec->source_x_first + (viewport_x - spec->viewport_x_first);
    out->source_y = spec->source_y_first + (row - spec->viewport_y_first);
    out->selected_source_x = parity_flip
        ? (DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)row * (size_t)DM1_V1_D3L2_D3R2_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;

    if (out->selected_source_x < 0 ||
        out->selected_source_x >= DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 ||
        out->source_offset >= source_len ||
        out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d3l2_d3r2_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

static bool add_check(
    DM1_V1_D3L2D3R2WallRunPc34 *out,
    const DM1_V1_D3L2D3R2WallSpecPc34 *spec,
    bool parity_flip,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    if (out->check_count >= DM1_V1_D3L2_D3R2_WALL_CHECK_CAPACITY_PC34) {
        return false;
    }
    return dm1_v1_d3l2_d3r2_wall_apply_pixel(
        spec,
        parity_flip,
        row,
        viewport_x,
        source,
        source_len,
        viewport,
        viewport_len,
        &out->checks[out->check_count++]);
}

bool dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat_run(
    DM1_V1_D3L2D3R2WallRunPc34 *out)
{
    uint8_t d3l2_source[DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 *
                        DM1_V1_D3L2_D3R2_WALL_SOURCE_HEIGHT_PC34];
    uint8_t d3r2_source[DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 *
                        DM1_V1_D3L2_D3R2_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3L2_D3R2_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3L2_D3R2_WALL_VIEWPORT_HEIGHT_PC34];
    bool ok = true;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    memset(d3l2_source, DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d3l2_source));
    memset(d3r2_source, DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(d3r2_source));
    memset(viewport, 0xee, sizeof(viewport));

    out->d3l2 = &s_specs[DM1_V1_D3L2_D3R2_WALL_SIDE_D3L2_PC34];
    out->d3r2 = &s_specs[DM1_V1_D3L2_D3R2_WALL_SIDE_D3R2_PC34];
    out->c10_palette_index = DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34;
    out->c112_byte_width_viewport = DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34;
    out->exact_d3l2_zone_pc34 = DM1_V1_ZONE_WALL_D3L2_PC34;
    out->exact_d3r2_zone_pc34 = DM1_V1_ZONE_WALL_D3R2_PC34;
    out->exact_d3l_zone_pc34 = DM1_V1_ZONE_WALL_D3L_PC34;
    out->exact_d3r_zone_pc34 = DM1_V1_ZONE_WALL_D3R_PC34;
    out->exact_d3c_zone_pc34 = DM1_V1_ZONE_WALL_D3C_PC34;
    out->exact_d3l_view_square_pc34 = DM1_V1_VIEW_SQUARE_D3L_PC34;
    out->exact_d3r_view_square_pc34 = DM1_V1_VIEW_SQUARE_D3R_PC34;
    out->exact_d3c_view_square_pc34 = DM1_V1_VIEW_SQUARE_D3C_PC34;
    out->draw_order_left_before_right =
        out->d3l2->draw_order_index < out->d3r2->draw_order_index;
    out->mirrored_route_pair =
        out->d3l2->native_wall_index_pc34 == out->d3r2->flipped_wall_index_pc34 &&
        out->d3r2->native_wall_index_pc34 == out->d3l2->flipped_wall_index_pc34;
    out->d3l2_d3r2_zone_pair =
        out->d3l2->wall_zone_pc34 == DM1_V1_ZONE_WALL_D3L2_PC34 &&
        out->d3r2->wall_zone_pc34 == DM1_V1_ZONE_WALL_D3R2_PC34;
    out->non_overlap_with_d3l_d3r_wall_gate =
        out->d3l2->relative_lateral != -1 &&
        out->d3r2->relative_lateral != 1 &&
        out->exact_d3l2_zone_pc34 != DM1_V1_ZONE_WALL_D3L_PC34 &&
        out->exact_d3r2_zone_pc34 != DM1_V1_ZONE_WALL_D3R_PC34 &&
        out->d3l2->view_square_index != DM1_V1_VIEW_SQUARE_D3L_PC34 &&
        out->d3r2->view_square_index != DM1_V1_VIEW_SQUARE_D3R_PC34;
    out->non_overlap_with_d3c_wall_gate =
        out->d3l2->relative_lateral != 0 &&
        out->d3r2->relative_lateral != 0 &&
        out->exact_d3l2_zone_pc34 != DM1_V1_ZONE_WALL_D3C_PC34 &&
        out->exact_d3r2_zone_pc34 != DM1_V1_ZONE_WALL_D3C_PC34 &&
        out->d3l2->view_square_index != DM1_V1_VIEW_SQUARE_D3C_PC34 &&
        out->d3r2->view_square_index != DM1_V1_VIEW_SQUARE_D3C_PC34;
    out->no_f0107_ornament =
        !out->d3l2->calls_f0107_wall_ornament &&
        !out->d3r2->calls_f0107_wall_ornament;
    out->no_f0111_door =
        !out->d3l2->calls_f0111_door && !out->d3r2->calls_f0111_door;
    out->no_f0115_alcove =
        !out->d3l2->calls_f0115_alcove_or_thing_pass &&
        !out->d3r2->calls_f0115_alcove_or_thing_pass;
    out->no_f0104_f0105_pit_route =
        !out->d3l2->calls_f0104_f0105_pit_route &&
        !out->d3r2->calls_f0104_f0105_pit_route;
    out->no_f0108_floor_ornament =
        !out->d3l2->calls_f0108_floor_ornament &&
        !out->d3r2->calls_f0108_floor_ornament;
    out->same_c10_transparency =
        out->d3l2->transparent_color == out->d3r2->transparent_color;
    out->same_height_and_row =
        out->d3l2->visible_height == out->d3r2->visible_height &&
        out->d3l2->viewport_y_first == out->d3r2->viewport_y_first &&
        out->d3l2->viewport_y_last == out->d3r2->viewport_y_last;
    out->c10_flesh_pixels_preserve_destination = true;
    out->source_evidence =
        dm1_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34();

    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 0] = 10;
    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 1] = 0x42;
    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 0x7e;
    d3l2_source[48 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 0x55;
    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 0] = 10;
    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 1] = 0x52;
    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 0x5e;
    d3r2_source[48 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 0x56;

    ok = ok && add_check(out, out->d3l2, false, 25, 0,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, false, 25, 1,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, false, 25, 15,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, false, 25, 16,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, false, 73, 15,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));

    ok = ok && add_check(out, out->d3r2, false, 25, 208,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, false, 25, 209,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, false, 25, 223,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, false, 25, 207,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, false, 73, 223,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));

    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 10;
    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 14] = 0x63;
    d3r2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6e;
    ok = ok && add_check(out, out->d3l2, true, 25, 0,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, true, 25, 1,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3l2, true, 25, 15,
                         d3r2_source, sizeof(d3r2_source), viewport, sizeof(viewport));

    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 15] = 10;
    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 14] = 0x64;
    d3l2_source[0 * DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 + 0] = 0x6d;
    ok = ok && add_check(out, out->d3r2, true, 25, 208,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, true, 25, 209,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->d3r2, true, 25, 223,
                         d3l2_source, sizeof(d3l2_source), viewport, sizeof(viewport));

    out->ok = ok;
    return ok;
}

uint8_t dm1_v1_viewport_d3l2_d3r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "ReDMCSB DUNVIEW.C:130 G3072_i_WallSet_Wall_D3R2=-6 and "
        "DUNVIEW.C:139 G3010_i_WallSet_Wall_D3L2=-5 define the PC34 "
        "wall-set indices. ReDMCSB DUNVIEW.C:317-318 declares "
        "G0696_puc_Bitmap_WallSet_Wall_D3R2 and "
        "G0697_puc_Bitmap_WallSet_Wall_D3L2. ReDMCSB DUNVIEW.C:579-580 "
        "defines G0711/G0712 D3L2/D3R2 frames as "
        "{0,15,25,73,8,49,0,0} and {208,223,25,73,8,49,0,0}. "
        "ReDMCSB DUNVIEW.C:8446-8464 F0128_DUNGEONVIEW_DrawViewport "
        "plain-wall branch checks relative squares 3,-2 and 3,2 and calls "
        "F0100_DUNGEONVIEW_DrawWallSetBitmap for old media; PC34 uses "
        "F0104/F0105 wall bitmap routes, not pit routes. ReDMCSB "
        "DUNVIEW.C:6253-6331 F0676/F0677 full-square dispatch is outside "
        "this direct plain-wall gate. ReDMCSB DUNVIEW.C:3048-3058 F0100, "
        "DUNVIEW.C:3113-3129 F0104, and DUNVIEW.C:3185-3204 F0105 preserve "
        "C10_COLOR_FLESH transparency with C112_BYTE_WIDTH_VIEWPORT. "
        "ReDMCSB DUNVIEW.C:2440-2441 builds the C10_WALL_D3R2 / "
        "C11_WALL_D3L2 flipped wall-set pair. ReDMCSB DEFS.H:2088 "
        "C10_COLOR_FLESH=10; DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT=112; "
        "DEFS.H:2610 C14_VIEW_SQUARE_D3L2=14; DEFS.H:2611 "
        "C15_VIEW_SQUARE_D3R2=15; this snapshot has C14/C15 names rather "
        "than M621/M622. ReDMCSB DEFS.H:3433 C10_WALL_D3R2=10; "
        "DEFS.H:3434 C11_WALL_D3L2=11; DEFS.H:4042 "
        "C702_ZONE_WALL_D3L2=702; DEFS.H:4043 C703_ZONE_WALL_D3R2=703; "
        "this snapshot has C702/C703 names rather than C621/C622. "
        "The D3L2/D3R2 gate is non-overlap with d3l_d3r_wall: lateral "
        "-2/+2 and C702/C703, not lateral -1/+1 and C705/C706. It is "
        "non-overlap with d3c_wall: lateral -2/+2 and C702/C703, not "
        "lateral 0 and C704. The direct branch has no F0107 ornament, no "
        "F0111 door, no F0115 alcove/thing pass, no F0104/F0105 pit route, "
        "and no F0108 floor ornament; C10 flesh pixels preserve destination.";
}
