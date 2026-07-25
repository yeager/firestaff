#include "dm1_v1_creature_attributes_f0144_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    DM1_V1_F0144_GroupRecordPc34 groups[4];
    DM1_V1_F0144_CreatureInfoPc34 creatureInfos[27];
    DM1_V1_F0144_DungeonWorldPc34 world;
    uint16_t group0;
    (void)group0;
    uint16_t group1;
    (void)group1;
    uint16_t group2;
    (void)group2;
    uint16_t weaponThing;
    (void)weaponThing;
    size_t i;

    for (i = 0; i < sizeof(creatureInfos) / sizeof(creatureInfos[0]); ++i) {
        creatureInfos[i].attributes = 0;
    }
    creatureInfos[2].attributes = DM1_V1_F0144_ATTR_DROP_FIXED_POSSESSIONS_PC34;
    creatureInfos[7].attributes =
        DM1_V1_F0144_ATTR_NON_MATERIAL_PC34 |
        DM1_V1_F0144_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34;
    creatureInfos[23].attributes =
        DM1_V1_F0144_ATTR_ARCHENEMY_PC34 |
        DM1_V1_F0144_ATTR_SEE_INVISIBLE_PC34 |
        DM1_V1_F0144_ATTR_NIGHT_VISION_PC34;

    groups[0].creatureType = 7;
    groups[1].creatureType = 23;
    groups[2].creatureType = 2;
    groups[3].creatureType = 26;

    world.groups = groups;
    world.groupCount = sizeof(groups) / sizeof(groups[0]);
    world.creatureInfos = creatureInfos;
    world.creatureInfoCount = sizeof(creatureInfos) / sizeof(creatureInfos[0]);

    group0 = DM1_V1_F0144_MakeThingPc34(
        DM1_V1_F0144_THING_TYPE_GROUP_PC34, 0);
    group1 = DM1_V1_F0144_MakeThingPc34(
        DM1_V1_F0144_THING_TYPE_GROUP_PC34, 1);
    group2 = DM1_V1_F0144_MakeThingPc34(
        DM1_V1_F0144_THING_TYPE_GROUP_PC34, 2);
    weaponThing = DM1_V1_F0144_MakeThingPc34(5, 0);

    assert(strstr(DM1_V1_F0144_SourceEvidencePc34(),
                  "F0144_DUNGEON_GetCreatureAttributes") != 0);
    assert(DM1_V1_F0144_ThingTypePc34(group0) ==
           DM1_V1_F0144_THING_TYPE_GROUP_PC34);
    assert(DM1_V1_F0144_ThingIndexPc34(group2) == 2);

    assert(F0144_DUNGEON_GetCreatureAttributes(&world, group0) ==
           (int)(DM1_V1_F0144_ATTR_NON_MATERIAL_PC34 |
                 DM1_V1_F0144_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34));
    assert(F0144_DUNGEON_GetCreatureAttributes(&world, group1) ==
           (int)(DM1_V1_F0144_ATTR_ARCHENEMY_PC34 |
                 DM1_V1_F0144_ATTR_SEE_INVISIBLE_PC34 |
                 DM1_V1_F0144_ATTR_NIGHT_VISION_PC34));
    assert(F0144_DUNGEON_GetCreatureAttributes(&world, group2) ==
           (int)DM1_V1_F0144_ATTR_DROP_FIXED_POSSESSIONS_PC34);
    assert(DM1_V1_Dungeon_GetCreatureAttributesF0144Pc34Compat(
               &world, group0) ==
           F0144_DUNGEON_GetCreatureAttributes(&world, group0));
    assert(DM1_V1_Dungeon_GetCreatureTypeAttributesF0144Pc34Compat(
               creatureInfos, world.creatureInfoCount, 23) ==
           F0144_DUNGEON_GetCreatureAttributes(&world, group1));

    assert(F0144_DUNGEON_GetCreatureAttributes(&world, weaponThing) == 0);
    assert(F0144_DUNGEON_GetCreatureAttributes(
               &world, DM1_V1_F0144_THING_NONE_PC34) == 0);
    assert(F0144_DUNGEON_GetCreatureAttributes(
               &world, DM1_V1_F0144_THING_END_OF_LIST_PC34) == 0);
    assert(F0144_DUNGEON_GetCreatureAttributes(
               &world,
               DM1_V1_F0144_MakeThingPc34(
                   DM1_V1_F0144_THING_TYPE_GROUP_PC34, 9)) == 0);
    assert(F0144_DUNGEON_GetCreatureAttributes(0, group0) == 0);
    assert(DM1_V1_Dungeon_GetCreatureTypeAttributesF0144Pc34Compat(
               creatureInfos, world.creatureInfoCount, -1) == 0);

    groups[3].creatureType = 27;
    assert(F0144_DUNGEON_GetCreatureAttributes(
               &world,
               DM1_V1_F0144_MakeThingPc34(
                   DM1_V1_F0144_THING_TYPE_GROUP_PC34, 3)) == 0);
    world.creatureInfoCount = DM1_V1_F0144_CREATURE_TYPE_COUNT_PC34 + 1;
    assert(F0144_DUNGEON_GetCreatureAttributes(&world, group0) == 0);

    return 0;
}
