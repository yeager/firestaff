#include "dm1_v1_viewport_d0r2_wall_pc34_compat.h"

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

static void test_anchor_table_source_locked(void)
{
    size_t count = 0;
    const DM1_V1_D0R2WallAnchorPc34 *a =
        dm1_v1_viewport_d0r2_wall_anchor_table_pc34(&count);

    expect_int("anchors.count", (int)count, DM1_V1_D0R2_WALL_ANCHOR_COUNT_PC34,
               "required anchor table");
    expect_int("anchors.pointer", a != NULL, 1, "anchor table pointer");

    expect_contains("anchor0.function", a[0].function, "DrawSquareD0R",
                    "DUNVIEW.C:8064-8162");
    expect_int("anchor0.first", a[0].line_first, 8064, "DUNVIEW.C F0126 start");
    expect_int("anchor0.last", a[0].line_last, 8162, "DUNVIEW.C F0126 end");
    expect_contains("anchor0.claim", a[0].claim, "wall case", "DUNVIEW.C:8117");
    expect_int("anchor1.first", a[1].line_first, 8117, "DUNVIEW.C F0126 wall");
    expect_int("anchor1.last", a[1].line_last, 8144, "DUNVIEW.C F0126 return");
    expect_contains("anchor1.claim", a[1].claim, "F0105", "DUNVIEW.C:8127");
    expect_contains("anchor1.claim.native", a[1].claim, "F0104", "DUNVIEW.C:8139");
    expect_contains("anchor2.function", a[2].function, "DrawSquareD3R",
                    "DUNVIEW.C:6500-6622");
    expect_int("anchor2.first", a[2].line_first, 6500, "DUNVIEW.C F0117 start");
    expect_int("anchor2.last", a[2].line_last, 6622, "DUNVIEW.C F0117 F0115 tail");
    expect_contains("anchor3.function", a[3].function, "DrawSquareD0L",
                    "DUNVIEW.C:7960-8062");
    expect_int("anchor3.first", a[3].line_first, 7960, "DUNVIEW.C F0125 start");
    expect_int("anchor3.last", a[3].line_last, 8062, "DUNVIEW.C F0125 end");
    expect_contains("anchor3.claim", a[3].claim, "disjoint", "D0L sibling only");
    expect_int("anchor4.first", a[4].line_first, 8508, "DUNVIEW.C F0128 D2R2");
    expect_int("anchor4.last", a[4].line_last, 8542, "DUNVIEW.C F0128 D0C");
    expect_contains("anchor4.claim", a[4].claim, "D0L", "DUNVIEW.C:8537");
    expect_contains("anchor4.claim.right", a[4].claim, "D0R", "DUNVIEW.C:8541");
    expect_int("anchor5.first", a[5].line_first, 3048, "DUNVIEW.C F0100");
    expect_int("anchor5.last", a[5].line_last, 3204, "DUNVIEW.C F0105");
    expect_contains("anchor5.claim", a[5].claim, "C10", "DUNVIEW.C:3055");
    expect_int("anchor6.first", a[6].line_first, 1769, "DUNGEON.C F0163");
    expect_int("anchor6.last", a[6].line_last, 1838, "DUNGEON.C F0163");
    expect_int("anchor7.first", a[7].line_first, 1840, "DUNGEON.C F0164");
    expect_int("anchor7.last", a[7].line_last, 1905, "DUNGEON.C F0164");
    expect_int("anchor8.first", a[8].line_first, 2466, "DUNGEON.C F0172");
    expect_int("anchor8.last", a[8].line_last, 2523, "DUNGEON.C F0172");
    expect_int("anchor9.first", a[9].line_first, 2088, "DEFS.H C10");
    expect_int("anchor9.last", a[9].line_last, 4057, "DEFS.H C717");
    expect_contains("anchor9.claim", a[9].claim, "C717", "DEFS.H:4057");
    expect_int("anchor10.first", a[10].line_first, 594, "DUNVIEW.C G0163 D0R");
    expect_int("anchor10.last", a[10].line_last, 731, "DUNVIEW.C G0188 D0R");
    expect_contains("anchor10.claim", a[10].claim, "192,223,0,135",
                    "DUNVIEW.C:594");
}

static void test_route_specs_source_locked(void)
{
    const DM1_V1_D0R2WallRouteSpecPc34 *native =
        dm1_v1_viewport_d0r2_wall_route_spec_pc34(
            DM1_V1_D0R2_WALL_ROUTE_NATIVE_PC34);
    const DM1_V1_D0R2WallRouteSpecPc34 *parity =
        dm1_v1_viewport_d0r2_wall_route_spec_pc34(
            DM1_V1_D0R2_WALL_ROUTE_PARITY_FLIPPED_PC34);

    expect_int("spec.native.present", native != NULL, 1, "DUNVIEW.C:8117-8144");
    expect_int("spec.parity.present", parity != NULL, 1, "DUNVIEW.C:8127");
    if (!native || !parity) return;

    expect_int("spec.native.contract", native->contract_only ? 1 : 0, 1,
               "contract-only gate");
    expect_int("spec.parity.contract", parity->contract_only ? 1 : 0, 1,
               "contract-only gate");
    expect_int("spec.no_asset_parity",
               native->real_asset_bitmap_parity || parity->real_asset_bitmap_parity, 0,
               "no real-asset parity claim");
    expect_int("spec.native.wall_case", native->wall_case ? 1 : 0, 1,
               "DUNVIEW.C:8117 case C00_ELEMENT_WALL");
    expect_int("spec.parity.wall_case", parity->wall_case ? 1 : 0, 1,
               "DUNVIEW.C:8117 case C00_ELEMENT_WALL");
    expect_int("spec.native.no_flip", native->parity_flip ? 1 : 0, 0,
               "DUNVIEW.C:8139 native F0104");
    expect_int("spec.parity.flip", parity->parity_flip ? 1 : 0, 1,
               "DUNVIEW.C:8127 parity F0105");
    expect_int("spec.depth", native->relative_depth, 0, "DUNVIEW.C:8540 relative depth");
    expect_int("spec.lateral", native->relative_lateral, 1,
               "DUNVIEW.C:8540 relative lateral");
    expect_int("spec.view_square", native->view_square_index, 2,
               "DEFS.H:2598 M611_VIEW_SQUARE_D0R=2");
    expect_int("spec.native.wall", native->native_wall_index_pc34, 0,
               "DEFS.H:3423 C00_WALL_D0R=0");
    expect_int("spec.native.opposite", native->opposite_wall_index_pc34, 1,
               "DEFS.H:3424 C01_WALL_D0L=1");
    expect_int("spec.native.wall_set", native->native_wall_set_index_pc34, -17,
               "DUNVIEW.C:144 G3015_i_WallSet_Wall_D0R=-17");
    expect_int("spec.opposite.wall_set", native->opposite_wall_set_index_pc34, -16,
               "DUNVIEW.C:143 G3014_i_WallSet_Wall_D0L=-16");
    expect_int("spec.zone.old", native->old_media_wall_zone_pc34, 715,
               "DEFS.H:4038 C715_ZONE_WALL_D0R");
    expect_int("spec.zone.pc34", native->wall_zone_pc34, 717,
               "DEFS.H:4057 C717_ZONE_WALL_D0R");
    expect_int("spec.frame.left", native->frame_left_x, 192, "DUNVIEW.C:594");
    expect_int("spec.frame.right", native->frame_right_x, 223, "DUNVIEW.C:594");
    expect_int("spec.frame.top", native->frame_top_y, 0, "DUNVIEW.C:594");
    expect_int("spec.frame.bottom", native->frame_bottom_y, 135, "DUNVIEW.C:594");
    expect_int("spec.frame.byte_width", native->frame_byte_width, 16, "DUNVIEW.C:594");
    expect_int("spec.frame.height", native->frame_height, 136, "DUNVIEW.C:594");
    expect_int("spec.frame.source_x", native->frame_source_x, 0, "DUNVIEW.C:594");
    expect_int("spec.frame.source_y", native->frame_source_y, 0, "DUNVIEW.C:594");
    expect_int("spec.field.index", native->field_aspect_index, 2,
               "G0188 maps M611 to D0R row");
    expect_int("spec.field.native_index", native->field_native_bitmap_relative_index, 0,
               "DUNVIEW.C:731");
    expect_int("spec.field.base", native->field_base_start_unit_index, 63,
               "DUNVIEW.C:731");
    expect_int("spec.field.mask", native->field_mask, 0x03, "DUNVIEW.C:731");
    expect_int("spec.field.byte_width", native->field_byte_width, 16, "DUNVIEW.C:731");
    expect_int("spec.field.height", native->field_height, 136, "DUNVIEW.C:731");
    expect_int("spec.field.source_x", native->field_source_x, 0, "DUNVIEW.C:731");
    expect_int("spec.field.word_count", native->field_bitplane_word_count, 64,
               "DUNVIEW.C:731");
    expect_int("spec.c10", native->transparent_color, 10, "DEFS.H:2088");
    expect_int("spec.uses_f0100", native->uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:8119");
    expect_int("spec.uses_f0104", native->uses_f0104_c10_wall_blit ? 1 : 0, 1,
               "DUNVIEW.C:8139");
    expect_int("spec.parity.f0105", parity->uses_f0105_c10_flipped_wall_blit ? 1 : 0, 1,
               "DUNVIEW.C:8127");
    expect_int("spec.parity.opposite_bitmap",
               parity->parity_uses_opposite_native_bitmap ? 1 : 0, 1,
               "DUNVIEW.C:8127 flips C01_D0L into D0R");
    expect_int("spec.return.before_f0112",
               native->wall_case_returns_before_f0112 ? 1 : 0, 1,
               "DUNVIEW.C:8144 before DUNVIEW.C:8146");
    expect_int("spec.return.before_f0115",
               native->wall_case_returns_before_f0115 ? 1 : 0, 1,
               "DUNVIEW.C:8144 before F0115 branch");
    expect_int("spec.no_f0111", native->calls_f0111_door ? 1 : 0, 0,
               "F0126 D0R wall case has no F0111");
    expect_int("spec.disjoint", native->disjoint_from_d0l2_wall_anchor ? 1 : 0, 1,
               "F0126 not F0125");
    expect_int("spec.no_link_unlink", native->thing_list_link_unlink_excluded ? 1 : 0, 1,
               "DUNGEON.C F0163/F0164 excluded");
    expect_contains("spec.native.name", native->route_name, "F0104", "native route");
    expect_contains("spec.parity.name", parity->route_name, "F0105", "parity route");
    expect_contains("spec.native.lines", native->source_lines, "DUNVIEW.C:8117-8144",
                    "source lines");
    expect_contains("spec.parity.lines", parity->source_lines, "DUNVIEW.C:8127",
                    "source lines");
    expect_int("invalid.route",
               dm1_v1_viewport_d0r2_wall_route_spec_pc34(
                   (DM1_V1_D0R2WallRoutePc34)99) == NULL,
               1, "invalid route rejected");
}

static void test_pixel_run_contract(void)
{
    DM1_V1_D0R2WallRunPc34 run;
    const DM1_V1_D0R2WallPixelPc34 *c;

    expect_int("run.ok", dm1_v1_viewport_d0r2_wall_pc34_compat_run(&run) ? 1 : 0,
               1, "synthetic D0R2 contract run");
    c = run.checks;
    expect_int("run.ok_field", run.ok ? 1 : 0, 1, "run field");
    expect_int("run.count", (int)run.check_count, 13, "8-wide x 4-tall check set");
    expect_int("run.c10", run.c10_palette_index, 10, "DEFS.H:2088");
    expect_int("run.c112", run.c112_byte_width_viewport, 112, "DEFS.H:2478");
    expect_int("run.view.d0r", run.d0r_view_square_pc34, 2, "DEFS.H:2598");
    expect_int("run.view.d0l", run.d0l_view_square_pc34, 1, "DEFS.H:2597");
    expect_int("run.wall.d0r", run.d0r_wall_pc34, 0, "DEFS.H:3423");
    expect_int("run.wall.d0l", run.d0l_wall_pc34, 1, "DEFS.H:3424");
    expect_int("run.zone.d0r.old", run.d0r_zone_old_media_pc34, 715, "DEFS.H:4038");
    expect_int("run.zone.d0r", run.d0r_zone_pc34, 717, "DEFS.H:4057");
    expect_int("run.zone.d0l", run.d0l_zone_pc34, 716, "DEFS.H:4056");
    expect_int("run.zone.ceiling_d0r", run.ceiling_pit_d0r_zone_pc34, 872,
               "DEFS.H:4219");
    expect_int("run.f0128.d0l_before_d0r", run.f0128_dispatch_d0l_before_d0r ? 1 : 0,
               1, "DUNVIEW.C:8536-8541");
    expect_int("run.wall_return_before_f0112",
               run.f0126_wall_returns_before_ceiling_pit ? 1 : 0, 1,
               "DUNVIEW.C:8144");
    expect_int("run.wall_return_before_f0115",
               run.f0126_wall_returns_before_thing_pass ? 1 : 0, 1,
               "DUNVIEW.C:8144");
    expect_int("run.corridor_branch",
               run.f0126_corridor_branch_has_f0112_and_f0115 ? 1 : 0, 1,
               "DUNVIEW.C:8106-8115");
    expect_int("run.f0117_family", run.f0117_family_right_wall_return_anchor ? 1 : 0,
               1, "DUNVIEW.C:6500-6622");
    expect_int("run.f0163_f0164_excluded",
               run.f0163_f0164_not_part_of_wall_return ? 1 : 0, 1,
               "DUNGEON.C:1769-1905");
    expect_int("run.f0172", run.f0172_square_aspect_feeds_wall_switch ? 1 : 0, 1,
               "DUNGEON.C:2466-2523");
    expect_int("run.c10_preserve", run.c10_flesh_pixels_preserve_destination ? 1 : 0, 1,
               "DUNVIEW.C:3055");
    expect_int("run.right_edge", run.right_edge_clipped ? 1 : 0, 1,
               "synthetic right-edge clipping");
    expect_int("run.no_f0111", run.no_f0111_marker ? 1 : 0, 1,
               "D0R wall case excludes F0111");

    expect_int("native.skip.in_clip", c[0].in_clip ? 1 : 0, 1, "synthetic x=0");
    expect_int("native.skip.src_x", c[0].source_x, 0, "synthetic x=0");
    expect_int("native.skip.selected", c[0].selected_source_x, 0, "native no flip");
    expect_int("native.skip.c10", c[0].transparent_skip ? 1 : 0, 1, "C10 skip");
    expect_int("native.skip.no_write", c[0].writes_pixel ? 1 : 0, 0, "C10 no write");
    expect_int("native.skip.after", c[0].pixel_after, 0xee, "C10 preserves destination");
    expect_int("native.opaque.selected", c[1].selected_source_x, 1, "native no flip");
    expect_int("native.opaque.write", c[1].writes_pixel ? 1 : 0, 1, "opaque write");
    expect_int("native.opaque.value", c[1].pixel_after, 0x21, "opaque value");
    expect_int("native.edge.selected", c[2].selected_source_x, 7, "right edge selected");
    expect_int("native.edge.value", c[2].pixel_after, 0x7a, "right edge write");
    expect_int("native.after_edge.no_write", c[3].no_write_metadata ? 1 : 0, 1,
               "x=8 clipped");
    expect_int("native.row1.offset", (int)c[4].source_offset, 8, "row 1 offset");
    expect_int("native.row1.value", c[4].pixel_after, 0x32, "row 1 value");
    expect_int("native.bottom.offset", (int)c[5].source_offset, 31, "bottom offset");
    expect_int("native.bottom.value", c[5].pixel_after, 0x4f, "bottom value");
    expect_int("native.after_bottom.no_write", c[6].no_write_metadata ? 1 : 0, 1,
               "row 4 clipped");

    expect_int("parity.skip.flip", c[7].parity_flip ? 1 : 0, 1, "F0105 flip");
    expect_int("parity.skip.scratch", c[7].uses_scratch ? 1 : 0, 1, "F0105 scratch");
    expect_int("parity.skip.selected", c[7].selected_source_x, 7, "flipped selected x=7");
    expect_int("parity.skip.c10", c[7].transparent_skip ? 1 : 0, 1, "flipped C10 skip");
    expect_int("parity.skip.after", c[7].pixel_after, 0xee, "C10 preserved");
    expect_int("parity.next.selected", c[8].selected_source_x, 6, "flipped x=1 selects 6");
    expect_int("parity.next.value", c[8].pixel_after, 0x63, "flipped opaque value");
    expect_int("parity.edge.selected", c[9].selected_source_x, 0, "flipped x=7 selects 0");
    expect_int("parity.edge.value", c[9].pixel_after, 0x6e, "flipped edge value");
    expect_int("parity.row1.offset", (int)c[10].source_offset, 15, "row1 flipped offset");
    expect_int("parity.row1.value", c[10].pixel_after, 0x70, "row1 flipped value");
    expect_int("parity.bottom.selected", c[11].selected_source_x, 0, "bottom x=7 selects 0");
    expect_int("parity.bottom.after", c[11].pixel_after, 0x4f,
               "bottom C10 skip preserves previous destination");
    expect_int("parity.after_edge.no_write", c[12].no_write_metadata ? 1 : 0, 1,
               "parity x=8 clipped");

    expect_int("blend.c10",
               dm1_v1_viewport_d0r2_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "DUNVIEW.C:3055 C10 skip");
    expect_int("blend.opaque",
               dm1_v1_viewport_d0r2_wall_blend_pixel_pc34(0x44, 0x55, 10),
               0x55, "DUNVIEW.C:3055 opaque write");
    expect_int("invalid.null_run",
               dm1_v1_viewport_d0r2_wall_pc34_compat_run(NULL) ? 1 : 0,
               0, "null run rejected");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d0r2_wall_source_evidence_pc34();

    expect_contains("evidence.contract", e, "contract_only=1", "source evidence");
    expect_contains("evidence.no_asset", e, "no_asset_parity=1", "source evidence");
    expect_contains("evidence.f0126", e, "DUNVIEW.C:8064-8162", "F0126 body");
    expect_contains("evidence.wall_case", e, "DUNVIEW.C:8117-8144", "F0126 wall case");
    expect_contains("evidence.f0117", e, "DUNVIEW.C:6500-6622", "F0117 family");
    expect_contains("evidence.f0125", e, "DUNVIEW.C:7960-8062", "F0125 sibling");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8508-8542", "F0128 row");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100", "F0100");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104", "F0104");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105", "F0105");
    expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838 F0163", "F0163");
    expect_contains("evidence.f0164", e, "DUNGEON.C:1840-1905 F0164", "F0164");
    expect_contains("evidence.f0172", e, "DUNGEON.C:2466-2523", "F0172");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH=10", "C10");
    expect_contains("evidence.square", e, "DEFS.H:2598 M611_VIEW_SQUARE_D0R=2",
                    "view square");
    expect_contains("evidence.c00", e, "DEFS.H:3423 C00_WALL_D0R=0", "D0R wall");
    expect_contains("evidence.c01", e, "DEFS.H:3424 C01_WALL_D0L=1", "D0L wall");
    expect_contains("evidence.c715", e, "DEFS.H:4038 C715_ZONE_WALL_D0R=715",
                    "old zone");
    expect_contains("evidence.c716", e, "DEFS.H:4056 C716_ZONE_WALL_D0L=716",
                    "D0L zone");
    expect_contains("evidence.c717", e, "DEFS.H:4057 C717_ZONE_WALL_D0R=717",
                    "D0R zone");
    expect_contains("evidence.c872", e, "DEFS.H:4219 C872_ZONE_CEILING_PIT_D0R=872",
                    "ceiling zone");
    expect_contains("evidence.g0163", e, "DUNVIEW.C:594 G0163 D0R frame row",
                    "frame row");
    expect_contains("evidence.g0188", e, "DUNVIEW.C:731 G0188 D0R field row",
                    "field row");
    expect_contains("evidence.no_f0111", e, "no F0111 door marker", "no-F0111");
    expect_contains("evidence.no_f0112", e, "no F0112 ceiling-pit marker", "no-F0112");
    expect_contains("evidence.no_f0115", e, "no F0115 thing-pass marker", "no-F0115");
    expect_contains("evidence.return", e, "DUNVIEW.C:8144", "return path");
}

int main(void)
{
    test_anchor_table_source_locked();
    test_route_specs_source_locked();
    test_pixel_run_contract();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0r2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0r2_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
