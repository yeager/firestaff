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
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = 0;
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
        unsigned char raw[16];
        unsigned short dagger =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

        memset(&state, 0, sizeof(state));
        memset(&profile, 0, sizeof(profile));
        init_csb_dungeon(&dungeon, raw, sizeof(raw));
        write_u16(raw + 0, THING_ENDOFLIST);
        write_u16(raw + 2, 8u); /* ReDMCSB OBJECT.C F0033 dagger icon C032. */

        profile.runtime.dungeon_handle = &dungeon;
        state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
        state.csbBootProfile = &profile;
        state.world.party.championCount = 1;
        state.world.party.activeChampionIndex = 0;
        state.world.party.champions[0].present = 1;
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
    }

    if (g_failures != 0) {
        return 1;
    }
    printf("PASS: m11_csb_leader_hand_no_dm1_fallback\n");
    return 0;
}
