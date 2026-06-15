#include "csb_v1_viewport_d3l_d3r_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136,
    SOURCE_WIDTH = 64,
    SOURCE_HEIGHT = 51,
    TRANSPARENT = 10
};

static const char *A_F0116 =
    "ReDMCSB DUNVIEW.C F0116:6361-6480 D3L wall composition";
static const char *A_F0117 =
    "ReDMCSB DUNVIEW.C F0117:6500-6622 D3R wall composition";
static const char *A_F0104 =
    "ReDMCSB DUNVIEW.C F0104:3113-3156 native wall blit";
static const char *A_F0105 =
    "ReDMCSB DUNVIEW.C F0105:3185-3247 flipped wall blit";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107:3502-3938 side/front wall ornaments";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128:8478-8500 D3L/D3R ordering";
static const char *A_DEFS_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency";
static const char *A_DEFS_SQUARES =
    "ReDMCSB DEFS.H:2608-2609 C12/C13 D3L/D3R view squares";
static const char *A_DEFS_ZONES =
    "ReDMCSB DEFS.H:4045-4046 C705/C706 wall zones";
static const char *A_DEFS_WALL_VIEWS =
    "ReDMCSB DEFS.H:2698-2702 M575/M576/M577/M579 wall ornament views";
static const char *A_DEFS_FLOOR_VIEWS =
    "ReDMCSB DEFS.H:2752-2754 M588/M590 floor ornament contrast";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915 overlay shape";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        ++g_failures;
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != 0;
    return expect_int(label, got, 1, anchor);
}

static size_t viewport_offset(int y, int x)
{
    return (size_t)y * VIEWPORT_WIDTH + (size_t)x;
}

static size_t source_offset(int y, int x)
{
    return (size_t)y * SOURCE_WIDTH + (size_t)x;
}

static int test_specs(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RWallSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34);
    const CSB_V1_D3LD3RWallSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d3l_d3r_wall_spec_count_pc34(), 2,
                     A_F0128);
    ok &= expect_int("spec.index0.d3l",
                     csb_v1_viewport_d3l_d3r_wall_spec_at_pc34(0) == d3l, 1,
                     A_F0128);
    ok &= expect_int("spec.index1.d3r",
                     csb_v1_viewport_d3l_d3r_wall_spec_at_pc34(1) == d3r, 1,
                     A_F0128);
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d3l_d3r_wall_spec_at_pc34(2) == 0, 1,
                     A_F0128);
    ok &= expect_int("spec.unknown.null",
                     csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(99) == 0,
                     1, A_F0128);
    ok &= expect_int("d3l.function", d3l ? d3l->redmcsb_function_number : -1,
                     116, A_F0116);
    ok &= expect_int("d3r.function", d3r ? d3r->redmcsb_function_number : -1,
                     117, A_F0117);
    ok &= expect_int("d3l.view_square", d3l ? d3l->view_square : -1, 12,
                     A_DEFS_SQUARES);
    ok &= expect_int("d3r.view_square", d3r ? d3r->view_square : -1, 13,
                     A_DEFS_SQUARES);
    ok &= expect_int("d3l.depth", d3l ? d3l->relative_depth : -1, 3,
                     A_F0128);
    ok &= expect_int("d3r.depth", d3r ? d3r->relative_depth : -1, 3,
                     A_F0128);
    ok &= expect_int("d3l.lateral", d3l ? d3l->relative_lateral : 0, -1,
                     A_F0128);
    ok &= expect_int("d3r.lateral", d3r ? d3r->relative_lateral : 0, 1,
                     A_F0128);
    ok &= expect_int("d3l.order", d3l ? d3l->f0128_order_index : -1, 2,
                     A_F0128);
    ok &= expect_int("d3r.order", d3r ? d3r->f0128_order_index : -1, 3,
                     A_F0128);
    ok &= expect_int("d3l.wall_element", d3l ? d3l->wall_element : -1, 0,
                     A_F0116);
    ok &= expect_int("d3r.wall_element", d3r ? d3r->wall_element : -1, 0,
                     A_F0117);
    ok &= expect_int("d3l.zone", d3l ? d3l->wall_zone : -1, 705,
                     A_DEFS_ZONES);
    ok &= expect_int("d3r.zone", d3r ? d3r->wall_zone : -1, 706,
                     A_DEFS_ZONES);
    ok &= expect_int("d3l.native_wall", d3l ? d3l->native_wall_index : -1, 13,
                     A_F0116);
    ok &= expect_int("d3r.native_wall", d3r ? d3r->native_wall_index : -1, 12,
                     A_F0117);
    ok &= expect_int("d3l.flipped_wall", d3l ? d3l->flipped_wall_index : -1,
                     12, A_F0105);
    ok &= expect_int("d3r.flipped_wall", d3r ? d3r->flipped_wall_index : -1,
                     13, A_F0105);

    return ok;
}

static int test_frames_and_ornaments(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RWallSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34);
    const CSB_V1_D3LD3RWallSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34);

    ok &= expect_int("d3l.frame_row", d3l ? d3l->wall_frame_row : -1, 1,
                     "ReDMCSB DUNVIEW.C:581-585 G0163 row D3L");
    ok &= expect_int("d3r.frame_row", d3r ? d3r->wall_frame_row : -1, 2,
                     "ReDMCSB DUNVIEW.C:581-585 G0163 row D3R");
    ok &= expect_int("d3l.frame_x1", d3l ? d3l->wall_frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:584 D3L x1");
    ok &= expect_int("d3l.frame_x2", d3l ? d3l->wall_frame_x2 : -1, 83,
                     "ReDMCSB DUNVIEW.C:584 D3L x2");
    ok &= expect_int("d3r.frame_x1", d3r ? d3r->wall_frame_x1 : -1, 139,
                     "ReDMCSB DUNVIEW.C:585 D3R x1");
    ok &= expect_int("d3r.frame_x2", d3r ? d3r->wall_frame_x2 : -1, 223,
                     "ReDMCSB DUNVIEW.C:585 D3R x2");
    ok &= expect_int("d3l.frame_y1", d3l ? d3l->wall_frame_y1 : -1, 25,
                     "ReDMCSB DUNVIEW.C:584 D3L y1");
    ok &= expect_int("d3l.frame_y2", d3l ? d3l->wall_frame_y2 : -1, 75,
                     "ReDMCSB DUNVIEW.C:584 D3L y2");
    ok &= expect_int("d3r.frame_y1", d3r ? d3r->wall_frame_y1 : -1, 25,
                     "ReDMCSB DUNVIEW.C:585 D3R y1");
    ok &= expect_int("d3r.frame_y2", d3r ? d3r->wall_frame_y2 : -1, 75,
                     "ReDMCSB DUNVIEW.C:585 D3R y2");
    ok &= expect_int("d3l.byte_width", d3l ? d3l->wall_frame_byte_width : -1,
                     64, "ReDMCSB DUNVIEW.C:584 D3L byte width");
    ok &= expect_int("d3r.byte_width", d3r ? d3r->wall_frame_byte_width : -1,
                     64, "ReDMCSB DUNVIEW.C:585 D3R byte width");
    ok &= expect_int("d3l.height", d3l ? d3l->wall_frame_height : -1, 51,
                     "ReDMCSB DUNVIEW.C:584 D3L height");
    ok &= expect_int("d3r.height", d3r ? d3r->wall_frame_height : -1, 51,
                     "ReDMCSB DUNVIEW.C:585 D3R height");
    ok &= expect_int("d3l.source_x", d3l ? d3l->wall_frame_source_x : -1, 32,
                     "ReDMCSB DUNVIEW.C:584 D3L source X");
    ok &= expect_int("d3r.source_x", d3r ? d3r->wall_frame_source_x : -1, 0,
                     "ReDMCSB DUNVIEW.C:585 D3R source X");
    ok &= expect_int("d3l.side_slot",
                     d3l ? d3l->side_ornament_square_aspect_slot : -1, 551,
                     A_F0116);
    ok &= expect_int("d3r.side_slot",
                     d3r ? d3r->side_ornament_square_aspect_slot : -1, 553,
                     A_F0117);
    ok &= expect_int("d3l.front_slot",
                     d3l ? d3l->front_ornament_square_aspect_slot : -1, 552,
                     A_F0116);
    ok &= expect_int("d3r.front_slot",
                     d3r ? d3r->front_ornament_square_aspect_slot : -1, 552,
                     A_F0117);
    ok &= expect_int("d3l.side_view", d3l ? d3l->side_wall_ornament_view : -1,
                     2, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d3r.side_view", d3r ? d3r->side_wall_ornament_view : -1,
                     3, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d3l.front_view", d3l ? d3l->front_wall_ornament_view : -1,
                     4, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d3r.front_view", d3r ? d3r->front_wall_ornament_view : -1,
                     6, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d3l.floor_view", d3l ? d3l->floor_ornament_view : -1,
                     2, A_DEFS_FLOOR_VIEWS);
    ok &= expect_int("d3r.floor_view", d3r ? d3r->floor_ornament_view : -1,
                     4, A_DEFS_FLOOR_VIEWS);
    ok &= expect_int("d3l.f0104", d3l ? d3l->native_wall_blit_function : -1,
                     104, A_F0104);
    ok &= expect_int("d3r.f0104", d3r ? d3r->native_wall_blit_function : -1,
                     104, A_F0104);
    ok &= expect_int("d3l.f0105", d3l ? d3l->flipped_wall_blit_function : -1,
                     105, A_F0105);
    ok &= expect_int("d3r.f0105", d3r ? d3r->flipped_wall_blit_function : -1,
                     105, A_F0105);
    ok &= expect_int("d3l.side_f0107_order",
                     d3l ? d3l->side_f0107_order_index : -1, 0, A_F0107);
    ok &= expect_int("d3l.front_f0107_order",
                     d3l ? d3l->front_f0107_order_index : -1, 1, A_F0107);
    ok &= expect_int("d3r.side_f0107_order",
                     d3r ? d3r->side_f0107_order_index : -1, 2, A_F0107);
    ok &= expect_int("d3r.front_f0107_order",
                     d3r ? d3r->front_f0107_order_index : -1, 3, A_F0107);
    ok &= expect_int("d3l.c10", d3l ? d3l->c10_transparent_color : -1, 10,
                     A_DEFS_C10);
    ok &= expect_int("d3r.c10", d3r ? d3r->c10_transparent_color : -1, 10,
                     A_DEFS_C10);
    ok &= expect_int("d3l.scale", d3l ? d3l->f0107_depth3_scale : -1, 14,
                     A_F0107);
    ok &= expect_int("d3r.palette", d3r ? d3r->f0107_depth3_palette : -1, 3,
                     A_F0107);
    ok &= expect_int("d3l.lineage_open",
                     d3l ? d3l->lineage_open_room_shape : -1, 1192, A_LINEAGE);
    ok &= expect_int("d3r.lineage_door",
                     d3r ? d3r->lineage_door_front_overlay_shape : -1, 1903,
                     A_LINEAGE);

    return ok;
}

static int test_trace_and_transparency(void)
{
    int ok = 1;
    CSB_V1_D3LD3RWallTracePc34 trace;

    ok &= expect_int("trace.native.return",
                     csb_v1_viewport_d3l_d3r_wall_trace_pair_pc34(
                         0, 0, &trace),
                     0, A_F0128);
    ok &= expect_int("trace.native.ok", trace.ok, 1, A_F0128);
    ok &= expect_int("trace.native.contract", trace.source_locked_contract_only,
                     1, A_F0128);
    ok &= expect_int("trace.native.no_assets", trace.no_real_asset_bitmap_parity,
                     1, "contract-only source lock");
    ok &= expect_int("trace.native.no_data", trace.no_game_data_load, 1,
                     "asset-free source lock");
    ok &= expect_int("trace.native.order", trace.d3l_before_d3r, 1, A_F0128);
    ok &= expect_int("trace.native.wall_blits", trace.wall_blit_calls, 2,
                     A_F0104);
    ok &= expect_int("trace.native.f0104", trace.f0104_calls, 2, A_F0104);
    ok &= expect_int("trace.native.f0105", trace.f0105_calls, 0, A_F0105);
    ok &= expect_int("trace.native.side_f0107", trace.f0107_side_calls, 2,
                     A_F0107);
    ok &= expect_int("trace.native.front_f0107", trace.f0107_front_calls, 2,
                     A_F0107);
    ok &= expect_int("trace.native.no_f0115", trace.f0115_calls, 0, A_F0116);
    ok &= expect_int("trace.native.first_zone", trace.first_wall_zone, 705,
                     A_DEFS_ZONES);
    ok &= expect_int("trace.native.second_zone", trace.second_wall_zone, 706,
                     A_DEFS_ZONES);
    ok &= expect_int("trace.native.first_wall", trace.first_wall_index, 13,
                     A_F0116);
    ok &= expect_int("trace.native.second_wall", trace.second_wall_index, 12,
                     A_F0117);
    ok &= expect_int("trace.native.first_func", trace.first_blit_function, 104,
                     A_F0104);
    ok &= expect_int("trace.native.second_func", trace.second_blit_function, 104,
                     A_F0104);
    ok &= expect_int("trace.native.return_without_alcove",
                     trace.wall_returns_without_front_alcove, 1, A_F0116);
    ok &= expect_int("trace.native.side_views", trace.side_ornament_view,
                     (2 << 8) | 3, A_DEFS_WALL_VIEWS);
    ok &= expect_int("trace.native.front_views", trace.front_ornament_view,
                     (4 << 8) | 6, A_DEFS_WALL_VIEWS);
    ok &= expect_int("trace.native.c10", trace.c10_transparency_preserved, 1,
                     A_DEFS_C10);

    ok &= expect_int("trace.flip.return",
                     csb_v1_viewport_d3l_d3r_wall_trace_pair_pc34(
                         1, 1, &trace),
                     0, A_F0105);
    ok &= expect_int("trace.flip.f0104", trace.f0104_calls, 0, A_F0104);
    ok &= expect_int("trace.flip.f0105", trace.f0105_calls, 2, A_F0105);
    ok &= expect_int("trace.flip.first_wall", trace.first_wall_index, 12,
                     A_F0105);
    ok &= expect_int("trace.flip.second_wall", trace.second_wall_index, 13,
                     A_F0105);
    ok &= expect_int("trace.flip.first_func", trace.first_blit_function, 105,
                     A_F0105);
    ok &= expect_int("trace.flip.second_func", trace.second_blit_function, 105,
                     A_F0105);
    ok &= expect_int("trace.flip.f0115", trace.f0115_calls, 2, A_F0116);
    ok &= expect_int("trace.flip.alcove_zero",
                     trace.front_alcove_uses_zero_order, 1, A_F0117);
    ok &= expect_int("blend.transparent",
                     csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(0x7a, 10),
                     0x7a, A_DEFS_C10);
    ok &= expect_int("blend.opaque",
                     csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(0x7a, 0x33),
                     0x33, A_DEFS_C10);

    return ok;
}

static int test_synthetic_blit(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RWallSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34);
    const CSB_V1_D3LD3RWallSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34);
    uint8_t source[SOURCE_WIDTH * SOURCE_HEIGHT];
    uint8_t viewport[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
    CSB_V1_D3LD3RWallBlitStatsPc34 stats;

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 0)] = 0x21u;
    source[source_offset(0, 1)] = 0x22u;
    source[source_offset(50, 63)] = 0x23u;
    ok &= expect_int("blit.d3l.native",
                     csb_v1_viewport_d3l_d3r_wall_apply_c10_frame_clip_pc34(
                         d3l, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, A_F0104);
    ok &= expect_int("blit.d3l.transparent", stats.transparent_pixels,
                     (64 * 51) - 3, A_DEFS_C10);
    ok &= expect_int("blit.d3l.top_left", viewport[viewport_offset(25, 0)],
                     0x21, A_F0104);
    ok &= expect_int("blit.d3l.top_next", viewport[viewport_offset(25, 1)],
                     0x22, A_F0104);
    ok &= expect_int("blit.d3l.bottom_right", viewport[viewport_offset(75, 63)],
                     0x23, A_F0104);
    ok &= expect_int("blit.d3l.transparent_pixel",
                     viewport[viewport_offset(25, 2)], 0xee, A_DEFS_C10);
    ok &= expect_int("blit.d3l.after_band", viewport[viewport_offset(76, 0)],
                     0xee, "ReDMCSB DUNVIEW.C:584 D3L frame bottom");

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("blit.d3r.flip",
                     csb_v1_viewport_d3l_d3r_wall_apply_c10_frame_clip_pc34(
                         d3r, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, A_F0105);
    ok &= expect_int("blit.d3r.flipped_left",
                     viewport[viewport_offset(25, 202)], 0x21, A_F0105);
    ok &= expect_int("blit.d3r.flipped_next",
                     viewport[viewport_offset(25, 201)], 0x22, A_F0105);
    ok &= expect_int("blit.d3r.flipped_bottom",
                     viewport[viewport_offset(75, 139)], 0x23, A_F0105);
    ok &= expect_int("blit.d3r.left_neighbor",
                     viewport[viewport_offset(25, 138)], 0xee,
                     "ReDMCSB DUNVIEW.C:585 D3R frame left");
    ok &= expect_int("blit.reject",
                     csb_v1_viewport_d3l_d3r_wall_apply_c10_frame_clip_pc34(
                         d3r, source, 63, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     -1, A_F0104);
    ok &= expect_int("blit.reject.flag", stats.rejected, 1, A_F0104);

    return ok;
}

static int test_run_and_evidence(void)
{
    int ok = 1;
    CSB_V1_D3LD3RWallTracePc34 trace;
    const char *e = csb_v1_viewport_d3l_d3r_wall_source_evidence_pc34();

    ok &= expect_int("run.return",
                     csb_v1_viewport_d3l_d3r_wall_pc34_compat_run(&trace),
                     0, A_F0128);
    ok &= expect_int("run.ok", trace.ok, 1, A_F0128);
    ok &= expect_contains("evidence.F0116", e, "F0116", A_F0116);
    ok &= expect_contains("evidence.F0117", e, "F0117", A_F0117);
    ok &= expect_contains("evidence.F0104", e, "F0104", A_F0104);
    ok &= expect_contains("evidence.F0105", e, "F0105", A_F0105);
    ok &= expect_contains("evidence.F0107", e, "F0107", A_F0107);
    ok &= expect_contains("evidence.F0128", e, "F0128", A_F0128);
    ok &= expect_contains("evidence.C10", e, "C10_COLOR_FLESH", A_DEFS_C10);
    ok &= expect_contains("evidence.C705", e, "C705", A_DEFS_ZONES);
    ok &= expect_contains("evidence.C706", e, "C706", A_DEFS_ZONES);
    ok &= expect_contains("evidence.M575", e, "M575", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.M579", e, "M579", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1192-1209",
                          A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l_d3r_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l_d3r_wall_source_evidence_pc34());

    ok &= test_specs();
    ok &= test_frames_and_ornaments();
    ok &= test_trace_and_transparency();
    ok &= test_synthetic_blit();
    ok &= test_run_and_evidence();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    ok &= expect_int("assertion_count_at_least_60", g_assertions >= 60, 1,
                     "ReDMCSB DUNVIEW.C F0116/F0117/F0104/F0105/F0107/F0128");

    if (!ok || g_failures) {
        printf("FAIL csb_v1_viewport_d3l_d3r_wall_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS csb_v1_viewport_d3l_d3r_wall_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
