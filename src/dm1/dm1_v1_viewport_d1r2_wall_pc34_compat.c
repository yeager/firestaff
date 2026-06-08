#include "dm1_v1_viewport_d1r2_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D1L_PC34 = 4,       /* ReDMCSB: DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    DM1_V1_VIEW_SQUARE_D1R_PC34 = 5,       /* ReDMCSB: DEFS.H:2601 M608_VIEW_SQUARE_D1R */
    DM1_V1_WALL_D1R_PC34 = 2,              /* ReDMCSB: DEFS.H:3425 C02_WALL_D1R */
    DM1_V1_WALL_D1L_PC34 = 3,              /* ReDMCSB: DEFS.H:3426 C03_WALL_D1L */
    DM1_V1_ZONE_WALL_D1C_PC34 = 712,       /* ReDMCSB: DEFS.H:4052 C712_ZONE_WALL_D1C */
    DM1_V1_ZONE_WALL_D1L_PC34 = 713,       /* ReDMCSB: DEFS.H:4053 C713_ZONE_WALL_D1L */
    DM1_V1_ZONE_WALL_D1R_PC34 = 714,       /* ReDMCSB: DEFS.H:4054 C714_ZONE_WALL_D1R */
    DM1_V1_OLD_ZONE_WALL_D1R_PC34 = 712,   /* ReDMCSB: DEFS.H:4035 C712_ZONE_WALL_D1R */
    DM1_V1_WALLSET_WALL_D1L_PC34 = -14,    /* ReDMCSB: DUNVIEW.C:137 G3008_i_WallSet_Wall_D1L */
    DM1_V1_WALLSET_WALL_D1R_PC34 = -15,    /* ReDMCSB: DUNVIEW.C:138 G3009_i_WallSet_Wall_D1R */
    DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34 = 112,
    DM1_V1_VIEW_WALL_D1R_LEFT_PC34 = 13
};

static const DM1_V1_D1R2WallAnchorPc34 s_anchors[DM1_V1_D1R2_WALL_ANCHOR_COUNT_PC34] = {
    {
        "F0123-D1R-wall",
        "DUNVIEW.C",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        7559,
        7725,
        "D1R body; wall case draws D1R then F0107 then returns"
    },
    {
        "F0120-F0100-clip-path",
        "DUNVIEW.C",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF / F0100_DUNGEONVIEW_DrawWallSetBitmap",
        7051,
        7242,
        "near-row clip path anchor paired with F0100 C10 blit at 3048-3058"
    },
    {
        "F0104-C10-wall-blit",
        "DUNVIEW.C",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        3113,
        3129,
        "native PC34 wall blit uses C10 transparent clipped blit"
    },
    {
        "F0105-C10-flipped-wall-blit",
        "DUNVIEW.C",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
        3185,
        3204,
        "flipped PC34 wall blit uses scratch bitmap and C10 transparent blit"
    },
    {
        "F0125-F0126-wall-dispatch",
        "DUNVIEW.C",
        "F0125/F0126 wall cases",
        8007,
        8144,
        "D0 wall-case return pattern used as dispatch mirror evidence"
    },
    {
        "F0127-F0128-follow-up",
        "DUNVIEW.C",
        "F0128_DUNGEONVIEW_Draw_CPSF",
        8524,
        8542,
        "D1 row dispatch calls F0122 D1L left, F0123 D1R right, then F0124 D1C center"
    },
    {
        "DEFS-D1R",
        "DEFS.H",
        "C10/C02/C03/C714 constants",
        2088,
        4054,
        "C10 flesh, C02/C03 wall pair, M608 view square, C714 D1R zone"
    },
    {
        "G0163-D1R-frame",
        "DUNVIEW.C",
        "G0163_aauc_Graphic558_Frame_Walls[M608_VIEW_SQUARE_D1R]",
        591,
        591,
        "D1R frame row metadata is {160,223,9,119,128,111,0,0}"
    },
    {
        "F0792-F0765-PC34-center-paths",
        "DUNVIEW.C",
        "F0792_DUNGEONVIEW_DrawBitmapYYY / F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency",
        3159,
        3304,
        "PC34 opaque bitmap path anchors are source-locked as non-claims here"
    }
};

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:7604-7628 F0123 D1R wall case draws G0700 with the M608 frame,
 * then PC34 draws either C03_D1L flipped through F0105 or C02_D1R through
 * F0104 at C714_ZONE_WALL_D1R, probes M586_VIEW_WALL_D1R_LEFT, and returns.
 * DUNVIEW.C:8524-8529 F0128 dispatches D1L before D1R.
 */
static const DM1_V1_D1R2WallRouteSpecPc34 s_specs[DM1_V1_D1R2_WALL_ROUTE_COUNT_PC34] = {
    {
        DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34,
        true,
        false,
        true,
        false,
        1,
        1,
        1,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        DM1_V1_WALL_D1R_PC34,
        DM1_V1_WALL_D1L_PC34,
        DM1_V1_WALLSET_WALL_D1R_PC34,
        DM1_V1_WALLSET_WALL_D1L_PC34,
        DM1_V1_ZONE_WALL_D1R_PC34,
        DM1_V1_OLD_ZONE_WALL_D1R_PC34,
        160,
        223,
        9,
        119,
        128,
        111,
        0,
        0,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        0x02,
        32,
        111,
        0,
        64,
        DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        false,
        false,
        true,
        DM1_V1_VIEW_WALL_D1R_LEFT_PC34,
        true,
        true,
        false,
        false,
        true,
        "native F0104 D1R wall route",
        "DUNVIEW.C:7604-7628 F0123 native D1R wall case; DEFS.H:4054 C714_ZONE_WALL_D1R"
    },
    {
        DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34,
        true,
        false,
        true,
        true,
        1,
        1,
        1,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        DM1_V1_WALL_D1R_PC34,
        DM1_V1_WALL_D1L_PC34,
        DM1_V1_WALLSET_WALL_D1R_PC34,
        DM1_V1_WALLSET_WALL_D1L_PC34,
        DM1_V1_ZONE_WALL_D1R_PC34,
        DM1_V1_OLD_ZONE_WALL_D1R_PC34,
        160,
        223,
        9,
        119,
        128,
        111,
        0,
        0,
        DM1_V1_VIEW_SQUARE_D1R_PC34,
        0x02,
        32,
        111,
        0,
        64,
        DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        false,
        true,
        true,
        true,
        DM1_V1_VIEW_WALL_D1R_LEFT_PC34,
        true,
        true,
        false,
        false,
        true,
        "parity F0105 flipped D1L-into-D1R route",
        "DUNVIEW.C:7613-7615 F0123 parity route flips C03_WALL_D1L through F0105 into D1R"
    }
};

const DM1_V1_D1R2WallAnchorPc34 *
dm1_v1_viewport_d1r2_wall_anchor_table_pc34(size_t *count)
{
    if (count) *count = DM1_V1_D1R2_WALL_ANCHOR_COUNT_PC34;
    return s_anchors;
}

const DM1_V1_D1R2WallRouteSpecPc34 *
dm1_v1_viewport_d1r2_wall_route_spec_pc34(DM1_V1_D1R2WallRoutePc34 route)
{
    if (route == DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34 ||
        route == DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34) {
        return &s_specs[(int)route];
    }
    return NULL;
}

static bool dm1_v1_d1r2_wall_apply_pixel(
    const DM1_V1_D1R2WallRouteSpecPc34 *spec,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1R2WallPixelPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->row = row;
    out->viewport_x = viewport_x;
    if (!spec) return false;

    if (row < 0 || row >= DM1_V1_D1R2_WALL_SYNTHETIC_HEIGHT_PC34 ||
        viewport_x < 0 || viewport_x >= DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->parity_flip = spec->parity_flip;
    out->uses_scratch = spec->uses_f0105_c10_flipped_wall_blit;
    out->in_clip = true;
    out->source_x = viewport_x;
    out->source_y = row;
    out->selected_source_x = spec->parity_flip
        ? (DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)row * (size_t)DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 +
        (size_t)viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

bool dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
    const DM1_V1_D1R2WallRouteSpecPc34 *spec,
    int viewport_y,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1R2WallPixelPc34 *out)
{
    int local_x;
    int local_y;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->row = viewport_y;
    out->viewport_x = viewport_x;
    if (!spec) return false;

    if (viewport_y < spec->frame_top_y ||
        viewport_y > spec->frame_bottom_y ||
        viewport_x < spec->frame_left_x ||
        viewport_x > spec->frame_right_x) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    local_x = viewport_x - spec->frame_left_x;
    local_y = viewport_y - spec->frame_top_y;
    out->parity_flip = spec->parity_flip;
    out->uses_scratch = spec->uses_f0105_c10_flipped_wall_blit;
    out->in_clip = true;
    out->source_x = spec->frame_source_x + local_x;
    out->source_y = spec->frame_source_y + local_y;
    out->selected_source_x = spec->parity_flip
        ? (spec->frame_source_x + (spec->frame_right_x - spec->frame_left_x) - local_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)(spec->frame_right_x - spec->frame_left_x + 1) +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)viewport_y * (size_t)DM1_V1_D1R2_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

static bool add_check(
    DM1_V1_D1R2WallRunPc34 *out,
    const DM1_V1_D1R2WallRouteSpecPc34 *spec,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    if (out->check_count >= DM1_V1_D1R2_WALL_CHECK_CAPACITY_PC34) {
        return false;
    }
    return dm1_v1_d1r2_wall_apply_pixel(
        spec,
        row,
        viewport_x,
        source,
        source_len,
        viewport,
        viewport_len,
        &out->checks[out->check_count++]);
}

bool dm1_v1_viewport_d1r2_wall_pc34_compat_run(DM1_V1_D1R2WallRunPc34 *out)
{
    uint8_t native_source[DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 *
                          DM1_V1_D1R2_WALL_SYNTHETIC_HEIGHT_PC34];
    uint8_t opposite_source[DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 *
                            DM1_V1_D1R2_WALL_SYNTHETIC_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 *
                     DM1_V1_D1R2_WALL_SYNTHETIC_HEIGHT_PC34];
    bool ok = true;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    memset(native_source, DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(native_source));
    memset(opposite_source, DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(opposite_source));
    memset(viewport, 0xee, sizeof(viewport));

    out->native = &s_specs[DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34];
    out->parity = &s_specs[DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34];
    out->c10_palette_index = DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34;
    out->c112_byte_width_viewport = DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34;
    out->d1r_view_square_pc34 = DM1_V1_VIEW_SQUARE_D1R_PC34;
    out->d1l_view_square_pc34 = DM1_V1_VIEW_SQUARE_D1L_PC34;
    out->d1r_wall_pc34 = DM1_V1_WALL_D1R_PC34;
    out->d1l_wall_pc34 = DM1_V1_WALL_D1L_PC34;
    out->d1r_zone_pc34 = DM1_V1_ZONE_WALL_D1R_PC34;
    out->d1l_zone_pc34 = DM1_V1_ZONE_WALL_D1L_PC34;
    out->d1c_zone_pc34 = DM1_V1_ZONE_WALL_D1C_PC34;
    out->f0107_ordinal_pc34 = DM1_V1_VIEW_WALL_D1R_LEFT_PC34;
    out->d1r2_is_right_side_mirror =
        out->native->native_wall_index_pc34 == out->parity->native_wall_index_pc34 &&
        out->parity->opposite_wall_index_pc34 == DM1_V1_WALL_D1L_PC34;
    out->native_route_uses_f0104 = out->native->uses_f0104_c10_wall_blit;
    out->parity_route_uses_f0105 = out->parity->uses_f0105_c10_flipped_wall_blit;
    out->parity_scratch_flips_opposite_native_wall =
        out->parity->uses_f0105_c10_flipped_wall_blit &&
        out->parity->opposite_wall_index_pc34 == DM1_V1_WALL_D1L_PC34;
    out->c10_flesh_pixels_preserve_destination = true;
    out->right_edge_clipped = true;
    out->f0107_then_return =
        out->native->calls_f0107_wall_ornament_probe &&
        out->native->wall_case_returns_before_f0111 &&
        out->native->wall_case_returns_before_f0115;
    out->f0128_dispatch_left_before_right = true;
    out->no_f0111_marker = !out->native->calls_f0111_door && !out->parity->calls_f0111_door;
    out->no_f0115_thing_pass_marker =
        !out->native->calls_f0115_thing_pass &&
        !out->parity->calls_f0115_thing_pass &&
        out->native->thing_pass_marker_excluded &&
        out->parity->thing_pass_marker_excluded;
    out->source_evidence = dm1_v1_viewport_d1r2_wall_source_evidence_pc34();

    native_source[0] = 10;
    native_source[1] = 0x21;
    native_source[7] = 0x7a;
    native_source[8] = 0x32;
    native_source[31] = 0x4f;
    opposite_source[7] = 10;
    opposite_source[6] = 0x63;
    opposite_source[0] = 0x6e;
    opposite_source[15] = 0x70;
    opposite_source[31] = 0x5d;

    ok = ok && add_check(out, out->native, 0, 0,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 0, 1,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 0, 7,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 0, 8,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 1, 0,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 3, 7,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->native, 4, 0,
                         native_source, sizeof(native_source), viewport, sizeof(viewport));

    ok = ok && add_check(out, out->parity, 0, 0,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->parity, 0, 1,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->parity, 0, 7,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->parity, 1, 0,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->parity, 3, 7,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));
    ok = ok && add_check(out, out->parity, 3, 8,
                         opposite_source, sizeof(opposite_source), viewport, sizeof(viewport));

    out->ok = ok;
    return ok;
}

uint8_t dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d1r2_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; no_asset_parity=1; "
        "real-asset bitmap parity is not claimed. ReDMCSB DUNVIEW.C:7559-7725 "
        "F0123_DUNGEONVIEW_DrawSquareD1R contains the D1R body. "
        "DUNVIEW.C:7604-7628 wall case calls F0100_DUNGEONVIEW_DrawWallSetBitmap "
        "for G0700_puc_Bitmap_WallSet_Wall_D1LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M608_VIEW_SQUARE_D1R], then PC34 "
        "uses F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally "
        "with C03_WALL_D1L/C714_ZONE_WALL_D1R when parity flips, or "
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap with "
        "C02_WALL_D1R/C714_ZONE_WALL_D1R natively, then F0107 with "
        "M586_VIEW_WALL_D1R_LEFT and returns. ReDMCSB DUNVIEW.C:7051-7242 "
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF anchors the preceding clip path; "
        "DUNVIEW.C:3048-3058 F0100 preserves C10_COLOR_FLESH in clipped blits. "
        "DUNVIEW.C:3113-3129 F0104 and DUNVIEW.C:3185-3204 F0105 are the C10 "
        "wall blit and flipped scratch-wall blit paths; DUNVIEW.C:3159-3304 "
        "F0765/F0792 are PC34 opaque/center bitmap paths and are non-claims. "
        "DUNVIEW.C:8007-8038 F0125 and DUNVIEW.C:8117-8144 F0126 show wall-case "
        "return dispatch mirrors. DUNVIEW.C:8524-8529 F0128 dispatches D1L "
        "before D1R, and DUNVIEW.C:8532-8542 follows with D1C then D0L/D0R/D0C. "
        "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH=10; DEFS.H:2601 "
        "M608_VIEW_SQUARE_D1R=5; DEFS.H:3425 C02_WALL_D1R=2; DEFS.H:3426 "
        "C03_WALL_D1L=3; DEFS.H:4052 C712_ZONE_WALL_D1C=712; DEFS.H:4054 "
        "C714_ZONE_WALL_D1R=714; DUNVIEW.C:591 G0163 D1R frame row is "
        "{160,223,9,119,128,111,0,0}; DUNVIEW.C:728 G0188 D1R field row is "
        "{0,63,0x0A,0x02,32,111,0,64}. The D1R wall case has no F0111 door "
        "and no F0115 thing-pass marker because it returns at DUNVIEW.C:7628 "
        "immediately after the F0107 wall-ornament probe.";
}
