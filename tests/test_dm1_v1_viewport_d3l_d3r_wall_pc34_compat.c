#include "dm1_v1_viewport_d3l_d3r_wall_pc34_compat.h"

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
    const DM1_V1_D3LD3RWallSpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_wall_spec_pc34(
            DM1_V1_D3L_D3R_WALL_SIDE_D3L_PC34);
    const DM1_V1_D3LD3RWallSpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_wall_spec_pc34(
            DM1_V1_D3L_D3R_WALL_SIDE_D3R_PC34);

    expect_int("spec.d3l.present", d3l != NULL, 1,
               "DUNVIEW.C:6361 F0116_DUNGEONVIEW_DrawSquareD3L");
    expect_int("spec.d3r.present", d3r != NULL, 1,
               "DUNVIEW.C:6500 F0117_DUNGEONVIEW_DrawSquareD3R");
    if (!d3l || !d3r) return;

    expect_int("spec.d3l.contract_only", d3l->contract_only ? 1 : 0, 1,
               "source-locked contract gate only");
    expect_int("spec.d3r.contract_only", d3r->contract_only ? 1 : 0, 1,
               "source-locked contract gate only");
    expect_int("spec.no_real_asset_parity",
               d3l->real_asset_bitmap_parity || d3r->real_asset_bitmap_parity, 0,
               "no real-asset bitmap parity claim");
    expect_int("spec.d3l.view_square", d3l->view_square_index, 12,
               "DEFS.H:2608 M601_VIEW_SQUARE_D3L=12");
    expect_int("spec.d3r.view_square", d3r->view_square_index, 13,
               "DEFS.H:2609 M602_VIEW_SQUARE_D3R=13");
    expect_int("spec.d3l.relative_depth", d3l->relative_depth, 3,
               "DUNVIEW.C:8490 D3L depth 3");
    expect_int("spec.d3l.relative_lateral", d3l->relative_lateral, -1,
               "DUNVIEW.C:8490 D3L lateral -1");
    expect_int("spec.d3r.relative_depth", d3r->relative_depth, 3,
               "DUNVIEW.C:8494 D3R depth 3");
    expect_int("spec.d3r.relative_lateral", d3r->relative_lateral, 1,
               "DUNVIEW.C:8494 D3R lateral 1");
    expect_int("spec.d3l.native_wall", d3l->native_wall_index_pc34, 13,
               "DEFS.H:3436 C13_WALL_D3L=13");
    expect_int("spec.d3r.native_wall", d3r->native_wall_index_pc34, 12,
               "DEFS.H:3435 C12_WALL_D3R=12");
    expect_int("spec.d3l.flipped_wall", d3l->flipped_wall_index_pc34, 12,
               "DUNVIEW.C:6421-6427 flips C12 into D3L");
    expect_int("spec.d3r.flipped_wall", d3r->flipped_wall_index_pc34, 13,
               "DUNVIEW.C:6554-6564 flips C13 into D3R");
    expect_int("spec.d3l.zone", d3l->wall_zone_pc34, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("spec.d3r.zone", d3r->wall_zone_pc34, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");
    expect_int("spec.draw_order", d3l->draw_order_index < d3r->draw_order_index, 1,
               "DUNVIEW.C:8490-8495 F0128 calls D3L before D3R");
    expect_int("spec.same_row", d3l->viewport_y_first == d3r->viewport_y_first &&
               d3l->viewport_y_last == d3r->viewport_y_last, 1,
               "DUNVIEW.C:583-585 G0163 D3 side wall row");
    expect_int("spec.same_height", d3l->visible_height, d3r->visible_height,
               "D3L/D3R visible spans are 51 rows");
    expect_int("spec.d3l.frame_left", d3l->frame_left_x, 0,
               "DUNVIEW.C:584 D3L frame X1");
    expect_int("spec.d3l.frame_right", d3l->frame_right_x, 83,
               "DUNVIEW.C:584 D3L frame X2 metadata");
    expect_int("spec.d3r.frame_left", d3r->frame_left_x, 139,
               "DUNVIEW.C:585 D3R frame X1");
    expect_int("spec.d3r.frame_right", d3r->frame_right_x, 223,
               "DUNVIEW.C:585 D3R frame X2 metadata");
    expect_int("spec.d3l.visible_width", d3l->visible_width, 32,
               "D3L source x=32..63 reaches viewport x=0..31");
    expect_int("spec.d3r.visible_width", d3r->visible_width, 64,
               "D3R source x=0..63 reaches viewport x=139..202");
    expect_int("spec.d3l.source_x", d3l->frame_source_x, 32,
               "DUNVIEW.C:584 frame source X for D3L");
    expect_int("spec.d3r.source_x", d3r->frame_source_x, 0,
               "DUNVIEW.C:585 frame source X for D3R");
    expect_int("spec.c10", d3l->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.c10.symmetry", d3l->transparent_color == d3r->transparent_color, 1,
               "F0100/F0104/F0105 share C10 transparent route");
    expect_int("spec.wall_no_f0108", d3l->calls_f0108_floor_ornament_on_wall ||
               d3r->calls_f0108_floor_ornament_on_wall, 0,
               "DUNVIEW.C:6406-6573 wall case excludes F0108");
    expect_int("spec.wall_no_f0111", d3l->calls_f0111_door_on_wall ||
               d3r->calls_f0111_door_on_wall, 0,
               "DUNVIEW.C:6406-6573 wall case excludes F0111");
    expect_int("spec.wall_alcove_can_f0115", d3l->wall_case_can_enter_alcove_thing_pass &&
               d3r->wall_case_can_enter_alcove_thing_pass, 1,
               "DUNVIEW.C:6432-6435 and 6568-6571");
}

static void test_run_pixel_contract(void)
{
    DM1_V1_D3LD3RWallRunPc34 run;
    const DM1_V1_D3LD3RWallPixelPc34 *checks;

    expect_int("run.ok",
               dm1_v1_viewport_d3l_d3r_wall_pc34_compat_run(&run) ? 1 : 0,
               1, "internal runtime gate entry point");
    checks = run.checks;
    expect_int("run.check_count", (int)run.check_count, 16,
               "native, clip, and flipped-pair pixel checks");
    expect_int("run.c10_palette", run.c10_palette_index, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("run.draw_order", run.draw_order_left_before_right ? 1 : 0, 1,
               "DUNVIEW.C:8490-8495 draw order");
    expect_int("run.mirrored_route", run.mirrored_route_pair ? 1 : 0, 1,
               "DUNVIEW.C:2434-2435 C12/C13 flipped wall-set pair");
    expect_int("run.zone_pair", run.d3l_d3r_zone_pair ? 1 : 0, 1,
               "DEFS.H:4045-4046 C705/C706");
    expect_int("run.d3l2_not_c707", run.d3l2_d3r2_not_c707_c708 ? 1 : 0, 1,
               "DEFS.H:4042-4043 D3L2/D3R2 are C702/C703");
    expect_int("run.d2l2_are_c707", run.d2l2_d2r2_are_c707_c708 ? 1 : 0, 1,
               "DEFS.H:4047-4048 C707/C708");
    expect_int("run.d2l_are_c710", run.d2l_d2r_are_c710_c711 ? 1 : 0, 1,
               "DEFS.H:4050-4051 C710/C711");
    expect_int("run.same_c10", run.same_c10_transparency ? 1 : 0, 1,
               "F0100/F0104/F0105 transparent blit");
    expect_int("run.same_height_row", run.same_height_and_row ? 1 : 0, 1,
               "D3L/D3R same row and height");
    expect_int("run.mirror_source_x", run.d3r_is_horizontal_mirror_source ? 1 : 0, 1,
               "DUNVIEW.C:584-585 asymmetric source X");
    expect_int("run.no_f0108_f0111", run.excludes_f0108_f0111_on_wall ? 1 : 0, 1,
               "DUNVIEW.C:6406-6573 wall route");
    expect_int("run.d3l2_zone", run.exact_d3l2_zone_pc34, 702,
               "DEFS.H:4042 C702_ZONE_WALL_D3L2");
    expect_int("run.d3r2_zone", run.exact_d3r2_zone_pc34, 703,
               "DEFS.H:4043 C703_ZONE_WALL_D3R2");
    expect_int("run.d2l2_zone", run.exact_d2l2_zone_pc34, 707,
               "DEFS.H:4047 C707_ZONE_WALL_D2L2");
    expect_int("run.d2r2_zone", run.exact_d2r2_zone_pc34, 708,
               "DEFS.H:4048 C708_ZONE_WALL_D2R2");
    expect_int("run.d2l_zone", run.exact_d2l_zone_pc34, 710,
               "DEFS.H:4050 C710_ZONE_WALL_D2L");
    expect_int("run.d2r_zone", run.exact_d2r_zone_pc34, 711,
               "DEFS.H:4051 C711_ZONE_WALL_D2R");

    expect_int("d3l.native.skip.in_clip", checks[0].in_clip ? 1 : 0, 1,
               "DUNVIEW.C:6408 native D3L route");
    expect_int("d3l.native.skip.src_x", checks[0].source_x, 32,
               "D3L clipped source x starts at 32");
    expect_int("d3l.native.skip.c10", checks[0].transparent_skip ? 1 : 0, 1,
               "C10 transparent pixel preserves viewport");
    expect_int("d3l.native.skip.after", checks[0].pixel_after, 0xee,
               "DUNVIEW.C:3048-3058 F0100 C10 skip");
    expect_int("d3l.native.next.value", checks[1].pixel_after, 0x42,
               "opaque D3L wall pixel writes");
    expect_int("d3l.native.edge.value", checks[2].pixel_after, 0x7e,
               "D3L clipped right edge");
    expect_int("d3l.native.after_clip", checks[3].no_write_metadata ? 1 : 0, 1,
               "D3L viewport x=32 is outside source clip");
    expect_int("d3l.native.bottom.value", checks[4].pixel_after, 0x55,
               "D3L bottom row y=75");

    expect_int("d3r.native.skip.src_x", checks[5].source_x, 0,
               "D3R clipped source x starts at 0");
    expect_int("d3r.native.skip.c10", checks[5].transparent_skip ? 1 : 0, 1,
               "C10 transparent pixel preserves viewport");
    expect_int("d3r.native.next.value", checks[6].pixel_after, 0x52,
               "opaque D3R wall pixel writes");
    expect_int("d3r.native.edge.value", checks[7].pixel_after, 0x5e,
               "D3R clipped right edge");
    expect_int("d3r.native.after_clip", checks[8].no_write_metadata ? 1 : 0, 1,
               "D3R viewport x=203 is outside source clip");
    expect_int("d3r.native.bottom.value", checks[9].pixel_after, 0x56,
               "D3R bottom row y=75");

    expect_int("d3l.flip.selected_x", checks[10].selected_source_x, 31,
               "DUNVIEW.C:6421-6427 flips C12 into D3L");
    expect_int("d3l.flip.skip", checks[10].transparent_skip ? 1 : 0, 1,
               "flipped D3R C10 skip maps to D3L left pixel");
    expect_int("d3l.flip.next", checks[11].pixel_after, 0x63,
               "flipped D3R source x=30 maps to D3L next pixel");
    expect_int("d3l.flip.edge", checks[12].pixel_after, 0x6e,
               "flipped D3R source x=0 maps to D3L right edge");

    expect_int("d3r.flip.selected_x", checks[13].selected_source_x, 63,
               "DUNVIEW.C:6554-6564 flips C13 into D3R");
    expect_int("d3r.flip.skip", checks[13].transparent_skip ? 1 : 0, 1,
               "flipped D3L C10 skip maps to D3R left pixel");
    expect_int("d3r.flip.next", checks[14].pixel_after, 0x64,
               "flipped D3L source x=62 maps to D3R next pixel");
    expect_int("d3r.flip.edge", checks[15].pixel_after, 0x6d,
               "flipped D3L source x=0 maps to D3R right edge");
}

static void test_invalid_inputs_and_blend(void)
{
    expect_int("invalid.bad_side",
               dm1_v1_viewport_d3l_d3r_wall_spec_pc34(
                   (DM1_V1_D3LD3RWallSidePc34)99) == NULL,
               1, "spec rejects unknown side");
    expect_int("invalid.null_run",
               dm1_v1_viewport_d3l_d3r_wall_pc34_compat_run(NULL) ? 1 : 0,
               0, "run rejects null output");
    expect_int("blend.c10",
               dm1_v1_viewport_d3l_d3r_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "C10 transparent pixel preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d3l_d3r_wall_blend_pixel_pc34(0x44, 0x55, 10),
               0x55, "opaque wall pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d3l_d3r_wall_source_evidence_pc34();

    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "source evidence");
    expect_contains("evidence.frames", e, "DUNVIEW.C:581-585",
                    "DUNVIEW.C frame table");
    expect_contains("evidence.f0116", e, "DUNVIEW.C:6406-6437 F0116_DUNGEONVIEW_DrawSquareD3L",
                    "DUNVIEW.C D3L function");
    expect_contains("evidence.f0117", e, "DUNVIEW.C:6545-6573 F0117_DUNGEONVIEW_DrawSquareD3R",
                    "DUNVIEW.C D3R function");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8490-8495",
                    "DUNVIEW.C D3L/D3R caller");
    expect_contains("evidence.flip_pair", e, "DUNVIEW.C:2434-2435",
                    "DUNVIEW.C flipped wall-set pair");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C wall bitmap route");
    expect_contains("evidence.f0104_f0105", e, "DUNVIEW.C:3113-3204 F0104/F0105",
                    "DUNVIEW.C native/flipped bitmap routes");
    expect_contains("evidence.f0097", e, "DRAWVIEW.C:709-723",
                    "DRAWVIEW.C viewport presentation");
    expect_contains("evidence.d3l_square", e, "DEFS.H:2608 M601_VIEW_SQUARE_D3L=12",
                    "DEFS.H D3L view square");
    expect_contains("evidence.d3r_square", e, "DEFS.H:2609 M602_VIEW_SQUARE_D3R=13",
                    "DEFS.H D3R view square");
    expect_contains("evidence.c12", e, "DEFS.H:3435 C12_WALL_D3R=12",
                    "DEFS.H D3R wall index");
    expect_contains("evidence.c13", e, "DEFS.H:3436 C13_WALL_D3L=13",
                    "DEFS.H D3L wall index");
    expect_contains("evidence.c04_note", e, "requested C04/C05 names are not the D3L/D3R wall symbols",
                    "requested C04/C05 correction");
    expect_contains("evidence.c702", e, "D3L2/D3R2 are C702/C703",
                    "DEFS.H D3L2/D3R2 zones");
    expect_contains("evidence.c705", e, "C705_ZONE_WALL_D3L=705",
                    "DEFS.H D3L zone");
    expect_contains("evidence.c706", e, "C706_ZONE_WALL_D3R=706",
                    "DEFS.H D3R zone");
    expect_contains("evidence.c707", e, "C707/C708 are D2L2/D2R2",
                    "DEFS.H D2L2/D2R2 zones");
    expect_contains("evidence.c710", e, "C710/C711 are D2L/D2R",
                    "DEFS.H D2L/D2R zones");
    expect_contains("evidence.no_f0108", e, "does not call F0108 floor ornaments",
                    "no floor ornament route");
    expect_contains("evidence.no_f0111", e, "F0111 doors",
                    "no door route");
    expect_contains("evidence.f0115_alcove", e, "F0115 only follows the explicit front-alcove path",
                    "alcove thing pass exception");
}

int main(void)
{
    test_specs_source_locked();
    test_run_pixel_contract();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3l_d3r_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3l_d3r_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
