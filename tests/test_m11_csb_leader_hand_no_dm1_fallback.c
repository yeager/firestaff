/*
 * Data-free regression for the M11 CSB leader-hand handoff.
 *
 * CSBWin GAMEBLOCK2 and ReDMCSB CHAMPION.C F0297/F0298 preserve the
 * transient G4055 leader-hand thing separately from champion equipment.
 * Firestaff imports that raw CSB thing before full CSB object-icon binding
 * exists.  This test pins the important boundary: a missing CSB icon/name
 * binding must stay blank instead of being reinterpreted through DM1's
 * G0237/F0033 object tables.
 */

#include "m11_game_view.h"
#include "csb_v1_boot.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++g_failures;
    }
}

static void write_u16(unsigned char *p, unsigned int value)
{
    p[0] = (unsigned char)(value & 0xffu);
    p[1] = (unsigned char)((value >> 8) & 0xffu);
}

static void init_csb_dungeon(CSB_V1_DungeonData *dungeon,
                             unsigned char *raw,
                             size_t raw_size)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->thing_type_counts[THING_TYPE_WEAPON] = 1;
    dungeon->thing_type_counts[THING_TYPE_CONTAINER] = 1;
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = 0;
    dungeon->thing_data_bases[THING_TYPE_CONTAINER] = 8;
}

int main(void)
{
    M11_GameViewState state;
    char name[32];
    unsigned short thing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    memset(&state, 0, sizeof(state));
    memset(name, 0, sizeof(name));
    state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    state.leaderHandObjectPresent = 1;
    state.leaderHandThing = thing;
    state.leaderHandIconIndex = -1;

    check(M11_GameView_GetV1LeaderHandThing(&state) == thing,
          "CSB raw leader-hand thing is preserved");
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == -1,
          "CSB missing icon binding does not fall back to DM1 icon tables");
    check(!M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
          "CSB missing name binding does not fall back to DM1 item names");
    check(name[0] == '\0',
          "CSB missing name binding leaves output blank");

    state.leaderHandIconIndex = 77;
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 77,
          "CSB explicit icon binding can still surface when present");

    snprintf(state.leaderHandObjectName,
             sizeof(state.leaderHandObjectName),
             "%s",
             "DAGGER");
    memset(name, 0, sizeof(name));
    check(M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
          "CSB explicit runtime name binding can surface when present");
    check(strcmp(name, "DAGGER") == 0,
          "CSB explicit runtime name is returned");

    memset(name, 0, sizeof(name));
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state.leaderHandIconIndex = 77;
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 77,
          "DM1 cached leader-hand icon still surfaces");

    {
        CSB_V1_BootProfile profile;
        CSB_V1_DungeonData dungeon;
        unsigned char actions[3];
        unsigned char raw[32];
        unsigned short dagger =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        unsigned short chest =
            (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
        int sx = 0, sy = 0, sw = 0, sh = 0;

        memset(&state, 0, sizeof(state));
        memset(&profile, 0, sizeof(profile));
        csb_v1_runtime_init(&profile.runtime, NULL);
        init_csb_dungeon(&dungeon, raw, sizeof(raw));
        write_u16(raw + 0, THING_ENDOFLIST);
        write_u16(raw + 2, 8u); /* ReDMCSB OBJECT.C F0033 dagger icon C032. */
        write_u16(raw + 8, THING_ENDOFLIST);
        write_u16(raw + 10, 0u); /* Chest subtype 0; ObjectInfo idx 1. */

        profile.runtime.dungeon_handle = &dungeon;
        profile.runtime.party_state_valid = 1;
        profile.runtime.party_state.ChampionCount = 1;
        profile.runtime.party_state.LeaderHandThing = THING_NONE;
        profile.runtime.party_state.Champions[0].CurrentHealth = 10;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        profile.runtime.party_state.Champions[0].CurrentMana = 10;
        profile.runtime.party_state.Champions[0].Cell = 0;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            dagger;
        state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
        state.csbBootProfile = &profile;
        state.active = 1;
        state.inventoryPanelActive = 1;
        state.world.party.championCount = 1;
        state.world.party.activeChampionIndex = 0;
        state.world.party.champions[0].present = 1;
        state.world.party.champions[0].hp.current = 10;
        state.world.party.champions[0].hp.maximum = 10;
        state.world.party.champions[0].stamina.current = 100;
        state.world.party.champions[0].stamina.maximum = 100;
        state.world.party.champions[0].mana.current = 10;
        state.world.party.champions[0].mana.maximum = 10;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            dagger;

        check(M11_GameView_GetObjectIconIndexForThing(&state, dagger) == 32,
              "CSB object icon accessor uses CSB runtime dungeon records without DM1 world.things");
        check(M11_GameView_GetV1InventorySlotIconIndex(
                  &state,
                  CHAMPION_SLOT_ACTION_HAND) == 32,
              "CSB inventory slot icon accessor uses CSB runtime records without DM1 world.things");
        check(M11_GameView_SetV1LeaderHandObject(&state, dagger),
              "CSB leader-hand set accepts runtime-owned object binding");
        check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 32,
              "CSB leader-hand icon resolves through CSB runtime object binding");
        memset(name, 0, sizeof(name));
        check(M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
              "CSB leader-hand name resolves through CSB runtime object binding");
        check(strcmp(name, "DAGGER") == 0,
              "CSB leader-hand name comes from CSB runtime resolver");
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB action menu opens from runtime object action-set without DM1 world.things");
        check(M11_GameView_GetActingActionIndices(&state, actions),
              "CSB action rows resolve through runtime object action-set");
        check(actions[0] == 42 && actions[1] == 9 && actions[2] == 28,
              "CSB dagger exposes THROW/STAB/SLASH action rows");
        check(M11_GameView_TriggerActionRow(&state, 0) == 1,
              "CSB dagger THROW action dispatches through CSB runtime without DM1 world.things");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB THROW clears M11 action-hand mirror");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB THROW clears runtime action-hand slot");
        check(profile.runtime.projectiles.count == 1,
              "CSB THROW allocates one runtime projectile");
        check(profile.runtime.projectiles.entries[0].reserved1 == dagger,
              "CSB THROW projectile preserves thrown thing identity");
        check(profile.runtime.projectiles.entries[0].ownerKind ==
                  PROJECTILE_OWNER_CHAMPION,
              "CSB THROW projectile is champion-owned");
        check(profile.runtime.timeline_queue.eventCount > 0,
              "CSB THROW schedules first projectile movement event");
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            dagger;
        state.world.party.champions[0].stamina.current = 100;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            dagger;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB action menu reopens for melee action without DM1 world.things");
        check(M11_GameView_TriggerActionRow(&state, 1) == 1,
              "CSB dagger STAB action records through CSB runtime without DM1 attack tick");
        check(profile.runtime.party_state.Champions[0].ActionIndex == actions[1],
              "CSB STAB stores selected action index on runtime champion");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  dagger,
              "CSB STAB keeps action-hand object in M11 mirror");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  dagger,
              "CSB STAB keeps action-hand object in runtime slot");
        check(profile.runtime.projectiles.count == 1,
              "CSB STAB does not allocate a projectile");
        check(profile.runtime.party_state.Champions[0].CurrentStamina < 100,
              "CSB STAB writes M11 stamina cost back to runtime");
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].mana.current = 20;
        profile.runtime.party_state.Champions[0].CurrentMana = 20;
        check(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state,
                  0,
                  DM1_ACTION_FIREBALL) == 1,
              "CSB FIREBALL action spawns through CSB runtime without DM1 projectile list");
        check(profile.runtime.projectiles.count == 2,
              "CSB FIREBALL allocates one additional runtime projectile");
        check(profile.runtime.projectiles.entries[1].projectileCategory ==
                  PROJECTILE_CATEGORY_MAGICAL,
              "CSB FIREBALL projectile is magical");
        check(profile.runtime.projectiles.entries[1].projectileSubtype ==
                  PROJECTILE_SUBTYPE_FIREBALL,
              "CSB FIREBALL projectile keeps source subtype");
        check(profile.runtime.party_state.Champions[0].ActionIndex ==
                  DM1_ACTION_FIREBALL,
              "CSB FIREBALL stores selected action index on runtime champion");
        check(profile.runtime.party_state.Champions[0].CurrentMana < 20,
              "CSB FIREBALL writes mana cost back to runtime");
        check(state.world.projectiles.count == 0,
              "CSB FIREBALL does not allocate into DM1 M11 projectile list");

        M11_GameView_ClearV1LeaderHandObject(&state);
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            chest;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            chest;
        check(M11_GameView_GetV1InventorySourceSlotBoxZone(
                  9, &sx, &sy, &sw, &sh),
              "C508 action-hand slot zone is available for CSB inventory click");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB action-hand slot click picks container into leader hand");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB action-hand M11 mirror is empty after pickup");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB runtime action-hand slot is empty after pickup");
        check(M11_GameView_GetV1LeaderHandThing(&state) == chest,
              "CSB leader hand holds picked-up container");
        check(profile.runtime.party_state.LeaderHandThing == chest,
              "CSB runtime leader hand holds picked-up container");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB action-hand slot click places container from leader hand");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  chest,
              "CSB action-hand M11 mirror stores placed container");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  chest,
              "CSB runtime action-hand slot stores placed container");
        check(M11_GameView_GetV1LeaderHandThing(&state) == THING_NONE,
              "CSB leader hand is empty after placing container");
        check(profile.runtime.party_state.LeaderHandThing == THING_NONE,
              "CSB runtime leader hand is empty after placing container");
    }

    if (g_failures != 0) {
        return 1;
    }
    printf("PASS: m11_csb_leader_hand_no_dm1_fallback\n");
    return 0;
}
