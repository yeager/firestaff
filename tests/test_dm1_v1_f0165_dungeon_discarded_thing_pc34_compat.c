#include "dm1_csb_f0165_dungeon_discarded_thing_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum {
    kMaxThings = 16,
    kWeaponVisible = 0x4001,
    kEnabledSensor = 0x0002,
    kWeaponBehindSensor = 0x4003,
    kProtectedGroup = 0x1004,
    kDiscardGroup = 0x1005,
    kProjectile = 0x3806,
    kProtectedWeapon = 0x4007,
    kDiscardWeapon = 0xc008
};

typedef struct TestThing {
    uint16_t thing;
    uint16_t next;
    uint8_t type;
    int enabledSensor;
    int protectedThing;
} TestThing;

typedef struct TestWorld {
    uint16_t first[2][4][4];
    TestThing things[kMaxThings];
    enum DM1_CSB_F0165_DiscardAction lastAction;
    uint16_t lastThing;
    uint16_t lastMap;
    uint16_t lastX;
    uint16_t lastY;
    int discardCount;
} TestWorld;

static TestThing *find_thing(TestWorld *world, uint16_t thing)
{
    int i;

    for (i = 0; i < kMaxThings; ++i) {
        if (world->things[i].thing == thing) {
            return &world->things[i];
        }
    }
    return NULL;
}

static void put_thing(
    TestWorld *world,
    int slot,
    uint16_t thing,
    uint8_t type,
    uint16_t next,
    int enabledSensor,
    int protectedThing)
{
    world->things[slot].thing = thing;
    world->things[slot].type = type;
    world->things[slot].next = next;
    world->things[slot].enabledSensor = enabledSensor;
    world->things[slot].protectedThing = protectedThing;
}

static int get_map_bounds(
    void *context,
    uint16_t map_index,
    uint16_t *out_max_x,
    uint16_t *out_max_y)
{
    (void)context;
    if (map_index >= 2 || !out_max_x || !out_max_y) {
        return 0;
    }
    *out_max_x = 3;
    *out_max_y = 3;
    return 1;
}

static uint16_t get_first_thing(
    void *context,
    uint16_t map_index,
    uint16_t map_x,
    uint16_t map_y)
{
    TestWorld *world = (TestWorld *)context;

    assert(world != NULL);
    return world->first[map_index][map_x][map_y];
}

static uint16_t get_next_thing(void *context, uint16_t thing)
{
    TestThing *record = find_thing((TestWorld *)context, thing);

    return record ? record->next : DM1_CSB_F0165_THING_ENDOFLIST;
}

static uint8_t get_thing_type(void *context, uint16_t thing)
{
    TestThing *record = find_thing((TestWorld *)context, thing);

    return record ? record->type : 0xffu;
}

static int sensor_is_enabled(void *context, uint16_t thing)
{
    TestThing *record = find_thing((TestWorld *)context, thing);

    return record ? record->enabledSensor : 0;
}

static int thing_is_protected(void *context, uint16_t thing)
{
    TestThing *record = find_thing((TestWorld *)context, thing);

    return record ? record->protectedThing : 0;
}

static int discard_thing(
    void *context,
    enum DM1_CSB_F0165_DiscardAction action,
    uint16_t thing,
    uint16_t map_index,
    uint16_t map_x,
    uint16_t map_y)
{
    TestWorld *world = (TestWorld *)context;

    world->lastAction = action;
    world->lastThing = thing;
    world->lastMap = map_index;
    world->lastX = map_x;
    world->lastY = map_y;
    ++world->discardCount;
    return 1;
}

static const DM1_CSB_F0165_DungeonOps kOps = {
    get_map_bounds,
    get_first_thing,
    get_next_thing,
    get_thing_type,
    sensor_is_enabled,
    thing_is_protected,
    discard_thing
};

static void init_world(TestWorld *world)
{
    memset(world, 0, sizeof(*world));
    memset(world->first, 0xff, sizeof(world->first));

    world->lastAction = DM1_CSB_F0165_DISCARD_OBJECT;
    put_thing(world, 0, kWeaponVisible, DM1_CSB_F0165_THING_TYPE_WEAPON,
              DM1_CSB_F0165_THING_ENDOFLIST, 0, 0);
    put_thing(world, 1, kEnabledSensor, DM1_CSB_F0165_THING_TYPE_SENSOR,
              kWeaponBehindSensor, 1, 0);
    put_thing(world, 2, kWeaponBehindSensor, DM1_CSB_F0165_THING_TYPE_WEAPON,
              DM1_CSB_F0165_THING_ENDOFLIST, 0, 0);
    put_thing(world, 3, kProtectedGroup, DM1_CSB_F0165_THING_TYPE_GROUP,
              kDiscardGroup, 0, 1);
    put_thing(world, 4, kDiscardGroup, DM1_CSB_F0165_THING_TYPE_GROUP,
              DM1_CSB_F0165_THING_ENDOFLIST, 0, 0);
    put_thing(world, 5, kProjectile, DM1_CSB_F0165_THING_TYPE_PROJECTILE,
              DM1_CSB_F0165_THING_ENDOFLIST, 0, 0);
    put_thing(world, 6, kProtectedWeapon, DM1_CSB_F0165_THING_TYPE_WEAPON,
              kDiscardWeapon, 0, 1);
    put_thing(world, 7, kDiscardWeapon, DM1_CSB_F0165_THING_TYPE_WEAPON,
              DM1_CSB_F0165_THING_ENDOFLIST, 0, 0);

    world->first[0][1][1] = kWeaponVisible;
    world->first[1][0][0] = kEnabledSensor;
    world->first[1][0][1] = kProtectedGroup;
    world->first[1][0][2] = kProjectile;
    world->first[1][1][0] = kProtectedWeapon;
}

static void test_source_named_boundary_discards_first_unprotected_group(void)
{
    DM1_CSB_F0165_DiscardState state = {{0}};
    TestWorld world;
    uint16_t result;
    (void)result;

    init_world(&world);
    result = F0165_DUNGEON_GetDiscardedThing(
        &state, &kOps, &world, DM1_CSB_F0165_THING_TYPE_GROUP, 2,
        0, 1, 1);

    assert(result == (uint16_t)(kDiscardGroup & 0x3fffu));
    assert(world.discardCount == 1);
    assert(world.lastAction == DM1_CSB_F0165_DISCARD_GROUP);
    assert(world.lastThing == kDiscardGroup);
    assert(world.lastMap == 1);
    assert(world.lastX == 0);
    assert(world.lastY == 1);
    assert(state.last_discarded_map[DM1_CSB_F0165_THING_TYPE_GROUP] == 1);
}

static void test_party_visible_square_and_enabled_sensor_block_candidates(void)
{
    DM1_CSB_F0165_DiscardState state = {{0}};
    TestWorld world;
    uint16_t result;
    (void)result;

    init_world(&world);
    result = F0165_DUNGEON_GetDiscardedThing(
        &state, &kOps, &world, DM1_CSB_F0165_THING_TYPE_WEAPON, 2,
        0, 1, 1);

    assert(result == (uint16_t)(kDiscardWeapon & 0x3fffu));
    assert(world.discardCount == 1);
    assert(world.lastThing == kDiscardWeapon);
    assert(world.lastAction == DM1_CSB_F0165_DISCARD_OBJECT);
    assert(world.lastMap == 1);
    assert(world.lastX == 1);
    assert(world.lastY == 0);
}

static void test_projectile_uses_projectile_discard_action(void)
{
    DM1_CSB_F0165_DiscardState state = {{0}};
    TestWorld world;
    uint16_t result;
    (void)result;

    init_world(&world);
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &kOps, &world, DM1_CSB_F0165_THING_TYPE_PROJECTILE, 2,
        0, 1, 1);

    assert(result == (uint16_t)(kProjectile & 0x3fffu));
    assert(world.discardCount == 1);
    assert(world.lastAction == DM1_CSB_F0165_DISCARD_PROJECTILE);
}

static void test_invalid_inputs_and_explosions_return_none(void)
{
    DM1_CSB_F0165_DiscardState state = {{0}};
    (void)state;
    TestWorld world;

    init_world(&world);
    assert(F0165_DUNGEON_GetDiscardedThing(
               NULL, &kOps, &world, DM1_CSB_F0165_THING_TYPE_GROUP, 2,
               0, 1, 1) == DM1_CSB_F0165_THING_NONE);
    assert(F0165_DUNGEON_GetDiscardedThing(
               &state, &kOps, &world, DM1_CSB_F0165_THING_TYPE_EXPLOSION, 2,
               0, 1, 1) == DM1_CSB_F0165_THING_NONE);
    assert(F0165_DUNGEON_GetDiscardedThing(
               &state, &kOps, &world, DM1_CSB_F0165_THING_TYPE_GROUP, 0,
               0, 1, 1) == DM1_CSB_F0165_THING_NONE);
}

int main(void)
{
    test_source_named_boundary_discards_first_unprotected_group();
    test_party_visible_square_and_enabled_sensor_block_candidates();
    test_projectile_uses_projectile_discard_action();
    test_invalid_inputs_and_explosions_return_none();

    puts("ok: DM1 F0165 discarded thing callable");
    return 0;
}
