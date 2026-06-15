/*
 * DM1 V1 inventory status-hand runtime probe.
 *
 * Firestaff-side runtime evidence: opens the V1 inventory with an action-hand
 * chest already open, then clicks a non-inventory champion's top-row ready
 * hand through the real M11 pointer route.  The click must resolve as
 * C020..C027 status-hand routing, swap the leader-hand object with that
 * champion hand, and leave the active champion's open chest sentinel intact.
 *
 * Source evidence:
 *   ReDMCSB CLIKCHAM.C F0367 line 32 dispatches C020..C027 to F0302 with
 *   slotBoxIndex = command - C020.
 *   ReDMCSB CHAMPION.C F0302 lines 677-683 rejects candidate flow, the
 *   currently open inventory champion, out-of-party champions, and dead
 *   champions before mapping status hand slot boxes.
 *   ReDMCSB DEFS.H line 1878 M070_HAND_SLOT_INDEX maps even status slot boxes
 *   to ready hand and odd status slot boxes to action hand.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_CHAMPION_COUNT = 4,
    PROBE_REQUIRED_JUNKS = 7
};

static unsigned short thing_ref(int thingType, int thingIndex)
{
    return (unsigned short)(((thingType & 0x0F) << 10) |
                            (thingIndex & 0x03FF));
}

static int expect_true(const char* label, int ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = DIR_NORTH;
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 80;
    champ->mana.current = 40;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static int seed_records(M11_GameViewState* game,
                        unsigned short actionChestThing)
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < PROBE_REQUIRED_JUNKS) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = thing_ref(THING_TYPE_JUNK, 0);
    things->containers[0].type = 0;

    for (i = 0; i < PROBE_REQUIRED_JUNKS; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)((i % 2) + 1);
        things->junks[i].next = THING_ENDOFLIST;
    }

    memset(game->world.party.champions, 0,
           sizeof(game->world.party.champions));
    seed_champion(&game->world.party.champions[0], "TIGGY", 0);
    seed_champion(&game->world.party.champions[1], "HALK", 1);
    seed_champion(&game->world.party.champions[2], "WUUF", 2);
    seed_champion(&game->world.party.champions[3], "ALEX", 3);
    game->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        actionChestThing;
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    M11_GameView_ClearV1LeaderHandObject(game);
    return 1;
}

static int click_status_hand(M11_GameViewState* game,
                             int championSlot,
                             int handIndex)
{
    int x, y, w, h;

    if (!M11_GameView_GetV1StatusHandZone(
            championSlot, handIndex, &x, &y, &w, &h)) {
        return M11_GAME_INPUT_IGNORED;
    }
    return (int)M11_GameView_HandlePointerButton(
        game, x + w / 2, y + h / 2, M11_DM1_MOUSE_MASK_LEFT);
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const unsigned short actionChestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short champion1ReadyThing = thing_ref(THING_TYPE_JUNK, 1);
    const unsigned short leaderThing = thing_ref(THING_TYPE_JUNK, 2);
    const unsigned short activeReadyThing = thing_ref(THING_TYPE_JUNK, 3);
    const unsigned short deadReadyThing = thing_ref(THING_TYPE_JUNK, 4);
    const unsigned short rejectedLeaderThing = thing_ref(THING_TYPE_JUNK, 5);
    const unsigned short champion3ActionThing = thing_ref(THING_TYPE_JUNK, 6);
    int space = 0;
    int zoneId = 0;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("source-backed container and junk records available",
                      seed_records(&game, actionChestThing));
    ok &= expect_true("open active champion action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("open chest starts active",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);

    game.world.party.champions[1].inventory[CHAMPION_SLOT_HAND_LEFT] =
        champion1ReadyThing;
    ok &= expect_true("set leader hand before status-hand click",
                      M11_GameView_SetV1LeaderHandObject(&game,
                                                         leaderThing));
    {
        int x, y, w, h;
        ok &= expect_true("champion1 ready status-hand zone",
                          M11_GameView_GetV1StatusHandZone(1, 0, &x, &y,
                                                           &w, &h));
        ok &= expect_int("champion1 ready status-hand command",
                         M11_GameView_GetV1MouseCommandForPoint(
                             M11_DM1_MOUSE_LIST_INVENTORY,
                             x + w / 2, y + h / 2,
                             M11_DM1_MOUSE_MASK_LEFT, &space, &zoneId),
                         22);
        ok &= expect_int("champion1 ready status-hand zone id", zoneId, 213);
    }
    ok &= expect_int("champion1 ready click swaps through pointer route",
                     click_status_hand(&game, 1, 0),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("champion1 ready receives leader object",
                     (int)game.world.party.champions[1]
                         .inventory[CHAMPION_SLOT_HAND_LEFT],
                     (int)leaderThing);
    ok &= expect_int("leader hand receives champion1 ready object",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)champion1ReadyThing);
    ok &= expect_int("status-hand click preserves open chest",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);
    ok &= expect_int("inventory owner remains active champion",
                     game.world.party.activeChampionIndex, 0);

    game.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
        activeReadyThing;
    ok &= expect_true("replace leader hand before active-owner reject",
                      M11_GameView_SetV1LeaderHandObject(&game,
                                                         rejectedLeaderThing));
    ok &= expect_int("active inventory champion status hand rejects",
                     click_status_hand(&game, 0, 0),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("active ready hand unchanged after reject",
                     (int)game.world.party.champions[0]
                         .inventory[CHAMPION_SLOT_HAND_LEFT],
                     (int)activeReadyThing);
    ok &= expect_int("leader hand unchanged after active-owner reject",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)rejectedLeaderThing);

    game.world.party.champions[2].hp.current = 0;
    game.world.party.champions[2].inventory[CHAMPION_SLOT_HAND_LEFT] =
        deadReadyThing;
    ok &= expect_int("dead champion status hand rejects",
                     click_status_hand(&game, 2, 0),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("dead champion ready hand unchanged",
                     (int)game.world.party.champions[2]
                         .inventory[CHAMPION_SLOT_HAND_LEFT],
                     (int)deadReadyThing);
    ok &= expect_int("leader hand unchanged after dead reject",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)rejectedLeaderThing);
    ok &= expect_int("open chest still active after rejects",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);

    game.world.party.champions[3].inventory[CHAMPION_SLOT_ACTION_HAND] =
        champion3ActionThing;
    ok &= expect_true("replace leader hand before champion3 action route",
                      M11_GameView_SetV1LeaderHandObject(&game,
                                                         rejectedLeaderThing));
    {
        int x, y, w, h;
        ok &= expect_true("champion3 action status-hand zone",
                          M11_GameView_GetV1StatusHandZone(3, 1, &x, &y,
                                                           &w, &h));
        ok &= expect_int("champion3 action status-hand command",
                         M11_GameView_GetV1MouseCommandForPoint(
                             M11_DM1_MOUSE_LIST_INVENTORY,
                             x + w / 2, y + h / 2,
                             M11_DM1_MOUSE_MASK_LEFT, &space, &zoneId),
                         27);
        ok &= expect_int("champion3 action status-hand zone id", zoneId, 218);
    }
    ok &= expect_int("champion3 action click swaps through pointer route",
                     click_status_hand(&game, 3, 1),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("champion3 action receives leader object",
                     (int)game.world.party.champions[3]
                         .inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)rejectedLeaderThing);
    ok &= expect_int("leader hand receives champion3 action object",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)champion3ActionThing);
    ok &= expect_int("action status-hand click preserves open chest",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);
    ok &= expect_int("action status-hand click keeps inventory owner",
                     game.world.party.activeChampionIndex, 0);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 inventory status-hand runtime probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
