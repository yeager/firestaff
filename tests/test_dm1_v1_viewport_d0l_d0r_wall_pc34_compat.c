#include "dm1_v1_viewport_d0l_d0r_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void expect_contains(const char *id, const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s missing=%s\n", id, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s contains %s\n", id, needle);
    }
}

static void test_d0l_native_route_spec(void)
{
    DM1_V1_D0LD0RWallInputPc34 input = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        0,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallSpecPc34 spec;
    int source_x = -1;
    int scratch_x = -1;

    /* ReDMCSB: DUNVIEW.C:8007-8038 routes D0L WALL through C716 and returns. */
    expect_int("d0l.resolve", M11_GameView_D0LD0RWallResolvePc34(&input, &spec) ? 1 : 0, 1);
    expect_int("d0l.view_square", spec.view_square_index, 1);
    expect_int("d0l.wall_bitmap", spec.selected_wall_bitmap_index, 1);
    expect_int("d0l.opposite_bitmap", spec.opposite_wall_bitmap_index, 0);
    expect_int("d0l.zone", spec.pc34_wall_zone_index, 716);
    expect_int("d0l.zone_family_first", spec.pc34_wall_zone_family_first, 702);
    expect_int("d0l.zone_family_last", spec.pc34_wall_zone_family_last, 717);
    expect_int("d0l.source_x_first", spec.source_x_first, 16);
    expect_int("d0l.source_x_last", spec.source_x_last, 63);
    expect_int("d0l.viewport_x_first", spec.viewport_x_first, 0);
    expect_int("d0l.viewport_x_last", spec.viewport_x_last, 47);
    expect_int("d0l.f0104", spec.uses_f0104_native_blit ? 1 : 0, 1);
    expect_int("d0l.no_f0105", spec.uses_f0105_parity_scratch_flip ? 1 : 0, 0);
    expect_int("d0l.c10", spec.transparent_color, 10);
    expect_int("d0l.contract_only", strstr(spec.contract, "Source-locked contract gate only") != NULL, 1);

    expect_int("d0l.map.left",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 0, &source_x, &scratch_x) ? 1 : 0, 1);
    expect_int("d0l.map.left_source", source_x, 16);
    expect_int("d0l.map.left_scratch", scratch_x, 16);
    expect_int("d0l.map.right",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 47, &source_x, &scratch_x) ? 1 : 0, 1);
    expect_int("d0l.map.right_source", source_x, 63);
    expect_int("d0l.map.after_strip",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 48, &source_x, &scratch_x) ? 1 : 0, 0);
}

static void test_d0l_pixel_slice_and_c10(void)
{
    uint8_t source[DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D0L_D0R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D0L_D0R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D0LD0RWallInputPc34 input = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        0,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallPixelPc34 pixel;

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[15] = 0x33;
    source[16] = 10;
    source[17] = 0x42;
    source[63] = 0x7e;

    /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH preserves the destination pixel. */
    expect_int("d0l.pixel.c10",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 0, &pixel) ? 1 : 0, 1);
    expect_int("d0l.pixel.c10_visible", pixel.visible, 1);
    expect_int("d0l.pixel.c10_source_x", pixel.source_x, 16);
    expect_int("d0l.pixel.c10_preserved", pixel.pixel_after, 0xee);

    expect_int("d0l.pixel.opaque",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 1, &pixel) ? 1 : 0, 1);
    expect_int("d0l.pixel.opaque_source_x", pixel.source_x, 17);
    expect_int("d0l.pixel.opaque_written", pixel.pixel_after, 0x42);
    expect_int("d0l.pixel.last",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 47, &pixel) ? 1 : 0, 1);
    expect_int("d0l.pixel.last_source_x", pixel.source_x, 63);
    expect_int("d0l.pixel.last_written", pixel.pixel_after, 0x7e);
    expect_int("d0l.pixel.after_strip",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 48, &pixel) ? 1 : 0, 1);
    expect_int("d0l.pixel.after_strip_invisible", pixel.visible, 0);
    expect_int("d0l.pixel.after_strip_untouched",
               viewport[48], 0xee);
    expect_int("d0l.pixel.pre_span_not_copied",
               viewport[0] != 0x33, 1);
}

static void test_d0r_parity_route_spec_and_pixel_slice(void)
{
    uint8_t source[DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D0L_D0R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D0L_D0R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D0LD0RWallInputPc34 input = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0R_PARITY_PC34,
        0,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallSpecPc34 spec;
    DM1_V1_D0LD0RWallPixelPc34 pixel;
    int source_x = -1;
    int scratch_x = -1;

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[0] = 0x6e;
    source[46] = 0x52;
    source[47] = 10;
    source[48] = 0x33;

    /* ReDMCSB: DUNVIEW.C:8127 uses F0105 and the parity scratch flip. */
    expect_int("d0r.resolve", M11_GameView_D0LD0RWallResolvePc34(&input, &spec) ? 1 : 0, 1);
    expect_int("d0r.view_square", spec.view_square_index, 2);
    expect_int("d0r.parity_source_bitmap", spec.selected_wall_bitmap_index, 1);
    expect_int("d0r.native_partner_bitmap", spec.opposite_wall_bitmap_index, 0);
    expect_int("d0r.zone", spec.pc34_wall_zone_index, 717);
    expect_int("d0r.source_x_first", spec.source_x_first, 0);
    expect_int("d0r.source_x_last", spec.source_x_last, 47);
    expect_int("d0r.viewport_x_first", spec.viewport_x_first, 16);
    expect_int("d0r.viewport_x_last", spec.viewport_x_last, 63);
    expect_int("d0r.no_f0104", spec.uses_f0104_native_blit ? 1 : 0, 0);
    expect_int("d0r.f0105", spec.uses_f0105_parity_scratch_flip ? 1 : 0, 1);

    expect_int("d0r.map.left",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 16, &source_x, &scratch_x) ? 1 : 0, 1);
    expect_int("d0r.map.left_source", source_x, 47);
    expect_int("d0r.map.left_scratch", scratch_x, 0);
    expect_int("d0r.map.right",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 63, &source_x, &scratch_x) ? 1 : 0, 1);
    expect_int("d0r.map.right_source", source_x, 0);
    expect_int("d0r.map.before_strip",
               M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
                   &spec, 15, &source_x, &scratch_x) ? 1 : 0, 0);

    expect_int("d0r.pixel.c10_left",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 16, &pixel) ? 1 : 0, 1);
    expect_int("d0r.pixel.c10_preserved", pixel.pixel_after, 0xee);
    expect_int("d0r.pixel.next",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 17, &pixel) ? 1 : 0, 1);
    expect_int("d0r.pixel.next_source_x", pixel.source_x, 46);
    expect_int("d0r.pixel.next_written", pixel.pixel_after, 0x52);
    expect_int("d0r.pixel.right",
               M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), 63, &pixel) ? 1 : 0, 1);
    expect_int("d0r.pixel.right_source_x", pixel.source_x, 0);
    expect_int("d0r.pixel.right_written", pixel.pixel_after, 0x6e);
    expect_int("d0r.pixel.before_strip_untouched", viewport[15], 0xee);
    expect_int("d0r.pixel.after_strip_untouched", viewport[64], 0xee);
    expect_int("d0r.pixel.source48_not_copied", viewport[63] != 0x33, 1);
}

static void test_wall_case_excludes_other_routes_and_invalid_inputs(void)
{
    DM1_V1_D0LD0RWallInputPc34 input = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        135,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallInputPc34 bad_row = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        136,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallInputPc34 bad_route = {
        (DM1_V1_D0LD0RWallRoutePc34)99,
        0,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallSpecPc34 spec;

    expect_int("contract.resolve", M11_GameView_D0LD0RWallResolvePc34(&input, &spec) ? 1 : 0, 1);
    expect_int("contract.wall_returns", spec.wall_case_returns ? 1 : 0, 1);
    expect_int("contract.no_f0111", spec.calls_f0111_door ? 1 : 0, 0);
    expect_int("contract.no_f0115", spec.calls_f0115_thing_pass ? 1 : 0, 0);
    expect_int("contract.no_f0108", spec.calls_f0108_floor_ornament ? 1 : 0, 0);
    expect_int("contract.blend_transparent",
               M11_GameView_D0LD0RWallBlendPixelPc34(0x44, 10, 10), 0x44);
    expect_int("contract.blend_opaque",
               M11_GameView_D0LD0RWallBlendPixelPc34(0x44, 0x51, 10), 0x51);
    expect_int("invalid.null_input", M11_GameView_D0LD0RWallResolvePc34(NULL, &spec) ? 1 : 0, 0);
    expect_int("invalid.null_output", M11_GameView_D0LD0RWallResolvePc34(&input, NULL) ? 1 : 0, 0);
    expect_int("invalid.row", M11_GameView_D0LD0RWallResolvePc34(&bad_row, &spec) ? 1 : 0, 0);
    expect_int("invalid.route", M11_GameView_D0LD0RWallResolvePc34(&bad_route, &spec) ? 1 : 0, 0);
}

static void test_source_evidence_mentions_all_required_anchors(void)
{
    const char *e = M11_GameView_D0LD0RWallSourceLockPc34();

    expect_contains("evidence.contract_only", e, "Source-locked contract gate only");
    expect_contains("evidence.required_f0122_anchor", e, "F0122:7400-7600");
    expect_contains("evidence.d0l_wall", e, "DUNVIEW.C:8007-8038");
    expect_contains("evidence.d0r_wall", e, "8117-8144");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_contains("evidence.c701", e, "DEFS.H:4041 C701");
    expect_contains("evidence.c702", e, "DEFS.H:4042-4057 C702..C717");
    expect_contains("evidence.c716", e, "C716_ZONE_WALL_D0L");
    expect_contains("evidence.c717", e, "C717_ZONE_WALL_D0R");
    expect_contains("evidence.no_f0111", e, "No F0111_DUNGEONVIEW_DrawDoor");
    expect_contains("evidence.no_f0115", e, "no F0115 thing pass");
    expect_contains("evidence.no_f0108", e, "no F0108 floor-ornament");
}

int main(void)
{
    test_d0l_native_route_spec();
    test_d0l_pixel_slice_and_c10();
    test_d0r_parity_route_spec_and_pixel_slice();
    test_wall_case_excludes_other_routes_and_invalid_inputs();
    test_source_evidence_mentions_all_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l_d0r_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l_d0r_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
