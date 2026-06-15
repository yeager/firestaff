/*
 * DM1 V1 open-chest slot boundary runtime probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, then clicks C538 just outside each edge and on the two
 * inclusive corner pixels.  Outside-adjacent clicks must not mutate G0426,
 * the leader hand, or the visible chest chain; edge clicks must route through
 * the real pointer/COMMAND path and pick up the C538 item.
 *
 * Source evidence:
 *   ReDMCSB COMMAND.C lines 215-227 defines G0456 panel-chest click boxes
 *   with inclusive C538 bounds x=106..121, y=109..124 in viewport space.
 *   ReDMCSB COMMAND.C lines 1982 and 2174-2176 dispatch C058..C065 panel
 *   commands to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302 lines 688-710 reads C30+ chest slots from
 *   G0425_aT_ChestSlots and swaps the selected slot with the leader hand.
 *   ReDMCSB CHEST.C F0333 lines 53-76 materializes visible contents into
 *   C537..C544/G0425; F0334 lines 112-132 rewrites only those slots.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_SLOT_C538 = 1,
    PROBE_CHAIN_COUNT = 3
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
                        const unsigned short items[PROBE_CHAIN_COUNT])
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
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

static int expect_open_chain(const M11_GameViewState* game,
                             const unsigned short items[PROBE_CHAIN_COUNT],
                             unsigned short chestThing,
                             const char* label)
{
    char msg[128];
    int ok = 1;

    snprintf(msg, sizeof(msg), "%s G0426 names opened chest", label);
    ok &= expect_thing(msg, M11_GameView_GetV1OpenChestThing(game),
                       chestThing);
    snprintf(msg, sizeof(msg), "%s container head remains C537", label);
    ok &= expect_thing(msg, game->world.things->containers[0].slot,
                       items[0]);
    snprintf(msg, sizeof(msg), "%s C537 links to C538", label);
    ok &= expect_thing(msg, next_thing(game, items[0]), items[1]);
    snprintf(msg, sizeof(msg), "%s C538 links to C539", label);
    ok &= expect_thing(msg, next_thing(game, items[1]), items[2]);
    snprintf(msg, sizeof(msg), "%s C539 terminates", label);
    ok &= expect_thing(msg, next_thing(game, items[2]), THING_ENDOFLIST);
    snprintf(msg, sizeof(msg), "%s leader hand remains empty", label);
    ok &= expect_thing(msg, M11_GameView_GetV1LeaderHandThing(game),
                       THING_NONE);
    return ok;
}

static int run_corner_case(M11_GameViewState* game,
                           unsigned short chestThing,
                           const unsigned short items[PROBE_CHAIN_COUNT],
                           int clickX,
                           int clickY,
                           const char* label)
{
    char msg[128];
    int ok = 1;

    ok &= expect_true("reset seeded chest records", seed_records(game,
                                                                 chestThing,
                                                                 items));
    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(game));
    ok &= expect_open_chain(game, items, chestThing, label);
    snprintf(msg, sizeof(msg), "%s inclusive C538 corner picks item", label);
    ok &= expect_true(msg,
                      M11_GameView_HandlePointerButton(
                          game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    snprintf(msg, sizeof(msg), "%s leader hand holds C538", label);
    ok &= expect_thing(msg, M11_GameView_GetV1LeaderHandThing(game),
                       items[PROBE_SLOT_C538]);
    snprintf(msg, sizeof(msg), "%s C537 now links to C539", label);
    ok &= expect_thing(msg, next_thing(game, items[0]), items[2]);
    snprintf(msg, sizeof(msg), "%s picked C538 isolated", label);
    ok &= expect_thing(msg, next_thing(game, items[1]), THING_ENDOFLIST);
    return ok;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short items[PROBE_CHAIN_COUNT] = {
        thing_ref(THING_TYPE_WEAPON, 0),
        thing_ref(THING_TYPE_POTION, 0),
        thing_ref(THING_TYPE_JUNK, 0)
    };
    int vx = 0;
    int vy = 0;
    int vw = 0;
    int vh = 0;
    int sx = 0;
    int sy = 0;
    int sw = 0;
    int sh = 0;
    int left;
    int right;
    int top;
    int bottom;
    int centerX;
    int centerY;
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

    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C538 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C538, &sx, &sy, &sw, &sh) &&
                      sw == 16 && sh == 16);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    left = vx + sx;
    right = vx + sx + sw - 1;
    top = vy + sy;
    bottom = vy + sy + sh - 1;
    centerX = left + (sw / 2);
    centerY = top + (sh / 2);

    ok &= expect_true("source-backed container/weapon/potion/junk records",
                      seed_records(&game, chestThing, items));
    ok &= expect_true("open action-hand chest for outside-boundary checks",
                      M11_GameView_OpenV1ActionHandChest(&game));

    ok &= expect_true("click just left of C538 is ignored",
                      M11_GameView_HandlePointerButton(
                          &game, left - 1, centerY,
                          M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED);
    ok &= expect_open_chain(&game, items, chestThing, "after left miss");

    ok &= expect_true("click just right of C538 is ignored",
                      M11_GameView_HandlePointerButton(
                          &game, right + 1, centerY,
                          M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED);
    ok &= expect_open_chain(&game, items, chestThing, "after right miss");

    ok &= expect_true("click just above C538 is ignored",
                      M11_GameView_HandlePointerButton(
                          &game, centerX, top - 1,
                          M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED);
    ok &= expect_open_chain(&game, items, chestThing, "after top miss");

    ok &= expect_true("click just below C538 is ignored",
                      M11_GameView_HandlePointerButton(
                          &game, centerX, bottom + 1,
                          M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED);
    ok &= expect_open_chain(&game, items, chestThing, "after bottom miss");

    ok &= run_corner_case(&game, chestThing, items, left, top,
                          "top-left edge");
    ok &= run_corner_case(&game, chestThing, items, right, bottom,
                          "bottom-right edge");

    printf("sourceEvidence=COMMAND.C:215-227,1982,2174-2176;CHAMPION.C:F0302:688-710;CHEST.C:F0333:53-76;CHEST.C:F0334:112-132\n");
    printf("%s dm1 v1 chest slot boundary runtime probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
