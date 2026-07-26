#include "dm1_v1_dungeon_square_structs_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t corridor_reader(int map_x, int map_y, void *user_data)
{
    (void)user_data;
    (void)map_x;
    (void)map_y;
    return (DM1_ELEMENT_CORRIDOR << 5);
}

static uint8_t wall_reader(int map_x, int map_y, void *user_data)
{
    (void)user_data;
    (void)map_x;
    (void)map_y;
    return (DM1_ELEMENT_WALL << 5);
}

static void test_decode_wall(void)
{
    dm1_dungeon_square_t sq;
    uint8_t raw = (DM1_ELEMENT_WALL << 5);

    dm1_decode_square(raw, &sq);
    assert(sq.element == DM1_ELEMENT_WALL);
    assert(sq.has_thing_list == false);
}

static void test_decode_corridor_with_things(void)
{
    dm1_dungeon_square_t sq;
    uint8_t raw = (DM1_ELEMENT_CORRIDOR << 5) | 0x10;

    dm1_decode_square(raw, &sq);
    assert(sq.element == DM1_ELEMENT_CORRIDOR);
    assert(sq.has_thing_list == true);
}

static void test_decode_door(void)
{
    dm1_dungeon_square_t sq;
    uint8_t raw = (DM1_ELEMENT_DOOR << 5);

    dm1_decode_square(raw, &sq);
    assert(sq.element == DM1_ELEMENT_DOOR);
}

static void test_decode_all_elements(void)
{
    dm1_dungeon_square_t sq;

    for (int e = 0; e <= 6; e++) {
        uint8_t raw = (uint8_t)(e << 5);
        dm1_decode_square(raw, &sq);
        assert(sq.element == e);
    }
}

static void test_relative_map_coords(void)
{
    int out_x, out_y;

    dm1_get_relative_map_coords(5, 5, 0, 1, 0, &out_x, &out_y);
    assert(out_y == 4);
    assert(out_x == 5);

    dm1_get_relative_map_coords(5, 5, 1, 1, 0, &out_x, &out_y);
    assert(out_x == 6);
    assert(out_y == 5);
}

static void test_wall_blocks_movement(void)
{
    uint8_t wall_raw = (DM1_ELEMENT_WALL << 5);
    uint8_t corridor_raw = (DM1_ELEMENT_CORRIDOR << 5);

    assert(dm1_square_blocks_movement(wall_raw) == true);
    assert(dm1_square_blocks_movement(corridor_raw) == false);
}

static void test_classify_aspect_element(void)
{
    uint8_t wall_raw = (DM1_ELEMENT_WALL << 5);
    int cls = dm1_classify_square_aspect_element(wall_raw, 0);
    (void)cls;
    assert(cls == DM1_ELEMENT_WALL);

    uint8_t corridor_raw = (DM1_ELEMENT_CORRIDOR << 5);
    cls = dm1_classify_square_aspect_element(corridor_raw, 0);
    assert(cls == DM1_ELEMENT_CORRIDOR);
}

static void test_build_viewport(void)
{
    dm1_viewport_state_t vp;
    uint8_t map[16 * 16];

    memset(map, (DM1_ELEMENT_CORRIDOR << 5), sizeof(map));
    memset(&vp, 0, sizeof(vp));

    dm1_build_viewport(8, 8, 0, 0, corridor_reader, NULL, &vp);
    int vis[DM1_VIEWPORT_SQUARE_COUNT];
    int count = dm1_get_visible_squares(&vp, vis);
    (void)count;
    assert(count >= 0);
}

static void test_front_wall_at_depth(void)
{
    dm1_viewport_state_t vp;

    memset(&vp, 0, sizeof(vp));

    dm1_build_viewport(8, 8, 0, 0, wall_reader, NULL, &vp);
    assert(dm1_is_front_wall_at_depth(&vp, 0) == true);
}

static void test_viewport_flipped_wall(void)
{
    bool f1 = dm1_viewport_uses_flipped_wall_and_footprints(5, 5, 0);
    bool f2 = dm1_viewport_uses_flipped_wall_and_footprints(5, 5, 1);
    (void)f1;
    (void)f2;
}

int main(void)
{
    test_decode_wall();
    test_decode_corridor_with_things();
    test_decode_door();
    test_decode_all_elements();
    test_relative_map_coords();
    test_wall_blocks_movement();
    test_classify_aspect_element();
    test_build_viewport();
    test_front_wall_at_depth();
    test_viewport_flipped_wall();

    puts("ok: DM1 dungeon square structs (Q-DM1-04) 10 tests passed");
    return 0;
}
