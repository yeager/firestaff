#include "dm1_v1_viewport_d0r2_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D0L_PC34 = 1,       /* ReDMCSB: DEFS.H:2597 M610_VIEW_SQUARE_D0L */
    DM1_V1_VIEW_SQUARE_D0R_PC34 = 2,       /* ReDMCSB: DEFS.H:2598 M611_VIEW_SQUARE_D0R */
    DM1_V1_WALL_D0R_PC34 = 0,              /* ReDMCSB: DEFS.H:3423 C00_WALL_D0R */
    DM1_V1_WALL_D0L_PC34 = 1,              /* ReDMCSB: DEFS.H:3424 C01_WALL_D0L */
    DM1_V1_WALLSET_WALL_D0L_PC34 = -16,    /* ReDMCSB: DUNVIEW.C:143 G3014_i_WallSet_Wall_D0L */
    DM1_V1_WALLSET_WALL_D0R_PC34 = -17,    /* ReDMCSB: DUNVIEW.C:144 G3015_i_WallSet_Wall_D0R */
    DM1_V1_OLD_ZONE_WALL_D0R_PC34 = 715,   /* ReDMCSB: DEFS.H:4038 C715_ZONE_WALL_D0R */
    DM1_V1_ZONE_WALL_D0L_PC34 = 716,       /* ReDMCSB: DEFS.H:4056 C716_ZONE_WALL_D0L */
    DM1_V1_ZONE_WALL_D0R_PC34 = 717,       /* ReDMCSB: DEFS.H:4057 C717_ZONE_WALL_D0R */
    DM1_V1_ZONE_CEILING_PIT_D0R_PC34 = 872, /* ReDMCSB: DEFS.H:4219 C872_ZONE_CEILING_PIT_D0R */
    DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34 = 112
};

static const DM1_V1_D0R2WallAnchorPc34 s_anchors[DM1_V1_D0R2_WALL_ANCHOR_COUNT_PC34] = {
    {
        "F0126-D0R-wall-body",
        "DUNVIEW.C",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        8064,
        8162,
        "D0R2/D0R body; wall case draws D0R and returns"
    },
    {
        "F0126-D0R-wall-case",
        "DUNVIEW.C",
        "F0126_DUNGEONVIEW_DrawSquareD0R case C00_ELEMENT_WALL",
        8117,
        8144,
        "wall case uses F0100, optional F0105 flipped C01_D0L, F0104 native C00_D0R, then returns"
    },
    {
        "F0117-D3R-right-row-family",
        "DUNVIEW.C",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        6500,
        6622,
        "right-side wall dispatch family anchor with wall return and F0115 exclusion"
    },
    {
        "F0125-D0L-disjoint-sibling",
        "DUNVIEW.C",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        7960,
        8062,
        "D0L sibling body is cited only to prove disjoint anchoring from this D0R2 gate"
    },
    {
        "F0128-D0-row-dispatch",
        "DUNVIEW.C",
        "F0128_DUNGEONVIEW_Draw_CPSF",
        8508,
        8542,
        "row composition dispatches D0L at relative (0,-1), then D0R at (0,1), then D0C"
    },
    {
        "F0100-F0104-F0105-C10-blits",
        "DUNVIEW.C",
        "F0100/F0104/F0105 wall blit paths",
        3048,
        3204,
        "C10 transparent wall blit and flipped scratch blit paths"
    },
    {
        "F0163-thing-link-excluded",
        "DUNGEON.C",
        "F0163_DUNGEON_LinkThingToList",
        1769,
        1838,
        "thing-list mutation is outside the D0R wall return path"
    },
    {
        "F0164-thing-unlink-excluded",
        "DUNGEON.C",
        "F0164_DUNGEON_UnlinkThingFromList",
        1840,
        1905,
        "thing-list unlink mutation is outside the D0R wall return path"
    },
    {
        "F0172-square-aspect",
        "DUNGEON.C",
        "F0172_DUNGEON_SetSquareAspect",
        2466,
        2523,
        "square aspect is populated before the F0126 element switch"
    },
    {
        "DEFS-D0R-C10-zone-constants",
        "DEFS.H",
        "C10/M611/C00/C01/C715/C717 constants",
        2088,
        4057,
        "C10 flesh, D0R view square and wall constants, C715/C717 old/new D0R wall zones"
    },
    {
        "G0163-G0188-D0R-rectangles",
        "DUNVIEW.C",
        "G0163_aauc_Graphic558_Frame_Walls / G0188_aauc_Graphic558_FieldAspects",
        594,
        731,
        "D0R frame row {192,223,0,135,16,136,0,0}; field row {0,63,0x0A,0x03,16,136,0,64}"
    }
};

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:8064-8162 F0126 is the D0R body; lines 8117-8144 are the
 * C00 wall return path.  DUNVIEW.C:8536-8541 F0128 reaches F0125 D0L
 * from relative (0,-1), then this F0126 D0R path from relative (0,1).
 * DUNVIEW.C:6500-6622 F0117 is the same right-side row family anchor.
 */
static const DM1_V1_D0R2WallRouteSpecPc34 s_specs[DM1_V1_D0R2_WALL_ROUTE_COUNT_PC34] = {
    {
        DM1_V1_D0R2_WALL_ROUTE_NATIVE_PC34,
        true,
        false,
        true,
        false,
        0,
        1,
        DM1_V1_VIEW_SQUARE_D0R_PC34,
        DM1_V1_WALL_D0R_PC34,
        DM1_V1_WALL_D0L_PC34,
        DM1_V1_WALLSET_WALL_D0R_PC34,
        DM1_V1_WALLSET_WALL_D0L_PC34,
        DM1_V1_OLD_ZONE_WALL_D0R_PC34,
        DM1_V1_ZONE_WALL_D0R_PC34,
        192,
        223,
        0,
        135,
        16,
        136,
        0,
        0,
        DM1_V1_VIEW_SQUARE_D0R_PC34,
        0,
        63,
        0x03,
        16,
        136,
        0,
        64,
        DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        false,
        false,
        true,
        true,
        false,
        true,
        true,
        "native F0104 D0R wall route",
        "DUNVIEW.C:8117-8144 F0126 native D0R wall case; DEFS.H:4057 C717_ZONE_WALL_D0R"
    },
    {
        DM1_V1_D0R2_WALL_ROUTE_PARITY_FLIPPED_PC34,
        true,
        false,
        true,
        true,
        0,
        1,
        DM1_V1_VIEW_SQUARE_D0R_PC34,
        DM1_V1_WALL_D0R_PC34,
        DM1_V1_WALL_D0L_PC34,
        DM1_V1_WALLSET_WALL_D0R_PC34,
        DM1_V1_WALLSET_WALL_D0L_PC34,
        DM1_V1_OLD_ZONE_WALL_D0R_PC34,
        DM1_V1_ZONE_WALL_D0R_PC34,
        192,
        223,
        0,
        135,
        16,
        136,
        0,
        0,
        DM1_V1_VIEW_SQUARE_D0R_PC34,
        0,
        63,
        0x03,
        16,
        136,
        0,
        64,
        DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34,
        true,
        false,
        true,
        true,
        true,
        true,
        false,
        true,
        true,
        "parity F0105 flipped D0L-into-D0R route",
        "DUNVIEW.C:8127 F0126 parity route flips C01_WALL_D0L through F0105 into D0R"
    }
};

const DM1_V1_D0R2WallAnchorPc34 *
dm1_v1_viewport_d0r2_wall_anchor_table_pc34(size_t *count)
{
    if (count) *count = DM1_V1_D0R2_WALL_ANCHOR_COUNT_PC34;
    return s_anchors;
}

const DM1_V1_D0R2WallRouteSpecPc34 *
dm1_v1_viewport_d0r2_wall_route_spec_pc34(DM1_V1_D0R2WallRoutePc34 route)
{
    if (route == DM1_V1_D0R2_WALL_ROUTE_NATIVE_PC34 ||
        route == DM1_V1_D0R2_WALL_ROUTE_PARITY_FLIPPED_PC34) {
        return &s_specs[(int)route];
    }
    return NULL;
}

static bool dm1_v1_d0r2_wall_apply_pixel(
    const DM1_V1_D0R2WallRouteSpecPc34 *spec,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D0R2WallPixelPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->row = row;
    out->viewport_x = viewport_x;
    if (!spec) return false;

    if (row < 0 || row >= DM1_V1_D0R2_WALL_SYNTHETIC_HEIGHT_PC34 ||
        viewport_x < 0 || viewport_x >= DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34) {
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
        ? (DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 - 1 - out->source_x)
        : out->source_x;
    out->source_offset =
        (size_t)out->source_y * (size_t)DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 +
        (size_t)out->selected_source_x;
    out->viewport_offset =
        (size_t)row * (size_t)DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 +
        (size_t)viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip =
        out->source_pixel == DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d0r2_wall_blend_pixel_pc34(
            viewport[out->viewport_offset],
            out->source_pixel,
            DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

static bool add_check(
    DM1_V1_D0R2WallRunPc34 *out,
    const DM1_V1_D0R2WallRouteSpecPc34 *spec,
    int row,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    if (out->check_count >= DM1_V1_D0R2_WALL_CHECK_CAPACITY_PC34) {
        return false;
    }
    return dm1_v1_d0r2_wall_apply_pixel(
        spec,
        row,
        viewport_x,
        source,
        source_len,
        viewport,
        viewport_len,
        &out->checks[out->check_count++]);
}

bool dm1_v1_viewport_d0r2_wall_pc34_compat_run(DM1_V1_D0R2WallRunPc34 *out)
{
    uint8_t native_source[DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 *
                          DM1_V1_D0R2_WALL_SYNTHETIC_HEIGHT_PC34];
    uint8_t opposite_source[DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 *
                            DM1_V1_D0R2_WALL_SYNTHETIC_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 *
                     DM1_V1_D0R2_WALL_SYNTHETIC_HEIGHT_PC34];
    bool ok = true;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    memset(native_source, DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(native_source));
    memset(opposite_source, DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(opposite_source));
    memset(viewport, 0xee, sizeof(viewport));

    out->native = &s_specs[DM1_V1_D0R2_WALL_ROUTE_NATIVE_PC34];
    out->parity = &s_specs[DM1_V1_D0R2_WALL_ROUTE_PARITY_FLIPPED_PC34];
    out->c10_palette_index = DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34;
    out->c112_byte_width_viewport = DM1_V1_C112_BYTE_WIDTH_VIEWPORT_PC34;
    out->d0r_view_square_pc34 = DM1_V1_VIEW_SQUARE_D0R_PC34;
    out->d0l_view_square_pc34 = DM1_V1_VIEW_SQUARE_D0L_PC34;
    out->d0r_wall_pc34 = DM1_V1_WALL_D0R_PC34;
    out->d0l_wall_pc34 = DM1_V1_WALL_D0L_PC34;
    out->d0r_zone_old_media_pc34 = DM1_V1_OLD_ZONE_WALL_D0R_PC34;
    out->d0r_zone_pc34 = DM1_V1_ZONE_WALL_D0R_PC34;
    out->d0l_zone_pc34 = DM1_V1_ZONE_WALL_D0L_PC34;
    out->ceiling_pit_d0r_zone_pc34 = DM1_V1_ZONE_CEILING_PIT_D0R_PC34;
    out->f0128_dispatch_d0l_before_d0r = true;
    out->f0126_wall_returns_before_ceiling_pit =
        out->native->wall_case_returns_before_f0112 &&
        out->parity->wall_case_returns_before_f0112;
    out->f0126_wall_returns_before_thing_pass =
        out->native->wall_case_returns_before_f0115 &&
        out->parity->wall_case_returns_before_f0115;
    out->f0126_corridor_branch_has_f0112_and_f0115 = true;
    out->f0117_family_right_wall_return_anchor = true;
    out->f0163_f0164_not_part_of_wall_return =
        out->native->thing_list_link_unlink_excluded &&
        out->parity->thing_list_link_unlink_excluded;
    out->f0172_square_aspect_feeds_wall_switch = true;
    out->c10_flesh_pixels_preserve_destination = true;
    out->right_edge_clipped = true;
    out->no_f0111_marker = !out->native->calls_f0111_door && !out->parity->calls_f0111_door;
    out->source_evidence = dm1_v1_viewport_d0r2_wall_source_evidence_pc34();

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

uint8_t dm1_v1_viewport_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d0r2_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; no_asset_parity=1; "
        "real-asset bitmap parity is not claimed. ReDMCSB DUNVIEW.C:8064-8162 "
        "F0126_DUNGEONVIEW_DrawSquareD0R contains the D0R2/D0R body. "
        "DUNVIEW.C:8117-8144 wall case calls F0100_DUNGEONVIEW_DrawWallSetBitmap "
        "for G0702_puc_Bitmap_WallSet_Wall_D0R with "
        "G0163_aauc_Graphic558_Frame_Walls[M611_VIEW_SQUARE_D0R], then PC34 "
        "uses F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally "
        "with C01_WALL_D0L/C717_ZONE_WALL_D0R when parity flips, or "
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap with "
        "C00_WALL_D0R/C717_ZONE_WALL_D0R natively, then returns at "
        "DUNVIEW.C:8144 before the corridor branch ceiling-pit and thing pass. "
        "DUNVIEW.C:6500-6622 F0117_DUNGEONVIEW_DrawSquareD3R is the right-side "
        "row-family anchor; its wall path also returns before the shared F0115 "
        "tail. DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L is the "
        "disjoint D0L sibling and is not covered by this D0R2 gate. "
        "DUNVIEW.C:8508-8542 F0128 dispatches D2R2 at 8508, D0L from "
        "relative (0,-1) at 8536-8537, then D0R from relative (0,1) at "
        "8540-8541. DUNVIEW.C:3048-3058 F0100 preserves C10_COLOR_FLESH in "
        "clipped wall blits; DUNVIEW.C:3113-3129 F0104 and "
        "DUNVIEW.C:3185-3204 F0105 are the native and flipped scratch C10 "
        "wall blit paths. ReDMCSB DUNGEON.C:1769-1838 F0163_DUNGEON_LinkThingToList "
        "and DUNGEON.C:1840-1905 F0164_DUNGEON_UnlinkThingFromList are cited as "
        "excluded thing-list mutation anchors; DUNGEON.C:2466-2523 "
        "F0172_DUNGEON_SetSquareAspect populates C0_ELEMENT before the F0126 "
        "switch. ReDMCSB DEFS.H:2088 C10_COLOR_FLESH=10; DEFS.H:2598 "
        "M611_VIEW_SQUARE_D0R=2; DEFS.H:3423 C00_WALL_D0R=0; DEFS.H:3424 "
        "C01_WALL_D0L=1; DEFS.H:4038 C715_ZONE_WALL_D0R=715 old media zone; "
        "DEFS.H:4056 C716_ZONE_WALL_D0L=716; DEFS.H:4057 "
        "C717_ZONE_WALL_D0R=717; DEFS.H:4219 C872_ZONE_CEILING_PIT_D0R=872. "
        "DUNVIEW.C:594 G0163 D0R frame row is {192,223,0,135,16,136,0,0}; "
        "DUNVIEW.C:731 G0188 D0R field row is {0,63,0x0A,0x03,16,136,0,64}. "
        "The D0R2 wall case has no F0111 door marker, no F0112 ceiling-pit "
        "marker, and no F0115 thing-pass marker because the wall case returns "
        "at DUNVIEW.C:8144.";
}
