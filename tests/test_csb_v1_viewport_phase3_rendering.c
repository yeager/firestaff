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

static void test_source_evidence(void)
{
    const char *e = csb_v1_viewport_source_evidence();

    check_true("evidence.f0676", e && strstr(e, "F0676") != NULL);
    check_true("evidence.f0677", e && strstr(e, "F0677") != NULL);
    check_true("evidence.f0678", e && strstr(e, "F0678") != NULL);
    check_true("evidence.f0679", e && strstr(e, "F0679") != NULL);
    check_true("evidence.f0128", e && strstr(e, "F0128") != NULL);
    check_true("evidence.custom_backgrounds", e && strstr(e, "CustomBackgrounds") != NULL);
}

int main(void)
{
    test_config_defaults_and_setters();
    test_null_framebuffer_render_is_noop();
    test_csb_only_draw_order_and_coordinates();
    test_csb_frame_and_zone_contracts();
    test_source_evidence();

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
