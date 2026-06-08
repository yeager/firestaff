#include "dm1_v1_viewport_d1r2_wall_pc34_compat.h"

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
    const DM1_V1_D1R2WallAnchorPc34 *a =
        dm1_v1_viewport_d1r2_wall_anchor_table_pc34(&count);

    expect_int("anchors.count", (int)count, DM1_V1_D1R2_WALL_ANCHOR_COUNT_PC34,
               "ReDMCSB required anchor table");
    expect_int("anchors.pointer", a != NULL, 1, "anchor table pointer");

    expect_contains("anchor0.id", a[0].id, "F0123", "DUNVIEW.C:7559-7725");
    expect_contains("anchor0.file", a[0].file, "DUNVIEW.C", "DUNVIEW.C:7559-7725");
    expect_contains("anchor0.function", a[0].function, "DrawSquareD1R",
                    "DUNVIEW.C:7559-7725");
    expect_int("anchor0.first", a[0].line_first, 7559, "DUNVIEW.C F0123 start");
    expect_int("anchor0.last", a[0].line_last, 7725, "DUNVIEW.C F0123 end");
    expect_contains("anchor0.claim.wall", a[0].claim, "wall case",
                    "DUNVIEW.C:7604-7628");

    expect_contains("anchor1.function", a[1].function, "F0120", "DUNVIEW.C:7051-7242");
    expect_contains("anchor1.claim", a[1].claim, "F0100", "DUNVIEW.C:3048-3058");
    expect_int("anchor1.first", a[1].line_first, 7051, "DUNVIEW.C F0120 start");
    expect_int("anchor2.first", a[2].line_first, 3113, "DUNVIEW.C F0104");
    expect_int("anchor2.last", a[2].line_last, 3129, "DUNVIEW.C F0104");
    expect_contains("anchor2.claim", a[2].claim, "C10", "DUNVIEW.C:3128");
    expect_int("anchor3.first", a[3].line_first, 3185, "DUNVIEW.C F0105");
    expect_int("anchor3.last", a[3].line_last, 3204, "DUNVIEW.C F0105");
    expect_contains("anchor3.claim", a[3].claim, "scratch", "DUNVIEW.C:3199-3201");
    expect_int("anchor4.first", a[4].line_first, 8007, "DUNVIEW.C F0125");
    expect_int("anchor4.last", a[4].line_last, 8144, "DUNVIEW.C F0126");
    expect_contains("anchor4.function", a[4].function, "F0125/F0126",
                    "DUNVIEW.C:8007-8144");
    expect_int("anchor5.first", a[5].line_first, 8524, "DUNVIEW.C F0128 D1L");
    expect_int("anchor5.last", a[5].line_last, 8542, "DUNVIEW.C F0128 D0C");
    expect_contains("anchor5.claim", a[5].claim, "D1L", "DUNVIEW.C:8525");
    expect_contains("anchor5.claim.right", a[5].claim, "D1R", "DUNVIEW.C:8529");
    expect_contains("anchor6.file", a[6].file, "DEFS.H", "DEFS.H constants");
    expect_int("anchor6.first", a[6].line_first, 2088, "DEFS.H C10");
    expect_int("anchor6.last", a[6].line_last, 4054, "DEFS.H C714");
    expect_contains("anchor6.claim", a[6].claim, "C714", "DEFS.H:4054");
    expect_int("anchor7.first", a[7].line_first, 591, "DUNVIEW.C G0163 D1R");
    expect_int("anchor7.last", a[7].line_last, 591, "DUNVIEW.C G0163 D1R");
    expect_contains("anchor7.claim", a[7].claim, "160,223,9,119",
                    "DUNVIEW.C:591");
    expect_int("anchor8.first", a[8].line_first, 3159, "DUNVIEW.C F0765");
    expect_int("anchor8.last", a[8].line_last, 3304, "DUNVIEW.C F0792");
    expect_contains("anchor8.claim", a[8].claim, "non-claims",
                    "DUNVIEW.C:3159-3304");
}

static void test_route_specs_source_locked(void)
{
    const DM1_V1_D1R2WallRouteSpecPc34 *native =
        dm1_v1_viewport_d1r2_wall_route_spec_pc34(
            DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34);
    const DM1_V1_D1R2WallRouteSpecPc34 *parity =
        dm1_v1_viewport_d1r2_wall_route_spec_pc34(
            DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34);

    expect_int("spec.native.present", native != NULL, 1, "DUNVIEW.C:7604-7628");
    expect_int("spec.parity.present", parity != NULL, 1, "DUNVIEW.C:7613-7615");
    if (!native || !parity) return;

    expect_int("spec.native.contract", native->contract_only ? 1 : 0, 1,
               "contract-only gate");
    expect_int("spec.parity.contract", parity->contract_only ? 1 : 0, 1,
               "contract-only gate");
    expect_int("spec.no_asset_parity",
               native->real_asset_bitmap_parity || parity->real_asset_bitmap_parity, 0,
               "no real-asset parity claim");
    expect_int("spec.native.wall_case", native->wall_case ? 1 : 0, 1,
               "DUNVIEW.C:7604 case C00_ELEMENT_WALL");
    expect_int("spec.parity.wall_case", parity->wall_case ? 1 : 0, 1,
               "DUNVIEW.C:7604 case C00_ELEMENT_WALL");
    expect_int("spec.native.no_flip", native->parity_flip ? 1 : 0, 0,
               "DUNVIEW.C:7622 native F0104");
    expect_int("spec.parity.flip", parity->parity_flip ? 1 : 0, 1,
               "DUNVIEW.C:7614 parity F0105");
    expect_int("spec.depth", native->relative_depth, 1, "DUNVIEW.C:8528 D1R depth");
    expect_int("spec.lateral", native->relative_lateral, 1, "DUNVIEW.C:8528 D1R lateral");
    expect_int("spec.view_square", native->view_square_index, 5,
               "DEFS.H:2601 M608_VIEW_SQUARE_D1R=5");
    expect_int("spec.parity.view_square", parity->view_square_index, 5,
               "DEFS.H:2601 M608_VIEW_SQUARE_D1R=5");
    expect_int("spec.native.wall", native->native_wall_index_pc34, 2,
               "DEFS.H:3425 C02_WALL_D1R=2");
    expect_int("spec.native.opposite", native->opposite_wall_index_pc34, 3,
               "DEFS.H:3426 C03_WALL_D1L=3");
    expect_int("spec.native.wall_set", native->native_wall_set_index_pc34, -15,
               "DUNVIEW.C:138 G3009_i_WallSet_Wall_D1R=-15");
    expect_int("spec.opposite.wall_set", native->opposite_wall_set_index_pc34, -14,
               "DUNVIEW.C:137 G3008_i_WallSet_Wall_D1L=-14");
    expect_int("spec.parity.opposite", parity->opposite_wall_index_pc34, 3,
               "DUNVIEW.C:7614 G2107_WallSet[C03_WALL_D1L]");
    expect_int("spec.zone.pc34", native->wall_zone_pc34, 714,
               "DEFS.H:4054 C714_ZONE_WALL_D1R");
    expect_int("spec.zone.old", native->old_media_wall_zone_pc34, 712,
               "DEFS.H:4035 C712_ZONE_WALL_D1R older PC set");
    expect_int("spec.zone.parity", parity->wall_zone_pc34, 714,
               "DEFS.H:4054 C714_ZONE_WALL_D1R");
    expect_int("spec.frame.left", native->frame_left_x, 160, "DUNVIEW.C:591");
    expect_int("spec.frame.right", native->frame_right_x, 223, "DUNVIEW.C:591");
    expect_int("spec.frame.top", native->frame_top_y, 9, "DUNVIEW.C:591");
    expect_int("spec.frame.bottom", native->frame_bottom_y, 119, "DUNVIEW.C:591");
    expect_int("spec.frame.byte_width", native->frame_byte_width, 128, "DUNVIEW.C:591");
    expect_int("spec.frame.height", native->frame_height, 111, "DUNVIEW.C:591");
    expect_int("spec.frame.source_x", native->frame_source_x, 0, "DUNVIEW.C:591");
    expect_int("spec.frame.source_y", native->frame_source_y, 0, "DUNVIEW.C:591");
    expect_int("spec.field.index", native->field_aspect_index, 5,
               "G0188 maps M608 to D1R row");
    expect_int("spec.field.mask", native->field_mask, 0x02, "DUNVIEW.C:728");
    expect_int("spec.field.byte_width", native->field_byte_width, 32, "DUNVIEW.C:728");
    expect_int("spec.field.height", native->field_height, 111, "DUNVIEW.C:728");
    expect_int("spec.field.source_x", native->field_source_x, 0, "DUNVIEW.C:728");
    expect_int("spec.field.word_count", native->field_bitplane_word_count, 64,
               "DUNVIEW.C:728");
    expect_int("spec.c10", native->transparent_color, 10, "DEFS.H:2088");
    expect_int("spec.c10.parity", parity->transparent_color, 10, "DEFS.H:2088");
    expect_int("spec.uses_f0100", native->uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:7606");
    expect_int("spec.uses_f0104", native->uses_f0104_c10_wall_blit ? 1 : 0, 1,
               "DUNVIEW.C:7622");
    expect_int("spec.native.no_f0105", native->uses_f0105_c10_flipped_wall_blit ? 1 : 0, 0,
               "native route is not flipped");
    expect_int("spec.parity.no_f0104", parity->uses_f0104_c10_wall_blit ? 1 : 0, 0,
               "parity route uses F0105");
    expect_int("spec.parity.f0105", parity->uses_f0105_c10_flipped_wall_blit ? 1 : 0, 1,
               "DUNVIEW.C:7614");
    expect_int("spec.parity.opposite_bitmap",
               parity->parity_uses_opposite_native_bitmap ? 1 : 0, 1,
               "DUNVIEW.C:7614 flips C03_D1L into D1R");
    expect_int("spec.f0107", native->calls_f0107_wall_ornament_probe ? 1 : 0, 1,
               "DUNVIEW.C:7627");
    expect_int("spec.f0107.ordinal", native->f0107_wall_ornament_ordinal, 13,
               "DEFS.H:2709 M586_VIEW_WALL_D1R_LEFT");
    expect_int("spec.return.before_f0111", native->wall_case_returns_before_f0111 ? 1 : 0, 1,
               "DUNVIEW.C:7628 return before door case");
    expect_int("spec.return.before_f0115", native->wall_case_returns_before_f0115 ? 1 : 0, 1,
               "DUNVIEW.C:7628 return before thing pass");
    expect_int("spec.no_f0111", native->calls_f0111_door ? 1 : 0, 0,
               "DUNVIEW.C:7665 door route is separate");
    expect_int("spec.no_f0115", native->calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:7704 thing pass is corridor/door path");
    expect_int("spec.thing_marker", native->thing_pass_marker_excluded ? 1 : 0, 1,
               "no-F0115 marker");
    expect_contains("spec.native.name", native->route_name, "F0104", "native route");
    expect_contains("spec.parity.name", parity->route_name, "F0105", "parity route");
    expect_contains("spec.native.lines", native->source_lines, "DUNVIEW.C:7604-7628",
                    "source lines");
    expect_contains("spec.parity.lines", parity->source_lines, "DUNVIEW.C:7613-7615",
                    "source lines");
    expect_int("invalid.route",
               dm1_v1_viewport_d1r2_wall_route_spec_pc34(
                   (DM1_V1_D1R2WallRoutePc34)99) == NULL,
               1, "invalid route rejected");
}

static void test_pixel_run_contract(void)
{
    DM1_V1_D1R2WallRunPc34 run;
    const DM1_V1_D1R2WallPixelPc34 *c;

    expect_int("run.ok", dm1_v1_viewport_d1r2_wall_pc34_compat_run(&run) ? 1 : 0,
               1, "synthetic D1R2 contract run");
    c = run.checks;
    expect_int("run.ok_field", run.ok ? 1 : 0, 1, "run field");
    expect_int("run.count", (int)run.check_count, 13, "8-wide x 4-tall check set");
    expect_int("run.c10", run.c10_palette_index, 10, "DEFS.H:2088");
    expect_int("run.c112", run.c112_byte_width_viewport, 112, "DEFS.H:2478");
    expect_int("run.view.d1r", run.d1r_view_square_pc34, 5, "DEFS.H:2601");
    expect_int("run.view.d1l", run.d1l_view_square_pc34, 4, "DEFS.H:2600");
    expect_int("run.wall.d1r", run.d1r_wall_pc34, 2, "DEFS.H:3425");
    expect_int("run.wall.d1l", run.d1l_wall_pc34, 3, "DEFS.H:3426");
    expect_int("run.zone.d1r", run.d1r_zone_pc34, 714, "DEFS.H:4054");
    expect_int("run.zone.d1l", run.d1l_zone_pc34, 713, "DEFS.H:4053");
    expect_int("run.zone.d1c", run.d1c_zone_pc34, 712, "DEFS.H:4052");
    expect_int("run.f0107.ordinal", run.f0107_ordinal_pc34, 13, "DEFS.H:2709");
    expect_int("run.mirror", run.d1r2_is_right_side_mirror ? 1 : 0, 1,
               "D1R mirrors D1L through opposite bitmap");
    expect_int("run.native_f0104", run.native_route_uses_f0104 ? 1 : 0, 1,
               "DUNVIEW.C:7622");
    expect_int("run.parity_f0105", run.parity_route_uses_f0105 ? 1 : 0, 1,
               "DUNVIEW.C:7614");
    expect_int("run.scratch_flip", run.parity_scratch_flips_opposite_native_wall ? 1 : 0, 1,
               "DUNVIEW.C:3199-3201");
    expect_int("run.c10_preserve", run.c10_flesh_pixels_preserve_destination ? 1 : 0, 1,
               "DUNVIEW.C:3055");
    expect_int("run.right_edge", run.right_edge_clipped ? 1 : 0, 1,
               "synthetic right-edge clipping");
    expect_int("run.f0107_return", run.f0107_then_return ? 1 : 0, 1,
               "DUNVIEW.C:7627-7628");
    expect_int("run.f0128.left_right", run.f0128_dispatch_left_before_right ? 1 : 0, 1,
               "DUNVIEW.C:8524-8529");
    expect_int("run.no_f0111", run.no_f0111_marker ? 1 : 0, 1,
               "DUNVIEW.C:7628 no F0111");
    expect_int("run.no_f0115", run.no_f0115_thing_pass_marker ? 1 : 0, 1,
               "DUNVIEW.C:7628 no F0115");

    expect_int("native.skip.in_clip", c[0].in_clip ? 1 : 0, 1, "synthetic x=0");
    expect_int("native.skip.src_x", c[0].source_x, 0, "synthetic x=0");
    expect_int("native.skip.selected", c[0].selected_source_x, 0, "native no flip");
    expect_int("native.skip.c10", c[0].transparent_skip ? 1 : 0, 1, "C10 skip");
    expect_int("native.skip.no_write", c[0].writes_pixel ? 1 : 0, 0, "C10 no write");
    expect_int("native.skip.after", c[0].pixel_after, 0xee, "C10 preserves destination");
    expect_int("native.opaque.src_x", c[1].source_x, 1, "synthetic x=1");
    expect_int("native.opaque.selected", c[1].selected_source_x, 1, "native no flip");
    expect_int("native.opaque.write", c[1].writes_pixel ? 1 : 0, 1, "opaque write");
    expect_int("native.opaque.value", c[1].pixel_after, 0x21, "opaque value");
    expect_int("native.edge.src_x", c[2].source_x, 7, "right edge x=7");
    expect_int("native.edge.selected", c[2].selected_source_x, 7, "right edge selected");
    expect_int("native.edge.value", c[2].pixel_after, 0x7a, "right edge write");
    expect_int("native.after_edge.no_write", c[3].no_write_metadata ? 1 : 0, 1,
               "x=8 clipped");
    expect_int("native.after_edge.in_clip", c[3].in_clip ? 1 : 0, 0, "x=8 clipped");
    expect_int("native.row1.src_y", c[4].source_y, 1, "row 1");
    expect_int("native.row1.offset", (int)c[4].source_offset, 8, "row 1 offset");
    expect_int("native.row1.value", c[4].pixel_after, 0x32, "row 1 value");
    expect_int("native.bottom.src_y", c[5].source_y, 3, "bottom row");
    expect_int("native.bottom.offset", (int)c[5].source_offset, 31, "bottom offset");
    expect_int("native.bottom.value", c[5].pixel_after, 0x4f, "bottom value");
    expect_int("native.after_bottom.no_write", c[6].no_write_metadata ? 1 : 0, 1,
               "row 4 clipped");

    expect_int("parity.skip.flip", c[7].parity_flip ? 1 : 0, 1, "F0105 flip");
    expect_int("parity.skip.scratch", c[7].uses_scratch ? 1 : 0, 1, "F0105 scratch");
    expect_int("parity.skip.src_x", c[7].source_x, 0, "viewport x=0");
    expect_int("parity.skip.selected", c[7].selected_source_x, 7, "flipped selected x=7");
    expect_int("parity.skip.c10", c[7].transparent_skip ? 1 : 0, 1, "flipped C10 skip");
    expect_int("parity.skip.after", c[7].pixel_after, 0xee, "C10 preserved");
    expect_int("parity.next.selected", c[8].selected_source_x, 6, "flipped x=1 selects 6");
    expect_int("parity.next.value", c[8].pixel_after, 0x63, "flipped opaque value");
    expect_int("parity.edge.selected", c[9].selected_source_x, 0, "flipped x=7 selects 0");
    expect_int("parity.edge.value", c[9].pixel_after, 0x6e, "flipped edge value");
    expect_int("parity.row1.selected", c[10].selected_source_x, 7, "row1 x=0 selects 7");
    expect_int("parity.row1.offset", (int)c[10].source_offset, 15, "row1 flipped offset");
    expect_int("parity.row1.value", c[10].pixel_after, 0x70, "row1 flipped value");
    expect_int("parity.bottom.selected", c[11].selected_source_x, 0, "bottom x=7 selects 0");
    expect_int("parity.bottom.offset", (int)c[11].source_offset, 24, "bottom flipped offset");
    expect_int("parity.bottom.after", c[11].pixel_after, 0x4f,
               "bottom C10 skip preserves previous destination");
    expect_int("parity.after_edge.no_write", c[12].no_write_metadata ? 1 : 0, 1,
               "parity x=8 clipped");
    expect_int("parity.after_edge.in_clip", c[12].in_clip ? 1 : 0, 0,
               "parity x=8 clipped");

    expect_int("blend.c10",
               dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "DUNVIEW.C:3055 C10 skip");
    expect_int("blend.opaque",
               dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(0x44, 0x55, 10),
               0x55, "DUNVIEW.C:3055 opaque write");
    expect_int("invalid.null_run",
               dm1_v1_viewport_d1r2_wall_pc34_compat_run(NULL) ? 1 : 0,
               0, "null run rejected");
}

static void test_actual_g0163_frame_pixel_contract(void)
{
    uint8_t native_source[64 * 111];
    uint8_t parity_source[64 * 111];
    uint8_t viewport[DM1_V1_D1R2_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D1R2_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D1R2WallPixelPc34 out;
    const DM1_V1_D1R2WallRouteSpecPc34 *native =
        dm1_v1_viewport_d1r2_wall_route_spec_pc34(
            DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34);
    const DM1_V1_D1R2WallRouteSpecPc34 *parity =
        dm1_v1_viewport_d1r2_wall_route_spec_pc34(
            DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34);

    memset(native_source, DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(native_source));
    memset(parity_source, DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34,
           sizeof(parity_source));
    memset(viewport, 0xee, sizeof(viewport));

    native_source[0] = 0x11;
    native_source[1] = DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34;
    native_source[63] = 0x7d;
    native_source[110 * 64 + 63] = 0x6f;
    parity_source[63] = 0x3f;
    parity_source[62] = DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34;
    parity_source[0] = 0x2a;

    expect_int("frame.native.left.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 160, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 G0163 D1R x=160 y=9");
    expect_int("frame.native.left.in_clip", out.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:591 G0163 D1R frame");
    expect_int("frame.native.left.source_x", out.source_x, 0,
               "DUNVIEW.C:591 frame source x=0");
    expect_int("frame.native.left.source_y", out.source_y, 0,
               "DUNVIEW.C:591 frame source y=0");
    expect_int("frame.native.left.offset", (int)out.source_offset, 0,
               "DUNVIEW.C:3048-3058 F0100 source offset");
    expect_int("frame.native.left.viewport_offset", (int)out.viewport_offset,
               9 * 224 + 160, "DM1 PC34 C007 viewport row stride");
    expect_int("frame.native.left.value", out.pixel_after, 0x11,
               "synthetic source-locked D1R frame pixel");

    expect_int("frame.native.c10.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 161, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("frame.native.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("frame.native.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("frame.native.c10.preserved", out.pixel_after, 0xee,
               "C10 preserves destination");

    expect_int("frame.native.right.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 223, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 G0163 D1R x=223");
    expect_int("frame.native.right.source_x", out.source_x, 63,
               "DUNVIEW.C:591 D1R right edge");
    expect_int("frame.native.right.offset", (int)out.source_offset, 63,
               "DUNVIEW.C:3048-3058 F0100 source offset");
    expect_int("frame.native.right.value", out.pixel_after, 0x7d,
               "synthetic source-locked D1R right edge");

    expect_int("frame.native.bottom.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 119, 223, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 G0163 D1R y=119");
    expect_int("frame.native.bottom.source_y", out.source_y, 110,
               "DUNVIEW.C:591 D1R height=111");
    expect_int("frame.native.bottom.offset", (int)out.source_offset, 110 * 64 + 63,
               "DUNVIEW.C:3048-3058 F0100 bottom source offset");
    expect_int("frame.native.bottom.value", out.pixel_after, 0x6f,
               "synthetic source-locked D1R bottom edge");

    expect_int("frame.parity.left.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   parity, 9, 160, parity_source, sizeof(parity_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:7613-7615 F0105 flipped D1L into D1R");
    expect_int("frame.parity.left.selected", out.selected_source_x, 63,
               "DUNVIEW.C:3185-3204 F0105 scratch flip");
    expect_int("frame.parity.left.value", out.pixel_after, 0x3f,
               "synthetic flipped source pixel");

    expect_int("frame.parity.next.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   parity, 9, 161, parity_source, sizeof(parity_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3185-3204 F0105 flipped x=161");
    expect_int("frame.parity.next.selected", out.selected_source_x, 62,
               "DUNVIEW.C:3185-3204 F0105 scratch flip");
    expect_int("frame.parity.next.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 skip through F0105");

    expect_int("frame.parity.right.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   parity, 9, 223, parity_source, sizeof(parity_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:7613-7615 F0105 right edge");
    expect_int("frame.parity.right.selected", out.selected_source_x, 0,
               "DUNVIEW.C:3185-3204 F0105 right edge selects source left");
    expect_int("frame.parity.right.value", out.pixel_after, 0x2a,
               "synthetic flipped right-edge source pixel");

    expect_int("frame.outside.left.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 159, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 outside D1R left frame");
    expect_int("frame.outside.left.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:591 outside D1R left frame");
    expect_int("frame.outside.right.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 224, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 outside D1R right frame");
    expect_int("frame.outside.right.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:591 outside D1R right frame");
    expect_int("frame.outside.bottom.ok",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 120, 223, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:591 outside D1R bottom frame");
    expect_int("frame.outside.bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:591 outside D1R bottom frame");
    expect_int("frame.invalid.null_out",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   native, 9, 160, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), NULL) ? 1 : 0,
               0, "null frame output rejected");
    expect_int("frame.invalid.null_spec",
               dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
                   NULL, 9, 160, native_source, sizeof(native_source),
                   viewport, sizeof(viewport), &out) ? 1 : 0,
               0, "null frame spec rejected");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d1r2_wall_source_evidence_pc34();

    expect_contains("evidence.contract", e, "contract_only=1", "source evidence");
    expect_contains("evidence.no_asset", e, "no_asset_parity=1", "source evidence");
    expect_contains("evidence.f0123", e, "DUNVIEW.C:7559-7725", "F0123 body");
    expect_contains("evidence.wall_case", e, "DUNVIEW.C:7604-7628", "F0123 wall case");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100", "F0100");
    expect_contains("evidence.f0120", e, "DUNVIEW.C:7051-7242 F0120", "F0120");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104", "F0104");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105", "F0105");
    expect_contains("evidence.f0125", e, "DUNVIEW.C:8007-8038 F0125", "F0125");
    expect_contains("evidence.f0126", e, "DUNVIEW.C:8117-8144 F0126", "F0126");
    expect_contains("evidence.f0128.lr", e, "DUNVIEW.C:8524-8529", "F0128 D1L then D1R");
    expect_contains("evidence.f0128.follow", e, "DUNVIEW.C:8532-8542", "F0128 follow-up");
    expect_contains("evidence.f0765", e, "DUNVIEW.C:3159-3304 F0765/F0792",
                    "PC34 center paths");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH=10", "C10");
    expect_contains("evidence.square", e, "DEFS.H:2601 M608_VIEW_SQUARE_D1R=5",
                    "view square");
    expect_contains("evidence.c02", e, "DEFS.H:3425 C02_WALL_D1R=2", "D1R wall");
    expect_contains("evidence.c03", e, "DEFS.H:3426 C03_WALL_D1L=3", "D1L wall");
    expect_contains("evidence.c712", e, "DEFS.H:4052 C712_ZONE_WALL_D1C=712",
                    "C712 nearby zone");
    expect_contains("evidence.c714", e, "DEFS.H:4054 C714_ZONE_WALL_D1R=714",
                    "D1R zone");
    expect_contains("evidence.g0163", e, "DUNVIEW.C:591 G0163 D1R frame row",
                    "frame row");
    expect_contains("evidence.g0188", e, "DUNVIEW.C:728 G0188 D1R field row",
                    "field row");
    expect_contains("evidence.f0107", e, "M586_VIEW_WALL_D1R_LEFT", "F0107 ordinal");
    expect_contains("evidence.no_f0111", e, "no F0111 door", "no-F0111 marker");
    expect_contains("evidence.no_f0115", e, "no F0115 thing-pass marker",
                    "no-F0115 marker");
    expect_contains("evidence.return", e, "DUNVIEW.C:7628", "return path");
}

int main(void)
{
    test_anchor_table_source_locked();
    test_route_specs_source_locked();
    test_pixel_run_contract();
    test_actual_g0163_frame_pixel_contract();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1r2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1r2_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
