#include "dm1_v1_viewport_3d_pc34_compat.h"

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

static void test_door_front_occlusion_split_passes(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        const char *rear_line;
        const char *frame_line;
        const char *door_line;
        const char *front_line;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L, "6444", "6446", "6457", "6459" },
        { DM1_VIEW_SQUARE_D3C, "6723", "6725", "6744", "6746" },
        { DM1_VIEW_SQUARE_D2C, "7315", "7317", "7339", "7341" },
        { DM1_VIEW_SQUARE_D1C, "7875", "7877", "7905", "7937" },
    };

    check_int("door_front_occlusion.count", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 4);
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportDoorFrontOcclusionSpec *spec =
            dm1_viewport_3d_get_door_front_occlusion_spec_for_square(expected[i].square);
        DM1_ViewportCellOrder rear;
        DM1_ViewportCellOrder front;
        char id[96];
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.nonnull", i);
        check_nonnull(id, spec);
        if (!spec) continue;
        check_int("door_front_occlusion.rear_order", spec->rear_cell_order, 0x0218);
        check_int("door_front_occlusion.front_order", spec->front_cell_order, 0x0349);
        rear = dm1_viewport_3d_decode_cell_order(spec->rear_cell_order);
        front = dm1_viewport_3d_decode_cell_order(spec->front_cell_order);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_pass", i);
        check_int(id, rear.door_pass, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_cells", i);
        check_int(id, rear.cell_count == 2 && rear.cells[0] == 1 && rear.cells[1] == 2, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_pass", i);
        check_int(id, front.door_pass, 2);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_cells", i);
        check_int(id, front.cell_count == 2 && front.cells[0] == 4 && front.cells[1] == 3, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.rear_line", i);
        check_int(id, strstr(spec->rear_pass_source_lines, expected[i].rear_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.frame_line", i);
        check_int(id, strstr(spec->frame_source_lines, expected[i].frame_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.door_line", i);
        check_int(id, strstr(spec->door_source_lines, expected[i].door_line) != NULL, 1);
        snprintf(id, sizeof(id), "door_front_occlusion.%zu.front_line", i);
        check_int(id, strstr(spec->front_pass_source_lines, expected[i].front_line) != NULL, 1);
    }
    check_int("door_front_occlusion.out_of_range", dm1_viewport_3d_get_door_front_occlusion_spec(4) == NULL, 1);
    check_int("door_front_occlusion.no_side_door_spec", dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1L) == NULL, 1);
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
    check_int("post_command_redraw.mainloop_source", strstr(spec->main_loop_source_lines, "GAMELOOP.C:55-90") != NULL, 1);
    check_int("post_command_redraw.present_source", strstr(spec->present_source_lines, "DRAWVIEW.C:709-722") != NULL, 1);
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
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, "F0676_DrawD3L2", 0x3421, 1, "6237-6252", "6275-6278", "6286", "6288-6289", "6253-6264" },
        { DM1_VIEW_SQUARE_D3L,  "F0116_DUNGEONVIEW_DrawSquareD3L", 0x3421, 1, "6375-6405", "6461-6472", "6480", "6482-6495", "6406-6437" },
        { DM1_VIEW_SQUARE_D3C,  "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", 0x3421, 1, "6666-6696", "6748-6762", "6816", "6818-6831", "6697-6720" },
        { DM1_VIEW_SQUARE_D2C,  "F0121_DUNGEONVIEW_DrawSquareD2C", 0x3421, 1, "7260-7288", "7343-7353", "7367-7368", "7370-7388", "7289-7312" },
        { DM1_VIEW_SQUARE_D0C,  "F0127_DUNGEONVIEW_DrawSquareD0C", 0x0021, 0, "8241-8273", "8274-8292", "8294", "8295-8308", "8185-8240" },
    };

    check_int("floor_field_order.count", (int)dm1_viewport_3d_floor_field_order_spec_count(), 5);
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
        check_int(id, spec->stairs_draw_before_floor_ornament ? 1 : 0, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.pit_before_floor", i);
        check_int(id, spec->pit_draw_before_floor_ornament ? 1 : 0, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.floor_before_things", i);
        check_int(id, spec->floor_ornament_before_things ? 1 : 0, expected[i].has_floor_ornament);
        snprintf(id, sizeof(id), "floor_field_order.%zu.layer_z", i);
        check_int(id, spec->objects_creatures_projectiles_before_explosions ? 1 : 0, 1);
        snprintf(id, sizeof(id), "floor_field_order.%zu.field_after_things", i);
        check_int(id, spec->field_after_things ? 1 : 0, 1);
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
    check_int("floor_field_order.out_of_range", dm1_viewport_3d_get_floor_field_order_spec(5) == NULL, 1);
    check_int("floor_field_order.no_d1_side_spec", dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D1L) == NULL, 1);
}

static void test_source_evidence_mentions_visual_lane(void)
{
    const char *e = dm1_viewport_3d_source_evidence();
    check_nonnull("source_evidence.nonnull", e);
    if (!e) return;
    check_int("source_evidence.g0163", strstr(e, "G0163_aauc_Graphic558_Frame_Walls") != NULL, 1);
    check_int("source_evidence.f0128", strstr(e, "DUNVIEW.C:8318 F0128_DUNGEONVIEW_Draw_CPSF") != NULL, 1);
    check_int("source_evidence.g2107", strstr(e, "G2107_WallSet[15]") != NULL, 1);
    check_int("source_evidence.pc34_side", strstr(e, "PC34 parity side-wall selection") != NULL, 1);
    check_int("source_evidence.pc34_d2_side", strstr(e, "F0678/F0679 PC34 D2L2/D2R2") != NULL, 1);
    check_int("source_evidence.f0115", strstr(e, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions") != NULL, 1);
    check_int("source_evidence.f0115_cell_order", strstr(e, "packed cell-order") != NULL, 1);
    check_int("source_evidence.f0115_projectiles", strstr(e, "projectile draw pass") != NULL, 1);
    check_int("source_evidence.projectile_occlusion", strstr(e, "G2028 row and C2900 zone mapping") != NULL, 1);
    check_int("source_evidence.f0115_explosion_global", strstr(e, "explosion pass after all ordered cells") != NULL, 1);
    check_int("source_evidence.floor_field_order", strstr(e, "stairs/pit/floor-ornament/F0115/teleporter-field order") != NULL, 1);
    check_int("source_evidence.d0c_field_order", strstr(e, "DUNVIEW.C:8241-8308") != NULL, 1);
    check_int("source_evidence.door_front_occlusion", strstr(e, "door-front occlusion") != NULL, 1);
    check_int("source_evidence.d1c_door_front_occlusion", strstr(e, "DUNVIEW.C:7874-7937") != NULL, 1);
    check_int("source_evidence.defs_zones", strstr(e, "DEFS.H:4040-4057") != NULL, 1);
    check_int("source_evidence.occlusion", strstr(e, "wall case returns") != NULL, 1);
    check_int("source_evidence.command_dispatch", strstr(e, "COMMAND.C:2045-2156") != NULL, 1);
    check_int("source_evidence.next_redraw", strstr(e, "GAMELOOP.C:55-90") != NULL, 1);
    check_int("source_evidence.present_wait", strstr(e, "DRAWVIEW.C:709-722") != NULL, 1);
}

int main(void)
{
    test_redmcsb_g0163_wall_frames();
    test_redmcsb_f0128_draw_order();
    test_pc34_wall_bitmap_selection();
    test_f0115_cell_order_and_layer_z_order();
    test_projectile_occlusion_zone_mapping();
    test_door_front_occlusion_split_passes();
    test_floor_field_stairs_pit_teleporter_order();
    test_parity_flip_restore();
    test_floor_ceiling_bands_and_zones();
    test_post_command_redraw_contract();
    test_source_evidence_mentions_visual_lane();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_3d_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_3d_source_lock\n");
    return 0;
}
