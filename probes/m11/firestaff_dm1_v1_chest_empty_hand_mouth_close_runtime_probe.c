/*
 * DM1 V1 empty-hand mouth close runtime probe.
 *
 * Firestaff-side runtime evidence: opens an action-hand chest with a hidden
 * ninth tail item, then clicks the empty-hand mouth icon through the real M11
 * pointer route.  ReDMCSB switches to the food/water panel only after closing
 * the current G0426 chest, so the source visible-slot rewrite must compact the
 * chest to C537..C544 and drop the hidden tail.
 *
 * Source evidence:
 *   ReDMCSB PANEL.C F0349 lines 1788-1818 handles empty-hand mouth clicks and
 *   routes to the food/water/poison panel.
 *   ReDMCSB PANEL.C F0345 lines 1554-1556 closes G0426 through F0334 before
 *   drawing the food/water/poison panel.
 *   ReDMCSB CHEST.C F0333 lines 53-76 materializes only C537..C544 into
 *   G0425_aT_ChestSlots.
 *   ReDMCSB CHEST.C F0334 lines 112-132 rewrites the container from non-empty
 *   visible G0425 slots, truncating any ninth-and-later hidden tail.
 *   ReDMCSB COMMAND.C G0449 routes C545 mouth clicks at viewport-relative
 *   x=56..71, y=13..28 to command 70.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_CHAIN_COUNT = 9,
    PROBE_VISIBLE_CHEST_SLOTS = 8,
    PROBE_MOUTH_X = 56 + 8,
    PROBE_MOUTH_Y = 33 + 13 + 8,
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
    champ->food = 1200;
    champ->water = 900;
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

static int seed_records(M11_GameViewState* game,
                        unsigned short actionChestThing,
                        const unsigned short actionItems[PROBE_CHAIN_COUNT])
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < PROBE_CHAIN_COUNT) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = actionItems[0];
    things->containers[0].type = 0;

    for (i = 0; i < PROBE_CHAIN_COUNT; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)((i % 2) + 1);
        things->junks[i].next =
            (i + 1 < PROBE_CHAIN_COUNT) ?
            actionItems[i + 1] : THING_ENDOFLIST;
    }

    seed_champion(&game->world.party.champions[0], actionChestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    M11_GameView_ClearV1LeaderHandObject(game);
    return 1;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const unsigned short actionChestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    unsigned short actionItems[PROBE_CHAIN_COUNT];
    int ok = 1;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    for (i = 0; i < PROBE_CHAIN_COUNT; ++i) {
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

    ok &= expect_true("source-backed container and junk records available",
                      seed_records(&game, actionChestThing, actionItems));
    ok &= expect_int("action chest starts with hidden tail",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_CHAIN_COUNT + 2),
                     PROBE_CHAIN_COUNT);
    ok &= expect_true("open action-hand chest with hidden tail",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 bridge initially names action chest",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);
    ok &= expect_int("action-hand chest icon is open before mouth click",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     145);
    ok &= expect_int("leader hand is empty before mouth click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)THING_NONE);

    ok &= expect_true("empty-hand mouth click routes to food/water panel",
                      M11_GameView_HandlePointer(
                          &game, PROBE_MOUTH_X, PROBE_MOUTH_Y, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("G0426 bridge is closed after mouth click",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)THING_NONE);
    ok &= expect_int("food/water panel active after mouth click",
                     game.v1FoodWaterPanelActive, 1);
    ok &= expect_int("object description panel inactive after mouth click",
                     game.v1ObjectDescriptionPanelActive, 0);
    ok &= expect_int("champion stats panel inactive after mouth click",
                     game.v1ChampionStatsPanelActive, 0);
    ok &= expect_int("action-hand chest icon is closed after mouth click",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_CLOSED_ICON);
    ok &= expect_int("mouth close compacted chest to visible slots",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_CHAIN_COUNT + 2),
                     PROBE_VISIBLE_CHEST_SLOTS);
    ok &= expect_true("ninth hidden tail dropped by mouth close",
                      !chain_contains(&game, game.world.things->containers[0].slot,
                                      actionItems[PROBE_VISIBLE_CHEST_SLOTS],
                                      PROBE_CHAIN_COUNT + 2));

    printf("sourceEvidence=PANEL.C:F0349:1788-1818;PANEL.C:F0345:1554-1556;CHEST.C:F0333:53-76;CHEST.C:F0334:112-132;COMMAND.C:G0449:C545\n");
    printf("%s dm1 v1 empty-hand mouth closes open chest runtime probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
