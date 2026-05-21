#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

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

static void test_floor_ceiling_bands_and_zones(void)
{
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
        const char *things_line;
        const char *field_line;
        const char *wall_return_line;
        int d0c_foreground_before_things;
        int has_things_pass;
        int field_after_things;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, "F0676_DrawD3L2", 0x3421, 1, "6237-6252", "6275-6278", "6286", "6288-6289", "6253-6264", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3R2, "F0677_DrawD3R2", 0x4312, 1, "6304-6319", "6342-6345", "6353", "6355-6356", "6320-6331", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3L,  "F0116_DUNGEONVIEW_DrawSquareD3L", 0x3421, 1, "6375-6405", "6461-6472", "6480", "6482-6495", "6406-6437", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3R,  "F0117_DUNGEONVIEW_DrawSquareD3R", 0x4312, 1, "6514-6544", "6603-6614", "6622", "6624-6638", "6545-6573", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D3C,  "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", 0x3421, 1, "6666-6696", "6748-6762", "6816", "6818-6831", "6697-6720", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2L,  "F0119_DUNGEONVIEW_DrawSquareD2L", 0x3421, 1, "6914-6944", "7005-7015", "7031", "7033-7048", "6945-6973", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2R,  "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", 0x4312, 1, "7065-7095", "7198-7208", "7224", "7226-7240", "7097-7166", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2C,  "F0121_DUNGEONVIEW_DrawSquareD2C", 0x3421, 1, "7260-7288", "7343-7353", "7367-7368", "7370-7388", "7289-7312", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D2L2, "F0678_DrawD2L2", 0x0000, 0, "6846-6865", "6846-6865", "no F0115 thing pass", "6863-6865", "6848-6862", 0, 0, 0 },
        { DM1_VIEW_SQUARE_D2R2, "F0679_DrawD2R2", 0x0000, 0, "6877-6896", "6877-6896", "no F0115 thing pass", "6894-6896", "6879-6893", 0, 0, 0 },
        { DM1_VIEW_SQUARE_D1L,  "F0122_DUNGEONVIEW_DrawSquareD1L", 0x0032, 1, "7405-7435", "7510-7520", "7535-7536", "7538-7555", "7436-7460", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D1R,  "F0123_DUNGEONVIEW_DrawSquareD1R", 0x0041, 1, "7573-7603", "7678-7688", "7703-7704", "7706-7722", "7604-7628", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0L,  "F0125_DUNGEONVIEW_DrawSquareD0L", 0x0002, 0, "7978-7988", "7989-7998", "8005", "8050-8059", "8007-8038", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0R,  "F0126_DUNGEONVIEW_DrawSquareD0R", 0x0001, 0, "8082-8092", "8093-8102", "8115", "8150-8159", "8117-8144", 0, 1, 1 },
        { DM1_VIEW_SQUARE_D0C,  "F0127_DUNGEONVIEW_DrawSquareD0C", 0x0021, 0, "8241-8273", "8274-8292", "8294", "8295-8308", "8185-8240", 1, 1, 1 },
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
    test_f0115_cell_order_and_layer_z_order();
    test_projectile_occlusion_zone_mapping();
    test_explosion_occlusion_zone_mapping();
    test_projectile_wall_zone_movement_visibility_gate();
    test_door_front_occlusion_split_passes();
    test_side_door_stairs_occlusion_cell_orders();
    test_floor_field_stairs_pit_teleporter_order();
    test_d0c_thieves_eye_door_frame_occlusion_order();
    test_parity_flip_restore();
    test_floor_ceiling_bands_and_zones();
    test_d0_d1_visible_square_draw_order_gate();
    test_post_command_redraw_contract();
    test_same_viewport_capture_contract();
    test_source_evidence_mentions_visual_lane();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_3d_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_3d_source_lock\n");
    return 0;
}
