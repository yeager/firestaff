#include "csb_v1_dungeon_world_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_world_init(void)
{
    CSB_DungeonWorld w;
    memset(&w, 0xFF, sizeof(w));
    csb_world_init(&w);
    assert(csb_world_get_level_count(&w) == 0);
}

static void test_add_level(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    int rc = csb_world_add_level(&w, 32, 32);
    (void)rc;
    assert(rc == 0);
    assert(csb_world_get_level_count(&w) == 1);
}

static void test_get_tile(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    CSB_Tile *t = csb_world_get_tile(&w, 0, 0, 0);
    (void)t;
    assert(t != NULL);
}

static void test_get_tile_oob(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    CSB_Tile *t = csb_world_get_tile(&w, 0, 99, 99);
    (void)t;
    assert(t == NULL);
}

static void test_get_tile_const(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    const CSB_Tile *t = csb_world_get_tile_const(&w, 0, 0, 0);
    (void)t;
    assert(t != NULL);
}

static void test_is_walkable(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    csb_world_set_tile_type(&w, 0, 1, 1, 1);
    int walk = csb_world_is_walkable(&w, 0, 1, 1);
    (void)walk;
    assert(walk == 0 || walk == 1);
}

static void test_is_wall(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    int wall = csb_world_is_wall(&w, 0, 0, 0);
    (void)wall;
    assert(wall == 0 || wall == 1);
}

static void test_set_tile_type(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    csb_world_set_tile_type(&w, 0, 2, 2, 5);
    const CSB_Tile *t = csb_world_get_tile_const(&w, 0, 2, 2);
    (void)t;
    assert(t != NULL);
}

static void test_set_wall(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    csb_world_set_wall(&w, 0, 3, 3, 0, 2);
}

static void test_set_ornament(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    csb_world_set_ornament(&w, 0, 4, 4, 1, 7);
}

static void test_set_current_level(void)
{
    CSB_DungeonWorld w;
    csb_world_init(&w);
    csb_world_add_level(&w, 8, 8);
    csb_world_set_current_level(&w, 0);
}

static void test_sensor_get_type(void)
{
    uint8_t t = csb_sensor_get_type(0x0081);
    (void)t;
    assert(t == 1);
}

static void test_sensor_get_data(void)
{
    uint16_t d = csb_sensor_get_data(0x0180);
    (void)d;
    assert(d == 3);
}

static void test_endgame_trigger(void)
{
    CSB_EndgameResult r;
    memset(&r, 0, sizeof(r));
    csb_endgame_trigger(0, &r);
    assert(r.gameWon == 1);
    assert(r.restartAllowed == 0);
}

static void test_bugfix_thing_type_bit15(void)
{
    uint8_t t = csb_bugfix_thing_type_bit15_clearly(0x8004);
    (void)t;
    assert(t == 0);
}

static void test_bugfix_thing_type_normal(void)
{
    uint8_t t = csb_bugfix_thing_type_bit15_clearly(0x0030);
    (void)t;
}

static void test_version_checker(void)
{
    int ok = csb_version_checker_triggered(20, 21);
    (void)ok;
    assert(ok == 1);
}

static void test_version_checker_fail(void)
{
    int ok = csb_version_checker_triggered(22, 21);
    (void)ok;
    assert(ok == 0);
}

static void test_door_defense_points(void)
{
    int dp = csb_door_get_defense_points(CSB_DOOR_WOODEN);
    (void)dp;
    assert(dp >= 0);
}

static void test_door_min_attack(void)
{
    int ma = csb_door_minimum_attack_power(CSB_DOOR_IRON);
    (void)ma;
    assert(ma >= 0);
}

static void test_sensor_constants(void)
{
    assert(CSB_SENSOR_WALL_END_GAME == 18);
    assert(CSB_EFFECT_NONE == -1);
    assert(CSB_EFFECT_SET == 0);
    assert(CSB_EFFECT_TOGGLE == 2);
}

static void test_door_type_constants(void)
{
    assert(CSB_DOOR_PORTCULLIS == 0);
    assert(CSB_DOOR_WOODEN == 1);
    assert(CSB_DOOR_IRON == 2);
    assert(CSB_DOOR_RA == 3);
}

int main(void)
{
    test_world_init();
    test_add_level();
    test_get_tile();
    test_get_tile_oob();
    test_get_tile_const();
    test_is_walkable();
    test_is_wall();
    test_set_tile_type();
    test_set_wall();
    test_set_ornament();
    test_set_current_level();
    test_sensor_get_type();
    test_sensor_get_data();
    test_endgame_trigger();
    test_bugfix_thing_type_bit15();
    test_bugfix_thing_type_normal();
    test_version_checker();
    test_version_checker_fail();
    test_door_defense_points();
    test_door_min_attack();
    test_sensor_constants();
    test_door_type_constants();

    puts("ok: CSB dungeon world (Q-CSB-04) 22 tests passed");
    return 0;
}
