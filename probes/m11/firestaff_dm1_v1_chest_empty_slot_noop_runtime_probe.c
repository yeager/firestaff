/*
 * DM1 V1 open-chest empty-slot no-op runtime probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest through
 * the M11 V1 bridge, clicks an empty visible chest slot with an empty leader
 * hand through the real pointer route, and proves the click is ignored without
 * mutating G0426, the leader hand, the visible chest chain, or the rendered
 * C025/C539 panel pixels.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-76 opens
 *   G0426, blits C025, then materializes C537..C544 visible slots.
 *   ReDMCSB COMMAND.C F0359 lines 2174-2176 routes C058..C065 panel commands
 *   to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302 lines 688-695 reads the selected C30+ chest slot
 *   from G0425_aT_ChestSlots and returns before screen update when both the
 *   leader hand and selected slot are empty.
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
    PROBE_SLOT_C539 = 2
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
                        unsigned short item0)
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = item0;
    things->containers[0].type = 0;

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    things->weapons[0].type = 8;
    things->weapons[0].next = THING_ENDOFLIST;

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

static int next_is_end(const M11_GameViewState* game, unsigned short thing)
{
    int index;
    if (!game || !game->world.things ||
        THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
        return 0;
    }
    index = (int)THING_GET_INDEX(thing);
    return game->world.things->weapons &&
           index >= 0 &&
           index < game->world.things->weaponCount &&
           game->world.things->weapons[index].next == THING_ENDOFLIST;
}

static int rect_equal(const unsigned char* a,
                      const unsigned char* b,
                      int x,
                      int y,
                      int w,
                      int h)
{
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (a[(y + yy) * PROBE_FB_W + x + xx] !=
                b[(y + yy) * PROBE_FB_W + x + xx]) {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char beforeFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char afterFb[PROBE_FB_W * PROBE_FB_H];
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short item0 = thing_ref(THING_TYPE_WEAPON, 0);
    int vx = 0;
    int vy = 0;
    int vw = 0;
    int vh = 0;
    int sx = 0;
    int sy = 0;
    int sw = 0;
    int sh = 0;
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
    if (!game.assetsAvailable) {
        fprintf(stderr, "FAIL DM1 V1 GRAPHICS.DAT unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("source-backed one-item chest records",
                      seed_records(&game, chestThing, item0));
    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C539 empty chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_SLOT_C539, &sx, &sy, &sw, &sh) &&
                      sw == 16 && sh == 16);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    memset(beforeFb, 0, sizeof(beforeFb));
    M11_GameView_Draw(&game, beforeFb, PROBE_FB_W, PROBE_FB_H);
    centerX = vx + sx + (sw / 2);
    centerY = vy + sy + (sh / 2);

    ok &= expect_thing("leader hand empty before C539 no-op",
                       M11_GameView_GetV1LeaderHandThing(&game), THING_NONE);
    ok &= expect_thing("G0426 names opened chest before C539 no-op",
                       M11_GameView_GetV1OpenChestThing(&game), chestThing);
    ok &= expect_thing("container head remains the C537 item before no-op",
                       game.world.things->containers[0].slot, item0);
    ok &= expect_true("C537 item terminates before no-op",
                      next_is_end(&game, item0));

    ok &= expect_true("empty-hand empty-C539 click is ignored",
                      M11_GameView_HandlePointerButton(
                          &game, centerX, centerY,
                          M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED);

    memset(afterFb, 0, sizeof(afterFb));
    M11_GameView_Draw(&game, afterFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_thing("leader hand remains empty after C539 no-op",
                       M11_GameView_GetV1LeaderHandThing(&game), THING_NONE);
    ok &= expect_thing("G0426 remains open after C539 no-op",
                       M11_GameView_GetV1OpenChestThing(&game), chestThing);
    ok &= expect_thing("container head remains the C537 item after no-op",
                       game.world.things->containers[0].slot, item0);
    ok &= expect_true("C537 item still terminates after no-op",
                      next_is_end(&game, item0));
    ok &= expect_true("C539 panel pixels are unchanged after ignored click",
                      rect_equal(beforeFb, afterFb, vx + sx, vy + sy, sw, sh));

    printf("sourceEvidence=CHEST.C:F0333:43-76;COMMAND.C:F0359:2174-2176;CHAMPION.C:F0302:688-695\n");
    printf("%s dm1 v1 chest empty-slot no-op runtime probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
