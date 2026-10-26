/* Lane C regression test:
 *   - door state -> open_pct source table
 *   - source table1d6ad0 mapping for center, side and deep draw cells
 *   - custom wall-button pushed variant field selection
 */
#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else fprintf(stderr, "FAIL: %s\n", label); \
} while (0)

static void test_door_open_pct_from_state(void)
{
    CHECK("state OPEN (0) derives 100% open",
          dm2_v1_viewport_door_open_pct_from_state(0, 0) == 100);
    CHECK("state OPEN keeps explicit 100",
          dm2_v1_viewport_door_open_pct_from_state(0, 100) == 100);
    CHECK("state CLOSED_ONE_FOURTH (1) derives 75",
          dm2_v1_viewport_door_open_pct_from_state(1, 0) == 75);
    CHECK("state CLOSED_HALF (2) derives 50",
          dm2_v1_viewport_door_open_pct_from_state(2, 0) == 50);
    CHECK("state CLOSED_THREE_QUARTER (3) derives 25",
          dm2_v1_viewport_door_open_pct_from_state(3, 0) == 25);
    CHECK("state CLOSED (4) derives 0",
          dm2_v1_viewport_door_open_pct_from_state(4, 0) == 0);
    CHECK("state DESTROYED (5) derives 100",
          dm2_v1_viewport_door_open_pct_from_state(5, 0) == 100);
    CHECK("explicit runtime animation pct is preserved for closed",
          dm2_v1_viewport_door_open_pct_from_state(4, 50) == 50);
    CHECK("unknown negative state falls back to closed",
          dm2_v1_viewport_door_open_pct_from_state(-1, 0) == 0);
    CHECK("unknown high state falls back to closed",
          dm2_v1_viewport_door_open_pct_from_state(7, 0) == 0);
}

static void test_static_object_side_deep_cell_ordering(void)
{
    static const int8_t source_lateral[23] = {
         0, -1,  1,  0, -1,  1,  0, -1,  1, -2,  2,  0,
        -1,  1, -2,  2,  0, -1,  1, -2,  2, -3,  3
    };
    static const int8_t source_forward[23] = {
        0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 3,
        3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4
    };
    static const int8_t source_pass[23] = {
        -1, 18, 19, 17, 15, 16, 14, 12, 13, 10, 11, 9,
        7, 8, 5, 6, 4, 2, 3, 0, 1, -1, -1
    };
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    int cell, pass;

    /* Party faces north (dir=0) at (10,10). */
    CHECK("D1L maps to source cell 9 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(9, 9, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 4 && pass == 15);
    CHECK("D1R maps to source cell 10 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(11, 9, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 5 && pass == 16);
    CHECK("D0L maps to source cell 1 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(9, 10, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 1 && pass == 18);
    CHECK("D0R maps to source cell 2 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(11, 10, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 2 && pass == 19);
    CHECK("D2L maps to source cell 7 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(9, 8, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 7 && pass == 12);
    CHECK("D2R maps to source cell 8 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(11, 8, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 8 && pass == 13);
    CHECK("D3L maps to source cell 4 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(9, 7, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 12 && pass == 7);
    CHECK("D3R maps to source cell 5 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(11, 7, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 13 && pass == 8);
    CHECK("D3C maps to source cell 11 with valid pass",
          dm2_v1_viewport_static_object_cell_for_map(10, 7, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 11 && pass == 9);
    CHECK("D0C remains without a generic table pass",
          dm2_v1_viewport_static_object_cell_for_map(10, 10, 0, 10, 10,
                                                     &cell, &pass) == 0);
    CHECK("D2 far-right cell maps to table1d7029 cell 10",
          dm2_v1_viewport_static_object_cell_for_map(12, 8, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 10 && pass == 11);
    CHECK("D3 far-left cell maps to table1d7029 cell 14",
          dm2_v1_viewport_static_object_cell_for_map(8, 7, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 14 && pass == 5);
    CHECK("D3 far-right cell maps to table1d7029 cell 15",
          dm2_v1_viewport_static_object_cell_for_map(12, 7, 0, 10, 10,
                                                     &cell, &pass) == 1 &&
              cell == 15 && pass == 6);
    CHECK("out-of-range forward is rejected",
          dm2_v1_viewport_static_object_cell_for_map(10, 4, 0, 10, 10,
                                                     &cell, &pass) == 0);

    /* Rotate every table1d6ad0 cell through all four party directions. This
     * catches missing ±2 lanes and verifies each cell's table1d7029 pass. */
    for (int dir = 0; dir < 4; ++dir) {
        for (int expected_cell = 1; expected_cell <= 20; ++expected_cell) {
            int map_x = 10 + dx[dir] * source_forward[expected_cell] -
                dy[dir] * source_lateral[expected_cell];
            int map_y = 10 + dy[dir] * source_forward[expected_cell] +
                dx[dir] * source_lateral[expected_cell];
            int expected = source_pass[expected_cell] >= 0;
            int actual = dm2_v1_viewport_static_object_cell_for_map(
                map_x, map_y, dir, 10, 10, &cell, &pass);
            CHECK("rotated table1d6ad0 cell/pass mapping",
                  actual == expected &&
                  (!expected || (cell == expected_cell &&
                                  pass == source_pass[expected_cell])));
        }
    }

    /* Static-object plans cover cells 1..15. Other view cells remain
     * available for source ordering, while DRAW_PUT_DOWN_ITEM itself rejects
     * cells beyond 15. */
    {
        DM2_V1_StaticObjectSourcePlan plan;
        CHECK("D1L source plan derives its side-cell placement",
              dm2_v1_viewport_static_object_source_plan(
                  4, 15, 0x10, 0, 0, 0, 0, 1u,
                  1u << 6, &plan) == 1 &&
                  plan.position_5x5 == 6 &&
                  plan.clip_rect_id == (0x8000 | (5000 + 4 * 25 + 6)) &&
                  plan.y_distance == 1 &&
                  plan.stretch_factor64 == 0x40 &&
                  plan.slot_x_offset == 2 &&
                  plan.slot_y_offset == -3);
        CHECK("D3C source plan derives its deep-cell placement",
              dm2_v1_viewport_static_object_source_plan(
                  11, 9, 0x10, 0, 0, 0, 0, 1u,
                  1u << 6, &plan) == 1 &&
                  plan.position_5x5 == 6 &&
                  plan.clip_rect_id == (0x8000 | (5000 + 11 * 25 + 6)) &&
                  plan.y_distance == 3 &&
                  plan.stretch_factor64 == 0x1c);
        CHECK("party cell source plan is fail-closed",
              dm2_v1_viewport_static_object_source_plan(
                  0, 0, 0x10, 0, 0, 0, 0, 1u,
                  1u << 6, &plan) == 0);
        CHECK("D4 cell source plan is fail-closed",
              dm2_v1_viewport_static_object_source_plan(
                  16, 4, 0x10, 0, 0, 0, 0, 1u,
                  1u << 6, &plan) == 0);
    }
}

static void test_wall_button_pushed_variant(void)
{
    DM2_V1_ViewportState s;
    DM2_V1_DoorRenderPlan plan;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    s.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    s.squares[DM2_SQ_D0C].door_wall_button = 1;
    s.squares[DM2_SQ_D0C].door_wall_button_index = 0x2a;
    s.squares[DM2_SQ_D0C].door_wall_button_field = 1;
    s.squares[DM2_SQ_D0C].door_wall_button_state = 1;
    s.squares[DM2_SQ_D0C].door_wall_button_x = 6;
    s.squares[DM2_SQ_D0C].door_wall_button_y = 7;
    s.squares[DM2_SQ_D0C].door_wall_button_object_id = 0x8abcu;

    CHECK("wall button pushed state advances field",
          dm2_v1_viewport_build_door_render_plan(&s, &plan) == 1 &&
              plan.door_count == 1 &&
              plan.doors[0].wall_button_field == 2 &&
              plan.doors[0].button_gdat_index ==
                  dm2_v1_viewport_wall_button_graphic_index(0x2a, 2));
}

int main(void)
{
    test_door_open_pct_from_state();
    test_static_object_side_deep_cell_ordering();
    test_wall_button_pushed_variant();
    printf("DM2 V1 door-state and static-object side-cell ordering: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
