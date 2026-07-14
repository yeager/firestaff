#include "dm1_csb_f0165_dungeon_discarded_thing_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint16_t thing;
    uint16_t next;
    uint8_t type;
    int enabled_sensor;
    int protected_thing;
    int present;
} Record;

typedef struct {
    Record records[8];
    uint16_t heads[3][7][1];
    int discarded_count;
    enum DM1_CSB_F0165_DiscardAction last_action;
    uint16_t last_thing;
    uint16_t last_map;
} Fixture;

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static Record *find_record(Fixture *fixture, uint16_t thing)
{
    int index;
    for (index = 0; index < 8; ++index) {
        if (fixture->records[index].thing == thing) return &fixture->records[index];
    }
    return NULL;
}

static int get_map_bounds(void *context, uint16_t map, uint16_t *max_x,
                          uint16_t *max_y)
{
    (void)context;
    if (map >= 3) return 0;
    *max_x = 6;
    *max_y = 0;
    return 1;
}

static uint16_t get_first_thing(void *context, uint16_t map, uint16_t x,
                                uint16_t y)
{
    Fixture *fixture = context;
    return fixture->heads[map][x][y];
}

static uint16_t get_next_thing(void *context, uint16_t thing)
{
    Record *record = find_record(context, thing);
    return record ? record->next : DM1_CSB_F0165_THING_ENDOFLIST;
}

static uint8_t get_thing_type(void *context, uint16_t thing)
{
    Record *record = find_record(context, thing);
    return record ? record->type : 0xffu;
}

static int sensor_is_enabled(void *context, uint16_t thing)
{
    Record *record = find_record(context, thing);
    return record && record->enabled_sensor;
}

static int thing_is_protected(void *context, uint16_t thing)
{
    Record *record = find_record(context, thing);
    return record && record->protected_thing;
}

static int discard_thing(void *context, enum DM1_CSB_F0165_DiscardAction action,
                         uint16_t thing, uint16_t map, uint16_t x, uint16_t y)
{
    Fixture *fixture = context;
    Record *record = find_record(fixture, thing);
    (void)x;
    (void)y;
    if (!record || !record->present) return 0;
    record->present = 0;
    fixture->discarded_count++;
    fixture->last_action = action;
    fixture->last_thing = thing;
    fixture->last_map = map;
    return 1;
}

static const DM1_CSB_F0165_DungeonOps ops = {
    get_map_bounds, get_first_thing, get_next_thing, get_thing_type,
    sensor_is_enabled, thing_is_protected, discard_thing
};

static void init_fixture(Fixture *fixture)
{
    int map;
    int x;
    int y;
    memset(fixture, 0, sizeof(*fixture));
    for (map = 0; map < 3; ++map)
        for (x = 0; x < 7; ++x)
            for (y = 0; y < 1; ++y)
                fixture->heads[map][x][y] = DM1_CSB_F0165_THING_ENDOFLIST;
}

int main(void)
{
    DM1_CSB_F0165_DiscardState state;
    Fixture fixture;
    uint16_t result;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    init_fixture(&fixture);
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_EXPLOSION,
        3, 1, 0, 0);
    ok &= check(result == DM1_CSB_F0165_THING_NONE && fixture.discarded_count == 0,
                "explosions are never discarded");

    init_fixture(&fixture);
    fixture.records[0] = (Record){ 0x8401u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_WEAPON, 0, 0, 1 };
    fixture.heads[0][0][0] = fixture.records[0].thing;
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_WEAPON,
        3, 1, 0, 0);
    ok &= check(result == 0x0401u && fixture.discarded_count == 1 &&
                fixture.last_action == DM1_CSB_F0165_DISCARD_OBJECT &&
                fixture.last_map == 0,
                "eligible non-party weapon is removed and cell bits clear");

    init_fixture(&fixture);
    fixture.records[0] = (Record){ 0x1402u, 0x1403u,
        DM1_CSB_F0165_THING_TYPE_SENSOR, 1, 0, 1 };
    fixture.records[1] = (Record){ 0x1403u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_WEAPON, 0, 0, 1 };
    fixture.records[2] = (Record){ 0x1404u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_WEAPON, 0, 0, 1 };
    fixture.heads[0][0][0] = fixture.records[0].thing;
    fixture.heads[0][1][0] = fixture.records[2].thing;
    memset(&state, 0, sizeof(state));
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_WEAPON,
        3, 1, 0, 0);
    ok &= check(result == 0x1404u && fixture.last_thing == 0x1404u,
                "enabled sensor rejects its entire square before selection");

    init_fixture(&fixture);
    fixture.records[0] = (Record){ 0x1005u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_WEAPON, 0, 0, 1 };
    fixture.records[1] = (Record){ 0x1006u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_WEAPON, 0, 0, 1 };
    fixture.heads[1][0][0] = fixture.records[0].thing;
    fixture.heads[1][6][0] = fixture.records[1].thing;
    memset(&state, 0, sizeof(state));
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_WEAPON,
        2, 1, 0, 0);
    ok &= check(result == 0x1006u && fixture.last_map == 1,
                "party visible 11x11 square is skipped before party-map fallback");

    init_fixture(&fixture);
    fixture.records[0] = (Record){ 0x1007u, 0x1008u,
        DM1_CSB_F0165_THING_TYPE_GROUP, 0, 1, 1 };
    fixture.records[1] = (Record){ 0x3808u, DM1_CSB_F0165_THING_ENDOFLIST,
        DM1_CSB_F0165_THING_TYPE_PROJECTILE, 0, 0, 1 };
    fixture.heads[0][0][0] = fixture.records[0].thing;
    memset(&state, 0, sizeof(state));
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_GROUP,
        3, 1, 0, 0);
    ok &= check(result == DM1_CSB_F0165_THING_NONE && fixture.discarded_count == 0,
                "protected groups are never discarded");
    result = F0165_DUNGEON_GetDiscardedThing_Compat(
        &state, &ops, &fixture, DM1_CSB_F0165_THING_TYPE_PROJECTILE,
        3, 1, 0, 0);
    ok &= check(result == 0x3808u && fixture.last_action ==
                    DM1_CSB_F0165_DISCARD_PROJECTILE,
                "projectiles use their owning event and list deletion path");

    if (!ok) return 1;
    puts("PASS dm1_csb_f0165_dungeon_discarded_thing_pc34_compat");
    return 0;
}
