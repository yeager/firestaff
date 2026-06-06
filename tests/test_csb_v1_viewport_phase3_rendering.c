#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

int csb_v1_viewport_near_wall_d2_wall_bitmap_index(int view_square,
                                                   int use_flipped_wall_bitmaps);
int csb_v1_viewport_near_wall_d2_wall_zone(int view_square);
int csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(
    int view_square,
    int use_flipped_wall_bitmaps);

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

static void test_csb_custom_background_slot_contracts(void)
{
    static const struct {
        int room_num;
        int relative_forward;
        int relative_side;
        const char *call_anchor;
    } expected[] = {
        { 0, 3, -2, "6919 room 0" },
        { 2, 3, -1, "6920 room 2" },
        { 1, 3, 2, "6940 room 1" },
        { 3, 3, 1, "6941 room 3" },
        { 4, 3, 0, "6961 room 4" },
        { 5, 2, -2, "6981 room 5" },
        { 7, 2, -1, "6982 room 7" },
        { 6, 2, 2, "7002 room 6" },
        { 8, 2, 1, "7003 room 8" },
        { 9, 2, 0, "7023 room 9" },
        { 10, 1, -1, "7043 room 10" },
        { 11, 1, 1, "7063 room 11" },
        { 12, 1, 0, "7081 room 12" },
        { 13, 0, -1, "7102 room 13" },
        { 14, 0, 1, "7122 room 14" },
        { 15, 0, 0, "7140 room 15" },
    };

    check_int("csb.custom_background_slots.count",
              (int)csb_v1_viewport_custom_background_slot_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        char id[128];
        const CSB_V1_ViewportCustomBackgroundSlotSpec *spec =
            csb_v1_viewport_get_custom_background_slot_spec(i);
        const CSB_V1_ViewportCustomBackgroundSlotSpec *by_room =
            csb_v1_viewport_get_custom_background_slot_spec_for_room(expected[i].room_num);

        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.room", i);
        check_int(id, spec->room_num, expected[i].room_num);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.by_room", i);
        check_true(id, by_room == spec);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.call_order", i);
        check_int(id, spec->call_order, (int)i);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.forward", i);
        check_int(id, spec->relative_forward, expected[i].relative_forward);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.side", i);
        check_int(id, spec->relative_side, expected[i].relative_side);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.before_cell", i);
        check_int(id, spec->applies_before_cell_draw, 1);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.mask_slots", i);
        check_int(id, spec->source_mask_slots, 3);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.bitmap_slots", i);
        check_int(id, spec->source_bitmap_slots, 3);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.large_min", i);
        check_int(id, spec->large_bitmap_min_bytes, 7840);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.middle_min", i);
        check_int(id, spec->middle_bitmap_min_bytes, 3248);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.near_min", i);
        check_int(id, spec->near_bitmap_min_bytes, 4144);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.function", i);
        check_true(id, strstr(spec->csbwin_function, "CustomBackgrounds") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.redmcsb_floor", i);
        check_true(id, strstr(spec->source_lines, "DUNVIEW.C:8337-8339") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.redmcsb_f0098", i);
        check_true(id, strstr(spec->source_lines, "2962-3002 F0098") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.csbwin_relpos", i);
        check_true(id, strstr(spec->source_lines, "5317-5325 relpos") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.csbwin_apply", i);
        check_true(id, strstr(spec->source_lines, "6567-6615 CustomBackgrounds") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_slots.%zu.call_anchor", i);
        check_true(id, strstr(spec->source_lines, expected[i].call_anchor) != NULL);
    }

    check_true("csb.custom_background_slots.out_of_range",
               csb_v1_viewport_get_custom_background_slot_spec(16) == NULL);
    check_true("csb.custom_background_slots.unknown_room",
               csb_v1_viewport_get_custom_background_slot_spec_for_room(16) == NULL);
}

static void test_csb_custom_background_bitmap_application_contracts(void)
{
    const size_t count = csb_v1_viewport_custom_background_slot_spec_count();

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 and 8443 establish the
     * floor/ceiling baseline via F0098 lines 2962-3002. CSBWin:
     * Viewport.cpp lines 5317-5325 translate room slots, 5402-5412 selects
     * CSD/CSD-I34 background-library bitmaps, 6444-6470 composites through
     * ApplyBackground, and 6567-6615 gates the runtime skin/mask layers. */
    check_int("csb.custom_background_application.count",
              (int)csb_v1_viewport_custom_background_bitmap_application_spec_count(),
              (int)count);

    for (size_t i = 0; i < count; ++i) {
        char id[160];
        const CSB_V1_ViewportCustomBackgroundSlotSpec *slot =
            csb_v1_viewport_get_custom_background_slot_spec(i);
        const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *app =
            csb_v1_viewport_get_custom_background_bitmap_application_spec(i);
        const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *by_room =
            slot ? csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(
                       slot->room_num) :
                   NULL;

        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.present", i);
        check_true(id, app != NULL);
        if (!app || !slot) continue;

        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.matches_slot", i);
        check_true(id, app->room_slot == slot);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.by_room", i);
        check_true(id, by_room == app);

        for (int facing = 0; facing < 4; ++facing) {
            int got_x = 0;
            int got_y = 0;
            int want_x = 40;
            int want_y = 50;

            if (facing == 0) {
                want_x += slot->relative_side;
                want_y -= slot->relative_forward;
            } else if (facing == 1) {
                want_x += slot->relative_forward;
                want_y += slot->relative_side;
            } else if (facing == 2) {
                want_x -= slot->relative_side;
                want_y += slot->relative_forward;
            } else {
                want_x -= slot->relative_forward;
                want_y -= slot->relative_side;
            }

            snprintf(id, sizeof(id),
                     "csb.custom_background_application.%zu.facing%d.translate",
                     i, facing);
            check_true(id,
                       csb_v1_viewport_custom_background_translate_cell(
                           app, 40, 50, facing, &got_x, &got_y));
            snprintf(id, sizeof(id),
                     "csb.custom_background_application.%zu.facing%d.x",
                     i, facing);
            check_int(id, got_x, want_x);
            snprintf(id, sizeof(id),
                     "csb.custom_background_application.%zu.facing%d.y",
                     i, facing);
            check_int(id, got_y, want_y);
        }

        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.translation_gate", i);
        check_int(id, app->uses_csbwin_relative_translation, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.bounds_gate", i);
        check_int(id, app->checks_map_bounds_before_skin_lookup, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.default_skin", i);
        check_int(id, app->uses_cell_skin_before_default_skin, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.skin_def_id", i);
        check_int(id, app->skin_def_background_graphic_id, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.skin_min", i);
        check_int(id, app->skin_def_min_bytes, 18);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.large_idx", i);
        check_int(id, app->large_bitmap_skin_def_index, 0);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.large_mask_idx", i);
        check_int(id, app->large_mask_skin_def_index, 4);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.large_mask_min", i);
        check_int(id, app->large_mask_min_bytes, 64);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.large_bitmap_min", i);
        check_int(id, app->large_bitmap_min_bytes, slot->large_bitmap_min_bytes);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.middle_idx", i);
        check_int(id, app->middle_bitmap_skin_def_index, 2);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.middle_mask_idx", i);
        check_int(id, app->middle_mask_skin_def_index, 6);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.middle_mask_min", i);
        check_int(id, app->middle_mask_min_bytes, 64);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.middle_bitmap_min", i);
        check_int(id, app->middle_bitmap_min_bytes, slot->middle_bitmap_min_bytes);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_idx", i);
        check_int(id, app->near_bitmap_skin_def_index, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_mask_idx", i);
        check_int(id, app->near_mask_skin_def_index, 5);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_mask_min", i);
        check_int(id, app->near_mask_min_bytes, 20);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_bitmap_min", i);
        check_int(id, app->near_bitmap_min_bytes, slot->near_bitmap_min_bytes);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_gate", i);
        check_int(id, app->applies_near_layer, slot->room_num < 5);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.near_limit", i);
        check_int(id, app->near_layer_room_num_limit, 5);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.csd_bitmap", i);
        check_int(id, app->selects_csd_i34_background_bitmap, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.base_bitmap", i);
        check_int(id, app->selects_redmcsb_base_bitmap, 0);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.baseline", i);
        check_int(id, app->extends_redmcsb_floor_ceiling_baseline, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.composite", i);
        check_int(id, app->applybackground_masked_composite, 1);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.function", i);
        check_true(id, strstr(app->csbwin_function, "CustomBackgrounds") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.redmcsb_f0128", i);
        check_true(id, strstr(app->source_lines, "DUNVIEW.C:8337-8339,8443 F0128") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.redmcsb_f0098", i);
        check_true(id, strstr(app->source_lines, "2962-3002 F0098") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.csbwin_getbitmap", i);
        check_true(id, strstr(app->source_lines, "5402-5412 GetBitmap") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.csd_i34", i);
        check_true(id, strstr(app->source_lines, "CSD/CSD-I34") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.applybackground", i);
        check_true(id, strstr(app->source_lines, "6444-6470 ApplyBackground") != NULL);
        snprintf(id, sizeof(id), "csb.custom_background_application.%zu.runtime_apply", i);
        check_true(id, strstr(app->source_lines, "6567-6615 CustomBackgrounds") != NULL);
    }

    check_true("csb.custom_background_application.out_of_range",
               csb_v1_viewport_get_custom_background_bitmap_application_spec(count) == NULL);
    check_true("csb.custom_background_application.unknown_room",
               csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(16) == NULL);
    check_true("csb.custom_background_application.translate_null",
               !csb_v1_viewport_custom_background_translate_cell(NULL, 40, 50, 0, NULL, NULL));
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

static void test_csb_f0678_f0679_d2_wall_bitmap_flip_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int normal_bitmap;
        int flipped_bitmap;
        int zone;
        const char *function_name;
        const char *wall_branch_anchor;
        const char *return_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D2L2, 6, 5, 707, "F0678_DrawD2L2",
          "6848-6865", "returns without F0107" },
        { DM1_VIEW_SQUARE_D2R2, 5, 6, 708, "F0679_DrawD2R2",
          "6877-6896", "returns without F0107" },
    };

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportWallOrnamentRouteSpec *ornament_route =
            csb_v1_viewport_get_wall_ornament_route_spec_for_square(
                (int)expected[i].square);
        const CSB_V1_ViewportFloorOrnamentRouteSpec *floor_route =
            csb_v1_viewport_get_floor_ornament_route_spec_for_square(
                (int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *thing_order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square(
                (int)expected[i].square);
        const CSB_V1_ViewportTeleporterFieldSpec *teleporter =
            csb_v1_viewport_get_teleporter_field_spec_for_square(
                (int)expected[i].square);
        char id[128];

        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.ornament_route", i);
        check_true(id, ornament_route != NULL);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.teleporter_route", i);
        check_true(id, teleporter != NULL);
        if (!ornament_route || !teleporter) continue;

        /* ReDMCSB: DUNVIEW.C F0678 lines 6848-6862 and F0679 lines
         * 6879-6893 draw only the D2L2/D2R2 wall bitmap, swapping
         * G2107[C06_WALL_D2L2] and G2107[C05_WALL_D2R2] when G0076 is
         * set, then returning before the teleporter F0113 cases at
         * lines 6863-6865 and 6894-6896. DEFS.H lines 3428-3429 define
         * C05/C06; DEFS.H lines 4047-4048 define C707/C708. */
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.normal_bitmap", i);
        check_int(id,
                  csb_v1_viewport_near_wall_d2_wall_bitmap_index(
                      (int)expected[i].square, 0),
                  expected[i].normal_bitmap);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.flipped_bitmap", i);
        check_int(id,
                  csb_v1_viewport_near_wall_d2_wall_bitmap_index(
                      (int)expected[i].square, 1),
                  expected[i].flipped_bitmap);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.zone", i);
        check_int(id, csb_v1_viewport_near_wall_d2_wall_zone((int)expected[i].square),
                  expected[i].zone);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.normal_blitter", i);
        check_int(id,
                  csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(
                      (int)expected[i].square, 0),
                  0);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.flipped_blitter", i);
        check_int(id,
                  csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(
                      (int)expected[i].square, 1),
                  1);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.no_ornament", i);
        check_int(id, ornament_route->draws_wall_ornament, 0);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.no_floor_route", i);
        check_true(id, floor_route == NULL);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.no_thing_order", i);
        check_true(id, thing_order == NULL);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.teleporter_separate", i);
        check_int(id, teleporter->after_thing_pass, 0);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.function", i);
        check_true(id, strstr(ornament_route->redmcsb_function,
                              expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.source_wall_branch", i);
        check_true(id, strstr(ornament_route->source_lines,
                              expected[i].wall_branch_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.f0678_f0679_d2_wall_flip.%zu.source_return", i);
        check_true(id, strstr(ornament_route->source_lines,
                              expected[i].return_anchor) != NULL);
    }

    check_int("csb.f0678_f0679_d2_wall_flip.unknown_bitmap",
              csb_v1_viewport_near_wall_d2_wall_bitmap_index(999, 0), -1);
    check_int("csb.f0678_f0679_d2_wall_flip.unknown_zone",
              csb_v1_viewport_near_wall_d2_wall_zone(999), -1);
    check_int("csb.f0678_f0679_d2_wall_flip.unknown_blitter",
              csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(999, 1), -1);
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

static void test_csb_f0107_wall_ornament_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int view_wall_index;
        int flip;
        int zone0;
        int zone3;
        const char *ordinal_slot;
        const char *function_name;
        const char *source_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 0, 1004, 1049,
          "M551_RIGHT_WALL_ORNAMENT_ORDINAL", "F0676_DrawD3L2", "6263 F0107" },
        { DM1_VIEW_SQUARE_D3R2, 1, 1, 1005, 1050,
          "M553_LEFT_WALL_ORNAMENT_ORDINAL", "F0677_DrawD3R2", "6330 F0107" },
    };

    check_int("csb.wall_ornament_blit.count",
              (int)csb_v1_viewport_wall_ornament_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportWallOrnamentBlitSpec *spec =
            csb_v1_viewport_get_wall_ornament_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportWallOrnamentRouteSpec *route =
            csb_v1_viewport_get_wall_ornament_route_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.route_present", i);
        check_true(id, route != NULL);
        if (!spec || !route) continue;

        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.route_draws", i);
        check_int(id, route->draws_wall_ornament, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.view_wall", i);
        check_int(id, spec->view_wall_index, expected[i].view_wall_index);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.zero_skip", i);
        check_int(id, spec->ordinal_zero_skips_blit, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.index_delta", i);
        check_int(id, spec->ordinal_to_index_delta, -1);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.native_increment", i);
        check_int(id, spec->native_bitmap_index_increment, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.zone0", i);
        check_int(id, csb_v1_viewport_wall_ornament_blit_zone(spec, 0),
                  expected[i].zone0);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.zone3", i);
        check_int(id, csb_v1_viewport_wall_ornament_blit_zone(spec, 3),
                  expected[i].zone3);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.native", i);
        check_int(id, csb_v1_viewport_wall_ornament_native_bitmap_index(spec, 200),
                  200);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.scale_x", i);
        check_int(id, spec->scale_x, 30);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.scale_y", i);
        check_int(id, spec->scale_y, 14);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.flip", i);
        check_int(id, spec->horizontal_flip, expected[i].flip);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.scaled_bitmap", i);
        check_int(id, spec->uses_scaled_bitmap, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.f0791", i);
        check_int(id, spec->uses_f0791_blit, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.slot", i);
        check_true(id, strstr(spec->ornament_ordinal_slot,
                              expected[i].ordinal_slot) != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_blit.%zu.source", i);
        check_true(id, strstr(spec->source_lines, expected[i].source_anchor) != NULL &&
                       strstr(spec->source_lines, "3571 skips ordinal 0") != NULL &&
                       strstr(spec->source_lines, "3587 zone C1004") != NULL &&
                       strstr(spec->source_lines, "3824-3829 F0675") != NULL &&
                       strstr(spec->source_lines, "3921-3923 F0791") != NULL);
    }

    {
        const CSB_V1_ViewportWallOrnamentBlitSpec *left =
            csb_v1_viewport_get_wall_ornament_blit_spec_for_square((int)DM1_VIEW_SQUARE_D3L2);
        const CSB_V1_ViewportWallOrnamentBlitSpec *right =
            csb_v1_viewport_get_wall_ornament_blit_spec_for_square((int)DM1_VIEW_SQUARE_D3R2);
        const uint8_t left_src[6] = { 10, 11, 12, 13, 10, 14 };
        const uint8_t right_src[6] = { 21, 10, 22, 23, 24, 10 };
        uint8_t left_dst[6] = { 99, 99, 99, 99, 99, 99 };
        uint8_t right_dst[6] = { 77, 77, 77, 77, 77, 77 };

        check_int("csb.wall_ornament_blit.pixel_left.copied",
                  csb_v1_viewport_wall_ornament_blit_pixels(left, left_src, 3,
                                                            left_dst, 3, 3, 2),
                  4);
        check_int("csb.wall_ornament_blit.pixel_left.transparent0", left_dst[0], 99);
        check_int("csb.wall_ornament_blit.pixel_left.copy1", left_dst[1], 11);
        check_int("csb.wall_ornament_blit.pixel_left.copy2", left_dst[2], 12);
        check_int("csb.wall_ornament_blit.pixel_left.copy3", left_dst[3], 13);
        check_int("csb.wall_ornament_blit.pixel_left.transparent4", left_dst[4], 99);
        check_int("csb.wall_ornament_blit.pixel_left.copy5", left_dst[5], 14);

        check_int("csb.wall_ornament_blit.pixel_right.copied",
                  csb_v1_viewport_wall_ornament_blit_pixels(right, right_src, 3,
                                                            right_dst, 3, 3, 2),
                  4);
        check_int("csb.wall_ornament_blit.pixel_right.flip0", right_dst[0], 22);
        check_int("csb.wall_ornament_blit.pixel_right.transparent1", right_dst[1], 77);
        check_int("csb.wall_ornament_blit.pixel_right.flip2", right_dst[2], 21);
        check_int("csb.wall_ornament_blit.pixel_right.transparent3", right_dst[3], 77);
        check_int("csb.wall_ornament_blit.pixel_right.flip4", right_dst[4], 24);
        check_int("csb.wall_ornament_blit.pixel_right.flip5", right_dst[5], 23);
    }

    check_int("csb.wall_ornament_blit.zone_null",
              csb_v1_viewport_wall_ornament_blit_zone(NULL, 0), -1);
    check_int("csb.wall_ornament_blit.zone_bad_coord",
              csb_v1_viewport_wall_ornament_blit_zone(
                  csb_v1_viewport_get_wall_ornament_blit_spec(0), -1), -1);
    check_int("csb.wall_ornament_blit.native_null",
              csb_v1_viewport_wall_ornament_native_bitmap_index(NULL, 200), -1);
    check_int("csb.wall_ornament_blit.native_bad_base",
              csb_v1_viewport_wall_ornament_native_bitmap_index(
                  csb_v1_viewport_get_wall_ornament_blit_spec(0), -1), -1);
    check_int("csb.wall_ornament_blit.pixel_invalid",
              csb_v1_viewport_wall_ornament_blit_pixels(NULL, NULL, 0, NULL, 0, 0, 0), -1);
    check_true("csb.wall_ornament_blit.out_of_range",
               csb_v1_viewport_get_wall_ornament_blit_spec(2) == NULL);
    check_true("csb.wall_ornament_blit.unknown_square",
               csb_v1_viewport_get_wall_ornament_blit_spec_for_square(999) == NULL);
}

static void test_csb_f0107_wall_ornament_d3_side_effect_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int view_wall_index;
        const char *function_name;
        const char *route_anchor;
        const char *defs_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, "F0676_DrawD3L2", "6263 calls F0107", "DEFS.H:2696" },
        { DM1_VIEW_SQUARE_D3R2, 1, "F0677_DrawD3R2", "6330 calls F0107", "DEFS.H:2697" },
    };

    check_int("csb.wall_ornament_side_effect.count",
              (int)csb_v1_viewport_wall_ornament_side_effect_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportWallOrnamentSideEffectSpec *spec =
            csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportWallOrnamentBlitSpec *blit =
            csb_v1_viewport_get_wall_ornament_blit_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.blit_present", i);
        check_true(id, blit != NULL);
        if (!spec || !blit) continue;

        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.view_wall", i);
        check_int(id, spec->view_wall_index, expected[i].view_wall_index);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.blit_view_wall_match", i);
        check_int(id, spec->view_wall_index, blit->view_wall_index);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.alcove_predicate", i);
        check_int(id, spec->evaluates_alcove_predicate, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_facing_alcove", i);
        check_int(id, spec->updates_facing_alcove_state, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_vi_altar", i);
        check_int(id, spec->updates_facing_vi_altar_state, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_fountain", i);
        check_int(id, spec->updates_facing_fountain_state, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_clickbox", i);
        check_int(id, spec->updates_wall_clickbox, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_portrait", i);
        check_int(id, spec->draws_champion_portrait_overlay, 0);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.i34_d3_path", i);
        check_int(id, spec->uses_d3_i34_scaled_bitmap_path, 1);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.no_cache_slot", i);
        check_int(id, spec->derived_bitmap_cache_slot, -1);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);

        /* ReDMCSB: DUNVIEW.C F0107 lines 3589 and 3608-3753
         * compute alcove status but keep facing/clickbox state inside the
         * D1 branch; 3817-3829 is the CSB/I34 D3 scaled-bitmap path, and
         * 3923-3928 is only the M587 D1C champion portrait overlay. */
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.route_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].route_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.alcove_source", i);
        check_true(id, strstr(spec->source_lines, "3589 evaluates F0149") != NULL &&
                       strstr(spec->source_lines, "DUNGEON.C:1330-1347") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.d1_state_source", i);
        check_true(id, strstr(spec->source_lines, "3608 gates D1-only") != NULL &&
                       strstr(spec->source_lines, "3726-3744") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.scaled_path_source", i);
        check_true(id, strstr(spec->source_lines, "3817-3829") != NULL &&
                       strstr(spec->source_lines, "CM1_DERIVED_BITMAP_NONE") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.portrait_source", i);
        check_true(id, strstr(spec->source_lines, "3923-3928") != NULL &&
                       strstr(spec->source_lines, "M587_VIEW_WALL_D1C_FRONT") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_side_effect.%zu.defs_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].defs_anchor) != NULL);
    }

    check_true("csb.wall_ornament_side_effect.out_of_range",
               csb_v1_viewport_get_wall_ornament_side_effect_spec(2) == NULL);
    check_true("csb.wall_ornament_side_effect.unknown_square",
               csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square(999) == NULL);
}

static void test_csb_f0107_wall_ornament_d1d2_path_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int view_wall_index;
        int slot;
        int returns_alcove;
        int uses_d2_scaled;
        int uses_d1_native;
        int native_increment;
        int derived_increment;
        int scale;
        int flip;
        int state;
        int clickbox;
        int portrait;
        int zone0;
        int zone2;
        const char *function_name;
        const char *route_anchor;
        const char *defs_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D2L, 7, 1, 0, 1, 0, 0, 2, 21, 0, 0, 0, 0,
          1011, 1041, "F0119_DrawSquareD2L", "6968 side F0107", "DEFS.H:2703" },
        { DM1_VIEW_SQUARE_D2L, 9, 2, 1, 1, 0, 1, 3, 21, 0, 0, 0, 0,
          1013, 1043, "F0119_DrawSquareD2L", "6969 front F0107", "DEFS.H:2705" },
        { DM1_VIEW_SQUARE_D2R, 8, 3, 0, 1, 0, 0, 2, 21, 1, 0, 0, 0,
          1012, 1042, "F0120_DrawSquareD2R", "7119 side F0107", "DEFS.H:2704" },
        { DM1_VIEW_SQUARE_D2R, 11, 2, 1, 1, 0, 1, 3, 21, 0, 0, 0, 0,
          1015, 1045, "F0120_DrawSquareD2R", "7120 front F0107", "DEFS.H:2707" },
        { DM1_VIEW_SQUARE_D2C, 10, 2, 1, 1, 0, 1, 3, 21, 0, 0, 0, 0,
          1014, 1044, "F0121_DrawSquareD2C", "7308 front F0107", "DEFS.H:2706" },
        { DM1_VIEW_SQUARE_D1L, 12, 1, 0, 0, 1, 0, -1, 0, 0, 0, 0, 0,
          1016, 1046, "F0122_DrawSquareD1L", "7459 side F0107", "DEFS.H:2708" },
        { DM1_VIEW_SQUARE_D1R, 13, 3, 0, 0, 1, 0, -1, 0, 1, 0, 0, 0,
          1017, 1047, "F0123_DrawSquareD1R", "7627 side F0107", "DEFS.H:2709" },
        { DM1_VIEW_SQUARE_D1C, 14, 2, 1, 0, 1, 1, -1, 0, 0, 1, 1, 1,
          1018, 1048, "F0124_DrawSquareD1C", "7842 front F0107", "DEFS.H:2710" },
    };

    check_int("csb.wall_ornament_d1d2.count",
              (int)csb_v1_viewport_wall_ornament_d1d2_path_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));

    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportWallOrnamentD1D2PathSpec *spec =
            csb_v1_viewport_get_wall_ornament_d1d2_path_spec(i);
        char id[96];

        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.present", i);
        check_true(id, spec != NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.view_wall", i);
        check_int(id, spec->view_wall_index, expected[i].view_wall_index);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.slot", i);
        check_int(id, spec->ornament_ordinal_slot, expected[i].slot);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.return", i);
        check_int(id, spec->returns_alcove_to_square_draw, expected[i].returns_alcove);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.d2_scaled", i);
        check_int(id, spec->uses_d2_scaled_bitmap_path, expected[i].uses_d2_scaled);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.d1_native", i);
        check_int(id, spec->uses_d1_native_bitmap_path, expected[i].uses_d1_native);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.native_inc", i);
        check_int(id, spec->native_bitmap_index_increment, expected[i].native_increment);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.derived_inc", i);
        check_int(id, spec->derived_bitmap_index_increment, expected[i].derived_increment);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.scale", i);
        check_int(id, spec->scale, expected[i].scale);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.flip", i);
        check_int(id, spec->horizontal_flip, expected[i].flip);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.d1_state", i);
        check_int(id, spec->updates_d1_front_state, expected[i].state);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.clickbox", i);
        check_int(id, spec->updates_wall_clickbox, expected[i].clickbox);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.portrait", i);
        check_int(id, spec->draws_champion_portrait_overlay, expected[i].portrait);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.zone0", i);
        check_int(id, csb_v1_viewport_wall_ornament_d1d2_path_zone(spec, 0),
                  expected[i].zone0);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.zone2", i);
        check_int(id, csb_v1_viewport_wall_ornament_d1d2_path_zone(spec, 2),
                  expected[i].zone2);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);

        /* ReDMCSB: DUNVIEW.C F0107 lines 3571-3589 build the
         * ordinal->map-index and C1004 zone; lines 3817-3860 are the
         * D2 scaled path, lines 3608-3760 are the D1 native path, and
         * lines 3923-3928 are D1C-front-only clickbox/portrait work. */
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.route_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].route_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.ordinal_source", i);
        check_true(id, strstr(spec->source_lines, "3571-3589") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.zone_source", i);
        check_true(id, strstr(spec->source_lines, "COORD.C:921-1025") != NULL);
        snprintf(id, sizeof(id), "csb.wall_ornament_d1d2.%zu.defs_source", i);
        check_true(id, strstr(spec->source_lines, expected[i].defs_anchor) != NULL);
    }

    check_true("csb.wall_ornament_d1d2.out_of_range",
               csb_v1_viewport_get_wall_ornament_d1d2_path_spec(8) == NULL);
    check_int("csb.wall_ornament_d1d2.null_zone",
              csb_v1_viewport_wall_ornament_d1d2_path_zone(NULL, 0), -1);
    check_int("csb.wall_ornament_d1d2.bad_coord",
              csb_v1_viewport_wall_ornament_d1d2_path_zone(
                  csb_v1_viewport_get_wall_ornament_d1d2_path_spec(0), -1), -1);
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

static void test_csb_f0108_floor_ornament_bitmap_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int floor_view_index;
        int zone_coordinate_set0;
        int flip;
        const char *function_name;
        const char *door_slot;
        const char *route_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 1500, 0,
          "F0676_DrawD3L2", "M552_FRONT_WALL_ORNAMENT_ORDINAL", "6270 F0108" },
        { DM1_VIEW_SQUARE_D3R2, 1, 1501, 1,
          "F0677_DrawD3R2", "M558_FLOOR_ORNAMENT_ORDINAL", "6337 F0108" },
    };

    check_int("csb.floor_ornament_blit.count",
              (int)csb_v1_viewport_floor_ornament_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportFloorOrnamentBlitSpec *spec =
            csb_v1_viewport_get_floor_ornament_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportFloorOrnamentRouteSpec *route =
            csb_v1_viewport_get_floor_ornament_route_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.route_present", i);
        check_true(id, route != NULL);
        if (!spec || !route) continue;

        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.floor_view", i);
        check_int(id, spec->floor_view_index, expected[i].floor_view_index);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.ordinal_zero", i);
        check_int(id, spec->ordinal_zero_skips_blit, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.ordinal_delta", i);
        check_int(id, spec->ordinal_to_index_delta, -1);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.bitmap_increment", i);
        check_int(id, spec->native_bitmap_index_increment, 0);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.bitmap_index", i);
        check_int(id, csb_v1_viewport_floor_ornament_native_bitmap_index(spec, 42), 42);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.coord_set", i);
        check_int(id, spec->coordinate_set_index, 0);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.zone_base", i);
        check_int(id, spec->zone_base, 1500);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.coord_stride", i);
        check_int(id, spec->coordinate_set_stride, 11);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.zone_set0", i);
        check_int(id, csb_v1_viewport_floor_ornament_blit_zone(spec, 0),
                  expected[i].zone_coordinate_set0);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.flip", i);
        check_int(id, spec->horizontal_flip, expected[i].flip);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.door_slot", i);
        check_true(id, strstr(spec->door_front_ordinal_slot, expected[i].door_slot) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.corridor_slot", i);
        check_true(id, strstr(spec->corridor_pit_ordinal_slot, "M558_FLOOR_ORNAMENT_ORDINAL") != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.route_corridor", i);
        check_int(id, route->draws_corridor_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.route_pit_bug64", i);
        check_int(id, route->draws_pit_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.route_door_front", i);
        check_int(id, route->draws_door_front_floor_ornament, 1);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.route_anchor", i);
        check_true(id, strstr(spec->source_lines, expected[i].route_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.f0108_source", i);
        check_true(id, strstr(spec->source_lines, "3965") != NULL &&
                       strstr(spec->source_lines, "3998 F0791") != NULL &&
                       strstr(spec->source_lines, "G0191") != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.coord_set_source", i);
        check_true(id, strstr(spec->source_lines, "G0195:1008-1017") != NULL);
        snprintf(id, sizeof(id), "csb.floor_ornament_blit.%zu.zone_source", i);
        check_true(id, strstr(spec->source_lines, "C1500") != NULL &&
                       strstr(spec->source_lines, "COORD.C:903-913") != NULL);
    }

    check_int("csb.floor_ornament_blit.null_zone",
              csb_v1_viewport_floor_ornament_blit_zone(NULL, 0), -1);
    check_int("csb.floor_ornament_blit.bad_zone",
              csb_v1_viewport_floor_ornament_blit_zone(
                  csb_v1_viewport_get_floor_ornament_blit_spec(0), -1), -1);
    check_int("csb.floor_ornament_blit.null_bitmap",
              csb_v1_viewport_floor_ornament_native_bitmap_index(NULL, 42), -1);
    check_int("csb.floor_ornament_blit.bad_bitmap",
              csb_v1_viewport_floor_ornament_native_bitmap_index(
                  csb_v1_viewport_get_floor_ornament_blit_spec(0), -1), -1);
    check_true("csb.floor_ornament_blit.out_of_range",
               csb_v1_viewport_get_floor_ornament_blit_spec(2) == NULL);
    check_true("csb.floor_ornament_blit.unknown_square",
               csb_v1_viewport_get_floor_ornament_blit_spec_for_square(999) == NULL);
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
        int projectile_row;
        int projectile_zone2;
        int projectile_zone3;
        const char *function_name;
        const char *source_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0, 0x0218, 0x0349, 0x3421, 0x0321,
          1, 2, 4, 3, 1, 3, 1, 3, 3, 2914, 2915,
          "F0676_DrawD3L2", "6271 F0115" },
        { DM1_VIEW_SQUARE_D3R2, 1, 0x0128, 0x0439, 0x4312, 0x0412,
          2, 1, 3, 4, 2, 4, 2, 4, 4, 2918, 2919,
          "F0677_DrawD3R2", "6338 F0115" },
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
        const DM1_ViewportProjectileOcclusionSpec *projectile_spec =
            dm1_viewport_3d_get_projectile_occlusion_spec_for_square(expected[i].square);
        DM1_ViewportCellOrder rear;
        DM1_ViewportCellOrder front;
        DM1_ViewportCellOrder corridor;
        DM1_ViewportCellOrder side;
        char id[96];

        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_present", i);
        check_true(id, projectile_spec != NULL);
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
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_row", i);
        check_int(id, spec->f0115_projectile_g2028_row, expected[i].projectile_row);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone_base", i);
        check_int(id, spec->f0115_projectile_zone_base, 2900);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone_stride", i);
        check_int(id, spec->f0115_projectile_zone_stride, 4);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_restart", i);
        check_int(id, spec->f0115_projectile_restarts_thing_list, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_cell_match", i);
        check_int(id, spec->f0115_projectile_requires_cell_match, 1);
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_depth3_gate", i);
        check_int(id, spec->f0115_projectile_suppresses_depth3_front_cells, 1);
        if (projectile_spec) {
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_dm1_row", i);
            check_int(id, projectile_spec->g2028_row, spec->f0115_projectile_g2028_row);
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone0_hidden", i);
            check_int(id, dm1_viewport_3d_projectile_zone_for_cell(projectile_spec, 0), -1);
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone1_hidden", i);
            check_int(id, dm1_viewport_3d_projectile_zone_for_cell(projectile_spec, 1), -1);
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone2", i);
            check_int(id, dm1_viewport_3d_projectile_zone_for_cell(projectile_spec, 2),
                      expected[i].projectile_zone2);
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_zone3", i);
            check_int(id, dm1_viewport_3d_projectile_zone_for_cell(projectile_spec, 3),
                      expected[i].projectile_zone3);
            snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_dm1_source", i);
            check_true(id, strstr(projectile_spec->source_lines, "5683") != NULL);
        }
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
        snprintf(id, sizeof(id), "csb.thing_pass_order.%zu.projectile_source", i);
        check_true(id, strstr(spec->source_lines, "5683 C2900 zone") != NULL &&
                       strstr(spec->source_lines, "5881-5883 blit") != NULL);
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

static void test_csb_f0115_object_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int object_row;
        int layout_cell3;
        int layout_cell4;
        int raw_cell3;
        int raw_cell4;
        const char *function_name;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 3, 2515, 2516, 35283, 35284,
          "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 4, 2519, 2520, 35287, 35288,
          "F0677_DrawD3R2" },
    };

    check_int("csb.object_blit.count",
              (int)csb_v1_viewport_object_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportObjectBlitSpec *spec =
            csb_v1_viewport_get_object_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportObjectVisibilitySpec *visibility =
            csb_v1_viewport_get_object_visibility_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.object_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.visibility_present", i);
        check_true(id, visibility != NULL);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.order_present", i);
        check_true(id, order != NULL);
        if (!spec || !visibility || !order) continue;

        snprintf(id, sizeof(id), "csb.object_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.depth", i);
        check_int(id, spec->view_depth, 3);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.row", i);
        check_int(id, spec->object_visibility_row, expected[i].object_row);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.visibility_row_match", i);
        check_int(id, spec->object_visibility_row, visibility->object_visibility_row);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.after_floor", i);
        check_int(id, order->f0115_objects_layer_order, 0);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.zone_base", i);
        check_int(id, spec->object_zone_base, 2500);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.zone_stride", i);
        check_int(id, spec->object_zone_cell_stride, 4);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.shift_mask", i);
        check_int(id, spec->shifts_objects_and_creatures, 0x8000);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.shift_set", i);
        check_int(id, spec->shift_set_index, 5);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.pile_shift_step", i);
        check_int(id, spec->pile_shift_advances_per_object, 1);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.uses_f0791", i);
        check_int(id, spec->uses_f0791_blit, 1);

        snprintf(id, sizeof(id), "csb.object_blit.%zu.layout_cell3", i);
        check_int(id, csb_v1_viewport_object_blit_layout_zone(spec, 3),
                  expected[i].layout_cell3);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.layout_cell4", i);
        check_int(id, csb_v1_viewport_object_blit_layout_zone(spec, 4),
                  expected[i].layout_cell4);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.raw_cell3", i);
        check_int(id, csb_v1_viewport_object_blit_zone(spec, 3),
                  expected[i].raw_cell3);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.raw_cell4", i);
        check_int(id, csb_v1_viewport_object_blit_zone(spec, 4),
                  expected[i].raw_cell4);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.source_predicate", i);
        check_true(id, strstr(spec->source_lines, "4923 F0115") != NULL);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.source_zone", i);
        check_true(id, strstr(spec->source_lines, "5071-5082 C2500_ZONE_") != NULL &&
                       strstr(spec->source_lines, "MASK0x8000") != NULL);
        snprintf(id, sizeof(id), "csb.object_blit.%zu.source_blit", i);
        check_true(id, strstr(spec->source_lines, "5109 F0791") != NULL &&
                       strstr(spec->source_lines, "COORD.C:1129-1193") != NULL);
    }

    check_int("csb.object_blit.null_layout",
              csb_v1_viewport_object_blit_layout_zone(NULL, 3), -1);
    check_int("csb.object_blit.bad_layout_cell",
              csb_v1_viewport_object_blit_layout_zone(
                  csb_v1_viewport_get_object_blit_spec(0), 5), -1);
    check_int("csb.object_blit.null_raw",
              csb_v1_viewport_object_blit_zone(NULL, 3), -1);
    check_true("csb.object_blit.out_of_range",
               csb_v1_viewport_get_object_blit_spec(2) == NULL);
    check_true("csb.object_blit.unknown_square",
               csb_v1_viewport_get_object_blit_spec_for_square(999) == NULL);
}

static void test_csb_f0115_projectile_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int projectile_row;
        int zone_cell2;
        int zone_cell3;
        int zone_cell4;
        const char *function_name;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 3, 2914, 2915, 2916, "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 4, 2918, 2919, 2920, "F0677_DrawD3R2" },
    };

    check_int("csb.projectile_blit.count",
              (int)csb_v1_viewport_projectile_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportProjectileBlitSpec *spec =
            csb_v1_viewport_get_projectile_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.order_present", i);
        check_true(id, order != NULL);
        if (!spec || !order) continue;

        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.depth", i);
        check_int(id, spec->view_depth, 3);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.row", i);
        check_int(id, spec->projectile_visibility_row, expected[i].projectile_row);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.order_row_match", i);
        check_int(id, spec->projectile_visibility_row, order->f0115_projectile_g2028_row);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.zone_base", i);
        check_int(id, spec->projectile_zone_base, 2900);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.zone_stride", i);
        check_int(id, spec->projectile_zone_cell_stride, 4);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.projectile_type", i);
        check_int(id, spec->requires_projectile_type, 1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.cell_match", i);
        check_int(id, spec->requires_thing_cell_match, 1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.restart", i);
        check_int(id, spec->restarts_thing_list, 1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.front_suppression", i);
        check_int(id, spec->suppresses_depth3_front_cells, 1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.d0_suppression", i);
        check_int(id, spec->suppresses_depth0_back_cells, 0);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.derived_none", i);
        check_int(id, spec->derived_bitmap_cache_slot_for_scaled_path, -1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.uses_f0791", i);
        check_int(id, spec->uses_f0791_blit, 1);

        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.reject_cell0", i);
        check_int(id, csb_v1_viewport_projectile_blit_zone(spec, 0), -1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.reject_cell1", i);
        check_int(id, csb_v1_viewport_projectile_blit_zone(spec, 1), -1);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.zone_cell2", i);
        check_int(id, csb_v1_viewport_projectile_blit_zone(spec, 2),
                  expected[i].zone_cell2);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.zone_cell3", i);
        check_int(id, csb_v1_viewport_projectile_blit_zone(spec, 3),
                  expected[i].zone_cell3);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.zone_cell4", i);
        check_int(id, csb_v1_viewport_projectile_blit_zone(spec, 4),
                  expected[i].zone_cell4);

        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.source_zone", i);
        check_true(id, strstr(spec->source_lines, "5668-5683 F0115") != NULL &&
                       strstr(spec->source_lines, "C2900_ZONE_ + row*4 + ViewCell") != NULL);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.source_scale", i);
        check_true(id, strstr(spec->source_lines, "5710-5722") != NULL);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.source_blit", i);
        check_true(id, strstr(spec->source_lines, "5881-5882 F0791") != NULL &&
                       strstr(spec->source_lines, "C10 transparency") != NULL);
        snprintf(id, sizeof(id), "csb.projectile_blit.%zu.source_coord", i);
        check_true(id, strstr(spec->source_lines, "COORD.C:1194-1239") != NULL);
    }

    {
        const uint8_t source[6] = { 1, 10, 2, 3, 4, 5 };
        uint8_t destination[6] = { 88, 88, 88, 88, 88, 88 };
        const CSB_V1_ViewportProjectileBlitSpec *spec =
            csb_v1_viewport_get_projectile_blit_spec(0);

        /* ReDMCSB: DUNVIEW.C F0115 lines 5755-5762 and 5791-5802 set
         * horizontal/vertical flip bits; lines 5881-5882 forward them to
         * F0791 with C10 transparency for the PC34/I34 projectile blit. */
        check_int("csb.projectile_blit.synthetic_pixels",
                  csb_v1_viewport_projectile_blit_pixels(
                      spec, 0x0001 | 0x0002, source, 3, destination, 3, 3, 2),
                  5);
        check_int("csb.projectile_blit.synthetic_pixel_0", destination[0], 5);
        check_int("csb.projectile_blit.synthetic_pixel_1", destination[1], 4);
        check_int("csb.projectile_blit.synthetic_pixel_2", destination[2], 3);
        check_int("csb.projectile_blit.synthetic_pixel_3", destination[3], 2);
        check_int("csb.projectile_blit.synthetic_transparent", destination[4], 88);
        check_int("csb.projectile_blit.synthetic_pixel_5", destination[5], 1);
    }

    check_int("csb.projectile_blit.null_zone",
              csb_v1_viewport_projectile_blit_zone(NULL, 2), -1);
    check_int("csb.projectile_blit.bad_cell",
              csb_v1_viewport_projectile_blit_zone(
                  csb_v1_viewport_get_projectile_blit_spec(0), 5), -1);
    check_int("csb.projectile_blit.null_pixels",
              csb_v1_viewport_projectile_blit_pixels(NULL, 0, NULL, 0, NULL, 0, 0, 0), -1);
    check_int("csb.projectile_blit.bad_stride",
              csb_v1_viewport_projectile_blit_pixels(
                  csb_v1_viewport_get_projectile_blit_spec(0), 0,
                  (const uint8_t *)"x", 2, (uint8_t *)"x", 3, 3, 1), -1);
    check_true("csb.projectile_blit.out_of_range",
               csb_v1_viewport_get_projectile_blit_spec(2) == NULL);
    check_true("csb.projectile_blit.unknown_square",
               csb_v1_viewport_get_projectile_blit_spec_for_square(999) == NULL);
}

static void test_csb_creature_visibility_zone_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int creature_row;
        int zone_cell2;
        int zone_cell3;
        int coord1_cell4;
        const char *function_name;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 3, 3217, 3218, 3284, "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 4, 3222, 3223, 3289, "F0677_DrawD3R2" },
    };

    check_int("csb.creature_visibility.count",
              (int)csb_v1_viewport_creature_visibility_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportCreatureVisibilitySpec *spec =
            csb_v1_viewport_get_creature_visibility_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.order_present", i);
        check_true(id, order != NULL);
        if (!spec || !order) continue;

        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.depth", i);
        check_int(id, spec->view_depth, 3);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.row", i);
        check_int(id, spec->creature_visibility_row, expected[i].creature_row);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.group_marker", i);
        check_int(id, spec->requires_group_marker, 1);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.reject_missing_row", i);
        check_int(id, spec->rejects_missing_creature_row, 1);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.zone_base", i);
        check_int(id, spec->creature_zone_base, 3200);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.coord_stride", i);
        check_int(id, spec->creature_coordinate_set_stride, 65);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.cell_stride", i);
        check_int(id, spec->creature_zone_cell_stride, 5);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.shift_mask", i);
        check_int(id, spec->shifts_objects_and_creatures, 0x8000);

        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.zone_cell2", i);
        check_int(id, csb_v1_viewport_creature_visibility_zone(spec, 0, 2),
                  expected[i].zone_cell2);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.zone_cell3", i);
        check_int(id, csb_v1_viewport_creature_visibility_zone(spec, 0, 3),
                  expected[i].zone_cell3);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.coord1_cell4", i);
        check_int(id, csb_v1_viewport_creature_visibility_zone(spec, 1, 4),
                  expected[i].coord1_cell4);

        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.after_objects", i);
        check_int(id, order->f0115_objects_layer_order < order->f0115_creatures_layer_order, 1);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.before_projectiles", i);
        check_int(id, order->f0115_creatures_layer_order < order->f0115_projectiles_layer_order, 1);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.source_g2033", i);
        check_true(id, strstr(spec->source_lines, "G2033") != NULL);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.source_group", i);
        check_true(id, strstr(spec->source_lines, "4840-4842") != NULL);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.source_zone", i);
        check_true(id, strstr(spec->source_lines, "5615-5627 C3200_ZONE_") != NULL &&
                       strstr(spec->source_lines, "MASK0x8000") != NULL);
        snprintf(id, sizeof(id), "csb.creature_visibility.%zu.source_coord", i);
        check_true(id, strstr(spec->source_lines, "COORD.C:1248-1251") != NULL &&
                       strstr(spec->source_lines, "2074-2075") != NULL);
    }

    check_int("csb.creature_visibility.null_zone",
              csb_v1_viewport_creature_visibility_zone(NULL, 0, 2), -1);
    check_int("csb.creature_visibility.bad_coord",
              csb_v1_viewport_creature_visibility_zone(
                  csb_v1_viewport_get_creature_visibility_spec(0), -1, 2), -1);
    check_int("csb.creature_visibility.bad_cell",
              csb_v1_viewport_creature_visibility_zone(
                  csb_v1_viewport_get_creature_visibility_spec(0), 0, 5), -1);
    check_true("csb.creature_visibility.out_of_range",
               csb_v1_viewport_get_creature_visibility_spec(2) == NULL);
    check_true("csb.creature_visibility.unknown_square",
               csb_v1_viewport_get_creature_visibility_spec_for_square(999) == NULL);
}

static void test_csb_f0115_explosion_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int explosion_row;
        int field_aspect;
        int field_zone;
        int rebirth_step1_zone;
        int rebirth_step2_zone;
        int centered_zone;
        int side_zone0;
        int side_zone1;
        const char *function_name;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 6, 0, 702, 3006, 3013, 3020, 3043, 3044,
          "F0676_DrawD3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 7, 1, 703, 3007, 3014, 3021, 3045, 3046,
          "F0677_DrawD3R2" },
    };

    check_int("csb.explosion_blit.count",
              (int)csb_v1_viewport_explosion_blit_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportExplosionBlitSpec *spec =
            csb_v1_viewport_get_explosion_blit_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.order_present", i);
        check_true(id, order != NULL);
        if (!spec || !order) continue;

        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.depth", i);
        check_int(id, spec->view_depth, 3);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.row", i);
        check_int(id, spec->explosion_row, expected[i].explosion_row);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.field_aspect", i);
        check_int(id, spec->field_aspect_index, expected[i].field_aspect);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.after_cells", i);
        check_int(id, spec->restarts_thing_list_after_cells, 1);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.order_after_cells", i);
        check_int(id, order->f0115_explosions_after_all_cells, 1);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.rebirth_cell_match", i);
        check_int(id, spec->rebirth_requires_visible_row_and_cell_match, 1);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.fluxcage_defers", i);
        check_int(id, spec->fluxcage_defers_to_field, 1);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.field_zone", i);
        check_int(id, spec->fluxcage_field_zone, expected[i].field_zone);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.step1_base", i);
        check_int(id, spec->rebirth_step1_zone_base, 3000);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.step2_base", i);
        check_int(id, spec->rebirth_step2_zone_base, 3007);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.centered_base", i);
        check_int(id, spec->centered_zone_base, 3014);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.side_base", i);
        check_int(id, spec->side_zone_base, 3031);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.side_stride", i);
        check_int(id, spec->side_zone_cell_stride, 2);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.uses_f0791", i);
        check_int(id, spec->uses_f0791_blit, 1);

        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.rebirth_step1_zone", i);
        check_int(id, csb_v1_viewport_explosion_rebirth_step1_zone(spec),
                  expected[i].rebirth_step1_zone);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.rebirth_step2_zone", i);
        check_int(id, csb_v1_viewport_explosion_rebirth_step2_zone(spec),
                  expected[i].rebirth_step2_zone);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.centered_zone", i);
        check_int(id, csb_v1_viewport_explosion_centered_zone(spec),
                  expected[i].centered_zone);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.side_zone0", i);
        check_int(id, csb_v1_viewport_explosion_side_zone(spec, 0),
                  expected[i].side_zone0);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.side_zone1", i);
        check_int(id, csb_v1_viewport_explosion_side_zone(spec, 1),
                  expected[i].side_zone1);

        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.source_restart", i);
        check_true(id, strstr(spec->source_lines, "5915-5933") != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.source_rows", i);
        check_true(id, strstr(spec->source_lines, "5920-5924 G2034/G2035") != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.source_zones", i);
        check_true(id, strstr(spec->source_lines, "C3000") != NULL &&
                       strstr(spec->source_lines, "C3031") != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.source_blit", i);
        check_true(id, strstr(spec->source_lines, "6192-6193 F0791") != NULL);
        snprintf(id, sizeof(id), "csb.explosion_blit.%zu.source_fluxcage", i);
        check_true(id, strstr(spec->source_lines, "6202-6219 fluxcage") != NULL);
    }

    check_int("csb.explosion_blit.null_step1",
              csb_v1_viewport_explosion_rebirth_step1_zone(NULL), -1);
    check_int("csb.explosion_blit.null_step2",
              csb_v1_viewport_explosion_rebirth_step2_zone(NULL), -1);
    check_int("csb.explosion_blit.null_centered",
              csb_v1_viewport_explosion_centered_zone(NULL), -1);
    check_int("csb.explosion_blit.null_side",
              csb_v1_viewport_explosion_side_zone(NULL, 0), -1);
    check_int("csb.explosion_blit.bad_side_cell",
              csb_v1_viewport_explosion_side_zone(
                  csb_v1_viewport_get_explosion_blit_spec(0), 2), -1);
    check_true("csb.explosion_blit.out_of_range",
               csb_v1_viewport_get_explosion_blit_spec(2) == NULL);
    check_true("csb.explosion_blit.unknown_square",
               csb_v1_viewport_get_explosion_blit_spec_for_square(999) == NULL);
}

static void test_csb_teleporter_field_route_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int redmcsb_index;
        int field_aspect;
        int field_zone;
        int has_floor_route;
        int has_thing_order;
        int after_thing_pass;
        const char *function_name;
        const char *route_anchor;
        const char *f0108_anchor;
        const char *f0115_anchor;
        const char *zone_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 14, 0, 702, 1, 1, 1, "F0676_DrawD3L2",
          "6288-6290", "6284 F0108", "6286 F0115", "C702_ZONE_WALL_D3L2" },
        { DM1_VIEW_SQUARE_D3R2, 15, 1, 703, 1, 1, 1, "F0677_DrawD3R2",
          "6355-6357", "6351 F0108", "6353 F0115", "C703_ZONE_WALL_D3R2" },
        { DM1_VIEW_SQUARE_D2L2, 9, 5, 707, 0, 0, 0, "F0678_DrawD2L2",
          "6863-6865", "without F0108", "without F0115", "C707_ZONE_WALL_D2L2" },
        { DM1_VIEW_SQUARE_D2R2, 10, 6, 708, 0, 0, 0, "F0679_DrawD2R2",
          "6894-6896", "without F0108", "without F0115", "C708_ZONE_WALL_D2R2" },
    };

    check_int("csb.teleporter_field.count",
              (int)csb_v1_viewport_teleporter_field_spec_count(),
              (int)(sizeof(expected) / sizeof(expected[0])));
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const CSB_V1_ViewportTeleporterFieldSpec *spec =
            csb_v1_viewport_get_teleporter_field_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportFloorOrnamentRouteSpec *floor_route =
            csb_v1_viewport_get_floor_ornament_route_spec_for_square((int)expected[i].square);
        const CSB_V1_ViewportThingPassOrderSpec *thing_order =
            csb_v1_viewport_get_thing_pass_order_spec_for_square((int)expected[i].square);
        char id[96];

        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.present", i);
        check_true(id, spec != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.floor_route_present", i);
        check_true(id, expected[i].has_floor_route ? floor_route != NULL : floor_route == NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.thing_order_present", i);
        check_true(id, expected[i].has_thing_order ? thing_order != NULL : thing_order == NULL);
        if (!spec) continue;

        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.square", i);
        check_int(id, spec->view_square, (int)expected[i].square);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.redmcsb_index", i);
        check_int(id, spec->redmcsb_view_square_index, expected[i].redmcsb_index);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.teleporter_only", i);
        check_int(id, spec->draws_only_for_teleporter, 1);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.after_thing_pass", i);
        check_int(id, spec->after_thing_pass, expected[i].after_thing_pass);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.field_aspect", i);
        check_int(id, spec->field_aspect_index, expected[i].field_aspect);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.field_zone", i);
        check_int(id, spec->field_zone, expected[i].field_zone);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.uses_f0113", i);
        check_int(id, spec->uses_f0113_draw_field, 1);
        if (floor_route) {
            snprintf(id, sizeof(id), "csb.teleporter_field.%zu.floor_branch", i);
            check_int(id, floor_route->draws_corridor_floor_ornament, 1);
        }
        if (thing_order) {
            snprintf(id, sizeof(id), "csb.teleporter_field.%zu.thing_branch", i);
            check_int(id, thing_order->corridor_cell_order > 0, 1);
        }
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.function", i);
        check_true(id, strstr(spec->redmcsb_function, expected[i].function_name) != NULL);

        /* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6288-6290 and 6355-6357
         * draw F0113 after D3L2/D3R2 F0108/F0115 work. F0678/F0679 lines
         * 6863-6865 and 6894-6896 draw D2L2/D2R2 teleporter fields with
         * no floor/thing pass. Line 377 supplies G2035; DEFS.H 4042-4048
         * supplies the C702/C703/C707/C708 wall zones. */
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_route", i);
        check_true(id, strstr(spec->source_lines, expected[i].route_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_f0108", i);
        check_true(id, strstr(spec->source_lines, expected[i].f0108_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_f0115", i);
        check_true(id, strstr(spec->source_lines, expected[i].f0115_anchor) != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_g2035", i);
        check_true(id, strstr(spec->source_lines, "DUNVIEW.C:377 G2035") != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_f0113", i);
        check_true(id, strstr(spec->source_lines, "4382-4409 F0113") != NULL);
        snprintf(id, sizeof(id), "csb.teleporter_field.%zu.source_zone", i);
        check_true(id, strstr(spec->source_lines, expected[i].zone_anchor) != NULL);
    }

    check_true("csb.teleporter_field.out_of_range",
               csb_v1_viewport_get_teleporter_field_spec(4) == NULL);
    check_true("csb.teleporter_field.unknown_square",
               csb_v1_viewport_get_teleporter_field_spec_for_square(999) == NULL);
}

static void test_csb_f0111_door_panel_blit_contracts(void)
{
    static const struct {
        DM1_ViewSquareIndex square;
        int zone;
        int parent_record;
        int dst_x;
        int vertical_state2_zone;
        int horizontal_state2_first_zone;
        int horizontal_state2_final_zone;
        const char *function_name;
        const char *zone_anchor;
    } expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 3700, 129, 24, 3702, 3708, 20089,
          "F0676_DrawD3L2", "C3700_ZONE_DOOR_D3L2" },
        { DM1_VIEW_SQUARE_D3R2, 3710, 130, 88, 3712, 3718, 20099,
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
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_state", i);
        check_int(id, spec->destroyed_state, 5);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_mask", i);
        check_int(id, spec->destroyed_mask_ornament_ordinal, 15);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_view_slot", i);
        check_int(id, spec->destroyed_mask_uses_view_ornament_index, 1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.transparent", i);
        check_int(id, spec->transparent_color, 10);

        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.open_first_zone", i);
        check_int(id, csb_v1_viewport_door_panel_first_half_zone(spec, 0, 1), -1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.open_final_zone", i);
        check_int(id, csb_v1_viewport_door_panel_final_zone(spec, 0, 1), -1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.closed_final_zone", i);
        check_int(id, csb_v1_viewport_door_panel_final_zone(spec, 4, 0), expected[i].zone);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_final_zone", i);
        check_int(id, csb_v1_viewport_door_panel_final_zone(spec, spec->destroyed_state, 1),
                  expected[i].zone);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.vertical_partial_first_zone", i);
        check_int(id, csb_v1_viewport_door_panel_first_half_zone(spec, 2, 0), -1);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.vertical_partial_final_zone", i);
        check_int(id, csb_v1_viewport_door_panel_final_zone(spec, 2, 0),
                  expected[i].vertical_state2_zone);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.horizontal_partial_first_zone", i);
        check_int(id, csb_v1_viewport_door_panel_first_half_zone(spec, 2, 1),
                  expected[i].horizontal_state2_first_zone);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.horizontal_partial_final_zone", i);
        check_int(id, csb_v1_viewport_door_panel_final_zone(spec, 2, 1),
                  expected[i].horizontal_state2_final_zone);

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
                       strstr(spec->source_lines, "4298-4321") != NULL &&
                       strstr(spec->source_lines, "4334 F0791") != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_source", i);
        check_true(id, strstr(spec->source_lines, "4301-4302") != NULL &&
                       strstr(spec->source_lines, "C15 destroyed mask") != NULL);
        snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.coord_source", i);
        check_true(id, strstr(spec->source_lines, "COORD.C") != NULL);

        {
            uint8_t source[48 * 41];
            uint8_t destination[48 * 41];
            for (size_t j = 0; j < sizeof(source); ++j) source[j] = 10;
            for (size_t j = 0; j < sizeof(destination); ++j) destination[j] = 77;
            source[0] = 1;
            source[47] = 2;
            source[(39 * 48) + 0] = 3;
            source[(39 * 48) + 47] = 4;
            source[40 * 48] = 5;

            /* ReDMCSB: DUNVIEW.C F0111 lines 4248 and 4334, with
             * COORD.C lines 1556-1560. State 0 draws nothing; non-open
             * states blit the D3 48x41 source through the 48x40 panel clip
             * and preserve C10 transparent pixels in the destination. */
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.closed_pixels", i);
            check_int(id, csb_v1_viewport_door_panel_blit_pixels(
                          spec, 4, source, 48, destination, 48), 4);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.pixel_0_0", i);
            check_int(id, destination[0], 1);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.transparent_0_1", i);
            check_int(id, destination[1], 77);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.pixel_47_0", i);
            check_int(id, destination[47], 2);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.pixel_0_39", i);
            check_int(id, destination[39 * 48], 3);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.pixel_47_39", i);
            check_int(id, destination[(39 * 48) + 47], 4);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.clips_row40", i);
            check_int(id, destination[40 * 48], 77);

            for (size_t j = 0; j < sizeof(destination); ++j) destination[j] = 66;
            /* ReDMCSB: DUNVIEW.C F0111 lines 4301-4302 applies
             * C15_DOOR_ORNAMENT_DESTROYED_MASK to P0128_i_ViewDoorOrnamentIndex
             * for C5_DOOR_STATE_DESTROYED, then still reaches the final F0791
             * transparent blit at line 4334. */
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_pixels", i);
            check_int(id, csb_v1_viewport_door_panel_blit_pixels(
                          spec, spec->destroyed_state, source, 48, destination, 48), 4);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_pixel_0_0", i);
            check_int(id, destination[0], 1);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.destroyed_transparent", i);
            check_int(id, destination[1], 66);

            for (size_t j = 0; j < sizeof(destination); ++j) destination[j] = 88;
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.open_skips", i);
            check_int(id, csb_v1_viewport_door_panel_blit_pixels(
                          spec, 0, source, 48, destination, 48), 0);
            snprintf(id, sizeof(id), "csb.door_panel_blit.%zu.open_preserves", i);
            check_int(id, destination[0], 88);
        }
    }

    check_int("csb.door_panel_blit.null_pixels",
              csb_v1_viewport_door_panel_blit_pixels(NULL, 4, NULL, 0, NULL, 0), -1);
    check_int("csb.door_panel_blit.bad_source_stride",
              csb_v1_viewport_door_panel_blit_pixels(
                  csb_v1_viewport_get_door_panel_blit_spec(0), 4, (const uint8_t *)"x", 47,
                  (uint8_t *)"x", 48), -1);
    check_int("csb.door_panel_blit.null_first_zone",
              csb_v1_viewport_door_panel_first_half_zone(NULL, 2, 1), -1);
    check_int("csb.door_panel_blit.bad_first_zone_state",
              csb_v1_viewport_door_panel_first_half_zone(
                  csb_v1_viewport_get_door_panel_blit_spec(0), -1, 1), -1);
    check_int("csb.door_panel_blit.null_final_zone",
              csb_v1_viewport_door_panel_final_zone(NULL, 2, 1), -1);
    check_int("csb.door_panel_blit.bad_final_zone_state",
              csb_v1_viewport_door_panel_final_zone(
                  csb_v1_viewport_get_door_panel_blit_spec(0), -1, 1), -1);
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
    check_true("evidence.f0115_projectiles", e && strstr(e, "C2900_ZONE_ + G2028") != NULL);
    check_true("evidence.f0115_projectile_flip",
               e && strstr(e, "MASK0x0001/MASK0x0002 flip flags") != NULL);
    check_true("evidence.f0115_object_filter", e && strstr(e, "F0115 filters weapon..junk") != NULL);
    check_true("evidence.f0115_object_blit", e && strstr(e, "C2500_ZONE_ | MASK0x8000") != NULL);
    check_true("evidence.f0115_object_coord", e && strstr(e, "COORD.C:1129-1193") != NULL);
    check_true("evidence.f0115_creatures", e && strstr(e, "G2033 and C3200_ZONE_") != NULL);
    check_true("evidence.f0115_creature_shift", e && strstr(e, "MASK0x8000_SHIFT_OBJECTS_AND_CREATURES") != NULL);
    check_true("evidence.f0115_explosions", e && strstr(e, "F0115 restarts for explosions") != NULL);
    check_true("evidence.f0115_explosion_zones", e && strstr(e, "C3000/C3007/C3014/C3031") != NULL);
    check_true("evidence.f0115_fluxcage", e && strstr(e, "fluxcage field deferral") != NULL);
    check_true("evidence.teleporter_fields",
               e && strstr(e, "draw teleporter fields through G2035") != NULL);
    check_true("evidence.f0678_f0679_d2_wall_bitmap_flip",
               e && strstr(e, "D2L2/D2R2 wall branches swap C06/C05") != NULL);
    check_true("evidence.d2_wall_bitmap_defs",
               e && strstr(e, "DEFS.H:3428-3429 C05_WALL_D2R2 / C06_WALL_D2L2") != NULL);
    check_true("evidence.f0107_wall_ornament_blit", e && strstr(e, "F0107 maps CSB/I34") != NULL);
    check_true("evidence.c1004_wall_ornament", e && strstr(e, "C1004_ZONE_WALL_ORNAMENT") != NULL);
    check_true("evidence.f0107_side_effects",
               e && strstr(e, "skips D1-only facing state") != NULL);
    check_true("evidence.f0107_d1d2_paths",
               e && strstr(e, "F0119-F0124 call F0107 for D2/D1") != NULL);
    check_true("evidence.f0107_d2_scaled",
               e && strstr(e, "D2 uses C21/G0199 derived scaled bitmaps") != NULL);
    check_true("evidence.f0107_d1_native",
               e && strstr(e, "D1 side uses native CM1_DERIVED_BITMAP_NONE") != NULL);
    check_true("evidence.d1d2_view_walls",
               e && strstr(e, "DEFS.H:2703-2710 M580..M587") != NULL);
    check_true("evidence.f0149_alcove",
               e && strstr(e, "F0149_DUNGEON_IsWallOrnamentAnAlcove") != NULL);
    check_true("evidence.f0108_bitmap_index", e && strstr(e, "G0191 native bitmap increment") != NULL);
    check_true("evidence.f0108_c1500", e && strstr(e, "C1500_ZONE_FLOOR_ORNAMENT") != NULL);
    check_true("evidence.f0108_g0195", e && strstr(e, "G0195 CSB/I34") != NULL);
    check_true("evidence.f0111_door_zone_records", e && strstr(e, "COORD.C:1548-1565") != NULL);
    check_true("evidence.f0111_destroyed_mask",
               e && strstr(e, "C15_DOOR_ORNAMENT_DESTROYED_MASK") != NULL);
    check_true("evidence.d3l2_view_wall", e && strstr(e, "C00_VIEW_WALL_D3L2_RIGHT") != NULL);
    check_true("evidence.d3r2_view_wall", e && strstr(e, "C01_VIEW_WALL_D3R2_LEFT") != NULL);
    check_true("evidence.custom_backgrounds", e && strstr(e, "CustomBackgrounds") != NULL);
    check_true("evidence.custom_backgrounds_redmcsb_floor",
               e && strstr(e, "8337-8339 F0128") != NULL);
    check_true("evidence.custom_backgrounds_redmcsb_f0098",
               e && strstr(e, "F0098 2962-3002") != NULL);
    check_true("evidence.custom_backgrounds_csbwin_relpos",
               e && strstr(e, "5317-5325 relposSid/relposFwd") != NULL);
    check_true("evidence.custom_backgrounds_csbwin_getbitmap",
               e && strstr(e, "5402-5412 GetBitmap CSD/CSD-I34") != NULL);
    check_true("evidence.custom_backgrounds_csbwin_applybackground",
               e && strstr(e, "6444-6470 ApplyBackground masked composite") != NULL);
    check_true("evidence.custom_backgrounds_csbwin_slots",
               e && strstr(e, "6919-7140 sixteen background room slots") != NULL);
}

int main(void)
{
    test_config_defaults_and_setters();
    test_null_framebuffer_render_is_noop();
    test_csb_custom_background_slot_contracts();
    test_csb_custom_background_bitmap_application_contracts();
    test_csb_only_draw_order_and_coordinates();
    test_csb_frame_and_zone_contracts();
    test_csb_f0678_f0679_d2_wall_bitmap_flip_contracts();
    test_csb_wall_ornament_route_contracts();
    test_csb_f0107_wall_ornament_blit_contracts();
    test_csb_f0107_wall_ornament_d3_side_effect_contracts();
    test_csb_f0107_wall_ornament_d1d2_path_contracts();
    test_csb_floor_ornament_route_contracts();
    test_csb_f0108_floor_ornament_bitmap_blit_contracts();
    test_csb_thing_pass_order_contracts();
    test_csb_object_visibility_filter_contracts();
    test_csb_f0115_object_blit_contracts();
    test_csb_f0115_projectile_blit_contracts();
    test_csb_creature_visibility_zone_contracts();
    test_csb_f0115_explosion_blit_contracts();
    test_csb_teleporter_field_route_contracts();
    test_csb_f0111_door_panel_blit_contracts();
    test_source_evidence();

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
