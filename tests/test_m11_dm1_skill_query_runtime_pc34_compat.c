/*
 * test_m11_dm1_skill_query_runtime_pc34_compat.c
 *
 * Verifies that M11's runtime skill query uses the source-locked DM1
 * CHAMPION.C F0303 path, including live inventory object modifiers.
 */
#include "m11_game_view.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static void init_state(M11_GameViewState* state,
                       struct DungeonThings_Compat* things,
                       struct DungeonWeapon_Compat* weapons,
                       struct DungeonJunk_Compat* junks) {
    struct ChampionState_Compat* champ;
    memset(state, 0, sizeof(*state));
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat) * 2);
    memset(junks, 0, sizeof(struct DungeonJunk_Compat) * 2);

    state->active = 1;
    state->world.party.championCount = 1;
    state->world.things = things;

    champ = &state->world.party.champions[0];
    champ->present = 1;
    for (int i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
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

static void test_m11_skill_query_uses_inventory_modifiers(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    champ = &state.world.party.champions[0];
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    champ->inventory[CHAMPION_SLOT_NECK] =
        make_thing(THING_TYPE_JUNK, 0);

    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 500;

    /* Base wizard level 2 + Complete Firestaff 2 + Pendant Feral 1. */
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_WIZARD) == 5);

    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = THING_NONE;
    champ->inventory[CHAMPION_SLOT_NECK] = THING_NONE;
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_WIZARD) == 2);
}

static void test_m11_skill_query_uses_hidden_skill_average_and_heal_items(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    champ = &state.world.party.champions[0];
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 1);
    champ->inventory[CHAMPION_SLOT_NECK] =
        make_thing(THING_TYPE_JUNK, 1);

    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_HEAL].experience = 500;

    /* Hidden HEAL level averages Priest+Heal to level 2, then Gem/Sceptre
     * contributes a single F0303 modifier despite both being equipped. */
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_HEAL) == 3);
}

static void test_m11_skill_query_resting_returns_neophyte(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    champ = &state.world.party.champions[0];
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    champ->inventory[CHAMPION_SLOT_NECK] =
        make_thing(THING_TYPE_JUNK, 0);

    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 4000;
    state.resting = 1;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_WIZARD) == 1);

    state.resting = 0;
    state.world.partyIsResting = 1;
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_WIZARD) == 1);

    state.world.partyIsResting = 0;
    state.world.lifecycle.rest.isResting = 1;
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_WIZARD) == 1);
}

static void test_m11_cast_spell_validation_uses_f0303_skill_query(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    state.world.gameTick = 11;
    state.world.party.activeChampionIndex = 0;
    champ = &state.world.party.champions[0];
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->mana.current = 27; /* On Ful Ir: 3 + 10 + 14 (MENU.C G0485/6). */
    champ->mana.maximum = 100;
    champ->attributes[CHAMPION_ATTR_WISDOM] = 0;
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIRE].experience = 8000;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_FIRE) >= 6);
    assert(M11_GameView_OpenSpellPanel(&state) == 1);
    assert(M11_GameView_EnterRune(&state, 2) == 1); /* On */
    assert(M11_GameView_EnterRune(&state, 3) == 1); /* Ful */
    assert(M11_GameView_EnterRune(&state, 3) == 1); /* Ir */
    assert(M11_GameView_CastSpell(&state) == 1);
    assert(state.spellPanelOpen == 1);
    assert(state.spellBuffer.runeCount == 0);
    assert(champ->mana.current == 0); /* Paid runes cast at zero remaining mana. */
}

static int failed_cast_xp_at_map(int mapIndex, int difficulty, int availability) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    state.world.gameTick = 12;
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    dungeon.header.mapCount = 3;
    dungeon.maps = availability == 2 ? NULL : maps;
    for (int i = 0; i < 3; ++i) maps[i].difficulty = difficulty;
    state.world.dungeon = availability ? &dungeon : NULL;
    state.world.party.mapIndex = mapIndex;
    state.world.party.activeChampionIndex = 0;
    champ = &state.world.party.champions[0];
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->mana.current = 100;
    champ->mana.maximum = 100;
    champ->attributes[CHAMPION_ATTR_WISDOM] = 0;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_FIRE) < 6);
    assert(M11_GameView_OpenSpellPanel(&state) == 1);
    assert(M11_GameView_EnterRune(&state, 2) == 1); /* On */
    assert(M11_GameView_EnterRune(&state, 3) == 1); /* Ful */
    assert(M11_GameView_EnterRune(&state, 3) == 1); /* Ir */
    assert(M11_GameView_CastSpell(&state) == 1);
    assert(state.spellPanelOpen == 1);
    assert(state.spellBuffer.runeCount == 0);
    assert(champ->mana.current == 73); /* Failed practice cannot refund runes. */
    assert(M11_GameView_GetProjectileCount(&state) == 0);
    assert(strcmp(state.lastOutcome,
                  "EMPTY NEEDS MORE PRACTICE WITH THIS WIZARD SPELL.") == 0);
    return state.world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_FIRE].experience;
}

static void test_m11_cast_spell_validation_failure_stops_cast(void) {
    int baseline = failed_cast_xp_at_map(0, 0, 0);
    assert(baseline > 0);
    /* CHAMPION.C F0304:870-871: change difficulty independently of ordinal.
     * RAM map descriptors isolate this input; they are not game media. */
    assert(failed_cast_xp_at_map(0, 3, 1) == baseline * 3);
    assert(failed_cast_xp_at_map(2, 3, 1) == baseline * 3);
    assert(failed_cast_xp_at_map(2, 0, 1) == baseline);
    assert(failed_cast_xp_at_map(2, 3, 0) == baseline);
    assert(failed_cast_xp_at_map(2, 3, 2) == baseline);
    assert(failed_cast_xp_at_map(-1, 3, 1) == baseline);
    assert(failed_cast_xp_at_map(3, 3, 1) == baseline);
}

static void test_m11_f0230_parry_attack_uses_f0303_query(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];

    init_state(&state, &things, weapons, junks);
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PARRY].experience = 500;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_PARRY) == 2);
    assert(M11_GameView_ProbeF0230ParryAdjustedAttack(
               &state, 0, 10, 20, 4) == 30);
}

static void test_m11_f0352_potion_eye_uses_f0303_priest_query(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    char text[32];
    (void)text;

    init_state(&state, &things, weapons, junks);
    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_PRIEST] = 0;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 500;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_PRIEST) == 2);
    assert(M11_GameView_ProbeF0352PotionEyeDescription(
               &state, 0, THING_TYPE_POTION, 150, 80,
               "ROS POTION", text, sizeof(text)) == 1);
    assert(strcmp(text, "a ROS POTION") == 0);
}

static void test_m11_f0407_shoot_and_f0328_throw_use_f0303_hidden_skills(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];

    init_state(&state, &things, weapons, junks);
    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_NINJA] = 0;
    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] = 9;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_NINJA].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_THROW].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SHOOT].experience = 500;

    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_THROW) == 2);
    assert(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_SHOOT) == 2);
    assert(M11_GameView_ProbeF0407ShootAttack(&state, 0, 50) == 104);
    assert(M11_GameView_ProbeF0328ThrowAttack(&state, 0, 15) == 34);
}

static void test_f0304_xp_scaling_boundaries(void) {
    /* CHAMPION.C:866-885: stale halve BEFORE difficulty, strict 150/25
     * boundaries, recent bonus for every hidden skill (not just combat). */
    static const int delays[] = {24,25,26,149,150,151};
    for (int skill = 0; skill < 20; ++skill)
    for (int d = 0; d < 6; ++d)
    for (int difficulty = 0; difficulty <= 3; difficulty += 3) {
        struct ChampionLifecycleState_Compat champion;
        int expected = 7;
        int base = skill < 4 ? skill : (skill - 4) / 4;
        memset(&champion, 0, sizeof(champion));
        if (skill >= 4 && skill <= 11 && delays[d] > 150) expected >>= 1;
        if (difficulty) expected *= difficulty;
        if (skill >= 4 && delays[d] < 25) expected *= 2;
        (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
            &champion, skill, 7, difficulty, 300, 300-delays[d], NULL, NULL);
        assert(champion.skills20[skill].experience == expected);
        assert(champion.skills20[base].experience == expected);
    }
}

static void test_f0304_unsigned_startup_times(void) {
    /* ReDMCSB DEFS.H:5708,5794 and CHAMPION.C:866,883. Explicit
     * expected awards preserve unsigned 32-bit wrap, not elapsed-time math. */
    static const struct { uint32_t now, attack; int combat, magic; } cases[] = {
        {0, 0, 3, 7}, {24, 24, 3, 7}, {25, 25, 6, 14},
        {149, 149, 6, 14}, {150, 150, 14, 14},
        {0, UINT32_MAX - 199, 3, 7},
        {25, UINT32_MAX - 199, 6, 14},
        {300, UINT32_MAX - 199, 14, 14}
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        for (int skill = 4; skill <= 12; skill += 8) {
            struct ChampionLifecycleState_Compat champion;
            memset(&champion, 0, sizeof(champion));
            (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
                &champion, skill, 7, 0, cases[i].now, cases[i].attack, NULL, NULL);
            assert(champion.skills20[skill].experience ==
                   (skill == 4 ? cases[i].combat : cases[i].magic));
        }
    }
}

static void test_f0304_temporary_xp_destination_and_threshold(void) {
    /* CHAMPION.C F0304:887-895: no temporary award to the parent skill;
     * zero XP is a no-op and the 32000 test precedes the bounded addition. */
    static const struct { int xp, before, after; } cases[] = {
        {0, 0, 0}, {7, 0, 1}, {800, 0, 100},
        {800, 31999, 32099}, {800, 32000, 32000},
        {800, 32099, 32099}
    };
    for (int skill = 0; skill < 20; ++skill)
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        struct ChampionLifecycleState_Compat champion;
        int base = skill < 4 ? skill : (skill - 4) / 4;
        memset(&champion, 0, sizeof(champion));
        champion.skills20[skill].temporaryExperience = cases[i].before;
        (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
            &champion, skill, cases[i].xp, 0, 300, 200, NULL, NULL);
        assert(champion.skills20[skill].temporaryExperience == cases[i].after);
        if (skill != base) assert(champion.skills20[base].temporaryExperience == 0);
    }
}

static void test_f0303_has_no_artificial_level_cap(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    /* CHAMPION.C F0303:765-770,822: shift until below 500, no cap.
     * These positive sums fit both signed and unsigned source editions. */
    for (int level = 17; level <= 24; ++level) {
        init_state(&state, &things, weapons, junks);
        state.world.lifecycle.champions[0].skills20[0].experience =
            dm1_skill_level_threshold(level);
        assert(F0848_LIFECYCLE_ComputeSkillLevel_Compat(
            &state.world.lifecycle.champions[0], 0, 1) == level);
        assert(M11_GameView_GetSkillLevel(&state, 0, 0) == level);
        assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
            &state.world, 0, 0) == level);
        --state.world.lifecycle.champions[0].skills20[0].experience;
        state.world.lifecycle.champions[0].skills20[0].temporaryExperience = 1;
        assert(F0848_LIFECYCLE_ComputeSkillLevel_Compat(
            &state.world.lifecycle.champions[0], 0, 1) == level - 1);
        assert(F0848_LIFECYCLE_ComputeSkillLevel_Compat(
            &state.world.lifecycle.champions[0], 0, 0) == level);
        assert(M11_GameView_GetSkillLevel(&state, 0, 0) == level);
        assert(F0888_ORCH_GetChampionF0303SkillLevel_Compat(
            &state.world, 0, 0) == level);
    }
}

static void test_f0304_award_word_width(void) {
    /* CHAMPION.C:834,866-889: unsigned 16-bit assignments; the
     * nonzero gate precedes multiplication and recent-attack doubling. */
    static const struct { int xp, difficulty, recent, expected, temporary; } cases[] = {
        {32768, 2, 0, 0, 1},
        {32768, 0, 1, 0, 1},
        {40000, 2, 1, 28928, 100},
        {65535, 0, 1, 65534, 100},
        {65536, 3, 1, 0, 0},
        {65537, 3, 1, 6, 1}
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        struct ChampionLifecycleState_Compat champion;
        memset(&champion, 0, sizeof(champion));
        (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
            &champion, 12, cases[i].xp, cases[i].difficulty,
            300, cases[i].recent ? 300 : 200, NULL, NULL);
        assert(champion.skills20[12].experience == cases[i].expected);
        assert(champion.skills20[2].experience == cases[i].expected);
        assert(champion.skills20[12].temporaryExperience == cases[i].temporary);
        assert(champion.skills20[2].temporaryExperience == 0);
    }
}

static void test_f0304_level_gain_ignores_temporary_xp(void) {
    struct ChampionLifecycleState_Compat champion;
    int before, after;
    memset(&champion, 0, sizeof(champion));
    champion.skills20[0].experience = 499;
    champion.skills20[0].temporaryExperience = 100;
    assert(F0849_LIFECYCLE_AddSkillExperience_Compat(
        &champion, 4, 1, 0, 300, 200, &before, &after) == 1);
    assert(before == 1 && after == 2);
    memset(&champion, 0, sizeof(champion));
    champion.skills20[0].experience = 398;
    champion.skills20[0].temporaryExperience = 100;
    assert(F0849_LIFECYCLE_AddSkillExperience_Compat(
        &champion, 0, 2, 0, 300, 200, &before, &after) == 0);
    assert(before == 1 && after == 1);
}

static void test_f0304_levelup_bonuses_and_random_order(void)
{
    static const uint32_t seeds[] = {0u, 31459u, 0x12345678u};
    int skill, level;
    size_t seed_index;

    /* Bounded unit fixtures, not game assets or emulator captures.
     * Independent PC34 oracle: CHAMPION.C F0304:896-989 consumes minor,
     * major, vitality and antifire draws before its class switch. Priest
     * and wizard consume mana then antimagic (&3 in MEDIA722); health and
     * stamina are last. BASE.C:1688-1775 owns the single LCG stream. */
    for (skill = 0; skill < 4; ++skill) {
        for (level = 2; level <= 3; ++level) {
            for (seed_index = 0; seed_index < sizeof(seeds) / sizeof(seeds[0]);
                 ++seed_index) {
                struct ChampionLifecycleState_Compat actual, expected;
                struct RngState_Compat rng;
                struct LevelUpMarker_Compat marker;
                uint32_t expected_seed = seeds[seed_index];
                unsigned int draw[8];
                int n, health_factor, stamina_factor, last_draw;
                int magical = skill == LIFECYCLE_SKILL_PRIEST ||
                              skill == LIFECYCLE_SKILL_WIZARD;
                memset(&actual, 0, sizeof(actual));
                memset(actual.statistics, 40, sizeof(actual.statistics));
                actual.maxHealth = 123;
                actual.maxStamina = 987;
                actual.maxMana = 234;
                expected = actual;
                for (n = 0; n < (magical ? 8 : 6); ++n) {
                    expected_seed = expected_seed * UINT32_C(0xbb40e62d) + 11u;
                    draw[n] = (unsigned int)((expected_seed >> 8) & 0xffffu);
                }
                expected.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM] +=
                    (uint8_t)((draw[2] & 1u) &
                              (skill == LIFECYCLE_SKILL_PRIEST ? 1u : (unsigned int)level));
                expected.statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_MAXIMUM] +=
                    (uint8_t)((draw[3] & 1u) & ~(unsigned int)level);
                health_factor = level;
                if (skill == LIFECYCLE_SKILL_FIGHTER) {
                    expected.statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM] +=
                        (uint8_t)(1u + (draw[1] & 1u));
                    expected.statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM] +=
                        (uint8_t)(draw[0] & 1u);
                    health_factor = 3 * level;
                    stamina_factor = 987 >> 4;
                } else if (skill == LIFECYCLE_SKILL_NINJA) {
                    expected.statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM] +=
                        (uint8_t)(draw[0] & 1u);
                    expected.statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM] +=
                        (uint8_t)(1u + (draw[1] & 1u));
                    health_factor = 2 * level;
                    stamina_factor = 987 / 21;
                } else {
                    unsigned int mana_random = draw[4] & 3u;
                    if (mana_random > (unsigned int)(level - 1)) mana_random = (unsigned int)(level - 1);
                    expected.maxMana += (uint16_t)(level + mana_random);
                    expected.statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_MAXIMUM] +=
                        (uint8_t)(draw[5] & 3u);
                    if (skill == LIFECYCLE_SKILL_WIZARD) {
                        expected.maxMana += (uint16_t)(level >> 1);
                        expected.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM] +=
                            (uint8_t)(1u + (draw[1] & 1u));
                        stamina_factor = 987 >> 5;
                    } else {
                        expected.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM] +=
                            (uint8_t)(draw[0] & 1u);
                        health_factor += (level + 1) >> 1;
                        stamina_factor = 987 / 25;
                    }
                }
                last_draw = magical ? 6 : 4;
                expected.maxHealth += (uint16_t)(health_factor +
                    draw[last_draw] % (unsigned int)((health_factor >> 1) + 1));
                expected.maxStamina += (uint16_t)(stamina_factor +
                    draw[last_draw + 1] % (unsigned int)((stamina_factor >> 1) + 1));
                rng.seed = seeds[seed_index];
                memset(&marker, 0, sizeof(marker));
                assert(F0850_LIFECYCLE_ApplyLevelUp_Compat(&actual, skill, level,
                                                         &rng, &marker) == 1);
                assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
                assert(rng.seed == expected_seed);
                assert(marker.baseSkillIndex == skill && marker.previousLevel == level - 1 &&
                       marker.newLevel == level && marker.championIndex == -1);
            }
        }
    }
}

int main(void) {
    {
        struct LifecycleState_Compat lifecycle;
        struct GameWorld_Compat world;
        memset(&world, 0, sizeof(world));
        assert(F0859_LIFECYCLE_Init_Compat(&lifecycle, NULL) == 1);
        /* ReDMCSB PROJEXPL.C:5: initial signed -200 as a 32-bit word. */
        assert(lifecycle.lastCreatureAttackTime == UINT32_MAX - 199u);
        assert(F0881_WORLD_InitDefault_Compat(&world, 1u) == 1);
        assert(world.lifecycle.lastCreatureAttackTime == UINT32_MAX - 199u);
        F0883_WORLD_Free_Compat(&world);
    }
    test_f0303_has_no_artificial_level_cap();
    test_f0304_levelup_bonuses_and_random_order();
    test_f0304_award_word_width();
    test_f0304_level_gain_ignores_temporary_xp();
    test_f0304_temporary_xp_destination_and_threshold();
    test_f0304_xp_scaling_boundaries();
    test_f0304_unsigned_startup_times();
    test_m11_skill_query_uses_inventory_modifiers();
    test_m11_skill_query_uses_hidden_skill_average_and_heal_items();
    test_m11_skill_query_resting_returns_neophyte();
    test_m11_cast_spell_validation_uses_f0303_skill_query();
    test_m11_cast_spell_validation_failure_stops_cast();
    test_m11_f0230_parry_attack_uses_f0303_query();
    test_m11_f0352_potion_eye_uses_f0303_priest_query();
    test_m11_f0407_shoot_and_f0328_throw_use_f0303_hidden_skills();
    puts("ok: M11 DM1 runtime skill query uses F0303 inventory/rest inputs");
    return 0;
}
