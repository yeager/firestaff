#include "dm1_v1_creature_allowed_on_map_f0139_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const uint8_t map0Allowed[] = { 2, 5, 9, 23 };
    const uint8_t map1Allowed[] = { 1, 4, 7 };
    const uint8_t map2Allowed[] = { 23, 23, 8 };
    DM1_V1_F0139_GroupRecordPc34 groups[4];
    DM1_V1_F0139_MapRecordPc34 maps[3];
    DM1_V1_F0139_DungeonWorldPc34 world;
    uint16_t group0;
    (void)group0;
    uint16_t group1;
    (void)group1;
    uint16_t group2;
    (void)group2;
    uint16_t weaponThing;
    (void)weaponThing;

    groups[0].creatureType = 5;
    groups[1].creatureType = 7;
    groups[2].creatureType = 23;
    groups[3].creatureType = 11;

    maps[0].allowedCreatureTypes = map0Allowed;
    maps[0].allowedCreatureTypeCount =
        sizeof(map0Allowed) / sizeof(map0Allowed[0]);
    maps[1].allowedCreatureTypes = map1Allowed;
    maps[1].allowedCreatureTypeCount =
        sizeof(map1Allowed) / sizeof(map1Allowed[0]);
    maps[2].allowedCreatureTypes = map2Allowed;
    maps[2].allowedCreatureTypeCount =
        sizeof(map2Allowed) / sizeof(map2Allowed[0]);

    world.groups = groups;
    world.groupCount = sizeof(groups) / sizeof(groups[0]);
    world.maps = maps;
    world.mapCount = sizeof(maps) / sizeof(maps[0]);

    group0 = DM1_V1_F0139_MakeThingPc34(
        DM1_V1_F0139_THING_TYPE_GROUP_PC34, 0);
    group1 = DM1_V1_F0139_MakeThingPc34(
        DM1_V1_F0139_THING_TYPE_GROUP_PC34, 1);
    group2 = DM1_V1_F0139_MakeThingPc34(
        DM1_V1_F0139_THING_TYPE_GROUP_PC34, 2);
    weaponThing = DM1_V1_F0139_MakeThingPc34(5, 0);

    assert(strstr(DM1_V1_F0139_SourceEvidencePc34(),
                  "F0139_DUNGEON_IsCreatureAllowedOnMap") != 0);
    assert(DM1_V1_F0139_ThingTypePc34(group0) ==
           DM1_V1_F0139_THING_TYPE_GROUP_PC34);
    assert(DM1_V1_F0139_ThingIndexPc34(group2) == 2);

    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group0, 0) == 1);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group0, 1) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group1, 1) == 1);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group2, 2) == 1);

    assert(DM1_V1_Dungeon_IsCreatureAllowedOnMapF0139Pc34Compat(
               &world, group2, 0) == 1);
    assert(DM1_V1_Dungeon_IsCreatureTypeAllowedOnMapF0139Pc34Compat(
               map1Allowed, sizeof(map1Allowed) / sizeof(map1Allowed[0]),
               4) == 1);

    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, weaponThing, 0) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(
               &world, DM1_V1_F0139_THING_NONE_PC34, 0) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(
               &world, DM1_V1_F0139_MakeThingPc34(
                           DM1_V1_F0139_THING_TYPE_GROUP_PC34, 9),
               0) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group0, -1) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group0, 3) == 0);
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(0, group0, 0) == 0);

    maps[1].allowedCreatureTypeCount =
        DM1_V1_F0139_MAP_ALLOWED_CREATURE_CAPACITY_PC34 + 1;
    assert(F0139_DUNGEON_IsCreatureAllowedOnMap(&world, group1, 1) == 0);

    return 0;
}
