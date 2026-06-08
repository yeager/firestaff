/*
 * DM1 V1 open-chest slot-click drop runtime pixel probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks an empty C539 chest slot while the leader hand
 * holds a source-backed item, then closes/reopens the chest.  The dropped
 * item must leave the leader hand, draw in C539, and persist in the compacted
 * container chain after the ReDMCSB close/reopen cycle.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-76 opens
 *   G0426, blits C025, and materializes visible contents in C537..C544.
 *   ReDMCSB COMMAND.C F0359 lines 2174-2176 routes inventory commands
 *   C028..C065 to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302 lines 688-710 swaps the leader hand with the
 *   selected C30+ G0425_aT_ChestSlots entry.
 *   ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container by scanning
 *   the visible G0425 slots and skipping empty entries, compacting the list.
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
    PROBE_DROP_SLOT_C539 = 2,
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
                        const unsigned short items[PROBE_CHAIN_COUNT])
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < 2 ||
        !things->junks || things->junkCount < 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = items[0];
    things->containers[0].type = 0;

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    memset(&things->weapons[1], 0, sizeof(things->weapons[1]));
    things->weapons[0].type = 8;  /* DAGGER */
    things->weapons[1].type = 2;  /* TORCH */
    things->weapons[0].next = items[1];
    things->weapons[1].next = THING_ENDOFLIST;

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    things->junks[0].type = 1;  /* object-info 128: container-compatible */
    things->junks[0].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0], chestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    return M11_GameView_SetV1LeaderHandObject(game, items[2]);
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
    } else {
        if (!game->world.things->junks || index < 0 ||
            index >= game->world.things->junkCount) {
            return THING_NONE;
        }
        return game->world.things->junks[index].next;
    }
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
    const unsigned short items[PROBE_CHAIN_COUNT] = {
        thing_ref(THING_TYPE_WEAPON, 0),
        thing_ref(THING_TYPE_WEAPON, 1),
        thing_ref(THING_TYPE_JUNK, 0)
    };
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int sx = 0, sy = 0, sw = 0, sh = 0;
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

    ok &= expect_true("source-backed chest and weapon records available",
                      seed_records(&game, chestThing, items));
    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C539 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_DROP_SLOT_C539, &sx, &sy, &sw, &sh));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("leader hand holds drop item before C539 click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)items[2]);

    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(beforeFb, reusedFb, sizeof(beforeFb));

    clickX = vx + sx + (sw / 2);
    clickY = vy + sy + (sh / 2);
    ok &= expect_true("left click on empty C539 drops leader item",
                      M11_GameView_HandlePointerButton(
                          &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("leader hand empty after C539 drop",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)THING_NONE);
    ok &= expect_int("chest remains open after C539 drop",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(cleanAfterFb, 0, sizeof(cleanAfterFb));
    M11_GameView_Draw(&game, cleanAfterFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("C539 changes from empty slot to dropped item icon",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + sx, vy + sy, sw, sh) > 8);
    ok &= expect_true("reused framebuffer C539 matches clean drop redraw",
                      rect_equal(reusedFb, cleanAfterFb,
                                 vx + sx, vy + sy, sw, sh));

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_int("close compacts three visible chest items",
                     read_chest_chain(&game, compacted), PROBE_CHAIN_COUNT);
    ok &= expect_int("compacted chain first item",
                     (int)compacted[0], (int)items[0]);
    ok &= expect_int("compacted chain second item",
                     (int)compacted[1], (int)items[1]);
    ok &= expect_int("compacted chain third dropped item",
                     (int)compacted[2], (int)items[2]);

    ok &= expect_true("reopen dropped-item chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("reopened chain still has three visible items",
                     read_chest_chain(&game, compacted), PROBE_CHAIN_COUNT);
    ok &= expect_int("reopened third item remains dropped item",
                     (int)compacted[2], (int)items[2]);

    printf("sourceEvidence=CHEST.C:F0333:43-76;COMMAND.C:F0359:2174-2176;CHAMPION.C:F0302:688-710;CHEST.C:F0334:117-132\n");
    printf("%s dm1 v1 chest slot-click drop runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
