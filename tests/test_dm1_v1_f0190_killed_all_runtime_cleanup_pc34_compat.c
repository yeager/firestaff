/* ReDMCSB GROUP.C F0189 -> F0181 runtime consumer regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned short thing_ref(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static __attribute__((unused)) unsigned short read_u16(const unsigned char* bytes) {
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static __attribute__((unused)) int same_thing_identity(unsigned short a, unsigned short b) {
    return (a & 0x3fffu) == (b & 0x3fffu);
}

int main(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[2];
    struct DungeonWeapon_Compat weapons[4];
    struct DungeonArmour_Compat armours[4];
    struct DungeonGroup_Compat groups[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    unsigned char rawGroups[16];
    unsigned char rawJunks[8];
    unsigned char rawWeapons[16];
    unsigned char rawArmours[16];
    struct TimelineEvent_Compat event;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int i;
    int stale = 0;
    (void)stale;
    int otherSquare = 0;
    (void)otherSquare;
    int delayedThudCount = 0;
    (void)delayedThudCount;
    int emittedThudCount = 0;
    (void)emittedThudCount;
    int killedAllNotified = 0;
    (void)killedAllNotified;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(weapons, 0, sizeof(weapons));
    memset(armours, 0, sizeof(armours));
    memset(groups, 0, sizeof(groups));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(rawGroups, 0, sizeof(rawGroups));
    memset(rawJunks, 0, sizeof(rawJunks));
    memset(rawWeapons, 0, sizeof(rawWeapons));
    memset(rawArmours, 0, sizeof(rawArmours));
    squareFirstThings[0] = THING_NONE;
    for (i = 0; i < 4; ++i) weapons[i].next = THING_NONE;
    for (i = 0; i < 4; ++i) armours[i].next = THING_NONE;
    memset(rawWeapons, 0xff, sizeof(rawWeapons));
    memset(rawArmours, 0xff, sizeof(rawArmours));

    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = thing_ref(THING_TYPE_GROUP, 0);
    groups[0].next = thing_ref(THING_TYPE_JUNK, 0);
    groups[0].slot = thing_ref(THING_TYPE_JUNK, 1);
    groups[0].creatureType = DM1_CREATURE_TYPE_ANIMATED_ARMOUR;
    groups[0].count = 0;
    groups[0].cells = 0xffu;
    groups[0].health[0] = 1;
    groups[0].health[1] = 0x1111u;
    groups[0].health[2] = 0x2222u;
    groups[0].health[3] = 0x3333u;
    junks[0].next = THING_ENDOFLIST;
    junks[1].next = thing_ref(THING_TYPE_WEAPON, 2);
    weapons[2].next = THING_ENDOFLIST;
    weapons[3].next = THING_ENDOFLIST;
    weapons[3].type = 8;
    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.junks = junks;
    things.junkCount = 2;
    things.weapons = weapons;
    things.weaponCount = 4;
    things.armours = armours;
    things.armourCount = 4;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.thingCounts[THING_TYPE_JUNK] = 2;
    things.thingCounts[THING_TYPE_WEAPON] = 4;
    things.thingCounts[THING_TYPE_ARMOUR] = 4;
    things.rawThingData[THING_TYPE_GROUP] = rawGroups;
    things.rawThingData[THING_TYPE_JUNK] = rawJunks;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapons;
    things.rawThingData[THING_TYPE_ARMOUR] = rawArmours;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 60;
    world.masterRng.seed = 1;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;
    world.party.championCount = 1;
    world.party.activeChampionIndex = 0;
    world.party.champions[0].present = 1;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        world.party.champions[0].inventory[i] = THING_NONE;
    }
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thing_ref(THING_TYPE_WEAPON, 3);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].stamina.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick + 20u;
    event.mapIndex = 0;
    event.mapX = 2;
    event.mapY = 1;
    event.aux2 = DM1_EVENT_REACTION_DANGER_ON_SQUARE;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    event.mapX = 1;
    event.mapY = 1;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);

    memset(&result, 0, sizeof(result));
    memset(&input, 0, sizeof(input));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_KILL_NOTIFY &&
            result.emissions[i].payload[2] ==
                COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            killedAllNotified = 1;
        }
    }
    assert(killedAllNotified);
    assert(world.creatureAICount == 0);
    assert(groups[0].next == THING_NONE);
    assert(groups[0].health[0] == 0 && groups[0].health[1] == 0 &&
           groups[0].health[2] == 0 && groups[0].health[3] == 0);
    assert(squareFirstThings[0] == thing_ref(THING_TYPE_JUNK, 0));
    assert(same_thing_identity(junks[0].next, thing_ref(THING_TYPE_ARMOUR, 0)));
    assert(same_thing_identity(armours[0].next, thing_ref(THING_TYPE_ARMOUR, 1)));
    assert(same_thing_identity(armours[1].next, thing_ref(THING_TYPE_ARMOUR, 2)));
    assert(same_thing_identity(armours[2].next, thing_ref(THING_TYPE_WEAPON, 0)));
    assert(same_thing_identity(weapons[0].next, thing_ref(THING_TYPE_ARMOUR, 3)));
    assert(same_thing_identity(armours[3].next, thing_ref(THING_TYPE_WEAPON, 1)));
    assert(same_thing_identity(weapons[1].next, thing_ref(THING_TYPE_JUNK, 1)));
    assert(same_thing_identity(junks[1].next, thing_ref(THING_TYPE_WEAPON, 2)));
    assert(weapons[2].next == THING_ENDOFLIST);
    assert(armours[0].type == 41 && armours[0].cursed == 1);
    assert(weapons[0].type == 10 && weapons[0].cursed == 1);
    assert(read_u16(rawGroups + 0) == THING_NONE);
    assert(read_u16(rawGroups + 2) == THING_ENDOFLIST);
    assert(read_u16(rawGroups + 6) == 0 && read_u16(rawGroups + 8) == 0 &&
           read_u16(rawGroups + 10) == 0 && read_u16(rawGroups + 12) == 0);
    assert(same_thing_identity(read_u16(rawJunks + 0), thing_ref(THING_TYPE_ARMOUR, 0)));
    assert(same_thing_identity(read_u16(rawArmours + 0), thing_ref(THING_TYPE_ARMOUR, 1)));
    assert(same_thing_identity(read_u16(rawArmours + 4), thing_ref(THING_TYPE_ARMOUR, 2)));
    assert(same_thing_identity(read_u16(rawArmours + 8), thing_ref(THING_TYPE_WEAPON, 0)));
    assert(same_thing_identity(read_u16(rawWeapons + 0), thing_ref(THING_TYPE_ARMOUR, 3)));
    assert(same_thing_identity(read_u16(rawArmours + 12), thing_ref(THING_TYPE_WEAPON, 1)));
    assert(same_thing_identity(read_u16(rawWeapons + 4), thing_ref(THING_TYPE_JUNK, 1)));
    assert(same_thing_identity(read_u16(rawJunks + 4), thing_ref(THING_TYPE_WEAPON, 2)));
    assert(read_u16(rawWeapons + 8) == THING_ENDOFLIST);
    for (i = 0; i < world.timeline.count; ++i) {
        const struct TimelineEvent_Compat* current = &world.timeline.events[i];
        if (current->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            current->mapIndex == 0 && current->mapX == 2 && current->mapY == 1 &&
            current->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            current->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) stale = 1;
        if (current->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            current->mapX == 1 && current->mapY == 1 &&
            current->aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) otherSquare = 1;
        if (current->kind == TIMELINE_EVENT_PLAY_SOUND &&
            current->fireAtTick == world.gameTick + 1u && current->mapIndex == 0 &&
            current->mapX == 2 && current->mapY == 1 && current->aux0 == 0) {
            ++delayedThudCount;
        }
    }
    assert(!stale);
    assert(otherSquare);
    /* F0186 fixed possessions and F0188 GROUP.Slot both use C02 delayed
     * metallic thuds for this Animated Armour / carried-weapon fixture. */
    assert(delayedThudCount == 2);
    world.gameTick++;
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 1);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == 0 && result.emissions[i].payload[1] == 2 &&
            result.emissions[i].payload[2] == 1 && result.emissions[i].payload[3] == 0) {
            ++emittedThudCount;
        }
    }
    assert(emittedThudCount == 2);
    puts("PASS dm1_v1_f0190_killed_all_runtime_cleanup_pc34_compat");
    return 0;
}
