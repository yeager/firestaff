/*
 * DM1 V1 open-chest slot click pickup runtime pixel probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks a visible chest slot through the real
 * pointer/COMMAND route, then redraws into the already-used framebuffer.
 * The first-slot branch is already covered by the original version of this
 * probe. This hardened path fills all eight visible slots and covers both the
 * C544 second-row/rightmost pickup and a C538 middle pickup. The middle pickup
 * must compact the visible runtime chain so downstream item icons shift left
 * into C538/C539 and C544 repaints to a clean empty slot.
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
#include "asset_loader_m11.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

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
    PROBE_SLOT_C544 = 7,
    PROBE_CHEST_SLOT_COUNT = 8
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
        !things->junks || things->junkCount < PROBE_CHEST_SLOT_COUNT) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = items[0];
    things->containers[0].type = 0;

    for (int i = 0; i < PROBE_CHEST_SLOT_COUNT; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)((i % 2) + 1);
        things->junks[i].next =
            (i + 1 < PROBE_CHEST_SLOT_COUNT) ?
            items[i + 1] : THING_ENDOFLIST;
    }

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

static int count_icon_matches(const M11_GameViewState* game,
                              const unsigned char* fb,
                              int iconIndex,
                              int dstX,
                              int dstY,
                              const char* label)
{
    const M11_AssetSlot* iconSheet;
    int graphicIndex = 0;
    int srcX = 0;
    int srcY = 0;
    int srcW = 0;
    int srcH = 0;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;
    char zoneLabel[128];

    snprintf(zoneLabel, sizeof(zoneLabel), "%s icon source zone", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                             &graphicIndex,
                                                             &srcX, &srcY,
                                                             &srcW, &srcH) &&
                      srcW == 16 && srcH == 16);
    iconSheet = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                     (unsigned int)graphicIndex);
    snprintf(zoneLabel, sizeof(zoneLabel), "%s icon sheet", label);
    ok &= expect_true(zoneLabel,
                      iconSheet && iconSheet->loaded && iconSheet->pixels &&
                      srcX + srcW <= (int)iconSheet->width &&
                      srcY + srcH <= (int)iconSheet->height);
    if (!ok || !iconSheet || !iconSheet->pixels) {
        return 0;
    }

    for (yy = 0; yy < srcH; ++yy) {
        int xx;
        for (xx = 0; xx < srcW; ++xx) {
            unsigned char src = (unsigned char)
                (iconSheet->pixels[(srcY + yy) * (int)iconSheet->width +
                                   srcX + xx] & 0x0F);
            unsigned char dst = (unsigned char)
                M11_FB_DECODE_INDEX(fb[(dstY + yy) * PROBE_FB_W +
                                       dstX + xx]);
            if (src != 12) {
                ++total;
                if (src == dst) {
                    ++matched;
                }
            }
        }
    }
    snprintf(zoneLabel, sizeof(zoneLabel), "%s visible icon pixels", label);
    if (!(total > 0 && matched == total)) {
        fprintf(stderr, "FAIL %s matched=%d total=%d icon=%d dst=%d,%d\n",
                zoneLabel, matched, total, iconIndex, dstX, dstY);
        return 0;
    }
    printf("PASS %s matched=%d total=%d icon=%d\n",
           zoneLabel, matched, total, iconIndex);
    return 1;
}

static int check_chest_slot_icon(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int chestOrdinal,
                                 int expectedIcon,
                                 const char* label)
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int vx = 0;
    int vy = 0;
    int vw = 0;
    int vh = 0;
    char zoneLabel[128];
    int ok = 1;

    snprintf(zoneLabel, sizeof(zoneLabel), "%s viewport origin", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    snprintf(zoneLabel, sizeof(zoneLabel), "%s C537+Cn zone", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal,
                                                         &x, &y, &w, &h) &&
                      w == 16 && h == 16);
    if (!ok) return 0;
    return count_icon_matches(game, fb, expectedIcon, vx + x, vy + y, label);
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
    unsigned short items[PROBE_CHEST_SLOT_COUNT];
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int firstSx = 0, firstSy = 0, firstSw = 0, firstSh = 0;
    int secondSx = 0, secondSy = 0, secondSw = 0, secondSh = 0;
    int thirdSx = 0, thirdSy = 0, thirdSw = 0, thirdSh = 0;
    int pickSx = 0, pickSy = 0, pickSw = 0, pickSh = 0;
    int clickX;
    int clickY;
    int ok = 1;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    for (i = 0; i < PROBE_CHEST_SLOT_COUNT; ++i) {
        items[i] = thing_ref(THING_TYPE_JUNK, i);
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
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("source-backed chest and junk records available",
                      seed_records(&game, chestThing, items));
    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C537 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C537,
                          &firstSx, &firstSy, &firstSw, &firstSh));
    ok &= expect_true("C538 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C538,
                          &secondSx, &secondSy, &secondSw, &secondSh));
    ok &= expect_true("C539 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C539,
                          &thirdSx, &thirdSy, &thirdSw, &thirdSh));
    ok &= expect_true("C544 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C544,
                          &pickSx, &pickSy, &pickSw, &pickSh));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 bridge open chest thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);
    ok &= expect_int("leader hand empty before C544 click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)THING_NONE);

    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(beforeFb, reusedFb, sizeof(beforeFb));

    clickX = vx + pickSx + (pickSw / 2);
    clickY = vy + pickSy + (pickSh / 2);
    ok &= expect_true("left click on C544 picks up last visible item",
                      M11_GameView_HandlePointerButton(
                          &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("leader hand holds picked C544 item",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)items[PROBE_SLOT_C544]);
    ok &= expect_int("chest remains open after C544 pickup",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(cleanAfterFb, 0, sizeof(cleanAfterFb));
    M11_GameView_Draw(&game, cleanAfterFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("setup drew a visible icon in C544 before pickup",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + pickSx, vy + pickSy,
                                      pickSw, pickSh) > 8);
    ok &= expect_true("C537 remains stable after C544 pickup",
                      rect_equal(beforeFb, cleanAfterFb,
                                 vx + firstSx, vy + firstSy,
                                 firstSw, firstSh));
    ok &= expect_true("reused framebuffer C544 matches clean empty-slot redraw",
                      rect_equal(reusedFb, cleanAfterFb,
                                 vx + pickSx, vy + pickSy, pickSw, pickSh));

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_true("reset full chest for C538 pickup compaction",
                      seed_records(&game, chestThing, items));
    ok &= expect_true("reopen seeded full action-hand chest for C538 pickup",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("leader hand empty before C538 click",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)THING_NONE);

    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(beforeFb, reusedFb, sizeof(beforeFb));

    clickX = vx + secondSx + (secondSw / 2);
    clickY = vy + secondSy + (secondSh / 2);
    ok &= expect_true("left click on C538 picks up middle visible item",
                      M11_GameView_HandlePointerButton(
                          &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("leader hand holds picked C538 item",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)items[PROBE_SLOT_C538]);
    ok &= expect_int("chest remains open after C538 pickup",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(cleanAfterFb, 0, sizeof(cleanAfterFb));
    M11_GameView_Draw(&game, cleanAfterFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("C537 remains stable after C538 compaction pickup",
                      rect_equal(beforeFb, cleanAfterFb,
                                 vx + firstSx, vy + firstSy,
                                 firstSw, firstSh));
    ok &= expect_true("C538 visibly changes after middle pickup",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + secondSx, vy + secondSy,
                                      secondSw, secondSh) > 8);
    ok &= expect_true("C539 visibly changes after middle pickup",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + thirdSx, vy + thirdSy,
                                      thirdSw, thirdSh) > 8);
    ok &= expect_true("C544 clears after middle pickup compaction",
                      rect_diff_count(beforeFb, cleanAfterFb,
                                      vx + pickSx, vy + pickSy,
                                      pickSw, pickSh) > 8);
    ok &= expect_true("reused framebuffer C544 matches clean empty tail slot",
                      rect_equal(reusedFb, cleanAfterFb,
                                 vx + pickSx, vy + pickSy,
                                 pickSw, pickSh));
    ok &= check_chest_slot_icon(
        &game, cleanAfterFb, PROBE_SLOT_C538,
        M11_GameView_GetObjectIconIndexForThing(&game, items[2]),
        "post-C538-pickup compacted C538 item");
    ok &= check_chest_slot_icon(
        &game, cleanAfterFb, PROBE_SLOT_C539,
        M11_GameView_GetObjectIconIndexForThing(&game, items[3]),
        "post-C538-pickup compacted C539 item");

    printf("sourceEvidence=CHEST.C:F0333:43-76;COMMAND.C:F0359:2174-2176;CHAMPION.C:F0302:688-710;CHEST.C:F0334:117-132\n");
    printf("%s dm1 v1 chest slot-click pickup runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
