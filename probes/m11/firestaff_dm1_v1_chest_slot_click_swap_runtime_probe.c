/*
 * DM1 V1 open-chest occupied slot-click swap runtime pixel probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks occupied C538 while the leader hand holds a
 * different source-backed object, and verifies the real pointer/COMMAND route
 * swaps the objects rather than behaving like a one-sided pickup or drop.
 * The changed C538 pixels must redraw in a reused framebuffer, neighboring
 * C537/C539 slots must remain stable, and close/reopen must preserve the
 * compacted visible chain containing the leader-hand replacement.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-76 opens
 *   G0426, blits C025, and materializes visible contents in C537..C544.
 *   ReDMCSB COMMAND.C F0359 lines 2174-2176 routes inventory commands
 *   C028..C065 to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302 lines 688-710 reads C30+ chest slots from
 *   G0425_aT_ChestSlots, removes the leader-hand object, removes an occupied
 *   slot object to the leader hand, and writes the old leader object into the
 *   selected chest slot.
 *   ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container by scanning
 *   the visible G0425 slots and skipping empty entries, preserving order.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_SLOT_C537 = 0,
    PROBE_SLOT_C538 = 1,
    PROBE_SLOT_C539 = 2,
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
                          unsigned short chestThing)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memcpy(champ->name, "TIGGY   ", 8);
    champ->portraitIndex = 0;
    champ->direction = DIR_NORTH;
    champ->hp.current = 90;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 100;
    champ->mana.current = 40;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
}

static int seed_records(M11_GameViewState* game,
                        unsigned short chestThing,
                        const unsigned short chainItems[PROBE_CHAIN_COUNT],
                        unsigned short leaderItem)
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < 2 ||
        !things->junks || things->junkCount < 2) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = chainItems[0];
    things->containers[0].type = 0;

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    memset(&things->weapons[1], 0, sizeof(things->weapons[1]));
    things->weapons[0].type = 8;  /* DAGGER */
    things->weapons[1].type = 2;  /* TORCH */
    things->weapons[0].next = chainItems[1];
    things->weapons[1].next = chainItems[2];

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    memset(&things->junks[1], 0, sizeof(things->junks[1]));
    things->junks[0].type = 1;  /* container-compatible junk */
    things->junks[1].type = 2;  /* distinct container-compatible junk */
    things->junks[0].next = THING_ENDOFLIST;
    things->junks[1].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0], chestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    return M11_GameView_SetV1LeaderHandObject(game, leaderItem);
}

static unsigned short raw_next_seeded_thing(const M11_GameViewState* game,
                                            unsigned short thing)
{
    int index;

    if (!game || !game->world.things ||
        thing == THING_NONE || thing == THING_ENDOFLIST ||
        (THING_GET_TYPE(thing) != THING_TYPE_WEAPON &&
         THING_GET_TYPE(thing) != THING_TYPE_JUNK)) {
        return THING_NONE;
    }
    index = (int)THING_GET_INDEX(thing);
    if (THING_GET_TYPE(thing) == THING_TYPE_WEAPON) {
        if (!game->world.things->weapons || index < 0 ||
            index >= game->world.things->weaponCount) {
            return THING_NONE;
        }
        return game->world.things->weapons[index].next;
    }
    if (!game->world.things->junks || index < 0 ||
        index >= game->world.things->junkCount) {
        return THING_NONE;
    }
    return game->world.things->junks[index].next;
}

static int read_chest_chain(const M11_GameViewState* game,
                            unsigned short out[PROBE_CHAIN_COUNT])
{
    int count = 0;
    unsigned short thing;

    if (!game || !game->world.things || !game->world.things->containers ||
        game->world.things->containerCount < 1 || !out) {
        return -1;
    }
    thing = game->world.things->containers[0].slot;
    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < PROBE_CHAIN_COUNT) {
        out[count++] = thing;
        thing = raw_next_seeded_thing(game, thing);
    }
    return count;
}

static int rect_diff_count(const unsigned char* a,
                           const unsigned char* b,
                           int x,
                           int y,
                           int w,
                           int h)
{
    int diff = 0;
    int yy;

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (a[(y + yy) * PROBE_FB_W + x + xx] !=
                b[(y + yy) * PROBE_FB_W + x + xx]) {
                ++diff;
            }
        }
    }
    return diff;
}

static int rect_equal(const unsigned char* a,
                      const unsigned char* b,
                      int x,
                      int y,
                      int w,
                      int h)
{
    return rect_diff_count(a, b, x, y, w, h) == 0;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char reusedFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char beforeFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char cleanAfterFb[PROBE_FB_W * PROBE_FB_H];
    unsigned short compacted[PROBE_CHAIN_COUNT];
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short chainItems[PROBE_CHAIN_COUNT] = {
        thing_ref(THING_TYPE_WEAPON, 0),
        thing_ref(THING_TYPE_WEAPON, 1),
        thing_ref(THING_TYPE_JUNK, 0)
    };
    const unsigned short leaderItem = thing_ref(THING_TYPE_JUNK, 1);
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int sx0 = 0, sy0 = 0, sw0 = 0, sh0 = 0;
    int sx1 = 0, sy1 = 0, sw1 = 0, sh1 = 0;
    int sx2 = 0, sy2 = 0, sw2 = 0, sh2 = 0;
    int clickX;
    int clickY;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    memset(compacted, 0, sizeof(compacted));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("source-backed chest, weapon, and junk records available",
                      seed_records(&game, chestThing, chainItems, leaderItem));
    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C537 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C537, &sx0, &sy0, &sw0, &sh0));
    ok &= expect_true("C538 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C538, &sx1, &sy1, &sw1, &sh1));
    ok &= expect_true("C539 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C539, &sx2, &sy2, &sw2, &sh2));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("leader hand holds replacement item before C538 click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)leaderItem);

    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(beforeFb, reusedFb, sizeof(beforeFb));

    clickX = vx + sx1 + (sw1 / 2);
    clickY = vy + sy1 + (sh1 / 2);
    ok &= expect_true("left click on occupied C538 swaps with leader hand",
                      M11_GameView_HandlePointerButton(
                          &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("leader hand now holds old C538 item",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)chainItems[PROBE_SLOT_C538]);
    ok &= expect_int("chest remains open after occupied C538 swap",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(cleanAfterFb, 0, sizeof(cleanAfterFb));
    M11_GameView_Draw(&game, cleanAfterFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("C538 changes from old item to replacement icon",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + sx1, vy + sy1, sw1, sh1) > 8);
    ok &= expect_true("C537 remains stable after C538 swap",
                      rect_equal(beforeFb, cleanAfterFb,
                                 vx + sx0, vy + sy0, sw0, sh0));
    ok &= expect_true("C539 remains stable after C538 swap",
                      rect_equal(beforeFb, cleanAfterFb,
                                 vx + sx2, vy + sy2, sw2, sh2));
    ok &= expect_true("reused framebuffer C538 matches clean swap redraw",
                      rect_equal(reusedFb, cleanAfterFb,
                                 vx + sx1, vy + sy1, sw1, sh1));

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_int("close compacts three visible chest items",
                     read_chest_chain(&game, compacted), PROBE_CHAIN_COUNT);
    ok &= expect_int("compacted chain first item unchanged",
                     (int)compacted[0], (int)chainItems[0]);
    ok &= expect_int("compacted chain second item is replacement",
                     (int)compacted[1], (int)leaderItem);
    ok &= expect_int("compacted chain third item unchanged",
                     (int)compacted[2], (int)chainItems[2]);

    ok &= expect_true("reopen swapped-item chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("reopened chain still has three visible items",
                     read_chest_chain(&game, compacted), PROBE_CHAIN_COUNT);
    ok &= expect_int("reopened second item remains replacement",
                     (int)compacted[1], (int)leaderItem);
    ok &= expect_int("leader hand still holds old C538 item after reopen",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)chainItems[PROBE_SLOT_C538]);

    printf("sourceEvidence=CHEST.C:F0333:43-76;COMMAND.C:F0359:2174-2176;CHAMPION.C:F0302:688-710;CHEST.C:F0334:117-132\n");
    printf("%s dm1 v1 chest slot-click occupied-swap runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
