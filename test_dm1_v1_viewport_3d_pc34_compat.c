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
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, 0, DM1_PC34_ZONE_WALL_D3L2, 1, 0 },
        { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, 0, DM1_PC34_ZONE_WALL_D3R2, 1, 0 },
        { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  0, DM1_PC34_ZONE_WALL_D3L,  1, 1 },
        { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  0, DM1_PC34_ZONE_WALL_D3R,  1, 1 },
        { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  1, DM1_PC34_ZONE_WALL_D3C,  1, 1 },
        { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, 0, DM1_PC34_ZONE_WALL_D2L2, 1, 0 },
        { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, 0, DM1_PC34_ZONE_WALL_D2R2, 1, 0 },
        { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  0, DM1_PC34_ZONE_WALL_D2L,  1, 1 },
        { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  0, DM1_PC34_ZONE_WALL_D2R,  1, 1 },
        { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  1, DM1_PC34_ZONE_WALL_D2C,  1, 1 },
        { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  0, DM1_PC34_ZONE_WALL_D1L,  1, 0 },
        { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  0, DM1_PC34_ZONE_WALL_D1R,  1, 0 },
        { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  1, DM1_PC34_ZONE_WALL_D1C,  0, 1 },
        { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  0, DM1_PC34_ZONE_WALL_D0L,  1, 0 },
        { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  0, DM1_PC34_ZONE_WALL_D0R,  1, 0 },
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

static void test_source_evidence_mentions_visual_lane(void)
{
    const char *e = dm1_viewport_3d_source_evidence();
    check_nonnull("source_evidence.nonnull", e);
    if (!e) return;
    check_int("source_evidence.g0163", strstr(e, "G0163_aauc_Graphic558_Frame_Walls") != NULL, 1);
    check_int("source_evidence.f0128", strstr(e, "DUNVIEW.C:8318 F0128_DUNGEONVIEW_Draw_CPSF") != NULL, 1);
    check_int("source_evidence.g2107", strstr(e, "G2107_WallSet[15]") != NULL, 1);
    check_int("source_evidence.pc34_side", strstr(e, "PC34 parity side-wall selection") != NULL, 1);
    check_int("source_evidence.f0115", strstr(e, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions") != NULL, 1);
    check_int("source_evidence.f0115_cell_order", strstr(e, "packed cell-order") != NULL, 1);
    check_int("source_evidence.f0115_projectiles", strstr(e, "projectile draw pass") != NULL, 1);
    check_int("source_evidence.f0115_explosion_global", strstr(e, "explosion pass after all ordered cells") != NULL, 1);
    check_int("source_evidence.defs_zones", strstr(e, "DEFS.H:4040-4057") != NULL, 1);
    check_int("source_evidence.occlusion", strstr(e, "wall case returns") != NULL, 1);
}

int main(void)
{
    test_redmcsb_g0163_wall_frames();
    test_redmcsb_f0128_draw_order();
    test_pc34_wall_bitmap_selection();
    test_f0115_cell_order_and_layer_z_order();
    test_parity_flip_restore();
    test_floor_ceiling_bands_and_zones();
    test_source_evidence_mentions_visual_lane();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_3d_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_3d_source_lock\n");
    return 0;
}
