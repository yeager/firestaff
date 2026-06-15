/*
 * DM1 V1 sparse open-chest close-rewire runtime probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks C538 through the real pointer route to lift the
 * middle visible item into the leader hand, closes G0426, then reopens the
 * same chest.  The underlying DungeonThings_Compat container list must be
 * compacted from the remaining visible C537/C539 slots in visual order.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 53-76 copies at
 *   most the first eight linked contents into G0425_aT_ChestSlots.
 *   ReDMCSB CHAMPION.C F0302 lines 688-710 routes C30+ chest slot clicks
 *   through G0425 and swaps the selected slot with G4055 leader hand.
 *   ReDMCSB CHEST.C F0334_INVENTORY_CloseChest lines 112-132 clears G0426,
 *   skips empty G0425 slots, and relinks remaining visible contents.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_CHEST_SLOT_TO_PICK_UP = 1,
    PROBE_ITEM_COUNT = 3
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

static int expect_thing(const char* label,
                        unsigned short got,
                        unsigned short want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=0x%04x want=0x%04x\n",
                label, (unsigned int)got, (unsigned int)want);
        return 0;
    }
    printf("PASS %s thing=0x%04x\n", label, (unsigned int)got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          unsigned short chestThing)
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
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
}

static unsigned short next_thing(const M11_GameViewState* game,
                                 unsigned short thing)
{
    int index;

    if (!game || !game->world.things ||
        thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    index = (int)THING_GET_INDEX(thing);
    switch (THING_GET_TYPE(thing)) {
        case THING_TYPE_WEAPON:
            if (game->world.things->weapons &&
                index >= 0 && index < game->world.things->weaponCount) {
                return game->world.things->weapons[index].next;
            }
            break;
        case THING_TYPE_POTION:
            if (game->world.things->potions &&
                index >= 0 && index < game->world.things->potionCount) {
                return game->world.things->potions[index].next;
            }
            break;
        case THING_TYPE_JUNK:
            if (game->world.things->junks &&
                index >= 0 && index < game->world.things->junkCount) {
                return game->world.things->junks[index].next;
            }
            break;
        default:
            break;
    }
    return THING_ENDOFLIST;
}

static int seed_records(M11_GameViewState* game,
                        unsigned short chestThing,
                        const unsigned short items[PROBE_ITEM_COUNT])
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    (void)chestThing;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < 1 ||
        !things->potions || things->potionCount < 1 ||
        !things->junks || things->junkCount < 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = items[0];
    things->containers[0].type = 0;

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    things->weapons[0].type = 8;
    things->weapons[0].next = items[1];

    memset(&things->potions[0], 0, sizeof(things->potions[0]));
    things->potions[0].type = 5;
    things->potions[0].next = items[2];

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    things->junks[0].type = 1;
    things->junks[0].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0], chestThing);
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
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short items[PROBE_ITEM_COUNT] = {
        thing_ref(THING_TYPE_WEAPON, 0),
        thing_ref(THING_TYPE_POTION, 0),
        thing_ref(THING_TYPE_JUNK, 0)
    };
    int sx = 0, sy = 0, sw = 0, sh = 0;
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

    ok &= expect_true("source-backed container/weapon/potion/junk records",
                      seed_records(&game, chestThing, items));
    ok &= expect_thing("initial container head",
                       game.world.things->containers[0].slot, items[0]);
    ok &= expect_thing("initial first next",
                       next_thing(&game, items[0]), items[1]);
    ok &= expect_thing("initial second next",
                       next_thing(&game, items[1]), items[2]);
    ok &= expect_true("open action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_true("C538 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_CHEST_SLOT_TO_PICK_UP, &sx, &sy, &sw, &sh) &&
                      sw > 0 && sh > 0);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("click C538 picks up middle chest item",
                      M11_GameView_HandlePointer(
                          &game, sx + (sw / 2), 33 + sy + (sh / 2), 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_thing("leader hand holds picked middle item",
                       M11_GameView_GetV1LeaderHandThing(&game), items[1]);
    ok &= expect_thing("runtime chain already skips emptied C538",
                       next_thing(&game, items[0]), items[2]);
    ok &= expect_thing("picked middle item is isolated",
                       next_thing(&game, items[1]), THING_ENDOFLIST);

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_thing("close clears G0426 bridge",
                       M11_GameView_GetV1OpenChestThing(&game), THING_NONE);
    ok &= expect_thing("closed sparse head remains C537 item",
                       game.world.things->containers[0].slot, items[0]);
    ok &= expect_thing("closed sparse list relinks C537 to C539",
                       next_thing(&game, items[0]), items[2]);
    ok &= expect_thing("closed sparse tail terminates",
                       next_thing(&game, items[2]), THING_ENDOFLIST);
    ok &= expect_thing("closed sparse list excludes picked C538",
                       next_thing(&game, items[1]), THING_ENDOFLIST);

    ok &= expect_true("reopen compacted sparse chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_thing("reopened compacted head is original C537",
                       game.world.things->containers[0].slot, items[0]);
    ok &= expect_thing("reopened compacted second visible is original C539",
                       next_thing(&game, items[0]), items[2]);
    ok &= expect_thing("leader hand still holds picked C538",
                       M11_GameView_GetV1LeaderHandThing(&game), items[1]);

    printf("sourceEvidence=CHEST.C:F0333:53-76;CHAMPION.C:F0302:688-710;CHEST.C:F0334:112-132\n");
    printf("%s dm1 v1 chest sparse close-rewire runtime probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
