#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int has_op(const DM1_V2_DrawCommand* commands,
                  int count,
                  DM1_V2_DrawOp op,
                  DM1_V2_ViewSquare square) {
    int i;
    for (i = 0; i < count; ++i) {
        if (commands[i].op == op && commands[i].square == square) {
            return 1;
        }
    }
    return 0;
}

static DM1_V2_Color pixel_for_single_square(int element,
                                            int hasObjects,
                                            int hasField,
                                            DM1_V2_ViewSquare square,
                                            int x,
                                            int y) {
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_ViewportState vp;
    DM1_V2_Color color;
    int depth = 2;
    int lateralIndex = 1;

    dm1_v2_vp_composition_init(&input);
    dm1_v2_vp_init(&vp);

    if (square == DM1_V2_VIEW_SQUARE_D1C) {
        depth = 1;
    } else if (square == DM1_V2_VIEW_SQUARE_D0C) {
        depth = 0;
    } else if (square == DM1_V2_VIEW_SQUARE_D3C) {
        depth = 3;
    }
    input.squares[depth][lateralIndex].element = element;
    input.squares[depth][lateralIndex].hasObjects = hasObjects;
    input.squares[depth][lateralIndex].hasField = hasField;

    CHECK(dm1_v2_vp_render_composition_flat(&vp, &input) == 1);
    color = dm1_v2_vp_get_pixel(&vp, x, y);
    return color;
}

static void test_source_locked_dimensions_and_square_types(void) {
    CHECK(DM1_V2_VIEWPORT_W == 224);
    CHECK(DM1_V2_VIEWPORT_H == 136);
    CHECK(DM1_V2_VIEWPORT_W / 2 == 112);

    /* ReDMCSB DEFS.H:922-941 M034_SQUARE_TYPE and DUNGEON.C:2199-2250. */
    CHECK(dm1_v2_vp_square_element_from_raw(0x00, 0) == DM1_V2_ELEMENT_WALL);
    CHECK(dm1_v2_vp_square_element_from_raw(0x20, 0) == DM1_V2_ELEMENT_CORRIDOR);
    CHECK(dm1_v2_vp_square_element_from_raw(0x40, 0) == DM1_V2_ELEMENT_PIT);
    CHECK(dm1_v2_vp_square_element_from_raw(0x68, 0) == DM1_V2_ELEMENT_STAIRS_FRONT);
    CHECK(dm1_v2_vp_square_element_from_raw(0x60, 0) == DM1_V2_ELEMENT_STAIRS_SIDE);
    CHECK(dm1_v2_vp_square_element_from_raw(0x88, 0) == DM1_V2_ELEMENT_DOOR_FRONT);
    CHECK(dm1_v2_vp_square_element_from_raw(0x80, 0) == DM1_V2_ELEMENT_DOOR_SIDE);
    CHECK(dm1_v2_vp_square_element_from_raw(0xA0, 0) == DM1_V2_ELEMENT_TELEPORTER);
    CHECK(dm1_v2_vp_square_element_from_raw(0xC0, 0) == DM1_V2_ELEMENT_WALL);
    CHECK(dm1_v2_vp_square_element_from_raw(0xC4, 0) == DM1_V2_ELEMENT_CORRIDOR);
}

static void test_draw_list_material_categories(void) {
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_DrawCommand commands[DM1_V2_MAX_DRAW_COMMANDS];
    int count;

    dm1_v2_vp_composition_init(&input);
    input.squares[3][1].element = DM1_V2_ELEMENT_WALL;
    input.squares[2][0].element = DM1_V2_ELEMENT_PIT;
    input.squares[2][2].element = DM1_V2_ELEMENT_TELEPORTER;
    input.squares[1][1].element = DM1_V2_ELEMENT_DOOR_FRONT;
    input.squares[0][1].element = DM1_V2_ELEMENT_STAIRS_FRONT;
    input.squares[1][0].hasObjects = 1;
    input.squares[1][2].hasField = 1;

    memset(commands, 0, sizeof(commands));
    count = dm1_v2_vp_emit_d0_d3_draw_list(&input, commands,
                                           DM1_V2_MAX_DRAW_COMMANDS);

    CHECK(count > 0);
    CHECK(commands[0].op == DM1_V2_DRAW_FLOOR_CEILING);
    CHECK(has_op(commands, count, DM1_V2_DRAW_WALL, DM1_V2_VIEW_SQUARE_D3C));
    CHECK(has_op(commands, count, DM1_V2_DRAW_PIT, DM1_V2_VIEW_SQUARE_D2L));
    CHECK(has_op(commands, count, DM1_V2_DRAW_FIELD, DM1_V2_VIEW_SQUARE_D2R));
    CHECK(has_op(commands, count, DM1_V2_DRAW_DOOR_FRONT, DM1_V2_VIEW_SQUARE_D1C));
    CHECK(has_op(commands, count, DM1_V2_DRAW_STAIRS_FRONT, DM1_V2_VIEW_SQUARE_D0C));
    CHECK(has_op(commands, count, DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES,
                 DM1_V2_VIEW_SQUARE_D1L));
    CHECK(has_op(commands, count, DM1_V2_DRAW_FIELD, DM1_V2_VIEW_SQUARE_D1R));
}

static void test_flat_render_material_pixels(void) {
    DM1_V2_Color c;

    c = pixel_for_single_square(DM1_V2_ELEMENT_DOOR_FRONT, 0, 0,
                                DM1_V2_VIEW_SQUARE_D1C, 112, 72);
    CHECK(c.r == 73 && c.g == 73 && c.b == 73 && c.a == 255);

    c = pixel_for_single_square(DM1_V2_ELEMENT_DOOR_FRONT, 0, 0,
                                DM1_V2_VIEW_SQUARE_D1C, 112, 20);
    CHECK(c.r == 146 && c.g == 146 && c.b == 146 && c.a == 255);

    c = pixel_for_single_square(DM1_V2_ELEMENT_PIT, 0, 0,
                                DM1_V2_VIEW_SQUARE_D2C, 112, 84);
    CHECK(c.r == 0 && c.g == 0 && c.b == 0 && c.a == 255);

    c = pixel_for_single_square(DM1_V2_ELEMENT_TELEPORTER, 0, 0,
                                DM1_V2_VIEW_SQUARE_D2C, 112, 68);
    CHECK(c.r == 0 && c.g == 0 && c.b == 0 && c.a == 255);

    c = pixel_for_single_square(DM1_V2_ELEMENT_CORRIDOR, 1, 0,
                                DM1_V2_VIEW_SQUARE_D2C, 112, 70);
    CHECK(c.r == 73 && c.g == 73 && c.b == 73 && c.a == 255);
}

int main(void) {
    test_source_locked_dimensions_and_square_types();
    test_draw_list_material_categories();
    test_flat_render_material_pixels();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_viewport_materials_pc34: ok");
    return 0;
}
