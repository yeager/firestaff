#include "dm1_v1_viewport_d0c_ceiling_f0098_pc34_compat.h"

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

static void expect_contains(const char *id, const char *haystack, const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void expect_apply(const char *id,
                         const DM1_V1_D0CCeilingF0098PixelProbePc34 *probe,
                         DM1_V1_D0CCeilingF0098PixelResultPc34 *out,
                         const char *anchor)
{
    expect_int(id, dm1_v1_viewport_d0c_ceiling_f0098_apply_pixel_pc34(probe, out) ? 1 : 0,
               1, anchor);
}

static void test_f0098_row_contract_boundaries(void)
{
    const DM1_V1_D0CCeilingF0098SpecPc34 *spec =
        dm1_v1_viewport_d0c_ceiling_f0098_spec_pc34();

    expect_bool("spec.contract_only", spec->contract_only, true,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_bool("spec.no_real_asset_parity", spec->real_asset_bitmap_parity, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_int("spec.viewport_width", spec->viewport_width, 224,
               "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_int("spec.viewport_height", spec->viewport_height, 136,
               "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_int("spec.ceiling_first", spec->ceiling_first_row, 0,
               "DUNVIEW.C F0098:2962-3002 D0C top row index");
    expect_int("spec.ceiling_last", spec->ceiling_last_row, 28,
               "DUNVIEW.C F0098:2962-3002 ceiling 29-row copy");
    expect_int("spec.gap_first", spec->gap_first_row, 29,
               "DUNVIEW.C F0098:2962-3002 black-area row boundary");
    expect_int("spec.gap_last", spec->gap_last_row, 65,
               "DUNVIEW.C F0098:2962-3002 black-area row boundary");
    expect_int("spec.floor_first", spec->floor_first_row, 66,
               "DUNVIEW.C F0098:2962-3002 floor 70-row copy");
    expect_int("spec.floor_last", spec->floor_last_row, 135,
               "DUNVIEW.C F0098:2962-3002 D0C bottom row index");
    expect_int("spec.depth", spec->depth, 0,
               "DUNVIEW.C F0098:2962-3002 D0C depth 0 row-ownership contract");
    expect_int("spec.lane", spec->lane, 0,
               "DUNVIEW.C F0098:2962-3002 D0C lane 0 row-ownership contract");
    expect_int("spec.ceiling_zone", spec->viewport_ceiling_zone, 700,
               "DEFS.H:4041-4043 viewport ceiling/floor and C702/C703 anchors");
    expect_int("spec.floor_zone", spec->viewport_floor_zone, 701,
               "DEFS.H:4041-4043 viewport floor zone C701");
    expect_int("spec.d3l2_anchor", spec->d3l2_wall_zone_anchor, 702,
               "DEFS.H:4041-4043 C702 anchor");
    expect_int("spec.d3r2_anchor", spec->d3r2_wall_zone_anchor, 703,
               "DEFS.H:4041-4043 C703 anchor");
}

static void test_ceiling_floor_rows_write_only_in_f0098_owned_bands(void)
{
    DM1_V1_D0CCeilingF0098PixelResultPc34 out;
    DM1_V1_D0CCeilingF0098PixelProbePc34 ceiling_top = {
        0, DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_BASE_PC34, 0x11, 0x31
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 ceiling_bottom = {
        28, DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_BASE_PC34, 0x11, 0x32
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 ceiling_gap = {
        29, DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_BASE_PC34, 0x11, 0x33
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 floor_gap = {
        65, DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_BASE_PC34, 0x22, 0x41
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 floor_top = {
        66, DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_BASE_PC34, 0x22, 0x42
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 floor_bottom = {
        135, DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_BASE_PC34, 0x22, 0x43
    };

    expect_apply("ceiling.top.apply", &ceiling_top, &out,
                 "DUNVIEW.C F0098:2962-3002 ceiling top row");
    expect_int("ceiling.top.owner", out.row_owner,
               DM1_V1_D0C_CEILING_F0098_ROW_CEILING_PC34,
               "DUNVIEW.C F0098:2962-3002 ceiling top row");
    expect_bool("ceiling.top.writes", out.writes_pixel, true,
                "DUNVIEW.C F0098:2962-3002 ceiling row write");
    expect_int("ceiling.top.pixel", out.pixel_after, 0x31,
               "DUNVIEW.C F0098:2962-3002 ceiling row write");

    expect_apply("ceiling.bottom.apply", &ceiling_bottom, &out,
                 "DUNVIEW.C F0098:2962-3002 ceiling bottom row");
    expect_bool("ceiling.bottom.writes", out.writes_pixel, true,
                "DUNVIEW.C F0098:2962-3002 ceiling row write");
    expect_int("ceiling.bottom.pixel", out.pixel_after, 0x32,
               "DUNVIEW.C F0098:2962-3002 ceiling row write");

    expect_apply("ceiling.gap.apply", &ceiling_gap, &out,
                 "DUNVIEW.C F0098:2962-3002 no ceiling write in gap");
    expect_int("ceiling.gap.owner", out.row_owner,
               DM1_V1_D0C_CEILING_F0098_ROW_GAP_PC34,
               "DUNVIEW.C F0098:2962-3002 black-area row");
    expect_bool("ceiling.gap.no_write", out.writes_pixel, false,
                "DUNVIEW.C F0098:2962-3002 no ceiling write in gap");
    expect_int("ceiling.gap.pixel_preserved", out.pixel_after, 0x11,
               "DUNVIEW.C F0098:2962-3002 no ceiling write in gap");

    expect_apply("floor.gap.apply", &floor_gap, &out,
                 "DUNVIEW.C F0098:2962-3002 no floor write before floor band");
    expect_bool("floor.gap.no_write", out.writes_pixel, false,
                "DUNVIEW.C F0098:2962-3002 no floor write before floor band");
    expect_int("floor.gap.pixel_preserved", out.pixel_after, 0x22,
               "DUNVIEW.C F0098:2962-3002 no floor write before floor band");

    expect_apply("floor.top.apply", &floor_top, &out,
                 "DUNVIEW.C F0098:2962-3002 floor top row");
    expect_int("floor.top.owner", out.row_owner,
               DM1_V1_D0C_CEILING_F0098_ROW_FLOOR_PC34,
               "DUNVIEW.C F0098:2962-3002 floor top row");
    expect_bool("floor.top.writes", out.writes_pixel, true,
                "DUNVIEW.C F0098:2962-3002 floor row write");
    expect_int("floor.top.pixel", out.pixel_after, 0x42,
               "DUNVIEW.C F0098:2962-3002 floor row write");

    expect_apply("floor.bottom.apply", &floor_bottom, &out,
                 "DUNVIEW.C F0098:2962-3002 floor bottom row");
    expect_bool("floor.bottom.writes", out.writes_pixel, true,
                "DUNVIEW.C F0098:2962-3002 floor row write");
    expect_int("floor.bottom.pixel", out.pixel_after, 0x43,
               "DUNVIEW.C F0098:2962-3002 floor row write");
}

static void test_c10_transparency_and_d0c_pit_pixels_stay_in_f0098_rows(void)
{
    DM1_V1_D0CCeilingF0098PixelResultPc34 out;
    DM1_V1_D0CCeilingF0098PixelProbePc34 ceiling_pit_opaque = {
        10, DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_PIT_F0112_PC34, 0x55, 0x66
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 ceiling_pit_c10 = {
        10, DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_PIT_F0112_PC34,
        0x55, DM1_V1_D0C_CEILING_F0098_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 floor_pit_opaque = {
        90, DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_PIT_D0C_PC34, 0x77, 0x68
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 floor_pit_c10 = {
        90, DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_PIT_D0C_PC34,
        0x77, DM1_V1_D0C_CEILING_F0098_C10_COLOR_FLESH_PC34
    };

    expect_apply("ceiling_pit.opaque.apply", &ceiling_pit_opaque, &out,
                 "DUNVIEW.C F0112:4341-4470 ceiling pit dispatch");
    expect_bool("ceiling_pit.opaque.f0112", out.calls_f0112, true,
                "DUNVIEW.C F0112:4341-4470 ceiling pit dispatch");
    expect_bool("ceiling_pit.opaque.f0098_row", out.row_owned_by_f0098_d0c, true,
                "DUNVIEW.C F0098:2962-3002 D0C ceiling row ownership");
    expect_bool("ceiling_pit.opaque.no_f0108", out.calls_f0108_floor_ornament, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0108");
    expect_bool("ceiling_pit.opaque.no_f0107", out.calls_f0107_wall_ornament, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0107");
    expect_bool("ceiling_pit.opaque.writes", out.writes_pixel, true,
                "DUNVIEW.C F0112:4341-4470 opaque ceiling pit pixel");
    expect_int("ceiling_pit.opaque.pixel", out.pixel_after, 0x66,
               "DUNVIEW.C F0112:4341-4470 opaque ceiling pit pixel");

    expect_apply("ceiling_pit.c10.apply", &ceiling_pit_c10, &out,
                 "DUNVIEW.C F0112:4341-4470 C10 transparent ceiling pit pixel");
    expect_bool("ceiling_pit.c10.skip", out.transparent_skip, true,
                "DUNVIEW.C F0112:4341-4470 C10 transparency");
    expect_bool("ceiling_pit.c10.no_write", out.writes_pixel, false,
                "DUNVIEW.C F0112:4341-4470 C10 transparency");
    expect_int("ceiling_pit.c10.pixel_preserved", out.pixel_after, 0x55,
               "DUNVIEW.C F0112:4341-4470 C10 transparency");

    expect_apply("floor_pit.opaque.apply", &floor_pit_opaque, &out,
                 "DUNVIEW.C F0098:2962-3002 D0C floor row ownership");
    expect_bool("floor_pit.opaque.f0098_row", out.row_owned_by_f0098_d0c, true,
                "DUNVIEW.C F0098:2962-3002 D0C floor row ownership");
    expect_bool("floor_pit.opaque.no_f0108", out.calls_f0108_floor_ornament, false,
                "DUNVIEW.C F0098:2962-3002 D0C floor row not F0108");
    expect_bool("floor_pit.opaque.no_f0107", out.calls_f0107_wall_ornament, false,
                "DUNVIEW.C F0098:2962-3002 D0C floor row not F0107");
    expect_bool("floor_pit.opaque.writes", out.writes_pixel, true,
                "DUNVIEW.C F0098:2962-3002 D0C floor row write");
    expect_int("floor_pit.opaque.pixel", out.pixel_after, 0x68,
               "DUNVIEW.C F0098:2962-3002 D0C floor row write");

    expect_apply("floor_pit.c10.apply", &floor_pit_c10, &out,
                 "DUNVIEW.C F0112:4341-4470 C10 transparency contract");
    expect_bool("floor_pit.c10.skip", out.transparent_skip, true,
                "DUNVIEW.C F0112:4341-4470 C10 transparency contract");
    expect_int("floor_pit.c10.pixel_preserved", out.pixel_after, 0x77,
               "DUNVIEW.C F0112:4341-4470 C10 transparency contract");
}

static void test_rejects_f0108_and_f0107_for_this_d0c_slice(void)
{
    DM1_V1_D0CCeilingF0098PixelResultPc34 out;
    DM1_V1_D0CCeilingF0098PixelProbePc34 f0108 = {
        90, DM1_V1_D0C_CEILING_F0098_PIXEL_F0108_FLOOR_ORNAMENT_PC34, 0x12, 0x34
    };
    DM1_V1_D0CCeilingF0098PixelProbePc34 f0107 = {
        10, DM1_V1_D0C_CEILING_F0098_PIXEL_F0107_WALL_ORNAMENT_PC34, 0x12, 0x34
    };

    expect_apply("reject.f0108.apply", &f0108, &out,
                 "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0108");
    expect_bool("reject.f0108.flag", out.calls_f0108_floor_ornament, true,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0108");
    expect_bool("reject.f0108.no_write", out.writes_pixel, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0108");
    expect_bool("reject.f0108.not_owned", out.row_owned_by_f0098_d0c, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0108");

    expect_apply("reject.f0107.apply", &f0107, &out,
                 "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0107");
    expect_bool("reject.f0107.flag", out.calls_f0107_wall_ornament, true,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0107");
    expect_bool("reject.f0107.no_write", out.writes_pixel, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0107");
    expect_bool("reject.f0107.not_owned", out.row_owned_by_f0098_d0c, false,
                "DUNVIEW.C F0098:2962-3002 D0C row-ownership not F0107");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D0CCeilingF0098SpecPc34 *spec =
        dm1_v1_viewport_d0c_ceiling_f0098_spec_pc34();
    const char *e = dm1_v1_viewport_d0c_ceiling_f0098_source_evidence_pc34();

    expect_bool("evidence.spec_source_pointer", spec->source_lines == e, true,
                "DUNVIEW.C F0098:2962-3002 source evidence pointer");
    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "DUNVIEW.C F0098:2962-3002 contract marker");
    expect_contains("evidence.no_real_asset_parity", e,
                    "no real-asset floor/ceiling bitmap parity claim",
                    "DUNVIEW.C F0098:2962-3002 contract marker");
    expect_contains("evidence.f0098", e, "F0098:2962-3002",
                    "DUNVIEW.C F0098:2962-3002");
    expect_contains("evidence.f0112", e, "F0112:4341-4470",
                    "DUNVIEW.C F0112:4341-4470");
    expect_contains("evidence.defs", e, "DEFS.H:4041-4043",
                    "DEFS.H:4041-4043 C701/C702/C703 anchors");
    expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                    "DEFS.H:4041-4043; DEFS.H:2088 C10_COLOR_FLESH");
    expect_contains("evidence.depth_lane", e, "depth 0 lane 0",
                    "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_contains("evidence.not_f0108", e, "not F0108",
                    "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
    expect_contains("evidence.not_f0107", e, "not F0107",
                    "DUNVIEW.C F0098:2962-3002 D0C row-ownership contract");
}

int main(void)
{
    test_f0098_row_contract_boundaries();
    test_ceiling_floor_rows_write_only_in_f0098_owned_bands();
    test_c10_transparency_and_d0c_pit_pixels_stay_in_f0098_rows();
    test_rejects_f0108_and_f0107_for_this_d0c_slice();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0c_ceiling_f0098_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0c_ceiling_f0098_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
