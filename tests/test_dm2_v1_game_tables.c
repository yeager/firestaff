#include "dm2_v1_game_tables.h"
#include <assert.h>
#include <stdio.h>

static void test_direction_deltas(void)
{
    assert(dm2_v1_direction_dx[0] ==  0 && dm2_v1_direction_dy[0] == -1);
    assert(dm2_v1_direction_dx[1] ==  1 && dm2_v1_direction_dy[1] ==  0);
    assert(dm2_v1_direction_dx[2] ==  0 && dm2_v1_direction_dy[2] ==  1);
    assert(dm2_v1_direction_dx[3] == -1 && dm2_v1_direction_dy[3] ==  0);
}

static void test_direction_delta_receipt(void)
{
    DM2_V1_DirectionDeltaReceipt r;
    assert(dm2_v1_direction_delta(0, &r));
    assert(r.valid && r.dx == 0 && r.dy == -1);

    assert(dm2_v1_direction_delta(2, &r));
    assert(r.dx == 0 && r.dy == 1);

    assert(!dm2_v1_direction_delta(4, &r));
    assert(!r.valid);
    assert(!dm2_v1_direction_delta(-1, &r));
    assert(!dm2_v1_direction_delta(0, NULL));
}

static void test_light_attenuation(void)
{
    assert(dm2_v1_light_attenuation[0] == 99);
    assert(dm2_v1_light_attenuation[1] == 75);
    assert(dm2_v1_light_attenuation[2] == 50);
    assert(dm2_v1_light_attenuation[3] == 25);
    assert(dm2_v1_light_attenuation[4] == 1);
    assert(dm2_v1_light_attenuation[5] == 0);
}

static void test_movement_speed(void)
{
    assert(dm2_v1_movement_speed[0] == 5);
    assert(dm2_v1_movement_speed[5] == 1);
    assert(dm2_v1_movement_speed[23] == 5);
}

static void test_creature_scatter(void)
{
    assert(dm2_v1_creature_scatter[0][0] == 0x08);
    assert(dm2_v1_creature_scatter[0][1] == 0x04);
    assert(dm2_v1_creature_scatter[15][0] == 0x10);
    assert(dm2_v1_creature_scatter[15][1] == 0x10);
}

static void test_creature_occupy(void)
{
    assert(dm2_v1_creature_occupy[0] == 1);
    assert(dm2_v1_creature_occupy[9] == 0);
    assert(dm2_v1_creature_occupy[10] == 0);
    assert(dm2_v1_creature_occupy[11] == 1);
}

static void test_skill_tables(void)
{
    assert(dm2_v1_skill_map[0] == 5);
    assert(dm2_v1_skill_map[3] == 6);
    assert(dm2_v1_skill_class[0] == 0);
    assert(dm2_v1_skill_class[1] == 3);
}

static void test_action_hand_map(void)
{
    assert(dm2_v1_action_hand_map[0] == 5);
    assert(dm2_v1_action_hand_map[1] == 4);
    assert(dm2_v1_action_hand_map[26] == 10);
    assert(dm2_v1_action_hand_map[27] == 2);
}

int main(void)
{
    test_direction_deltas();
    test_direction_delta_receipt();
    test_light_attenuation();
    test_movement_speed();
    test_creature_scatter();
    test_creature_occupy();
    test_skill_tables();
    test_action_hand_map();
    assert(dm2_v1_game_tables_source_evidence() != NULL);
    printf("All dm2_v1_game_tables tests passed.\n");
    return 0;
}
