/*
 * DM1 V1 champion panel zero-party repaint runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders a deterministic full-party
 * HUD frame, then renders a zero-champion party into the same framebuffer and
 * proves the party HUD/status-box/champion-panel zones no longer retain stale
 * champion pixels. It does not claim original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0293 lines 1134-1138 iterates only while
 *   championIndex < G0305_ui_PartyChampionCount.
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-905 owns the C151..C154 status-box,
 *   name, and bar refresh for present champions.
 *   ReDMCSB CHAMDRAW.C F0292 lines 937-955 routes ready/action status-hand
 *   slots only through present champion state redraw.
 *   ReDMCSB MENUS.C F0386 draws the C089..C092 action-hand icon cells only
 *   for party champions.
 */
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
    PROBE_CHAMPION_COUNT = 4,
    PROBE_BLACK = 0
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int count_color(const unsigned char* fb,
                       int width,
                       int x,
                       int y,
                       int w,
                       int h,
                       int color) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if ((int)px_index(fb, width, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int count_nonblack(const unsigned char* fb,
                          int width,
                          int x,
                          int y,
                          int w,
                          int h) {
    return w * h - count_color(fb, width, x, y, w, h, PROBE_BLACK);
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction,
                          int hp,
                          int stamina,
                          int mana) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = 100;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = 80;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_full_party(M11_GameViewState* game) {
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 80, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 70, 55, 30);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 45, 35, 20);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 25, 20, 10);
}

static void seed_zero_party(M11_GameViewState* game) {
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = 0;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->actingChampionOrdinal = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
}

static int check_full_party_slot(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int slot) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label), "full slot%d status box populated", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        w == 67 && h == 29 &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 500);

    snprintf(label, sizeof(label), "full slot%d champion icon populated", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 20);

    snprintf(label, sizeof(label), "full slot%d action icon cell populated", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ActionIconCellZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 100);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "full slot%d stat%d bar populated",
                 slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            w == 4 && h == 25 &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == w * h);
    }
    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label), "full slot%d hand%d populated",
                 slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            w == 18 && h == 18 &&
            M11_GameView_GetV1StatusHandSlotGraphic(game, slot, hand) != 0 &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 60);
    }
    return ok;
}

static int check_zero_party_slot(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int slot) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label), "zero slot%d status box cleared", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    snprintf(label, sizeof(label), "zero slot%d champion icon cleared", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    snprintf(label, sizeof(label), "zero slot%d action icon cell cleared", slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ActionIconCellZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    snprintf(label, sizeof(label), "zero slot%d name color unavailable", slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusNameColor(game, slot), -1);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "zero slot%d stat%d bar cleared",
                 slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label), "zero slot%d hand%d graphic unavailable",
                 slot, hand);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusHandSlotGraphic(game,
                                                                 slot,
                                                                 hand),
                         0);
        snprintf(label, sizeof(label), "zero slot%d hand%d cleared",
                 slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int slot;
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

    seed_full_party(&game);
    memset(fb, 0x0F, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_full_party_slot(&game, fb, slot);
    }

    seed_zero_party(&game);
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_zero_party_slot(&game, fb, slot);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel zero-party repaint runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
