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

    if (g_failures != 0) {
        return 1;
    }
    printf("PASS: m11_csb_leader_hand_no_dm1_fallback\n");
    return 0;
}
