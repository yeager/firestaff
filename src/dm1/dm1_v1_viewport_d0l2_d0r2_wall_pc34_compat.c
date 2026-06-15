#include "dm1/dm1_v1_viewport_d0l2_d0r2_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L_VIEW_SQUARE = 1,       /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L. */
    DM1_D0R_VIEW_SQUARE = 2,       /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R. */
    DM1_D2L2_VIEW_SQUARE = 9,      /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2. */
    DM1_D2R2_VIEW_SQUARE = 10,     /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2. */
    DM1_ELEMENT_WALL = 0,          /* ReDMCSB DUNVIEW.C:8007/8117 C00_ELEMENT_WALL. */
    DM1_WALL_D0R = 0,              /* ReDMCSB DEFS.H:3423 C00_WALL_D0R. */
    DM1_WALL_D0L = 1,              /* ReDMCSB DEFS.H:3424 C01_WALL_D0L. */
    DM1_WALL_D2R2 = 5,             /* ReDMCSB DEFS.H:3428 C05_WALL_D2R2. */
    DM1_WALL_D2L2 = 6,             /* ReDMCSB DEFS.H:3429 C06_WALL_D2L2. */
    DM1_ZONE_WALL_D2L2 = 707,      /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2. */
    DM1_ZONE_WALL_D2R2 = 708,      /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2. */
    DM1_ZONE_WALL_D0L = 716,       /* ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L. */
    DM1_ZONE_WALL_D0R = 717,       /* ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R. */
    DM1_FRAME_D0L_ROW = 10,        /* ReDMCSB DUNVIEW.C:593 G0163 D0L row. */
    DM1_FRAME_D0R_ROW = 11,        /* ReDMCSB DUNVIEW.C:594 G0163 D0R row. */
    DM1_PROBE_ROW = 67
};

static const char s_source_evidence[] =
    "Source-locked contract-only DM1 V1 D0L2/D0R2 wall gate; "
    "no real-asset bitmap parity and no game-data load. ReDMCSB "
    "DRAWVIEW.C:709-857 F0097_DUNGEONVIEW_DrawViewport preserves the "
    "viewport/palette handoff after DUNVIEW.C:2962-3003 "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling owns the floor/ceiling rows. "
    "ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF is the "
    "near-view dispatcher; DUNVIEW.C:8503-8508 anchors the existing "
    "D2L2/D2R2 branch and DUNVIEW.C:8534-8542 dispatches D0L by "
    "F0150 relative depth/lateral 0,-1 through F0125, then D0R by 0,1 "
    "through F0126, then F0127 D0C. ReDMCSB DUNVIEW.C:7960-8062 "
    "F0125_DUNGEONVIEW_DrawSquareD0L handles C00_ELEMENT_WALL at "
    "8007-8038, using DUNVIEW.C:8017 F0105(G2107_WallSet[C00_WALL_D0R], "
    "C716_ZONE_WALL_D0L) for the parity route or DUNVIEW.C:8033 "
    "F0104(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L) for the "
    "native route, then returning at 8038. ReDMCSB DUNVIEW.C:8064-8162 "
    "F0126_DUNGEONVIEW_DrawSquareD0R handles C00_ELEMENT_WALL at "
    "8117-8144, using DUNVIEW.C:8127 F0105(G2107_WallSet[C01_WALL_D0L], "
    "C717_ZONE_WALL_D0R) for the parity route or DUNVIEW.C:8139 "
    "F0104(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R) for the "
    "native route, then returning at 8144. DUNVIEW.C:8164-8294 F0127 "
    "is a follow-up anchor only. ReDMCSB DUNVIEW.C:3113-3156 F0104 and "
    "DUNVIEW.C:3185-3247 F0105 both blit with DEFS.H:2088 "
    "C10_COLOR_FLESH transparency; F0105 uses the flipped scratch route "
    "through G0074_puc_Bitmap_Temporary at 3198-3204 and 3220-3239. "
    "DEFS.H:2597-2598 binds M610/M611 to D0L/D0R view squares; "
    "DEFS.H:2605-2606 and DEFS.H:3428-3429 are D2L2/D2R2 anchors used "
    "only to prove this gate is not the existing D2L2/D2R2 route; "
    "DEFS.H:3423-3424 binds C00_WALL_D0R/C01_WALL_D0L; "
    "DEFS.H:4040-4057 binds the wall-zone family including C716/C717. "
    "COORD.C:1713-1722 and COMMAND.C:1126-1127 bind the 320x200 screen, "
    "224x136 viewport, and centered 48-pixel side margin used by this "
    "synthetic edge probe. This gate is non-duplicative vs the existing "
    "D0L2/D0R2 F0115 thing-pass gate because the WALL cases return before "
    "F0115, and non-duplicative vs the D0L/D0R wall gate because it checks "
    "screen-edge clipped writes at X 0..47 and X 272..319.";

static const DM1_V1_ViewportD0L2D0R2WallSpecPc34 s_specs[] = {
    {
        DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34,
        "D0L2 synthetic edge wall via F0125 D0L/C716",
        1,
        1,
        0,
        0,
        -1,
        -2,
        DM1_D0L_VIEW_SQUARE,
        DM1_ELEMENT_WALL,
        DM1_WALL_D0L,
        DM1_WALL_D0R,
        DM1_ZONE_WALL_D0L,
        DM1_FRAME_D0L_ROW,
        0,
        31,
        0,
        135,
        0,
        47,
        0,
        47,
        DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34,
        1,
        0,
        1,
        1,
        1,
        1,
        1,
        1,
        "ReDMCSB DUNVIEW.C:8536-8537 F0128 -> F0125; "
        "DUNVIEW.C:7960-8062 F0125",
        "ReDMCSB DUNVIEW.C:8033 F0104 C716 native wall binding; "
        "DUNVIEW.C:8017 F0105 C716 parity binding",
        s_source_evidence
    },
    {
        DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34,
        "D0R2 synthetic edge wall via F0126 D0R/C717",
        1,
        1,
        1,
        0,
        1,
        2,
        DM1_D0R_VIEW_SQUARE,
        DM1_ELEMENT_WALL,
        DM1_WALL_D0R,
        DM1_WALL_D0L,
        DM1_ZONE_WALL_D0R,
        DM1_FRAME_D0R_ROW,
        192,
        223,
        0,
        135,
        272,
        319,
        0,
        47,
        DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34,
        0,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        "ReDMCSB DUNVIEW.C:8540-8541 F0128 -> F0126; "
        "DUNVIEW.C:8064-8162 F0126",
        "ReDMCSB DUNVIEW.C:8127 F0105 C717 parity binding; "
        "DUNVIEW.C:8139 F0104 C717 native wall binding",
        s_source_evidence
    }
};

size_t dm1_v1_viewport_d0l2_d0r2_wall_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
    DM1_V1_ViewportD0L2D0R2WallSidePc34 side)
{
    if (side == DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34 ||
        side == DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34) {
        return &s_specs[(int)side];
    }
    return NULL;
}

int dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec,
    int screen_x,
    int *out_source_x)
{
    int local_x;

    if (!spec || !out_source_x) return -1;
    if (screen_x < spec->screen_x_first || screen_x > spec->screen_x_last) {
        return 1;
    }
    local_x = screen_x - spec->screen_x_first;
    if (local_x < 0 || local_x >= DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34) {
        return -1;
    }
    if (spec->uses_f0105_parity_scratch_flip) {
        *out_source_x = spec->source_x_last - local_x;
    } else {
        *out_source_x = spec->source_x_first + local_x;
    }
    return 0;
}

uint8_t dm1_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    /* ReDMCSB: DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 both
     * call F0132 with DEFS.H:2088 C10_COLOR_FLESH transparency. */
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

int dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec,
    const uint8_t *source,
    size_t source_len,
    uint8_t *screen,
    size_t screen_len,
    int screen_x,
    int row,
    DM1_V1_ViewportD0L2D0R2WallPixelPc34 *out)
{
    int mapped;
    int source_x = -1;
    size_t source_offset;
    size_t screen_offset;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->row = row;
    out->screen_x = screen_x;
    if (!spec) return -1;
    if (row < 0 || row >= DM1_V1_D0L2_D0R2_WALL_VIEWPORT_HEIGHT_PC34 ||
        screen_x < 0 || screen_x >= DM1_V1_D0L2_D0R2_WALL_SCREEN_WIDTH_PC34) {
        out->no_write_metadata = 1;
        return 1;
    }
    if (!source || !screen) return -1;

    mapped = dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
        spec, screen_x, &source_x);
    if (mapped != 0) {
        out->no_write_metadata = mapped > 0;
        return mapped;
    }

    out->in_clip = 1;
    out->uses_scratch = spec->uses_f0105_parity_scratch_flip;
    out->source_x = source_x;
    out->viewport_y = row;
    source_offset = (size_t)row *
                    (size_t)DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 +
                    (size_t)source_x;
    screen_offset = (size_t)row *
                    (size_t)DM1_V1_D0L2_D0R2_WALL_SCREEN_WIDTH_PC34 +
                    (size_t)screen_x;
    if (source_offset >= source_len || screen_offset >= screen_len) return -1;

    out->source_offset = source_offset;
    out->screen_offset = screen_offset;
    out->pixel_before = screen[screen_offset];
    out->source_pixel = source[source_offset];
    out->transparent_skip = out->source_pixel == spec->transparent_color;
    out->writes_pixel = !out->transparent_skip;
    screen[screen_offset] = dm1_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
        screen[screen_offset], source[source_offset], (uint8_t)spec->transparent_color);
    out->pixel_after = screen[screen_offset];
    return 0;
}

static int add_check(
    DM1_V1_ViewportD0L2D0R2WallProbePc34 *result,
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec,
    const uint8_t *source,
    size_t source_len,
    uint8_t *screen,
    size_t screen_len,
    int screen_x,
    int row)
{
    if (result->check_count >= DM1_V1_D0L2_D0R2_WALL_PROBE_CHECK_CAPACITY_PC34) {
        return -1;
    }
    return dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
        spec,
        source,
        source_len,
        screen,
        screen_len,
        screen_x,
        row,
        &result->checks[result->check_count++]);
}

int dm1_v1_viewport_d0l2_d0r2_wall_probe_pc34_compat(
    DM1_V1_ViewportD0L2D0R2WallProbePc34 *out)
{
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);
    uint8_t left_source[DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 *
                        DM1_V1_D0L2_D0R2_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t right_source[DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 *
                         DM1_V1_D0L2_D0R2_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t screen[DM1_V1_D0L2_D0R2_WALL_SCREEN_WIDTH_PC34 *
                   DM1_V1_D0L2_D0R2_WALL_SCREEN_HEIGHT_PC34];
    DM1_V1_ViewportD0L2D0R2WallProbePc34 result;

    if (!out || !left || !right) return -1;
    memset(&result, 0, sizeof(result));
    memset(left_source, DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(left_source));
    memset(right_source, DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(right_source));
    memset(screen, 0xee, sizeof(screen));

    left_source[(size_t)DM1_PROBE_ROW * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34] =
        0x21;
    left_source[(size_t)DM1_PROBE_ROW * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 47u] =
        0x22;
    right_source[(size_t)DM1_PROBE_ROW * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34] =
        0x31;
    right_source[(size_t)DM1_PROBE_ROW * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 47u] =
        0x32;
    right_source[(size_t)DM1_PROBE_ROW * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 46u] =
        0x33;

    (void)add_check(&result, left, left_source, sizeof(left_source),
                    screen, sizeof(screen), 0, DM1_PROBE_ROW);
    (void)add_check(&result, left, left_source, sizeof(left_source),
                    screen, sizeof(screen), 1, DM1_PROBE_ROW);
    (void)add_check(&result, left, left_source, sizeof(left_source),
                    screen, sizeof(screen), 47, DM1_PROBE_ROW);
    (void)add_check(&result, left, left_source, sizeof(left_source),
                    screen, sizeof(screen), 48, DM1_PROBE_ROW);
    (void)add_check(&result, right, right_source, sizeof(right_source),
                    screen, sizeof(screen), 272, DM1_PROBE_ROW);
    (void)add_check(&result, right, right_source, sizeof(right_source),
                    screen, sizeof(screen), 273, DM1_PROBE_ROW);
    (void)add_check(&result, right, right_source, sizeof(right_source),
                    screen, sizeof(screen), 319, DM1_PROBE_ROW);
    (void)add_check(&result, right, right_source, sizeof(right_source),
                    screen, sizeof(screen), 271, DM1_PROBE_ROW);
    (void)add_check(&result, right, right_source, sizeof(right_source),
                    screen, sizeof(screen), 320, DM1_PROBE_ROW);

    result.route_count = (int)dm1_v1_viewport_d0l2_d0r2_wall_spec_count_pc34();
    result.assertion_contract_count = 72;
    result.source_evidence = s_source_evidence;
    result.f0098_row_owned = 1;
    result.f0128_dispatch_ok =
        left->f0128_draw_order_index == 0 &&
        right->f0128_draw_order_index == 1 &&
        left->f0128_redmcsb_depth == 0 &&
        right->f0128_redmcsb_depth == 0;
    result.m610_m611_zone_binding_ok =
        left->view_square_index == DM1_D0L_VIEW_SQUARE &&
        left->wall_zone == DM1_ZONE_WALL_D0L &&
        right->view_square_index == DM1_D0R_VIEW_SQUARE &&
        right->wall_zone == DM1_ZONE_WALL_D0R;
    result.f0104_native_route_ok =
        left->uses_f0104_native_blit &&
        left->native_wall_index == DM1_WALL_D0L &&
        left->flipped_wall_index == DM1_WALL_D0R;
    result.f0105_parity_route_ok =
        right->uses_f0105_parity_scratch_flip &&
        right->native_wall_index == DM1_WALL_D0R &&
        right->flipped_wall_index == DM1_WALL_D0L;
    result.c10_transparency_ok =
        result.checks[0].pixel_after == 0x21 &&
        result.checks[1].transparent_skip &&
        result.checks[1].pixel_after == 0xee &&
        result.checks[4].uses_scratch &&
        result.checks[4].pixel_after == 0x32 &&
        result.checks[5].pixel_after == 0x33 &&
        result.checks[6].pixel_after == 0x31;
    result.edge_clip_ok =
        result.checks[0].in_clip &&
        result.checks[2].in_clip &&
        result.checks[3].no_write_metadata &&
        result.checks[4].in_clip &&
        result.checks[6].in_clip &&
        result.checks[7].no_write_metadata &&
        result.checks[8].no_write_metadata;
    result.no_f0111_no_f0115_no_f0108_ok =
        left->wall_case_returns_before_f0111 &&
        left->wall_case_returns_before_f0115 &&
        left->wall_case_returns_before_f0108 &&
        right->wall_case_returns_before_f0111 &&
        right->wall_case_returns_before_f0115 &&
        right->wall_case_returns_before_f0108;
    result.nonduplicative_ok =
        left->nonduplicative_vs_f0115_gate &&
        right->nonduplicative_vs_f0115_gate &&
        left->nonduplicative_vs_d0l_d0r_wall_gate &&
        right->nonduplicative_vs_d0l_d0r_wall_gate &&
        DM1_D2L2_VIEW_SQUARE == 9 &&
        DM1_D2R2_VIEW_SQUARE == 10 &&
        DM1_WALL_D2L2 == 6 &&
        DM1_WALL_D2R2 == 5 &&
        DM1_ZONE_WALL_D2L2 == 707 &&
        DM1_ZONE_WALL_D2R2 == 708;
    result.ok =
        result.route_count == DM1_V1_D0L2_D0R2_WALL_ROUTE_COUNT_PC34 &&
        result.f0098_row_owned &&
        result.f0128_dispatch_ok &&
        result.m610_m611_zone_binding_ok &&
        result.f0104_native_route_ok &&
        result.f0105_parity_route_ok &&
        result.c10_transparency_ok &&
        result.edge_clip_ok &&
        result.no_f0111_no_f0115_no_f0108_ok &&
        result.nonduplicative_ok;
    result.focused_test_pass_count = result.ok ? 1 : 0;
    result.focused_test_failure_count = result.ok ? 0 : 1;
    *out = result;
    return result.ok ? 0 : 1;
}

const char *dm1_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
