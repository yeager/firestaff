/*
 * DM1 V1 D0L2/D0R2 wall source-lock test.
 *
 * ReDMCSB anchors:
 * - DRAWVIEW.C:709-857 F0097_DUNGEONVIEW_DrawViewport.
 * - DUNVIEW.C:2962-3003 F0098_DUNGEONVIEW_DrawFloorAndCeiling.
 * - DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF, especially
 *   8503-8508 for the disjoint D2L2/D2R2 row and 8534-8542 for
 *   the D0L/D0R/D0C dispatch.
 * - DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L.
 * - DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R.
 * - DUNVIEW.C:8164-8294 F0127_DUNGEONVIEW_DrawSquareD0C follow-up.
 * - DUNVIEW.C:3113-3156 F0104 native blit and 3185-3247 F0105
 *   parity scratch flip.
 * - DEFS.H:2088, 2597-2606, 3423-3424, 3428-3429, 4040-4057.
 * - COORD.C:1713-1722 and COMMAND.C:1126-1127 for the 320x200 /
 *   224x136 synthetic edge clipping geometry.
 */
#include "dm1/dm1_v1_viewport_d0l2_d0r2_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_contains(
    const char *id,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               id,
               needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_spec_identity_and_dispatch(void)
{
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);

    expect_int("spec.count",
               (int)dm1_v1_viewport_d0l2_d0r2_wall_spec_count_pc34(),
               2,
               "DUNVIEW.C:8534-8542 F0128 D0 side pair");
    expect_int("spec.left.present", left != NULL, 1, "DUNVIEW.C:7960 F0125");
    expect_int("spec.right.present", right != NULL, 1, "DUNVIEW.C:8064 F0126");
    expect_int("spec.invalid.null",
               dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
                   (DM1_V1_ViewportD0L2D0R2WallSidePc34)99) == NULL,
               1,
               "D0L2/D0R2 route table has two entries only");
    if (!left || !right) return;

    expect_int("left.contract", left->source_locked_contract_only, 1,
               "contract-only source lock");
    expect_int("right.contract", right->source_locked_contract_only, 1,
               "contract-only source lock");
    expect_int("left.no_asset", left->no_real_asset_bitmap_parity, 1,
               "no real-asset bitmap parity");
    expect_int("right.no_asset", right->no_real_asset_bitmap_parity, 1,
               "no real-asset bitmap parity");
    expect_int("left.order", left->f0128_draw_order_index, 0,
               "DUNVIEW.C:8536-8537 F0128 draws D0L first");
    expect_int("right.order", right->f0128_draw_order_index, 1,
               "DUNVIEW.C:8540-8541 F0128 draws D0R second");
    expect_int("left.redmcsb_depth", left->f0128_redmcsb_depth, 0,
               "DUNVIEW.C:8536 F0150 depth 0");
    expect_int("right.redmcsb_depth", right->f0128_redmcsb_depth, 0,
               "DUNVIEW.C:8540 F0150 depth 0");
    expect_int("left.redmcsb_lateral", left->f0128_redmcsb_lateral, -1,
               "DUNVIEW.C:8536 F0150 lateral -1");
    expect_int("right.redmcsb_lateral", right->f0128_redmcsb_lateral, 1,
               "DUNVIEW.C:8540 F0150 lateral +1");
    expect_int("left.synthetic_lane", left->synthetic_edge_lane, -2,
               "COORD.C:1713-1722 centered 48-pixel edge lane");
    expect_int("right.synthetic_lane", right->synthetic_edge_lane, 2,
               "COORD.C:1713-1722 centered 48-pixel edge lane");
    expect_int("left.view_square", left->view_square_index, 1,
               "DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("right.view_square", right->view_square_index, 2,
               "DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("left.wall_element", left->wall_element, 0,
               "DUNVIEW.C:8007 case C00_ELEMENT_WALL");
    expect_int("right.wall_element", right->wall_element, 0,
               "DUNVIEW.C:8117 case C00_ELEMENT_WALL");
}

static void test_zone_wall_and_blit_bindings(void)
{
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);

    if (!left || !right) return;
    expect_int("left.native_wall", left->native_wall_index, 1,
               "DEFS.H:3424 C01_WALL_D0L");
    expect_int("left.flipped_wall", left->flipped_wall_index, 0,
               "DEFS.H:3423 C00_WALL_D0R");
    expect_int("right.native_wall", right->native_wall_index, 0,
               "DEFS.H:3423 C00_WALL_D0R");
    expect_int("right.flipped_wall", right->flipped_wall_index, 1,
               "DEFS.H:3424 C01_WALL_D0L");
    expect_int("left.zone", left->wall_zone, 716,
               "DEFS.H:4056 C716_ZONE_WALL_D0L");
    expect_int("right.zone", right->wall_zone, 717,
               "DEFS.H:4057 C717_ZONE_WALL_D0R");
    expect_int("zones.adjacent", left->wall_zone + 1 == right->wall_zone, 1,
               "DEFS.H:4040-4057 wall-zone family");
    expect_int("left.frame_row", left->wall_frame_row, 10,
               "DUNVIEW.C:593 G0163 D0L row");
    expect_int("right.frame_row", right->wall_frame_row, 11,
               "DUNVIEW.C:594 G0163 D0R row");
    expect_int("left.frame_left", left->wall_frame_left, 0,
               "DUNVIEW.C:593 G0163 D0L x1");
    expect_int("left.frame_right", left->wall_frame_right, 31,
               "DUNVIEW.C:593 G0163 D0L x2");
    expect_int("right.frame_left", right->wall_frame_left, 192,
               "DUNVIEW.C:594 G0163 D0R x1");
    expect_int("right.frame_right", right->wall_frame_right, 223,
               "DUNVIEW.C:594 G0163 D0R x2");
    expect_int("left.f0104", left->uses_f0104_native_blit, 1,
               "DUNVIEW.C:8033 F0104 native C716 route");
    expect_int("right.f0104", right->uses_f0104_native_blit, 0,
               "DUNVIEW.C:8127 chooses F0105 for parity check");
    expect_int("left.f0105", left->uses_f0105_parity_scratch_flip, 0,
               "DUNVIEW.C:8033 native route selected");
    expect_int("right.f0105", right->uses_f0105_parity_scratch_flip, 1,
               "DUNVIEW.C:8127 F0105 parity scratch flip");
    expect_int("left.c10", left->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("right.c10", right->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("left.preserve_c10", left->preserves_c10_transparency, 1,
               "DUNVIEW.C:3113-3156 F0104");
    expect_int("right.preserve_c10", right->preserves_c10_transparency, 1,
               "DUNVIEW.C:3185-3247 F0105");
}

static void test_edge_mapping_and_pixels(void)
{
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);
    uint8_t source[DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 *
                   DM1_V1_D0L2_D0R2_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t screen[DM1_V1_D0L2_D0R2_WALL_SCREEN_WIDTH_PC34 *
                   DM1_V1_D0L2_D0R2_WALL_SCREEN_HEIGHT_PC34];
    DM1_V1_ViewportD0L2D0R2WallPixelPc34 p;
    int source_x = -1;
    const int row = 67;

    memset(source, DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34, sizeof(source));
    memset(screen, 0xee, sizeof(screen));
    source[(size_t)row * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 0u] = 0x41;
    source[(size_t)row * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 46u] = 0x42;
    source[(size_t)row * DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 + 47u] = 0x43;

    expect_int("left.screen_x_first", left ? left->screen_x_first : -1, 0,
               "COORD.C:1713-1722/COMMAND.C:1126-1127 left edge");
    expect_int("left.screen_x_last", left ? left->screen_x_last : -1, 47,
               "COORD.C:1713-1722/COMMAND.C:1126-1127 left edge");
    expect_int("right.screen_x_first", right ? right->screen_x_first : -1, 272,
               "COORD.C:1713-1722/COMMAND.C:1126-1127 right edge");
    expect_int("right.screen_x_last", right ? right->screen_x_last : -1, 319,
               "COORD.C:1713-1722/COMMAND.C:1126-1127 right edge");
    expect_int("map.left.x0",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   left, 0, &source_x),
               0,
               "DUNVIEW.C:8033 F0104 native C716");
    expect_int("map.left.x0.source", source_x, 0,
               "native left source begins at x0");
    expect_int("map.left.x47",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   left, 47, &source_x),
               0,
               "DUNVIEW.C:8033 F0104 native C716");
    expect_int("map.left.x47.source", source_x, 47,
               "native left source ends at x47");
    expect_int("map.left.x48.clip",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   left, 48, &source_x),
               1,
               "clipped outside left edge");
    expect_int("map.right.x272",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   right, 272, &source_x),
               0,
               "DUNVIEW.C:8127 F0105 parity C717");
    expect_int("map.right.x272.source", source_x, 47,
               "F0105 horizontal flip selects source x47");
    expect_int("map.right.x319",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   right, 319, &source_x),
               0,
               "DUNVIEW.C:8127 F0105 parity C717");
    expect_int("map.right.x319.source", source_x, 0,
               "F0105 horizontal flip selects source x0");
    expect_int("map.right.x271.clip",
               dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
                   right, 271, &source_x),
               1,
               "clipped outside right edge");

    expect_int("apply.left.opaque",
               dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                   left, source, sizeof(source), screen, sizeof(screen), 0, row, &p),
               0,
               "DUNVIEW.C:8033 F0104 native C716");
    expect_int("pixel.left.value", p.pixel_after, 0x41,
               "F0104 opaque source writes destination");
    expect_int("pixel.left.source_x", p.source_x, 0,
               "native source x0");
    expect_int("apply.left.transparent",
               dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                   left, source, sizeof(source), screen, sizeof(screen), 1, row, &p),
               0,
               "DUNVIEW.C:3113-3156 F0104 C10");
    expect_int("pixel.left.transparent_skip", p.transparent_skip, 1,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("pixel.left.transparent_after", p.pixel_after, 0xee,
               "C10 preserves destination");
    expect_int("apply.right.flip",
               dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                   right, source, sizeof(source), screen, sizeof(screen), 272, row, &p),
               0,
               "DUNVIEW.C:8127 F0105 parity C717");
    expect_int("pixel.right.uses_scratch", p.uses_scratch, 1,
               "DUNVIEW.C:3185-3247 F0105 scratch flip");
    expect_int("pixel.right.source_x", p.source_x, 47,
               "F0105 flipped selected source x47");
    expect_int("pixel.right.value", p.pixel_after, 0x43,
               "F0105 flipped opaque source writes destination");
    expect_int("apply.right.flip.next",
               dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                   right, source, sizeof(source), screen, sizeof(screen), 273, row, &p),
               0,
               "DUNVIEW.C:8127 F0105 parity C717");
    expect_int("pixel.right.next_source_x", p.source_x, 46,
               "F0105 flipped selected source x46");
    expect_int("pixel.right.next_value", p.pixel_after, 0x42,
               "F0105 flipped opaque source writes destination");
    expect_int("apply.right.clip",
               dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                   right, source, sizeof(source), screen, sizeof(screen), 271, row, &p),
               1,
               "right clipped edge");
    expect_int("pixel.right.clip_metadata", p.no_write_metadata, 1,
               "right clipped edge records no write");
    expect_int("blend.c10",
               dm1_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(0x55, 10, 10),
               0x55,
               "DEFS.H:2088 C10 transparent preserve");
    expect_int("blend.opaque",
               dm1_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(0x55, 0x66, 10),
               0x66,
               "DUNVIEW.C:3113-3156/3185-3247 opaque write");
}

static void test_probe_and_keepouts(void)
{
    DM1_V1_ViewportD0L2D0R2WallProbePc34 run;
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
            DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);

    expect_int("probe.return",
               dm1_v1_viewport_d0l2_d0r2_wall_probe_pc34_compat(&run),
               0,
               "synthetic 224x136/320x200 source-lock probe");
    expect_int("probe.ok", run.ok, 1, "probe result");
    expect_int("probe.route_count", run.route_count, 2,
               "DUNVIEW.C:8534-8542 F0128 D0 side pair");
    expect_int("probe.assertion_contract", run.assertion_contract_count, 72,
               "test requires at least 60 anchored assertions");
    expect_int("probe.f0098", run.f0098_row_owned, 1,
               "DUNVIEW.C:2962-3003 F0098 row ownership");
    expect_int("probe.f0128", run.f0128_dispatch_ok, 1,
               "DUNVIEW.C:8534-8542 F0128 dispatch line");
    expect_int("probe.m610_m611", run.m610_m611_zone_binding_ok, 1,
               "DEFS.H:2597-2598 and 4056-4057");
    expect_int("probe.f0104", run.f0104_native_route_ok, 1,
               "DUNVIEW.C:3113-3156 F0104 native route");
    expect_int("probe.f0105", run.f0105_parity_route_ok, 1,
               "DUNVIEW.C:3185-3247 F0105 parity scratch flip");
    expect_int("probe.c10", run.c10_transparency_ok, 1,
               "DEFS.H:2088 C10 transparency preservation");
    expect_int("probe.edge_clip", run.edge_clip_ok, 1,
               "COORD.C:1713-1722 320/224 clipped edge");
    expect_int("probe.keepouts", run.no_f0111_no_f0115_no_f0108_ok, 1,
               "DUNVIEW.C:8038/8144 WALL cases return");
    expect_int("probe.nonduplicative", run.nonduplicative_ok, 1,
               "DUNVIEW.C:8503-8508 and F0115 keep-out");
    expect_int("probe.check_count", (int)run.check_count, 9,
               "synthetic edge check set");
    expect_int("probe.left.x0", run.checks[0].pixel_after, 0x21,
               "DUNVIEW.C:8033 F0104 native left edge");
    expect_int("probe.left.transparent", run.checks[1].transparent_skip, 1,
               "DEFS.H:2088 C10");
    expect_int("probe.left.x47", run.checks[2].pixel_after, 0x22,
               "DUNVIEW.C:8033 F0104 native left edge");
    expect_int("probe.left.x48_clip", run.checks[3].no_write_metadata, 1,
               "clipped outside X 0..47");
    expect_int("probe.right.x272", run.checks[4].pixel_after, 0x32,
               "DUNVIEW.C:8127 F0105 parity right edge");
    expect_int("probe.right.x273", run.checks[5].pixel_after, 0x33,
               "DUNVIEW.C:8127 F0105 parity right edge");
    expect_int("probe.right.x319", run.checks[6].pixel_after, 0x31,
               "DUNVIEW.C:8127 F0105 parity right edge");
    expect_int("probe.right.x271_clip", run.checks[7].no_write_metadata, 1,
               "clipped outside X 272..319");
    expect_int("probe.right.x320_clip", run.checks[8].no_write_metadata, 1,
               "clipped outside X 272..319");
    expect_int("left.no_f0111", left ? left->wall_case_returns_before_f0111 : 0, 1,
               "DUNVIEW.C:8007-8038 no F0111");
    expect_int("right.no_f0111", right ? right->wall_case_returns_before_f0111 : 0, 1,
               "DUNVIEW.C:8117-8144 no F0111");
    expect_int("left.no_f0115", left ? left->wall_case_returns_before_f0115 : 0, 1,
               "DUNVIEW.C:8038 before F0115");
    expect_int("right.no_f0115", right ? right->wall_case_returns_before_f0115 : 0, 1,
               "DUNVIEW.C:8144 before F0115");
    expect_int("left.no_f0108", left ? left->wall_case_returns_before_f0108 : 0, 1,
               "DUNVIEW.C:8007-8038 no F0108");
    expect_int("right.no_f0108", right ? right->wall_case_returns_before_f0108 : 0, 1,
               "DUNVIEW.C:8117-8144 no F0108");
    expect_int("probe.null",
               dm1_v1_viewport_d0l2_d0r2_wall_probe_pc34_compat(NULL),
               -1,
               "null probe rejected");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34();

    expect_contains("evidence.contract", e, "contract-only",
                    "source evidence");
    expect_contains("evidence.drawview", e, "DRAWVIEW.C:709-857 F0097",
                    "DRAWVIEW.C F0097");
    expect_contains("evidence.f0098", e, "DUNVIEW.C:2962-3003 F0098",
                    "DUNVIEW.C F0098");
    expect_contains("evidence.f0128.full", e, "DUNVIEW.C:8318-8542 F0128",
                    "DUNVIEW.C F0128");
    expect_contains("evidence.f0128.d2l2", e, "DUNVIEW.C:8503-8508",
                    "existing D2L2/D2R2 branch");
    expect_contains("evidence.f0128.d0", e, "DUNVIEW.C:8534-8542",
                    "D0 side dispatch");
    expect_contains("evidence.f0125", e, "DUNVIEW.C:7960-8062 F0125",
                    "DUNVIEW.C F0125");
    expect_contains("evidence.f0125.wall", e, "8007-8038",
                    "DUNVIEW.C F0125 wall case");
    expect_contains("evidence.f0126", e, "DUNVIEW.C:8064-8162 F0126",
                    "DUNVIEW.C F0126");
    expect_contains("evidence.f0126.wall", e, "8117-8144",
                    "DUNVIEW.C F0126 wall case");
    expect_contains("evidence.f0127", e, "DUNVIEW.C:8164-8294 F0127",
                    "DUNVIEW.C F0127 follow-up");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3156 F0104",
                    "DUNVIEW.C F0104");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3247 F0105",
                    "DUNVIEW.C F0105");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H C10");
    expect_contains("evidence.m610", e, "DEFS.H:2597-2598",
                    "DEFS.H M610/M611");
    expect_contains("evidence.d2l2", e, "DEFS.H:2605-2606",
                    "DEFS.H D2L2/D2R2 view squares");
    expect_contains("evidence.c00c01", e, "DEFS.H:3423-3424",
                    "DEFS.H D0 wall ids");
    expect_contains("evidence.c05c06", e, "DEFS.H:3428-3429",
                    "DEFS.H D2L2/D2R2 wall ids");
    expect_contains("evidence.zones", e, "DEFS.H:4040-4057",
                    "DEFS.H wall zones");
    expect_contains("evidence.coord", e, "COORD.C:1713-1722",
                    "COORD.C 320x200/224x136");
    expect_contains("evidence.command", e, "COMMAND.C:1126-1127",
                    "COMMAND.C centered viewport formula");
    expect_contains("evidence.no_f0115", e, "return before F0115",
                    "DUNVIEW.C:8038/8144 no F0115");
    expect_contains("evidence.no_d0ldr", e, "X 0..47 and X 272..319",
                    "nonduplicative edge writes");
}

int main(void)
{
    test_spec_identity_and_dispatch();
    test_zone_wall_and_blit_bindings();
    test_edge_mapping_and_pixels();
    test_probe_and_keepouts();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l2_d0r2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures,
               g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l2_d0r2_wall_pc34_compat %d/%d assertions\n",
           g_assertions,
           g_assertions);
    return 0;
}
