#include "dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat.h"

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

static void test_specs_source_locked(void)
{
    const DM1_V1_D2L2D2R2WallSpecPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_wall_spec_pc34(
            DM1_V1_D2L2_D2R2_WALL_SIDE_D2L2_PC34);
    const DM1_V1_D2L2D2R2WallSpecPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_wall_spec_pc34(
            DM1_V1_D2L2_D2R2_WALL_SIDE_D2R2_PC34);

    expect_int("spec.d2l2.present", d2l2 != NULL, 1,
               "DUNVIEW.C:6837 F0678_DrawD2L2");
    expect_int("spec.d2r2.present", d2r2 != NULL, 1,
               "DUNVIEW.C:6868 F0679_DrawD2R2");
    if (!d2l2 || !d2r2) return;

    expect_int("spec.d2l2.contract_only", d2l2->contract_only ? 1 : 0, 1,
               "source-locked contract gate only");
    expect_int("spec.d2r2.contract_only", d2r2->contract_only ? 1 : 0, 1,
               "source-locked contract gate only");
    expect_int("spec.no_real_asset_parity",
               d2l2->real_asset_bitmap_parity || d2r2->real_asset_bitmap_parity, 0,
               "no real-asset bitmap parity claim");
    expect_int("spec.d2l2.view_square", d2l2->view_square_index, 9,
               "DEFS.H:2605 C09_VIEW_SQUARE_D2L2=9");
    expect_int("spec.d2r2.view_square", d2r2->view_square_index, 10,
               "DEFS.H:2606 C10_VIEW_SQUARE_D2R2=10");
    expect_int("spec.d2l2.native_wall", d2l2->native_wall_index_pc34, 6,
               "DEFS.H:3429 C06_WALL_D2L2=6");
    expect_int("spec.d2r2.native_wall", d2r2->native_wall_index_pc34, 5,
               "DEFS.H:3428 C05_WALL_D2R2=5");
    expect_int("spec.d2l2.flipped_wall", d2l2->flipped_wall_index_pc34, 5,
               "DUNVIEW.C:6851 C05_WALL_D2R2 flipped for D2L2");
    expect_int("spec.d2r2.flipped_wall", d2r2->flipped_wall_index_pc34, 6,
               "DUNVIEW.C:6882 C06_WALL_D2L2 flipped for D2R2");
    expect_int("spec.d2l2.zone", d2l2->wall_zone_pc34, 707,
               "DEFS.H:4047 C707_ZONE_WALL_D2L2");
    expect_int("spec.d2r2.zone", d2r2->wall_zone_pc34, 708,
               "DEFS.H:4048 C708_ZONE_WALL_D2R2");
    expect_int("spec.draw_order", d2l2->draw_order_index < d2r2->draw_order_index, 1,
               "DUNVIEW.C:8503-8508 F0128 calls D2L2 before D2R2");
    expect_int("spec.same_row", d2l2->viewport_y_first == d2r2->viewport_y_first &&
               d2l2->viewport_y_last == d2r2->viewport_y_last, 1,
               "D2L2/D2R2 central wall row");
    expect_int("spec.same_height", d2l2->visible_height, d2r2->visible_height,
               "both D2L2/D2R2 visible spans are 71 rows");
    expect_int("spec.d2l2.visible_width", d2l2->visible_width, 6,
               "D2L2 source x=30..35 reaches viewport x=0..5");
    expect_int("spec.d2r2.visible_width", d2r2->visible_width, 36,
               "D2R2 source x=0..35 reaches viewport x=186..221");
    expect_int("spec.c10", d2l2->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.c10.symmetry", d2l2->transparent_color == d2r2->transparent_color, 1,
               "F0104/F0105 share C10 transparent route");
    expect_int("spec.no_f0108", d2l2->calls_f0108_floor_ornament ||
               d2r2->calls_f0108_floor_ornament, 0,
               "DUNVIEW.C:6848-6893 wall case excludes F0108");
    expect_int("spec.no_f0111", d2l2->calls_f0111_door || d2r2->calls_f0111_door, 0,
               "DUNVIEW.C:6848-6893 wall case excludes F0111");
    expect_int("spec.no_f0115", d2l2->calls_f0115_thing_pass ||
               d2r2->calls_f0115_thing_pass, 0,
               "DUNVIEW.C:6848-6893 wall case returns before F0115");
}

static void test_run_pixel_contract(void)
{
    DM1_V1_D2L2D2R2WallRunPc34 run;
    const DM1_V1_D2L2D2R2WallPixelPc34 *checks;

    expect_int("run.ok",
               dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(&run) ? 1 : 0,
               1, "internal runtime gate entry point");
    checks = run.checks;
    expect_int("run.check_count", (int)run.check_count, 16,
               "native, clip, and flipped-pair pixel checks");
    expect_int("run.c10_palette", run.c10_palette_index, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("run.draw_order", run.draw_order_left_before_right ? 1 : 0, 1,
               "DUNVIEW.C:8503-8508 F0128 draw order");
    expect_int("run.mirrored_route", run.mirrored_route_pair ? 1 : 0, 1,
               "DUNVIEW.C:2442-2443 C05/C06 flipped wall-set pair");
    expect_int("run.zone_family", run.same_wall_zone_family ? 1 : 0, 1,
               "DEFS.H:4047-4048 C707/C708 adjacent zones");
    expect_int("run.same_c10", run.same_c10_transparency ? 1 : 0, 1,
               "F0104/F0105 transparent blit");
    expect_int("run.same_height_row", run.same_height_and_row ? 1 : 0, 1,
               "D2L2/D2R2 central wall row symmetry");
    expect_int("run.excludes_f0108_f0111_f0115",
               run.excludes_f0108_f0111_f0115 ? 1 : 0, 1,
               "DUNVIEW.C:6848-6893 wall cases return");

    expect_int("d2l2.native.skip.in_clip", checks[0].in_clip ? 1 : 0, 1,
               "DUNVIEW.C:6854-6858 native D2L2 route");
    expect_int("d2l2.native.skip.src_x", checks[0].source_x, 30,
               "D2L2 clipped source x starts at 30");
    expect_int("d2l2.native.skip.c10", checks[0].transparent_skip ? 1 : 0, 1,
               "C10 transparent pixel preserves viewport");
    expect_int("d2l2.native.skip.after", checks[0].pixel_after, 0xee,
               "DUNVIEW.C:3113-3129 F0104 C10 skip");
    expect_int("d2l2.native.next.value", checks[1].pixel_after, 0x42,
               "opaque D2L2 wall pixel writes");
    expect_int("d2l2.native.edge.value", checks[2].pixel_after, 0x7e,
               "D2L2 clipped right edge");
    expect_int("d2l2.native.after_clip", checks[3].no_write_metadata ? 1 : 0, 1,
               "D2L2 viewport x=6 is outside source clip");
    expect_int("d2l2.native.bottom.value", checks[4].pixel_after, 0x55,
               "D2L2 bottom row y=90");

    expect_int("d2r2.native.skip.src_x", checks[5].source_x, 0,
               "D2R2 clipped source x starts at 0");
    expect_int("d2r2.native.skip.c10", checks[5].transparent_skip ? 1 : 0, 1,
               "C10 transparent pixel preserves viewport");
    expect_int("d2r2.native.next.value", checks[6].pixel_after, 0x52,
               "opaque D2R2 wall pixel writes");
    expect_int("d2r2.native.edge.value", checks[7].pixel_after, 0x5e,
               "D2R2 clipped right edge");
    expect_int("d2r2.native.after_clip", checks[8].no_write_metadata ? 1 : 0, 1,
               "D2R2 viewport x=222 is outside source clip");
    expect_int("d2r2.native.bottom.value", checks[9].pixel_after, 0x56,
               "D2R2 bottom row y=90");

    expect_int("d2l2.flip.selected_x", checks[10].selected_source_x, 5,
               "DUNVIEW.C:6851 F0105 flips C05 into D2L2");
    expect_int("d2l2.flip.skip", checks[10].transparent_skip ? 1 : 0, 1,
               "flipped D2R2 C10 skip maps to D2L2 left pixel");
    expect_int("d2l2.flip.next", checks[11].pixel_after, 0x63,
               "flipped D2R2 source x=4 maps to D2L2 next pixel");
    expect_int("d2l2.flip.edge", checks[12].pixel_after, 0x6e,
               "flipped D2R2 source x=0 maps to D2L2 right edge");

    expect_int("d2r2.flip.selected_x", checks[13].selected_source_x, 35,
               "DUNVIEW.C:6882 F0105 flips C06 into D2R2");
    expect_int("d2r2.flip.skip", checks[13].transparent_skip ? 1 : 0, 1,
               "flipped D2L2 C10 skip maps to D2R2 left pixel");
    expect_int("d2r2.flip.next", checks[14].pixel_after, 0x64,
               "flipped D2L2 source x=34 maps to D2R2 next pixel");
    expect_int("d2r2.flip.edge", checks[15].pixel_after, 0x6d,
               "flipped D2L2 source x=0 maps to D2R2 right edge");
}

static void test_invalid_inputs_and_blend(void)
{
    expect_int("invalid.bad_side",
               dm1_v1_viewport_d2l2_d2r2_wall_spec_pc34(
                   (DM1_V1_D2L2D2R2WallSidePc34)99) == NULL,
               1, "spec rejects unknown side");
    expect_int("invalid.null_run",
               dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(NULL) ? 1 : 0,
               0, "run rejects null output");
    expect_int("blend.c10",
               dm1_v1_viewport_d2l2_d2r2_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "C10 transparent pixel preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d2l2_d2r2_wall_blend_pixel_pc34(0x44, 0x55, 10),
               0x55, "opaque wall pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34();

    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "source evidence");
    expect_contains("evidence.f0678", e, "DUNVIEW.C:6837-6865 F0678_DrawD2L2",
                    "DUNVIEW.C D2L2 function");
    expect_contains("evidence.f0679", e, "DUNVIEW.C:6868-6896 F0679_DrawD2R2",
                    "DUNVIEW.C D2R2 function");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8503-8508 F0128",
                    "DUNVIEW.C D2L2/D2R2 caller");
    expect_contains("evidence.flip_pair", e, "DUNVIEW.C:2442-2443",
                    "DUNVIEW.C flipped wall-set pair");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104",
                    "DUNVIEW.C native bitmap route");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105",
                    "DUNVIEW.C flipped bitmap route");
    expect_contains("evidence.c09", e, "DEFS.H:2605 C09_VIEW_SQUARE_D2L2=9",
                    "DEFS.H D2L2 view square");
    expect_contains("evidence.c10", e, "DEFS.H:2606 C10_VIEW_SQUARE_D2R2=10",
                    "DEFS.H D2R2 view square");
    expect_contains("evidence.c05", e, "DEFS.H:3428 C05_WALL_D2R2=5",
                    "DEFS.H D2R2 wall index");
    expect_contains("evidence.c06", e, "DEFS.H:3429 C06_WALL_D2L2=6",
                    "DEFS.H D2L2 wall index");
    expect_contains("evidence.c707", e, "C707_ZONE_WALL_D2L2=707",
                    "DEFS.H D2L2 zone");
    expect_contains("evidence.c708", e, "C708_ZONE_WALL_D2R2=708",
                    "DEFS.H D2R2 zone");
    expect_contains("evidence.panel", e, "PANEL.C:571",
                    "PANEL.C wall/fakewall flag bits");
    expect_contains("evidence.f0676_note", e, "F0676:6271-6273",
                    "requested anchor note");
    expect_contains("evidence.no_f0108", e, "excludes F0108 floor ornaments",
                    "no floor ornament route");
    expect_contains("evidence.no_f0111", e, "F0111 doors",
                    "no door route");
    expect_contains("evidence.no_f0115", e, "F0115 thing passes",
                    "no thing pass");
}

int main(void)
{
    test_specs_source_locked();
    test_run_pixel_contract();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
