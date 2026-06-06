#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

static void check_int(const char *label, int got, int want)
{
    if (got == want) {
        ++passed;
        printf("PASS %s == %d\n", label, want);
    } else {
        ++failed;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    }
}

static void check_true(const char *label, int value)
{
    check_int(label, value ? 1 : 0, 1);
}

static void test_config_defaults_and_setters(void)
{
    CSB_V1_ViewportConfig cfg;
    static const uint8_t grid[4] = { 0, 1, 2, 5 };

    csb_v1_viewport_init(&cfg);
    check_int("cfg.default_wall_set", cfg.wall_set_index, 0);
    check_int("cfg.default_custom_background", cfg.custom_background, 0);
    check_int("cfg.default_prison_door", cfg.prison_door_open, 0);
    check_int("cfg.default_stride", cfg.viewport_stride, 320);
    check_true("cfg.default_no_pixels", cfg.viewport_pixels == NULL);

    csb_v1_viewport_set_wall_set(&cfg, 3);
    csb_v1_viewport_set_custom_background(&cfg, 7);
    csb_v1_viewport_set_dungeon_grid(&cfg, grid, 2, 2);
    check_int("cfg.set_wall_set", cfg.wall_set_index, 3);
    check_int("cfg.set_custom_background", cfg.custom_background, 7);
    check_true("cfg.grid_ptr", cfg.dungeon_grid == grid);
    check_int("cfg.grid_width", cfg.dungeon_width, 2);
    check_int("cfg.grid_height", cfg.dungeon_height, 2);

    csb_v1_viewport_set_wall_set(NULL, 1);
    csb_v1_viewport_set_custom_background(NULL, 1);
    csb_v1_viewport_set_dungeon_grid(NULL, grid, 2, 2);
    csb_v1_viewport_render_frame(NULL, 0, 0, 0);
}

static void test_null_framebuffer_render_is_noop(void)
{
    CSB_V1_ViewportConfig cfg;

    csb_v1_viewport_init(&cfg);
    csb_v1_viewport_set_wall_set(&cfg, 4);
    csb_v1_viewport_set_custom_background(&cfg, 2);
    csb_v1_viewport_render_frame(&cfg, 1, 10, 20);

    check_int("noop.wall_set_preserved", cfg.wall_set_index, 4);
    check_int("noop.custom_background_preserved", cfg.custom_background, 2);
    check_true("noop.viewport_pixels_still_null", cfg.viewport_pixels == NULL);
}

static void test_csb_only_draw_order_and_coordinates(void)
{
    static const struct {
        size_t order_index;
        DM1_ViewSquareIndex square;
        int depth;
        int lateral;
        int north_x;
        int north_y;
        const char *fn;
        const char *line_anchor;
    } expected[] = {
        { 3, DM1_VIEW_SQUARE_D3L2, 3, -2, 8, 17, "F0676_DrawD3L2", "8478-8482" },
        { 4, DM1_VIEW_SQUARE_D3R2, 3,  2, 12, 17, "F0677_DrawD3R2", "8483-8486" },
        { 8, DM1_VIEW_SQUARE_D2L2, 2, -2, 8, 18, "F0678_DrawD2L2", "8500-8504" },
        { 9, DM1_VIEW_SQUARE_D2R2, 2,  2, 12, 18, "F0679_DrawD2R2", "8505-8508" },
    };

    check_int("csb.draw_order.count", (int)dm1_viewport_3d_draw_order_count(), 19);
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_ViewportDrawStep *step =
            dm1_viewport_3d_get_draw_order_step(expected[i].order_index);
        int16_t x = 0;
        int16_t y = 0;
        char id[96];

        snprintf(id, sizeof(id), "csb.draw_order.%zu.present", i);
        check_true(id, step != NULL);
        if (!step) continue;

        snprintf(id, sizeof(id), "csb.draw_order.%zu.square", i);
        check_int(id, (int)step->square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.draw_order.%zu.depth", i);
        check_int(id, step->rel_depth, expected[i].depth);
        snprintf(id, sizeof(id), "csb.draw_order.%zu.lateral", i);
        check_int(id, step->rel_lateral, expected[i].lateral);
        snprintf(id, sizeof(id), "csb.draw_order.%zu.function", i);
        check_true(id, strstr(step->redmcsb_function, expected[i].fn) != NULL);
        snprintf(id, sizeof(id), "csb.draw_order.%zu.source", i);
        check_true(id, strstr(step->source_lines, expected[i].line_anchor) != NULL);

        check_true("csb.relative.resolve",
                   dm1_viewport_3d_resolve_relative_map_xy(0, step->rel_depth,
                                                           step->rel_lateral,
                                                           10, 20, &x, &y));
        snprintf(id, sizeof(id), "csb.relative.%zu.north_x", i);
        check_int(id, x, expected[i].north_x);
        snprintf(id, sizeof(id), "csb.relative.%zu.north_y", i);
        check_int(id, y, expected[i].north_y);
    }

    check_int("csb.draw_order.d3l2_before_d3l",
              dm1_viewport_3d_get_draw_order_step(3)->square == DM1_VIEW_SQUARE_D3L2 &&
              dm1_viewport_3d_get_draw_order_step(5)->square == DM1_VIEW_SQUARE_D3L,
              1);
    check_int("csb.draw_order.d2l2_before_d2l",
              dm1_viewport_3d_get_draw_order_step(8)->square == DM1_VIEW_SQUARE_D2L2 &&
              dm1_viewport_3d_get_draw_order_step(10)->square == DM1_VIEW_SQUARE_D2L,
              1);
}

static void test_csb_frame_and_zone_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int left;
        int right;
        int top;
        int bottom;
        int byte_width;
        int height;
        int blit_x;
        int zone;
        const char *function_name;
        const char *source_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 15, 25, 73, 8, 49, 0, DM1_PC34_ZONE_WALL_D3L2, "F0676_DrawD3L2", "6254-6260" },
        { DM1_VIEW_SQUARE_D3R2, 208, 223, 25, 73, 8, 49, 0, DM1_PC34_ZONE_WALL_D3R2, "F0677_DrawD3R2", "6321-6327" },
        { DM1_VIEW_SQUARE_D2L2, 0, 37, 20, 90, 36, 71, 30, DM1_PC34_ZONE_WALL_D2L2, "F0678_DrawD2L2", "6849-6858" },
        { DM1_VIEW_SQUARE_D2R2, 186, 223, 20, 90, 36, 71, 0, DM1_PC34_ZONE_WALL_D2R2, "F0679_DrawD2R2", "6880-6889" },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_WallFrame *frame =
            dm1_viewport_3d_get_wall_frame(expected[i].square);
        const DM1_ViewportWallDrawSpec *spec =
            dm1_viewport_3d_get_wall_draw_spec_for_square(expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.frame.%zu.present", i);
        check_true(id, frame != NULL);
        snprintf(id, sizeof(id), "csb.wall_spec.%zu.present", i);
        check_true(id, spec != NULL);
        if (!frame || !spec) continue;

        snprintf(id, sizeof(id), "csb.frame.%zu.left", i);
        check_int(id, frame->left_x, expected[i].left);
        snprintf(id, sizeof(id), "csb.frame.%zu.right", i);
        check_int(id, frame->right_x, expected[i].right);
        snprintf(id, sizeof(id), "csb.frame.%zu.top", i);
        check_int(id, frame->top_y, expected[i].top);
        snprintf(id, sizeof(id), "csb.frame.%zu.bottom", i);
        check_int(id, frame->bottom_y, expected[i].bottom);
        snprintf(id, sizeof(id), "csb.frame.%zu.byte_width", i);
        check_int(id, frame->byte_width, expected[i].byte_width);
        snprintf(id, sizeof(id), "csb.frame.%zu.height", i);
        check_int(id, frame->height, expected[i].height);
        snprintf(id, sizeof(id), "csb.frame.%zu.blit_x", i);
        check_int(id, frame->blit_x, expected[i].blit_x);
        snprintf(id, sizeof(id), "csb.wall_spec.%zu.zone", i);
        check_int(id, spec->pc34_zone, expected[i].zone);
        snprintf(id, sizeof(id), "csb.wall_spec.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.wall_spec.%zu.source", i);
        check_true(id, strstr(spec->source_lines, expected[i].source_anchor) != NULL);
    }
}

static void test_csb_wall_ornament_route_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int zone;
        int draws_wall_ornament;
        int ornament_slot;
        int view_wall_index;
        const char *function_name;
        const char *source_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, DM1_PC34_ZONE_WALL_D3L2, 1, 1, 0, "F0676_DrawD3L2", "6254-6263" },
        { DM1_VIEW_SQUARE_D3R2, DM1_PC34_ZONE_WALL_D3R2, 1, 3, 1, "F0677_DrawD3R2", "6321-6330" },
        { DM1_VIEW_SQUARE_D2L2, DM1_PC34_ZONE_WALL_D2L2, 0, -1, -1, "F0678_DrawD2L2", "6848-6865" },
        { DM1_VIEW_SQUARE_D2R2, DM1_PC34_ZONE_WALL_D2R2, 0, -1, -1, "F0679_DrawD2R2", "6877-6896" },
    };

    check_int("csb.ornament_route.count",
              (int)csb_v1_viewport_wall_ornament_route_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportWallOrnamentRouteSpec *spec =
            csb_v1_viewport_get_wall_ornament_route_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.ornament_route.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.ornament_route.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.zone", i);
        check_int(id, spec->wall_zone, expected[i].zone);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.draws", i);
        check_int(id, spec->draws_wall_ornament, expected[i].draws_wall_ornament);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.slot", i);
        check_int(id, spec->ornament_ordinal_slot, expected[i].ornament_slot);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.view_wall", i);
        check_int(id, spec->view_wall_index, expected[i].view_wall_index);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.ornament_route.%zu.source", i);
        check_true(id, strstr(spec->source_lines, expected[i].source_anchor) != NULL);
    }

    check_true("csb.ornament_route.out_of_range",
               csb_v1_viewport_get_wall_ornament_route_spec(4) == NULL);
    check_true("csb.ornament_route.unknown_square",
               csb_v1_viewport_get_wall_ornament_route_spec_for_square(999) == NULL);
}

static void test_csb_floor_ornament_route_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int floor_view_index;
        int door_pass1;
        int door_pass2;
        int door_zone;
        const char *function_name;
        const char *floor_anchor;
        const char *door_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 0x0218, 0x0349, 3700,
          "F0676_DrawD3L2", "6270", "C3700_ZONE_DOOR_D3L2" },
        { DM1_VIEW_SQUARE_D3R2, 1, 0x0128, 0x0439, 3710,
          "F0677_DrawD3R2", "6337", "C3710_ZONE_DOOR_D3R2" },
    };

    check_int("csb.floor_ornament_route.count",
              (int)csb_v1_viewport_floor_ornament_route_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportFloorOrnamentRouteSpec *spec =
            csb_v1_viewport_get_floor_ornament_route_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.floor_view", i);
        check_int(id, spec->floor_view_index, expected[i].floor_view_index);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.corridor", i);
        check_int(id, spec->draws_corridor_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.pit_bug64", i);
        check_int(id, spec->draws_pit_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.door_front", i);
        check_int(id, spec->draws_door_front_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.door_pass1", i);
        check_int(id, spec->door_front_pass1_order, expected[i].door_pass1);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.door_pass2", i);
        check_int(id, spec->door_front_pass2_order, expected[i].door_pass2);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.door_zone", i);
        check_int(id, spec->door_zone, expected[i].door_zone);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.floor_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].floor_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.door_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].door_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.f0108", i);
        check_true(id, strstr(spec->source_lines, "F0108") != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_route.%zu.f0111", i);
        check_true(id, strstr(spec->source_lines, "F0111") != NULL);
    }

    check_true("csb.floor_ornament_route.out_of_range",
               csb_v1_viewport_get_floor_ornament_route_spec(2) == NULL);
    check_true("csb.floor_ornament_route.unknown_square",
               csb_v1_viewport_get_floor_ornament_route_spec_for_square(999) == NULL);
}

static void test_csb_thing_pass_order_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int floor_view_index;
        uint16_t rear_order;
        uint16_t front_order;
        uint16_t corridor_order;
        uint16_t side_order;
        unsigned char rear_cell0;
        unsigned char rear_cell1;
        unsigned char front_cell0;
        unsigned char front_cell1;
        unsigned char corridor_cell0;
        unsigned char corridor_cell3;
        unsigned char side_cell0;
        unsigned char side_cell2;
        const char *function_name;
        const char *source_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 0x0218, 0x0349, 0x3421, 0x0321,
          1, 2, 4, 3, 1, 3, 1, 3, "F0676_DrawD3L2", "6271 F0115" },
        { DM1_VIEW_SQUARE_D3R2, 1, 0x0128, 0x0439, 0x4312, 0x0412,
          2, 1, 3, 4, 2, 4, 2, 4, "F0677_DrawD3R2", "6338 F0115" },
    };
    const DM1_ViewportThingLayerSpec *objects =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_OBJECTS);
    const DM1_ViewportThingLayerSpec *creatures =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_CREATURES);
    const DM1_ViewportThingLayerSpec *projectiles =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_PROJECTILES);
    const DM1_ViewportThingLayerSpec *explosions =
        dm1_viewport_3d_get_thing_layer_spec(DM1_VIEWPORT_THING_LAYER_EXPLOSIONS);

    check_int("csb.thing_pass_order.count",
              (int)csb_v1_viewport_thing_pass_order_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    check_true("csb.thing_pass_order.layer_objects", objects != NULL);
    check_true("csb.thing_pass_order.layer_creatures", creatures != NULL);
    check_true("csb.thing_pass_order.layer_projectiles", projectiles != NULL);
    check_true("csb.thing_pass_order.layer_explosions", explosions != NULL);

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportThingPassOrderSpec *spec =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        DM1_ViewportCellOrder rear;
        DM1_ViewportCellOrder front;
        DM1_ViewportCellOrder corridor;
        DM1_ViewportCellOrder side;
        char id[96];

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        rear = dm1_viewport_3d_decode_cell_order(spec->door_front_rear_cell_order);
        front = dm1_viewport_3d_decode_cell_order(spec->door_front_front_cell_order);
        corridor = dm1_viewport_3d_decode_cell_order(spec->corridor_cell_order);
        side = dm1_viewport_3d_decode_cell_order(spec->side_cell_order);

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.floor_view", i);
        check_int(id, spec->floor_view_index, expected[i].floor_view_index);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.seq_f0108", i);
        check_int(id, spec->door_front_floor_ornament_order, 0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.seq_rear_f0115", i);
        check_int(id, spec->door_front_rear_f0115_order, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.seq_f0111", i);
        check_int(id, spec->door_front_f0111_order, 2);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.seq_front_f0115", i);
        check_int(id, spec->door_front_front_f0115_order, 3);

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.rear_order", i);
        check_int(id, spec->door_front_rear_cell_order, expected[i].rear_order);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.front_order", i);
        check_int(id, spec->door_front_front_cell_order, expected[i].front_order);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.corridor_order", i);
        check_int(id, spec->corridor_cell_order, expected[i].corridor_order);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.side_order", i);
        check_int(id, spec->side_cell_order, expected[i].side_order);

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.rear_pass", i);
        check_int(id, rear.door_pass, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.front_pass", i);
        check_int(id, front.door_pass, 2);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.rear_cell0", i);
        check_int(id, rear.cells[0], expected[i].rear_cell0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.rear_cell1", i);
        check_int(id, rear.cells[1], expected[i].rear_cell1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.front_cell0", i);
        check_int(id, front.cells[0], expected[i].front_cell0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.front_cell1", i);
        check_int(id, front.cells[1], expected[i].front_cell1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.corridor_count", i);
        check_int(id, corridor.cell_count, 4);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.corridor_cell0", i);
        check_int(id, corridor.cells[0], expected[i].corridor_cell0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.corridor_cell3", i);
        check_int(id, corridor.cells[3], expected[i].corridor_cell3);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.side_count", i);
        check_int(id, side.cell_count, 3);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.side_cell0", i);
        check_int(id, side.cells[0], expected[i].side_cell0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.side_cell2", i);
        check_int(id, side.cells[2], expected[i].side_cell2);

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.layer_objects", i);
        check_int(id, spec->f0115_objects_layer_order, 0);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.layer_creatures", i);
        check_int(id, spec->f0115_creatures_layer_order, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.layer_projectiles", i);
        check_int(id, spec->f0115_projectiles_layer_order, 2);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.layer_explosions", i);
        check_int(id, spec->f0115_explosions_layer_order, 3);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.explosions_after_cells", i);
        check_int(id, spec->f0115_explosions_after_all_cells, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.source", i);
        check_true(id, strstr(spec->source_lines, expected[i].source_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.f0115_source", i);
        check_true(id, strstr(spec->source_lines, "F0115:4567-4581") != NULL);
    }

    if (objects && creatures && projectiles && explosions) {
        check_int("csb.thing_pass_order.dm1_layer.objects", (int)objects->layer,
                  DM1_VIEWPORT_THING_LAYER_OBJECTS);
        check_int("csb.thing_pass_order.dm1_layer.creatures", (int)creatures->layer,
                  DM1_VIEWPORT_THING_LAYER_CREATURES);
        check_int("csb.thing_pass_order.dm1_layer.projectiles", (int)projectiles->layer,
                  DM1_VIEWPORT_THING_LAYER_PROJECTILES);
        check_int("csb.thing_pass_order.dm1_layer.explosions", (int)explosions->layer,
                  DM1_VIEWPORT_THING_LAYER_EXPLOSIONS);
        check_int("csb.thing_pass_order.dm1_layer.objects_per_cell",
                  objects->repeats_per_cell ? 1 : 0, 1);
        check_int("csb.thing_pass_order.dm1_layer.creatures_per_cell",
                  creatures->repeats_per_cell ? 1 : 0, 1);
        check_int("csb.thing_pass_order.dm1_layer.projectiles_per_cell",
                  projectiles->repeats_per_cell ? 1 : 0, 1);
        check_int("csb.thing_pass_order.dm1_layer.explosions_after_cells",
                  explosions->after_all_cells ? 1 : 0, 1);
    }

    check_true("csb.thing_pass_order.out_of_range",
               csb_v1_viewport_get_thing_pass_order_spec(2) == NULL);
    check_true("csb.thing_pass_order.unknown_square",
               csb_v1_viewport_get_thing_pass_order_spec_for_square(999) == NULL);
}

static void test_csb_object_visibility_filter_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int object_row;
        const char *function_name;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 3, "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 4, "F0677_DrawD3R2" },
    };

    check_int("csb.object_visibility.count",
              (int)csb_v1_viewport_object_visibility_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportObjectVisibilitySpec *spec =
            csb_v1_viewport_get_object_visibility_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.object_visibility.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.object_visibility.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.depth", i);
        check_int(id, spec->view_depth, 3);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.row", i);
        check_int(id, spec->object_visibility_row, expected[i].object_row);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.type_gate", i);
        check_int(id, spec->requires_item_type_range, 1);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.cell_match", i);
        check_int(id, spec->requires_thing_cell_match, 1);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.depth3_gate", i);
        check_int(id, spec->suppresses_depth3_front_cells, 1);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.depth0_gate", i);
        check_int(id, spec->suppresses_depth0_back_cells, 0);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.first_cell", i);
        check_int(id, spec->first_visible_cell_ordinal, 3);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.last_cell", i);
        check_int(id, spec->last_visible_cell_ordinal, 4);

        snprintf(id, sizeof(id), "csb.object_visibility.%zu.cell1_hidden", i);
        check_int(id, csb_v1_viewport_object_visibility_allows_cell(spec, 1), 0);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.cell2_hidden", i);
        check_int(id, csb_v1_viewport_object_visibility_allows_cell(spec, 2), 0);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.cell3_visible", i);
        check_int(id, csb_v1_viewport_object_visibility_allows_cell(spec, 3), 1);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.cell4_visible", i);
        check_int(id, csb_v1_viewport_object_visibility_allows_cell(spec, 4), 1);

        snprintf(id, sizeof(id), "csb.object_visibility.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.source_row", i);
        check_true(id, strstr(spec->source_lines, "G2028") != NULL);
        snprintf(id, sizeof(id), "csb.object_visibility.%zu.source_predicate", i);
        check_true(id, strstr(spec->source_lines, "4923 F0115") != NULL);
    }

    check_true("csb.object_visibility.null_helper",
               csb_v1_viewport_object_visibility_allows_cell(NULL, 3) == 0);
    check_true("csb.object_visibility.out_of_range",
               csb_v1_viewport_get_object_visibility_spec(2) == NULL);
    check_true("csb.object_visibility.unknown_square",
               csb_v1_viewport_get_object_visibility_spec_for_square(999) == NULL);
}

static void test_csb_f0111_door_panel_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int zone;
        int parent_record;
        int dst_x;
        const char *function_name;
        const char *zone_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 3700, 129, 24,
          "F0676_DrawD3L2", "C3700_ZONE_DOOR_D3L2" },
        { DM1_VIEW_SQUARE_D3R2, 3710, 130, 88,
          "F0677_DrawD3R2", "C3710_ZONE_DOOR_D3R2" },
    };

    check_int("csb.door_panel_blit.count",
              (int)csb_v1_viewport_door_panel_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportDoorPanelBlitSpec *spec =
            csb_v1_viewport_get_door_panel_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.order_present", i);
        check_true(id, order != NULL);
        if (!spec || !order) continue;

        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.zone", i);
        check_int(id, spec->door_zone_base, expected[i].zone);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.closed_record_type", i);
        check_int(id, spec->closed_record_type, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.parent", i);
        check_int(id, spec->closed_parent_record, expected[i].parent_record);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.parent_x", i);
        check_int(id, spec->closed_parent_x, expected[i].dst_x);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.parent_y", i);
        check_int(id, spec->closed_parent_y, 28);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.clip_record", i);
        check_int(id, spec->clip_record, 126);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.native_width", i);
        check_int(id, spec->native_bitmap_width, 48);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.native_height", i);
        check_int(id, spec->native_bitmap_height, 41);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.clipped_width", i);
        check_int(id, spec->clipped_width, 48);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.clipped_height", i);
        check_int(id, spec->clipped_height, 40);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.dst_x", i);
        check_int(id, spec->closed_dst_x, expected[i].dst_x);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.dst_y", i);
        check_int(id, spec->closed_dst_y, 28);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.ornament_index", i);
        check_int(id, spec->door_ornament_index, 0);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.skip_open", i);
        check_int(id, spec->skips_open_state, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.state_shift", i);
        check_int(id, spec->shifts_zone_by_state, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.half1_offset", i);
        check_int(id, spec->horizontal_first_half_zone_offset, 6);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.half2_offset", i);
        check_int(id, spec->horizontal_second_half_zone_offset, 3);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);

        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.order_before", i);
        check_int(id, order->door_front_rear_f0115_order < order->door_front_f0111_order, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.order_after", i);
        check_int(id, order->door_front_f0111_order < order->door_front_front_f0115_order, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.zone_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].zone_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.f0111_source", i);
        check_true(id, strstr(spec->source_lines, "F0111:4248") != NULL &&
                       strstr(spec->source_lines, "4331 F0791") != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.coord_source", i);
        check_true(id, strstr(spec->source_lines, "COORD.C") != NULL);
    }

    check_true("csb.door_panel_blit.out_of_range",
               csb_v1_viewport_get_door_panel_blit_spec(2) == NULL);
    check_true("csb.door_panel_blit.unknown_square",
               csb_v1_viewport_get_door_panel_blit_spec_for_square(999) == NULL);
}

static void test_source_evidence(void)
{
    const char *e = csb_v1_viewport_source_evidence();

    check_true("evidence.f0676", e && strstr(e, "F0676") != NULL);
    check_true("evidence.f0677", e && strstr(e, "F0677") != NULL);
    check_true("evidence.f0678", e && strstr(e, "F0678") != NULL);
    check_true("evidence.f0679", e && strstr(e, "F0679") != NULL);
    check_true("evidence.f0128", e && strstr(e, "F0128") != NULL);
    check_true("evidence.f0107", e && strstr(e, "F0107") != NULL);
    check_true("evidence.f0108", e && strstr(e, "F0108") != NULL);
    check_true("evidence.f0111", e && strstr(e, "F0111") != NULL);
    check_true("evidence.f0115_layers", e && strstr(e, "F0115 draws objects") != NULL);
    check_true("evidence.f0115_object_filter", e && strstr(e, "F0115 filters weapon..junk") != NULL);
    check_true("evidence.f0115_explosions", e && strstr(e, "F0115 restarts for explosions") != NULL);
    check_true("evidence.f0111_door_zone_records", e && strstr(e, "COORD.C:1548-1565") != NULL);
    check_true("evidence.d3l2_view_wall", e && strstr(e, "C00_VIEW_WALL_D3L2_RIGHT") != NULL);
    check_true("evidence.d3r2_view_wall", e && strstr(e, "C01_VIEW_WALL_D3R2_LEFT") != NULL);
    check_true("evidence.custom_backgrounds", e && strstr(e, "CustomBackgrounds") != NULL);
}

int main(void)
{
    test_config_defaults_and_setters();
    test_null_framebuffer_render_is_noop();
    test_csb_only_draw_order_and_coordinates();
    test_csb_frame_and_zone_contracts();
    test_csb_wall_ornament_route_contracts();
    test_csb_floor_ornament_route_contracts();
    test_csb_thing_pass_order_contracts();
    test_csb_object_visibility_filter_contracts();
    test_csb_f0111_door_panel_blit_contracts();
    test_source_evidence();

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
