#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;
extern const uint8_t *g_dm1_wall_frame_bitmaps;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_nonnull(const char *id, const void *ptr)
{
    if (!ptr) {
        printf("FAIL %s is NULL\n", id);
        ++g_failures;
    } else {
        printf("PASS %s non-null\n", id);
    }
}

static void test_redmcsb_g0163_wall_frames(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        unsigned char v[8];
        const char *id;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3C, {  74,149,25, 75, 64, 51, 18,0 }, "D3C" },
        { DM1_VIEW_SQUARE_D3L, {   0, 83,25, 75, 64, 51, 32,0 }, "D3L" },
        { DM1_VIEW_SQUARE_D3R, { 139,223,25, 75, 64, 51,  0,0 }, "D3R" },
        { DM1_VIEW_SQUARE_D2C, {  60,163,20, 90, 72, 71, 16,0 }, "D2C" },
        { DM1_VIEW_SQUARE_D2L, {   0, 74,20, 90, 72, 71, 61,0 }, "D2L" },
        { DM1_VIEW_SQUARE_D2R, { 149,223,20, 90, 72, 71,  0,0 }, "D2R" },
        { DM1_VIEW_SQUARE_D1C, {  32,191, 9,119,128,111, 48,0 }, "D1C" },
        { DM1_VIEW_SQUARE_D1L, {   0, 63, 9,119,128,111,192,0 }, "D1L" },
        { DM1_VIEW_SQUARE_D1R, { 160,223, 9,119,128,111,  0,0 }, "D1R" },
        { DM1_VIEW_SQUARE_D0C, {   0,223, 0,135,  0,  0,  0,0 }, "D0C" },
        { DM1_VIEW_SQUARE_D0L, {   0, 31, 0,135, 16,136,  0,0 }, "D0L" },
        { DM1_VIEW_SQUARE_D0R, { 192,223, 0,135, 16,136,  0,0 }, "D0R" },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_WallFrame *f = dm1_viewport_3d_get_wall_frame(expected[i].square);
        char id[64];
        snprintf(id, sizeof(id), "G0163.%s.nonnull", expected[i].id);
        check_nonnull(id, f);
        if (!f) continue;
        snprintf(id, sizeof(id), "G0163.%s.left", expected[i].id);
        check_int(id, f->left_x, expected[i].v[0]);
        snprintf(id, sizeof(id), "G0163.%s.right", expected[i].id);
        check_int(id, f->right_x, expected[i].v[1]);
        snprintf(id, sizeof(id), "G0163.%s.top", expected[i].id);
        check_int(id, f->top_y, expected[i].v[2]);
        snprintf(id, sizeof(id), "G0163.%s.bottom", expected[i].id);
        check_int(id, f->bottom_y, expected[i].v[3]);
        snprintf(id, sizeof(id), "G0163.%s.byte_width", expected[i].id);
        check_int(id, f->byte_width, expected[i].v[4]);
        snprintf(id, sizeof(id), "G0163.%s.height", expected[i].id);
        check_int(id, f->height, expected[i].v[5]);
        snprintf(id, sizeof(id), "G0163.%s.blit_x", expected[i].id);
        check_int(id, f->blit_x, expected[i].v[6]);
        snprintf(id, sizeof(id), "G0163.%s.blit_y", expected[i].id);
        check_int(id, f->blit_y, expected[i].v[7]);
    }
}


static void test_redmcsb_g0163_wall_frames_resolve_clip_gate(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int visible;
        int src_x;
        int src_y;
        int dst_x;
        int dst_y;
        int width;
        int height;
        const char *id;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3C, 1, 18, 0,  74,25,  46, 51, "D3C" },
        { DM1_VIEW_SQUARE_D3L, 1, 32, 0,   0,25,  32, 51, "D3L" },
        { DM1_VIEW_SQUARE_D3R, 1,  0, 0, 139,25,  64, 51, "D3R" },
        { DM1_VIEW_SQUARE_D2C, 1, 16, 0,  60,20,  56, 71, "D2C" },
        { DM1_VIEW_SQUARE_D2L, 1, 61, 0,   0,20,  11, 71, "D2L" },
        { DM1_VIEW_SQUARE_D2R, 1,  0, 0, 149,20,  72, 71, "D2R" },
        { DM1_VIEW_SQUARE_D1C, 1, 48, 0,  32, 9,  80,111, "D1C" },
        { DM1_VIEW_SQUARE_D1L, 0,  0, 0,   0, 0,   0,  0, "D1L" },
        { DM1_VIEW_SQUARE_D1R, 1,  0, 0, 160, 9,  64,111, "D1R" },
        { DM1_VIEW_SQUARE_D0C, 0,  0, 0,   0, 0,   0,  0, "D0C" },
        { DM1_VIEW_SQUARE_D0L, 1,  0, 0,   0, 0,  16,136, "D0L" },
        { DM1_VIEW_SQUARE_D0R, 1,  0, 0, 192, 0,  16,136, "D0R" },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(expected[i].square);
        char id[96];
        snprintf(id, sizeof(id), "G0163.clip.%s.frame", expected[i].id);
        check_nonnull(id, frame);
        if (!frame) continue;
        DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
        snprintf(id, sizeof(id), "G0163.clip.%s.visible", expected[i].id);
        check_int(id, gate.visible ? 1 : 0, expected[i].visible);
        snprintf(id, sizeof(id), "G0163.clip.%s.source_lock", expected[i].id);
        check_int(id, gate.source_lines && strstr(gate.source_lines, "DUNVIEW.C:3053-3058") && strstr(gate.source_lines, "COORD.C:2390-2409") && strstr(gate.source_lines, "IMAGE3.C:866-889"), 1);
        if (!expected[i].visible) continue;
        snprintf(id, sizeof(id), "G0163.clip.%s.src_x", expected[i].id);
        check_int(id, gate.src_x, expected[i].src_x);
        snprintf(id, sizeof(id), "G0163.clip.%s.src_y", expected[i].id);
        check_int(id, gate.src_y, expected[i].src_y);
        snprintf(id, sizeof(id), "G0163.clip.%s.dst_x", expected[i].id);
        check_int(id, gate.dst_x, expected[i].dst_x);
        snprintf(id, sizeof(id), "G0163.clip.%s.dst_y", expected[i].id);
        check_int(id, gate.dst_y, expected[i].dst_y);
        snprintf(id, sizeof(id), "G0163.clip.%s.width", expected[i].id);
        check_int(id, gate.width, expected[i].width);
        snprintf(id, sizeof(id), "G0163.clip.%s.height", expected[i].id);
        check_int(id, gate.height, expected[i].height);
    }
}

static void test_redmcsb_f0128_draw_order(void)
{
    static const DM1_ViewSquareIndex expected_squares[] = {
        DM1_VIEW_SQUARE_D4L, DM1_VIEW_SQUARE_D4R, DM1_VIEW_SQUARE_D4C,
        DM1_VIEW_SQUARE_D3L2, DM1_VIEW_SQUARE_D3R2,
        DM1_VIEW_SQUARE_D3L, DM1_VIEW_SQUARE_D3R, DM1_VIEW_SQUARE_D3C,
        DM1_VIEW_SQUARE_D2L2, DM1_VIEW_SQUARE_D2R2,
        DM1_VIEW_SQUARE_D2L, DM1_VIEW_SQUARE_D2R, DM1_VIEW_SQUARE_D2C,
        DM1_VIEW_SQUARE_D1L, DM1_VIEW_SQUARE_D1R, DM1_VIEW_SQUARE_D1C,
        DM1_VIEW_SQUARE_D0L, DM1_VIEW_SQUARE_D0R, DM1_VIEW_SQUARE_D0C,
    };
    static const signed char expected_depth[] = {
        4,4,4, 3,3, 3,3,3, 2,2, 2,2,2, 1,1,1, 0,0,0
    };
    static const signed char expected_lateral[] = {
        -1,1,0, -2,2, -1,1,0, -2,2, -1,1,0, -1,1,0, -1,1,0
    };

    size_t n = dm1_viewport_3d_draw_order_count();
    check_int("F0128.draw_order.count", (int)n, 19);
    for (size_t i = 0; i < n && i < 19; ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(i);
        char id[80];
        snprintf(id, sizeof(id), "F0128.draw_order.%02zu.nonnull", i);
        check_nonnull(id, step);
        if (!step) continue;
        snprintf(id, sizeof(id), "F0128.draw_order.%02zu.square", i);
        check_int(id, (int)step->square, (int)expected_squares[i]);
        snprintf(id, sizeof(id), "F0128.draw_order.%02zu.depth", i);
        check_int(id, step->rel_depth, expected_depth[i]);
        snprintf(id, sizeof(id), "F0128.draw_order.%02zu.lateral", i);
        check_int(id, step->rel_lateral, expected_lateral[i]);
        snprintf(id, sizeof(id), "F0128.draw_order.%02zu.source", i);
        check_nonnull(id, step->source_lines);
    }
    check_int("F0128.draw_order.out_of_range", dm1_viewport_3d_get_draw_order_step(19) == NULL, 1);
}

static void test_f0128_d4_far_object_pass_order(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int rel_lateral;
        int redmcsb_id;
        const char *source_line;
        const char *defs_line;
    } expected[] = {
        { DM1_VIEW_SQUARE_D4L, -1, 17, "8466-8469", "2613" },
        { DM1_VIEW_SQUARE_D4R,  1, 18, "8470-8473", "2614" },
        { DM1_VIEW_SQUARE_D4C,  0, 16, "8474-8477", "2612" },
    };

    check_int("F0128.d4_far_object.count", (int)dm1_viewport_3d_far_object_pass_spec_count(), 3);
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportFarObjectPassSpec *spec =
            dm1_viewport_3d_get_far_object_pass_spec_for_square(expected[i].square);
        DM1_ViewportCellOrder order;
        char id[96];
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.depth", i);
        check_int(id, spec->rel_depth, 4);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.lateral", i);
        check_int(id, spec->rel_lateral, expected[i].rel_lateral);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.redmcsb_id", i);
        check_int(id, spec->redmcsb_view_square_id, expected[i].redmcsb_id);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.first_object", i);
        check_int(id, spec->uses_square_first_object ? 1 : 0, 1);
        order = dm1_viewport_3d_decode_cell_order(spec->cell_order);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.cell_order", i);
        check_int(id, spec->cell_order, 0x0001);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.cell_count", i);
        check_int(id, order.cell_count, 1);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.cell0", i);
        check_int(id, order.cells[0], 1);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.source_line", i);
        check_int(id, strstr(spec->source_lines, expected[i].source_line) != NULL, 1);
        snprintf(id, sizeof(id), "F0128.d4_far_object.%zu.defs_line", i);
        check_int(id, strstr(spec->source_lines, expected[i].defs_line) != NULL, 1);
    }
    check_int("F0128.d4_far_object.before_d3_wall",
              dm1_viewport_3d_get_draw_order_step(3)->square == DM1_VIEW_SQUARE_D3L2, 1);
    check_int("F0128.d4_far_object.out_of_range",
              dm1_viewport_3d_get_far_object_pass_spec(3) == NULL, 1);
}

static void test_f0128_draw_order_resolves_relative_map_coordinates(void)
{
    static const struct {
        int x;
        int y;
    } expected_north[] = {
        {  9, 16 }, { 11, 16 }, { 10, 16 },
        {  8, 17 }, { 12, 17 },
        {  9, 17 }, { 11, 17 }, { 10, 17 },
        {  8, 18 }, { 12, 18 },
        {  9, 18 }, { 11, 18 }, { 10, 18 },
        {  9, 19 }, { 11, 19 }, { 10, 19 },
        {  9, 20 }, { 11, 20 }, { 10, 20 },
    };
    DM1_ViewportResolvedDrawStep resolved;
    int16_t x = 0;
    int16_t y = 0;

    check_int("F0150.viewport_relative.null_x", dm1_viewport_3d_resolve_relative_map_xy(0, 1, 0, 10, 20, NULL, &y), 0);
    check_int("F0150.viewport_relative.null_y", dm1_viewport_3d_resolve_relative_map_xy(0, 1, 0, 10, 20, &x, NULL), 0);

    for (size_t i = 0; i < dm1_viewport_3d_draw_order_count(); ++i) {
        char id[96];
        check_int("F0150.viewport_relative.resolve", dm1_viewport_3d_resolve_draw_order_step(i, 0, 10, 20, &resolved), 1);
        snprintf(id, sizeof(id), "F0150.viewport_relative.%02zu.x", i);
        check_int(id, resolved.map_x, expected_north[i].x);
        snprintf(id, sizeof(id), "F0150.viewport_relative.%02zu.y", i);
        check_int(id, resolved.map_y, expected_north[i].y);
        snprintf(id, sizeof(id), "F0150.viewport_relative.%02zu.source", i);
        check_int(id, strstr(resolved.source_lines, "DUNGEON.C:1371-1421") != NULL, 1);
    }

    check_int("F0150.viewport_relative.east", dm1_viewport_3d_resolve_relative_map_xy(1, 3, -1, 10, 20, &x, &y), 1);
    check_int("F0150.viewport_relative.east.x", x, 13);
    check_int("F0150.viewport_relative.east.y", y, 19);

    check_int("F0150.viewport_relative.south", dm1_viewport_3d_resolve_relative_map_xy(2, 3, -1, 10, 20, &x, &y), 1);
    check_int("F0150.viewport_relative.south.x", x, 11);
    check_int("F0150.viewport_relative.south.y", y, 23);

    check_int("F0150.viewport_relative.west", dm1_viewport_3d_resolve_relative_map_xy(3, 3, -1, 10, 20, &x, &y), 1);
    check_int("F0150.viewport_relative.west.x", x, 7);
    check_int("F0150.viewport_relative.west.y", y, 21);

    check_int("F0150.viewport_relative.normalize", dm1_viewport_3d_resolve_relative_map_xy(5, 3, -1, 10, 20, &x, &y), 1);
    check_int("F0150.viewport_relative.normalize.x", x, 13);
    check_int("F0150.viewport_relative.normalize.y", y, 19);

    check_int("F0150.viewport_relative.out_of_range", dm1_viewport_3d_resolve_draw_order_step(19, 0, 10, 20, &resolved), 0);
    check_int("F0150.viewport_relative.null_result", dm1_viewport_3d_resolve_draw_order_step(0, 0, 10, 20, NULL), 0);
}



static void test_pc34_wall_bitmap_selection(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        DM1_WallSetIndex native_wall;
        DM1_WallSetIndex parity_wall;
        int center;
        int zone;
        int wall_return;
        int front_alcove;
        const char *occlusion_needle;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, 0, DM1_PC34_ZONE_WALL_D3L2, 1, 0, "6264" },
        { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, 0, DM1_PC34_ZONE_WALL_D3R2, 1, 0, "6331" },
        { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  0, DM1_PC34_ZONE_WALL_D3L,  1, 1, "6437" },
        { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  0, DM1_PC34_ZONE_WALL_D3R,  1, 1, "6573" },
        { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  1, DM1_PC34_ZONE_WALL_D3C,  1, 1, "6720" },
        { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, 0, DM1_PC34_ZONE_WALL_D2L2, 1, 0, "6862" },
        { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, 0, DM1_PC34_ZONE_WALL_D2R2, 1, 0, "6893" },
        { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  0, DM1_PC34_ZONE_WALL_D2L,  1, 1, "6973" },
        { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  0, DM1_PC34_ZONE_WALL_D2R,  1, 1, "7166" },
        { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  1, DM1_PC34_ZONE_WALL_D2C,  1, 1, "7312" },
        { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  0, DM1_PC34_ZONE_WALL_D1L,  1, 0, "7460" },
        { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  0, DM1_PC34_ZONE_WALL_D1R,  1, 0, "7628" },
        { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  1, DM1_PC34_ZONE_WALL_D1C,  0, 1, "7843" },
        { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  0, DM1_PC34_ZONE_WALL_D0L,  1, 0, "8038" },
        { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  0, DM1_PC34_ZONE_WALL_D0R,  1, 0, "8144" },
    };

    check_int("PC34.wall_draw_spec.count", (int)dm1_viewport_3d_wall_draw_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportWallDrawSpec *spec = dm1_viewport_3d_get_wall_draw_spec_for_square(expected[i].square);
        char id[96];
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.native", i);
        check_int(id, (int)spec->native_wall, (int)expected[i].native_wall);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.parity", i);
        check_int(id, (int)spec->parity_wall, (int)expected[i].parity_wall);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.center", i);
        check_int(id, spec->center_wall ? 1 : 0, expected[i].center);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.zone", i);
        check_int(id, spec->pc34_zone, expected[i].zone);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.return", i);
        check_int(id, spec->wall_case_returns ? 1 : 0, expected[i].wall_return);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.front_alcove", i);
        check_int(id, spec->front_alcove_reveals_contents ? 1 : 0, expected[i].front_alcove);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.occlusion_source", i);
        check_nonnull(id, spec->occlusion_source_lines);
        snprintf(id, sizeof(id), "PC34.wall_spec.%02zu.occlusion_line", i);
        check_int(id, strstr(spec->occlusion_source_lines, expected[i].occlusion_needle) != NULL, 1);

        bool flip = true;
        DM1_WallSetIndex native_sel = dm1_viewport_3d_select_wall_bitmap(spec, false, &flip);
        snprintf(id, sizeof(id), "PC34.wall_select.%02zu.native_index", i);
        check_int(id, (int)native_sel, (int)expected[i].native_wall);
        snprintf(id, sizeof(id), "PC34.wall_select.%02zu.native_flip", i);
        check_int(id, flip ? 1 : 0, 0);

        DM1_WallSetIndex parity_sel = dm1_viewport_3d_select_wall_bitmap(spec, true, &flip);
        snprintf(id, sizeof(id), "PC34.wall_select.%02zu.parity_index", i);
        check_int(id, (int)parity_sel, (int)expected[i].parity_wall);
        snprintf(id, sizeof(id), "PC34.wall_select.%02zu.parity_flip", i);
        check_int(id, flip ? 1 : 0, 1);
    }

    bool flip = true;
    check_int("PC34.wall_select.null_index", (int)dm1_viewport_3d_select_wall_bitmap(NULL, true, &flip), (int)DM1_WALL_SET_COUNT);
    check_int("PC34.wall_select.null_flip", flip ? 1 : 0, 0);
}

static void test_wall_item_occlusion_alcove_exception(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int alcove_reveals;
        const char *source_line;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, "6264" },
        { DM1_VIEW_SQUARE_D3R2, 0, "6331" },
        { DM1_VIEW_SQUARE_D3L,  1, "6437" },
        { DM1_VIEW_SQUARE_D3R,  1, "6573" },
        { DM1_VIEW_SQUARE_D3C,  1, "6720" },
        { DM1_VIEW_SQUARE_D2L2, 0, "6862" },
        { DM1_VIEW_SQUARE_D2R2, 0, "6893" },
        { DM1_VIEW_SQUARE_D2L,  1, "6973" },
        { DM1_VIEW_SQUARE_D2R,  1, "7166" },
        { DM1_VIEW_SQUARE_D2C,  1, "7312" },
        { DM1_VIEW_SQUARE_D1L,  0, "7460" },
        { DM1_VIEW_SQUARE_D1R,  0, "7628" },
        { DM1_VIEW_SQUARE_D1C,  1, "7843" },
        { DM1_VIEW_SQUARE_D0L,  0, "8038" },
        { DM1_VIEW_SQUARE_D0R,  0, "8144" },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportWallDrawSpec *spec =
            dm1_viewport_3d_get_wall_draw_spec_for_square(expected[i].square);
        char id[112];

        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.spec", i);
        check_nonnull(id, spec);
        if (!spec) continue;

        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.normal_blocks", i);
        check_int(id, dm1_viewport_3d_wall_occludes_floor_items(spec, false) ? 1 : 0, 1);
        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.normal_order", i);
        check_int(id, dm1_viewport_3d_wall_item_cell_order(spec, false), 0xffff);

        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.alcove_blocks", i);
        check_int(id, dm1_viewport_3d_wall_occludes_floor_items(spec, true) ? 1 : 0, expected[i].alcove_reveals ? 0 : 1);
        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.alcove_order", i);
        check_int(id, dm1_viewport_3d_wall_item_cell_order(spec, true), expected[i].alcove_reveals ? 0x0000 : 0xffff);

        snprintf(id, sizeof(id), "wall_item_occlusion.%zu.source", i);
        check_int(id, strstr(spec->occlusion_source_lines, expected[i].source_line) != NULL, 1);
    }

    check_int("wall_item_occlusion.null_blocks", dm1_viewport_3d_wall_occludes_floor_items(NULL, true) ? 1 : 0, 1);
    check_int("wall_item_occlusion.null_order", dm1_viewport_3d_wall_item_cell_order(NULL, true), 0xffff);
}

static void test_parity_flip_restore(void)
{
    unsigned char viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_Viewport3DState state;
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_load_wall_set(&state, 0, 0);

    int16_t native[DM1_WALL_SET_COUNT];
    memcpy(native, state.wall_set_native, sizeof(native));

    dm1_viewport_3d_draw_frame(&state, 0, 1, 0);
    check_int("F0128.parity.true", state.parity_flip, 1);
    check_int("F0128.parity.wall_restore", memcmp(state.wall_set, native, sizeof(native)) == 0, 1);

    dm1_viewport_3d_draw_frame(&state, 0, 2, 0);
    check_int("F0128.parity.false", state.parity_flip, 0);
    check_int("F0128.native.wall_stable", memcmp(state.wall_set, native, sizeof(native)) == 0, 1);
}

static void test_wall_frame_bitmap_global_null_guard(void)
{
    unsigned char viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    unsigned char expected[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_Viewport3DState state;
    memset(viewport, 0x5a, sizeof(viewport));
    memset(expected, 0x5a, sizeof(expected));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_load_wall_set(&state, 0, 0);
    state.floor_ceiling_dirty = false;

    check_int("g_dm1_wall_frame_bitmaps.default_null", g_dm1_wall_frame_bitmaps == NULL, 1);

    dm1_viewport_3d_draw_frame(&state, 0, 2, 0);
    check_int("g_dm1_wall_frame_bitmaps.null_guard_no_viewport_write",
              memcmp(viewport, expected, sizeof(viewport)) == 0, 1);
    check_int("g_dm1_wall_frame_bitmaps.null_guard_parity_still_updates", state.parity_flip, 0);
}

static void test_floor_ceiling_bands_and_zones(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_Viewport3DState state;

    /*
     * ReDMCSB: DUNVIEW.C F0098 lines 2962-3004 clears
     * G0086_puc_Bitmap_ViewportBlackArea, copies the ceiling/floor bitmaps
     * through F0674_F0128_sub on PC34/I34E, then resets
     * G0297_B_DrawFloorAndCeilingRequested.  Firestaff's current asset-free
     * fallback must still keep the same row ownership: top black-area rows and
     * the floor area are refreshed, while the intervening wall band is not
     * over-cleared before the square draw walk.
     */
    check_int("F0098.viewport.width", DM1_VIEWPORT_WIDTH, 224);
    check_int("F0098.viewport.height", DM1_VIEWPORT_HEIGHT, 136);
    check_int("F0098.black_area_h", DM1_VIEWPORT_BLACK_AREA_H, 37);
    check_int("F0098.ceiling_h", DM1_VIEWPORT_CEILING_H, 29);
    check_int("F0098.floor_y", DM1_VIEWPORT_FLOOR_Y, 66);
    check_int("F0098.floor_h", DM1_VIEWPORT_FLOOR_H, 70);
    check_int("PC34.zone.ceiling", DM1_PC34_ZONE_VIEWPORT_CEILING_AREA, 700);
    check_int("PC34.zone.floor", DM1_PC34_ZONE_VIEWPORT_FLOOR_AREA, 701);
    check_int("PC34.zone.wall_d0r", DM1_PC34_ZONE_WALL_D0R, 717);
    check_int("PC34.zone.door_frame_left_d2c", DM1_PC34_ZONE_DOOR_FRAME_LEFT_D2C, 724);
    check_int("PC34.zone.door_frame_right_d2c", DM1_PC34_ZONE_DOOR_FRAME_RIGHT_D2C, 725);
    check_int("PC34.zone.door_frame_top_d2c", DM1_PC34_ZONE_DOOR_FRAME_TOP_D2C, 730);

    memset(viewport, 0x5a, sizeof(viewport));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    state.floor_ceiling_dirty = true;

    dm1_viewport_3d_draw_floor_ceiling(&state);
    check_int("F0098.pixel.black_area_first_row_clear",
              viewport[0 * DM1_VIEWPORT_WIDTH + 17], 0);
    check_int("F0098.pixel.black_area_last_row_clear",
              viewport[(DM1_VIEWPORT_BLACK_AREA_H - 1) * DM1_VIEWPORT_WIDTH + 223], 0);
    check_int("F0098.pixel.wall_band_after_black_preserved",
              viewport[DM1_VIEWPORT_BLACK_AREA_H * DM1_VIEWPORT_WIDTH + 17], 0x5a);
    check_int("F0098.pixel.wall_band_before_floor_preserved",
              viewport[(DM1_VIEWPORT_FLOOR_Y - 1) * DM1_VIEWPORT_WIDTH + 111], 0x5a);
    check_int("F0098.pixel.floor_first_row_clear",
              viewport[DM1_VIEWPORT_FLOOR_Y * DM1_VIEWPORT_WIDTH + 0], 0);
    check_int("F0098.pixel.floor_last_row_clear",
              viewport[(DM1_VIEWPORT_HEIGHT - 1) * DM1_VIEWPORT_WIDTH + 223], 0);
    check_int("F0098.pixel.dirty_flag_reset", state.floor_ceiling_dirty ? 1 : 0, 0);
}


static void test_f0115_cell_order_and_layer_z_order(void)
{
    DM1_ViewportCellOrder o = dm1_viewport_3d_decode_cell_order(0x3421);
    check_int("F0115.cell_order.3421.count", o.cell_count, 4);
    check_int("F0115.cell_order.3421.cell0", o.cells[0], 1);
    check_int("F0115.cell_order.3421.cell1", o.cells[1], 2);
    check_int("F0115.cell_order.3421.cell2", o.cells[2], 4);
    check_int("F0115.cell_order.3421.cell3", o.cells[3], 3);
    check_int("F0115.cell_order.3421.door_pass", o.door_pass, 0);
    check_int("F0115.cell_order.3421.alcove", o.alcove ? 1 : 0, 0);

    o = dm1_viewport_3d_decode_cell_order(0x0218);
    check_int("F0115.cell_order.0218.count", o.cell_count, 2);
    check_int("F0115.cell_order.0218.door_pass", o.door_pass, 1);
    check_int("F0115.cell_order.0218.cell0", o.cells[0], 1);
    check_int("F0115.cell_order.0218.cell1", o.cells[1], 2);

    o = dm1_viewport_3d_decode_cell_order(0x0349);
    check_int("F0115.cell_order.0349.count", o.cell_count, 2);
    check_int("F0115.cell_order.0349.door_pass", o.door_pass, 2);
    check_int("F0115.cell_order.0349.cell0", o.cells[0], 4);
    check_int("F0115.cell_order.0349.cell1", o.cells[1], 3);

    o = dm1_viewport_3d_decode_cell_order(0x0000);
    check_int("F0115.cell_order.alcove", o.alcove ? 1 : 0, 1);
    check_int("F0115.cell_order.alcove.count", o.cell_count, 0);

    check_int("F0115.layer.count", (int)dm1_viewport_3d_thing_layer_spec_count(), 4);
    for (size_t i = 0; i < dm1_viewport_3d_thing_layer_spec_count(); ++i) {
        const DM1_ViewportThingLayerSpec *layer = dm1_viewport_3d_get_thing_layer_spec(i);
        char id[96];
        snprintf(id, sizeof(id), "F0115.layer.%zu.nonnull", i);
        check_nonnull(id, layer);
        if (!layer) continue;
        snprintf(id, sizeof(id), "F0115.layer.%zu.ordinal", i);
        check_int(id, (int)layer->layer, (int)i);
        snprintf(id, sizeof(id), "F0115.layer.%zu.source", i);
        check_nonnull(id, layer->source_lines);
        snprintf(id, sizeof(id), "F0115.layer.%zu.per_cell", i);
        check_int(id, layer->repeats_per_cell ? 1 : 0, i < 3 ? 1 : 0);
        snprintf(id, sizeof(id), "F0115.layer.%zu.after_all_cells", i);
        check_int(id, layer->after_all_cells ? 1 : 0, i == 3 ? 1 : 0);
    }
    check_int("F0115.layer.out_of_range", dm1_viewport_3d_get_thing_layer_spec(4) == NULL, 1);
}




static void test_projectile_occlusion_zone_mapping(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int source_id;
        int depth;
        int row;
        int zone0;
        int zone1;
        int zone2;
        int zone3;
        int scale0;
        int scale2;
        const char *line_needle;
    } expected[] = {
        { DM1_VIEW_SQUARE_D0C,   0, 0, 11, 2944, 2945, -1,   -1, 0, -1, "5675" },
        { DM1_VIEW_SQUARE_D1C,   3, 1,  8, 2932, 2933, 2934, 2935, 2, 1, "5667" },
        { DM1_VIEW_SQUARE_D1L,   4, 1,  9, 2936, 2937, 2938, 2939, 2, 1, "5667" },
        { DM1_VIEW_SQUARE_D1R,   5, 1, 10, 2940, 2941, 2942, 2943, 2, 1, "5667" },
        { DM1_VIEW_SQUARE_D2C,   6, 2,  5, 2920, 2921, 2922, 2923, 4, 3, "5667" },
        { DM1_VIEW_SQUARE_D2L,   7, 2,  6, 2924, 2925, 2926, 2927, 4, 3, "5667" },
        { DM1_VIEW_SQUARE_D2R,   8, 2,  7, 2928, 2929, 2930, 2931, 4, 3, "5667" },
        { DM1_VIEW_SQUARE_D3C,  11, 3,  0,   -1,   -1, 2902, 2903, -1, 5, "5672" },
        { DM1_VIEW_SQUARE_D3L,  12, 3,  1,   -1,   -1, 2906, 2907, -1, 5, "5672" },
        { DM1_VIEW_SQUARE_D3R,  13, 3,  2,   -1,   -1, 2910, 2911, -1, 5, "5672" },
        { DM1_VIEW_SQUARE_D3L2, 14, 3,  3,   -1,   -1, 2914, 2915, -1, 5, "5672" },
        { DM1_VIEW_SQUARE_D3R2, 15, 3,  4,   -1,   -1, 2918, 2919, -1, 5, "5672" },
    };

    check_int("projectile_occlusion.count", (int)dm1_viewport_3d_projectile_occlusion_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportProjectileOcclusionSpec *spec = dm1_viewport_3d_get_projectile_occlusion_spec_for_square(expected[i].square);
        char id[112];
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.source_id", i);
        check_int(id, spec->redmcsb_view_square_id, expected[i].source_id);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.depth", i);
        check_int(id, spec->view_depth, expected[i].depth);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.g2028_row", i);
        check_int(id, spec->g2028_row, expected[i].row);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.zone0", i);
        check_int(id, dm1_viewport_3d_projectile_zone_for_cell(spec, 0), expected[i].zone0);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.zone1", i);
        check_int(id, dm1_viewport_3d_projectile_zone_for_cell(spec, 1), expected[i].zone1);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.zone2", i);
        check_int(id, dm1_viewport_3d_projectile_zone_for_cell(spec, 2), expected[i].zone2);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.zone3", i);
        check_int(id, dm1_viewport_3d_projectile_zone_for_cell(spec, 3), expected[i].zone3);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.scale0", i);
        check_int(id, dm1_viewport_3d_projectile_scale_index_for_cell(spec, 0), expected[i].scale0);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.scale2", i);
        check_int(id, dm1_viewport_3d_projectile_scale_index_for_cell(spec, 2), expected[i].scale2);
        snprintf(id, sizeof(id), "projectile_occlusion.%zu.source", i);
        check_int(id, strstr(spec->source_lines, expected[i].line_needle) != NULL, 1);
    }
    check_int("projectile_occlusion.d0l_unsupported", dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0L) == NULL, 1);
    check_int("projectile_occlusion.out_of_range", dm1_viewport_3d_get_projectile_occlusion_spec(12) == NULL, 1);
    check_int("projectile_occlusion.null_zone", dm1_viewport_3d_projectile_zone_for_cell(NULL, 0), -1);
}


static void test_explosion_occlusion_zone_mapping(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int source_id;
        int depth;
        int row;
        int field_aspect;
        int rebirth_row;
        int d0c_zone;
        int centered_zone;
        int cell0_zone;
        int cell1_zone;
        int rebirth1_zone;
        int rebirth2_zone;
        const char *line_needle;
    } expected[] = {
        { DM1_VIEW_SQUARE_D0C,   0, 0, 14, 13, 11, 4, -1,   -1,   -1, 3011,   -1, "6031" },
        { DM1_VIEW_SQUARE_D0L,   1, 0, 15, 14, -1, -1, 3029, 3061, 3062, -1,   -1, "6106" },
        { DM1_VIEW_SQUARE_D0R,   2, 0, 16, 15, -1, -1, 3030, 3063, 3064, -1,   -1, "6106" },
        { DM1_VIEW_SQUARE_D1C,   3, 1, 11, 10,  8, -1, 3025, 3053, 3054, 3008, 3015, "5983" },
        { DM1_VIEW_SQUARE_D1L,   4, 1, 12, 11,  9, -1, 3026, 3055, 3056, 3009, 3016, "5983" },
        { DM1_VIEW_SQUARE_D1R,   5, 1, 13, 12, 10, -1, 3027, 3057, 3058, 3010, 3017, "5983" },
        { DM1_VIEW_SQUARE_D2C,   6, 2,  8,  7,  5, -1, 3022, 3047, 3048, 3005, 3012, "5983" },
        { DM1_VIEW_SQUARE_D2L,   7, 2,  9,  8,  6, -1, 3023, 3049, 3050, 3006, 3013, "5983" },
        { DM1_VIEW_SQUARE_D2R,   8, 2, 10,  9,  7, -1, 3024, 3051, 3052, 3007, 3014, "5983" },
        { DM1_VIEW_SQUARE_D3C,  11, 3,  3,  2,  0, -1, 3017, 3037, 3038, 3000, 3007, "5983" },
        { DM1_VIEW_SQUARE_D3L,  12, 3,  4,  3,  1, -1, 3018, 3039, 3040, 3001, 3008, "5983" },
        { DM1_VIEW_SQUARE_D3R,  13, 3,  5,  4,  2, -1, 3019, 3041, 3042, 3002, 3009, "5983" },
        { DM1_VIEW_SQUARE_D3L2, 14, 3,  6,  0,  3, -1, 3020, 3043, 3044, 3003, 3010, "5983" },
        { DM1_VIEW_SQUARE_D3R2, 15, 3,  7,  1,  4, -1, 3021, 3045, 3046, 3004, 3011, "5983" },
        { DM1_VIEW_SQUARE_D4C,  16, 4,  0, -1, -1, -1, 3014, 3031, 3032, -1,   -1, "6106" },
        { DM1_VIEW_SQUARE_D4L,  17, 4,  1, -1, -1, -1, 3015, 3033, 3034, -1,   -1, "6106" },
        { DM1_VIEW_SQUARE_D4R,  18, 4,  2, -1, -1, -1, 3016, 3035, 3036, -1,   -1, "6106" },
    };

    check_int("explosion_occlusion.count", (int)dm1_viewport_3d_explosion_occlusion_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportExplosionOcclusionSpec *spec = dm1_viewport_3d_get_explosion_occlusion_spec_for_square(expected[i].square);
        char id[112];
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.source_id", i);
        check_int(id, spec->redmcsb_view_square_id, expected[i].source_id);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.depth", i);
        check_int(id, spec->view_depth, expected[i].depth);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.g2034_row", i);
        check_int(id, spec->g2034_row, expected[i].row);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.g2035_field_aspect", i);
        check_int(id, spec->g2035_field_aspect, expected[i].field_aspect);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.rebirth_row", i);
        check_int(id, spec->rebirth_row, expected[i].rebirth_row);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.d0c_zone", i);
        check_int(id, dm1_viewport_3d_explosion_d0c_pattern_zone(spec), expected[i].d0c_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.centered_zone", i);
        check_int(id, dm1_viewport_3d_explosion_centered_zone(spec), expected[i].centered_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.cell0_zone", i);
        check_int(id, dm1_viewport_3d_explosion_two_cell_zone(spec, 0), expected[i].cell0_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.cell1_zone", i);
        check_int(id, dm1_viewport_3d_explosion_two_cell_zone(spec, 1), expected[i].cell1_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.rebirth1_zone", i);
        check_int(id, dm1_viewport_3d_explosion_rebirth_step1_zone(spec), expected[i].rebirth1_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.rebirth2_zone", i);
        check_int(id, dm1_viewport_3d_explosion_rebirth_step2_zone(spec), expected[i].rebirth2_zone);
        snprintf(id, sizeof(id), "explosion_occlusion.%zu.source", i);
        check_int(id, strstr(spec->source_lines, expected[i].line_needle) != NULL, 1);
    }
    check_int("explosion_occlusion.d2l2_regular_unsupported", dm1_viewport_3d_get_explosion_occlusion_spec_for_square(DM1_VIEW_SQUARE_D2L2) == NULL, 1);
    check_int("explosion_occlusion.out_of_range", dm1_viewport_3d_get_explosion_occlusion_spec(17) == NULL, 1);
    check_int("explosion_occlusion.null_zone", dm1_viewport_3d_explosion_centered_zone(NULL), -1);
    check_int("explosion_occlusion.bad_cell", dm1_viewport_3d_explosion_two_cell_zone(dm1_viewport_3d_get_explosion_occlusion_spec_for_square(DM1_VIEW_SQUARE_D3C), 2), -1);
}

static void test_projectile_wall_zone_movement_visibility_gate(void)
{
    struct ProjectileInstance_Compat projectile;
    struct CellContentDigest_Compat digest;
    struct ProjectileInstance_Compat next;
    struct ProjectileTickResult_Compat result;
    const DM1_ViewportWallDrawSpec *plain_wall =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2L2);
    const DM1_ViewportWallDrawSpec *alcove_wall =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3L);
    const DM1_ViewportProjectileOcclusionSpec *d1c_projectiles =
        dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1C);
    int blocker = -1;

    memset(&projectile, 0, sizeof(projectile));
    projectile.slotIndex = 3;
    projectile.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile.ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile.ownerIndex = 0;
    projectile.mapIndex = 0;
    projectile.mapX = 10;
    projectile.mapY = 10;
    projectile.cell = 0;
    projectile.direction = 0;
    projectile.kineticEnergy = 40;
    projectile.attack = 20;
    projectile.stepEnergy = 5;
    projectile.firstMoveGraceFlag = 0;
    projectile.attackTypeCode = COMBAT_ATTACK_NORMAL;
    projectile.reserved3 = 1;

    memset(&digest, 0, sizeof(digest));
    digest.sourceMapIndex = 0;
    digest.sourceMapX = 10;
    digest.sourceMapY = 10;
    digest.sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destMapIndex = 0;
    digest.destMapX = 10;
    digest.destMapY = 9;
    digest.destSquareType = PROJECTILE_ELEMENT_WALL;
    digest.destDoorState = PROJECTILE_DOOR_STATE_NONE;
    digest.destTeleporterNewDirection = -1;

    check_int("projectile_wall_zone.d1c_projectile_zone_cell0",
              dm1_viewport_3d_projectile_zone_for_cell(d1c_projectiles, 0), 2932);
    check_int("projectile_wall_zone.inspect_wall_blocker",
              F0814_PROJECTILE_InspectDestination_Compat(&digest, &blocker), 1);
    check_int("projectile_wall_zone.blocker_is_wall", blocker, PROJECTILE_BLOCKER_WALL);
    check_int("projectile_wall_zone.advance_reports_wall_hit",
              F0811_PROJECTILE_Advance_Compat(&projectile, &digest, 77, NULL, &next, &result), 1);
    check_int("projectile_wall_zone.wall_hit_result", result.resultKind, PROJECTILE_RESULT_HIT_WALL);
    check_int("projectile_wall_zone.wall_hit_despawns", result.despawn, 1);
    check_int("projectile_wall_zone.wall_hit_not_committed_to_destination", result.newMapY, 10);

    check_nonnull("projectile_wall_zone.plain_wall_nonnull", plain_wall);
    if (plain_wall) {
        check_int("projectile_wall_zone.plain_wall_case_returns", plain_wall->wall_case_returns ? 1 : 0, 1);
        check_int("projectile_wall_zone.plain_wall_hides_projectile",
                  dm1_viewport_3d_projectile_visible_after_wall_case(plain_wall, false) ? 1 : 0, 0);
    }

    check_nonnull("projectile_wall_zone.alcove_wall_nonnull", alcove_wall);
    if (alcove_wall) {
        check_int("projectile_wall_zone.alcove_without_front_hides",
                  dm1_viewport_3d_projectile_visible_after_wall_case(alcove_wall, false) ? 1 : 0, 0);
        check_int("projectile_wall_zone.front_alcove_reveals_projectile_layer",
                  dm1_viewport_3d_projectile_visible_after_wall_case(alcove_wall, true) ? 1 : 0, 1);
    }
}

static void test_door_front_occlusion_split_passes(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        const char *floor_line;
        const char *rear_line;
        const char *frame_line;
        const char *button_line;
        const char *door_line;
        const char *front_line;
        uint16_t rear_order;
        uint16_t front_order;
        unsigned char rear_cells[2];
        unsigned char front_cells[2];
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, "6270", "6271", "6272", NULL,   "6272", "6286", 0x0218, 0x0349, {1, 2}, {4, 3} },
        { DM1_VIEW_SQUARE_D3R2, "6337", "6338", "6339", NULL,   "6339", "6353", 0x0128, 0x0439, {2, 1}, {3, 4} },
        { DM1_VIEW_SQUARE_D3L, "6443", "6444", "6446", NULL,   "6457", "6459", 0x0218, 0x0349, {1, 2}, {4, 3} },
        { DM1_VIEW_SQUARE_D3R, "6579", "6580", "6582", "6592", "6598", "6601", 0x0128, 0x0439, {2, 1}, {3, 4} },
        { DM1_VIEW_SQUARE_D3C, "6722", "6723", "6725", "6737", "6744", "6746", 0x0218, 0x0349, {1, 2}, {4, 3} },
        { DM1_VIEW_SQUARE_D2L, "6988", "6989", "6991", NULL,   "7000", "7003", 0x0218, 0x0349, {1, 2}, {4, 3} },
        { DM1_VIEW_SQUARE_D2R, "7181", "7182", "7184", NULL,   "7193", "7196", 0x0128, 0x0439, {2, 1}, {3, 4} },
        { DM1_VIEW_SQUARE_D2C, "7314", "7315", "7317", "7332", "7339", "7341", 0x0218, 0x0349, {1, 2}, {4, 3} },
        { DM1_VIEW_SQUARE_D1L, "7493", "7494", "7496", NULL,   "7506", "7536", 0x0028, 0x0039, {2, 0}, {3, 0} },
        { DM1_VIEW_SQUARE_D1R, "7661", "7662", "7664", NULL,   "7674", "7704", 0x0018, 0x0049, {1, 0}, {4, 0} },
        { DM1_VIEW_SQUARE_D1C, "7874", "7875", "7877", "7901", "7905", "7937", 0x0218, 0x0349, {1, 2}, {4, 3} },
    };

    check_int("door_front_occlusion.count", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 11);
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportDoorFrontOcclusionSpec *spec =
            dm1_viewport_3d_get_door_front_occlusion_spec_for_square(expected[i].square);
        DM1_ViewportCellOrder rear;
        DM1_ViewportCellOrder front;
        char id[96];
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        check_int("door_front_occlusion.rear_order", spec->rear_cell_order, expected[i].rear_order);
        check_int("door_front_occlusion.front_order", spec->front_cell_order, expected[i].front_order);
        rear = dm1_viewport_3d_decode_cell_order(spec->rear_cell_order);
        front = dm1_viewport_3d_decode_cell_order(spec->front_cell_order);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.floor_line", i);
        check_int(id, strstr(spec->floor_source_lines, expected[i].floor_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_pass", i);
        check_int(id, rear.door_pass, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_cells", i);
        check_int(id, rear.cell_count >= 1 && rear.cells[0] == expected[i].rear_cells[0] && (rear.cell_count == 1 || rear.cells[1] == expected[i].rear_cells[1]), 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_pass", i);
        check_int(id, front.door_pass, 2);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_cells", i);
        check_int(id, front.cell_count >= 1 && front.cells[0] == expected[i].front_cells[0] && (front.cell_count == 1 || front.cells[1] == expected[i].front_cells[1]), 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_line", i);
        check_int(id, strstr(spec->rear_pass_source_lines, expected[i].rear_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.frame_line", i);
        check_int(id, strstr(spec->frame_source_lines, expected[i].frame_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.button_line", i);
        check_int(id, expected[i].button_line
            ? (spec->button_source_lines != NULL && strstr(spec->button_source_lines, expected[i].button_line) != NULL)
            : (spec->button_source_lines == NULL), 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.door_line", i);
        check_int(id, strstr(spec->door_source_lines, expected[i].door_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_line", i);
        check_int(id, strstr(spec->front_pass_source_lines, expected[i].front_line) != NULL, 1);
    }
    check_int("door_front_occlusion.out_of_range", dm1_viewport_3d_get_door_front_occlusion_spec(11) == NULL, 1);
    check_int("door_front_occlusion.d1l_side_door_front_spec", dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1L) != NULL, 1);
}

static void test_side_door_stairs_occlusion_cell_orders(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        uint16_t order;
        unsigned char cells[3];
        unsigned char count;
        const char *function_name;
        const char *branch_line;
        const char *f0115_line;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L, 0x0321, { 1, 2, 3 }, 3, "F0116_DUNGEONVIEW_DrawSquareD3L", "6438", "6480" },
        { DM1_VIEW_SQUARE_D3R, 0x0412, { 2, 1, 4 }, 3, "F0117_DUNGEONVIEW_DrawSquareD3R", "6574", "6621" },
        { DM1_VIEW_SQUARE_D2L, 0x0342, { 2, 4, 3 }, 3, "F0119_DUNGEONVIEW_DrawSquareD2L", "6974", "7027" },
        { DM1_VIEW_SQUARE_D2R, 0x0431, { 1, 3, 4 }, 3, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "7167", "7219" },
        { DM1_VIEW_SQUARE_D1L, 0x0032, { 2, 3, 0 }, 2, "F0122_DUNGEONVIEW_DrawSquareD1L", "7461", "7536" },
        { DM1_VIEW_SQUARE_D1R, 0x0041, { 1, 4, 0 }, 2, "F0123_DUNGEONVIEW_DrawSquareD1R", "7629", "7704" },
        { DM1_VIEW_SQUARE_D0L, 0x0002, { 2, 0, 0 }, 1, "F0125_DUNGEONVIEW_DrawSquareD0L", "8000", "8005" },
        { DM1_VIEW_SQUARE_D0R, 0x0001, { 1, 0, 0 }, 1, "F0126_DUNGEONVIEW_DrawSquareD0R", "8110", "8115" },
    };

    check_int("side_occlusion.count", (int)dm1_viewport_3d_side_occlusion_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportSideOcclusionSpec *spec =
            dm1_viewport_3d_get_side_occlusion_spec_for_square(expected[i].square);
        DM1_ViewportCellOrder decoded;
        char id[128];
        snprintf(id, sizeof(id), "side_occlusion.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "side_occlusion.%zu.function", i);
        check_int(id, strcmp(spec->function_name, expected[i].function_name) == 0, 1);
        snprintf(id, sizeof(id), "side_occlusion.%zu.order", i);
        check_int(id, spec->cell_order, expected[i].order);
        decoded = dm1_viewport_3d_decode_cell_order(spec->cell_order);
        snprintf(id, sizeof(id), "side_occlusion.%zu.not_door_pass", i);
        check_int(id, decoded.door_pass, 0);
        snprintf(id, sizeof(id), "side_occlusion.%zu.count", i);
        check_int(id, decoded.cell_count, expected[i].count);
        for (unsigned char c = 0; c < expected[i].count; ++c) {
            snprintf(id, sizeof(id), "side_occlusion.%zu.cell.%u", i, c);
            check_int(id, decoded.cells[c], expected[i].cells[c]);
        }
        snprintf(id, sizeof(id), "side_occlusion.%zu.branch_source", i);
        check_int(id, strstr(spec->branch_source_lines, expected[i].branch_line) != NULL, 1);
        snprintf(id, sizeof(id), "side_occlusion.%zu.f0115_source", i);
        check_int(id, strstr(spec->f0115_source_lines, expected[i].f0115_line) != NULL, 1);
    }
    check_int("side_occlusion.out_of_range", dm1_viewport_3d_get_side_occlusion_spec(8) == NULL, 1);
    check_int("side_occlusion.no_center_spec", dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_VIEW_SQUARE_D2C) == NULL, 1);
}

static void test_d0c_thieves_eye_door_frame_occlusion_order(void)
{
    const DM1_ViewportThievesEyeDoorFrameOcclusionSpec *spec =
        dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C);

    check_int("d0c_thieves_eye.count", (int)dm1_viewport_3d_thieves_eye_door_frame_occlusion_spec_count(), 1);
    check_nonnull("d0c_thieves_eye.spec", spec);
    if (!spec) return;
    check_int("d0c_thieves_eye.square", (int)spec->square, (int)DM1_VIEW_SQUARE_D0C);
    check_int("d0c_thieves_eye.cell_order", spec->cell_order, 0x0021);
    check_int("d0c_thieves_eye.door_frame_zone", spec->door_frame_zone, 728);
    check_int("d0c_thieves_eye.hole_zone", spec->hole_zone, 736);
    check_int("d0c_thieves_eye.branch_source", strstr(spec->branch_source_lines, "8185-8188") != NULL, 1);
    check_int("d0c_thieves_eye.copy_source", strstr(spec->copy_source_lines, "8199-8201") != NULL, 1);
    check_int("d0c_thieves_eye.hole_source", strstr(spec->hole_source_lines, "8206-8210") != NULL, 1);
    check_int("d0c_thieves_eye.frame_blit_source", strstr(spec->frame_blit_source_lines, "8215-8216") != NULL, 1);
    check_int("d0c_thieves_eye.f0115_source", strstr(spec->f0115_source_lines, "8294") != NULL, 1);
    check_int("d0c_thieves_eye.out_of_range",
        dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec(1) == NULL, 1);
}

static void test_post_command_redraw_contract(void)
{
    const DM1_ViewportPostCommandRedrawSpec *spec = dm1_viewport_3d_post_command_redraw_spec();
    check_nonnull("post_command_redraw.nonnull", spec);
    if (!spec) return;
    check_int("post_command_redraw.command_mutates_before_draw", spec->command_mutates_before_draw ? 1 : 0, 1);
    check_int("post_command_redraw.redraw_uses_party_tuple", spec->redraw_uses_party_tuple ? 1 : 0, 1);
    check_int("post_command_redraw.present_waits", spec->present_waits_for_viewport ? 1 : 0, 1);
    check_int("post_command_redraw.command_source", strstr(spec->command_source_lines, "COMMAND.C:2045-2156") != NULL, 1);
    check_int("post_command_redraw.pop_unlock_source", strstr(spec->command_source_lines, "2118-2127") != NULL, 1);
    check_int("post_command_redraw.turn_move_dispatch_source", strstr(spec->command_source_lines, "2150-2156") != NULL, 1);
    check_int("post_command_redraw.mainloop_source", strstr(spec->main_loop_source_lines, "GAMELOOP.C:55-90") != NULL, 1);
    check_int("post_command_redraw.present_source", strstr(spec->present_source_lines, "DRAWVIEW.C:709-722") != NULL, 1);
}

static void test_same_viewport_capture_contract(void)
{
    const DM1_ViewportSameViewportCaptureContract *spec =
        dm1_viewport_3d_same_viewport_capture_contract();

    check_nonnull("same_viewport_capture.nonnull", spec);
    if (!spec) return;
    check_int("same_viewport_capture.requires_original_transcript", spec->requires_original_command_transcript ? 1 : 0, 1);
    check_int("same_viewport_capture.requires_firestaff_tuple", spec->requires_same_firestaff_view_tuple ? 1 : 0, 1);
    check_int("same_viewport_capture.duplicate_hashes_block", spec->duplicate_original_viewport_hashes_block_promotion ? 1 : 0, 1);
    check_int("same_viewport_capture.requires_assets", spec->requires_pc34_asset_hashes ? 1 : 0, 1);
    check_int("same_viewport_capture.mouse_source", strstr(spec->mouse_zone_source_lines, "COMMAND.C:106-114") != NULL, 1);
    check_int("same_viewport_capture.queue_source", strstr(spec->queue_source_lines, "COMMAND.C:2045-2156") != NULL, 1);
    check_int("same_viewport_capture.turn_source", strstr(spec->turn_source_lines, "CLIKMENU.C:142-174") != NULL, 1);
    check_int("same_viewport_capture.move_source", strstr(spec->move_source_lines, "CLIKMENU.C:180-347") != NULL, 1);
    check_int("same_viewport_capture.draw_source", strstr(spec->draw_source_lines, "DUNVIEW.C:8318-8611") != NULL, 1);
    check_int("same_viewport_capture.present_source", strstr(spec->present_source_lines, "DRAWVIEW.C:709-858") != NULL, 1);
    check_int("same_viewport_capture.asset_source", strstr(spec->asset_source_lines, "GRAPHICS.DAT") != NULL && strstr(spec->asset_source_lines, "DUNGEON.DAT") != NULL, 1);
}

static void test_floor_field_stairs_pit_teleporter_order(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        const char *function_name;
        uint16_t order;
        int has_floor_ornament;
        const char *stairs_line;
        const char *pit_line;
        const char *floor_ornament_line;
        const char *things_line;
        const char *field_line;
        const char *wall_return_line;
        int d0c_foreground_before_things;
        int has_things_pass;
        int field_after_things;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, "F0676_DrawD3L2", 0x3421, 1, "6237-6252", "6275-6278", "6282-6284", "6286", "6288-6289", "6253-6264", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3R2, "F0677_DrawD3R2", 0x4312, 1, "6304-6319", "6342-6345", "6349-6351", "6353", "6355-6356", "6320-6331", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3L,  "F0116_DUNGEONVIEW_DrawSquareD3L", 0x3421, 1, "6375-6405", "6461-6472", "6475-6478", "6480", "6482-6495", "6406-6437", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3R,  "F0117_DUNGEONVIEW_DrawSquareD3R", 0x4312, 1, "6514-6544", "6603-6614", "6617-6620", "6622", "6624-6638", "6545-6573", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3C,  "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", 0x3421, 1, "6666-6696", "6748-6762", "6811-6814", "6816", "6818-6831", "6697-6720", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2L,  "F0119_DUNGEONVIEW_DrawSquareD2L", 0x3421, 1, "6914-6944", "7005-7015", "7017-7020", "7031", "7033-7048", "6945-6973", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2R,  "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", 0x4312, 1, "7065-7095", "7198-7208", "7210-7213", "7224", "7226-7240", "7097-7166", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2C,  "F0121_DUNGEONVIEW_DrawSquareD2C", 0x3421, 1, "7260-7288", "7343-7353", "7355-7357", "7367-7368", "7370-7388", "7289-7312", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2L2, "F0678_DrawD2L2", 0x0000, 0, "6846-6865", "6846-6865", "6846-6865", "no F0115 thing pass", "6863-6865", "6848-6862", 0, 0, 0 },
        { DM1_VIEW_SQUARE_D2R2, "F0679_DrawD2R2", 0x0000, 0, "6877-6896", "6877-6896", "6877-6896", "no F0115 thing pass", "6894-6896", "6879-6893", 0, 0, 0 },
        { DM1_VIEW_SQUARE_D1L,  "F0122_DUNGEONVIEW_DrawSquareD1L", 0x0032, 1, "7405-7435", "7510-7520", "7522-7533", "7535-7536", "7538-7555", "7436-7460", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D1R,  "F0123_DUNGEONVIEW_DrawSquareD1R", 0x0041, 1, "7573-7603", "7678-7688", "7690-7701", "7703-7704", "7706-7722", "7604-7628", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0L,  "F0125_DUNGEONVIEW_DrawSquareD0L", 0x0002, 0, "7978-7988", "7989-7998", "7999-8005", "8005", "8050-8059", "8007-8038", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0R,  "F0126_DUNGEONVIEW_DrawSquareD0R", 0x0001, 0, "8082-8092", "8093-8102", "8103-8115", "8115", "8150-8159", "8117-8144", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0C,  "F0127_DUNGEONVIEW_DrawSquareD0C", 0x0021, 0, "8241-8273", "8274-8292", "8284-8294", "8294", "8295-8308", "8185-8240", 1, 1, 1 },
    };

    check_int("floor_field_order.count", (int)dm1_viewport_3d_floor_field_order_spec_count(), 15);
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportFloorFieldOrderSpec *spec =
            dm1_viewport_3d_get_floor_field_order_spec_for_square(expected[i].square);
        char id[128];
        snprintf(id, sizeof(id), "floor_field_order.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        snprintf(id, sizeof(id), "floor_field_order.%zu.function", i);
        check_int(id, strcmp(spec->function_name, expected[i].function_name) == 0, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.cell_order", i);
        check_int(id, spec->cell_order, expected[i].order);
        snprintf(id, sizeof(id), "floor_field_order.%zu.stairs_before_floor", i);
        check_int(id, spec->stairs_draw_before_floor_ornament ? 1 : 0,
                  (expected[i].square == DM1_VIEW_SQUARE_D2L2 || expected[i].square == DM1_VIEW_SQUARE_D2R2) ? 0 : 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.pit_before_floor", i);
        check_int(id, spec->pit_draw_before_floor_ornament ? 1 : 0,
                  (expected[i].square == DM1_VIEW_SQUARE_D2L2 || expected[i].square == DM1_VIEW_SQUARE_D2R2) ? 0 : 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.floor_before_things", i);
        check_int(id, spec->floor_ornament_before_things ? 1 : 0, expected[i].has_floor_ornament);
        snprintf(id, sizeof(id), "floor_field_order.%zu.layer_z", i);
        check_int(id, spec->objects_creatures_projectiles_before_explosions ? 1 : 0, expected[i].has_things_pass);
        snprintf(id, sizeof(id), "floor_field_order.%zu.field_after_things", i);
        check_int(id, spec->field_after_things ? 1 : 0, expected[i].field_after_things);
        snprintf(id, sizeof(id), "floor_field_order.%zu.d0c_foreground_before_things", i);
        check_int(id, spec->d0c_foreground_before_things ? 1 : 0, expected[i].d0c_foreground_before_things);
        snprintf(id, sizeof(id), "floor_field_order.%zu.wall_return", i);
        check_int(id, spec->wall_case_returns_before_things ? 1 : 0, expected[i].square == DM1_VIEW_SQUARE_D0C ? 0 : 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.stairs_source", i);
        check_int(id, strstr(spec->stairs_source_lines, expected[i].stairs_line) != NULL, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.pit_source", i);
        check_int(id, strstr(spec->pit_source_lines, expected[i].pit_line) != NULL, 1);
        /*
         * Metadata-only ornament evidence: ReDMCSB DUNVIEW.C
         * F0108 lines 3940-4011 (PC34/I34E F0791 at line 3998)
         * performs the actual floor-ornament bitmap blit.  The per-square
         * call sites below source-lock the branch/order only, including the
         * BUG0_64 open-pit-overdraw behavior; this is not full real-asset
         * floor-ornament bitmap parity.
         */
        snprintf(id, sizeof(id), "floor_field_order.%zu.floor_ornament_source", i);
        check_int(id, strstr(spec->floor_ornament_source_lines, expected[i].floor_ornament_line) != NULL, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.floor_ornament_f0108", i);
        check_int(id, expected[i].has_floor_ornament
            ? strstr(spec->floor_ornament_source_lines, "F0108") != NULL
            : strstr(spec->floor_ornament_source_lines, "no floor") != NULL ||
              strstr(spec->floor_ornament_source_lines, "no D0C floor-ornament") != NULL ||
              strstr(spec->floor_ornament_source_lines, "no floor-ornament call") != NULL,
            1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.floor_ornament_open_pit_order", i);
        check_int(id, expected[i].has_floor_ornament
            ? strstr(spec->pit_source_lines, "BUG0_64") != NULL
            : strstr(spec->pit_source_lines, "BUG0_64") == NULL,
            1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.things_source", i);
        check_int(id, strstr(spec->things_source_lines, expected[i].things_line) != NULL, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.field_source", i);
        check_int(id, strstr(spec->field_source_lines, expected[i].field_line) != NULL, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.wall_source", i);
        check_int(id, strstr(spec->wall_return_source_lines, expected[i].wall_return_line) != NULL, 1);
    }
    check_int("floor_field_order.out_of_range", dm1_viewport_3d_get_floor_field_order_spec(15) == NULL, 1);
    check_int("floor_field_order.d0l_side_spec", dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D0L) != NULL, 1);
    check_int("floor_field_order.d0r_side_spec", dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D0R) != NULL, 1);
    check_int("floor_field_order.d2l2_no_thing_pass",
              dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D2L2)->objects_creatures_projectiles_before_explosions ? 1 : 0,
              0);
}


static void pass760_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_wall_ornament(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        DM1_ViewSquareIndex paired_wall_square;
        DM1_WallSetIndex native_wall;
        DM1_WallSetIndex parity_wall;
        uint16_t pc34_wall_zone;
        uint16_t companion_d3_wall_zone;
        int rel_lateral;
        int view_square_ordinal;
        int view_floor_ordinal;
        int view_wall_side_ordinal;
        int view_wall_front_ordinal;
        uint16_t open_cell_order;
        const char *wall_line;
    } expected[] = {
        { DM1_VIEW_SQUARE_D0L2, DM1_VIEW_SQUARE_D0L, DM1_WALL_D0L, DM1_WALL_D0R,
          DM1_PC34_ZONE_WALL_D0L, DM1_PC34_ZONE_WALL_D3L, -2, 1, 8, 2, 4, 0x0002,
          "6432-6480" },
        { DM1_VIEW_SQUARE_D0R2, DM1_VIEW_SQUARE_D0R, DM1_WALL_D0R, DM1_WALL_D0L,
          DM1_PC34_ZONE_WALL_D0R, DM1_PC34_ZONE_WALL_D3R, 2, 2, 10, 3, 6, 0x0001,
          "6545-6600" },
    };

    check_int("pass760.count",
              (int)dm1_viewport_3d_d0l2_d0r2_f0108_composition_spec_count(), 2);
    check_int("pass760.out_of_range",
              dm1_viewport_3d_get_d0l2_d0r2_f0108_composition_spec(2) == NULL, 1);
    check_int("pass760.d0l_not_d0l",
              DM1_VIEW_SQUARE_D0L2 != DM1_VIEW_SQUARE_D0L, 1);
    check_int("pass760.d0r_not_d0r",
              DM1_VIEW_SQUARE_D0R2 != DM1_VIEW_SQUARE_D0R, 1);
    check_int("pass760.d0l_not_d0c",
              DM1_VIEW_SQUARE_D0L2 != DM1_VIEW_SQUARE_D0C, 1);
    check_int("pass760.d0r_not_d0c",
              DM1_VIEW_SQUARE_D0R2 != DM1_VIEW_SQUARE_D0C, 1);
    check_int("pass760.d0l_not_d1l2",
              DM1_VIEW_SQUARE_D0L2 != DM1_VIEW_SQUARE_D2L2, 1);
    check_int("pass760.d0r_not_d1r2",
              DM1_VIEW_SQUARE_D0R2 != DM1_VIEW_SQUARE_D2R2, 1);

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportD0L2D0R2F0108CompositionSpec *spec =
            dm1_viewport_3d_get_d0l2_d0r2_f0108_composition_spec_for_square(expected[i].square);
        const DM1_ViewportD0L2D0R2F0108CompositionSpec *by_index =
            dm1_viewport_3d_get_d0l2_d0r2_f0108_composition_spec(i);
        const DM1_ViewportWallDrawSpec *wall =
            dm1_viewport_3d_get_wall_draw_spec_for_square(expected[i].paired_wall_square);
        char id[160];

        snprintf(id, sizeof(id), "pass760.%zu.nonnull", i);
        check_nonnull(id, spec);
        snprintf(id, sizeof(id), "pass760.%zu.by_index", i);
        check_int(id, spec == by_index, 1);
        snprintf(id, sizeof(id), "pass760.%zu.wall_nonnull", i);
        check_nonnull(id, wall);
        if (!spec || !wall) continue;

        snprintf(id, sizeof(id), "pass760.%zu.square", i);
        check_int(id, spec->square, expected[i].square);
        snprintf(id, sizeof(id), "pass760.%zu.paired_wall_square", i);
        check_int(id, spec->paired_wall_square, expected[i].paired_wall_square);
        snprintf(id, sizeof(id), "pass760.%zu.native_wall", i);
        check_int(id, spec->native_wall, expected[i].native_wall);
        snprintf(id, sizeof(id), "pass760.%zu.parity_wall", i);
        check_int(id, spec->parity_wall, expected[i].parity_wall);
        snprintf(id, sizeof(id), "pass760.%zu.wall_spec_native", i);
        check_int(id, wall->native_wall, expected[i].native_wall);
        snprintf(id, sizeof(id), "pass760.%zu.wall_spec_parity", i);
        check_int(id, wall->parity_wall, expected[i].parity_wall);
        snprintf(id, sizeof(id), "pass760.%zu.pc34_wall_zone", i);
        check_int(id, spec->pc34_wall_zone, expected[i].pc34_wall_zone);
        snprintf(id, sizeof(id), "pass760.%zu.companion_d3_wall_zone", i);
        check_int(id, spec->companion_d3_wall_zone, expected[i].companion_d3_wall_zone);
        snprintf(id, sizeof(id), "pass760.%zu.rel_depth", i);
        check_int(id, spec->rel_depth, 0);
        snprintf(id, sizeof(id), "pass760.%zu.rel_lateral", i);
        check_int(id, spec->rel_lateral, expected[i].rel_lateral);
        snprintf(id, sizeof(id), "pass760.%zu.view_square_ordinal", i);
        check_int(id, spec->view_square_ordinal, expected[i].view_square_ordinal);
        snprintf(id, sizeof(id), "pass760.%zu.view_floor_ordinal", i);
        check_int(id, spec->view_floor_ordinal, expected[i].view_floor_ordinal);
        snprintf(id, sizeof(id), "pass760.%zu.view_wall_side_ordinal", i);
        check_int(id, spec->view_wall_side_ordinal, expected[i].view_wall_side_ordinal);
        snprintf(id, sizeof(id), "pass760.%zu.view_wall_front_ordinal", i);
        check_int(id, spec->view_wall_front_ordinal, expected[i].view_wall_front_ordinal);
        snprintf(id, sizeof(id), "pass760.%zu.open_cell_order", i);
        check_int(id, spec->open_cell_order, expected[i].open_cell_order);
        snprintf(id, sizeof(id), "pass760.%zu.wall_return_cell_order", i);
        check_int(id, spec->wall_return_cell_order, 0);
        snprintf(id, sizeof(id), "pass760.%zu.keepout_mask", i);
        check_int(id, spec->floor_ornament_keepout_mask, 0x8000);
        snprintf(id, sizeof(id), "pass760.%zu.floor_zone_base", i);
        check_int(id, spec->floor_ornament_zone_base, 1500);
        snprintf(id, sizeof(id), "pass760.%zu.floor_zone_stride", i);
        check_int(id, spec->floor_ornament_zone_stride, 11);
        snprintf(id, sizeof(id), "pass760.%zu.wall_composition_locked", i);
        check_int(id, spec->wall_composition_locked ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.floor_ceiling_base_locked", i);
        check_int(id, spec->floor_ceiling_base_locked ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.floor_ornament_locked", i);
        check_int(id, spec->floor_ornament_locked ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.ceiling_locked", i);
        check_int(id, spec->ceiling_locked ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.wall_ornament_locked", i);
        check_int(id, spec->wall_ornament_locked ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.wall_returns", i);
        check_int(id, spec->wall_returns_before_f0115 ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.open_floor_before_f0115", i);
        check_int(id, spec->open_path_floor_before_f0115 ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.c10", i);
        check_int(id, spec->c10_transparent_blit ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_pass729", i);
        check_int(id, spec->non_duplicate_pass729_d0l_d0r ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_pass733", i);
        check_int(id, spec->non_duplicate_pass733_d0c ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_pass717", i);
        check_int(id, spec->non_duplicate_pass717_d1l2_d1r2_wall ? 1 : 0, 1);
        snprintf(id, sizeof(id), "pass760.%zu.wall_line", i);
        check_int(id, strstr(spec->redmcsb_wall_source_lines, expected[i].wall_line) != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.wall_return_line", i);
        check_int(id, strstr(spec->redmcsb_wall_source_lines, i == 0 ? "8016-8038" : "8126-8144") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0108_lines", i);
        check_int(id, strstr(spec->redmcsb_f0108_source_lines, "3940-4011") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0108_mask", i);
        check_int(id, strstr(spec->redmcsb_f0108_source_lines, "MASK0x8000") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0107_lines", i);
        check_int(id, strstr(spec->redmcsb_f0107_source_lines, "3502-3938") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0107_coordinate", i);
        check_int(id, strstr(spec->redmcsb_f0107_source_lines, "coordinateSet") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0098_lines", i);
        check_int(id, strstr(spec->redmcsb_f0098_source_lines, "2962-3002") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.f0115_lines", i);
        check_int(id, strstr(spec->redmcsb_f0115_source_lines, "4547-4581") != NULL &&
                      strstr(spec->redmcsb_f0115_source_lines, "5180-5188") != NULL &&
                      strstr(spec->redmcsb_f0115_source_lines, "5211-5214") != NULL &&
                      strstr(spec->redmcsb_f0115_source_lines, "5668-5671") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.defs_lines", i);
        check_int(id, strstr(spec->redmcsb_defs_source_lines, "2088") != NULL &&
                      strstr(spec->redmcsb_defs_source_lines, "2596-2611") != NULL &&
                      strstr(spec->redmcsb_defs_source_lines, "2668-2677") != NULL &&
                      strstr(spec->redmcsb_defs_source_lines, "2698-2702") != NULL &&
                      strstr(spec->redmcsb_defs_source_lines, "4045-4046") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.drawview_lines", i);
        check_int(id, strstr(spec->redmcsb_drawview_source_lines, "F0097:1-50") != NULL &&
                      strstr(spec->redmcsb_drawview_source_lines, "F0104:3113-3156") != NULL &&
                      strstr(spec->redmcsb_drawview_source_lines, "F0105:3185-3247") != NULL, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_standalone_d0l_d0r", i);
        check_int(id, spec->square != spec->paired_wall_square, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_d0c", i);
        check_int(id, spec->paired_wall_square != DM1_VIEW_SQUARE_D0C, 1);
        snprintf(id, sizeof(id), "pass760.%zu.not_d1_side", i);
        check_int(id, spec->paired_wall_square != DM1_VIEW_SQUARE_D1L &&
                      spec->paired_wall_square != DM1_VIEW_SQUARE_D1R, 1);
    }
}


static void test_wall_source_row_clip_occlusion_gate(void)
{
    DM1_WallFrame frame = { 2, 5, 3, 6, 10, 8, 4, 1 };
    DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, 10, 8);
    check_int("wall_clip_gate.151713.visible", gate.visible ? 1 : 0, 1);
    check_int("wall_clip_gate.151713.src_x", gate.src_x, 4);
    check_int("wall_clip_gate.151713.src_y", gate.src_y, 1);
    check_int("wall_clip_gate.151713.dst_x", gate.dst_x, 2);
    check_int("wall_clip_gate.151713.dst_y", gate.dst_y, 3);
    check_int("wall_clip_gate.151713.width", gate.width, 4);
    check_int("wall_clip_gate.151713.height", gate.height, 4);
    check_int("wall_clip_gate.151713.source", strstr(gate.source_lines, "COORD.C:2390-2409") != NULL, 1);

    frame = (DM1_WallFrame){ 222, 230, 134, 140, 20, 20, 1, 2 };
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, 20, 20);
    check_int("wall_clip_gate.1337626.viewport_visible", gate.visible ? 1 : 0, 1);
    check_int("wall_clip_gate.1337626.dst_x", gate.dst_x, 222);
    check_int("wall_clip_gate.1337626.dst_y", gate.dst_y, 134);
    check_int("wall_clip_gate.1337626.width", gate.width, 2);
    check_int("wall_clip_gate.1337626.height", gate.height, 2);

    frame = (DM1_WallFrame){ 0, 9, 0, 9, 10, 8, 8, 7 };
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, 10, 8);
    check_int("wall_clip_gate.2098602.source_visible", gate.visible ? 1 : 0, 1);
    check_int("wall_clip_gate.2098602.width", gate.width, 2);
    check_int("wall_clip_gate.2098602.height", gate.height, 1);

    frame = (DM1_WallFrame){ 0, 3, 0, 3, 4, 4, 4, 0 };
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, 4, 4);
    check_int("wall_clip_gate.occluded_source_row", gate.visible ? 1 : 0, 0);

    frame = (DM1_WallFrame){ 224, 230, 0, 3, 8, 8, 0, 0 };
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, 8, 8);
    check_int("wall_clip_gate.occluded_viewport", gate.visible ? 1 : 0, 0);
}

static void test_wall_draw_uses_clip_gate_source_offsets(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[10 * 8];
    DM1_Viewport3DState state;
    DM1_WallFrame frame = { 2, 5, 3, 6, 10, 8, 4, 1 };
    memset(viewport, 0, sizeof(viewport));
    for (int i = 0; i < (int)sizeof(bitmap); ++i) bitmap[i] = (uint8_t)(i + 1);
    bitmap[1 * 10 + 4] = 10;
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall(&state, bitmap, &frame);
    check_int("wall_clip_draw.transparent_skips_first", viewport[3 * DM1_VIEWPORT_WIDTH + 2], 0);
    check_int("wall_clip_draw.source_offset_next", viewport[3 * DM1_VIEWPORT_WIDTH + 3], bitmap[1 * 10 + 5]);
    check_int("wall_clip_draw.source_offset_last_row", viewport[6 * DM1_VIEWPORT_WIDTH + 5], bitmap[4 * 10 + 7]);
    check_int("wall_clip_draw.outside_left_untouched", viewport[3 * DM1_VIEWPORT_WIDTH + 1], 0);

    memset(viewport, 0, sizeof(viewport));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall_opaque(&state, bitmap, &frame);
    check_int("wall_clip_draw.opaque_copies_transparent_color", viewport[3 * DM1_VIEWPORT_WIDTH + 2], 10);
}

static void test_f0099_copy_and_flip_h_preserves_row_boundaries(void)
{
    /*
     * ReDMCSB source-lock for the shared parity scratch path:
     *   - DUNVIEW.C:3018-3045 F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal
     *     copies a bitmap and flips it horizontally.  On PC34/I34E, line 3038
     *     delegates to F0655_CopyBitmapAndFlip(..., MASK0x0001_FLIP_HORIZONTAL).
     *   - DUNVIEW.C:3197-3204 F0105 uses F0099 into G0074_puc_Bitmap_Temporary
     *     before the transparent viewport blit.
     *
     * Existing D0/D2/D3 parity gates prove selected wall lanes.  This pins the
     * lower-level F0099 row-local contract those lanes share: each source row
     * is mirrored independently into the destination, with no carry across row
     * boundaries.
     */
    const uint8_t src[15] = {
        0x10, 0x11, 0x12, 0x13, 0x14,
        0x20, 0x21, 0x22, 0x23, 0x24,
        0x30, 0x31, 0x32, 0x33, 0x34
    };
    const uint8_t src_before[15] = {
        0x10, 0x11, 0x12, 0x13, 0x14,
        0x20, 0x21, 0x22, 0x23, 0x24,
        0x30, 0x31, 0x32, 0x33, 0x34
    };
    uint8_t dst[15];

    memset(dst, 0xee, sizeof(dst));
    dm1_viewport_3d_copy_and_flip_h(src, dst, 5, 3);

    check_int("F0099.copy_flip.row0.left", dst[0], 0x14);
    check_int("F0099.copy_flip.row0.center", dst[2], 0x12);
    check_int("F0099.copy_flip.row0.right", dst[4], 0x10);
    check_int("F0099.copy_flip.row1.left", dst[5], 0x24);
    check_int("F0099.copy_flip.row1.right", dst[9], 0x20);
    check_int("F0099.copy_flip.row2.left", dst[10], 0x34);
    check_int("F0099.copy_flip.row2.right", dst[14], 0x30);
    check_int("F0099.copy_flip.source_preserved", memcmp(src, src_before, sizeof(src)) == 0, 1);
}

static void test_d3c_far_center_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[64 * 51];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3C);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 583 gives D3C as
     * {74,149,25,75,64,51,18,0}; F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
     * lines 6697-6720 draws the wall branch, while F0100 lines 3048-3058
     * forwards the frame to F0132 with C10 transparency.  COORD.C
     * lines 2390-2409 and IMAGE3.C lines 866-889 then clip and copy the
     * resolved source row.
     *
     * D3C is the ordinary far center wall, separate from the D3L2/D3R2
     * far-side strips.  Its source row starts at blit_x=18 in a 64-wide
     * bitmap, so only source columns 18..63 reach viewport x=74..119.
     * This pins that small centered far-wall slice without depending on
     * real wall-set assets.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d3c_far_center_wall_pixel.frame", frame);
    if (!frame) return;

    bitmap[0 * 64 + 17] = 0x33;
    bitmap[0 * 64 + 18] = 10;
    bitmap[0 * 64 + 19] = 0x42;
    bitmap[1 * 64 + 18] = 0x44;
    bitmap[0 * 64 + 63] = 0x7e;
    bitmap[50 * 64 + 63] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d3c_far_center_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d3c_far_center_wall_pixel.src_x", gate.src_x, 18);
    check_int("d3c_far_center_wall_pixel.src_y", gate.src_y, 0);
    check_int("d3c_far_center_wall_pixel.dst_x", gate.dst_x, 74);
    check_int("d3c_far_center_wall_pixel.dst_y", gate.dst_y, 25);
    check_int("d3c_far_center_wall_pixel.visible_width", gate.width, 46);
    check_int("d3c_far_center_wall_pixel.visible_height", gate.height, 51);
    check_int("d3c_far_center_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    check_int("d3c_far_center_wall_pixel.transparent_first_visible_skip",
              viewport[25 * DM1_VIEWPORT_WIDTH + 74], 0xee);
    check_int("d3c_far_center_wall_pixel.next_source_pixel_copied",
              viewport[25 * DM1_VIEWPORT_WIDTH + 75], 0x42);
    check_int("d3c_far_center_wall_pixel.left_edge_next_row_copied",
              viewport[26 * DM1_VIEWPORT_WIDTH + 74], 0x44);
    check_int("d3c_far_center_wall_pixel.column_before_dst_x_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 73], 0xee);
    check_int("d3c_far_center_wall_pixel.pre_blit_source_pixel_not_copied",
              viewport[25 * DM1_VIEWPORT_WIDTH + 74] != 0x33, 1);
    check_int("d3c_far_center_wall_pixel.last_visible_pixel_copied",
              viewport[25 * DM1_VIEWPORT_WIDTH + 119], 0x7e);
    check_int("d3c_far_center_wall_pixel.clipped_viewport_column_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 120], 0xee);
    check_int("d3c_far_center_wall_pixel.bottom_row_marker",
              viewport[75 * DM1_VIEWPORT_WIDTH + 119], 0x55);
    check_int("d3c_far_center_wall_pixel.row_before_frame_untouched",
              viewport[24 * DM1_VIEWPORT_WIDTH + 75], 0xee);
    check_int("d3c_far_center_wall_pixel.row_after_frame_untouched",
              viewport[76 * DM1_VIEWPORT_WIDTH + 74], 0xee);
}

static void test_d3l_d3r_far_side_wall_pixel_routes_use_redmcsb_frame_clip(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        const char *id;
        const char *function_name;
        const char *function_source;
        const char *return_source;
        int zone;
        int src_x;
        int dst_x;
        int visible_width;
        int before_x;
        int after_x;
        uint8_t next_pixel;
        uint8_t row_pixel;
        uint8_t edge_pixel;
        uint8_t bottom_pixel;
    } cases[] = {
        { DM1_VIEW_SQUARE_D3L, "d3l", "F0116_DUNGEONVIEW_DrawSquareD3L",
          "DUNVIEW.C:6421-6427", "DUNVIEW.C:6432-6437",
          DM1_PC34_ZONE_WALL_D3L, 32,   0, 32, -1,  32, 0x42, 0x44, 0x7e, 0x55 },
        { DM1_VIEW_SQUARE_D3R, "d3r", "F0117_DUNGEONVIEW_DrawSquareD3R",
          "DUNVIEW.C:6554-6564", "DUNVIEW.C:6568-6573",
          DM1_PC34_ZONE_WALL_D3R,  0, 139, 64, 138, 203, 0x52, 0x54, 0x5e, 0x56 },
    };
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[64 * 51];
    DM1_Viewport3DState state;

    /*
     * ReDMCSB source-lock for the ordinary D3 far side wall routes:
     *   - DUNVIEW.C:584-585 G0163 defines D3L as {0,83,25,75,64,51,32,0}
     *     and D3R as {139,223,25,75,64,51,0,0}.
     *   - DUNVIEW.C:6406-6408 F0116 and 6545-6547 F0117 route WALL
     *     through F0100_DUNGEONVIEW_DrawWallSetBitmap, then return at
     *     6432-6437 / 6568-6573 unless a front alcove reveals contents.
     *   - DUNVIEW.C:3053-3058 F0100 forwards the frame to F0132 with
     *     C10_COLOR_FLESH transparency; COORD.C:2390-2409 and
     *     IMAGE3.C:866-889 clip the source row before copying pixels.
     *
     * This deliberately covers D3L/D3R only.  D3C, D3L2/D3R2, D2*, D1*,
     * and D0* have separate gates in this file.
     */
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(cases[i].square);
        const DM1_ViewportWallDrawSpec *spec =
            dm1_viewport_3d_get_wall_draw_spec_for_square(cases[i].square);
        DM1_ViewportBlitClipGate gate;
        char check[128];

        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.frame", cases[i].id);
        check_nonnull(check, frame);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.spec", cases[i].id);
        check_nonnull(check, spec);
        if (!frame || !spec) continue;

        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.function", cases[i].id);
        check_int(check, strcmp(spec->redmcsb_function, cases[i].function_name) == 0, 1);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.pc34_source", cases[i].id);
        check_int(check, strstr(spec->source_lines, cases[i].function_source) != NULL, 1);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.return_source", cases[i].id);
        check_int(check, strstr(spec->occlusion_source_lines, cases[i].return_source) != NULL, 1);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.zone", cases[i].id);
        check_int(check, spec->pc34_zone, cases[i].zone);

        gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.visible", cases[i].id);
        check_int(check, gate.visible ? 1 : 0, 1);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.src_x", cases[i].id);
        check_int(check, gate.src_x, cases[i].src_x);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.dst_x", cases[i].id);
        check_int(check, gate.dst_x, cases[i].dst_x);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.dst_y", cases[i].id);
        check_int(check, gate.dst_y, 25);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.width", cases[i].id);
        check_int(check, gate.width, cases[i].visible_width);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.height", cases[i].id);
        check_int(check, gate.height, 51);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.source_evidence", cases[i].id);
        check_int(check,
                  strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
                  strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
                  strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

        memset(viewport, 0xee, sizeof(viewport));
        memset(bitmap, 10, sizeof(bitmap));
        bitmap[0 * 64 + cases[i].src_x] = 10;
        bitmap[0 * 64 + cases[i].src_x + 1] = cases[i].next_pixel;
        bitmap[1 * 64 + cases[i].src_x] = cases[i].row_pixel;
        bitmap[0 * 64 + cases[i].src_x + cases[i].visible_width - 1] = cases[i].edge_pixel;
        bitmap[50 * 64 + cases[i].src_x + cases[i].visible_width - 1] = cases[i].bottom_pixel;
        if (cases[i].src_x > 0) {
            bitmap[0 * 64 + cases[i].src_x - 1] = 0x33;
        }

        dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
        dm1_viewport_3d_draw_wall(&state, bitmap, frame);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.transparent_first_visible_skip", cases[i].id);
        check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].dst_x], 0xee);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.next_source_pixel_copied", cases[i].id);
        check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].dst_x + 1], cases[i].next_pixel);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.next_row_left_edge_copied", cases[i].id);
        check_int(check, viewport[26 * DM1_VIEWPORT_WIDTH + cases[i].dst_x], cases[i].row_pixel);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.last_visible_pixel_copied", cases[i].id);
        check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].dst_x + cases[i].visible_width - 1], cases[i].edge_pixel);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.bottom_row_marker", cases[i].id);
        check_int(check, viewport[75 * DM1_VIEWPORT_WIDTH + cases[i].dst_x + cases[i].visible_width - 1], cases[i].bottom_pixel);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.row_before_frame_untouched", cases[i].id);
        check_int(check, viewport[24 * DM1_VIEWPORT_WIDTH + cases[i].dst_x], 0xee);
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.row_after_frame_untouched", cases[i].id);
        check_int(check, viewport[76 * DM1_VIEWPORT_WIDTH + cases[i].dst_x], 0xee);
        if (cases[i].before_x >= 0) {
            snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.left_neighbor_untouched", cases[i].id);
            check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].before_x], 0xee);
        } else {
            snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.pre_blit_source_pixel_not_copied", cases[i].id);
            check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].dst_x] != 0x33, 1);
        }
        snprintf(check, sizeof(check), "d3_far_side_wall_pixel.%s.after_source_clip_untouched", cases[i].id);
        check_int(check, viewport[25 * DM1_VIEWPORT_WIDTH + cases[i].after_x], 0xee);
    }
}

static void test_d2l_side_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[72 * 71];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2L);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 586 gives D2L as
     * {0,74,20,90,72,71,61,0}; F0100 lines 3048-3058 forwards that
     * frame to F0132 with C10 transparency, and COORD.C:2390-2409 /
     * IMAGE3.C:866-889 clip the source row before copying pixels.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d2l_side_wall_pixel.frame", frame);
    if (!frame) return;

    bitmap[0 * 72 + 60] = 0x33;
    bitmap[0 * 72 + 61] = 10;
    bitmap[0 * 72 + 62] = 0x42;
    bitmap[70 * 72 + 71] = 0x7e;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d2l_side_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d2l_side_wall_pixel.src_x", gate.src_x, 61);
    check_int("d2l_side_wall_pixel.dst_x", gate.dst_x, 0);
    check_int("d2l_side_wall_pixel.visible_width", gate.width, 11);
    check_int("d2l_side_wall_pixel.visible_height", gate.height, 71);
    check_int("d2l_side_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    check_int("d2l_side_wall_pixel.transparent_first_visible_skip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d2l_side_wall_pixel.next_source_pixel_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 1], 0x42);
    check_int("d2l_side_wall_pixel.left_source_before_blit_x_not_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0] != 0x33, 1);
    check_int("d2l_side_wall_pixel.last_visible_pixel_copied",
              viewport[90 * DM1_VIEWPORT_WIDTH + 10], 0x7e);
    check_int("d2l_side_wall_pixel.clipped_viewport_column_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 11], 0xee);
    check_int("d2l_side_wall_pixel.row_before_frame_untouched",
              viewport[19 * DM1_VIEWPORT_WIDTH + 1], 0xee);
}

static void test_d2r_right_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[72 * 71];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2R);
    const DM1_ViewportWallDrawSpec *spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2R);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 588 gives D2R as
     * {149,223,20,90,72,71,0,0}.  F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
     * lines 7097-7115 draws the right-side wall through the wall-set route
     * and lines 7119-7166 handle alcove/return ordering.  F0100 lines
     * 3048-3058 forwards the frame to F0132 with C10 transparency; COORD.C
     * lines 2390-2409 and IMAGE3.C lines 866-889 clip/copy the source row.
     *
     * The D2R frame is 75 viewport columns wide, but the source bitmap is
     * only 72 columns.  This gate pins the C10 skip at viewport x=149, the
     * last source column at x=220, and the untouched frame tail at x=221.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d2r_right_wall_pixel.frame", frame);
    check_nonnull("d2r_right_wall_pixel.spec", spec);
    if (!frame || !spec) return;

    bitmap[0 * 72 + 0] = 10;
    bitmap[0 * 72 + 1] = 0x42;
    bitmap[1 * 72 + 0] = 0x44;
    bitmap[0 * 72 + 71] = 0x7e;
    bitmap[70 * 72 + 71] = 0x55;

    check_int("d2r_right_wall_pixel.function",
              strcmp(spec->redmcsb_function, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF") == 0, 1);
    check_int("d2r_right_wall_pixel.source_route",
              strstr(spec->source_lines, "DUNVIEW.C:7105-7115") != NULL, 1);
    check_int("d2r_right_wall_pixel.return_route",
              strstr(spec->occlusion_source_lines, "DUNVIEW.C:7119-7123") != NULL &&
              strstr(spec->occlusion_source_lines, "DUNVIEW.C:7166") != NULL, 1);

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d2r_right_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d2r_right_wall_pixel.src_x", gate.src_x, 0);
    check_int("d2r_right_wall_pixel.src_y", gate.src_y, 0);
    check_int("d2r_right_wall_pixel.dst_x", gate.dst_x, 149);
    check_int("d2r_right_wall_pixel.dst_y", gate.dst_y, 20);
    check_int("d2r_right_wall_pixel.visible_width", gate.width, 72);
    check_int("d2r_right_wall_pixel.visible_height", gate.height, 71);
    check_int("d2r_right_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    check_int("d2r_right_wall_pixel.transparent_first_visible_skip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 149], 0xee);
    check_int("d2r_right_wall_pixel.next_source_pixel_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 150], 0x42);
    check_int("d2r_right_wall_pixel.left_edge_next_row_copied",
              viewport[21 * DM1_VIEWPORT_WIDTH + 149], 0x44);
    check_int("d2r_right_wall_pixel.column_before_dst_x_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 148], 0xee);
    check_int("d2r_right_wall_pixel.last_visible_pixel_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 220], 0x7e);
    check_int("d2r_right_wall_pixel.frame_tail_after_source_clip_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 221], 0xee);
    check_int("d2r_right_wall_pixel.bottom_row_marker",
              viewport[90 * DM1_VIEWPORT_WIDTH + 220], 0x55);
    check_int("d2r_right_wall_pixel.row_before_frame_untouched",
              viewport[19 * DM1_VIEWPORT_WIDTH + 150], 0xee);
    check_int("d2r_right_wall_pixel.row_after_frame_untouched",
              viewport[91 * DM1_VIEWPORT_WIDTH + 149], 0xee);
}

static void test_d2c_center_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[72 * 71];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2C);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 586 gives D2C as
     * {60,163,20,90,72,71,16,0}; F0121 lines 7289-7312 draws the
     * center wall and F0100 lines 3048-3058 forwards that frame to
     * F0132 with C10 transparency.  COORD.C:2390-2409 /
     * IMAGE3.C:866-889 then clip the source row before copying pixels.
     *
     * D2C is the middle wall one tile beyond the nearest center wall.
     * Its frame is wider than the visible span from blit_x=16 in a
     * 72-wide source row, so this gate pins the left source clip,
     * transparent first visible pixel, right visible edge, and rows
     * immediately outside the frame.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d2c_center_wall_pixel.frame", frame);
    if (!frame) return;

    /* Source column 15 is left of the resolved src_x=16 and must not copy. */
    bitmap[0 * 72 + 15] = 0x33;
    /* First visible column 16 holds the C10 transparency sentinel. */
    bitmap[0 * 72 + 16] = 10;
    /* First opaque visible column 17. */
    bitmap[0 * 72 + 17] = 0x42;
    /* Left edge on the next source row proves src_x=16 remains visible. */
    bitmap[1 * 72 + 16] = 0x44;
    /* Last visible column 71 of the resolved 56-wide source span. */
    bitmap[0 * 72 + 71] = 0x7e;
    /* Bottom row marker on the right visible edge. */
    bitmap[70 * 72 + 71] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d2c_center_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d2c_center_wall_pixel.src_x", gate.src_x, 16);
    check_int("d2c_center_wall_pixel.src_y", gate.src_y, 0);
    check_int("d2c_center_wall_pixel.dst_x", gate.dst_x, 60);
    check_int("d2c_center_wall_pixel.dst_y", gate.dst_y, 20);
    check_int("d2c_center_wall_pixel.visible_width", gate.width, 56);
    check_int("d2c_center_wall_pixel.visible_height", gate.height, 71);
    check_int("d2c_center_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    check_int("d2c_center_wall_pixel.transparent_first_visible_skip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 60], 0xee);
    check_int("d2c_center_wall_pixel.next_source_pixel_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 61], 0x42);
    check_int("d2c_center_wall_pixel.left_edge_next_row_copied",
              viewport[21 * DM1_VIEWPORT_WIDTH + 60], 0x44);
    check_int("d2c_center_wall_pixel.column_before_dst_x_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 59], 0xee);
    check_int("d2c_center_wall_pixel.pre_blit_source_pixel_not_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 60] != 0x33, 1);
    check_int("d2c_center_wall_pixel.last_visible_pixel_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 115], 0x7e);
    check_int("d2c_center_wall_pixel.clipped_viewport_column_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 116], 0xee);
    check_int("d2c_center_wall_pixel.bottom_row_marker",
              viewport[90 * DM1_VIEWPORT_WIDTH + 115], 0x55);
    check_int("d2c_center_wall_pixel.row_before_frame_untouched",
              viewport[19 * DM1_VIEWPORT_WIDTH + 61], 0xee);
    check_int("d2c_center_wall_pixel.row_after_frame_untouched",
              viewport[91 * DM1_VIEWPORT_WIDTH + 60], 0xee);
}

static void verify_center_wall_opaque_pixel_slice(DM1_ViewSquareIndex square,
                                                  const char *name,
                                                  const char *source_anchor,
                                                  const char *occlusion_anchor,
                                                  int expected_zone)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(square);
    const DM1_ViewportWallDrawSpec *spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(square);
    DM1_ViewportBlitClipGate gate;
    char id[128];

    /*
     * ReDMCSB PC34/I34E source-lock for center-wall opaque routes:
     *   - DUNVIEW.C:581-594 G0163 gives D3C/D2C/D1C frame descriptors.
     *   - DUNVIEW.C:6707-6714,7299-7306,7833-7840 route PC34 center
     *     walls through F0792/F0765 with the corresponding G2107[] entry.
     *   - DUNVIEW.C:3065-3078 F0101 documents the center-wall optimization:
     *     CM1_COLOR_NO_TRANSPARENCY means C10 source pixels are copied.
     *   - COORD.C:2390-2409 and IMAGE3.C:866-889 still own the clipped
     *     source/destination span before the no-transparency copy runs.
     *
     * The transparent center-wall slices pin the shared C10-skip helper.
     * This gate pins the PC34 center-wall no-transparency behavior across
     * D3C/D2C/D1C: the first visible source pixel is C10 and must overwrite
     * the viewport for every centered wall depth.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 0x44, sizeof(bitmap));
    snprintf(id, sizeof(id), "%s_center_wall_opaque.frame", name);
    check_nonnull(id, frame);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.spec", name);
    check_nonnull(id, spec);
    if (!frame || !spec) return;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.center_wall", name);
    check_int(id, spec->center_wall ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.pc34_source", name);
    check_int(id, strstr(spec->source_lines, source_anchor) != NULL, 1);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.wall_return", name);
    check_int(id, strstr(spec->occlusion_source_lines, occlusion_anchor) != NULL, 1);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.zone", name);
    check_int(id, spec->pc34_zone, expected_zone);

    snprintf(id, sizeof(id), "%s_center_wall_opaque.gate_visible", name);
    check_int(id, gate.visible ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.src_x", name);
    check_int(id, gate.src_x, frame->blit_x);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.dst_x", name);
    check_int(id, gate.dst_x, frame->left_x);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.visible_width", name);
    check_int(id, gate.width, frame->byte_width - frame->blit_x);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.visible_height", name);
    check_int(id, gate.height, frame->height);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.source_evidence", name);
    check_int(id,
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    if (gate.src_x > 0) {
        bitmap[gate.src_y * frame->byte_width + gate.src_x - 1] = 0x33;
    }
    bitmap[gate.src_y * frame->byte_width + gate.src_x] = 10;
    bitmap[gate.src_y * frame->byte_width + gate.src_x + 1] = 0x42;
    bitmap[(gate.src_y + gate.height - 1) * frame->byte_width +
           gate.src_x + gate.width - 1] = 0x55;

    dm1_viewport_3d_draw_wall_opaque(&state, bitmap, frame);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.c10_copied", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x], 10);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.next_source_pixel_copied", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + 1], 0x42);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.pre_blit_source_not_copied", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x] != 0x33, 1);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.column_before_dst_x_untouched", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x - 1], 0xee);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.column_after_source_clip_untouched", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width], 0xee);
    snprintf(id, sizeof(id), "%s_center_wall_opaque.bottom_row_marker", name);
    check_int(id,
              viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH +
                       gate.dst_x + gate.width - 1],
              0x55);
}

static void test_center_wall_opaque_pixel_slices_use_pc34_no_transparency_route(void)
{
    verify_center_wall_opaque_pixel_slice(DM1_VIEW_SQUARE_D3C, "d3c",
                                          "DUNVIEW.C:6707-6714",
                                          "DUNVIEW.C:6716-6720",
                                          DM1_PC34_ZONE_WALL_D3C);
    verify_center_wall_opaque_pixel_slice(DM1_VIEW_SQUARE_D2C, "d2c",
                                          "DUNVIEW.C:7299-7306",
                                          "DUNVIEW.C:7308-7312",
                                          DM1_PC34_ZONE_WALL_D2C);
    verify_center_wall_opaque_pixel_slice(DM1_VIEW_SQUARE_D1C, "d1c",
                                          "DUNVIEW.C:7833-7840",
                                          "DUNVIEW.C:7842-7843",
                                          DM1_PC34_ZONE_WALL_D1C);
}

static void test_d2c_closed_door_panel_uses_temp_bitmap_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t door_bitmap[32 * 61];
    DM1_Viewport3DState state;
    DM1_WallFrame frame = { 80, 143, 24, 82, 32, 61, 0, 0 };
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB source-lock for the D2C closed-door panel:
     *   - DUNVIEW.C:658-668 G0183_s_Graphic558_Frames_Door_D2C gives
     *     ClosedOrDestroyed as {80,143,24,82,32,61,0,0}.
     *   - DUNVIEW.C:7336 F0121_DUNGEONVIEW_DrawSquareD2C routes a
     *     front door through F0111 with &G0183_s_Graphic558_Frames_Door_D2C.
     *   - DUNVIEW.C:4229-4297 F0111 copies the native door bitmap into
     *     G0074_puc_Bitmap_Temporary, applies ornaments, and for a closed
     *     door calls F0102 with ClosedOrDestroyed.
     *   - DUNVIEW.C:3082-3093 F0102 blits G0074_puc_Bitmap_Temporary to
     *     G0296_puc_Bitmap_Viewport with C10 transparency; COORD.C:2390-2409
     *     and IMAGE3.C:866-889 own the resolved viewport/source clip.
     *
     * This is a door-panel gate, not another wall-set lane.  It pins the
     * temporary-bitmap source, C10 skip, and D2C closed-door frame clip.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(door_bitmap, 10, sizeof(door_bitmap));
    door_bitmap[0 * 32 + 0] = 10;
    door_bitmap[0 * 32 + 1] = 0x42;
    door_bitmap[1 * 32 + 0] = 0x44;
    door_bitmap[0 * 32 + 31] = 0x7e;
    door_bitmap[58 * 32 + 31] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    state.temp_bitmap = door_bitmap;
    state.temp_bitmap_size = (int)sizeof(door_bitmap);

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(&frame, frame.byte_width, frame.height);
    check_int("d2c_closed_door_panel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d2c_closed_door_panel.src_x", gate.src_x, 0);
    check_int("d2c_closed_door_panel.src_y", gate.src_y, 0);
    check_int("d2c_closed_door_panel.dst_x", gate.dst_x, 80);
    check_int("d2c_closed_door_panel.dst_y", gate.dst_y, 24);
    check_int("d2c_closed_door_panel.visible_width", gate.width, 32);
    check_int("d2c_closed_door_panel.visible_height", gate.height, 59);
    check_int("d2c_closed_door_panel.source_evidence",
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_door(&state, &frame);
    check_int("d2c_closed_door_panel.transparent_first_visible_skip",
              viewport[24 * DM1_VIEWPORT_WIDTH + 80], 0xee);
    check_int("d2c_closed_door_panel.next_source_pixel_copied",
              viewport[24 * DM1_VIEWPORT_WIDTH + 81], 0x42);
    check_int("d2c_closed_door_panel.left_edge_next_row_copied",
              viewport[25 * DM1_VIEWPORT_WIDTH + 80], 0x44);
    check_int("d2c_closed_door_panel.column_before_dst_x_untouched",
              viewport[24 * DM1_VIEWPORT_WIDTH + 79], 0xee);
    check_int("d2c_closed_door_panel.last_visible_pixel_copied",
              viewport[24 * DM1_VIEWPORT_WIDTH + 111], 0x7e);
    check_int("d2c_closed_door_panel.frame_tail_after_source_clip_untouched",
              viewport[24 * DM1_VIEWPORT_WIDTH + 112], 0xee);
    check_int("d2c_closed_door_panel.bottom_row_marker",
              viewport[82 * DM1_VIEWPORT_WIDTH + 111], 0x55);
    check_int("d2c_closed_door_panel.row_before_frame_untouched",
              viewport[23 * DM1_VIEWPORT_WIDTH + 81], 0xee);
    check_int("d2c_closed_door_panel.row_after_frame_untouched",
              viewport[83 * DM1_VIEWPORT_WIDTH + 80], 0xee);
}

static void test_d2l2_d2r2_near_wall_pixel_and_no_thing_gate(void)
{
    /*
     * ReDMCSB source-lock for the MEDIA720 near-side lane:
     *   - DUNVIEW.C:581-594 G0163 covers the ordinary D2L/D2R/D2C wall
     *     frames; D2L2/D2R2 are not G0163 wall-frame entries.
     *   - DUNVIEW.C:6837-6865 F0678 and 6868-6896 F0679 route only WALL
     *     and TELEPORTER for D2L2/D2R2.  WALL uses C707/C708 through
     *     F0104/F0105 and returns before any F0115 thing pass.
     *   - DUNVIEW.C:3048-3058 F0100, 3113-3129 F0104, and 3185-3204 F0105
     *     all land in the same transparent blit clipping path, with
     *     COORD.C:2390-2409 / IMAGE3.C:866-889 rejecting empty source or
     *     viewport intersections before pixels are copied.
     */
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t assets[32 * DM1_VIEWPORT_BYTE_WIDTH];
    uint8_t grid[4 * 4];
    DM1_Viewport3DState state;
    const DM1_WallFrame *d2l2_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2L2);
    const DM1_WallFrame *d2r2_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2R2);
    const DM1_ViewportWallDrawSpec *d2l2_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2L2);
    const DM1_ViewportWallDrawSpec *d2r2_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2R2);
    const DM1_ViewportFloorFieldOrderSpec *d2l2_order =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D2L2);
    const DM1_ViewportFloorFieldOrderSpec *d2r2_order =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D2R2);
    uint8_t *d2l2_bitmap;
    uint8_t *d2r2_bitmap;

    memset(viewport, 0xee, sizeof(viewport));
    memset(assets, 10, sizeof(assets));
    memset(grid, DM1_VP_ELEMENT_WALL, sizeof(grid));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    state.dungeon_grid = grid;
    state.dungeon_width = 4;
    state.dungeon_height = 4;
    state.wall_set_native[DM1_WALL_D2L2] = 0;
    state.wall_set_native[DM1_WALL_D2R2] = 14;
    d2l2_bitmap = assets + state.wall_set_native[DM1_WALL_D2L2] * DM1_VIEWPORT_BYTE_WIDTH;
    d2r2_bitmap = assets + state.wall_set_native[DM1_WALL_D2R2] * DM1_VIEWPORT_BYTE_WIDTH;

    check_nonnull("d2l2_d2r2_gate.frame.d2l2", d2l2_frame);
    check_nonnull("d2l2_d2r2_gate.frame.d2r2", d2r2_frame);
    check_nonnull("d2l2_d2r2_gate.spec.d2l2", d2l2_spec);
    check_nonnull("d2l2_d2r2_gate.spec.d2r2", d2r2_spec);
    check_nonnull("d2l2_d2r2_gate.order.d2l2", d2l2_order);
    check_nonnull("d2l2_d2r2_gate.order.d2r2", d2r2_order);
    if (!d2l2_frame || !d2r2_frame || !d2l2_spec || !d2r2_spec ||
        !d2l2_order || !d2r2_order) {
        return;
    }

    check_int("d2l2_d2r2_gate.d2l2_src_x", d2l2_frame->blit_x, 30);
    check_int("d2l2_d2r2_gate.d2l2_visible_width",
              dm1_viewport_3d_resolve_wall_blit_clip_gate(
                  d2l2_frame, d2l2_frame->byte_width, d2l2_frame->height).width,
              6);
    check_int("d2l2_d2r2_gate.d2r2_visible_width",
              dm1_viewport_3d_resolve_wall_blit_clip_gate(
                  d2r2_frame, d2r2_frame->byte_width, d2r2_frame->height).width,
              36);
    check_int("d2l2_d2r2_gate.d2l2_zone", d2l2_spec->pc34_zone, DM1_PC34_ZONE_WALL_D2L2);
    check_int("d2l2_d2r2_gate.d2r2_zone", d2r2_spec->pc34_zone, DM1_PC34_ZONE_WALL_D2R2);
    check_int("d2l2_d2r2_gate.d2l2_wall_source",
              strstr(d2l2_spec->source_lines, "DUNVIEW.C:6849-6858") != NULL &&
              strstr(d2l2_spec->occlusion_source_lines, "DUNVIEW.C:6848-6862") != NULL,
              1);
    check_int("d2l2_d2r2_gate.d2r2_wall_source",
              strstr(d2r2_spec->source_lines, "DUNVIEW.C:6880-6889") != NULL &&
              strstr(d2r2_spec->occlusion_source_lines, "DUNVIEW.C:6882-6893") != NULL,
              1);
    check_int("d2l2_d2r2_gate.no_f0115_d2l2",
              !d2l2_order->objects_creatures_projectiles_before_explosions &&
              strstr(d2l2_order->things_source_lines, "no F0115 thing pass") != NULL,
              1);
    check_int("d2l2_d2r2_gate.no_f0115_d2r2",
              !d2r2_order->objects_creatures_projectiles_before_explosions &&
              strstr(d2r2_order->things_source_lines, "no F0115 thing pass") != NULL,
              1);

    dm1_viewport_3d_set_wall_frame_bitmaps(assets);

    /* D2L2 native: F0678 chooses C06_WALL_D2L2 and C707_ZONE_WALL_D2L2.
     * The D2L2 source starts at x=30, so only source columns 30..35 can
     * reach viewport x=0..5; the rest of the frame's 38 columns are clipped. */
    d2l2_bitmap[0 * 36 + 29] = 0x31;
    d2l2_bitmap[0 * 36 + 30] = 10;
    d2l2_bitmap[0 * 36 + 31] = 0x42;
    d2l2_bitmap[0 * 36 + 35] = 0x7e;
    d2l2_bitmap[70 * 36 + 35] = 0x55;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.d2l2_native_transparent_skip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d2l2_d2r2_gate.d2l2_native_next_pixel",
              viewport[20 * DM1_VIEWPORT_WIDTH + 1], 0x42);
    check_int("d2l2_d2r2_gate.d2l2_native_pre_src_not_copied",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0] != 0x31, 1);
    check_int("d2l2_d2r2_gate.d2l2_native_right_edge",
              viewport[20 * DM1_VIEWPORT_WIDTH + 5], 0x7e);
    check_int("d2l2_d2r2_gate.d2l2_native_column_after_clip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 6], 0xee);
    check_int("d2l2_d2r2_gate.d2l2_native_bottom_edge",
              viewport[90 * DM1_VIEWPORT_WIDTH + 5], 0x55);

    /* Raw DUNGEON.DAT bytes use M034_SQUARE_TYPE(raw >> 5).  A wall with
     * MASK0x0010_THING_LIST_PRESENT must still draw as a wall, while a raw
     * corridor byte (0x20) must not alias to C00_ELEMENT_WALL. */
    memset(viewport, 0xee, sizeof(viewport));
    grid[1 * 4 + 1] = 0x10;
    d2l2_bitmap[0 * 36 + 31] = 0x44;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.raw_wall_thing_list_draws",
              viewport[20 * DM1_VIEWPORT_WIDTH + 1], 0x44);
    memset(viewport, 0xee, sizeof(viewport));
    grid[1 * 4 + 1] = 0x20;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.raw_corridor_does_not_draw_wall",
              viewport[20 * DM1_VIEWPORT_WIDTH + 1], 0xee);
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_WALL;

    /* D2R2 native: F0679 chooses C05_WALL_D2R2 and C708_ZONE_WALL_D2R2. */
    memset(viewport, 0xee, sizeof(viewport));
    d2r2_bitmap[0 * 36 + 0] = 10;
    d2r2_bitmap[0 * 36 + 1] = 0x52;
    d2r2_bitmap[0 * 36 + 35] = 0x5e;
    d2r2_bitmap[70 * 36 + 35] = 0x56;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2R2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.d2r2_native_transparent_skip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 186], 0xee);
    check_int("d2l2_d2r2_gate.d2r2_native_next_pixel",
              viewport[20 * DM1_VIEWPORT_WIDTH + 187], 0x52);
    check_int("d2l2_d2r2_gate.d2r2_native_right_edge",
              viewport[20 * DM1_VIEWPORT_WIDTH + 221], 0x5e);
    check_int("d2l2_d2r2_gate.d2r2_native_column_after_clip",
              viewport[20 * DM1_VIEWPORT_WIDTH + 222], 0xee);
    check_int("d2l2_d2r2_gate.d2r2_native_bottom_edge",
              viewport[90 * DM1_VIEWPORT_WIDTH + 221], 0x56);

    /* D2L2 parity: F0678 chooses C05_WALL_D2R2, flips it, and still writes
     * only the C707 clipped D2L2 strip. */
    memset(viewport, 0xee, sizeof(viewport));
    d2r2_bitmap[0 * 36 + 5] = 10;
    d2r2_bitmap[0 * 36 + 4] = 0x63;
    d2r2_bitmap[0 * 36 + 0] = 0x6e;
    state.parity_flip = true;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.d2l2_parity_uses_d2r2_transparent",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d2l2_d2r2_gate.d2l2_parity_flipped_next",
              viewport[20 * DM1_VIEWPORT_WIDTH + 1], 0x63);
    check_int("d2l2_d2r2_gate.d2l2_parity_flipped_right_edge",
              viewport[20 * DM1_VIEWPORT_WIDTH + 5], 0x6e);
    check_int("d2l2_d2r2_gate.d2l2_parity_d2r2_zone_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 186], 0xee);

    /* Non-wall/non-teleporter elements are explicit no-ops for F0678/F0679:
     * no pit, stairs, floor ornament, object, creature, projectile, or
     * explosion pass is reachable before the helper returns. */
    memset(viewport, 0xee, sizeof(viewport));
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_PIT;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2R2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.pit_no_write_left",
              viewport[20 * DM1_VIEWPORT_WIDTH + 186], 0xee);
    check_int("d2l2_d2r2_gate.pit_no_write_right",
              viewport[20 * DM1_VIEWPORT_WIDTH + 221], 0xee);
    memset(viewport, 0xee, sizeof(viewport));
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.pit_no_write_d2l2_left",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d2l2_d2r2_gate.pit_no_write_d2l2_right",
              viewport[20 * DM1_VIEWPORT_WIDTH + 5], 0xee);

    /* Teleporter is the only non-wall write route and goes directly through
     * F0113 with C707/C708; this proves the no-thing contract is not a blanket
     * no-op for all non-wall squares. */
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_TELEPORTER;
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2R2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.teleporter_writes_d2r2_zone",
              viewport[20 * DM1_VIEWPORT_WIDTH + 186], 0x1c);
    check_int("d2l2_d2r2_gate.teleporter_left_zone_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    memset(viewport, 0xee, sizeof(viewport));
    dm1_viewport_3d_draw_csb_near_wall(&state, DM1_VIEW_SQUARE_D2L2, 0, 1, 1);
    check_int("d2l2_d2r2_gate.teleporter_writes_d2l2_zone",
              viewport[20 * DM1_VIEWPORT_WIDTH + 0], 0x1c);
    check_int("d2l2_d2r2_gate.teleporter_d2l2_zone_untouched",
              viewport[20 * DM1_VIEWPORT_WIDTH + 38], 0xee);

    dm1_viewport_3d_set_wall_frame_bitmaps(NULL);
}

static void test_d3l2_d3r2_far_wall_pixel_and_wall_return_gate(void)
{
    /*
     * ReDMCSB source-lock for the PC34 far-side wall lane:
     *   - DUNVIEW.C:579-580 defines G0711/G0712 D3L2/D3R2 wall frames
     *     as 8x49 strips at C702/C703.
     *   - DUNVIEW.C:6254-6264 F0676 and 6321-6331 F0677 route WALL
     *     through C11_WALL_D3L2/C10_WALL_D3R2, optional parity F0105, then
     *     F0107 wall ornament and return before F0115 or field drawing.
     *   - DUNVIEW.C:6270-6289 and 6337-6356 prove D3L2/D3R2 are not the
     *     D2L2/D2R2 no-thing helpers: corridor/door/teleporter paths hand
     *     off to F0115, with teleporter field drawn after that pass.
     *   - DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 land in the same
     *     C10-transparent blit path, with COORD.C:2390-2409 /
     *     IMAGE3.C:866-889 rejecting empty source or viewport intersections.
     */
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t assets[8 * DM1_VIEWPORT_BYTE_WIDTH];
    uint8_t grid[4 * 4];
    DM1_Viewport3DState state;
    const DM1_WallFrame *d3l2_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3L2);
    const DM1_WallFrame *d3r2_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3R2);
    const DM1_ViewportWallDrawSpec *d3l2_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3L2);
    const DM1_ViewportWallDrawSpec *d3r2_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3R2);
    const DM1_ViewportFloorFieldOrderSpec *d3l2_order =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D3L2);
    const DM1_ViewportFloorFieldOrderSpec *d3r2_order =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D3R2);
    uint8_t *d3l2_bitmap;
    uint8_t *d3r2_bitmap;
    DM1_ViewportBlitClipGate d3l2_gate;
    DM1_ViewportBlitClipGate d3r2_gate;

    memset(viewport, 0xee, sizeof(viewport));
    memset(assets, 10, sizeof(assets));
    memset(grid, DM1_VP_ELEMENT_WALL, sizeof(grid));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    state.dungeon_grid = grid;
    state.dungeon_width = 4;
    state.dungeon_height = 4;
    state.temp_bitmap = NULL;
    state.temp_bitmap_size = 0;
    state.wall_set_native[DM1_WALL_D3L2] = 0;
    state.wall_set_native[DM1_WALL_D3R2] = 3;
    d3l2_bitmap = assets + state.wall_set_native[DM1_WALL_D3L2] * DM1_VIEWPORT_BYTE_WIDTH;
    d3r2_bitmap = assets + state.wall_set_native[DM1_WALL_D3R2] * DM1_VIEWPORT_BYTE_WIDTH;

    check_nonnull("d3l2_d3r2_gate.frame.d3l2", d3l2_frame);
    check_nonnull("d3l2_d3r2_gate.frame.d3r2", d3r2_frame);
    check_nonnull("d3l2_d3r2_gate.spec.d3l2", d3l2_spec);
    check_nonnull("d3l2_d3r2_gate.spec.d3r2", d3r2_spec);
    check_nonnull("d3l2_d3r2_gate.order.d3l2", d3l2_order);
    check_nonnull("d3l2_d3r2_gate.order.d3r2", d3r2_order);
    if (!d3l2_frame || !d3r2_frame || !d3l2_spec || !d3r2_spec ||
        !d3l2_order || !d3r2_order) {
        return;
    }

    d3l2_gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(
        d3l2_frame, d3l2_frame->byte_width, d3l2_frame->height);
    d3r2_gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(
        d3r2_frame, d3r2_frame->byte_width, d3r2_frame->height);
    check_int("d3l2_d3r2_gate.d3l2_zone", d3l2_spec->pc34_zone, DM1_PC34_ZONE_WALL_D3L2);
    check_int("d3l2_d3r2_gate.d3r2_zone", d3r2_spec->pc34_zone, DM1_PC34_ZONE_WALL_D3R2);
    check_int("d3l2_d3r2_gate.d3l2_zone_value", DM1_PC34_ZONE_WALL_D3L2, 702);
    check_int("d3l2_d3r2_gate.d3r2_zone_value", DM1_PC34_ZONE_WALL_D3R2, 703);
    check_int("d3l2_d3r2_gate.d3l2_frame_width", d3l2_frame->byte_width, 8);
    check_int("d3l2_d3r2_gate.d3r2_frame_width", d3r2_frame->byte_width, 8);
    check_int("d3l2_d3r2_gate.d3l2_src_x", d3l2_gate.src_x, 0);
    check_int("d3l2_d3r2_gate.d3r2_src_x", d3r2_gate.src_x, 0);
    check_int("d3l2_d3r2_gate.d3l2_dst", d3l2_gate.dst_x == 0 && d3l2_gate.dst_y == 25, 1);
    check_int("d3l2_d3r2_gate.d3r2_dst", d3r2_gate.dst_x == 208 && d3r2_gate.dst_y == 25, 1);
    check_int("d3l2_d3r2_gate.d3l2_visible_span", d3l2_gate.width == 8 && d3l2_gate.height == 49, 1);
    check_int("d3l2_d3r2_gate.d3r2_visible_span", d3r2_gate.width == 8 && d3r2_gate.height == 49, 1);
    check_int("d3l2_d3r2_gate.clip_source_evidence",
              strstr(d3l2_gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(d3r2_gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);
    check_int("d3l2_d3r2_gate.d3l2_wall_source",
              strstr(d3l2_spec->source_lines, "DUNVIEW.C:6254-6260") != NULL &&
              strstr(d3l2_spec->occlusion_source_lines, "DUNVIEW.C:6263-6264") != NULL,
              1);
    check_int("d3l2_d3r2_gate.d3r2_wall_source",
              strstr(d3r2_spec->source_lines, "DUNVIEW.C:6321-6327") != NULL &&
              strstr(d3r2_spec->occlusion_source_lines, "DUNVIEW.C:6330-6331") != NULL,
              1);
    check_int("d3l2_d3r2_gate.wall_case_returns_d3l2",
              d3l2_order->wall_case_returns_before_things ? 1 : 0, 1);
    check_int("d3l2_d3r2_gate.wall_case_returns_d3r2",
              d3r2_order->wall_case_returns_before_things ? 1 : 0, 1);
    check_int("d3l2_d3r2_gate.d3l2_has_f0115_non_wall_path",
              d3l2_order->objects_creatures_projectiles_before_explosions &&
              strstr(d3l2_order->things_source_lines, "DUNVIEW.C:6286") != NULL, 1);
    check_int("d3l2_d3r2_gate.d3r2_has_f0115_non_wall_path",
              d3r2_order->objects_creatures_projectiles_before_explosions &&
              strstr(d3r2_order->things_source_lines, "DUNVIEW.C:6353") != NULL, 1);
    check_int("d3l2_d3r2_gate.d3l2_field_after_things",
              d3l2_order->field_after_things &&
              strstr(d3l2_order->field_source_lines, "DUNVIEW.C:6288-6289") != NULL, 1);
    check_int("d3l2_d3r2_gate.d3r2_field_after_things",
              d3r2_order->field_after_things &&
              strstr(d3r2_order->field_source_lines, "DUNVIEW.C:6355-6356") != NULL, 1);

    dm1_viewport_3d_set_wall_frame_bitmaps(assets);

    /* D3L2 native: F0676 chooses C11_WALL_D3L2 and C702_ZONE_WALL_D3L2. */
    d3l2_bitmap[0 * 8 + 0] = 10;
    d3l2_bitmap[0 * 8 + 1] = 0x42;
    d3l2_bitmap[0 * 8 + 7] = 0x7e;
    d3l2_bitmap[48 * 8 + 7] = 0x55;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.d3l2_native_transparent_skip",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d3l2_d3r2_gate.d3l2_native_next_pixel",
              viewport[25 * DM1_VIEWPORT_WIDTH + 1], 0x42);
    check_int("d3l2_d3r2_gate.d3l2_native_right_edge",
              viewport[25 * DM1_VIEWPORT_WIDTH + 7], 0x7e);
    check_int("d3l2_d3r2_gate.d3l2_native_column_after_source_clip",
              viewport[25 * DM1_VIEWPORT_WIDTH + 8], 0xee);
    check_int("d3l2_d3r2_gate.d3l2_native_bottom_edge",
              viewport[73 * DM1_VIEWPORT_WIDTH + 7], 0x55);
    check_int("d3l2_d3r2_gate.d3l2_native_row_after_frame",
              viewport[74 * DM1_VIEWPORT_WIDTH + 7], 0xee);

    /* Raw wall bytes with thing lists are still WALL; raw corridor bytes are
     * not WALL.  This protects the F0676/F0677 M034_SQUARE_TYPE source lock. */
    memset(viewport, 0xee, sizeof(viewport));
    grid[1 * 4 + 1] = 0x10;
    d3l2_bitmap[0 * 8 + 1] = 0x47;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.raw_wall_thing_list_draws",
              viewport[25 * DM1_VIEWPORT_WIDTH + 1], 0x47);
    memset(viewport, 0xee, sizeof(viewport));
    grid[1 * 4 + 1] = 0x20;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.raw_corridor_does_not_draw_wall",
              viewport[25 * DM1_VIEWPORT_WIDTH + 1], 0xee);
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_WALL;

    /* D3R2 native: F0677 chooses C10_WALL_D3R2 and C703_ZONE_WALL_D3R2. */
    memset(viewport, 0xee, sizeof(viewport));
    d3r2_bitmap[0 * 8 + 0] = 10;
    d3r2_bitmap[0 * 8 + 1] = 0x52;
    d3r2_bitmap[0 * 8 + 7] = 0x5e;
    d3r2_bitmap[48 * 8 + 7] = 0x56;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3R2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.d3r2_native_transparent_skip",
              viewport[25 * DM1_VIEWPORT_WIDTH + 208], 0xee);
    check_int("d3l2_d3r2_gate.d3r2_native_next_pixel",
              viewport[25 * DM1_VIEWPORT_WIDTH + 209], 0x52);
    check_int("d3l2_d3r2_gate.d3r2_native_right_edge",
              viewport[25 * DM1_VIEWPORT_WIDTH + 215], 0x5e);
    check_int("d3l2_d3r2_gate.d3r2_native_column_after_source_clip",
              viewport[25 * DM1_VIEWPORT_WIDTH + 216], 0xee);
    check_int("d3l2_d3r2_gate.d3r2_native_bottom_edge",
              viewport[73 * DM1_VIEWPORT_WIDTH + 215], 0x56);

    /* D3L2 parity: F0676 chooses C10_WALL_D3R2, flips it, and still writes
     * only C702.  The opposite C703 zone must remain untouched. */
    memset(viewport, 0xee, sizeof(viewport));
    memset(d3r2_bitmap, 10, 8 * 49);
    d3r2_bitmap[0 * 8 + 7] = 10;
    d3r2_bitmap[0 * 8 + 6] = 0x63;
    d3r2_bitmap[0 * 8 + 0] = 0x6e;
    state.parity_flip = true;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.d3l2_parity_uses_d3r2_transparent",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d3l2_d3r2_gate.d3l2_parity_flipped_next",
              viewport[25 * DM1_VIEWPORT_WIDTH + 1], 0x63);
    check_int("d3l2_d3r2_gate.d3l2_parity_flipped_right_edge",
              viewport[25 * DM1_VIEWPORT_WIDTH + 7], 0x6e);
    check_int("d3l2_d3r2_gate.d3l2_parity_d3r2_zone_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 208], 0xee);

    /* D3R2 parity: F0677 chooses C11_WALL_D3L2, flips it, and writes C703. */
    memset(viewport, 0xee, sizeof(viewport));
    memset(d3l2_bitmap, 10, 8 * 49);
    d3l2_bitmap[0 * 8 + 7] = 10;
    d3l2_bitmap[0 * 8 + 6] = 0x73;
    d3l2_bitmap[0 * 8 + 0] = 0x7a;
    state.parity_flip = true;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3R2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.d3r2_parity_uses_d3l2_transparent",
              viewport[25 * DM1_VIEWPORT_WIDTH + 208], 0xee);
    check_int("d3l2_d3r2_gate.d3r2_parity_flipped_next",
              viewport[25 * DM1_VIEWPORT_WIDTH + 209], 0x73);
    check_int("d3l2_d3r2_gate.d3r2_parity_flipped_right_edge",
              viewport[25 * DM1_VIEWPORT_WIDTH + 215], 0x7a);
    check_int("d3l2_d3r2_gate.d3r2_parity_d3l2_zone_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0xee);

    /* WALL returns before F0115/field; a fully transparent wall must not be
     * followed by the C702/C703 teleporter field fill. */
    memset(viewport, 0xee, sizeof(viewport));
    memset(d3l2_bitmap, 10, 8 * 49);
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_WALL;
    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.wall_no_field_after_return",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0xee);

    /* TELEPORTER is the explicit far-side field route after the common
     * F0115 handoff; this contrasts with the wall-return no-field case. */
    grid[1 * 4 + 1] = DM1_VP_ELEMENT_TELEPORTER;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.teleporter_field_writes_d3l2_zone",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0x1c);
    check_int("d3l2_d3r2_gate.teleporter_d3r2_zone_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 208], 0xee);

    /* ReDMCSB DUNVIEW.C:F0677 lines 6355-6356 draws the D3R2 teleporter
     * field through F0113 at C703_ZONE_WALL_D3R2 (DEFS.H:4043).  F0113
     * resolves the PC34 zone before blitting (DUNVIEW.C:4382-4397), so this
     * parity-on pixel gate is deliberately about the field zone, not the
     * wall-set flip route covered above. */
    memset(viewport, 0xee, sizeof(viewport));
    state.parity_flip = true;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3R2, 0, 1, 1);
    check_int("d3l2_d3r2_gate.d3r2_teleporter_field_parity_ignored_left_edge",
              viewport[25 * DM1_VIEWPORT_WIDTH + 208], 0x1c);
    check_int("d3l2_d3r2_gate.d3r2_teleporter_field_right_edge",
              viewport[73 * DM1_VIEWPORT_WIDTH + 223], 0x1c);
    check_int("d3l2_d3r2_gate.d3r2_teleporter_field_left_neighbor_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 207], 0xee);
    check_int("d3l2_d3r2_gate.d3r2_teleporter_field_d3l2_zone_untouched",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d3l2_d3r2_gate.d3r2_teleporter_field_row_after_untouched",
              viewport[74 * DM1_VIEWPORT_WIDTH + 223], 0xee);

    dm1_viewport_3d_set_wall_frame_bitmaps(NULL);
}

static void test_d1c_center_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1C);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 589 gives D1C as
     * {32,191,9,119,128,111,48,0}; F0100 lines 3048-3058 forwards that
     * frame to F0132 with C10 transparency, and COORD.C:2390-2409 /
     * IMAGE3.C:866-889 clip the source row before copying pixels.
     *
     * D1C is the nearest visible center wall (depth 1, center column).
     * F0124_DUNGEONVIEW_DrawSquareD1C is the only square draw with
     * wall_case_returns=false (DUNVIEW.C:7833-7843), so the wall blit
     * shares the row with the door-front split (DUNVIEW.C:7874-7937)
     * and any pixel gate must keep the row's left/center allocation
     * intact.  D1C's blit_x=48 in a 128-wide source also confirms
     * the asymmetric source-clip contract used by F0104/F0105 for the
     * nearest D1L/D1R left/right flipped pair (DUNVIEW.C:7438-7460).
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d1c_center_wall_pixel.frame", frame);
    if (!frame) return;

    /* Pre-blit column 47 should NOT reach the viewport (off the left of
     * the resolved src_x=48). */
    bitmap[0 * 128 + 47] = 0x33;
    /* First visible column 48 holds the C10 transparency sentinel. */
    bitmap[0 * 128 + 48] = 10;
    /* First opaque visible column 49. */
    bitmap[0 * 128 + 49] = 0x42;
    /* Last visible column 127 of the resolved 80-wide source span. */
    bitmap[0 * 128 + 127] = 0x7e;
    /* Sanity marker at the bottom of the last visible column. */
    bitmap[110 * 128 + 127] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d1c_center_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d1c_center_wall_pixel.src_x", gate.src_x, 48);
    check_int("d1c_center_wall_pixel.src_y", gate.src_y, 0);
    check_int("d1c_center_wall_pixel.dst_x", gate.dst_x, 32);
    check_int("d1c_center_wall_pixel.dst_y", gate.dst_y, 9);
    check_int("d1c_center_wall_pixel.visible_width", gate.width, 80);
    check_int("d1c_center_wall_pixel.visible_height", gate.height, 111);
    check_int("d1c_center_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    /* Row 9 (top of the resolved frame): transparency sentinel at
     * dst_x=32 is honored, the next source pixel 0x42 is copied to
     * dst_x=33, and the column just before dst_x stays untouched. */
    check_int("d1c_center_wall_pixel.transparent_first_visible_skip",
              viewport[9 * DM1_VIEWPORT_WIDTH + 32], 0xee);
    check_int("d1c_center_wall_pixel.next_source_pixel_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 33], 0x42);
    check_int("d1c_center_wall_pixel.column_before_dst_x_untouched",
              viewport[9 * DM1_VIEWPORT_WIDTH + 31], 0xee);
    check_int("d1c_center_wall_pixel.pre_blit_source_pixel_not_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 32] != 0x33, 1);
    /* Last visible source column 127 must reach dst_x=32+80-1=111. */
    check_int("d1c_center_wall_pixel.last_visible_pixel_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 111], 0x7e);
    /* Column 112 is past the resolved 80-wide dst span and must stay
     * untouched, confirming the F0100 byte-width clipping contract. */
    check_int("d1c_center_wall_pixel.clipped_viewport_column_untouched",
              viewport[9 * DM1_VIEWPORT_WIDTH + 112], 0xee);
    /* The frame's right_x=191 means the wall could nominally reach
     * dst_x=191, but the 128-wide source row shortens the visible
     * span to 80 columns; verify the row 0..79 of the source bitmap
     * beyond column 127 stays untouched. */
    check_int("d1c_center_wall_pixel.last_row_visibility_marker",
              viewport[119 * DM1_VIEWPORT_WIDTH + 111], 0x55);
    /* Row 8 is above the resolved top_y=9 and must stay untouched. */
    check_int("d1c_center_wall_pixel.row_before_frame_untouched",
              viewport[8 * DM1_VIEWPORT_WIDTH + 33], 0xee);
}

static void test_d1r_right_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1R);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 591 gives D1R as
     * {160,223,9,119,128,111,0,0}; F0123 lines 7604-7628 draws wall
     * squares through F0100, which forwards the frame to F0132 with
     * C10 transparency, and COORD.C:2390-2409 / IMAGE3.C:866-889
     * clip the source row before copying pixels.
     *
     * D1R is the nearest visible right wall (depth 1, lateral +1).
     * Its 128-wide source starts at viewport x=160, so the viewport's
     * 224-column right edge leaves only source columns 0..63 visible.
     * This pins the right-edge source clip and the C10 transparency
     * behavior for the asymmetric D1L/D1R pair.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d1r_right_wall_pixel.frame", frame);
    if (!frame) return;

    /* First visible column 0 holds the C10 transparency sentinel. */
    bitmap[0 * 128 + 0] = 10;
    /* First opaque visible column 1. */
    bitmap[0 * 128 + 1] = 0x42;
    /* Last visible column 63 before the viewport's right edge. */
    bitmap[0 * 128 + 63] = 0x7e;
    /* Source column 64 would land at viewport x=224 and must not copy. */
    bitmap[0 * 128 + 64] = 0x33;
    /* Bottom row marker to confirm the clipped width persists to y=119. */
    bitmap[110 * 128 + 63] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d1r_right_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d1r_right_wall_pixel.src_x", gate.src_x, 0);
    check_int("d1r_right_wall_pixel.src_y", gate.src_y, 0);
    check_int("d1r_right_wall_pixel.dst_x", gate.dst_x, 160);
    check_int("d1r_right_wall_pixel.dst_y", gate.dst_y, 9);
    check_int("d1r_right_wall_pixel.visible_width", gate.width, 64);
    check_int("d1r_right_wall_pixel.visible_height", gate.height, 111);
    check_int("d1r_right_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    /* Row 9: transparency sentinel at dst_x=160 is honored, the next
     * source pixel 0x42 is copied to dst_x=161, and the column just
     * before the D1R frame stays untouched. */
    check_int("d1r_right_wall_pixel.transparent_first_visible_skip",
              viewport[9 * DM1_VIEWPORT_WIDTH + 160], 0xee);
    check_int("d1r_right_wall_pixel.next_source_pixel_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 161], 0x42);
    check_int("d1r_right_wall_pixel.column_before_dst_x_untouched",
              viewport[9 * DM1_VIEWPORT_WIDTH + 159], 0xee);
    /* Last visible source column 63 must reach viewport x=223. */
    check_int("d1r_right_wall_pixel.last_visible_pixel_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 223], 0x7e);
    check_int("d1r_right_wall_pixel.source_after_right_edge_not_copied",
              viewport[9 * DM1_VIEWPORT_WIDTH + 223] != 0x33, 1);
    check_int("d1r_right_wall_pixel.bottom_row_marker",
              viewport[119 * DM1_VIEWPORT_WIDTH + 223], 0x55);
    /* Row 8 is above the resolved top_y=9 and must stay untouched. */
    check_int("d1r_right_wall_pixel.row_before_frame_untouched",
              viewport[8 * DM1_VIEWPORT_WIDTH + 161], 0xee);
}

static void test_d1l_left_wall_source_clipped_no_pixel_write(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t before[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1L);
    const DM1_ViewportWallDrawSpec *spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D1L);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB source-lock for the fully clipped D1L wall edge:
     *   - DUNVIEW.C:590 G0163 gives D1L as
     *     {0,63,9,119,128,111,192,0}: its blit_x starts past the
     *     128-wide D1 wall bitmap.
     *   - DUNVIEW.C:7436-7460 F0122_DUNGEONVIEW_DrawSquareD1L still
     *     routes WALL through F0100/F0104/F0105, draws the side ornament,
     *     then returns before F0115.
     *   - DUNVIEW.C:3053-3058 F0100 forwards the frame to F0132 with
     *     C10 transparency; COORD.C:2390-2409 / IMAGE3.C:866-889 reject
     *     the empty source intersection before pixels can be copied.
     *
     * D1R has a visible 64-column right-edge slice.  D1L is the paired
     * near-left wall, but source x=192 >= byte_width=128 means it is a
     * no-write wall path; this pins that asymmetric clip edge.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memcpy(before, viewport, sizeof(before));
    memset(bitmap, 0x42, sizeof(bitmap));
    check_nonnull("d1l_left_wall_no_pixel.frame", frame);
    check_nonnull("d1l_left_wall_no_pixel.spec", spec);
    if (!frame || !spec) return;

    check_int("d1l_left_wall_no_pixel.function",
              strcmp(spec->redmcsb_function, "F0122_DUNGEONVIEW_DrawSquareD1L") == 0, 1);
    check_int("d1l_left_wall_no_pixel.source_route",
              strstr(spec->source_lines, "DUNVIEW.C:7445-7455") != NULL, 1);
    check_int("d1l_left_wall_no_pixel.return_route",
              strstr(spec->occlusion_source_lines, "DUNVIEW.C:7459-7460") != NULL, 1);
    check_int("d1l_left_wall_no_pixel.zone", spec->pc34_zone, DM1_PC34_ZONE_WALL_D1L);

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d1l_left_wall_no_pixel.gate_invisible", gate.visible ? 1 : 0, 0);
    check_int("d1l_left_wall_no_pixel.frame_blit_x", frame->blit_x, 192);
    check_int("d1l_left_wall_no_pixel.frame_byte_width", frame->byte_width, 128);
    check_int("d1l_left_wall_no_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    check_int("d1l_left_wall_no_pixel.viewport_unchanged",
              memcmp(viewport, before, sizeof(viewport)) == 0, 1);
    check_int("d1l_left_wall_no_pixel.nominal_left_edge_untouched",
              viewport[9 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d1l_left_wall_no_pixel.nominal_right_edge_untouched",
              viewport[119 * DM1_VIEWPORT_WIDTH + 63], 0xee);
    check_int("d1l_left_wall_no_pixel.d1r_pair_remains_visible",
              dm1_viewport_3d_resolve_wall_blit_clip_gate(
                  dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1R), 128, 111).visible ? 1 : 0,
              1);
}

static void test_d0l_narrow_side_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[16 * 136];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D0L);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 593 gives D0L as
     * {0,31,0,135,16,136,0,0}; F0100 lines 3048-3058 forwards that
     * frame to F0132 with C10 transparency, and COORD.C:2390-2409 /
     * IMAGE3.C:866-889 clip the source row before copying pixels.
     *
     * D0L is the nearest visible left side wall (depth 0, lateral -1)
     * drawn LAST in F0128 (DUNVIEW.C:8534-8537), so it sits on top of
     * the champion panel and party.  Its byte_width=16 in a 32-wide
     * native bitmap (G0701_puc_Bitmap_WallSet_Wall_D0L,
     * STARTUP2.C:557) means the F0100 source row clip shortens the
     * resolved dst span from the frame's 32 columns to 16.  This test
     * pins that 16/32 contract so a future gate loosening cannot
     * regress the visible 16-column band and silently start repainting
     * the champion panel with the second half of the wall row.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d0l_narrow_side_wall_pixel.frame", frame);
    if (!frame) return;

    /* First visible column 0 holds the C10 transparency sentinel. */
    bitmap[0 * 16 + 0] = 10;
    /* First opaque visible column 1. */
    bitmap[0 * 16 + 1] = 0x42;
    /* Last visible column 15 of the resolved 16-wide source span. */
    bitmap[0 * 16 + 15] = 0x7e;
    /* Bottom row marker to confirm height=136 is rendered. */
    bitmap[135 * 16 + 8] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d0l_narrow_side_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d0l_narrow_side_wall_pixel.src_x", gate.src_x, 0);
    check_int("d0l_narrow_side_wall_pixel.src_y", gate.src_y, 0);
    check_int("d0l_narrow_side_wall_pixel.dst_x", gate.dst_x, 0);
    check_int("d0l_narrow_side_wall_pixel.dst_y", gate.dst_y, 0);
    check_int("d0l_narrow_side_wall_pixel.visible_width", gate.width, 16);
    check_int("d0l_narrow_side_wall_pixel.visible_height", gate.height, 136);
    check_int("d0l_narrow_side_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    /* Row 0: transparency sentinel at dst_x=0 is honored, the next
     * source pixel 0x42 is copied to dst_x=1. */
    check_int("d0l_narrow_side_wall_pixel.transparent_first_visible_skip",
              viewport[0 * DM1_VIEWPORT_WIDTH + 0], 0xee);
    check_int("d0l_narrow_side_wall_pixel.next_source_pixel_copied",
              viewport[0 * DM1_VIEWPORT_WIDTH + 1], 0x42);
    /* Last visible source column 15 must reach dst_x=15. */
    check_int("d0l_narrow_side_wall_pixel.last_visible_pixel_copied",
              viewport[0 * DM1_VIEWPORT_WIDTH + 15], 0x7e);
    /* The frame's right_x=31 implies a 32-wide wall, but the resolved
     * 16-wide source clip stops the write at column 15.  Verify the
     * 17th column is still 0xee (untouched) so the resolved gate keeps
     * the champion panel beneath the wall. */
    check_int("d0l_narrow_side_wall_pixel.post_frame_column_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 16], 0xee);
    /* The bottom row marker must reach dst_y=135. */
    check_int("d0l_narrow_side_wall_pixel.bottom_row_marker",
              viewport[135 * DM1_VIEWPORT_WIDTH + 8], 0x55);
}

static void test_d0r_narrow_side_wall_pixel_slice_uses_redmcsb_frame_clip(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t bitmap[16 * 136];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D0R);
    DM1_ViewportBlitClipGate gate;

    /*
     * ReDMCSB: DUNVIEW.C G0163 line 594 gives D0R as
     * {192,223,0,135,16,136,0,0}; F0126 lines 8117-8144 routes WALL
     * through the PC34 D0R wall zone and returns, while F0100 lines
     * 3048-3058 forwards the frame to F0132 with C10 transparency.
     * COORD.C:2390-2409 / IMAGE3.C:866-889 clip the row to the source
     * byte width before copying pixels.
     *
     * D0R is the nearest visible right side wall (depth 0, lateral +1)
     * drawn just before D0C in F0128 (DUNVIEW.C:8538-8541).  Its
     * byte_width=16 in a 32-wide native bitmap (G0702_puc_Bitmap_
     * WallSet_Wall_D0R, STARTUP2.C:557) means the source clip only
     * writes viewport columns 192..207 even though the frame spans
     * 192..223.  This pins the party-side right band so the extra
     * frame columns cannot start repainting the champion panel area.
     */
    memset(viewport, 0xee, sizeof(viewport));
    memset(bitmap, 10, sizeof(bitmap));
    check_nonnull("d0r_narrow_side_wall_pixel.frame", frame);
    if (!frame) return;

    /* First visible column 0 holds the C10 transparency sentinel. */
    bitmap[0 * 16 + 0] = 10;
    /* First opaque visible column 1. */
    bitmap[0 * 16 + 1] = 0x42;
    /* Last visible column 15 of the resolved 16-wide source span. */
    bitmap[0 * 16 + 15] = 0x7e;
    /* Bottom row marker to confirm height=136 is rendered. */
    bitmap[135 * 16 + 8] = 0x55;

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d0r_narrow_side_wall_pixel.gate_visible", gate.visible ? 1 : 0, 1);
    check_int("d0r_narrow_side_wall_pixel.src_x", gate.src_x, 0);
    check_int("d0r_narrow_side_wall_pixel.src_y", gate.src_y, 0);
    check_int("d0r_narrow_side_wall_pixel.dst_x", gate.dst_x, 192);
    check_int("d0r_narrow_side_wall_pixel.dst_y", gate.dst_y, 0);
    check_int("d0r_narrow_side_wall_pixel.visible_width", gate.width, 16);
    check_int("d0r_narrow_side_wall_pixel.visible_height", gate.height, 136);
    check_int("d0r_narrow_side_wall_pixel.source_evidence",
              strstr(gate.source_lines, "DUNVIEW.C:3053-3058") != NULL &&
              strstr(gate.source_lines, "COORD.C:2390-2409") != NULL &&
              strstr(gate.source_lines, "IMAGE3.C:866-889") != NULL, 1);

    dm1_viewport_3d_draw_wall(&state, bitmap, frame);
    /* Row 0: transparency sentinel at dst_x=192 is honored, the next
     * source pixel 0x42 is copied to dst_x=193. */
    check_int("d0r_narrow_side_wall_pixel.transparent_first_visible_skip",
              viewport[0 * DM1_VIEWPORT_WIDTH + 192], 0xee);
    check_int("d0r_narrow_side_wall_pixel.next_source_pixel_copied",
              viewport[0 * DM1_VIEWPORT_WIDTH + 193], 0x42);
    check_int("d0r_narrow_side_wall_pixel.column_before_dst_x_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 191], 0xee);
    /* Last visible source column 15 must reach dst_x=207. */
    check_int("d0r_narrow_side_wall_pixel.last_visible_pixel_copied",
              viewport[0 * DM1_VIEWPORT_WIDTH + 207], 0x7e);
    /* The frame's right_x=223 implies a 32-wide wall, but the resolved
     * 16-wide source clip stops the write at column 207. */
    check_int("d0r_narrow_side_wall_pixel.post_frame_column_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 208], 0xee);
    check_int("d0r_narrow_side_wall_pixel.frame_right_edge_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 223], 0xee);
    /* The bottom row marker must reach dst_y=135. */
    check_int("d0r_narrow_side_wall_pixel.bottom_row_marker",
              viewport[135 * DM1_VIEWPORT_WIDTH + 200], 0x55);
}

static void test_pc34_parity_wall_draw_uses_opposite_native_bitmap_without_temp(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t assets[32 * DM1_VIEWPORT_BYTE_WIDTH];
    DM1_Viewport3DState state;
    const uint8_t *base;
    uint8_t *d3l2_bitmap;
    uint8_t *d3r2_bitmap;
    int d3l2_offset;
    int d3r2_offset;

    memset(viewport, 0, sizeof(viewport));
    memset(assets, 10, sizeof(assets));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_load_wall_set(&state, 0, 0);
    state.temp_bitmap = NULL;
    state.temp_bitmap_size = 0;

    base = assets + 20 * DM1_VIEWPORT_BYTE_WIDTH;
    d3l2_offset = 20 + state.wall_set_native[DM1_WALL_D3L2];
    d3r2_offset = 20 + state.wall_set_native[DM1_WALL_D3R2];
    d3l2_bitmap = assets + d3l2_offset * DM1_VIEWPORT_BYTE_WIDTH;
    d3r2_bitmap = assets + d3r2_offset * DM1_VIEWPORT_BYTE_WIDTH;
    for (int x = 0; x < 8; ++x) {
        d3l2_bitmap[x] = 0x11;
        d3r2_bitmap[x] = 0x22;
    }

    dm1_viewport_3d_set_wall_frame_bitmaps(base);

    state.parity_flip = false;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 0, 0);
    check_int("PC34.parity_wall_draw.native_d3l2_pixel",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0x11);

    memset(viewport, 0, sizeof(viewport));
    state.parity_flip = true;
    dm1_viewport_3d_draw_csb_back_wall(&state, DM1_VIEW_SQUARE_D3L2, 0, 0, 0);
    check_int("PC34.parity_wall_draw.parity_uses_d3r2_native_source",
              viewport[25 * DM1_VIEWPORT_WIDTH + 0], 0x22);
    check_int("PC34.parity_wall_draw.parity_without_temp_still_draws",
              viewport[25 * DM1_VIEWPORT_WIDTH + 7], 0x22);

    dm1_viewport_3d_set_wall_frame_bitmaps(NULL);
}


static void test_d0l_d0r_parity_pixel_slice_uses_redmcsb_frame_clip(void)
{
    /*
     * Source lock for the DM1 V1 party-side wall parity pixel slice:
     *   - DUNVIEW.C:8016-8033 (F0125_DUNGEONVIEW_DrawSquareD0L) - PC34 MEDIA720
     *       native: F0104(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L)
     *       parity: F0105(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L)
     *   - DUNVIEW.C:8126-8139 (F0126_DUNGEONVIEW_DrawSquareD0R) - PC34 MEDIA720
     *       native: F0104(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R)
     *       parity: F0105(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R)
     *   - DUNVIEW.C:3185-3204 (F0105) copy/flip/blit with C10 transparency
     *   - COORD.C:2390-2409 / IMAGE3.C:866-889 source row clipping
     */
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t assets[32 * DM1_VIEWPORT_BYTE_WIDTH];
    DM1_Viewport3DState state;
    const uint8_t *base;
    uint8_t *d0l_bitmap;
    uint8_t *d0r_bitmap;
    const DM1_WallFrame *d0l_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D0L);
    const DM1_WallFrame *d0r_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D0R);
    const DM1_ViewportWallDrawSpec *d0l_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D0L);
    const DM1_ViewportWallDrawSpec *d0r_spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D0R);

    memset(viewport, 0xee, sizeof(viewport));
    memset(assets, 10, sizeof(assets));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_load_wall_set(&state, 0, 0);
    state.temp_bitmap = NULL;
    state.temp_bitmap_size = 0;

    /* D0 side-wall strips are 16x136, larger than one synthetic 224-byte
     * wall-set slot, so park the two controlled strips far apart. */
    base = assets;
    state.wall_set_native[DM1_WALL_D0R] = 0;
    state.wall_set_native[DM1_WALL_D0L] = 12;
    d0r_bitmap = assets + state.wall_set_native[DM1_WALL_D0R] * DM1_VIEWPORT_BYTE_WIDTH;
    d0l_bitmap = assets + state.wall_set_native[DM1_WALL_D0L] * DM1_VIEWPORT_BYTE_WIDTH;

    check_nonnull("d0l_d0r_parity.frame.d0l", d0l_frame);
    check_nonnull("d0l_d0r_parity.frame.d0r", d0r_frame);
    check_nonnull("d0l_d0r_parity.spec.d0l", d0l_spec);
    check_nonnull("d0l_d0r_parity.spec.d0r", d0r_spec);
    if (!d0l_frame || !d0r_frame || !d0l_spec || !d0r_spec) return;

    for (int row = 0; row < 136; ++row) {
        for (int x = 0; x < 16; ++x) {
            d0l_bitmap[row * 16 + x] = 0x99;
            d0r_bitmap[row * 16 + x] = 0x88;
        }
    }
    for (int x = 0; x < 16; ++x) {
        d0l_bitmap[x] = (uint8_t)(0x11 + x);
        d0r_bitmap[x] = (uint8_t)(0x21 + x);
    }
    d0l_bitmap[5 * 16 + 5] = 10;
    d0r_bitmap[7 * 16 + 3] = 10;

    dm1_viewport_3d_set_wall_frame_bitmaps(base);

    /* D0L parity: choose native D0R, then F0105 flips it into D0L's zone. */
    state.parity_flip = true;
    {
        bool flip_h = false;
        DM1_WallSetIndex wall_idx =
            dm1_viewport_3d_select_wall_bitmap(d0l_spec, state.parity_flip, &flip_h);
        const uint8_t *wall_bmp =
            base + (int)state.wall_set_native[wall_idx] * DM1_VIEWPORT_BYTE_WIDTH;
        check_int("d0l_d0r_parity.d0l_parity_selects_d0r",
                  (int)wall_idx, (int)DM1_WALL_D0R);
        check_int("d0l_d0r_parity.d0l_parity_flip_h", flip_h ? 1 : 0, 1);
        dm1_viewport_3d_draw_door_frame_flipped(&state, wall_bmp, d0l_frame);
    }
    check_int("d0l_d0r_parity.d0l_parity_leftmost_is_d0r_15",
              viewport[0 * DM1_VIEWPORT_WIDTH + 0], 0x30);
    check_int("d0l_d0r_parity.d0l_parity_rightmost_is_d0r_0",
              viewport[0 * DM1_VIEWPORT_WIDTH + 15], 0x21);
    check_int("d0l_d0r_parity.d0l_parity_c10_skip",
              viewport[7 * DM1_VIEWPORT_WIDTH + 12], 0xee);
    check_int("d0l_d0r_parity.d0l_parity_d0r_zone_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 192], 0xee);

    /* D0R native: choose native D0R and blit it directly into D0R's zone. */
    memset(viewport, 0xee, sizeof(viewport));
    state.parity_flip = false;
    {
        bool flip_h = false;
        DM1_WallSetIndex wall_idx =
            dm1_viewport_3d_select_wall_bitmap(d0r_spec, state.parity_flip, &flip_h);
        const uint8_t *wall_bmp =
            base + (int)state.wall_set_native[wall_idx] * DM1_VIEWPORT_BYTE_WIDTH;
        check_int("d0l_d0r_parity.d0r_native_selects_d0r",
                  (int)wall_idx, (int)DM1_WALL_D0R);
        check_int("d0l_d0r_parity.d0r_native_flip_h", flip_h ? 1 : 0, 0);
        dm1_viewport_3d_draw_wall(&state, wall_bmp, d0r_frame);
    }
    check_int("d0l_d0r_parity.d0r_native_leftmost",
              viewport[0 * DM1_VIEWPORT_WIDTH + 192], 0x21);
    check_int("d0l_d0r_parity.d0r_native_rightmost",
              viewport[0 * DM1_VIEWPORT_WIDTH + 207], 0x30);
    check_int("d0l_d0r_parity.d0r_native_c10_skip",
              viewport[7 * DM1_VIEWPORT_WIDTH + 195], 0xee);
    check_int("d0l_d0r_parity.d0r_native_outside_frame_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 191], 0xee);

    /* D0R parity: choose native D0L, then F0105 flips it into D0R's zone. */
    memset(viewport, 0xee, sizeof(viewport));
    state.parity_flip = true;
    {
        bool flip_h = false;
        DM1_WallSetIndex wall_idx =
            dm1_viewport_3d_select_wall_bitmap(d0r_spec, state.parity_flip, &flip_h);
        const uint8_t *wall_bmp =
            base + (int)state.wall_set_native[wall_idx] * DM1_VIEWPORT_BYTE_WIDTH;
        check_int("d0l_d0r_parity.d0r_parity_selects_d0l",
                  (int)wall_idx, (int)DM1_WALL_D0L);
        check_int("d0l_d0r_parity.d0r_parity_flip_h", flip_h ? 1 : 0, 1);
        dm1_viewport_3d_draw_door_frame_flipped(&state, wall_bmp, d0r_frame);
    }
    check_int("d0l_d0r_parity.d0r_parity_leftmost_is_d0l_15",
              viewport[0 * DM1_VIEWPORT_WIDTH + 192], 0x20);
    check_int("d0l_d0r_parity.d0r_parity_rightmost_is_d0l_0",
              viewport[0 * DM1_VIEWPORT_WIDTH + 207], 0x11);
    check_int("d0l_d0r_parity.d0r_parity_c10_skip",
              viewport[5 * DM1_VIEWPORT_WIDTH + 202], 0xee);
    check_int("d0l_d0r_parity.d0r_parity_d0l_zone_untouched",
              viewport[0 * DM1_VIEWPORT_WIDTH + 0], 0xee);

    check_int("d0l_d0r_parity.d0l_source_anchor",
              strstr(d0l_spec->source_lines, "DUNVIEW.C:8016-8033") != NULL, 1);
    check_int("d0l_d0r_parity.d0r_source_anchor",
              strstr(d0r_spec->source_lines, "DUNVIEW.C:8126-8139") != NULL, 1);
    check_int("d0l_d0r_parity.d0l_occlusion_anchor",
              strstr(d0l_spec->occlusion_source_lines, "DUNVIEW.C:8036-8038") != NULL, 1);
    check_int("d0l_d0r_parity.d0r_occlusion_anchor",
              strstr(d0r_spec->occlusion_source_lines, "DUNVIEW.C:8142-8144") != NULL, 1);

    dm1_viewport_3d_set_wall_frame_bitmaps(NULL);
}


static void test_d0_d1_visible_square_draw_order_gate(void)
{
    static const struct {
        size_t draw_index;
        DM1_ViewSquareIndex square;
        int depth;
        int lateral;
        const char *function_name;
        const char *draw_source;
    } draw_expected[] = {
        { 13, DM1_VIEW_SQUARE_D1L, 1, -1, "F0122_DUNGEONVIEW_DrawSquareD1L", "8522-8525" },
        { 14, DM1_VIEW_SQUARE_D1R, 1,  1, "F0123_DUNGEONVIEW_DrawSquareD1R", "8526-8529" },
        { 15, DM1_VIEW_SQUARE_D1C, 1,  0, "F0124_DUNGEONVIEW_DrawSquareD1C", "8530-8533" },
        { 16, DM1_VIEW_SQUARE_D0L, 0, -1, "F0125_DUNGEONVIEW_DrawSquareD0L", "8534-8537" },
        { 17, DM1_VIEW_SQUARE_D0R, 0,  1, "F0126_DUNGEONVIEW_DrawSquareD0R", "8538-8541" },
        { 18, DM1_VIEW_SQUARE_D0C, 0,  0, "F0127_DUNGEONVIEW_DrawSquareD0C", "8542" },
    };
    static const struct {
        DM1_ViewSquareIndex square;
        uint16_t side_order;
        const char *side_source;
        const char *field_source;
    } side_expected[] = {
        { DM1_VIEW_SQUARE_D1L, 0x0032, "7536", "7538-7555" },
        { DM1_VIEW_SQUARE_D1R, 0x0041, "7704", "7706-7722" },
        { DM1_VIEW_SQUARE_D0L, 0x0002, "8005", "8050-8059" },
        { DM1_VIEW_SQUARE_D0R, 0x0001, "8115", "8150-8159" },
    };
    const DM1_ViewportThingLayerSpec *objects =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_OBJECTS);
    const DM1_ViewportThingLayerSpec *creatures =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_CREATURES);
    const DM1_ViewportThingLayerSpec *projectiles =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_PROJECTILES);
    const DM1_ViewportThingLayerSpec *explosions =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_EXPLOSIONS);

    check_int("d0_d1_gate.draw_order_count", (int)dm1_viewport_3d_draw_order_count(), 19);
    for (size_t i = 0; i < sizeof(draw_expected) / sizeof(draw_expected[0]); ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(draw_expected[i].draw_index);
        char id[128];
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.nonnull", i);
        check_nonnull(id, step);
        if (!step) continue;
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.square", i);
        check_int(id, (int)step->square, (int)draw_expected[i].square);
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.depth", i);
        check_int(id, step->rel_depth, draw_expected[i].depth);
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.lateral", i);
        check_int(id, step->rel_lateral, draw_expected[i].lateral);
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.function", i);
        check_int(id, strcmp(step->redmcsb_function, draw_expected[i].function_name) == 0, 1);
        snprintf(id, sizeof(id), "d0_d1_gate.draw.%zu.source", i);
        check_int(id, strstr(step->source_lines, draw_expected[i].draw_source) != NULL, 1);
    }

    for (size_t i = 0; i < sizeof(side_expected) / sizeof(side_expected[0]); ++i) {
        const DM1_ViewportSideOcclusionSpec *side =
            dm1_viewport_3d_get_side_occlusion_spec_for_square(side_expected[i].square);
        char id[128];
        snprintf(id, sizeof(id), "d0_d1_gate.side.%zu.nonnull", i);
        check_nonnull(id, side);
        if (!side) continue;
        snprintf(id, sizeof(id), "d0_d1_gate.side.%zu.order", i);
        check_int(id, side->cell_order, side_expected[i].side_order);
        snprintf(id, sizeof(id), "d0_d1_gate.side.%zu.f0115", i);
        check_int(id, strstr(side->f0115_source_lines, side_expected[i].side_source) != NULL, 1);
        if (side_expected[i].field_source) {
            const DM1_ViewportFloorFieldOrderSpec *field =
                dm1_viewport_3d_get_floor_field_order_spec_for_square(side_expected[i].square);
            snprintf(id, sizeof(id), "d0_d1_gate.side.%zu.field_after_things", i);
            check_int(id, field && field->field_after_things && strstr(field->field_source_lines, side_expected[i].field_source) != NULL, 1);
        }
    }

    check_nonnull("d0_d1_gate.d1c_door_front", dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1C));
    check_nonnull("d0_d1_gate.d0c_floor_field", dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D0C));
    check_nonnull("d0_d1_gate.d0c_thieves_eye", dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C));
    check_nonnull("d0_d1_gate.d1_projectiles", dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1C));
    check_nonnull("d0_d1_gate.d0_projectiles", dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C));
    check_int("d0_d1_gate.layers.objects", objects && objects->repeats_per_cell && !objects->after_all_cells, 1);
    check_int("d0_d1_gate.layers.creatures", creatures && creatures->repeats_per_cell && !creatures->after_all_cells, 1);
    check_int("d0_d1_gate.layers.projectiles", projectiles && projectiles->repeats_per_cell && !projectiles->after_all_cells, 1);
    check_int("d0_d1_gate.layers.explosions", explosions && !explosions->repeats_per_cell && explosions->after_all_cells, 1);
}


static void test_source_evidence_mentions_visual_lane(void)
{
    const char *e = dm1_viewport_3d_source_evidence();
    check_nonnull("source_evidence.nonnull", e);
    if (!e) return;
    check_int("source_evidence.g0163", strstr(e, "G0163_aauc_Graphic558_Frame_Walls") != NULL, 1);
    check_int("source_evidence.f0128", strstr(e, "DUNVIEW.C:8318 F0128_DUNGEONVIEW_Draw_CPSF") != NULL, 1);
    check_int("source_evidence.f0150", strstr(e, "DUNGEON.C:1371-1421 F0150") != NULL, 1);
    check_int("source_evidence.g2107", strstr(e, "G2107_WallSet[15]") != NULL, 1);
    check_int("source_evidence.pc34_side", strstr(e, "PC34 parity side-wall selection") != NULL, 1);
    check_int("source_evidence.pc34_d2_side", strstr(e, "F0678/F0679 PC34 D2L2/D2R2") != NULL, 1);
    check_int("source_evidence.f0115", strstr(e, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions") != NULL, 1);
    check_int("source_evidence.f0115_cell_order", strstr(e, "packed cell-order") != NULL, 1);
    check_int("source_evidence.d4_far_object_pass",
              strstr(e, "DUNVIEW.C:8466-8477") != NULL && strstr(e, "C0x0001 before D3 walls") != NULL, 1);
    check_int("source_evidence.f0115_projectiles", strstr(e, "projectile draw pass") != NULL, 1);
    check_int("source_evidence.projectile_occlusion", strstr(e, "G2028 row and C2900 zone mapping") != NULL, 1);
    check_int("source_evidence.f0115_explosion_global", strstr(e, "explosion pass after all ordered cells") != NULL, 1);
    check_int("source_evidence.floor_field_order", strstr(e, "stairs/pit/floor-ornament/F0115/teleporter-field order") != NULL, 1);
    check_int("source_evidence.d0c_field_order", strstr(e, "8241-8308") != NULL, 1);
    check_int("source_evidence.explosion_zone_mapping", strstr(e, "PC34 explosion viewport zones") != NULL, 1);
    check_int("source_evidence.d3r_field_order", strstr(e, "6514-6638") != NULL, 1);
    check_int("source_evidence.d3r2_field_order", strstr(e, "6304-6356") != NULL, 1);
    check_int("source_evidence.d2l2_no_thing_pass", strstr(e, "6846-6865") != NULL && strstr(e, "no F0115 thing pass") != NULL, 1);
    check_int("source_evidence.d2r2_no_thing_pass", strstr(e, "6877-6896") != NULL && strstr(e, "no F0115 thing pass") != NULL, 1);
    check_int("source_evidence.d2l_field_order", strstr(e, "6914-7048") != NULL, 1);
    check_int("source_evidence.d2r_field_order", strstr(e, "7065-7240") != NULL, 1);
    check_int("source_evidence.d0_side_field_order",
        strstr(e, "DUNVIEW.C:7978-8062") != NULL && strstr(e, "DUNVIEW.C:8082-8162") != NULL, 1);
    check_int("source_evidence.d0c_foreground_before_things",
        strstr(e, "DUNVIEW.C:8185-8240") != NULL && strstr(e, "draw before common F0115") != NULL, 1);
    check_int("source_evidence.door_front_occlusion", strstr(e, "door-front occlusion") != NULL, 1);
    check_int("source_evidence.d2c_front_order",
        strstr(e, "DUNVIEW.C:7289-7312") != NULL &&
        strstr(e, "DUNVIEW.C:7353-7387") != NULL &&
        strstr(e, "DEFS.H:4082-4088") != NULL, 1);
    check_int("source_evidence.far_door_front_occlusion", strstr(e, "DUNVIEW.C:6270-6286") != NULL && strstr(e, "DUNVIEW.C:6337-6353") != NULL, 1);
    check_int("source_evidence.d1_side_door_front_occlusion", strstr(e, "DUNVIEW.C:7493-7536") != NULL && strstr(e, "DUNVIEW.C:7661-7704") != NULL, 1);
    check_int("source_evidence.d1c_door_front_occlusion", strstr(e, "DUNVIEW.C:7874-7937") != NULL, 1);
    check_int("source_evidence.d1c_door_button_occlusion", strstr(e, "frame/button/door") != NULL, 1);
    check_int("source_evidence.d0c_thieves_eye_frame_occlusion",
        strstr(e, "DUNVIEW.C:8185-8216") != NULL && strstr(e, "copy front frame, composite hole") != NULL, 1);
    check_int("source_evidence.side_occlusion", strstr(e, "side-door/stairs-side F0115 cell-order occlusion") != NULL, 1);
    check_int("source_evidence.defs_zones", strstr(e, "DEFS.H:4040-4057") != NULL, 1);
    check_int("source_evidence.wall_source_clip_gate", strstr(e, "COORD.C:2390-2409") != NULL, 1);
    check_int("source_evidence.wall_empty_blit_gate", strstr(e, "IMAGE3.C:866-889") != NULL, 1);
    check_int("source_evidence.occlusion", strstr(e, "wall case returns") != NULL, 1);
    check_int("source_evidence.command_dispatch", strstr(e, "COMMAND.C:2045-2156") != NULL, 1);
    check_int("source_evidence.next_redraw", strstr(e, "GAMELOOP.C:55-90") != NULL, 1);
    check_int("source_evidence.present_wait", strstr(e, "DRAWVIEW.C:709-722") != NULL, 1);
    check_int("source_evidence.same_viewport_mouse", strstr(e, "COMMAND.C:106-114") != NULL, 1);
    check_int("source_evidence.same_viewport_turn", strstr(e, "CLIKMENU.C:142-174") != NULL, 1);
    check_int("source_evidence.same_viewport_move", strstr(e, "CLIKMENU.C:180-347") != NULL, 1);
    check_int("source_evidence.same_viewport_draw", strstr(e, "DUNVIEW.C:8318-8611") != NULL, 1);
    check_int("source_evidence.same_viewport_present", strstr(e, "DRAWVIEW.C:709-858") != NULL, 1);
    check_int("source_evidence.same_viewport_assets", strstr(e, "canonical DM1 PC34 assets") != NULL, 1);
}

/* ── DM1 V1 Viewport 3D source-evidence drift regression ────────────────────
 * The Python verifiers in tools/verify_pass404/406/563/565/570/576/577_dm1_v1_*
 * previously checked that canonical evidence tokens (side-content center
 * blockers, redraw cadence, D1L/D1R wall rows, D0C Thieves Eye table, D1-side
 * door-front table, D2C door-front / floor-field / wall tables, the D-side wall
 * table, and the F0128 D0/D1 visible-square table) lived inside narrow line
 * ranges of the source files.  The metadata tables grew over time (e.g.,
 * D1L/D1R wall rows moved from lines 416-417 to 470-471 once
 * D3L/D3R/D3C/D2L2/D2R2/D2L/D2R/D2C/D1C/D0L/D0R rows were inserted above), and
 * the verifier line ranges drifted stale and started failing verifiers even
 * though the CTest itself remained green.
 *
 * The drift-proof whole-file scan in the Python verifiers is one half of the
 * fix; this test is the other half — it reads the source files at CTest
 * runtime and asserts the canonical evidence tokens are still present
 * anywhere in the file, with NO line-range constraint.  A future metadata
 * table reshuffle can no longer make this test fail, and a future *removal*
 * of any canonical evidence token will.
 */
#ifndef FIRESTAFF_SOURCE_DIR
#define FIRESTAFF_SOURCE_DIR "."
#endif

static int read_whole_file(const char *rel, char **out, size_t *out_size)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", FIRESTAFF_SOURCE_DIR, rel);
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return 0; }
    rewind(f);
    char *data = (char *)malloc((size_t)size + 1u);
    if (!data) { fclose(f); return 0; }
    size_t got = fread(data, 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { free(data); return 0; }
    data[size] = '\0';
    *out = data;
    *out_size = (size_t)size;
    return 1;
}

static int file_contains(const char *rel, const char *needle)
{
    char *data = NULL;
    size_t size = 0;
    if (!read_whole_file(rel, &data, &size)) return 0;
    int hit = strstr(data, needle) != NULL;
    free(data);
    return hit;
}

static void test_dm1_v1_viewport_3d_source_evidence_drift_regression(void)
{
    /* These are the canonical evidence tokens the stale-line-range verifier
     * LOCAL checks were trying to assert existed.  Re-stated here as a single
     * CTest-time whole-file scan so future metadata-table growth cannot defeat
     * the regression. */
    static const struct { const char *rel; const char *token; const char *id; } needles[] = {
        /* pass404: side-content/deferred-explosion center-blocker guards */
        { "src/engine/m11_game_view.c",
          "static void m11_draw_dm1_side_contents(const M11_GameViewState* state",
          "pass404.side_contents_function" },
        { "src/engine/m11_game_view.c",
          "blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
          "pass404.blocking_center_depth" },
        { "src/engine/m11_game_view.c",
          "if (blockingCenterDepth >= 0 && depth >= blockingCenterDepth)",
          "pass404.side_contents_blocker_gate" },
        { "src/engine/m11_game_view.c",
          "m11_draw_item_sprite(g_drawState, framebuffer",
          "pass404.side_contents_item_draw" },
        { "src/engine/m11_game_view.c",
          "static void m11_draw_dm1_deferred_explosion_pass(const M11_GameViewState* state",
          "pass404.deferred_explosion_function" },
        { "src/engine/m11_game_view.c",
          "m11_draw_dm1_deferred_side_explosion(",
          "pass404.deferred_side_explosion_draw" },
        /* pass406: M11 redraw cadence / viewport-dirty publication route */
        { "src/engine/m11_game_view.c",
          "static int m11_apply_dm1_v1_pipeline_tick(M11_GameViewState* state,",
          "pass406.pipeline_tick_function" },
        { "src/engine/m11_game_view.c",
          "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(",
          "pass406.enqueue_command" },
        { "src/engine/m11_game_view.c",
          "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(",
          "pass406.decrement_cooldowns" },
        { "src/engine/m11_game_view.c",
          "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(",
          "pass406.process_one_tick" },
        { "src/engine/m11_game_view.c",
          "return state->lastDm1V1MovementPipelineResult.viewportDirty ||",
          "pass406.redraw_publication" },
        { "src/engine/main_loop_m11.c",
          "redrawWasAfterViewportDirty =",
          "pass406.main_loop_dirty_snapshot" },
        { "src/engine/main_loop_m11.c",
          "lastInputRedrawAfterViewportDirty = redrawWasAfterViewportDirty;",
          "pass406.main_loop_dirty_record" },
        { "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c",
          "outResult->viewportDirty = outResult->core.viewportRedrawRequested;",
          "pass406.pipeline_viewport_dirty" },
        { "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c",
          "outResult->provenance.viewportPresentEvidence =",
          "pass406.pipeline_present_evidence" },
        /* pass563: D1L/D1R side-wall rows in the wall_draw_specs table */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  true,  false, DM1_PC34_ZONE_WALL_D1L",
          "pass563.d1l_wall_row" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  true,  false, DM1_PC34_ZONE_WALL_D1R",
          "pass563.d1r_wall_row" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DUNVIEW.C:7459-7460 side ornament then return",
          "pass563.d1l_ornament_citation" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DUNVIEW.C:7627-7628 side ornament then return",
          "pass563.d1r_ornament_citation" },
        /* pass565 d0c: D0C Thieves Eye door-frame occlusion table */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736",
          "pass565_d0c.thieves_eye_table" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion",
          "pass565_d0c.thieves_eye_citation" },
        /* pass565 d1: D1-side door-front table */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1L, 0x0028, 0x0039",
          "pass565_d1.d1l_door_front" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1R, 0x0018, 0x0049",
          "pass565_d1.d1r_door_front" },
        /* pass570: D2C door-front / floor-field / wall tables */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349",
          "pass570.d2c_door_front" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D2C, 0x3421",
          "pass570.d2c_floor_field" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  true,  true,  DM1_PC34_ZONE_WALL_D2C",
          "pass570.d2c_wall" },
        /* pass570 zone defines in the public header */
        { "include/dm1_v1_viewport_3d_pc34_compat.h",
          "#define DM1_PC34_ZONE_DOOR_FRAME_LEFT_D2C   724",
          "pass570.d2c_zone_left" },
        { "include/dm1_v1_viewport_3d_pc34_compat.h",
          "#define DM1_PC34_ZONE_DOOR_FRAME_RIGHT_D2C  725",
          "pass570.d2c_zone_right" },
        { "include/dm1_v1_viewport_3d_pc34_compat.h",
          "#define DM1_PC34_ZONE_DOOR_FRAME_TOP_D2C    730",
          "pass570.d2c_zone_top" },
        /* pass570 runtime test data */
        { "tests/test_dm1_v1_viewport_3d_pc34_compat.c",
          "{ DM1_VIEW_SQUARE_D2C, \"7314\", \"7315\", \"7317\", \"7332\", \"7339\", \"7341\", 0x0218, 0x0349, {1, 2}, {4, 3} },",
          "pass570.runtime_test" },
        /* pass576: D-side wall table */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, true,  false, DM1_PC34_ZONE_WALL_D2L2",
          "pass576.d2l2_wall" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  true,  false, DM1_PC34_ZONE_WALL_D0L",
          "pass576.d0l_wall" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate",
          "pass576.wall_clip_gate" },
        /* pass576 runtime assertions */
        { "tests/test_dm1_v1_viewport_3d_pc34_compat.c",
          "static void test_wall_source_row_clip_occlusion_gate(void)",
          "pass576.test_wall_source_row_clip" },
        /* pass577: F0128 D0/D1 visible-square draw-order table */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1L, 1, -1",
          "pass577.d1l_visible_square" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D0C, 0,  0",
          "pass577.d0c_visible_square" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "DM1_VIEW_SQUARE_D1C,   3, 1,  8",
          "pass577.d1c_projectile" },
        /* pass577 runtime test */
        { "tests/test_dm1_v1_viewport_3d_pc34_compat.c",
          "static void test_d0_d1_visible_square_draw_order_gate(void)",
          "pass577.runtime_test" },
        /* pass510: m11_game_view.c wall parity/native flip path */
        { "src/engine/m11_game_view.c",
          "ReDMCSB DUNVIEW.C F0128: G0076_B_UseFlippedWallAndFootprintsBitmaps is set",
          "pass510.party_tuple_source_citation" },
        { "src/engine/m11_game_view.c",
          "static int m11_dm1_use_flipped_walls(const M11_GameViewState* state)",
          "pass510.party_tuple_flip_predicate" },
        { "src/engine/m11_game_view.c",
          "wallSet * M11_GFX_DM1_WALLSET_COUNT +",
          "pass510.wallset_variant_binding" },
        { "src/engine/m11_game_view.c",
          "the native center-wall graphic flipped horizontally.",
          "pass510.center_wall_flip_path" },
        { "src/engine/m11_game_view.c",
          "left zones use the right-side graphic flipped horizontally",
          "pass510.side_wall_lr_swap_path" },
    };
    for (size_t i = 0; i < sizeof(needles) / sizeof(needles[0]); ++i) {
        char id[128];
        snprintf(id, sizeof(id), "drift.%s", needles[i].id);
        if (!file_contains(needles[i].rel, needles[i].token)) {
            printf("FAIL %s missing in %s\n", id, needles[i].rel);
            ++g_failures;
        } else {
            printf("PASS %s present in %s\n", id, needles[i].rel);
        }
    }
}

int main(void)
{
    test_redmcsb_g0163_wall_frames();
    test_redmcsb_g0163_wall_frames_resolve_clip_gate();
    test_redmcsb_f0128_draw_order();
    test_f0128_d4_far_object_pass_order();
    test_f0128_draw_order_resolves_relative_map_coordinates();
    test_pc34_wall_bitmap_selection();
    test_wall_item_occlusion_alcove_exception();
    test_wall_source_row_clip_occlusion_gate();
    test_wall_draw_uses_clip_gate_source_offsets();
    test_f0099_copy_and_flip_h_preserves_row_boundaries();
    test_d3c_far_center_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d3l_d3r_far_side_wall_pixel_routes_use_redmcsb_frame_clip();
    test_d2l_side_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d2r_right_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d2c_center_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_center_wall_opaque_pixel_slices_use_pc34_no_transparency_route();
    test_d2c_closed_door_panel_uses_temp_bitmap_frame_clip();
    test_d2l2_d2r2_near_wall_pixel_and_no_thing_gate();
    test_d3l2_d3r2_far_wall_pixel_and_wall_return_gate();
    test_d1c_center_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d1r_right_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d1l_left_wall_source_clipped_no_pixel_write();
    test_d0l_narrow_side_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_d0r_narrow_side_wall_pixel_slice_uses_redmcsb_frame_clip();
    test_pc34_parity_wall_draw_uses_opposite_native_bitmap_without_temp();
    test_d0l_d0r_parity_pixel_slice_uses_redmcsb_frame_clip();
    test_f0115_cell_order_and_layer_z_order();
    test_projectile_occlusion_zone_mapping();
    test_explosion_occlusion_zone_mapping();
    test_projectile_wall_zone_movement_visibility_gate();
    test_door_front_occlusion_split_passes();
    test_side_door_stairs_occlusion_cell_orders();
    test_floor_field_stairs_pit_teleporter_order();
    pass760_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_wall_ornament();
    test_d0c_thieves_eye_door_frame_occlusion_order();
    test_parity_flip_restore();
    test_wall_frame_bitmap_global_null_guard();
    test_floor_ceiling_bands_and_zones();
    test_d0_d1_visible_square_draw_order_gate();
    test_post_command_redraw_contract();
    test_same_viewport_capture_contract();
    test_source_evidence_mentions_visual_lane();
    test_dm1_v1_viewport_3d_source_evidence_drift_regression();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_3d_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_3d_source_lock\n");
    return 0;
}
