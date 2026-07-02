/*
 * test_memory_tick_orchestrator_f0303_skill_query_pc34_compat.c
 *
 * Locks the M10 orchestrator skill-level bridge used by CMD_CAST_SPELL.
 * ReDMCSB MENU.C/F0412 and F0407 call CHAMPION.C F0303 for spell/action
 * skill levels; this test ensures M10 does not fall back to raw
 * ChampionState_Compat.skillLevels[] for hidden spell skills.
 */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned short read_u16_le_for_test(const unsigned char* raw) {
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static void init_world(struct GameWorld_Compat* world,
                       struct DungeonThings_Compat* things,
                       struct DungeonWeapon_Compat* weapons,
                       struct DungeonJunk_Compat* junks) {
    struct ChampionState_Compat* champion;
    memset(world, 0, sizeof(*world));
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat) * 2);
    memset(junks, 0, sizeof(struct DungeonJunk_Compat) * 2);

    world->things = things;
    world->party.championCount = 1;
    world->party.activeChampionIndex = 0;
    champion = &world->party.champions[0];
    champion->present = 1;
    for (int i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champion->inventory[i] = THING_NONE;
    }

    weapons[0].type = DM1_SKILL_ICON_WEAPON_THE_FIRESTAFF_COMPLETE;
    weapons[1].type = DM1_SKILL_ICON_WEAPON_SCEPTRE_OF_LYF;
    junks[0].type = DM1_SKILL_ICON_JUNK_PENDANT_FERAL;
    junks[1].type = DM1_SKILL_ICON_JUNK_GEM_OF_AGES;

    things->weapons = weapons;
    things->weaponCount = 2;
    things->junks = junks;
    things->junkCount = 2;
}

static void test_orch_f0303_inventory_and_rest_query(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champion;

    init_world(&world, &things, weapons, junks);
    champion = &world.party.champions[0];
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    champion->inventory[CHAMPION_SLOT_NECK] =
        make_thing(THING_TYPE_JUNK, 0);
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 500;

    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_WIZARD) == 5);

    world.partyIsResting = 1;
    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_WIZARD) == 1);
    world.partyIsResting = 0;
    world.lifecycle.rest.isResting = 1;
    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_WIZARD) == 1);
}

static void test_orch_f0303_hidden_heal_query(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champion;

    init_world(&world, &things, weapons, junks);
    champion = &world.party.champions[0];
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 1);
    champion->inventory[CHAMPION_SLOT_NECK] =
        make_thing(THING_TYPE_JUNK, 1);
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_HEAL].experience = 500;

    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_HEAL) == 3);
}

static void test_orch_projectile_spell_uses_hidden_skill_query_value(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champion;
    struct SpellDefinition_Compat spell;
    struct SpellEffect_Compat effect;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int skillLevel;
    int sawSpellEffect;
    int i;

    init_world(&world, &things, weapons, junks);
    champion = &world.party.champions[0];
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_AIR].experience = 500;

    assert(F0752b_MAGIC_LookupSpellByTableIndex_Compat(14, &spell) == 1);
    assert(spell.skillIndex == DM1_SKILL_IDX_AIR);

    skillLevel = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, spell.skillIndex);
    assert(skillLevel == 4);

    memset(&effect, 0, sizeof(effect));
    assert(F0756_MAGIC_ProduceProjectileEffect_Compat(
        &spell, 1, skillLevel, &world.masterRng, &effect) == 1);
    /* Open Door doubles the Air skill in F0756:
     * (powerOrdinal+2) * (4 + ((skill*2) << 1)) = 3 * 20 = 60. */
    assert(effect.impactAttack == 60);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 14;
    input.reserved = 1;
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.timeline.count == 0);
    sawSpellEffect = 0;
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SPELL_EFFECT &&
            result.emissions[i].payload[0] == 0 &&
            result.emissions[i].payload[1] == C2_SPELL_KIND_PROJECTILE_COMPAT &&
            result.emissions[i].payload[2] ==
                C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT &&
            result.emissions[i].payload[3] == 1) {
            sawSpellEffect = 1;
        }
    }
    assert(sawSpellEffect == 1);
}

static void test_orch_light_spell_uses_source_light_amount_and_party_map(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    world.gameTick = 77;
    world.party.mapIndex = 3;
    world.partyMapIndex = 0; /* stale legacy mirror must not place event */
    world.party.mapX = 9;
    world.party.mapY = 4;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 6; /* Oh Ir Ra: LIGHT */
    input.reserved = 1;   /* Lo power ordinal */

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.magic.magicalLightAmount == 24);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY);
    assert(world.timeline.events[0].fireAtTick == 10077);
    assert(world.timeline.events[0].mapIndex == 3);
    assert(world.timeline.events[0].mapX == 9);
    assert(world.timeline.events[0].mapY == 4);
    assert(world.timeline.events[0].aux0 == -3);
}

static void test_orch_potion_spell_mutates_empty_flask_in_hand(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonPotion_Compat potions[1];
    unsigned char rawPotionData[4];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int sawSpellEffect = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(potions, 0, sizeof(potions));
    memset(rawPotionData, 0, sizeof(rawPotionData));

    potions[0].next = THING_ENDOFLIST;
    potions[0].type = 20; /* C20 empty flask / C195 icon. */
    potions[0].power = 7;
    rawPotionData[0] = 0xFEu;
    rawPotionData[1] = 0xFFu;
    rawPotionData[2] = 7u;
    rawPotionData[3] = 20u;
    things.potions = potions;
    things.potionCount = 1;
    things.rawThingData[THING_TYPE_POTION] = rawPotionData;
    things.thingCounts[THING_TYPE_POTION] = 1;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_POTION, 0);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 16; /* Ya: stamina potion. */
    input.reserved = 1;    /* Lo power ordinal. */
    input.reserved2 =
        CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK |
        ((uint32_t)CHAMPION_SLOT_ACTION_HAND <<
         CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_SHIFT);

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(potions[0].type == 11);
    assert(potions[0].power >= 40 && potions[0].power <= 55);
    assert(rawPotionData[2] == potions[0].power);
    assert(rawPotionData[3] == 11);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SPELL_EFFECT &&
            result.emissions[i].payload[0] == 0 &&
            result.emissions[i].payload[1] == C1_SPELL_KIND_POTION_COMPAT &&
            result.emissions[i].payload[2] == 11 &&
            result.emissions[i].payload[3] == 1) {
            sawSpellEffect = 1;
        }
    }
    assert(sawSpellEffect == 1);
}

static void test_orch_zokathra_spell_materializes_in_ready_hand(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    unsigned char rawJunkData[8];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    unsigned short zokathraThing = make_thing(THING_TYPE_JUNK, 0);

    init_world(&world, &things, weapons, junks);
    memset(rawJunkData, 0, sizeof(rawJunkData));
    junks[0].next = THING_NONE;
    junks[0].type = 0;
    junks[1].next = THING_NONE;
    junks[1].type = 0;
    rawJunkData[0] = 0xFFu;
    rawJunkData[1] = 0xFFu;
    rawJunkData[4] = 0xFFu;
    rawJunkData[5] = 0xFFu;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;
    things.thingCounts[THING_TYPE_JUNK] = 2;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 24; /* Zo Kath Ra: Zokathra. */
    input.reserved = 1;    /* Lo power ordinal. */

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(junks[0].type == 51);
    assert(junks[0].next == THING_ENDOFLIST);
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] ==
           zokathraThing);
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
           THING_NONE);
    assert(rawJunkData[0] == 0xFEu);
    assert(rawJunkData[1] == 0xFFu);
    assert(rawJunkData[2] == 51u);
    assert(rawJunkData[3] == 0u);
}

static void test_orch_zokathra_spell_falls_back_to_party_square(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[1];
    unsigned short squareFirstThings[1];
    unsigned char rawJunkData[8];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    unsigned short zokathraThing = make_thing(THING_TYPE_JUNK, 0);

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(rawJunkData, 0, sizeof(rawJunkData));
    squareData[0] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = THING_ENDOFLIST;
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 1;
    maps[0].height = 1;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 1;
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 1);
    junks[0].next = THING_NONE;
    junks[0].type = 0;
    junks[1].next = THING_NONE;
    junks[1].type = 0;
    rawJunkData[0] = 0xFFu;
    rawJunkData[1] = 0xFFu;
    rawJunkData[4] = 0xFFu;
    rawJunkData[5] = 0xFFu;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;
    things.thingCounts[THING_TYPE_JUNK] = 2;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 24; /* Zo Kath Ra: Zokathra. */
    input.reserved = 1;    /* Lo power ordinal. */

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(junks[0].type == 51);
    assert(junks[0].next == THING_ENDOFLIST);
    assert(squareFirstThings[0] == zokathraThing);
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] ==
           make_thing(THING_TYPE_WEAPON, 0));
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
           make_thing(THING_TYPE_WEAPON, 1));
    assert(rawJunkData[0] == 0xFEu);
    assert(rawJunkData[1] == 0xFFu);
    assert(rawJunkData[2] == 51u);
    assert(rawJunkData[3] == 0u);
}

static void test_orch_f0312_skill_bonus_uses_live_f0303_values(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];

    init_world(&world, &things, weapons, junks);
    world.party.champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] = 0;
    world.party.champions[0].skillLevels[CHAMPION_SKILL_NINJA] = 0;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_NINJA].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_THROW].experience = 500;

    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_SWING) == 2);
    assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        &world, 0, DM1_SKILL_IDX_THROW) == 2);
    assert(F0888_ORCH_GetChampionF0312SkillBonus_Compat(
        &world, 0, 2) == 4);
}

static void test_combat_f0313_wound_defense_final_shift_and_clamp(void) {
    struct CombatantChampionSnapshot_Compat champ;
    int defense = -1;
    int rngCalls = -1;
    struct RngState_Compat rng;

    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[CHAMPION_SLOT_TORSO] = 65;
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(
        &champ, CHAMPION_SLOT_TORSO, 0, &defense) == 1);
    /* ReDMCSB CHAMPION.C F0313 lines 1375-1382 halves the accumulated
     * defense before bounding it to 0..100. */
    assert(defense == 32);

    champ.woundDefense[CHAMPION_SLOT_TORSO] = 260;
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(
        &champ, CHAMPION_SLOT_TORSO, 0, &defense) == 1);
    assert(defense == 100);

    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[CHAMPION_SLOT_TORSO] = 64;
    champ.isResting = 1;
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(
        &champ, CHAMPION_SLOT_TORSO, 0, &defense) == 1);
    /* ReDMCSB F0313 line 1378 halves defense while the party is resting,
     * then line 1382 applies the final half-scale: (64 >> 1) >> 1. */
    assert(defense == 16);

    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[CHAMPION_SLOT_TORSO] = 64;
    champ.wounds = 1 << CHAMPION_SLOT_TORSO;
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(
        &champ, CHAMPION_SLOT_TORSO, 0, &defense) == 1);
    /* ReDMCSB F0313 lines 1375-1377 subtracts 8 + RANDOM(4) for an
     * already-wounded slot.  F0733 is deterministic, so this locks the fixed
     * source penalty before the final half-scale: (64 - 8) >> 1. */
    assert(defense == 28);

    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[CHAMPION_SLOT_TORSO] = 64;
    champ.statisticVitality = 64;
    champ.wounds = 1 << CHAMPION_SLOT_TORSO;
    assert(F0730_COMBAT_RngInit_Compat(&rng, 1u) == 1);
    assert(F0733b_COMBAT_GetChampionWoundDefenseRng_Compat(
        &champ, CHAMPION_SLOT_TORSO, 0, &rng, &defense, &rngCalls) == 1);
    /* ReDMCSB CHAMPION.C F0313 lines 1350 and 1364-1366 consume
     * RANDOM((64 >> 3) + 1) then RANDOM(4).  Firestaff's deterministic RNG
     * gives 8 and 2 for seed 1, so (64 + 8 - 8 - 2) >> 1. */
    assert(defense == 31);
    assert(rngCalls == 2);

    memset(&champ, 0, sizeof(champ));
    champ.woundDefense[CHAMPION_SLOT_TORSO] = 64;
    champ.statisticVitality = 64;
    assert(F0730_COMBAT_RngInit_Compat(&rng, 1u) == 1);
    assert(F0733b_COMBAT_GetChampionWoundDefenseRng_Compat(
        &champ, CHAMPION_SLOT_TORSO, 1, &rng, &defense, &rngCalls) == 1);
    /* Sharp defense halves only the random vitality component before the
     * accumulated F0313 final shift: (64 + (8 >> 1)) >> 1. */
    assert(defense == 34);
    assert(rngCalls == 1);
}

static void test_orch_turn_rotates_champion_cell_and_direction(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    world.party.direction = 0;
    world.party.champions[0].cell = 2;
    world.party.champions[0].direction = 3;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_TURN_RIGHT;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.party.direction == 1);
    assert(world.party.champions[0].cell == 3);
    assert(world.party.champions[0].direction == 0);
}

static unsigned char square_for_test(int elementType, int lowBits) {
    return (unsigned char)(((elementType & 7) << 5) | (lowBits & 0x1F));
}

static void test_orch_projectile_create_preserves_associated_thing(void) {
    struct ProjectileList_Compat list;
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    int slot = -1;
    unsigned short thrownThing = make_thing(THING_TYPE_WEAPON, 1);

    memset(&list, 0, sizeof(list));
    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 82;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.associatedThing = thrownThing;

    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &list, &slot, &firstMove) == 1);
    assert(slot == 0);
    assert(list.entries[0].reserved1 == thrownThing);

    memset(&list, 0, sizeof(list));
    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 82;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;

    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &list, &slot, &firstMove) == 1);
    assert(slot == 0);
    assert(list.entries[0].reserved1 == THING_NONE);
}

static void test_orch_projectile_move_event_advances_and_reschedules(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 82;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(slot == 0);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 1);
    assert(world.projectiles.entries[0].mapX == 2);
    assert(world.projectiles.entries[0].mapY == 1);
    assert(world.projectiles.entries[0].firstMoveGraceFlag == 0);
    assert(world.projectiles.entries[0].kineticEnergy == 82);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_PROJECTILE_MOVE);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].aux0 == 0);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 1);
    assert(world.projectiles.entries[0].mapX == 2);
    assert(world.projectiles.entries[0].mapY == 1);
    assert(world.projectiles.entries[0].cell == 2);
    assert(world.projectiles.entries[0].kineticEnergy == 72);
    assert(world.projectiles.entries[0].attack == 30);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].fireAtTick == 103);
    assert(world.timeline.events[0].aux0 == 0);
}

static void test_orch_projectile_wall_impact_creates_explosion(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_FIREBALL;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C000_EXPLOSION_FIREBALL);
    assert(world.explosions.entries[0].attack == 80);
    assert(world.explosions.entries[0].mapIndex == 0);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == 2);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].aux0 == 0);
    assert(world.timeline.events[0].aux1 == C000_EXPLOSION_FIREBALL);
    assert(world.timeline.events[0].aux2 == 80);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.explosions.count == 0);
    assert(world.explosions.entries[0].slotIndex == -1);
    assert(world.timeline.count == 0);
}

static void run_orch_magical_wall_zero_adjusted_explosion_case(int projectileSubtype,
                                                               int kineticEnergy) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = projectileSubtype;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = kineticEnergy;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    for (i = 0; i < result.emissionCount; ++i) {
        assert(result.emissions[i].kind != EMIT_SOUND_REQUEST);
    }
}

static void test_orch_magical_wall_impact_zero_adjusted_explosion_skips_spawn_and_sound(void) {
    run_orch_magical_wall_zero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 1);
    run_orch_magical_wall_zero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_POISON_BOLT, 3);
}

static void run_orch_magical_wall_nonzero_adjusted_explosion_case(
    int projectileSubtype,
    int kineticEnergy,
    int expectedExplosionType,
    int expectedAttack,
    int expectedCell)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = projectileSubtype;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = kineticEnergy;
    createIn.attack = kineticEnergy;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == expectedExplosionType);
    assert(world.explosions.entries[0].attack == expectedAttack);
    assert(world.explosions.entries[0].mapIndex == 0);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == expectedCell);
    assert(world.explosions.entries[0].centered ==
           (expectedCell == EXPLOSION_CELL_CENTERED));
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].aux0 == 0);
    assert(world.timeline.events[0].aux1 == expectedExplosionType);
    assert(world.timeline.events[0].aux2 == expectedAttack);
    for (i = 0; i < result.emissionCount; ++i) {
        assert(result.emissions[i].kind != EMIT_SOUND_REQUEST);
    }

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    if (expectedExplosionType == C007_EXPLOSION_POISON_CLOUD &&
        expectedAttack >= 6) {
        assert(world.explosions.count == 1);
        assert(world.explosions.entries[0].attack == expectedAttack - 3);
        assert(world.explosions.entries[0].currentFrame == 1);
        assert(world.timeline.count == 1);
        assert(world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE);
        assert(world.timeline.events[0].fireAtTick == 103);
        assert(world.timeline.events[0].aux0 == 0);
        assert(world.timeline.events[0].aux1 == expectedExplosionType);
        assert(world.timeline.events[0].aux2 == expectedAttack - 3);
    } else {
        assert(world.explosions.count == 0);
        assert(world.timeline.count == 0);
    }
}

static void test_orch_magical_wall_impact_nonzero_adjusted_explosion_spawns(void) {
    run_orch_magical_wall_nonzero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 6, C002_EXPLOSION_LIGHTNING_BOLT,
        3, 2);
    run_orch_magical_wall_nonzero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_POISON_BOLT, 8, C007_EXPLOSION_POISON_CLOUD,
        2, EXPLOSION_CELL_CENTERED);
    run_orch_magical_wall_nonzero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_POISON_CLOUD, 13, C007_EXPLOSION_POISON_CLOUD,
        13, EXPLOSION_CELL_CENTERED);
    run_orch_magical_wall_nonzero_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_HARM_NON_MATERIAL, 11,
        C003_EXPLOSION_HARM_NON_MATERIAL, 11, 2);
}

static void test_orch_projectile_wall_impact_emits_non_explosion_sound(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawSound = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_METALLIC_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);
}

static void test_orch_projectile_wall_impact_materializes_associated_weapon(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(1 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);
    squareFirstThings[0] = THING_ENDOFLIST;
    weapons[0].type = 27; /* ReDMCSB C27_WEAPON_ARROW. */
    weapons[0].next = THING_NONE;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    createIn.associatedThing = make_thing(THING_TYPE_WEAPON, 0);
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 0);
    /* ReDMCSB PROJEXPL.C F0215 lines 248-259 moves Projectile.Slot to
     * the projectile square when no GROUP.Slot keep-thrown-sharp target is
     * selected.  The wall impact at PROJEXPL.C F0219 lines 717-725 occurs
     * before committing the destination wall square, so the arrow remains at
     * the source square and keeps the projectile cell. */
    assert(squareFirstThings[0] ==
           (unsigned short)(make_thing(THING_TYPE_WEAPON, 0) | (2u << 14)));
    assert(weapons[0].next == THING_ENDOFLIST);
    assert(world.timeline.count == 0);
}

static void test_orch_projectile_wall_impact_appends_associated_weapon_raw_tail(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    unsigned char rawWeaponData[8];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    unsigned short existingThing;
    unsigned short materializedThing;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(rawWeaponData, 0, sizeof(rawWeaponData));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(1 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    existingThing = make_thing(THING_TYPE_WEAPON, 1);
    materializedThing =
        (unsigned short)(make_thing(THING_TYPE_WEAPON, 0) | (2u << 14));
    squareFirstThings[0] = existingThing;
    weapons[0].type = 27; /* ReDMCSB C27_WEAPON_ARROW. */
    weapons[0].next = THING_NONE;
    weapons[1].type = 8;  /* ReDMCSB C08_WEAPON_DAGGER. */
    weapons[1].next = THING_ENDOFLIST;
    rawWeaponData[0] = (unsigned char)(THING_NONE & 0xFFu);
    rawWeaponData[1] = (unsigned char)((THING_NONE >> 8) & 0xFFu);
    rawWeaponData[2] = 27;
    rawWeaponData[4] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawWeaponData[5] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawWeaponData[6] = 8;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    things.thingCounts[THING_TYPE_WEAPON] = 2;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    createIn.associatedThing = make_thing(THING_TYPE_WEAPON, 0);
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);

    /* ReDMCSB PROJEXPL.C F0215 lines 248-259 delegates the
     * Projectile.Slot move to the normal Thing-list machinery, so an
     * existing square chain must keep its first Thing and link the thrown
     * object at the tail in both decoded state and raw thing bytes. */
    assert(world.projectiles.count == 0);
    assert(squareFirstThings[0] == existingThing);
    assert(weapons[1].next == materializedThing);
    assert((unsigned short)(rawWeaponData[4] |
                            ((unsigned short)rawWeaponData[5] << 8)) ==
           materializedThing);
    assert(weapons[0].next == THING_ENDOFLIST);
    assert((unsigned short)(rawWeaponData[0] |
                            ((unsigned short)rawWeaponData[1] << 8)) ==
           THING_ENDOFLIST);
    assert(world.timeline.count == 0);
}

static void test_orch_slime_wall_impact_emits_wooden_thud_without_explosion(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawSound = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_SLIME;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_WOODEN_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);
}

static void test_orch_projectile_closed_door_impact_destroys_door(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawSound = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR, PROJECTILE_DOOR_STATE_CLOSED_FULL);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_FULL);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_DOOR_DESTRUCTION);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].mapIndex == 0);
    assert(world.timeline.events[0].mapX == 2);
    assert(world.timeline.events[0].mapY == 1);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_METALLIC_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_DESTROYED);
    assert(world.timeline.count == 0);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DOOR_STATE);
    assert(result.emissions[0].payload[0] == 2);
    assert(result.emissions[0].payload[1] == 1);
    assert(result.emissions[0].payload[2] == PROJECTILE_DOOR_STATE_DESTROYED);
}

static void test_orch_non_weapon_door_impact_emits_wooden_thud(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawSound = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR, PROJECTILE_DOOR_STATE_CLOSED_FULL);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_SLIME;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 0);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_FULL);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_DOOR_DESTRUCTION);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].mapX == 2);
    assert(world.timeline.events[0].mapY == 1);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_WOODEN_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);
}

static void test_orch_magical_door_impact_schedules_explosion_and_door_attack(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawExplosionAdvance = 0;
    int sawDoorDestruction = 0;
    int sawDoorState = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR, PROJECTILE_DOOR_STATE_CLOSED_FULL);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_FIREBALL;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C000_EXPLOSION_FIREBALL);
    assert(world.explosions.entries[0].attack == 80);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == 2);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_FULL);
    assert(world.timeline.count == 2);
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].aux1 == C000_EXPLOSION_FIREBALL) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[i].kind == TIMELINE_EVENT_DOOR_DESTRUCTION &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].mapX == 2 &&
            world.timeline.events[i].mapY == 1) {
            sawDoorDestruction = 1;
        }
    }
    assert(sawExplosionAdvance == 1);
    assert(sawDoorDestruction == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_DESTROYED);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_DOOR_STATE &&
            result.emissions[i].payload[0] == 2 &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == PROJECTILE_DOOR_STATE_DESTROYED) {
            sawDoorState = 1;
        }
    }
    assert(sawDoorState == 1);
}

static void run_orch_magical_door_adjusted_explosion_case(
    int projectileSubtype,
    int kineticEnergy,
    int expectExplosion,
    int expectedExplosionType,
    int expectedExplosionAttack,
    int expectedExplosionCell)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawExplosionAdvance = 0;
    int sawDoorDestruction = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR, PROJECTILE_DOOR_STATE_CLOSED_FULL);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = projectileSubtype;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = kineticEnergy;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    if (expectExplosion) {
        assert(world.explosions.count == 1);
        assert(world.explosions.entries[0].explosionType == expectedExplosionType);
        assert(world.explosions.entries[0].attack == expectedExplosionAttack);
        assert(world.explosions.entries[0].mapX == 2);
        assert(world.explosions.entries[0].mapY == 1);
        assert(world.explosions.entries[0].cell == expectedExplosionCell);
        assert(world.timeline.count == 2);
    } else {
        assert(world.explosions.count == 0);
        assert(world.timeline.count == 1);
    }
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].aux1 == expectedExplosionType) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[i].kind == TIMELINE_EVENT_DOOR_DESTRUCTION &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].mapX == 2 &&
            world.timeline.events[i].mapY == 1) {
            sawDoorDestruction = 1;
        }
    }
    assert(sawDoorDestruction == 1);
    assert(sawExplosionAdvance == expectExplosion);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_DESTROYED);
}

static void test_orch_magical_door_impact_zero_adjusted_explosion_skips_spawn(void) {
    run_orch_magical_door_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 1, 0, C002_EXPLOSION_LIGHTNING_BOLT,
        0, 2);
    run_orch_magical_door_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_POISON_BOLT, 3, 0, C007_EXPLOSION_POISON_CLOUD,
        0, EXPLOSION_CELL_CENTERED);
}

static void test_orch_magical_door_impact_nonzero_adjusted_explosion_spawns(void) {
    run_orch_magical_door_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 4, 1, C002_EXPLOSION_LIGHTNING_BOLT,
        2, 2);
    run_orch_magical_door_adjusted_explosion_case(
        PROJECTILE_SUBTYPE_POISON_BOLT, 8, 1, C007_EXPLOSION_POISON_CLOUD,
        2, EXPLOSION_CELL_CENTERED);
}

static void run_orch_thrown_potion_door_impact_case(
    int projectileSubtype,
    int potionPower,
    int expectedExplosionType,
    int expectedExplosionCell)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawExplosionAdvance = 0;
    int sawDoorDestruction = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR, PROJECTILE_DOOR_STATE_CLOSED_FULL);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = projectileSubtype;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 12;
    createIn.attack = 10;
    createIn.stepEnergy = 2;
    createIn.potionPower = potionPower;
    createIn.associatedThing = make_thing(THING_TYPE_POTION, 0);
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert((world.projectiles.entries[slot].flags &
            PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == expectedExplosionType);
    assert(world.explosions.entries[0].attack == potionPower);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == expectedExplosionCell);
    assert(world.timeline.count == 2);
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].aux1 == expectedExplosionType) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[i].kind == TIMELINE_EVENT_DOOR_DESTRUCTION &&
            world.timeline.events[i].fireAtTick == 102 &&
            world.timeline.events[i].mapX == 2 &&
            world.timeline.events[i].mapY == 1) {
            sawDoorDestruction = 1;
        }
    }
    assert(sawExplosionAdvance == 1);
    assert(sawDoorDestruction == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_DESTROYED);
}

static void test_orch_thrown_potion_door_impact_uses_potion_power(void) {
    run_orch_thrown_potion_door_impact_case(
        PROJECTILE_SUBTYPE_POISON_CLOUD, 77, C007_EXPLOSION_POISON_CLOUD,
        EXPLOSION_CELL_CENTERED);
    run_orch_thrown_potion_door_impact_case(
        PROJECTILE_SUBTYPE_FIREBALL, 96, C000_EXPLOSION_FIREBALL, 2);
    run_orch_thrown_potion_door_impact_case(
        PROJECTILE_SUBTYPE_POISON_CLOUD, 0, C007_EXPLOSION_POISON_CLOUD,
        EXPLOSION_CELL_CENTERED);
}

static void test_orch_open_door_projectile_toggles_button_door(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;
    int sawDoorState = 0;
    int sawSound = 0;

    init_world(&world, &things, weapons, junks);
    memset(doors, 0, sizeof(doors));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR,
                        DUNGEON_SQUARE_MASK_THING_LIST |
                        PROJECTILE_DOOR_STATE_CLOSED_FULL);
    squareFirstThings[0] = make_thing(THING_TYPE_DOOR, 0);
    doors[0].next = THING_ENDOFLIST;
    doors[0].button = 1;

    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.doors = doors;
    things.doorCount = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 2;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_FULL);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_SENSOR_DELAYED);
    assert(world.timeline.events[0].fireAtTick == 102);
    assert(world.timeline.events[0].mapX == 2);
    assert(world.timeline.events[0].mapY == 1);
    assert(world.timeline.events[0].aux0 == 10);
    assert(world.timeline.events[0].aux1 == DOOR_EFFECT_TOGGLE);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_WOODEN_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_THREE_FOURTH);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_DOOR_ANIMATE);
    assert(world.timeline.events[0].fireAtTick == 103);
    assert(world.timeline.events[0].aux1 == DOOR_EFFECT_SET);

    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_DOOR_STATE &&
            result.emissions[i].payload[0] == 2 &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] ==
                PROJECTILE_DOOR_STATE_CLOSED_THREE_FOURTH) {
            sawDoorState = 1;
        }
    }
    assert(sawDoorState);
}

static void test_orch_open_door_projectile_without_button_only_thuds(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;
    int sawSound = 0;

    init_world(&world, &things, weapons, junks);
    memset(doors, 0, sizeof(doors));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR,
                        DUNGEON_SQUARE_MASK_THING_LIST |
                        PROJECTILE_DOOR_STATE_CLOSED_FULL);
    squareFirstThings[0] = make_thing(THING_TYPE_DOOR, 0);
    doors[0].next = THING_ENDOFLIST;
    doors[0].button = 0;

    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.doors = doors;
    things.doorCount = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 1;
    createIn.cell = 2;
    createIn.direction = 1;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 2;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 1;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert((squareData[(2 * 3) + 1] & 0x07) ==
           PROJECTILE_DOOR_STATE_CLOSED_FULL);
    assert(world.timeline.count == 0);
    for (i = 0; i < result.emissionCount; ++i) {
        assert(result.emissions[i].kind != EMIT_DOOR_STATE);
        if (result.emissions[i].kind == EMIT_SOUND_REQUEST &&
            result.emissions[i].payload[0] == DM1_SND_WOODEN_THUD &&
            result.emissions[i].payload[1] == 1 &&
            result.emissions[i].payload[2] == 1 &&
            result.emissions[i].payload[3] == 0) {
            sawSound = 1;
        }
    }
    assert(sawSound == 1);
}

static void test_orch_projectile_champion_hit_applies_damage(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    world.party.champions[1].wounds = 0;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 60;
    createIn.attack = 30;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.timeline.count == 0);
    /* ReDMCSB PROJEXPL.C F0216 lines 283-296 defaults kinetic
     * projectile impact type to C3 blunt.  F0217 then sends the impact
     * through CHAMPION.C F0321, whose zero-defense body scale is
     * (attack * 130) >> 6, so raw 30 becomes 60. */
    assert(world.party.champions[1].hp.current == 40);
    assert((world.party.champions[1].wounds &
            (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO)) ==
           (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO));
    assert(result.emissionCount == 0);
}

static void test_orch_projectile_champion_hit_uses_lifecycle_shield_defense(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    world.lifecycle.champions[1].shieldDefense = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 60;
    createIn.attack = 30;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    /* ReDMCSB CHAMPION.C F0313 lines 1353-1382 adds Champion.ShieldDefense
     * then halves/bounds the slot defense before F0321 scales physical
     * damage.  Defense 100 becomes 50 over HEAD|TORSO, so raw 30 blunt impact
     * becomes (30 * (130 - 50)) >> 6 == 37. */
    assert(world.party.champions[1].hp.current == 63);
}

static void test_orch_projectile_champion_hit_uses_equipped_armour_defense(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonArmour_Compat armours[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(armours, 0, sizeof(armours));
    things.armours = armours;
    things.armourCount = 2;
    armours[0].type = 39;  /* ReDMCSB DUNGEON.C G0239: TORSO PLATE, defense 65. */
    armours[0].next = THING_ENDOFLIST;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        world.party.champions[1].inventory[i] = THING_NONE;
    }
    world.party.champions[1].inventory[CHAMPION_SLOT_TORSO] =
        make_thing(THING_TYPE_ARMOUR, 0);

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 60;
    createIn.attack = 30;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    /* F0217 applies HEAD|TORSO.  G0239 torso-plate defense 65 is halved by
     * F0313 before it is averaged with the unarmoured head slot for F0321:
     * avgDefense=(0+(65>>1))/2 == 16, damage=(30*(130-16))>>6 == 53. */
    assert(world.party.champions[1].hp.current == 47);
}

static void test_orch_projectile_champion_hit_uses_hand_shield_strength(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonArmour_Compat armours[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(armours, 0, sizeof(armours));
    things.armours = armours;
    things.armourCount = 2;
    armours[0].type = 52;  /* ReDMCSB DUNGEON.C G0239: Shield of Darc. */
    armours[0].next = THING_ENDOFLIST;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].stamina.current = 100;
    world.party.champions[1].stamina.maximum = 100;
    world.party.champions[1].attributes[CHAMPION_ATTR_STRENGTH] = 80;
    world.party.champions[1].maxLoad = 740;
    world.party.champions[1].cell = 1;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        world.party.champions[1].inventory[i] = THING_NONE;
    }
    world.party.champions[1].inventory[CHAMPION_SLOT_HAND_LEFT] =
        make_thing(THING_TYPE_ARMOUR, 0);

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 60;
    createIn.attack = 30;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    /* ReDMCSB CHAMPION.C F0313 lines 1336-1382 adds F0312 hand strength
     * to shield armour defense.  Deterministic F0312 baseline for this hand
     * is 54; Shield of Darc defense 100 gives pre-tail HEAD 24 and TORSO 19,
     * then F0313 halves them to 12 and 9 before F0321 scales raw 30 blunt
     * impact to 56 HP damage. */
    assert(world.party.champions[1].hp.current == 44);
}

static void test_orch_projectile_champion_hit_applies_poison(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawExplosionAdvance = 0;
    int sawPoisonEvent = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    world.party.champions[1].poisonDose = 0;
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 3u) == 1);

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 80;
    createIn.attack = 32;
    createIn.stepEnergy = 10;
    createIn.poisonAttack = 12;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.party.champions[1].hp.current == 67);
    assert(world.party.champions[1].poisonDose == 12);
    assert(world.lifecycle.champions[1].poisonEventCount == 1);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C007_EXPLOSION_POISON_CLOUD);
    assert(world.explosions.entries[0].attack == 20);
    assert(world.explosions.entries[0].cell == EXPLOSION_CELL_CENTERED);
    assert(world.explosions.entries[0].centered == 1);

    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
            world.timeline.events[i].fireAtTick == 102) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[i].kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            world.timeline.events[i].fireAtTick == 137 &&
            world.timeline.events[i].aux0 == LIFECYCLE_STATUS_POISON &&
            world.timeline.events[i].aux1 == 11 &&
            world.timeline.events[i].aux4 == 1) {
            sawPoisonEvent = 1;
        }
    }
    assert(sawExplosionAdvance);
    assert(sawPoisonEvent);
}

static void test_orch_projectile_champion_hit_uses_f0321_magic_scale(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    world.party.champions[1].attributes[CHAMPION_ATTR_ANTIMAGIC] = 170;
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 3u) == 1);

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_MAGICAL;
    createIn.subtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 80;
    createIn.attack = 64;
    createIn.stepEnergy = 10;
    createIn.attackTypeCode = COMBAT_ATTACK_MAGIC;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    /* ReDMCSB CHAMPION.C F0321 lines 1878-1888: C5 magic uses
     * antimagic F0307 scaling and skips the armor-defense body scale.
     * raw 64 with antimagic 170 becomes 8, not the old raw 64. */
    assert(world.party.champions[1].hp.current == 92);
    assert((world.party.champions[1].wounds &
            (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO)) ==
           (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO));
}

static void test_orch_projectile_champion_hit_uses_f0313_rng_scale(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    world.party.championCount = 2;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 100;
    world.party.champions[1].cell = 1;
    world.party.champions[1].attributes[CHAMPION_ATTR_VITALITY] = 64;
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u) == 1);

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CREATURE;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 32;
    createIn.attack = 32;
    createIn.stepEnergy = 10;
    createIn.attackTypeCode = COMBAT_ATTACK_BLUNT;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    /* ReDMCSB CHAMPION.C F0321 calls F0313 for HEAD and TORSO.  With
     * seed 1 and vitality 64, Firestaff's deterministic RNG draws 8 then 7
     * for RANDOM((64 >> 3) + 1), leaving the champion at 37 HP. */
    assert(world.party.champions[1].hp.current == 37);
    assert(world.masterRng.seed == 0x967eb0e7u);
    assert((world.party.champions[1].wounds &
            (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO)) ==
           (COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO));
}

static void test_orch_projectile_group_hit_applies_damage(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_SCREAMER;
    groups[0].count = 0;
    groups[0].health[0] = 1000;
    groups[0].cells = 0xFFu;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 100;
    world.creatureAI[0].reserved0 = 0;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 3;
    createIn.mapY = 1;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 200;
    createIn.attack = 200;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(world.timeline.events[0].fireAtTick == 108);
    assert(world.timeline.events[0].mapIndex == 0);
    assert(world.timeline.events[0].mapX == 2);
    assert(world.timeline.events[0].mapY == 1);
    assert(world.timeline.events[0].aux0 == 0);
    assert(world.timeline.events[0].aux1 == CREATURE_TYPE_SCREAMER);
    assert(world.timeline.events[0].aux2 == DM1_EVENT_REACTION_HIT_BY_PROJECTILE);
    assert(groups[0].health[0] < 1000);
    assert(groups[0].health[0] > 0);
}

static void test_orch_projectile_group_hit_at_zero_coordinate(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonGroup_Compat groups[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[0] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 2;
    world.party.mapY = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    groups[0].cells = 0xFFu;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 0;
    world.creatureAI[0].groupMapY = 0;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 100;
    world.creatureAI[0].reserved0 = 0;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 1;
    createIn.mapY = 0;
    createIn.cell = 3;
    createIn.direction = 3;
    createIn.kineticEnergy = 80;
    createIn.attack = 40;
    createIn.stepEnergy = 10;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(groups[0].health[0] < 100);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(world.timeline.events[0].mapIndex == 0);
    assert(world.timeline.events[0].mapX == 0);
    assert(world.timeline.events[0].mapY == 0);
    assert(world.timeline.events[0].aux0 == 0);
    assert(world.timeline.events[0].aux1 == CREATURE_TYPE_GIANT_SCORPION);
    assert(world.timeline.events[0].aux2 == DM1_EVENT_REACTION_HIT_BY_PROJECTILE);
}

static void test_orch_projectile_group_hit_all_kill_cleans_up_group(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawKillNotify = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_SCREAMER;
    groups[0].count = 0;
    groups[0].health[0] = 1;
    groups[0].cells = 0xFFu;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].reserved0 = 0;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 3;
    createIn.mapY = 1;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 80;
    createIn.attack = 80;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(groups[0].health[0] == 0);
    assert(groups[0].next == THING_NONE);
    assert(squareFirstThings[0] != make_thing(THING_TYPE_GROUP, 0));
    assert(world.creatureAICount == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C040_EXPLOSION_SMOKE);
    assert(world.explosions.entries[0].mapIndex == 0);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == EXPLOSION_CELL_CENTERED);
    assert(world.explosions.entries[0].attack == 110);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_KILL_NOTIFY &&
            result.emissions[i].payload[0] == 0 &&
            result.emissions[i].payload[2] ==
                COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            sawKillNotify = 1;
        }
    }
    assert(sawKillNotify);
}

static void test_orch_projectile_group_hit_killed_some_applies_f0190_side_effects(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[8];
    unsigned char rawGroupData[16];
    unsigned char rawWeaponData[32];
    unsigned char rawArmourData[32];
    unsigned char rawJunkData[32];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TimelineEvent_Compat creatureEvent;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int sawKillNotify = 0;
    int sawExplosionAdvance = 0;
    int sawProjectileReaction = 0;
    int sawShiftedAspect = 0;
    int sawShiftedBehavior = 0;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(weapons, 0, sizeof(weapons));
    memset(armours, 0, sizeof(armours));
    memset(junks, 0, sizeof(junks));
    memset(rawGroupData, 0xee, sizeof(rawGroupData));
    memset(rawWeaponData, 0, sizeof(rawWeaponData));
    memset(rawArmourData, 0, sizeof(rawArmourData));
    memset(rawJunkData, 0, sizeof(rawJunkData));
    for (i = 1; i < 8; ++i) weapons[i].next = THING_NONE;
    for (i = 0; i < 8; ++i) armours[i].next = THING_NONE;
    for (i = 0; i < 8; ++i) junks[i].next = THING_NONE;
    for (i = 0; i < 8; ++i) {
        rawWeaponData[(i * 4) + 0] = (unsigned char)(THING_NONE & 0xffu);
        rawWeaponData[(i * 4) + 1] = (unsigned char)(THING_NONE >> 8);
        rawWeaponData[(i * 4) + 2] = 0x7eu;
        rawArmourData[(i * 4) + 0] = (unsigned char)(THING_NONE & 0xffu);
        rawArmourData[(i * 4) + 1] = (unsigned char)(THING_NONE >> 8);
        rawArmourData[(i * 4) + 2] = 0x7du;
        rawJunkData[(i * 4) + 0] = (unsigned char)(THING_NONE & 0xffu);
        rawJunkData[(i * 4) + 1] = (unsigned char)(THING_NONE >> 8);
        rawJunkData[(i * 4) + 2] = 0x7cu;
    }
    things.weapons = weapons;
    things.weaponCount = 8;
    things.armours = armours;
    things.armourCount = 8;
    things.junks = junks;
    things.junkCount = 8;
    things.thingCounts[THING_TYPE_WEAPON] = 8;
    things.thingCounts[THING_TYPE_ARMOUR] = 8;
    things.thingCounts[THING_TYPE_JUNK] = 8;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    things.rawThingData[THING_TYPE_ARMOUR] = rawArmourData;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroupData;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_ANIMATED_ARMOUR;
    groups[0].count = 1;
    groups[0].health[0] = 1;
    groups[0].health[1] = 200;
    groups[0].cells = 0x09u; /* creature 0 in cell 1, creature 1 in cell 2. */
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 100;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 3;
    createIn.mapY = 1;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 80;
    createIn.attack = 80;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&creatureEvent, 0, sizeof(creatureEvent));
    creatureEvent.kind = TIMELINE_EVENT_CREATURE_REACTION;
    creatureEvent.fireAtTick = 140;
    creatureEvent.mapIndex = 0;
    creatureEvent.mapX = 2;
    creatureEvent.mapY = 1;
    creatureEvent.aux0 = 0;
    creatureEvent.aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    world.timeline.events[world.timeline.count++] = creatureEvent;
    creatureEvent.aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    world.timeline.events[world.timeline.count++] = creatureEvent;
    creatureEvent.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    world.timeline.events[world.timeline.count++] = creatureEvent;
    creatureEvent.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1;
    world.timeline.events[world.timeline.count++] = creatureEvent;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(groups[0].count == 0);
    assert(groups[0].health[0] == 200);
    assert(squareFirstThings[0] == make_thing(THING_TYPE_GROUP, 0));
    assert(read_u16_le_for_test(rawGroupData + 0) == groups[0].next);
    assert(read_u16_le_for_test(rawGroupData + 2) == groups[0].slot);
    assert(rawGroupData[4] == groups[0].creatureType);
    assert(rawGroupData[5] == groups[0].cells);
    assert(read_u16_le_for_test(rawGroupData + 6) == groups[0].health[0]);
    assert(read_u16_le_for_test(rawGroupData + 8) == groups[0].health[1]);
    assert(read_u16_le_for_test(rawGroupData + 10) == groups[0].health[2]);
    assert(read_u16_le_for_test(rawGroupData + 12) == groups[0].health[3]);
    assert(((read_u16_le_for_test(rawGroupData + 14) >> 5) & 0x03u) ==
           groups[0].count);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C040_EXPLOSION_SMOKE);
    assert(world.explosions.entries[0].mapIndex == 0);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == 1);
    assert(world.explosions.entries[0].attack == 110);
    assert(groups[0].next != THING_ENDOFLIST);
    assert(THING_GET_TYPE(groups[0].next) == THING_TYPE_ARMOUR);
    assert(armours[0].type == 41);
    assert(armours[0].cursed == 1);
    assert(weapons[1].type == 10);
    assert(weapons[1].cursed == 1);
    assert(weapons[2].type == 10);
    assert(weapons[2].cursed == 1);
    assert(read_u16_le_for_test(rawArmourData + 0) == armours[0].next);
    assert(read_u16_le_for_test(rawArmourData + 2) == 0x0129u);
    assert(read_u16_le_for_test(rawWeaponData + 4) == weapons[1].next);
    assert(read_u16_le_for_test(rawWeaponData + 6) == 0x010au);
    assert(read_u16_le_for_test(rawWeaponData + 8) == weapons[2].next);
    assert(read_u16_le_for_test(rawWeaponData + 10) == 0x010au);

    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_KILL_NOTIFY &&
            result.emissions[i].payload[0] == 0 &&
            result.emissions[i].payload[2] ==
                COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
            sawKillNotify = 1;
        }
    }
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[i].kind == TIMELINE_EVENT_CREATURE_REACTION) {
            if (world.timeline.events[i].aux2 == DM1_EVENT_REACTION_HIT_BY_PROJECTILE) {
                sawProjectileReaction = 1;
            }
            if (world.timeline.events[i].aux2 == DM1_EVENT_UPDATE_ASPECT_CREATURE_0) {
                sawShiftedAspect++;
            }
            if (world.timeline.events[i].aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) {
                sawShiftedBehavior++;
            }
            assert(world.timeline.events[i].aux2 !=
                   DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1);
            assert(world.timeline.events[i].aux2 !=
                   DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1);
        }
    }
    assert(sawKillNotify);
    assert(sawExplosionAdvance);
    assert(sawProjectileReaction);
    assert(sawShiftedAspect == 1);
    assert(sawShiftedBehavior == 1);
}

static void test_orch_projectile_group_hit_keeps_thrown_sharp_weapon(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    struct ProjectileCreateInput_Compat createIn;
    struct TimelineEvent_Compat firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    weapons[0].type = 27; /* ReDMCSB C27_WEAPON_ARROW. */
    weapons[0].next = THING_ENDOFLIST;
    junks[0].next = THING_ENDOFLIST;
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = make_thing(THING_TYPE_JUNK, 0);
    groups[0].creatureType = CREATURE_TYPE_WIZARD_EYE;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    groups[0].cells = 0xFFu;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 100;
    world.creatureAI[0].reserved0 = 0;

    memset(&createIn, 0, sizeof(createIn));
    createIn.category = PROJECTILE_CATEGORY_KINETIC;
    createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
    createIn.ownerIndex = 0;
    createIn.mapIndex = 0;
    createIn.mapX = 3;
    createIn.mapY = 1;
    createIn.cell = 0;
    createIn.direction = 3;
    createIn.kineticEnergy = 10;
    createIn.attack = 10;
    createIn.stepEnergy = 5;
    createIn.currentTick = 100;
    createIn.firstMoveGraceFlag = 0;
    assert(F0810_PROJECTILE_Create_Compat(
        &createIn, &world.projectiles, &slot, &firstMove) == 1);
    world.projectiles.entries[slot].reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(groups[0].health[0] > 0);
    assert(groups[0].slot == make_thing(THING_TYPE_WEAPON, 0));
    assert(weapons[0].next == make_thing(THING_TYPE_JUNK, 0));
    assert(junks[0].next == THING_ENDOFLIST);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(world.timeline.events[0].aux1 == CREATURE_TYPE_WIZARD_EYE);
}

static void test_orch_f0266_group_move_precheck_keeps_thrown_sharp_weapon(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonProjectile_Compat dungeonProjectiles[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[3];
    unsigned short squareFirstThings[2];
    struct DungeonGroup_Compat groups[1];
    struct TimelineEvent_Compat creatureTick;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(dungeonProjectiles, 0, sizeof(dungeonProjectiles));
    for (i = 0; i < 3; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[0] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    squareFirstThings[1] = make_thing(THING_TYPE_PROJECTILE, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 1;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 3;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 2;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    things.groups = groups;
    things.groupCount = 1;
    things.projectiles = dungeonProjectiles;
    things.projectileCount = 1;
    weapons[0].type = 27; /* ReDMCSB C27_WEAPON_ARROW. */
    weapons[0].next = THING_ENDOFLIST;
    junks[0].next = THING_ENDOFLIST;
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = make_thing(THING_TYPE_JUNK, 0);
    groups[0].creatureType = CREATURE_TYPE_WIZARD_EYE;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    groups[0].cells = 0xFFu;
    dungeonProjectiles[0].next = THING_ENDOFLIST;
    dungeonProjectiles[0].slot = make_thing(THING_TYPE_WEAPON, 0);
    dungeonProjectiles[0].kineticEnergy = 10;
    dungeonProjectiles[0].attack = 10;
    dungeonProjectiles[0].eventIndex = 0;

    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 0;
    world.creatureAI[0].groupMapY = 0;
    world.creatureAI[0].groupDirection = DIR_EAST;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 100;
    world.creatureAI[0].reserved0 = 0;

    memset(&creatureTick, 0, sizeof(creatureTick));
    creatureTick.kind = TIMELINE_EVENT_CREATURE_TICK;
    creatureTick.fireAtTick = 101;
    creatureTick.mapIndex = 0;
    creatureTick.mapX = 0;
    creatureTick.mapY = 0;
    creatureTick.aux0 = 0;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &creatureTick) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(groups[0].health[0] > 0);
    assert(groups[0].health[0] < 100);
    assert(groups[0].slot == make_thing(THING_TYPE_WEAPON, 0));
    assert(weapons[0].next == make_thing(THING_TYPE_JUNK, 0));
    assert(junks[0].next == THING_ENDOFLIST);
    assert(dungeonProjectiles[0].next == THING_NONE);
    assert(dungeonProjectiles[0].eventIndex == 0xFFFFu);
    assert(squareFirstThings[0] == THING_ENDOFLIST);
    assert(squareFirstThings[1] == make_thing(THING_TYPE_GROUP, 0));
    assert(world.creatureAI[0].groupMapX == 1);
    assert(world.creatureAI[0].groupMapY == 0);
}

static void test_orch_explosion_advance_applies_group_damage(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstAdvance;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(1 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.champions[0].hp.current = 100;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFFu;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = 0;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].reserved0 = 0;

    memset(&explosionIn, 0, sizeof(explosionIn));
    explosionIn.explosionType = C000_EXPLOSION_FIREBALL;
    explosionIn.attack = 80;
    explosionIn.mapIndex = 0;
    explosionIn.mapX = 1;
    explosionIn.mapY = 1;
    explosionIn.cell = 2;
    explosionIn.currentTick = 100;
    explosionIn.creatorProjectileSlot = -1;
    assert(F0821_EXPLOSION_Create_Compat(
        &explosionIn, &world.explosions, &slot, &firstAdvance) == 1);
    assert(slot == 0);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstAdvance) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    assert(groups[0].health[0] < 200);
    assert(groups[0].health[0] > 0);
}

static void test_orch_explosion_advance_applies_party_damage(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstAdvance;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].wounds = 0;

    memset(&explosionIn, 0, sizeof(explosionIn));
    explosionIn.explosionType = C000_EXPLOSION_FIREBALL;
    explosionIn.attack = 40;
    explosionIn.mapIndex = 0;
    explosionIn.mapX = 1;
    explosionIn.mapY = 1;
    explosionIn.cell = 2;
    explosionIn.currentTick = 100;
    explosionIn.creatorProjectileSlot = -1;
    assert(F0821_EXPLOSION_Create_Compat(
        &explosionIn, &world.explosions, &slot, &firstAdvance) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstAdvance) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    assert(world.party.champions[0].hp.current < 100);
    assert(world.party.champions[0].hp.current > 0);
    assert((world.party.champions[0].wounds &
            (COMBAT_WOUND_READY_HAND | COMBAT_WOUND_ACTION_HAND |
             COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO |
             COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET)) ==
           (COMBAT_WOUND_READY_HAND | COMBAT_WOUND_ACTION_HAND |
            COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO |
            COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET));
}

static void test_orch_explosion_advance_fire_shield_blocks_party_damage(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstAdvance;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].wounds = 0;
    world.magic.fireShieldDefense = 255;

    memset(&explosionIn, 0, sizeof(explosionIn));
    explosionIn.explosionType = C000_EXPLOSION_FIREBALL;
    explosionIn.attack = 40;
    explosionIn.mapIndex = 0;
    explosionIn.mapX = 1;
    explosionIn.mapY = 1;
    explosionIn.cell = 2;
    explosionIn.currentTick = 100;
    explosionIn.creatorProjectileSlot = -1;
    assert(F0821_EXPLOSION_Create_Compat(
        &explosionIn, &world.explosions, &slot, &firstAdvance) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstAdvance) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
           ORCH_OK);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    assert(world.party.champions[0].hp.current == 100);
    assert(world.party.champions[0].wounds == 0);
}

static void test_orch_explosion_advance_emits_party_champion_down(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstAdvance;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int slot = -1;
    int i;
    int sawDown = 0;
    int sawPartyDead = 0;

    init_world(&world, &things, weapons, junks);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.champions[0].hp.current = 1;
    world.party.champions[0].wounds = 0;

    memset(&explosionIn, 0, sizeof(explosionIn));
    explosionIn.explosionType = C000_EXPLOSION_FIREBALL;
    explosionIn.attack = 40;
    explosionIn.mapIndex = 0;
    explosionIn.mapX = 1;
    explosionIn.mapY = 1;
    explosionIn.cell = 2;
    explosionIn.currentTick = 100;
    explosionIn.creatorProjectileSlot = -1;
    assert(F0821_EXPLOSION_Create_Compat(
        &explosionIn, &world.explosions, &slot, &firstAdvance) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstAdvance) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
           ORCH_PARTY_DEAD);
    assert(world.explosions.count == 0);
    assert(world.timeline.count == 0);
    assert(world.party.champions[0].hp.current == 0);
    assert(world.partyDead == 1);

    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_CHAMPION_DOWN &&
            result.emissions[i].payload[0] == 0) {
            sawDown = 1;
        }
        if (result.emissions[i].kind == EMIT_PARTY_DEAD) {
            sawPartyDead = 1;
        }
    }
    assert(sawDown);
    assert(sawPartyDead);
}

static void test_champion_cell_serializes_through_reserved_v1_byte(void) {
    struct ChampionState_Compat champion;
    struct ChampionState_Compat decoded;
    struct PartyState_Compat party;
    unsigned char championBuf[CHAMPION_SERIALIZED_SIZE];
    unsigned char partyV1[PARTY_SERIALIZED_V1_SIZE];

    F0600_CHAMPION_InitEmpty_Compat(&champion);
    champion.present = 1;
    champion.cell = 3;
    champion.direction = 2;
    assert(F0602_CHAMPION_Serialize_Compat(
        &champion, championBuf, sizeof(championBuf)) == CHAMPION_SERIALIZED_SIZE);
    assert(championBuf[13] == 3);
    memset(&decoded, 0, sizeof(decoded));
    assert(F0603_CHAMPION_Deserialize_Compat(
        &decoded, championBuf, sizeof(championBuf)) == CHAMPION_SERIALIZED_SIZE);
    assert(decoded.cell == 3);
    assert(decoded.direction == 2);

    memset(partyV1, 0, sizeof(partyV1));
    partyV1[0] = 1;
    partyV1[32 + 0] = 1;
    partyV1[32 + 10] = 1;
    partyV1[32 + 13] = 2;
    memset(&party, 0, sizeof(party));
    assert(F0605_PARTY_Deserialize_Compat(
        &party, partyV1, sizeof(partyV1)) == PARTY_SERIALIZED_V1_SIZE);
    assert(party.champions[0].present == 1);
    assert(party.champions[0].direction == 1);
    assert(party.champions[0].cell == 2);
}

static void test_orch_cmd_attack_emits_live_f0312_skill_bonus_snapshot(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    world.party.champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] = 0;
    world.party.champions[0].skillLevels[CHAMPION_SKILL_NINJA] = 0;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_NINJA].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_THROW].experience = 500;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 2;
    input.reserved2 = CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[0] == 0);
    assert(result.emissions[0].payload[1] == 2);
    assert(result.emissions[0].payload[3] == 4);
}

static void test_orch_cmd_attack_prefers_live_action_hand_weapon_class(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    DM1_WeaponInfo info;

    init_world(&world, &things, weapons, junks);
    weapons[0].type = 8; /* ReDMCSB DUNGEON.C weapon type 8: DAGGER, class 2. */
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] = 0;
    world.party.champions[0].skillLevels[CHAMPION_SKILL_NINJA] = 0;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_NINJA].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_THROW].experience = 500;

    assert(F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
        &world, 0, &info) == 1);
    assert(info.weight == 5);
    assert(info.weaponClass == 2);
    assert(info.strength == 10);
    assert(info.kineticEnergy == 19);
    assert(info.attributes == 0x0200);
    assert(F0888_ORCH_GetChampionActionHandWeaponClass_Compat(&world, 0) == 2);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0; /* Fallback would be swing-only; live dagger wins. */
    input.reserved2 = CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[0] == 0);
    assert(result.emissions[0].payload[1] == 2);
    assert(result.emissions[0].payload[3] == 4);
}

static void test_orch_cmd_attack_unresolved_weapon_without_marker_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 9753) == 1);
    seedBefore = world.masterRng.seed;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
}

static void test_orch_creature_snapshot_uses_live_group_and_profile(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct CombatantCreatureSnapshot_Compat snapshot;

    init_world(&world, &things, weapons, junks);
    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0; /* Giant Scorpion, profile C00. */
    groups[0].count = 0;       /* One live creature. */
    groups[0].health[0] = 77;
    things.groups = groups;
    things.groupCount = 1;

    assert(F0888_ORCH_GetCreatureSnapshot_Compat(
        &world, 0, 0, 6, &snapshot) == 1);
    assert(snapshot.creatureType == 0);
    assert(snapshot.attack == 40);
    assert(snapshot.defense == 30);
    assert(snapshot.dexterity == 40);
    assert(snapshot.baseHealth == 80);
    assert(snapshot.poisonAttack == 5);
    assert(snapshot.attackType == COMBAT_ATTACK_NORMAL);
    assert(snapshot.attributes == 0);
    assert(snapshot.woundProbabilities == 0x0222);
    assert(snapshot.properties == 0x299B);
    assert(snapshot.doubledMapDifficulty == 6);
    assert(snapshot.creatureIndex == 0);
    assert(snapshot.healthBefore == 77);
    assert(snapshot.isCandidateInvulnerable == 0);

    assert(F0888_ORCH_GetCreatureSnapshot_Compat(
        &world, 0, 1, 6, &snapshot) == 0);
    assert(snapshot.creatureType == -1);
    assert(snapshot.creatureIndex == -1);
}

static int run_live_cmd_attack_damage_attempt(unsigned int seed,
                                              int actionIndex,
                                              int* outDamage,
                                              int* outBefore,
                                              int* outAfter) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    weapons[0].type = 8; /* ReDMCSB DUNGEON.C weapon type 8: DAGGER. */
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_NINJA].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_THROW].experience = 500;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0; /* Giant Scorpion, profile C00. */
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0; /* Live path treats this as group index. */
    input.reserved = 0;    /* Creature slot. */
    if (actionIndex >= 0) {
        input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
            (unsigned int)actionIndex;
    }

    *outBefore = groups[0].health[0];
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[0] == 0);
    assert(result.emissions[0].payload[1] == 0);
    assert(groups[0].health[0] <= *outBefore);

    *outDamage = result.emissions[0].payload[2];
    *outAfter = groups[0].health[0];
    return *outDamage > 0 && *outAfter < *outBefore;
}

static void test_orch_cmd_attack_applies_live_group_damage(void) {
    int sawMutation = 0;
    int damage = 0;
    int before = 0;
    int after = 0;
    unsigned int seed;

    for (seed = 1; seed <= 64 && !sawMutation; ++seed) {
        sawMutation = run_live_cmd_attack_damage_attempt(
            seed, -1, &damage, &before, &after);
    }

    assert(sawMutation == 1);
    assert(damage > 0);
    assert(after == before - damage || after == 0);
}

static int run_live_cmd_attack_f0231_side_effect_attempt(
    unsigned int seed,
    int forceWeakChampion,
    int* outDamage,
    int* outStaminaBefore,
    int* outStaminaAfter,
    int* outXpBefore,
    int* outXpAfter) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct ChampionState_Compat* champion;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);
    world.gameTick = 20;
    world.lifecycle.lastCreatureAttackTime = 20;

    weapons[0].type = 8; /* ReDMCSB DUNGEON.C weapon type 8: DAGGER. */
    champion = &world.party.champions[0];
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    champion->hp.current = 100;
    champion->stamina.current = 100;
    champion->stamina.maximum = 100;
    champion->attributes[CHAMPION_ATTR_STRENGTH] =
        (unsigned char)(forceWeakChampion ? 0 : 100);
    champion->attributes[CHAMPION_ATTR_DEXTERITY] =
        (unsigned char)(forceWeakChampion ? 0 : 100);
    champion->attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience =
            forceWeakChampion ? 0 : 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience =
            forceWeakChampion ? 0 : 500;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = forceWeakChampion ? 9 : 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;

    *outStaminaBefore = champion->stamina.current;
    *outXpBefore = world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience;
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    *outDamage = result.emissions[0].payload[2];
    *outStaminaAfter = champion->stamina.current;
    *outXpAfter = world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience;
    return *outDamage > 0;
}

static void test_orch_cmd_attack_awards_f0231_xp_and_hit_stamina(void) {
    int sawHit = 0;
    int damage = 0;
    int staminaBefore = 0;
    int staminaAfter = 0;
    int xpBefore = 0;
    int xpAfter = 0;
    unsigned int seed;

    for (seed = 1; seed <= 128 && !sawHit; ++seed) {
        sawHit = run_live_cmd_attack_f0231_side_effect_attempt(
            seed, 0, &damage, &staminaBefore, &staminaAfter,
            &xpBefore, &xpAfter);
    }

    assert(sawHit == 1);
    assert(damage > 0);
    assert(staminaBefore == 100);
    assert(staminaAfter >= 93 && staminaAfter <= 96);
    assert(xpAfter > xpBefore);
}

static void test_orch_cmd_attack_spends_f0231_miss_stamina_without_xp(void) {
    int sawMiss = 0;
    int damage = 0;
    int staminaBefore = 0;
    int staminaAfter = 0;
    int xpBefore = 0;
    int xpAfter = 0;
    unsigned int seed;

    for (seed = 1; seed <= 256 && !sawMiss; ++seed) {
        int hit = run_live_cmd_attack_f0231_side_effect_attempt(
            seed, 1, &damage, &staminaBefore, &staminaAfter,
            &xpBefore, &xpAfter);
        sawMiss = !hit;
    }

    assert(sawMiss == 1);
    assert(damage == 0);
    assert(staminaBefore == 100);
    assert(staminaAfter == 97 || staminaAfter == 98);
    assert(xpAfter == xpBefore);
}

static int run_live_cmd_attack_f0312_strength_attempt(
    unsigned int seed,
    int weakenedActionHand,
    int* outDamage) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct ChampionState_Compat* champion;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    weapons[0].type = 45; /* Complete Firestaff: weight 36, strength 100. */
    champion = &world.party.champions[0];
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    champion->hp.current = 100;
    champion->stamina.maximum = 100;
    champion->stamina.current = weakenedActionHand ? 0 : 100;
    champion->attributes[CHAMPION_ATTR_STRENGTH] = 100;
    champion->attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    champion->attributes[CHAMPION_ATTR_VITALITY] = 100;
    champion->maxLoad = 32;
    champion->wounds = weakenedActionHand ? COMBAT_WOUND_ACTION_HAND : 0;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    *outDamage = result.emissions[0].payload[2];
    return 1;
}

static void test_orch_cmd_attack_uses_f0312_weight_stamina_wound_strength(void) {
    int sawF0312Drop = 0;
    int healthyDamage = 0;
    int weakenedDamage = 0;
    unsigned int seed;

    for (seed = 1; seed <= 1024 && !sawF0312Drop; ++seed) {
        int healthy = 0;
        int weakened = 0;
        run_live_cmd_attack_f0312_strength_attempt(seed, 0, &healthy);
        run_live_cmd_attack_f0312_strength_attempt(seed, 1, &weakened);
        if (healthy > 0 && healthy > weakened) {
            healthyDamage = healthy;
            weakenedDamage = weakened;
            sawF0312Drop = 1;
        }
    }

    assert(sawF0312Drop == 1);
    assert(healthyDamage > weakenedDamage);
}

static void run_live_cmd_attack_closed_door_attempt(int doorSet,
                                                    int meleeDestructible,
                                                    int actionIndex,
                                                    int* outSquare,
                                                    int* outEmissionCount,
                                                    int* outTimelineCount,
                                                    int* outDispatchSquare,
                                                    int* outDispatchEmissions) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonDoor_Compat doors[1];
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 7) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(doors, 0, sizeof(doors));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].doorSet0 = (unsigned char)doorSet;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] =
        (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                        DUNGEON_SQUARE_MASK_THING_LIST | 4);
    squareFirstThings[0] = make_thing(THING_TYPE_DOOR, 0);

    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].stamina.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    weapons[0].type = 45; /* Complete Firestaff: enough for wooden door. */

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.doors = doors;
    things.doorCount = 1;
    doors[0].next = THING_ENDOFLIST;
    doors[0].type = 0;
    doors[0].meleeDestructible = (unsigned char)meleeDestructible;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)actionIndex;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    *outSquare = squareData[(2 * 3) + 1];
    *outEmissionCount = result.emissionCount;
    *outTimelineCount = world.timeline.count;

    memset(&result, 0, sizeof(result));
    world.gameTick = 2;
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    *outDispatchSquare = squareData[(2 * 3) + 1];
    *outDispatchEmissions = result.emissionCount;
}

static void test_orch_cmd_attack_f0407_closed_door_attack(void) {
    const int doorHitActions[] = {
        DM1_ACTION_CHOP,
        DM1_ACTION_KICK,
        DM1_ACTION_SWING,
        DM1_ACTION_HACK,
        DM1_ACTION_BERZERK,
        DM1_ACTION_BASH
    };
    const int meleeFallthroughActions[] = {
        DM1_ACTION_BLOCK,
        DM1_ACTION_STUN,
        DM1_ACTION_MELEE,
        DM1_ACTION_DISRUPT
    };
    int square = 0;
    int emissions = 0;
    int timelineCount = 0;
    int dispatchSquare = 0;
    int dispatchEmissions = 0;
    size_t i;

    run_live_cmd_attack_closed_door_attempt(
        1, 1, DM1_ACTION_SWING, &square, &emissions, &timelineCount,
        &dispatchSquare, &dispatchEmissions);
    assert((square & 0x07) == 4);
    assert(emissions == 0);
    assert(timelineCount == 2);
    assert((dispatchSquare & 0x07) == 5);
    assert(dispatchEmissions == 2);

    run_live_cmd_attack_closed_door_attempt(
        2, 1, DM1_ACTION_SWING, &square, &emissions, &timelineCount,
        &dispatchSquare, &dispatchEmissions);
    assert((square & 0x07) == 4);
    assert(emissions == 0);
    assert(timelineCount == 1);
    assert((dispatchSquare & 0x07) == 4);
    assert(dispatchEmissions == 1);

    run_live_cmd_attack_closed_door_attempt(
        1, 0, DM1_ACTION_SWING, &square, &emissions, &timelineCount,
        &dispatchSquare, &dispatchEmissions);
    assert((square & 0x07) == 4);
    assert(emissions == 0);
    assert(timelineCount == 1);
    assert((dispatchSquare & 0x07) == 4);
    assert(dispatchEmissions == 1);

    /* ReDMCSB MENU.C F0407 lines 1308-1324 limits closed-door melee to
     * BASH/HACK/BERZERK/KICK/SWING/CHOP before the F0402 melee fallthrough. */
    for (i = 0; i < sizeof(doorHitActions) / sizeof(doorHitActions[0]); ++i) {
        run_live_cmd_attack_closed_door_attempt(
            1, 1, doorHitActions[i], &square, &emissions, &timelineCount,
            &dispatchSquare, &dispatchEmissions);
        assert((square & 0x07) == 4);
        assert(emissions == 0);
        assert(timelineCount == 2);
        assert((dispatchSquare & 0x07) == 5);
        assert(dispatchEmissions == 2);
    }

    for (i = 0;
         i < sizeof(meleeFallthroughActions) / sizeof(meleeFallthroughActions[0]);
         ++i) {
        run_live_cmd_attack_closed_door_attempt(
            1, 1, meleeFallthroughActions[i], &square, &emissions,
            &timelineCount, &dispatchSquare, &dispatchEmissions);
        assert((square & 0x07) == 4);
        assert(emissions == 0);
        assert(timelineCount == 0);
        assert((dispatchSquare & 0x07) == 4);
        assert(dispatchEmissions == 0);
    }
}

static int run_live_cmd_attack_reaction_schedule_attempt(unsigned int seed) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;
    world.gameTick = 40;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].stamina.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 0;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 0;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 9; /* Stone Golem: hard to damage. */
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].reserved0 = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(world.timeline.events[0].fireAtTick == 41);
    assert(world.timeline.events[0].mapIndex == 0);
    assert(world.timeline.events[0].mapX == 2);
    assert(world.timeline.events[0].mapY == 1);
    assert(world.timeline.events[0].aux0 == 0);
    assert(world.timeline.events[0].aux2 == DM1_EVENT_REACTION_PARTY_IS_ADJACENT);

    memset(&result, 0, sizeof(result));
    world.gameTick = 41;
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert(world.timeline.count == 0);
    assert(world.creatureAI[0].stateKind == AI_STATE_ATTACK);
    assert(world.creatureAI[0].lastSeenPartyMapX == 1);
    assert(world.creatureAI[0].lastSeenPartyMapY == 1);
    return 1;
}

static void test_orch_cmd_attack_schedules_f0231_adjacent_reaction(void) {
    assert(run_live_cmd_attack_reaction_schedule_attempt(1) == 1);
}

static int run_live_cmd_attack_all_kill_attempt(unsigned int seed) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int e;
    int sawKillNotify = 0;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
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

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 1;
    groups[0].cells = 0xFFu;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].reserved0 = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    for (e = 0; e < result.emissionCount; ++e) {
        if (result.emissions[e].kind == EMIT_KILL_NOTIFY &&
            result.emissions[e].payload[2] == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            sawKillNotify = 1;
        }
    }
    if (!sawKillNotify) return 0;

    assert(groups[0].health[0] == 0);
    assert(groups[0].next == THING_NONE);
    assert(squareFirstThings[0] != make_thing(THING_TYPE_GROUP, 0));
    assert(world.creatureAICount == 0);
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C040_EXPLOSION_SMOKE);
    assert(world.explosions.entries[0].mapIndex == 0);
    assert(world.explosions.entries[0].mapX == 2);
    assert(world.explosions.entries[0].mapY == 1);
    assert(world.explosions.entries[0].cell == EXPLOSION_CELL_CENTERED);
    assert(world.explosions.entries[0].attack == 110);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE);
    return 1;
}

static void test_orch_cmd_attack_all_kill_unlinks_group_without_reaction(void) {
    int sawAllKill = 0;
    unsigned int seed;
    for (seed = 1; seed <= 128 && !sawAllKill; ++seed) {
        sawAllKill = run_live_cmd_attack_all_kill_attempt(seed);
    }
    assert(sawAllKill == 1);
}

static int run_live_cmd_attack_killed_some_smoke_attempt(unsigned int seed) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[8];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int sawKillNotify = 0;
    int sawExplosionAdvance = 0;
    int sawReaction = 0;
    int sawShiftedAspect = 0;
    int sawShiftedBehavior = 0;
    int e;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);
    world.gameTick = 90;
    memset(weapons, 0, sizeof(weapons));
    memset(armours, 0, sizeof(armours));
    memset(junks, 0, sizeof(junks));
    for (e = 1; e < 8; ++e) weapons[e].next = THING_NONE;
    for (e = 0; e < 8; ++e) armours[e].next = THING_NONE;
    for (e = 0; e < 8; ++e) junks[e].next = THING_NONE;
    weapons[0].type = 8;
    things.weapons = weapons;
    things.weaponCount = 8;
    things.armours = armours;
    things.armourCount = 8;
    things.junks = junks;
    things.junkCount = 8;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;

    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
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

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 18;
    groups[0].count = 1;
    groups[0].health[0] = 1;
    groups[0].health[1] = 200;
    groups[0].cells = 0x0004; /* creature 0 in cell 0, creature 1 in cell 1. */
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&world.timeline.events[0], 0, sizeof(world.timeline.events[0]) * 4);
    world.timeline.count = 4;
    world.timeline.events[0].kind = TIMELINE_EVENT_CREATURE_REACTION;
    world.timeline.events[0].fireAtTick = 120;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 2;
    world.timeline.events[0].mapY = 1;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    world.timeline.events[1] = world.timeline.events[0];
    world.timeline.events[1].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    world.timeline.events[2] = world.timeline.events[0];
    world.timeline.events[2].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    world.timeline.events[3] = world.timeline.events[0];
    world.timeline.events[3].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    for (e = 0; e < result.emissionCount; ++e) {
        if (result.emissions[e].kind == EMIT_KILL_NOTIFY &&
            result.emissions[e].payload[2] == COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
            sawKillNotify = 1;
        }
    }
    if (!sawKillNotify) return 0;

    assert(groups[0].count == 0);
    assert(groups[0].health[0] == 200);
    assert(squareFirstThings[0] == make_thing(THING_TYPE_GROUP, 0));
    assert(world.explosions.count == 1);
    assert(world.explosions.entries[0].explosionType == C040_EXPLOSION_SMOKE);
    assert(world.explosions.entries[0].cell == 0);
    assert(world.explosions.entries[0].attack == 110);
    assert(groups[0].next != THING_ENDOFLIST);
    assert(THING_GET_TYPE(groups[0].next) == THING_TYPE_ARMOUR);
    assert(armours[0].type == 41);
    assert(armours[0].cursed == 1);
    assert(weapons[1].type == 10);
    assert(weapons[1].cursed == 1);
    assert(weapons[2].type == 10);
    assert(weapons[2].cursed == 1);
    assert(world.timeline.count == 4);
    for (e = 0; e < world.timeline.count; ++e) {
        if (world.timeline.events[e].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[e].kind == TIMELINE_EVENT_CREATURE_REACTION) {
            if (world.timeline.events[e].aux2 == DM1_EVENT_REACTION_PARTY_IS_ADJACENT) {
                sawReaction = 1;
            }
            if (world.timeline.events[e].aux2 == DM1_EVENT_UPDATE_ASPECT_CREATURE_0) {
                sawShiftedAspect++;
            }
            if (world.timeline.events[e].aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) {
                sawShiftedBehavior++;
            }
            assert(world.timeline.events[e].aux2 !=
                   DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1);
            assert(world.timeline.events[e].aux2 !=
                   DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1);
        }
    }
    assert(sawExplosionAdvance == 1);
    assert(sawReaction == 1);
    assert(sawShiftedAspect == 1);
    assert(sawShiftedBehavior == 1);
    return 1;
}

static void test_orch_cmd_attack_killed_some_creates_f0190_smoke(void) {
    int sawKilledSome = 0;
    unsigned int seed;
    for (seed = 1; seed <= 128 && !sawKilledSome; ++seed) {
        sawKilledSome = run_live_cmd_attack_killed_some_smoke_attempt(seed);
    }
    assert(sawKilledSome == 1);
}

static int run_live_cmd_attack_killed_some_fear_attempt(unsigned int seed) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int sawKillNotify = 0;
    int sawExplosionAdvance = 0;
    int sawAdjacentReaction = 0;
    int e;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);
    world.gameTick = 160;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
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

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 1;
    groups[0].health[0] = 1;
    groups[0].health[1] = 200;
    groups[0].cells = 0x0004;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 2;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_SWING;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    for (e = 0; e < result.emissionCount; ++e) {
        if (result.emissions[e].kind == EMIT_KILL_NOTIFY &&
            result.emissions[e].payload[2] == COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
            sawKillNotify = 1;
        }
    }
    if (!sawKillNotify) return 0;
    if (groups[0].behavior != DM1_BEHAVIOR_FLEE) return 0;

    assert(world.creatureAICount == 1);
    assert(world.creatureAI[0].stateKind == AI_STATE_FLEE);
    assert(world.creatureAI[0].fearCounter >= 20);
    assert(world.explosions.count == 1);
    for (e = 0; e < world.timeline.count; ++e) {
        if (world.timeline.events[e].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
            sawExplosionAdvance = 1;
        }
        if (world.timeline.events[e].kind == TIMELINE_EVENT_CREATURE_REACTION &&
            world.timeline.events[e].aux2 == DM1_EVENT_REACTION_PARTY_IS_ADJACENT) {
            sawAdjacentReaction = 1;
        }
    }
    assert(sawExplosionAdvance == 1);
    assert(sawAdjacentReaction == 0);
    return 1;
}

static void test_orch_cmd_attack_killed_some_f0190_fear_skips_reaction(void) {
    int sawFear = 0;
    unsigned int seed;
    for (seed = 1; seed <= 512 && !sawFear; ++seed) {
        sawFear = run_live_cmd_attack_killed_some_fear_attempt(seed);
    }
    assert(sawFear == 1);
}

static void test_orch_cmd_attack_uses_reserved2_action_damage_factor(void) {
    int sawDifferentDamage = 0;
    int swingDamage = 0;
    int swingBefore = 0;
    int swingAfter = 0;
    int meleeDamage = 0;
    int meleeBefore = 0;
    int meleeAfter = 0;
    unsigned int seed;

    /* ReDMCSB MENU.C F0407/F0402 feeds F0231 through the Graphic560
     * action tables.  The live CMD_ATTACK bridge must keep reading the
     * shared G0492/G0493 source-lock accessors, not a private action table. */
    assert(dm1_v1_graphic560_action_hit_probability_get_pc34(
               DM1_ACTION_SWING) == 32);
    assert(dm1_v1_graphic560_action_damage_factor_get_pc34(
               DM1_ACTION_SWING) == 16);
    assert(dm1_v1_graphic560_action_hit_probability_get_pc34(
               CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34) == 64);
    assert(dm1_v1_graphic560_action_damage_factor_get_pc34(
               CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34) == 60);

    for (seed = 1; seed <= 128 && !sawDifferentDamage; ++seed) {
        int swingMutated = run_live_cmd_attack_damage_attempt(
            seed, DM1_ACTION_SWING, &swingDamage, &swingBefore, &swingAfter);
        int meleeMutated = run_live_cmd_attack_damage_attempt(
            seed, CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34,
            &meleeDamage, &meleeBefore, &meleeAfter);

        if (swingMutated && meleeMutated && meleeDamage > swingDamage) {
            sawDifferentDamage = 1;
        }
    }

    assert(sawDifferentDamage == 1);
    assert(swingAfter == swingBefore - swingDamage || swingAfter == 0);
    assert(meleeAfter == meleeBefore - meleeDamage || meleeAfter == 0);
}

static int run_live_cmd_attack_map_difficulty_attempt(unsigned int seed,
                                                      int mapDifficulty,
                                                      int* outDamage) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    maps[0].difficulty = (unsigned char)mapDifficulty;
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;

    weapons[0].type = 8; /* DAGGER. */
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    *outDamage = result.emissions[0].payload[2];
    return *outDamage > 0;
}

static void test_orch_cmd_attack_uses_doubled_map_difficulty(void) {
    int sawDifficultyEffect = 0;
    int lowDamage = 0;
    int highDamage = 0;
    unsigned int seed;

    for (seed = 1; seed <= 1024 && !sawDifficultyEffect; ++seed) {
        int lowHit = run_live_cmd_attack_map_difficulty_attempt(
            seed, 0, &lowDamage);
        (void)run_live_cmd_attack_map_difficulty_attempt(
            seed, 15, &highDamage);

        if (lowHit && lowDamage > highDamage) {
            sawDifficultyEffect = 1;
        }
    }

    assert(sawDifficultyEffect == 1);
    assert(lowDamage > highDamage);
}

static int run_live_cmd_attack_auto_front_group_attempt(unsigned int seed,
                                                        int* outDamage,
                                                        int* outAfter0,
                                                        int* outAfter1) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East: target square is (2,1). */

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 1;
    groups[0].cells = (unsigned char)((0 << 0) | (1 << 2));
    groups[0].health[0] = 200;
    groups[0].health[1] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[0] == 0);
    assert(result.emissions[0].payload[1] == 0);
    *outDamage = result.emissions[0].payload[2];
    *outAfter0 = groups[0].health[0];
    *outAfter1 = groups[0].health[1];
    return *outDamage > 0 && *outAfter0 == 200 && *outAfter1 < 200;
}

static void test_orch_cmd_attack_auto_targets_front_group(void) {
    int sawMutation = 0;
    int damage = 0;
    int after0 = 0;
    int after1 = 0;
    unsigned int seed;

    for (seed = 1; seed <= 128 && !sawMutation; ++seed) {
        sawMutation = run_live_cmd_attack_auto_front_group_attempt(
            seed, &damage, &after0, &after1);
    }

    assert(sawMutation == 1);
    assert(damage > 0);
    assert(after0 == 200);
    assert(after1 == 200 - damage || after1 == 0);
}

static void test_orch_cmd_attack_auto_target_without_group_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1234) == 1);
    seedBefore = world.masterRng.seed;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East: target square is empty. */

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
}

static void test_orch_cmd_attack_direct_live_action_without_group_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 4321) == 1);
    seedBefore = world.masterRng.seed;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0; /* Direct group-index request; no group exists. */
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
}

static void test_orch_cmd_attack_direct_invalid_group_with_live_table_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 2468) == 1);
    seedBefore = world.masterRng.seed;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 99; /* Invalid direct group-index request. */
    input.reserved = 0;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
    assert(groups[0].health[0] == 200);
}

static void test_orch_cmd_attack_direct_invalid_creature_with_live_table_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1357) == 1);
    seedBefore = world.masterRng.seed;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 3; /* Invalid explicit creature ordinal for a one-creature group. */
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
    assert(groups[0].health[0] == 200);
}

static int run_live_cmd_attack_empty_hand_punch_attempt(
    unsigned int seed,
    int* outDamage,
    int* outBefore,
    int* outAfter) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].stamina.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_PUNCH;

    *outBefore = groups[0].health[0];
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[0] == 0);
    assert(result.emissions[0].payload[1] == 0);
    *outDamage = result.emissions[0].payload[2];
    *outAfter = groups[0].health[0];
    return *outDamage > 0 && *outAfter < *outBefore;
}

static void test_orch_cmd_attack_empty_hand_punch_uses_live_melee(void) {
    int sawMutation = 0;
    int damage = 0;
    int before = 0;
    int after = 0;
    unsigned int seed;

    for (seed = 1; seed <= 256 && !sawMutation; ++seed) {
        sawMutation = run_live_cmd_attack_empty_hand_punch_attempt(
            seed, &damage, &before, &after);
    }

    assert(sawMutation == 1);
    assert(damage > 0);
    assert(after == before - damage || after == 0);
}

static void test_orch_cmd_attack_dead_champion_live_target_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 5678) == 1);
    seedBefore = world.masterRng.seed;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East: target square has a group. */

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 0;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].cells = 0xFFu;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
    assert(groups[0].health[0] == 200);
}

static void test_orch_cmd_attack_invalid_champion_auto_target_is_noop(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 9012) == 1);
    seedBefore = world.masterRng.seed;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East: target square has a group. */

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].cells = 0xFFu;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 99;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(groups[0].health[0] == 200);
}

static void test_orch_cmd_attack_candidate_target_no_action_side_effects(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t seedBefore;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 3456) == 1);
    seedBefore = world.masterRng.seed;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East: target square has a panel-owned candidate. */

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].cells = 0xFFu;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    world.candidateAttackInvulnerableEnabled = 1;
    world.candidateAttackInvulnerableGroupIndex = 0;
    world.candidateAttackInvulnerableCreatureIndex = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 0);
    assert(world.masterRng.seed == seedBefore);
    assert(world.party.champions[0].stamina.current == 100);
    assert(groups[0].health[0] == 200);
}

static void test_orch_cmd_attack_back_row_blocked_by_front_champion(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East. */
    world.party.championCount = 2;
    world.party.activeChampionIndex = 0;

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 3;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    world.party.champions[1].present = 1;
    world.party.champions[1].cell = 2;
    world.party.champions[1].hp.current = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].cells = 0xFFu;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[1] == 0);
    assert(result.emissions[0].payload[2] == 0);
    assert(groups[0].health[0] == 200);
    assert(world.party.champions[0].stamina.current == 100);
}

static void test_orch_cmd_attack_disrupt_rejects_material_creature(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1) == 1);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    world.dungeon = &dungeon;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.party.direction = 1; /* East. */

    weapons[0].type = 8; /* DAGGER. */
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].stamina.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].count = 0;
    groups[0].cells = 0xFFu;
    groups[0].health[0] = 200;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    input.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)DM1_ACTION_DISRUPT;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    assert(result.emissions[0].payload[1] == 0);
    assert(result.emissions[0].payload[2] == 0);
    assert(groups[0].health[0] == 200);
    assert(world.party.champions[0].stamina.current == 100);
}

static int run_live_cmd_attack_skill_route_attempt(unsigned int seed,
                                                   int actionIndex,
                                                   int clubExperience,
                                                   int* outDamage) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    weapons[0].type = 8; /* DAGGER. */
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 255;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_CLUB].experience = (unsigned long)clubExperience;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)actionIndex;

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_DAMAGE_DEALT);
    *outDamage = result.emissions[0].payload[2];
    return *outDamage > 0;
}

static void test_orch_cmd_attack_uses_reserved2_action_skill_index(void) {
    DM1_ActionXpRoute route;
    int sawSkillBoost = 0;
    int swingDamage = 0;
    int stunDamage = 0;
    int defaultDamage = 0;
    int invalidDamage = 0;
    unsigned int seed;

    assert(CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34 == DM1_ACTION_MELEE);
    assert(dm1_v1_action_xp_route(DM1_GRAPHIC560_ACTION_COUNT, &route) == 0);
    assert(route.valid == 0);

    assert(dm1_v1_action_xp_route(DM1_ACTION_SWING, &route) == 1);
    assert(route.valid == 1);
    assert(route.skillIndex == DM1_SKILL_IDX_SWING);
    assert(route.baseSkillIndex == DM1_SKILL_IDX_FIGHTER);

    assert(dm1_v1_action_xp_route(DM1_ACTION_STUN, &route) == 1);
    assert(route.valid == 1);
    assert(route.skillIndex == DM1_SKILL_IDX_CLUB);
    assert(route.baseSkillIndex == DM1_SKILL_IDX_FIGHTER);

    assert(dm1_v1_action_xp_route(DM1_ACTION_MELEE, &route) == 1);
    assert(route.valid == 1);
    assert(route.skillIndex == DM1_SKILL_IDX_CLUB);
    assert(route.baseSkillIndex == DM1_SKILL_IDX_FIGHTER);

    for (seed = 1; seed <= 512 && !sawSkillBoost; ++seed) {
        int swingHit = run_live_cmd_attack_skill_route_attempt(
            seed, DM1_ACTION_SWING, 0, &swingDamage);
        int stunHit = run_live_cmd_attack_skill_route_attempt(
            seed, DM1_ACTION_STUN, 500000, &stunDamage);

        if (swingHit && stunHit && stunDamage > swingDamage) {
            sawSkillBoost = 1;
        }
    }

    assert(sawSkillBoost == 1);

    for (seed = 1; seed <= 16; ++seed) {
        int defaultHit = run_live_cmd_attack_skill_route_attempt(
            seed, CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34, 500000,
            &defaultDamage);
        int invalidHit = run_live_cmd_attack_skill_route_attempt(
            seed, DM1_GRAPHIC560_ACTION_COUNT, 500000, &invalidDamage);
        assert(defaultHit == invalidHit);
        assert(defaultDamage == invalidDamage);
    }
}

static int run_live_cmd_attack_luck_attempt(unsigned int seed,
                                            int* outBefore,
                                            int* outAfter) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct DungeonGroup_Compat groups[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    init_world(&world, &things, weapons, junks);
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, seed) == 1);

    weapons[0].type = 8;
    world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 0;
    world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    world.lifecycle.champions[0]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT] = 20;
    world.lifecycle.champions[0]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MAXIMUM] = 30;
    world.lifecycle.champions[0]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MINIMUM] = 0;

    memset(groups, 0, sizeof(groups));
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    things.groups = groups;
    things.groupCount = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_ATTACK;
    input.commandArg1 = 0;
    input.commandArg2 = 0;
    input.reserved = 0;
    input.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
        (unsigned int)CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;

    *outBefore = world.lifecycle.champions[0]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT];
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(result.emissionCount == 1);
    *outAfter = world.lifecycle.champions[0]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT];
    return *outAfter != *outBefore;
}

static void test_orch_cmd_attack_writes_back_luck_after_f0735(void) {
    int sawLuckMutation = 0;
    int before = 0;
    int after = 0;
    unsigned int seed;

    for (seed = 1; seed <= 512 && !sawLuckMutation; ++seed) {
        sawLuckMutation = run_live_cmd_attack_luck_attempt(
            seed, &before, &after);
    }

    assert(sawLuckMutation == 1);
    assert(before == 20);
    assert(after == 18 || after == 22);
}

int main(void) {
    test_orch_f0303_inventory_and_rest_query();
    test_orch_f0303_hidden_heal_query();
    test_orch_projectile_spell_uses_hidden_skill_query_value();
    test_orch_light_spell_uses_source_light_amount_and_party_map();
    test_orch_potion_spell_mutates_empty_flask_in_hand();
    test_orch_zokathra_spell_materializes_in_ready_hand();
    test_orch_zokathra_spell_falls_back_to_party_square();
    test_orch_f0312_skill_bonus_uses_live_f0303_values();
    test_combat_f0313_wound_defense_final_shift_and_clamp();
    test_orch_turn_rotates_champion_cell_and_direction();
    test_orch_projectile_create_preserves_associated_thing();
    test_orch_projectile_move_event_advances_and_reschedules();
    test_orch_projectile_wall_impact_creates_explosion();
    test_orch_magical_wall_impact_zero_adjusted_explosion_skips_spawn_and_sound();
    test_orch_magical_wall_impact_nonzero_adjusted_explosion_spawns();
    test_orch_projectile_wall_impact_emits_non_explosion_sound();
    test_orch_projectile_wall_impact_materializes_associated_weapon();
    test_orch_projectile_wall_impact_appends_associated_weapon_raw_tail();
    test_orch_slime_wall_impact_emits_wooden_thud_without_explosion();
    test_orch_projectile_closed_door_impact_destroys_door();
    test_orch_non_weapon_door_impact_emits_wooden_thud();
    test_orch_magical_door_impact_schedules_explosion_and_door_attack();
    test_orch_magical_door_impact_zero_adjusted_explosion_skips_spawn();
    test_orch_magical_door_impact_nonzero_adjusted_explosion_spawns();
    test_orch_thrown_potion_door_impact_uses_potion_power();
    test_orch_open_door_projectile_toggles_button_door();
    test_orch_open_door_projectile_without_button_only_thuds();
    test_orch_projectile_champion_hit_applies_damage();
    test_orch_projectile_champion_hit_uses_lifecycle_shield_defense();
    test_orch_projectile_champion_hit_uses_equipped_armour_defense();
    test_orch_projectile_champion_hit_uses_hand_shield_strength();
    test_orch_projectile_champion_hit_applies_poison();
    test_orch_projectile_champion_hit_uses_f0321_magic_scale();
    test_orch_projectile_champion_hit_uses_f0313_rng_scale();
    test_orch_projectile_group_hit_applies_damage();
    test_orch_projectile_group_hit_at_zero_coordinate();
    test_orch_projectile_group_hit_all_kill_cleans_up_group();
    test_orch_projectile_group_hit_killed_some_applies_f0190_side_effects();
    test_orch_projectile_group_hit_keeps_thrown_sharp_weapon();
    test_orch_f0266_group_move_precheck_keeps_thrown_sharp_weapon();
    test_orch_explosion_advance_applies_group_damage();
    test_orch_explosion_advance_applies_party_damage();
    test_orch_explosion_advance_fire_shield_blocks_party_damage();
    test_orch_explosion_advance_emits_party_champion_down();
    test_champion_cell_serializes_through_reserved_v1_byte();
    test_orch_cmd_attack_emits_live_f0312_skill_bonus_snapshot();
    test_orch_cmd_attack_prefers_live_action_hand_weapon_class();
    test_orch_cmd_attack_unresolved_weapon_without_marker_is_noop();
    test_orch_creature_snapshot_uses_live_group_and_profile();
    test_orch_cmd_attack_applies_live_group_damage();
    test_orch_cmd_attack_awards_f0231_xp_and_hit_stamina();
    test_orch_cmd_attack_spends_f0231_miss_stamina_without_xp();
    test_orch_cmd_attack_uses_f0312_weight_stamina_wound_strength();
    test_orch_cmd_attack_f0407_closed_door_attack();
    test_orch_cmd_attack_schedules_f0231_adjacent_reaction();
    test_orch_cmd_attack_all_kill_unlinks_group_without_reaction();
    test_orch_cmd_attack_killed_some_creates_f0190_smoke();
    test_orch_cmd_attack_killed_some_f0190_fear_skips_reaction();
    test_orch_cmd_attack_uses_reserved2_action_damage_factor();
    test_orch_cmd_attack_uses_doubled_map_difficulty();
    test_orch_cmd_attack_auto_targets_front_group();
    test_orch_cmd_attack_auto_target_without_group_is_noop();
    test_orch_cmd_attack_direct_live_action_without_group_is_noop();
    test_orch_cmd_attack_direct_invalid_group_with_live_table_is_noop();
    test_orch_cmd_attack_direct_invalid_creature_with_live_table_is_noop();
    test_orch_cmd_attack_empty_hand_punch_uses_live_melee();
    test_orch_cmd_attack_dead_champion_live_target_is_noop();
    test_orch_cmd_attack_invalid_champion_auto_target_is_noop();
    test_orch_cmd_attack_candidate_target_no_action_side_effects();
    test_orch_cmd_attack_back_row_blocked_by_front_champion();
    test_orch_cmd_attack_disrupt_rejects_material_creature();
    test_orch_cmd_attack_uses_reserved2_action_skill_index();
    test_orch_cmd_attack_writes_back_luck_after_f0735();
    puts("ok: M10 orchestrator DM1 F0303 skill query uses lifecycle inventory/rest inputs");
    return 0;
}
