#include <dm1_v1_viewport_d2l_d2r_wall_pc34_compat.h>

#include <stdio.h>
#include <string.h>

static const char *A_F0119 =
    "ReDMCSB DUNVIEW.C F0119:6900-6973 D2L wall composition";
static const char *A_F0120 =
    "ReDMCSB DUNVIEW.C F0120:7051-7166 D2R wall composition";
static const char *A_F0104 =
    "ReDMCSB DUNVIEW.C F0104:3113-3156 native wall blit";
static const char *A_F0105 =
    "ReDMCSB DUNVIEW.C F0105:3185-3247 flipped wall blit";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107:3502-3938 side/front wall ornaments";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128:8503-8521 D2L/D2R before D2C";
static const char *A_DEFS_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency";
static const char *A_DEFS_SQUARES =
    "ReDMCSB DEFS.H:2582-2583 M604/M605 D2L/D2R view squares";
static const char *A_DEFS_WALL_VIEWS =
    "ReDMCSB DEFS.H:2703-2707 M580/M581/M582/M584 wall ornament views";
static const char *A_DEFS_WALLS =
    "ReDMCSB DEFS.H:3430-3431 C07/C08 wall indexes";
static const char *A_DEFS_ZONES =
    "ReDMCSB DEFS.H:4050-4051 C710/C711 wall zones";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915 FOV shape";

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

static int test_specs(void)
{
    int ok = 1;
    const DM1_V1_D2LD2RWallSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RWallSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);

    ok &= expect_int("spec.count",
                     (int)dm1_v1_viewport_d2l_d2r_wall_spec_count_pc34(), 2,
                     A_F0128);
    ok &= expect_int("spec.index0.d2l",
                     dm1_v1_viewport_d2l_d2r_wall_spec_at_pc34(0) == d2l, 1,
                     A_F0128);
    ok &= expect_int("spec.index1.d2r",
                     dm1_v1_viewport_d2l_d2r_wall_spec_at_pc34(1) == d2r, 1,
                     A_F0128);
    ok &= expect_int("spec.index2.null",
                     dm1_v1_viewport_d2l_d2r_wall_spec_at_pc34(2) == 0, 1,
                     A_F0128);
    ok &= expect_int("spec.unknown.null",
                     dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(99) == 0,
                     1, A_F0128);
    ok &= expect_int("d2l.function", d2l ? d2l->redmcsb_function_number : -1,
                     119, A_F0119);
    ok &= expect_int("d2r.function", d2r ? d2r->redmcsb_function_number : -1,
                     120, A_F0120);
    ok &= expect_int("d2l.view_square", d2l ? d2l->view_square : -1, 4,
                     A_DEFS_SQUARES);
    ok &= expect_int("d2r.view_square", d2r ? d2r->view_square : -1, 5,
                     A_DEFS_SQUARES);
    ok &= expect_int("d2l.depth", d2l ? d2l->relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2r.depth", d2r ? d2r->relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2l.lateral", d2l ? d2l->relative_lateral : 0, -1,
                     A_F0128);
    ok &= expect_int("d2r.lateral", d2r ? d2r->relative_lateral : 0, 1,
                     A_F0128);
    ok &= expect_int("d2l.order", d2l ? d2l->f0128_order_index : -1, 4,
                     A_F0128);
    ok &= expect_int("d2r.order", d2r ? d2r->f0128_order_index : -1, 5,
                     A_F0128);
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
    ok &= expect_int("d2l.frame_row", d2l ? d2l->wall_frame_row : -1, 4,
                     "ReDMCSB DUNVIEW.C:587 G0163 D2L row");
    ok &= expect_int("d2r.frame_row", d2r ? d2r->wall_frame_row : -1, 5,
                     "ReDMCSB DUNVIEW.C:588 G0163 D2R row");
    ok &= expect_int("d2l.frame_x1", d2l ? d2l->wall_frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:587 G0163 D2L x1");
    ok &= expect_int("d2l.frame_x2", d2l ? d2l->wall_frame_x2 : -1, 74,
                     "ReDMCSB DUNVIEW.C:587 G0163 D2L x2");
    ok &= expect_int("d2r.frame_x1", d2r ? d2r->wall_frame_x1 : -1, 149,
                     "ReDMCSB DUNVIEW.C:588 G0163 D2R x1");
    ok &= expect_int("d2r.frame_x2", d2r ? d2r->wall_frame_x2 : -1, 223,
                     "ReDMCSB DUNVIEW.C:588 G0163 D2R x2");
    ok &= expect_int("d2l.frame_y1", d2l ? d2l->wall_frame_y1 : -1, 20,
                     "ReDMCSB DUNVIEW.C:587 G0163 D2L y1");
    ok &= expect_int("d2r.frame_y2", d2r ? d2r->wall_frame_y2 : -1, 90,
                     "ReDMCSB DUNVIEW.C:588 G0163 D2R y2");
    ok &= expect_int("d2l.byte_width", d2l ? d2l->wall_frame_byte_width : -1,
                     72, "ReDMCSB DUNVIEW.C:587 G0163 byte width");
    ok &= expect_int("d2r.height", d2r ? d2r->wall_frame_height : -1, 71,
                     "ReDMCSB DUNVIEW.C:588 G0163 height");
    ok &= expect_int("d2l.source_x", d2l ? d2l->wall_frame_source_x : -1, 61,
                     "ReDMCSB DUNVIEW.C:587 G0163 source X");
    ok &= expect_int("d2r.source_x", d2r ? d2r->wall_frame_source_x : -1, 0,
                     "ReDMCSB DUNVIEW.C:588 G0163 source X");

    return ok;
}

static int test_ornament_and_order_metadata(void)
{
    int ok = 1;
    const DM1_V1_D2LD2RWallSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RWallSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);

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
    ok &= expect_int("d2l.floor_view", d2l ? d2l->floor_ornament_view : -1, 5,
                     "ReDMCSB DEFS.H:2755 M591_VIEW_FLOOR_D2L");
    ok &= expect_int("d2r.floor_view", d2r ? d2r->floor_ornament_view : -1, 7,
                     "ReDMCSB DEFS.H:2757 M593_VIEW_FLOOR_D2R");
    ok &= expect_int("d2l.f0104", d2l ? d2l->native_wall_blit_function : -1,
                     104, A_F0104);
    ok &= expect_int("d2r.f0105", d2r ? d2r->flipped_wall_blit_function : -1,
                     105, A_F0105);
    ok &= expect_int("d2l.rear_order", d2l ? d2l->rear_backdrop_order_index : -1,
                     0, A_F0119);
    ok &= expect_int("d2l.frame_top_order",
                     d2l ? d2l->frame_top_order_index : -1, 1, A_F0119);
    ok &= expect_int("d2l.frame_side_order",
                     d2l ? d2l->frame_side_order_index : -1, 2, A_F0119);
    ok &= expect_int("d2l.wall_order",
                     d2l ? d2l->wall_bitmap_order_index : -1, 3, A_F0119);
    ok &= expect_int("d2l.side_orn_order",
                     d2l ? d2l->side_ornament_order_index : -1, 4, A_F0107);
    ok &= expect_int("d2l.front_orn_order",
                     d2l ? d2l->front_ornament_order_index : -1, 5, A_F0107);
    ok &= expect_int("d2l.first_backdrop_order",
                     d2l ? d2l->front_first_backdrop_order_index : -1, 6,
                     A_F0119);
    ok &= expect_int("d2r.rear_order", d2r ? d2r->rear_backdrop_order_index : -1,
                     7, A_F0120);
    ok &= expect_int("d2r.wall_order", d2r ? d2r->wall_bitmap_order_index : -1,
                     10, A_F0120);
    ok &= expect_int("d2r.frame_side_flipped",
                     d2r ? d2r->c10_frame_side_is_flipped : -1, 1, A_F0105);
    ok &= expect_int("d2l.lineage_open",
                     d2l ? d2l->lineage_open_room_shape : -1, 1192,
                     A_LINEAGE);
    ok &= expect_int("d2r.lineage_door",
                     d2r ? d2r->lineage_door_front_overlay_shape : -1, 1903,
                     A_LINEAGE);
    ok &= expect_int("d2l.c10", d2l ? d2l->c10_transparent_color : -1, 10,
                     A_DEFS_C10);
    ok &= expect_int("d2r.c10", d2r ? d2r->c10_transparent_color : -1, 10,
                     A_DEFS_C10);

    return ok;
}

static int test_native_compose_trace(void)
{
    int ok = 1;
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallComposeStatePc34 state = { 0, 0 };
    DM1_V1_D2LD2RWallTracePc34 trace;

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("compose.native.return",
                     dm1_v1_viewport_d2l_d2r_wall_compose(
                         &state, viewport,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34, &trace),
                     0, A_F0128);
    ok &= expect_int("compose.native.ok", trace.ok, 1, A_F0128);
    ok &= expect_int("compose.native.contract",
                     trace.source_locked_contract_only, 1, A_F0128);
    ok &= expect_int("compose.native.no_assets",
                     trace.no_real_asset_bitmap_parity, 1,
                     "contract-only no real-asset parity");
    ok &= expect_int("compose.native.no_data", trace.no_game_data_load, 1,
                     "contract-only no game data load");
    ok &= expect_int("compose.native.blit_count", trace.blit_count, 12,
                     A_F0119);
    ok &= expect_int("compose.native.d2l_blits", trace.d2l_blit_count, 6,
                     A_F0119);
    ok &= expect_int("compose.native.d2r_blits", trace.d2r_blit_count, 6,
                     A_F0120);
    ok &= expect_int("compose.native.d2l_before_d2r", trace.d2l_before_d2r, 1,
                     A_F0128);
    ok &= expect_int("compose.native.d2l_before_d2c", trace.d2l_before_d2c, 1,
                     A_F0128);
    ok &= expect_int("compose.native.d2r_before_d2c", trace.d2r_before_d2c, 1,
                     A_F0128);
    ok &= expect_int("compose.native.d2c_order", trace.d2c_order_index, 6,
                     A_F0128);
    ok &= expect_int("compose.native.all_c10", trace.all_blits_use_c10, 1,
                     A_DEFS_C10);
    ok &= expect_int("compose.native.preserve_c10",
                     trace.all_blits_preserve_c10, 1, A_DEFS_C10);
    ok &= expect_int("compose.native.f0104", trace.f0104_calls, 6, A_F0104);
    ok &= expect_int("compose.native.f0105", trace.f0105_calls, 2, A_F0105);
    ok &= expect_int("compose.native.f0107_side", trace.f0107_side_calls, 2,
                     A_F0107);
    ok &= expect_int("compose.native.f0107_front", trace.f0107_front_calls, 2,
                     A_F0107);
    ok &= expect_int("compose.native.no_f0115",
                     trace.f0115_first_backdrop_calls, 0, A_F0119);
    ok &= expect_int("compose.native.first_zone", trace.first_wall_zone, 710,
                     A_DEFS_ZONES);
    ok &= expect_int("compose.native.second_zone", trace.second_wall_zone, 711,
                     A_DEFS_ZONES);
    ok &= expect_int("compose.native.first_wall", trace.first_wall_index, 8,
                     A_DEFS_WALLS);
    ok &= expect_int("compose.native.second_wall", trace.second_wall_index, 7,
                     A_DEFS_WALLS);
    ok &= expect_int("compose.native.d2l_square", trace.d2l_view_square, 4,
                     A_DEFS_SQUARES);
    ok &= expect_int("compose.native.d2r_square", trace.d2r_view_square, 5,
                     A_DEFS_SQUARES);
    ok &= expect_int("compose.native.d2l_side_view",
                     trace.d2l_side_ornament_view, 7, A_DEFS_WALL_VIEWS);
    ok &= expect_int("compose.native.d2r_side_view",
                     trace.d2r_side_ornament_view, 8, A_DEFS_WALL_VIEWS);
    ok &= expect_int("compose.native.d2l_front_view",
                     trace.d2l_front_ornament_view, 9, A_DEFS_WALL_VIEWS);
    ok &= expect_int("compose.native.d2r_front_view",
                     trace.d2r_front_ornament_view, 11, A_DEFS_WALL_VIEWS);

    ok &= expect_int("blit0.side", trace.blits[0].side,
                     DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34, A_F0119);
    ok &= expect_int("blit0.kind", trace.blits[0].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_REAR_BACKDROP_PC34, A_F0119);
    ok &= expect_int("blit1.kind", trace.blits[1].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_TOP_PC34, A_F0119);
    ok &= expect_int("blit2.kind", trace.blits[2].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_SIDE_PC34, A_F0119);
    ok &= expect_int("blit3.kind", trace.blits[3].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_WALL_BITMAP_PC34, A_F0119);
    ok &= expect_int("blit4.kind", trace.blits[4].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_SIDE_ORNAMENT_PC34, A_F0107);
    ok &= expect_int("blit5.kind", trace.blits[5].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_FRONT_ORNAMENT_PC34, A_F0107);
    ok &= expect_int("blit6.side", trace.blits[6].side,
                     DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34, A_F0120);
    ok &= expect_int("blit8.right_frame_flipped", trace.blits[8].flipped, 1,
                     A_F0105);
    ok &= expect_int("blit3.wall_index", trace.blits[3].wall_index, 8,
                     A_DEFS_WALLS);
    ok &= expect_int("blit9.wall_index", trace.blits[9].wall_index, 7,
                     A_DEFS_WALLS);
    ok &= expect_int("blit4.side_view", trace.blits[4].view_wall_index, 7,
                     A_DEFS_WALL_VIEWS);
    ok &= expect_int("blit10.side_view", trace.blits[10].view_wall_index, 8,
                     A_DEFS_WALL_VIEWS);
    ok &= expect_int("blit5.front_view", trace.blits[5].view_wall_index, 9,
                     A_DEFS_WALL_VIEWS);
    ok &= expect_int("blit11.front_view", trace.blits[11].view_wall_index, 11,
                     A_DEFS_WALL_VIEWS);
    ok &= expect_int("blit0.c10", trace.blits[0].transparent_color, 10,
                     A_DEFS_C10);
    ok &= expect_int("blit0.transparent_sample",
                     trace.blits[0].source_transparent_sample, 10, A_DEFS_C10);
    ok &= expect_int("blit0.transparent_preserved",
                     trace.blits[0].destination_after_transparent,
                     trace.blits[0].destination_before_transparent, A_DEFS_C10);
    ok &= expect_int("blit0.opaque_written",
                     trace.blits[0].destination_after_opaque, 0x21,
                     "synthetic framebuffer write");
    ok &= expect_int("blit6.opaque_written",
                     trace.blits[6].destination_after_opaque, 0x51,
                     "synthetic framebuffer write");

    return ok;
}

static int test_flipped_alcove_compose_trace(void)
{
    int ok = 1;
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallComposeStatePc34 state = { 1, 1 };
    DM1_V1_D2LD2RWallTracePc34 trace;

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("compose.flip.return",
                     dm1_v1_viewport_d2l_d2r_wall_compose(
                         &state, viewport,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34, &trace),
                     0, A_F0105);
    ok &= expect_int("compose.flip.blit_count", trace.blit_count, 14, A_F0119);
    ok &= expect_int("compose.flip.d2l_blits", trace.d2l_blit_count, 7, A_F0119);
    ok &= expect_int("compose.flip.d2r_blits", trace.d2r_blit_count, 7, A_F0120);
    ok &= expect_int("compose.flip.f0104", trace.f0104_calls, 4, A_F0104);
    ok &= expect_int("compose.flip.f0105", trace.f0105_calls, 4, A_F0105);
    ok &= expect_int("compose.flip.f0115", trace.f0115_first_backdrop_calls, 2,
                     A_F0119);
    ok &= expect_int("compose.flip.first_wall", trace.first_wall_index, 7,
                     A_F0105);
    ok &= expect_int("compose.flip.second_wall", trace.second_wall_index, 8,
                     A_F0105);
    ok &= expect_int("compose.flip.all_c10", trace.all_blits_use_c10, 1,
                     A_DEFS_C10);
    ok &= expect_int("compose.flip.preserve_c10", trace.all_blits_preserve_c10,
                     1, A_DEFS_C10);
    ok &= expect_int("compose.flip.d2l_first_backdrop_kind", trace.blits[6].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_FRONT_FIRST_BACKDROP_PC34,
                     A_F0119);
    ok &= expect_int("compose.flip.d2r_first_backdrop_kind", trace.blits[13].kind,
                     DM1_V1_D2L_D2R_WALL_BLIT_FRONT_FIRST_BACKDROP_PC34,
                     A_F0120);
    ok &= expect_int("compose.flip.d2l_wall_flipped", trace.blits[3].flipped, 1,
                     A_F0105);
    ok &= expect_int("compose.flip.d2r_wall_flipped", trace.blits[10].flipped, 1,
                     A_F0105);
    ok &= expect_int("compose.flip.d2l_first_backdrop_func",
                     trace.blits[6].blit_function, 115, A_F0119);
    ok &= expect_int("compose.flip.d2r_first_backdrop_func",
                     trace.blits[13].blit_function, 115, A_F0120);
    ok &= expect_int("compose.flip.d2l_first_backdrop_c10",
                     trace.blits[6].transparent_color, 10, A_DEFS_C10);
    ok &= expect_int("compose.flip.d2r_first_backdrop_c10",
                     trace.blits[13].transparent_color, 10, A_DEFS_C10);
    ok &= expect_int("compose.flip.d2c_still_after", trace.d2r_before_d2c, 1,
                     A_F0128);

    return ok;
}

static int test_frame_edge_pixel_gate(void)
{
    int ok = 1;
    uint8_t source[DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 *
                   DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallFramePixelPc34 pixel;
    const DM1_V1_D2LD2RWallSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RWallSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 + 61] = 0x31;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 + 135] = 0x7b;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 + 0] = 0x42;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 + 74] = 10;

    ok &= expect_int("pixel.source_width", DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34,
                     72, "ReDMCSB G0163 ByteWidth=72");
    ok &= expect_int("pixel.source_pixel_width",
                     DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34,
                     144, "ReDMCSB G0163 ByteWidth=72 packed PC34 pixels");
    ok &= expect_int("pixel.d2l.left.apply",
                     dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
                         d2l, 20, 0, source, sizeof(source), viewport,
                         sizeof(viewport), &pixel),
                     1, "ReDMCSB DUNVIEW.C:587 G0163 D2L left edge");
    ok &= expect_int("pixel.d2l.left.source_x", pixel.source_x, 61,
                     "ReDMCSB DUNVIEW.C:587 G0163 D2L source X");
    ok &= expect_int("pixel.d2l.left.value", pixel.pixel_after, 0x31,
                     "F0100/F0104 C10 transparent blit writes opaque pixel");
    ok &= expect_int("pixel.d2l.right.apply",
                     dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
                         d2l, 20, 74, source, sizeof(source), viewport,
                         sizeof(viewport), &pixel),
                     1, "ReDMCSB DUNVIEW.C:587 G0163 D2L right edge");
    ok &= expect_int("pixel.d2l.right.source_x", pixel.source_x, 135,
                     "D2L source X 61 + local X 74");
    ok &= expect_int("pixel.d2l.right.value", pixel.pixel_after, 0x7b,
                     "right-edge pixel requires 144-pixel source stride");
    ok &= expect_int("pixel.d2l.after_edge.apply",
                     dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
                         d2l, 20, 75, source, sizeof(source), viewport,
                         sizeof(viewport), &pixel),
                     1, "D2L no-write after frame X2");
    ok &= expect_int("pixel.d2l.after_edge.no_write", pixel.no_write_metadata, 1,
                     "D2L frame clips at viewport X2=74");

    ok &= expect_int("pixel.d2r.left.apply",
                     dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
                         d2r, 20, 149, source, sizeof(source), viewport,
                         sizeof(viewport), &pixel),
                     1, "ReDMCSB DUNVIEW.C:588 G0163 D2R left edge");
    ok &= expect_int("pixel.d2r.left.source_x", pixel.source_x, 0,
                     "ReDMCSB DUNVIEW.C:588 G0163 D2R source X");
    ok &= expect_int("pixel.d2r.left.value", pixel.pixel_after, 0x42,
                     "F0100/F0104 C10 transparent blit writes opaque pixel");
    ok &= expect_int("pixel.d2r.right.apply",
                     dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
                         d2r, 20, 223, source, sizeof(source), viewport,
                         sizeof(viewport), &pixel),
                     1, "ReDMCSB DUNVIEW.C:588 G0163 D2R right edge");
    ok &= expect_int("pixel.d2r.right.source_x", pixel.source_x, 74,
                     "D2R source X 0 + local X 74");
    ok &= expect_int("pixel.d2r.right.skip", pixel.transparent_skip, 1,
                     "DEFS.H:2088 C10_COLOR_FLESH transparent pixel");
    ok &= expect_int("pixel.d2r.right.preserved", pixel.pixel_after, 0xee,
                     "F0100/F0104 C10 transparent blit preserves destination");

    return ok;
}

static int test_blend_invalid_and_evidence(void)
{
    int ok = 1;
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallTracePc34 trace;
    const char *e = dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34();

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("blend.transparent",
                     dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x7a, 10),
                     0x7a, A_DEFS_C10);
    ok &= expect_int("blend.opaque",
                     dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x7a, 0x33),
                     0x33, A_DEFS_C10);
    ok &= expect_int("compose.invalid.null_viewport",
                     dm1_v1_viewport_d2l_d2r_wall_compose(
                         0, 0, DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34, &trace),
                     -1, "argument guard");
    ok &= expect_int("compose.invalid.small_width",
                     dm1_v1_viewport_d2l_d2r_wall_compose(
                         0, viewport,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 - 1,
                         DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34, &trace),
                     -1, "argument guard");
    ok &= expect_int("run.return",
                     dm1_v1_viewport_d2l_d2r_wall_pc34_compat_run(&trace),
                     0, A_F0128);
    ok &= expect_int("run.ok", trace.ok, 1, A_F0128);
    ok &= expect_contains("evidence.F0119", e, "F0119", A_F0119);
    ok &= expect_contains("evidence.F0120", e, "F0120", A_F0120);
    ok &= expect_contains("evidence.F0104", e, "F0104", A_F0104);
    ok &= expect_contains("evidence.F0105", e, "F0105", A_F0105);
    ok &= expect_contains("evidence.F0107", e, "F0107", A_F0107);
    ok &= expect_contains("evidence.F0128", e, "F0128", A_F0128);
    ok &= expect_contains("evidence.C10", e, "C10_COLOR_FLESH", A_DEFS_C10);
    ok &= expect_contains("evidence.M604", e, "M604", A_DEFS_SQUARES);
    ok &= expect_contains("evidence.M605", e, "M605", A_DEFS_SQUARES);
    ok &= expect_contains("evidence.M580", e, "M580", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.M584", e, "M584", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.not_d3", e, "not the D3", A_DEFS_WALL_VIEWS);
    ok &= expect_contains("evidence.C07", e, "C07", A_DEFS_WALLS);
    ok &= expect_contains("evidence.C08", e, "C08", A_DEFS_WALLS);
    ok &= expect_contains("evidence.C710", e, "C710", A_DEFS_ZONES);
    ok &= expect_contains("evidence.C711", e, "C711", A_DEFS_ZONES);
    ok &= expect_contains("evidence.Viewport", e, "Viewport.cpp:1192-1209",
                          A_LINEAGE);
    ok &= expect_contains("evidence.no_assets", e, "no real-asset bitmap parity",
                          "contract-only source lock");
    ok &= expect_contains("evidence.bytewidth", e, "ByteWidth=72",
                          "ReDMCSB G0163 packed source stride");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_viewport_d2l_d2r_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34());

    ok &= test_specs();
    ok &= test_ornament_and_order_metadata();
    ok &= test_native_compose_trace();
    ok &= test_flipped_alcove_compose_trace();
    ok &= test_frame_edge_pixel_gate();
    ok &= test_blend_invalid_and_evidence();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    ok &= expect_int("assertion_count_at_least_80", g_assertions >= 80, 1,
                     "pass687 DM1 D2L/D2R wall composition source lock");

    if (!ok || g_failures) {
        printf("FAIL dm1_v1_viewport_d2l_d2r_wall_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l_d2r_wall_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
