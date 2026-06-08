/*
 * DM1 V1 open-chest slot click pickup runtime pixel probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks the first visible C537 chest slot through the
 * real pointer/COMMAND route, then redraws into the already-used framebuffer.
 * The picked slot must repaint to the clean empty-slot state and the leader
 * hand must hold the picked item.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-76 opens
 *   G0426, blits C025, then materializes visible contents in C537..C544.
 *   ReDMCSB COMMAND.C F0359 lines 2174-2176 routes inventory commands
 *   C028..C065 to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302 lines 688-710 reads C30+ chest slots from
 *   G0425_aT_ChestSlots, removes an occupied slot, and puts it in the leader
 *   hand.  CHEST.C F0334 lines 117-132 later compacts only non-empty visible
 *   slots; Firestaff's bridge rewrites the same visible list after the click.
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
    PROBE_CHEST_SLOT_COUNT = 2
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
                        const unsigned short items[PROBE_CHEST_SLOT_COUNT])
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < PROBE_CHEST_SLOT_COUNT) {
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
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short items[PROBE_CHEST_SLOT_COUNT] = {
        thing_ref(THING_TYPE_WEAPON, 0),
        thing_ref(THING_TYPE_WEAPON, 1)
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
    ok &= expect_true("C537 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C537, &sx, &sy, &sw, &sh));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 bridge open chest thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);
    ok &= expect_int("leader hand empty before C537 click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)THING_NONE);

    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(beforeFb, reusedFb, sizeof(beforeFb));

    clickX = vx + sx + (sw / 2);
    clickY = vy + sy + (sh / 2);
    ok &= expect_true("left click on C537 picks up visible item",
                      M11_GameView_HandlePointerButton(
                          &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("leader hand holds picked C537 item",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)items[0]);
    ok &= expect_int("chest remains open after C537 pickup",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(cleanAfterFb, 0, sizeof(cleanAfterFb));
    M11_GameView_Draw(&game, cleanAfterFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("setup drew a visible icon in C537 before pickup",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + sx, vy + sy, sw, sh) > 8);
    ok &= expect_true("reused framebuffer C537 matches clean empty-slot redraw",
                      rect_equal(reusedFb, cleanAfterFb,
                                 vx + sx, vy + sy, sw, sh));

    printf("sourceEvidence=CHEST.C:F0333:43-76;COMMAND.C:F0359:2174-2176;CHAMPION.C:F0302:688-710;CHEST.C:F0334:117-132\n");
    printf("%s dm1 v1 chest slot-click pickup runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
