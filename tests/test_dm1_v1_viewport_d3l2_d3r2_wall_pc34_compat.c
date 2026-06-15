#include "dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat.h"

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
    const DM1_V1_D3L2D3R2WallSpecPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_wall_spec_pc34(
            DM1_V1_D3L2_D3R2_WALL_SIDE_D3L2_PC34);
    const DM1_V1_D3L2D3R2WallSpecPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_wall_spec_pc34(
            DM1_V1_D3L2_D3R2_WALL_SIDE_D3R2_PC34);

    expect_int("spec.d3l2.present", d3l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:8446-8452 D3L2 direct branch");
    expect_int("spec.d3r2.present", d3r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:8454-8464 D3R2 direct branch");
    if (!d3l2 || !d3r2) return;

    expect_int("spec.d3l2.contract_only", d3l2->contract_only ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 source-locked contract gate");
    expect_int("spec.d3r2.contract_only", d3r2->contract_only ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 source-locked contract gate");
    expect_int("spec.no_real_asset_parity",
               d3l2->real_asset_bitmap_parity || d3r2->real_asset_bitmap_parity, 0,
               "ReDMCSB DUNVIEW.C:317-318 pointer contract only");
    expect_int("spec.d3l2.view_square", d3l2->view_square_index, 14,
               "ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2=14");
    expect_int("spec.d3r2.view_square", d3r2->view_square_index, 15,
               "ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2=15");
    expect_int("spec.d3l2.relative_depth", d3l2->relative_depth, 3,
               "ReDMCSB DUNVIEW.C:8446 relative depth 3");
    expect_int("spec.d3l2.relative_lateral", d3l2->relative_lateral, -2,
               "ReDMCSB DUNVIEW.C:8446 relative lateral -2");
    expect_int("spec.d3r2.relative_depth", d3r2->relative_depth, 3,
               "ReDMCSB DUNVIEW.C:8454 relative depth 3");
    expect_int("spec.d3r2.relative_lateral", d3r2->relative_lateral, 2,
               "ReDMCSB DUNVIEW.C:8454 relative lateral 2");
    expect_int("spec.d3l2.native_wall", d3l2->native_wall_index_pc34, 11,
               "ReDMCSB DEFS.H:3434 C11_WALL_D3L2=11");
    expect_int("spec.d3r2.native_wall", d3r2->native_wall_index_pc34, 10,
               "ReDMCSB DEFS.H:3433 C10_WALL_D3R2=10");
    expect_int("spec.d3l2.flipped_wall", d3l2->flipped_wall_index_pc34, 10,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped C10/C11 pair");
    expect_int("spec.d3r2.flipped_wall", d3r2->flipped_wall_index_pc34, 11,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped C10/C11 pair");
    expect_int("spec.d3l2.wallset_global", d3l2->wall_set_global_index_pc34, -5,
               "ReDMCSB DUNVIEW.C:139 G3010_i_WallSet_Wall_D3L2=-5");
    expect_int("spec.d3r2.wallset_global", d3r2->wall_set_global_index_pc34, -6,
               "ReDMCSB DUNVIEW.C:130 G3072_i_WallSet_Wall_D3R2=-6");
    expect_int("spec.d3l2.zone", d3l2->wall_zone_pc34, 702,
               "ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2=702");
    expect_int("spec.d3r2.zone", d3r2->wall_zone_pc34, 703,
               "ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2=703");
    expect_int("spec.draw_order", d3l2->draw_order_index < d3r2->draw_order_index, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 D3L2 branch precedes D3R2 branch");
    expect_int("spec.same_row", d3l2->viewport_y_first == d3r2->viewport_y_first &&
               d3l2->viewport_y_last == d3r2->viewport_y_last, 1,
               "ReDMCSB DUNVIEW.C:579-580 both frames use y=25..73");
    expect_int("spec.same_height", d3l2->visible_height, d3r2->visible_height,
               "ReDMCSB DUNVIEW.C:579-580 both frames have height 49");
    expect_int("spec.d3l2.frame_left", d3l2->frame_left_x, 0,
               "ReDMCSB DUNVIEW.C:579 D3L2 frame X1");
    expect_int("spec.d3l2.frame_right", d3l2->frame_right_x, 15,
               "ReDMCSB DUNVIEW.C:579 D3L2 frame X2");
    expect_int("spec.d3r2.frame_left", d3r2->frame_left_x, 208,
               "ReDMCSB DUNVIEW.C:580 D3R2 frame X1");
    expect_int("spec.d3r2.frame_right", d3r2->frame_right_x, 223,
               "ReDMCSB DUNVIEW.C:580 D3R2 frame X2");
    expect_int("spec.d3l2.frame_top", d3l2->frame_top_y, 25,
               "ReDMCSB DUNVIEW.C:579 D3L2 frame Y1");
    expect_int("spec.d3r2.frame_bottom", d3r2->frame_bottom_y, 73,
               "ReDMCSB DUNVIEW.C:580 D3R2 frame Y2");
    expect_int("spec.frame_byte_width", d3l2->frame_byte_width, 8,
               "ReDMCSB DUNVIEW.C:579 D3L2 byte width");
    expect_int("spec.frame_height", d3r2->frame_height, 49,
               "ReDMCSB DUNVIEW.C:580 D3R2 height");
    expect_int("spec.d3l2.visible_width", d3l2->visible_width, 16,
               "ReDMCSB DUNVIEW.C:579 D3L2 X span 0..15");
    expect_int("spec.d3r2.visible_width", d3r2->visible_width, 16,
               "ReDMCSB DUNVIEW.C:580 D3R2 X span 208..223");
    expect_int("spec.d3l2.source_x", d3l2->frame_source_x, 0,
               "ReDMCSB DUNVIEW.C:579 D3L2 source X");
    expect_int("spec.d3r2.source_x", d3r2->frame_source_x, 0,
               "ReDMCSB DUNVIEW.C:580 D3R2 source X");
    expect_int("spec.c10", d3l2->transparent_color, 10,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.c10.symmetry", d3l2->transparent_color == d3r2->transparent_color, 1,
               "ReDMCSB DUNVIEW.C:3048-3058 F0100 C10 route");
    expect_int("spec.c112", DM1_V1_D3L2_D3R2_WALL_VIEWPORT_WIDTH_PC34 / 2, 112,
               "ReDMCSB DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT");
    expect_int("spec.d3l2.uses_f0100", d3l2->uses_f0100_wallset_bitmap ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8448 F0100 D3L2 old-media branch");
    expect_int("spec.d3r2.uses_f0100", d3r2->uses_f0100_wallset_bitmap ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8456 F0100 D3R2 old-media branch");
    expect_int("spec.d3l2.uses_f0104_wall", d3l2->uses_f0104_native_wall_route ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8451 F0104 wall route");
    expect_int("spec.d3r2.uses_f0104_wall", d3r2->uses_f0104_native_wall_route ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8462 F0104 wall route");
    expect_int("spec.d3r2.uses_f0105_wall", d3r2->uses_f0105_flipped_wall_route ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8459 F0105 wall route");
    expect_int("spec.no_f0107", d3l2->calls_f0107_wall_ornament ||
               d3r2->calls_f0107_wall_ornament, 0,
               "ReDMCSB DUNVIEW.C:8446-8464 direct branch excludes F0107");
    expect_int("spec.no_f0111", d3l2->calls_f0111_door || d3r2->calls_f0111_door, 0,
               "ReDMCSB DUNVIEW.C:8446-8464 direct branch excludes F0111");
    expect_int("spec.no_f0115", d3l2->calls_f0115_alcove_or_thing_pass ||
               d3r2->calls_f0115_alcove_or_thing_pass, 0,
               "ReDMCSB DUNVIEW.C:8446-8464 direct branch excludes F0115");
    expect_int("spec.no_f0104_f0105_pit", d3l2->calls_f0104_f0105_pit_route ||
               d3r2->calls_f0104_f0105_pit_route, 0,
               "ReDMCSB DUNVIEW.C:8446-8464 F0104/F0105 are wall, not pit routes");
    expect_int("spec.no_f0108", d3l2->calls_f0108_floor_ornament ||
               d3r2->calls_f0108_floor_ornament, 0,
               "ReDMCSB DUNVIEW.C:8446-8464 direct branch excludes F0108");
    expect_int("spec.non_overlap.d3l_d3r", d3l2->non_overlap_d3l_d3r_gate &&
               d3r2->non_overlap_d3l_d3r_gate, 1,
               "ReDMCSB DEFS.H:4042-4046 C702/C703 differ from C705/C706");
    expect_int("spec.non_overlap.d3c", d3l2->non_overlap_d3c_gate &&
               d3r2->non_overlap_d3c_gate, 1,
               "ReDMCSB DEFS.H:4042-4044 C702/C703 differ from C704");
}

static void test_run_pixel_contract(void)
{
    DM1_V1_D3L2D3R2WallRunPc34 run;
    const DM1_V1_D3L2D3R2WallPixelPc34 *checks;

    expect_int("run.ok",
               dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat_run(&run) ? 1 : 0,
               1, "ReDMCSB DUNVIEW.C:8446-8464 internal runtime gate");
    checks = run.checks;
    expect_int("run.check_count", (int)run.check_count, 16,
               "ReDMCSB DUNVIEW.C:579-580 native, clip, and flipped-pair checks");
    expect_int("run.c10_palette", run.c10_palette_index, 10,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("run.c112_byte_width", run.c112_byte_width_viewport, 112,
               "ReDMCSB DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT");
    expect_int("run.draw_order", run.draw_order_left_before_right ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 draw order");
    expect_int("run.mirrored_route", run.mirrored_route_pair ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:2440-2441 C10/C11 flipped wall-set pair");
    expect_int("run.zone_pair", run.d3l2_d3r2_zone_pair ? 1 : 0, 1,
               "ReDMCSB DEFS.H:4042-4043 C702/C703");
    expect_int("run.d3l2_zone", run.exact_d3l2_zone_pc34, 702,
               "ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2");
    expect_int("run.d3r2_zone", run.exact_d3r2_zone_pc34, 703,
               "ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2");
    expect_int("run.d3l_zone", run.exact_d3l_zone_pc34, 705,
               "ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("run.d3r_zone", run.exact_d3r_zone_pc34, 706,
               "ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R");
    expect_int("run.d3c_zone", run.exact_d3c_zone_pc34, 704,
               "ReDMCSB DEFS.H:4044 C704_ZONE_WALL_D3C");
    expect_int("run.non_overlap_d3l_d3r",
               run.non_overlap_with_d3l_d3r_wall_gate ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 lateral +/-2 differs from +/-1");
    expect_int("run.non_overlap_d3c",
               run.non_overlap_with_d3c_wall_gate ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 lateral +/-2 differs from 0");
    expect_int("run.no_f0107", run.no_f0107_ornament ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 no F0107 ornament");
    expect_int("run.no_f0111", run.no_f0111_door ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 no F0111 door");
    expect_int("run.no_f0115", run.no_f0115_alcove ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 no F0115 alcove/thing pass");
    expect_int("run.no_f0104_f0105_pit", run.no_f0104_f0105_pit_route ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 no F0104/F0105 pit route");
    expect_int("run.no_f0108", run.no_f0108_floor_ornament ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8446-8464 no F0108 floor ornament");
    expect_int("run.same_c10", run.same_c10_transparency ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:3048-3058 C10 transparent blit");
    expect_int("run.same_height_row", run.same_height_and_row ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:579-580 D3L2/D3R2 same row and height");
    expect_int("run.c10_preserves", run.c10_flesh_pixels_preserve_destination ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:3055 C10_COLOR_FLESH preserves destination");

    expect_int("d3l2.native.skip.in_clip", checks[0].in_clip ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8448 native D3L2 route");
    expect_int("d3l2.native.skip.src_x", checks[0].source_x, 0,
               "ReDMCSB DUNVIEW.C:579 D3L2 source x starts at 0");
    expect_int("d3l2.native.skip.src_y", checks[0].source_y, 0,
               "ReDMCSB DUNVIEW.C:579 D3L2 source y starts at 0");
    expect_int("d3l2.native.skip.c10", checks[0].transparent_skip ? 1 : 0, 1,
               "ReDMCSB DEFS.H:2088 C10 transparent pixel");
    expect_int("d3l2.native.skip.after", checks[0].pixel_after, 0xee,
               "ReDMCSB DUNVIEW.C:3048-3058 F0100 C10 skip");
    expect_int("d3l2.native.next.value", checks[1].pixel_after, 0x42,
               "ReDMCSB DUNVIEW.C:8448 opaque D3L2 wall pixel writes");
    expect_int("d3l2.native.edge.value", checks[2].pixel_after, 0x7e,
               "ReDMCSB DUNVIEW.C:579 D3L2 right edge");
    expect_int("d3l2.native.after_clip", checks[3].no_write_metadata ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:579 D3L2 viewport x=16 outside clip");
    expect_int("d3l2.native.bottom.value", checks[4].pixel_after, 0x55,
               "ReDMCSB DUNVIEW.C:579 D3L2 bottom row y=73");

    expect_int("d3r2.native.skip.in_clip", checks[5].in_clip ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:8456 native D3R2 route");
    expect_int("d3r2.native.skip.src_x", checks[5].source_x, 0,
               "ReDMCSB DUNVIEW.C:580 D3R2 source x starts at 0");
    expect_int("d3r2.native.skip.c10", checks[5].transparent_skip ? 1 : 0, 1,
               "ReDMCSB DEFS.H:2088 C10 transparent pixel");
    expect_int("d3r2.native.next.value", checks[6].pixel_after, 0x52,
               "ReDMCSB DUNVIEW.C:8456 opaque D3R2 wall pixel writes");
    expect_int("d3r2.native.edge.value", checks[7].pixel_after, 0x5e,
               "ReDMCSB DUNVIEW.C:580 D3R2 right edge");
    expect_int("d3r2.native.before_clip", checks[8].no_write_metadata ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:580 D3R2 viewport x=207 outside clip");
    expect_int("d3r2.native.bottom.value", checks[9].pixel_after, 0x56,
               "ReDMCSB DUNVIEW.C:580 D3R2 bottom row y=73");

    expect_int("d3l2.flip.selected_x", checks[10].selected_source_x, 15,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped C10 into C11");
    expect_int("d3l2.flip.skip", checks[10].transparent_skip ? 1 : 0, 1,
               "ReDMCSB DEFS.H:2088 flipped C10 skip maps to D3L2 left pixel");
    expect_int("d3l2.flip.next", checks[11].pixel_after, 0x63,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped D3R2 source x=14 maps next");
    expect_int("d3l2.flip.edge", checks[12].pixel_after, 0x6e,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped D3R2 source x=0 maps edge");

    expect_int("d3r2.flip.selected_x", checks[13].selected_source_x, 15,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped C11 into C10");
    expect_int("d3r2.flip.skip", checks[13].transparent_skip ? 1 : 0, 1,
               "ReDMCSB DEFS.H:2088 flipped C10 skip maps to D3R2 left pixel");
    expect_int("d3r2.flip.next", checks[14].pixel_after, 0x64,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped D3L2 source x=14 maps next");
    expect_int("d3r2.flip.edge", checks[15].pixel_after, 0x6d,
               "ReDMCSB DUNVIEW.C:2440-2441 flipped D3L2 source x=0 maps edge");
}

static void test_invalid_inputs_and_blend(void)
{
    expect_int("invalid.bad_side",
               dm1_v1_viewport_d3l2_d3r2_wall_spec_pc34(
                   (DM1_V1_D3L2D3R2WallSidePc34)99) == NULL,
               1, "ReDMCSB DUNVIEW.C:8446-8464 rejects unknown side");
    expect_int("invalid.null_run",
               dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat_run(NULL) ? 1 : 0,
               0, "ReDMCSB DUNVIEW.C:8446-8464 rejects null output");
    expect_int("blend.c10",
               dm1_v1_viewport_d3l2_d3r2_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "ReDMCSB DUNVIEW.C:3055 C10 transparent pixel preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d3l2_d3r2_wall_blend_pixel_pc34(0x44, 0x55, 10),
               0x55, "ReDMCSB DUNVIEW.C:3055 opaque wall pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34();

    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "ReDMCSB DUNVIEW.C:8446-8464 source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "ReDMCSB DUNVIEW.C:317-318 source evidence");
    expect_contains("evidence.wallset_d3r2", e, "DUNVIEW.C:130 G3072_i_WallSet_Wall_D3R2=-6",
                    "ReDMCSB DUNVIEW.C:130 wall-set index");
    expect_contains("evidence.wallset_d3l2", e, "DUNVIEW.C:139 G3010_i_WallSet_Wall_D3L2=-5",
                    "ReDMCSB DUNVIEW.C:139 wall-set index");
    expect_contains("evidence.bitmaps", e, "DUNVIEW.C:317-318",
                    "ReDMCSB DUNVIEW.C:317-318 bitmap pointers");
    expect_contains("evidence.frames", e, "DUNVIEW.C:579-580",
                    "ReDMCSB DUNVIEW.C:579-580 frame table");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8446-8464 F0128_DUNGEONVIEW_DrawViewport",
                    "ReDMCSB DUNVIEW.C:8446-8464 plain-wall dispatch");
    expect_contains("evidence.f0676_note", e, "DUNVIEW.C:6253-6331 F0676/F0677 full-square dispatch is outside",
                    "ReDMCSB DUNVIEW.C:6253-6331 non-gate note");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "ReDMCSB DUNVIEW.C:3048-3058 F0100");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104",
                    "ReDMCSB DUNVIEW.C:3113-3129 F0104");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105",
                    "ReDMCSB DUNVIEW.C:3185-3204 F0105");
    expect_contains("evidence.flip_pair", e, "DUNVIEW.C:2440-2441",
                    "ReDMCSB DUNVIEW.C:2440-2441 flipped pair");
    expect_contains("evidence.c10_color", e, "DEFS.H:2088 C10_COLOR_FLESH=10",
                    "ReDMCSB DEFS.H:2088 C10");
    expect_contains("evidence.c112", e, "DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT=112",
                    "ReDMCSB DEFS.H:2478 C112");
    expect_contains("evidence.d3l2_square", e, "DEFS.H:2610 C14_VIEW_SQUARE_D3L2=14",
                    "ReDMCSB DEFS.H:2610 D3L2 view square");
    expect_contains("evidence.d3r2_square", e, "DEFS.H:2611 C15_VIEW_SQUARE_D3R2=15",
                    "ReDMCSB DEFS.H:2611 D3R2 view square");
    expect_contains("evidence.m621_note", e, "rather than M621/M622",
                    "ReDMCSB DEFS.H:2610-2611 current symbol note");
    expect_contains("evidence.c10_wall", e, "DEFS.H:3433 C10_WALL_D3R2=10",
                    "ReDMCSB DEFS.H:3433 D3R2 wall index");
    expect_contains("evidence.c11_wall", e, "DEFS.H:3434 C11_WALL_D3L2=11",
                    "ReDMCSB DEFS.H:3434 D3L2 wall index");
    expect_contains("evidence.c702", e, "DEFS.H:4042 C702_ZONE_WALL_D3L2=702",
                    "ReDMCSB DEFS.H:4042 D3L2 zone");
    expect_contains("evidence.c703", e, "DEFS.H:4043 C703_ZONE_WALL_D3R2=703",
                    "ReDMCSB DEFS.H:4043 D3R2 zone");
    expect_contains("evidence.c621_note", e, "rather than C621/C622",
                    "ReDMCSB DEFS.H:4042-4043 current zone note");
    expect_contains("evidence.non_overlap_d3l_d3r", e, "non-overlap with d3l_d3r_wall",
                    "ReDMCSB DEFS.H:4042-4046 gate non-overlap");
    expect_contains("evidence.non_overlap_d3c", e, "non-overlap with d3c_wall",
                    "ReDMCSB DEFS.H:4042-4044 gate non-overlap");
    expect_contains("evidence.no_f0107", e, "no F0107 ornament",
                    "ReDMCSB DUNVIEW.C:8446-8464 no ornament");
    expect_contains("evidence.no_f0111", e, "no F0111 door",
                    "ReDMCSB DUNVIEW.C:8446-8464 no door");
    expect_contains("evidence.no_f0115", e, "no F0115 alcove/thing pass",
                    "ReDMCSB DUNVIEW.C:8446-8464 no alcove");
    expect_contains("evidence.no_pit_route", e, "no F0104/F0105 pit route",
                    "ReDMCSB DUNVIEW.C:8446-8464 no pit route");
    expect_contains("evidence.no_f0108", e, "no F0108 floor ornament",
                    "ReDMCSB DUNVIEW.C:8446-8464 no floor ornament");
    expect_contains("evidence.c10_preserve", e, "C10 flesh pixels preserve destination",
                    "ReDMCSB DUNVIEW.C:3055 C10 transparency");
}

int main(void)
{
    test_specs_source_locked();
    test_run_pixel_contract();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
