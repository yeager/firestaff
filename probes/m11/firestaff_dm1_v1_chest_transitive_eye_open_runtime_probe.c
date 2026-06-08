/*
 * DM1 V1 transitive open-chest pressing-eye runtime probe.
 *
 * Firestaff-side runtime evidence: opens an action-hand chest with a ninth
 * hidden tail item, then opens a different leader-hand chest through the eye
 * command.  ReDMCSB closes the previous G0426 chest before opening the new
 * one, so the first chest must be rewritten from only its visible C537..C544
 * slots and the hidden tail must be dropped.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 30-38 returns
 *   for the same G0426 chest and closes a different already-open chest before
 *   opening the requested container.
 *   ReDMCSB CHEST.C F0333 lines 53-76 materializes only C537..C544 into
 *   G0425_aT_ChestSlots.
 *   ReDMCSB CHEST.C F0334 lines 112-132 rewrites the container from non-empty
 *   visible G0425 slots, truncating any ninth-and-later hidden tail.
 *   ReDMCSB PANEL.C F0352 lines 2123-2159 routes eye-with-leader-hand-object
 *   to F0342, and PANEL.C F0342 lines 1132-1133 routes containers to F0333.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_ACTION_CHAIN_COUNT = 9,
    PROBE_VISIBLE_CHEST_SLOTS = 8,
    PROBE_LEADER_CHAIN_COUNT = 1,
    PROBE_EYE_X = 12 + 8,
    PROBE_EYE_Y = 33 + 13 + 8,
    PROBE_CHEST_CLOSED_ICON = 144
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
                          unsigned short actionChestThing)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memcpy(champ->name, "TIGGY   ", 8);
    champ->portraitIndex = 0;
    champ->direction = DIR_NORTH;
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 80;
    champ->mana.current = 60;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = actionChestThing;
}

static unsigned short next_seeded_thing(const M11_GameViewState* game,
                                        unsigned short thing)
{
    int index;

    if (!game || !game->world.things ||
        thing == THING_NONE || thing == THING_ENDOFLIST ||
        THING_GET_TYPE(thing) != THING_TYPE_JUNK) {
        return THING_ENDOFLIST;
    }
    index = (int)THING_GET_INDEX(thing);
    if (!game->world.things->junks || index < 0 ||
        index >= game->world.things->junkCount) {
        return THING_ENDOFLIST;
    }
    return game->world.things->junks[index].next;
}

static int chain_contains(const M11_GameViewState* game,
                          unsigned short first,
                          unsigned short target,
                          int maxWalk)
{
    int count = 0;
    unsigned short thing = first;

    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < maxWalk) {
        if (thing == target) return 1;
        thing = next_seeded_thing(game, thing);
        ++count;
    }
    return 0;
}

static int chain_count(const M11_GameViewState* game,
                       unsigned short first,
                       int maxWalk)
{
    int count = 0;
    unsigned short thing = first;

    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < maxWalk) {
        ++count;
        thing = next_seeded_thing(game, thing);
    }
    return count;
}

static int seed_records(M11_GameViewState* game,
                        unsigned short actionChestThing,
                        unsigned short leaderChestThing,
                        const unsigned short actionItems[PROBE_ACTION_CHAIN_COUNT],
                        unsigned short leaderItem)
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 2 ||
        !things->junks || things->junkCount < PROBE_ACTION_CHAIN_COUNT + 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = actionItems[0];
    things->containers[0].type = 0;

    memset(&things->containers[1], 0, sizeof(things->containers[1]));
    things->containers[1].next = THING_ENDOFLIST;
    things->containers[1].slot = leaderItem;
    things->containers[1].type = 0;

    for (i = 0; i < PROBE_ACTION_CHAIN_COUNT; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)((i % 2) + 1);
        things->junks[i].next =
            (i + 1 < PROBE_ACTION_CHAIN_COUNT) ?
            actionItems[i + 1] : THING_ENDOFLIST;
    }
    memset(&things->junks[PROBE_ACTION_CHAIN_COUNT], 0,
           sizeof(things->junks[PROBE_ACTION_CHAIN_COUNT]));
    things->junks[PROBE_ACTION_CHAIN_COUNT].type = 1;
    things->junks[PROBE_ACTION_CHAIN_COUNT].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0], actionChestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    return M11_GameView_SetV1LeaderHandObject(game, leaderChestThing);
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const unsigned short actionChestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short leaderChestThing = thing_ref(THING_TYPE_CONTAINER, 1);
    const unsigned short leaderItem =
        thing_ref(THING_TYPE_JUNK, PROBE_ACTION_CHAIN_COUNT);
    unsigned short actionItems[PROBE_ACTION_CHAIN_COUNT];
    int ok = 1;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    for (i = 0; i < PROBE_ACTION_CHAIN_COUNT; ++i) {
        actionItems[i] = thing_ref(THING_TYPE_JUNK, i);
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("source-backed containers and junk records available",
                      seed_records(&game, actionChestThing, leaderChestThing,
                                   actionItems, leaderItem));
    ok &= expect_int("action chest starts with hidden tail",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_ACTION_CHAIN_COUNT + 2),
                     PROBE_ACTION_CHAIN_COUNT);
    ok &= expect_true("open action-hand chest with hidden tail",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 bridge initially names action chest",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);

    ok &= expect_true("eye click opens different leader-hand chest",
                      M11_GameView_HandlePointer(
                          &game, PROBE_EYE_X, PROBE_EYE_Y, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("G0426 bridge names leader-hand chest after eye open",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)leaderChestThing);
    ok &= expect_int("leader hand still holds inspected chest",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)leaderChestThing);
    ok &= expect_int("action-hand chest icon is closed during eye-open chest",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_CLOSED_ICON);
    ok &= expect_int("previous action chest was compacted to visible slots",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_ACTION_CHAIN_COUNT + 2),
                     PROBE_VISIBLE_CHEST_SLOTS);
    ok &= expect_true("ninth hidden tail dropped from previous action chest",
                      !chain_contains(&game, game.world.things->containers[0].slot,
                                      actionItems[PROBE_VISIBLE_CHEST_SLOTS],
                                      PROBE_ACTION_CHAIN_COUNT + 2));
    ok &= expect_int("leader-hand chest contents preserved",
                     chain_count(&game, game.world.things->containers[1].slot,
                                 PROBE_LEADER_CHAIN_COUNT + 1),
                     PROBE_LEADER_CHAIN_COUNT);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 chest transitive pressing-eye open runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
