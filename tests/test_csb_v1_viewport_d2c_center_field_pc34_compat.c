#include "csb_v1_viewport_d2c_center_field_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_nonnull(const char *id, const void *got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_d2c_i34_dispatch_and_index_contract(void)
{
    const CSB_V1_D2CCenterFieldSpecPc34 *spec =
        M11_GameView_ViewportD2CCenterFieldPc34Spec();

    expect_nonnull("d2c.spec", spec, "ReDMCSB DUNVIEW.C:7244");
    if (!spec) return;

    expect_bool("d2c.calls_draw_square", spec->calls_d2c_draw_square, true,
                "ReDMCSB DUNVIEW.C:8520-8521");
    expect_contains("d2c.draw_function", spec->draw_square_function,
                    "F0121_DUNGEONVIEW_DrawSquareD2C", "ReDMCSB DUNVIEW.C:7244");
    expect_contains("d2c.draw_function_i34_f0128", spec->draw_square_function,
                    "I34 F0128 D2C dispatch", "ReDMCSB DUNVIEW.C:8520-8521");
    expect_int("d2c.view_square", (int)spec->view_square, 6,
               "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("d2c.view_square_macro", spec->view_square_macro_value, 6,
               "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("d2c.redmcsb_index", spec->redmcsb_view_square_index, 6,
               "ReDMCSB DUNVIEW.C:370 G2026/G2027 index");
    expect_int("d2c.view_depth", spec->view_depth, 2,
               "ReDMCSB DUNVIEW.C:372 G2027[6]");
    expect_int("d2c.view_lane", spec->view_lane, 0,
               "ReDMCSB DUNVIEW.C:371 G2026[6]");
    expect_int("d2c.field_aspect", spec->field_aspect_index, 7,
               "ReDMCSB DUNVIEW.C:377 G2035[6]");
    expect_contains("d2c.dispatch_source", spec->dispatch_source_lines, "8520-8521",
                    "ReDMCSB DUNVIEW.C:8520-8521");
}

static void test_d2c_no_wall_door_route_contract(void)
{
    const CSB_V1_D2CCenterFieldSpecPc34 *spec =
        M11_GameView_ViewportD2CCenterFieldPc34Spec();

    expect_nonnull("d2c.no_wall.spec", spec, "ReDMCSB DUNVIEW.C:7353-7388");
    if (!spec) return;

    expect_bool("d2c.no_f0100_wall_bitmap", spec->calls_f0100_wall_bitmap, false,
                "ReDMCSB DUNVIEW.C:7289-7312; 7353-7388");
    expect_bool("d2c.no_f0105_wall_flip", spec->calls_f0105_scratch_flip_for_wall, false,
                "ReDMCSB DUNVIEW.C:7327-7330 door case; 7353-7388 no-wall route");
    expect_bool("d2c.no_f0107_wall_ornament", spec->calls_f0107_wall_ornament, false,
                "ReDMCSB DUNVIEW.C:7308; 7353-7388 no-wall route");
    expect_bool("d2c.no_f0111_door", spec->calls_f0111_door, false,
                "ReDMCSB DUNVIEW.C:7338-7339; 7353-7388 no-wall route");
    expect_bool("d2c.calls_f0113_field", spec->calls_f0113_field, true,
                "ReDMCSB DUNVIEW.C:7386");
    expect_bool("d2c.only_field_surface_f0113", spec->only_field_surface_call_is_f0113, true,
                "ReDMCSB DUNVIEW.C:7353-7388");
    expect_bool("d2c.standard_f0115_precedes_field", spec->standard_f0115_precedes_field, true,
                "ReDMCSB DUNVIEW.C:7356-7368");
    expect_bool("d2c.floor_ceiling_prework", spec->ordinary_floor_ceiling_prework, true,
                "ReDMCSB DUNVIEW.C:7357-7365");
    expect_int("d2c.cell_order", (int)spec->cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:7356 C0x3421");
    expect_contains("d2c.center_route_source", spec->center_route_source_lines, "7353-7388",
                    "ReDMCSB DUNVIEW.C:7353-7388");
    expect_contains("d2c.field_source", spec->field_source_lines, "7386",
                    "ReDMCSB DUNVIEW.C:7386");
}

static void test_d2c_wall_zone_and_frame_anchors(void)
{
    const CSB_V1_D2CCenterFieldSpecPc34 *spec =
        M11_GameView_ViewportD2CCenterFieldPc34Spec();

    expect_nonnull("d2c.zone.spec", spec, "ReDMCSB DEFS.H:4030/4049");
    if (!spec) return;

    expect_bool("d2c.has_wall_ordinal", spec->has_c_wall_ordinal, true,
                "ReDMCSB DEFS.H:3432 C09_WALL_D2C");
    expect_int("d2c.wall_ordinal", spec->wall_ordinal, 9,
               "ReDMCSB DEFS.H:3432 C09_WALL_D2C");
    expect_int("d2c.media508_zone", spec->media508_field_zone, 707,
               "ReDMCSB DEFS.H:4030 C707_ZONE_WALL_D2C");
    expect_int("d2c.media720_zone", spec->media720_field_zone, 709,
               "ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C");
    expect_int("d2c.c702_base", spec->media720_base_zone_c702, 702,
               "ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2");
    expect_int("d2c.c703_next", spec->media720_next_zone_c703, 703,
               "ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2");
    expect_int("d2c.zone_from_c702_spec", spec->media720_field_zone_from_c702, 709,
               "ReDMCSB DUNVIEW.C:6219 C702 + field aspect");
    expect_int("d2c.zone_from_c702_helper",
               M11_GameView_ViewportD2CCenterFieldPc34ZoneFromC702Base(spec), 709,
               "ReDMCSB DUNVIEW.C:7386 and DEFS.H:4042/4049");
    expect_int("d2c.zone_from_c702_null",
               M11_GameView_ViewportD2CCenterFieldPc34ZoneFromC702Base(NULL), -1,
               "route helper rejects unresolved spec");
    expect_int("d2c.frame.x1", spec->wall_frame.x1, 60, "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.x2", spec->wall_frame.x2, 163, "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.y1", spec->wall_frame.y1, 20, "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.y2", spec->wall_frame.y2, 90, "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.byte_width", spec->wall_frame.byte_width, 72,
               "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.height", spec->wall_frame.height, 71,
               "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.blit_x", spec->wall_frame.blit_x, 16,
               "ReDMCSB DUNVIEW.C:586");
    expect_int("d2c.frame.blit_y", spec->wall_frame.blit_y, 0,
               "ReDMCSB DUNVIEW.C:586");
    expect_bool("d2c.wall_case_returns_before_field", spec->wall_case_returns_before_field, true,
                "ReDMCSB DUNVIEW.C:7312");
}

static void test_d2c_c10_transparency_contract(void)
{
    const CSB_V1_D2CCenterFieldSpecPc34 *spec =
        M11_GameView_ViewportD2CCenterFieldPc34Spec();
    uint8_t source[8] = { 10, 1, 2, 10, 3, 4, 10, 5 };
    uint8_t destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };

    expect_nonnull("d2c.c10.spec", spec, "ReDMCSB DEFS.H:2088");
    if (!spec) return;

    expect_int("d2c.transparent_color", spec->transparent_color,
               CSB_V1_D2C_CENTER_FIELD_PC34_C10_COLOR_FLESH,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d2c.transparent_color_value", spec->transparent_color, 10,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_bool("d2c.field_blit_preserves_c10",
                spec->field_blit_preserves_c10_transparency, true,
                "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d2c.blit.copied",
               M11_GameView_ViewportD2CCenterFieldPc34ApplySyntheticC10FieldBlit(
                   spec, source, 4, destination, 4, 4, 2),
               5, "synthetic F0113 C10 field contract");
    expect_int("d2c.blit.transparent0", destination[0], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d2c.blit.pixel1", destination[1], 1, "synthetic field copy");
    expect_int("d2c.blit.pixel2", destination[2], 2, "synthetic field copy");
    expect_int("d2c.blit.transparent3", destination[3], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d2c.blit.pixel4", destination[4], 3, "synthetic field copy");
    expect_int("d2c.blit.pixel5", destination[5], 4, "synthetic field copy");
    expect_int("d2c.blit.transparent6", destination[6], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d2c.blit.pixel7", destination[7], 5, "synthetic field copy");
    expect_int("d2c.blit.reject_null",
               M11_GameView_ViewportD2CCenterFieldPc34ApplySyntheticC10FieldBlit(
                   NULL, source, 4, destination, 4, 4, 2),
               -1, "route helper rejects unresolved spec");
    expect_bool("d2c.contract_only_marker", spec->contract_only, true,
                "Source-locked contract gate only");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = M11_GameView_ViewportD2CCenterFieldPc34SourceEvidence();
    const CSB_V1_D2CCenterFieldSpecPc34 *spec =
        M11_GameView_ViewportD2CCenterFieldPc34Spec();

    expect_nonnull("evidence.nonnull", e, "source evidence");
    expect_contains("evidence.contract_only", e, "Source-locked contract gate only",
                    "contract marker");
    expect_contains("evidence.no_full_asset_parity", e,
                    "not full real-asset field bitmap parity", "contract marker");
    expect_contains("evidence.dunview_arrays", e, "DUNVIEW.C:370-377",
                    "ReDMCSB DUNVIEW.C:370-377");
    expect_contains("evidence.f0128_dispatch", e, "I34 F0128 dispatcher",
                    "ReDMCSB DUNVIEW.C:8520-8521");
    expect_contains("evidence.center_route", e, "DUNVIEW.C:7353-7388",
                    "ReDMCSB DUNVIEW.C:7353-7388");
    expect_contains("evidence.no_f0100", e, "F0100_DUNGEONVIEW_DrawWallSetBitmap",
                    "ReDMCSB DUNVIEW.C:7289-7312");
    expect_contains("evidence.no_f0105", e,
                    "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
                    "ReDMCSB DUNVIEW.C:7327-7330");
    expect_contains("evidence.no_f0107", e,
                    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
                    "ReDMCSB DUNVIEW.C:7308");
    expect_contains("evidence.no_f0111", e, "F0111_DUNGEONVIEW_DrawDoor",
                    "ReDMCSB DUNVIEW.C:7338-7339");
    expect_contains("evidence.f0113", e, "F0113_DUNGEONVIEW_DrawField",
                    "ReDMCSB DUNVIEW.C:7386");
    expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                    "ReDMCSB DEFS.H:2088");
    expect_contains("evidence.c707", e, "C707", "ReDMCSB DEFS.H:4030");
    expect_contains("evidence.c709", e, "C709_ZONE_WALL_D2C",
                    "ReDMCSB DEFS.H:4049");
    expect_contains("evidence.c702", e, "C702_ZONE_WALL_D3L2",
                    "ReDMCSB DEFS.H:4042");
    expect_contains("evidence.c703", e, "C703_ZONE_WALL_D3R2",
                    "ReDMCSB DEFS.H:4043");
    expect_contains("evidence.lineage", e, "CSB-lineage Viewport.cpp:1151-1156",
                    "CSB-lineage Viewport.cpp");
    expect_contains("d2c.lineage_source", spec ? spec->lineage_source_lines : NULL,
                    "1414-1420", "CSB-lineage Viewport.cpp:1414-1420");
}

int main(void)
{
    test_d2c_i34_dispatch_and_index_contract();
    test_d2c_no_wall_door_route_contract();
    test_d2c_wall_zone_and_frame_anchors();
    test_d2c_c10_transparency_contract();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
