#include "dm1_v1_viewport_d0c_center_field_pc34_compat.h"

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

static void expect_contains(const char *id, const char *haystack, const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_d0c_f0127_dispatch_route(void)
{
    const DM1_V1_D0CCenterFieldSpecPc34 *spec =
        dm1_v1_viewport_d0c_center_field_pc34_compat_spec();

    expect_nonnull("d0c.spec", spec, "DUNVIEW.C F0127:8164");
    if (!spec) return;

    expect_bool("d0c.calls_f0127", spec->calls_f0127, true, "DUNVIEW.C:8542");
    expect_int("d0c.view_square", (int)spec->view_square, 9, "DEFS.H:2587 M609_VIEW_SQUARE_D0C");
    expect_int("d0c.view_square_macro", spec->view_square_macro_value, 9, "DEFS.H:2587 M609_VIEW_SQUARE_D0C");
    expect_contains("d0c.dispatch_function", spec->draw_square_function,
                    "F0127_DUNGEONVIEW_DrawSquareD0C", "DUNVIEW.C:8542");
    expect_contains("d0c.dispatch_source", spec->dispatch_source_lines, "8542", "DUNVIEW.C:8542");
    expect_int("d0c.viewport_width",
               DM1_V1_D0C_CENTER_FIELD_PC34_VIEWPORT_WIDTH, 224, "DUNVIEW.C G0163:581-594");
    expect_int("d0c.viewport_height",
               DM1_V1_D0C_CENTER_FIELD_PC34_VIEWPORT_HEIGHT, 136, "DUNVIEW.C G0163:581-594");
}

static void test_d0c_has_no_wall_bitmap_route(void)
{
    const DM1_V1_D0CCenterFieldSpecPc34 *spec =
        dm1_v1_viewport_d0c_center_field_pc34_compat_spec();

    expect_nonnull("d0c.no_wall.spec", spec, "DUNVIEW.C:5675-5683");
    if (!spec) return;

    expect_bool("d0c.no_f0100_wall_bitmap", spec->calls_f0100_wall_bitmap, false,
                "DUNVIEW.C:3048-3058; DUNVIEW.C:8164-8310");
    expect_bool("d0c.no_f0105_wall_scratch_flip", spec->calls_f0105_scratch_flip_for_wall, false,
                "DUNVIEW.C:3185-3195; DUNVIEW.C:8164-8310");
    expect_bool("d0c.no_f0107_wall_ornament", spec->calls_f0107_wall_ornament, false,
                "DUNVIEW.C:3502-3512; DUNVIEW.C:8164-8310");
    expect_bool("d0c.no_c_wall_ordinal", spec->has_c_wall_ordinal, false,
                "DEFS.H:2587; DEFS.H has no C*_WALL ordinal for D0C");
    expect_int("d0c.wall_ordinal", spec->wall_ordinal, -1,
               "DEFS.H:2587; no C*_WALL ordinal for D0C");
    expect_bool("d0c.wall_case_returns_false", spec->wall_case_returns, false,
                "DUNVIEW.C:5675-5683");
    expect_contains("d0c.no_wall_source_5675", spec->no_wall_source_lines, "5675-5683",
                    "DUNVIEW.C:5675-5683");
    expect_contains("d0c.no_wall_source_8164", spec->no_wall_source_lines, "8164-8310",
                    "DUNVIEW.C:8164-8310");
}

static void test_d0c_wall_frame_is_metadata_only(void)
{
    const DM1_V1_D0CCenterFieldSpecPc34 *spec =
        dm1_v1_viewport_d0c_center_field_pc34_compat_spec();

    expect_nonnull("d0c.frame.spec", spec, "DUNVIEW.C:592");
    if (!spec) return;

    expect_int("d0c.frame.x1", spec->wall_frame.x1, 0, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.x2", spec->wall_frame.x2, 223, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.y1", spec->wall_frame.y1, 0, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.y2", spec->wall_frame.y2, 135, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.byte_width", spec->wall_frame.byte_width, 0, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.height", spec->wall_frame.height, 0, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.blit_x", spec->wall_frame.blit_x, 0, "DUNVIEW.C:592 G0163 D0C");
    expect_int("d0c.frame.blit_y", spec->wall_frame.blit_y, 0, "DUNVIEW.C:592 G0163 D0C");
}

static void test_d0c_field_route_and_transparency(void)
{
    const DM1_V1_D0CCenterFieldSpecPc34 *spec =
        dm1_v1_viewport_d0c_center_field_pc34_compat_spec();

    expect_nonnull("d0c.field.spec", spec, "DUNVIEW.C:8295-8310");
    if (!spec) return;

    expect_bool("d0c.calls_f0113_field", spec->calls_f0113_field, true,
                "DUNVIEW.C:8295-8310 F0113_DUNGEONVIEW_DrawField");
    expect_int("d0c.field_zone_c713", spec->media508_field_zone, 713,
               "DEFS.H:4036; DUNVIEW.C:8305");
    expect_int("d0c.field_zone_c715", spec->media720_field_zone, 715,
               "DEFS.H:4055; DUNVIEW.C:8308");
    expect_int("d0c.transparent_color", spec->transparent_color,
               DM1_V1_D0C_CENTER_FIELD_PC34_C10_COLOR_FLESH,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d0c.transparent_color_value", spec->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_bool("d0c.field_blit_preserves_c10", spec->field_blit_preserves_c10_transparency, true,
                "DUNVIEW.C:8305; DEFS.H:2088");
    expect_contains("d0c.field_source_8295", spec->field_source_lines, "8295-8310",
                    "DUNVIEW.C:8295-8310");
}

static void test_d0c_non_door_and_non_extra_thing_contract(void)
{
    const DM1_V1_D0CCenterFieldSpecPc34 *spec =
        dm1_v1_viewport_d0c_center_field_pc34_compat_spec();

    expect_nonnull("d0c.thing_contract.spec", spec, "DUNVIEW.C:8294-8310");
    if (!spec) return;

    expect_bool("d0c.no_f0111_door", spec->calls_f0111_door, false,
                "DUNVIEW.C:4218-4226; DUNVIEW.C:8164-8310");
    expect_bool("d0c.no_extra_f0115_after_field", spec->calls_extra_f0115_thing_pass_after_field, false,
                "DUNVIEW.C:8294-8310");
    expect_bool("d0c.standard_f0115_precedes_field", spec->standard_f0115_precedes_field, true,
                "DUNVIEW.C:8294");
    expect_int("d0c.cell_order", (int)spec->cell_order, 0x0021,
               "DUNVIEW.C:8294 C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT");
    expect_contains("d0c.thing_contract_no_f0111", spec->thing_pass_contract_source_lines, "no F0111",
                    "DUNVIEW.C:8164-8310");
    expect_contains("d0c.thing_contract_no_extra_f0115", spec->thing_pass_contract_source_lines,
                    "no extra F0115", "DUNVIEW.C:8294-8310");
    expect_bool("d0c.contract_only_marker", spec->contract_only, true,
                "Source-locked contract gate only");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = dm1_v1_viewport_d0c_center_field_pc34_compat_source_evidence();

    expect_nonnull("evidence.nonnull", e, "source evidence");
    expect_contains("evidence.contract_only", e, "Source-locked contract gate only",
                    "contract marker");
    expect_contains("evidence.no_full_asset_parity", e, "not full real-asset wall/field parity",
                    "contract marker");
    expect_contains("evidence.f0115_no_wall_path", e, "5675-5683", "DUNVIEW.C:5675-5683");
    expect_contains("evidence.g0163_d0c_frame", e, "D0C line 592", "DUNVIEW.C:592");
    expect_contains("evidence.f0127", e, "F0127_DUNGEONVIEW_DrawSquareD0C",
                    "DUNVIEW.C:8164; DUNVIEW.C:8542");
    expect_contains("evidence.no_f0100", e, "no F0100_DUNGEONVIEW_DrawWallSetBitmap",
                    "DUNVIEW.C:3048-3058");
    expect_contains("evidence.no_f0105", e,
                    "no F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
                    "DUNVIEW.C:3185-3195");
    expect_contains("evidence.no_f0107", e,
                    "no F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
                    "DUNVIEW.C:3502-3512");
    expect_contains("evidence.no_f0111", e, "no F0111_DUNGEONVIEW_DrawDoor",
                    "DUNVIEW.C:4218-4226");
    expect_contains("evidence.f0113", e, "F0113_DUNGEONVIEW_DrawField",
                    "DUNVIEW.C:8295-8310");
    expect_contains("evidence.c713", e, "C713_ZONE_WALL_D0C",
                    "DEFS.H:4036; DUNVIEW.C:8305");
    expect_contains("evidence.c715", e, "C715_ZONE_WALL_D0C",
                    "DEFS.H:4055; DUNVIEW.C:8308");
    expect_contains("evidence.m609", e, "M609_VIEW_SQUARE_D0C",
                    "DEFS.H:2587");
    expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                    "DEFS.H:2088");
    expect_contains("evidence.no_wall_ordinal", e, "no C*_WALL ordinal",
                    "DEFS.H:2587");
}

int main(void)
{
    test_d0c_f0127_dispatch_route();
    test_d0c_has_no_wall_bitmap_route();
    test_d0c_wall_frame_is_metadata_only();
    test_d0c_field_route_and_transparency();
    test_d0c_non_door_and_non_extra_thing_contract();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
