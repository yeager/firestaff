/*
 * DM1 V1 open-chest slot clear runtime pixel probe.
 *
 * Firestaff-side runtime evidence: renders an open chest with three visible
 * objects, closes/reopens the same chest with only one visible object, then
 * redraws into the already-used framebuffer. The formerly occupied C539 slot
 * must match a clean second render, proving the C025 open-chest panel plus
 * C537..C544 slot boxes repaint empty slots before item icons are drawn.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-48 blits
 *   C025_GRAPHIC_PANEL_OPEN_CHEST, lines 53-76 fills G0425_aT_ChestSlots
 *   and draws every C38..C45 chest slot box, using C0xFFFF_ICON_NONE/-1 for
 *   empty slots.
 *   ReDMCSB CHEST.C F0334_INVENTORY_CloseChest lines 112-132 clears the
 *   open chest and rewrites only the non-empty visible slots.
 *   ReDMCSB PANEL.C F0342 lines 1132-1133 routes chest containers to F0333.
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
    PROBE_CHEST_SLOT_TO_CLEAR = 2
};

static unsigned short thing_ref(int thingType, int thingIndex) {
    return (unsigned short)(((thingType & 0x0F) << 10) | (thingIndex & 0x03FF));
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ) {
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
}

static int seed_records(M11_GameViewState* game,
                        unsigned short chestThing,
                        unsigned short item0,
                        unsigned short item1,
                        unsigned short item2) {
    struct DungeonThings_Compat* things;
    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->weapons || things->weaponCount < 3) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = item0;
    things->containers[0].type = 0;

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    memset(&things->weapons[1], 0, sizeof(things->weapons[1]));
    memset(&things->weapons[2], 0, sizeof(things->weapons[2]));
    things->weapons[0].type = 8;  /* DAGGER */
    things->weapons[1].type = 2;  /* TORCH */
    things->weapons[2].type = 5;  /* SWORD */
    things->weapons[0].next = item1;
    things->weapons[1].next = item2;
    things->weapons[2].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0]);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    return 1;
}

static int rect_diff_count(const unsigned char* a,
                           const unsigned char* b,
                           int x,
                           int y,
                           int w,
                           int h) {
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
                      int h) {
    return rect_diff_count(a, b, x, y, w, h) == 0;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char reusedFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char fullFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char cleanShortFb[PROBE_FB_W * PROBE_FB_H];
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short item0 = thing_ref(THING_TYPE_WEAPON, 0);
    const unsigned short item1 = thing_ref(THING_TYPE_WEAPON, 1);
    const unsigned short item2 = thing_ref(THING_TYPE_WEAPON, 2);
    int vx = 0, vy = 0, vw = 0, vh = 0;
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
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr, "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    ok &= expect_true("source-backed chest and weapon records available",
                      seed_records(&game, chestThing, item0, item1, item2));
    ok &= expect_true("V1 viewport zone available",
                      M11_GameView_GetV1ViewportZone(&vx, &vy, &vw, &vh));
    ok &= expect_true("C539 chest slot zone available",
                      M11_GameView_GetV1ChestSlotBoxZone(
                          PROBE_CHEST_SLOT_TO_CLEAR, &sx, &sy, &sw, &sh));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open seeded three-item action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    memset(reusedFb, 0, sizeof(reusedFb));
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);
    memcpy(fullFb, reusedFb, sizeof(fullFb));

    M11_GameView_CloseV1OpenChest(&game);
    game.world.things->containers[0].slot = item0;
    game.world.things->weapons[0].next = THING_ENDOFLIST;
    ok &= expect_true("reopen same chest with shorter visible chain",
                      M11_GameView_OpenV1ActionHandChest(&game));

    memset(cleanShortFb, 0, sizeof(cleanShortFb));
    M11_GameView_Draw(&game, cleanShortFb, PROBE_FB_W, PROBE_FB_H);
    M11_GameView_Draw(&game, reusedFb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("setup drew a visible icon in C539 before shortening",
                      rect_diff_count(fullFb, cleanShortFb,
                                      vx + sx, vy + sy, sw, sh) > 8);
    ok &= expect_true("reused framebuffer C539 matches clean empty-slot redraw",
                      rect_equal(reusedFb, cleanShortFb,
                                 vx + sx, vy + sy, sw, sh));

    printf("sourceEvidence=CHEST.C:F0333:43-76;CHEST.C:F0334:112-132;PANEL.C:F0342:1132-1133\n");
    printf("%s dm1 v1 chest panel slot-clear runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
