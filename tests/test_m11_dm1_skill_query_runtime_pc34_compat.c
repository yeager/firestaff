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
    champ->mana.current = 100;
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
    assert(state.spellPanelOpen == 0);
    assert(state.spellBuffer.runeCount == 0);
    assert(champ->mana.current < 100);
}

static void test_m11_cast_spell_validation_failure_stops_cast(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junks[2];
    struct ChampionState_Compat* champ;

    init_state(&state, &things, weapons, junks);
    state.world.gameTick = 12;
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
    assert(state.spellPanelOpen == 0);
    assert(state.spellBuffer.runeCount == 0);
    assert(champ->mana.current == 100);
    assert(M11_GameView_GetProjectileCount(&state) == 0);
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

int main(void) {
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
