#include "csb_v1_viewport_d2l_d2r_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136,
    SOURCE_WIDTH = 144,
    SOURCE_HEIGHT = 71,
    TRANSPARENT = 10
};

static const char *A_F0119 =
    "ReDMCSB DUNVIEW.C F0119:6900-7049 D2L wall composition";
static const char *A_F0120 =
    "ReDMCSB DUNVIEW.C F0120:7051-7242 D2R wall composition";
static const char *A_F0104 =
    "ReDMCSB DUNVIEW.C F0104:3113-3156 native wall blit";
static const char *A_F0105 =
    "ReDMCSB DUNVIEW.C F0105:3185-3247 flipped wall blit";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107:3502-3938 D2 side/front wall ornaments";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128:8504-8521 D2L/D2R before D2C";
static const char *A_DEFS_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency";
static const char *A_DEFS_SQUARES =
    "ReDMCSB DEFS.H:2603-2604 M604/M605 D2L/D2R view squares";
static const char *A_DEFS_NOT_C12 =
    "ReDMCSB DEFS.H:2608-2609 C12/C13 are D3L/D3R, not D2";
static const char *A_DEFS_ZONES =
    "ReDMCSB DEFS.H:4050-4051 C710/C711 wall zones";
static const char *A_DEFS_WALLS =
    "ReDMCSB DEFS.H:3430-3431 C07/C08 D2 wall set indices";
static const char *A_DEFS_WALL_VIEWS =
    "ReDMCSB DEFS.H:2703-2707 M580/M581/M582/M584 wall ornament views";
static const char *A_DEFS_FLOOR_VIEWS =
    "ReDMCSB DEFS.H:2755-2757 M591/M593 floor ornament contrast";
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
    const CSB_V1_D2LD2RWallSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const CSB_V1_D2LD2RWallSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2l_d2r_wall_spec_count_pc34(), 2,
                     A_F0128);
    ok &= expect_int("spec.index0.d2l",
                     csb_v1_viewport_d2l_d2r_wall_spec_at_pc34(0) == d2l, 1,
                     A_F0128);
    ok &= expect_int("spec.index1.d2r",
                     csb_v1_viewport_d2l_d2r_wall_spec_at_pc34(1) == d2r, 1,
                     A_F0128);
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d2l_d2r_wall_spec_at_pc34(2) == 0, 1,
                     A_F0128);
    ok &= expect_int("spec.unknown.null",
                     csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(99) == 0,
                     1, A_F0128);
    ok &= expect_int("d2l.function", d2l ? d2l->redmcsb_function_number : -1,
                     119, A_F0119);
    ok &= expect_int("d2r.function", d2r ? d2r->redmcsb_function_number : -1,
                     120, A_F0120);
    ok &= expect_int("d2l.view_square", d2l ? d2l->view_square : -1, 7,
                     A_DEFS_SQUARES);
    ok &= expect_int("d2r.view_square", d2r ? d2r->view_square : -1, 8,
                     A_DEFS_SQUARES);
    ok &= expect_int("d2l.reject_c12",
                     d2l ? d2l->rejected_d3_view_square : -1, 12,
                     A_DEFS_NOT_C12);
    ok &= expect_int("d2r.reject_c13",
                     d2r ? d2r->rejected_d3_view_square : -1, 13,
                     A_DEFS_NOT_C12);
    ok &= expect_int("d2l.depth", d2l ? d2l->relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2r.depth", d2r ? d2r->relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2l.lateral", d2l ? d2l->relative_lateral : 0, -1,
                     A_F0128);
    ok &= expect_int("d2r.lateral", d2r ? d2r->relative_lateral : 0, 1,
                     A_F0128);
    ok &= expect_int("d2l.order", d2l ? d2l->f0128_order_index : -1, 2,
                     A_F0128);
    ok &= expect_int("d2r.order", d2r ? d2r->f0128_order_index : -1, 3,
                     A_F0128);
    ok &= expect_int("d2l.d2c_order", d2l ? d2l->d2c_f0128_order_index : -1,
                     4, A_F0128);
    ok &= expect_int("d2r.d2c_order", d2r ? d2r->d2c_f0128_order_index : -1,
                     4, A_F0128);
    ok &= expect_int("d2l.before_d2c",
                     d2l ? d2l->f0128_order_index < d2l->d2c_f0128_order_index : 0,
                     1, A_F0128);
    ok &= expect_int("d2r.before_d2c",
                     d2r ? d2r->f0128_order_index < d2r->d2c_f0128_order_index : 0,
                     1, A_F0128);
    ok &= expect_int("d2l.wall_element", d2l ? d2l->wall_element : -1, 0,
                     A_F0119);
    ok &= expect_int("d2r.wall_element", d2r ? d2r->wall_element : -1, 0,
                     A_F0120);
    ok &= expect_int("d2l.zone", d2l ? d2l->wall_zone : -1, 710,
                     A_DEFS_ZONES);
    ok &= expect_int("d2r.zone", d2r ? d2r->wall_zone : -1, 711,
                     A_DEFS_ZONES);
    ok &= expect_int("d2l.native_wall", d2l ? d2l->native_wall_index : -1, 8,
                     A_DEFS_WALLS);
    ok &= expect_int("d2r.native_wall", d2r ? d2r->native_wall_index : -1, 7,
                     A_DEFS_WALLS);
    ok &= expect_int("d2l.flipped_wall", d2l ? d2l->flipped_wall_index : -1,
                     7, A_F0105);
    ok &= expect_int("d2r.flipped_wall", d2r ? d2r->flipped_wall_index : -1,
                     8, A_F0105);

    return ok;
}

static int test_frames_and_ornaments(void)
{
    int ok = 1;
    const CSB_V1_D2LD2RWallSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const CSB_V1_D2LD2RWallSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34);

    ok &= expect_int("d2l.frame_row", d2l ? d2l->wall_frame_row : -1, 4,
                     "ReDMCSB DUNVIEW.C:587 G0163 row D2L");
    ok &= expect_int("d2r.frame_row", d2r ? d2r->wall_frame_row : -1, 5,
                     "ReDMCSB DUNVIEW.C:588 G0163 row D2R");
    ok &= expect_int("d2l.frame_x1", d2l ? d2l->wall_frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:587 D2L x1");
    ok &= expect_int("d2l.frame_x2", d2l ? d2l->wall_frame_x2 : -1, 74,
                     "ReDMCSB DUNVIEW.C:587 D2L x2");
    ok &= expect_int("d2r.frame_x1", d2r ? d2r->wall_frame_x1 : -1, 149,
                     "ReDMCSB DUNVIEW.C:588 D2R x1");
    ok &= expect_int("d2r.frame_x2", d2r ? d2r->wall_frame_x2 : -1, 223,
                     "ReDMCSB DUNVIEW.C:588 D2R x2");
    ok &= expect_int("d2l.frame_y1", d2l ? d2l->wall_frame_y1 : -1, 20,
                     "ReDMCSB DUNVIEW.C:587 D2L y1");
    ok &= expect_int("d2l.frame_y2", d2l ? d2l->wall_frame_y2 : -1, 90,
                     "ReDMCSB DUNVIEW.C:587 D2L y2");
    ok &= expect_int("d2r.frame_y1", d2r ? d2r->wall_frame_y1 : -1, 20,
                     "ReDMCSB DUNVIEW.C:588 D2R y1");
    ok &= expect_int("d2r.frame_y2", d2r ? d2r->wall_frame_y2 : -1, 90,
                     "ReDMCSB DUNVIEW.C:588 D2R y2");
    ok &= expect_int("d2l.byte_width", d2l ? d2l->wall_frame_byte_width : -1,
                     72, "ReDMCSB DUNVIEW.C:587 D2L byte width");
    ok &= expect_int("d2r.byte_width", d2r ? d2r->wall_frame_byte_width : -1,
                     72, "ReDMCSB DUNVIEW.C:588 D2R byte width");
    ok &= expect_int("d2l.height", d2l ? d2l->wall_frame_height : -1, 71,
                     "ReDMCSB DUNVIEW.C:587 D2L height");
    ok &= expect_int("d2r.height", d2r ? d2r->wall_frame_height : -1, 71,
                     "ReDMCSB DUNVIEW.C:588 D2R height");
    ok &= expect_int("d2l.source_x", d2l ? d2l->wall_frame_source_x : -1, 61,
                     "ReDMCSB DUNVIEW.C:587 D2L source X");
    ok &= expect_int("d2r.source_x", d2r ? d2r->wall_frame_source_x : -1, 0,
                     "ReDMCSB DUNVIEW.C:588 D2R source X");
    ok &= expect_int("d2l.source_width",
                     d2l ? d2l->wall_bitmap_source_width : -1, 144,
                     "ReDMCSB DUNVIEW.C:2397-2403 D2LCR wall bitmap");
    ok &= expect_int("d2r.source_width",
                     d2r ? d2r->wall_bitmap_source_width : -1, 144,
                     "ReDMCSB DUNVIEW.C:2397-2403 D2LCR wall bitmap");
    ok &= expect_int("d2l.not_d3_y",
                     d2l ? d2l->wall_frame_y1 != 25 : 0, 1,
                     "ReDMCSB DUNVIEW.C:584-588 D2 row differs from D3");
    ok &= expect_int("d2r.not_d3_y",
                     d2r ? d2r->wall_frame_y1 != 25 : 0, 1,
                     "ReDMCSB DUNVIEW.C:584-588 D2 row differs from D3");
    ok &= expect_int("d2l.side_slot",
                     d2l ? d2l->side_ornament_square_aspect_slot : -1, 551,
                     A_F0119);
    ok &= expect_int("d2r.side_slot",
                     d2r ? d2r->side_ornament_square_aspect_slot : -1, 553,
                     A_F0120);
    ok &= expect_int("d2l.front_slot",
                     d2l ? d2l->front_ornament_square_aspect_slot : -1, 552,
                     A_F0119);
    ok &= expect_int("d2r.front_slot",
                     d2r ? d2r->front_ornament_square_aspect_slot : -1, 552,
                     A_F0120);
    ok &= expect_int("d2l.side_view", d2l ? d2l->side_wall_ornament_view : -1,
                     7, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d2r.side_view", d2r ? d2r->side_wall_ornament_view : -1,
                     8, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d2l.front_view", d2l ? d2l->front_wall_ornament_view : -1,
                     9, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d2r.front_view", d2r ? d2r->front_wall_ornament_view : -1,
                     11, A_DEFS_WALL_VIEWS);
    ok &= expect_int("d2l.floor_view", d2l ? d2l->floor_ornament_view : -1,
                     5, A_DEFS_FLOOR_VIEWS);
    ok &= expect_int("d2r.floor_view", d2r ? d2r->floor_ornament_view : -1,
                     7, A_DEFS_FLOOR_VIEWS);
    ok &= expect_int("d2l.f0104", d2l ? d2l->native_wall_blit_function : -1,
                     104, A_F0104);
    ok &= expect_int("d2r.f0104", d2r ? d2r->native_wall_blit_function : -1,
                     104, A_F0104);
    ok &= expect_int("d2l.f0105", d2l ? d2l->flipped_wall_blit_function : -1,
                     105, A_F0105);
    ok &= expect_int("d2r.f0105", d2r ? d2r->flipped_wall_blit_function : -1,
                     105, A_F0105);
    ok &= expect_int("d2l.rear_order",
                     d2l ? d2l->rear_d2_outer_order_index : -1, 0, A_F0128);
    ok &= expect_int("d2l.frame_order",
                     d2l ? d2l->transparent_frame_order_index : -1, 1, A_F0104);
    ok &= expect_int("d2l.wall_order",
                     d2l ? d2l->wall_blit_order_index : -1, 2, A_F0119);
    ok &= expect_int("d2l.side_f0107_order",
                     d2l ? d2l->side_f0107_order_index : -1, 3, A_F0107);
    ok &= expect_int("d2l.front_f0107_order",
                     d2l ? d2l->front_f0107_order_index : -1, 4, A_F0107);
    ok &= expect_int("d2r.rear_order",
                     d2r ? d2r->rear_d2_outer_order_index : -1, 0, A_F0128);
    ok &= expect_int("d2r.frame_order",
                     d2r ? d2r->transparent_frame_order_index : -1, 1, A_F0105);
    ok &= expect_int("d2r.wall_order",
                     d2r ? d2r->wall_blit_order_index : -1, 2, A_F0120);
    ok &= expect_int("d2r.side_f0107_order",
                     d2r ? d2r->side_f0107_order_index : -1, 5, A_F0107);
    ok &= expect_int("d2r.front_f0107_order",
                     d2r ? d2r->front_f0107_order_index : -1, 6, A_F0107);
    ok &= expect_int("d2l.c10", d2l ? d2l->c10_transparent_color : -1, 10,
                     A_DEFS_C10);
    ok &= expect_int("d2r.c10", d2r ? d2r->c10_transparent_color : -1, 10,
                     A_DEFS_C10);
    ok &= expect_int("d2l.scale", d2l ? d2l->f0107_depth2_scale : -1, 21,
                     A_F0107);
    ok &= expect_int("d2r.palette", d2r ? d2r->f0107_depth2_palette : -1, 2,
                     A_F0107);
    ok &= expect_int("d2l.lineage_open",
                     d2l ? d2l->lineage_open_room_shape : -1, 1192, A_LINEAGE);
    ok &= expect_int("d2r.lineage_door",
                     d2r ? d2r->lineage_door_front_overlay_shape : -1, 1903,
                     A_LINEAGE);

    return ok;
}

static int test_trace_and_transparency(void)
{
    int ok = 1;
    CSB_V1_D2LD2RWallTracePc34 trace;

    ok &= expect_int("trace.native.return",
                     csb_v1_viewport_d2l_d2r_wall_trace_pair_pc34(
                         0, 0, &trace),
                     0, A_F0128);
    ok &= expect_int("trace.native.ok", trace.ok, 1, A_F0128);
    ok &= expect_int("trace.native.contract", trace.source_locked_contract_only,
                     1, A_F0128);
    ok &= expect_int("trace.native.no_assets", trace.no_real_asset_bitmap_parity,
                     1, "contract-only source lock");
    ok &= expect_int("trace.native.no_data", trace.no_game_data_load, 1,
                     "asset-free source lock");
    ok &= expect_int("trace.native.order", trace.d2l_before_d2r, 1, A_F0128);
    ok &= expect_int("trace.native.before_d2c", trace.d2l_d2r_before_d2c, 1,
                     A_F0128);
    ok &= expect_int("trace.native.reject_c12_c13",
                     trace.rejected_c12_c13_as_d2, 1, A_DEFS_NOT_C12);
    ok &= expect_int("trace.native.rear_outer", trace.rear_d2_outer_calls, 2,
                     A_F0128);
    ok &= expect_int("trace.native.frames", trace.transparent_frame_calls, 2,
                     A_F0104);
    ok &= expect_int("trace.native.wall_blits", trace.wall_blit_calls, 2,
                     A_F0104);
    ok &= expect_int("trace.native.f0104", trace.f0104_calls, 2, A_F0104);
    ok &= expect_int("trace.native.f0105", trace.f0105_calls, 0, A_F0105);
    ok &= expect_int("trace.native.side_f0107", trace.f0107_side_calls, 2,
                     A_F0107);
    ok &= expect_int("trace.native.front_f0107", trace.f0107_front_calls, 2,
                     A_F0107);
    ok &= expect_int("trace.native.no_f0115", trace.f0115_calls, 0, A_F0119);
    ok &= expect_int("trace.native.first_zone", trace.first_wall_zone, 710,
                     A_DEFS_ZONES);
    ok &= expect_int("trace.native.second_zone", trace.second_wall_zone, 711,
                     A_DEFS_ZONES);
    ok &= expect_int("trace.native.first_wall", trace.first_wall_index, 8,
                     A_DEFS_WALLS);
    ok &= expect_int("trace.native.second_wall", trace.second_wall_index, 7,
                     A_DEFS_WALLS);
    ok &= expect_int("trace.native.first_func", trace.first_blit_function, 104,
                     A_F0104);
    ok &= expect_int("trace.native.second_func", trace.second_blit_function, 104,
                     A_F0104);
    ok &= expect_int("trace.native.return_without_alcove",
                     trace.wall_returns_without_front_alcove, 1, A_F0119);
    ok &= expect_int("trace.native.side_views", trace.side_ornament_view,
                     (7 << 8) | 8, A_DEFS_WALL_VIEWS);
    ok &= expect_int("trace.native.front_views", trace.front_ornament_view,
                     (9 << 8) | 11, A_DEFS_WALL_VIEWS);
    ok &= expect_int("trace.native.c10", trace.c10_transparency_preserved, 1,
                     A_DEFS_C10);
    ok &= expect_int("trace.native.d2_origin", trace.d2_row_origin_preserved, 1,
                     "ReDMCSB DUNVIEW.C:587-588 D2 row origin");

    ok &= expect_int("trace.flip.return",
                     csb_v1_viewport_d2l_d2r_wall_trace_pair_pc34(
                         1, 1, &trace),
                     0, A_F0105);
    ok &= expect_int("trace.flip.f0104", trace.f0104_calls, 0, A_F0104);
    ok &= expect_int("trace.flip.f0105", trace.f0105_calls, 2, A_F0105);
    ok &= expect_int("trace.flip.first_wall", trace.first_wall_index, 7,
                     A_F0105);
    ok &= expect_int("trace.flip.second_wall", trace.second_wall_index, 8,
                     A_F0105);
    ok &= expect_int("trace.flip.first_func", trace.first_blit_function, 105,
                     A_F0105);
    ok &= expect_int("trace.flip.second_func", trace.second_blit_function, 105,
                     A_F0105);
    ok &= expect_int("trace.flip.f0115", trace.f0115_calls, 2, A_F0119);
    ok &= expect_int("trace.flip.alcove_zero",
                     trace.front_alcove_uses_zero_order, 1, A_F0120);
    ok &= expect_int("blend.transparent",
                     csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x7a, 10),
                     0x7a, A_DEFS_C10);
    ok &= expect_int("blend.opaque",
                     csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x7a, 0x33),
                     0x33, A_DEFS_C10);

    return ok;
}

static int test_synthetic_blit(void)
{
    int ok = 1;
    const CSB_V1_D2LD2RWallSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const CSB_V1_D2LD2RWallSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34);
    uint8_t source[SOURCE_WIDTH * SOURCE_HEIGHT];
    uint8_t viewport[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
    CSB_V1_D2LD2RWallBlitStatsPc34 stats;

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 61)] = 0x21u;
    source[source_offset(0, 62)] = 0x22u;
    source[source_offset(70, 132)] = 0x23u;
    ok &= expect_int("blit.d2l.native",
                     csb_v1_viewport_d2l_d2r_wall_apply_c10_frame_clip_pc34(
                         d2l, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, A_F0104);
    ok &= expect_int("blit.d2l.transparent", stats.transparent_pixels,
                     (72 * 71) - 3, A_DEFS_C10);
    ok &= expect_int("blit.d2l.clipped", stats.clipped_pixels, 0, A_F0104);
    ok &= expect_int("blit.d2l.top_left", viewport[viewport_offset(20, 0)],
                     0x21, A_F0104);
    ok &= expect_int("blit.d2l.top_next", viewport[viewport_offset(20, 1)],
                     0x22, A_F0104);
    ok &= expect_int("blit.d2l.bottom_right", viewport[viewport_offset(90, 71)],
                     0x23, A_F0104);
    ok &= expect_int("blit.d2l.transparent_pixel",
                     viewport[viewport_offset(20, 2)], 0xee, A_DEFS_C10);
    ok &= expect_int("blit.d2l.d3_band_empty",
                     viewport[viewport_offset(25, 74)], 0xee,
                     "ReDMCSB DUNVIEW.C:584-588 D2 not D3 row width");
    ok &= expect_int("blit.d2l.after_band", viewport[viewport_offset(91, 0)],
                     0xee, "ReDMCSB DUNVIEW.C:587 D2L frame bottom");

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 0)] = 0x31u;
    source[source_offset(0, 1)] = 0x32u;
    source[source_offset(70, 71)] = 0x33u;
    ok &= expect_int("blit.d2r.flip",
                     csb_v1_viewport_d2l_d2r_wall_apply_c10_frame_clip_pc34(
                         d2r, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, A_F0105);
    ok &= expect_int("blit.d2r.transparent", stats.transparent_pixels,
                     (72 * 71) - 3, A_DEFS_C10);
    ok &= expect_int("blit.d2r.flipped_left",
                     viewport[viewport_offset(20, 220)], 0x31, A_F0105);
    ok &= expect_int("blit.d2r.flipped_next",
                     viewport[viewport_offset(20, 219)], 0x32, A_F0105);
    ok &= expect_int("blit.d2r.flipped_bottom",
                     viewport[viewport_offset(90, 149)], 0x33, A_F0105);
    ok &= expect_int("blit.d2r.left_neighbor",
                     viewport[viewport_offset(20, 148)], 0xee,
                     "ReDMCSB DUNVIEW.C:588 D2R frame left");
    ok &= expect_int("blit.reject",
                     csb_v1_viewport_d2l_d2r_wall_apply_c10_frame_clip_pc34(
                         d2l, source, 132, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     -1, A_F0104);
    ok &= expect_int("blit.reject.flag", stats.rejected, 1, A_F0104);

    return ok;
}

static int test_run_and_evidence(void)
{
    int ok = 1;
    CSB_V1_D2LD2RWallTracePc34 trace;
    const char *e = csb_v1_viewport_d2l_d2r_wall_source_evidence_pc34();

    ok &= expect_int("run.return",
                     csb_v1_viewport_d2l_d2r_wall_pc34_compat_run(&trace),
                     0, A_F0128);
    ok &= expect_int("run.ok", trace.ok, 1, A_F0128);
    ok &= expect_contains("evidence.F0119", e, "F0119", A_F0119);
    ok &= expect_contains("evidence.F0120", e, "F0120", A_F0120);
    ok &= expect_contains("evidence.F0104", e, "F0104", A_F0104);
    ok &= expect_contains("evidence.F0105", e, "F0105", A_F0105);
    ok &= expect_contains("evidence.F0107", e, "F0107", A_F0107);
    ok &= expect_contains("evidence.F0128", e, "F0128", A_F0128);
    ok &= expect_contains("evidence.C10", e, "C10_COLOR_FLESH", A_DEFS_C10);
    ok &= expect_contains("evidence.C710", e, "C710", A_DEFS_ZONES);
    ok &= expect_contains("evidence.C711", e, "C711", A_DEFS_ZONES);
    ok &= expect_contains("evidence.M580", e, "M580", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.M584", e, "M584", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.M604", e, "M604", A_DEFS_SQUARES);
    ok &= expect_contains("evidence.C12", e, "C12/C13 are D3L/D3R",
                          A_DEFS_NOT_C12);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1192-1209",
                          A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l_d2r_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l_d2r_wall_source_evidence_pc34());

    ok &= test_specs();
    ok &= test_frames_and_ornaments();
    ok &= test_trace_and_transparency();
    ok &= test_synthetic_blit();
    ok &= test_run_and_evidence();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    ok &= expect_int("assertion_count_at_least_80", g_assertions >= 80, 1,
                     "ReDMCSB DUNVIEW.C F0119/F0120/F0104/F0105/F0107/F0128");

    if (!ok || g_failures) {
        printf("FAIL csb_v1_viewport_d2l_d2r_wall_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS csb_v1_viewport_d2l_d2r_wall_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
