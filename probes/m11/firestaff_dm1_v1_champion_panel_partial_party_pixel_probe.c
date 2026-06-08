/*
 * DM1 V1 champion panel partial-party runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders a full-party warm frame
 * before each reduced-party case, then renders deterministic 1-, 2-, and
 * 3-champion parties into the same framebuffer through the real M11 V1 draw
 * stack. It checks that occupied party HUD/status-box/champion panel zones
 * are populated and that slots which were visibly populated by the previous
 * full-party frame are actively cleared to black. It does not claim original
 * DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C lines 1134-1138 redraws champion states only while
 *   championIndex < G0305_ui_PartyChampionCount.
 *   ReDMCSB CHAMDRAW.C F0292 lines 893-905 draws status name text and
 *   bottom-anchored HP/stamina/mana bars for present living champions.
 *   ReDMCSB CHAMDRAW.C F0292 lines 937-955 routes ready/action hand slots.
 *   ReDMCSB CHAMDRAW.C lines 1226-1230 limits status hand slot-box refresh
 *   to G0305_ui_PartyChampionCount << 1.
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
    champ->direction = direction;
    champ->portraitIndex = i & 3;
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

static void seed_party(M11_GameViewState* game, int championCount) {
    int slot;
    game->world.party.championCount = championCount;
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
                  "TIGGY", DIR_NORTH, 100, 80, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", DIR_EAST, 65, 44, 20);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", DIR_SOUTH, 25, 20, 10);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", DIR_WEST, 10, 8, 3);
    for (slot = championCount; slot < PROBE_CHAMPION_COUNT; ++slot) {
        memset(&game->world.party.champions[slot], 0,
               sizeof(game->world.party.champions[slot]));
    }
}

static int check_occupied_slot(const M11_GameViewState* game,
                               const unsigned char* fb,
                               int slot,
                               int partyCount) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    char label[128];

    snprintf(label, sizeof(label), "party%d slot%d status box populated",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        w == 67 && h == 29 &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 500);

    snprintf(label, sizeof(label), "party%d slot%d name color present",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusNameTextZone(slot, &x, &y, &w, &h) &&
        count_color(fb, PROBE_FB_W, x, y, w, h,
                    M11_GameView_GetV1StatusNameColor(game, slot)) > 0);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "party%d slot%d stat%d bar populated",
                 partyCount, slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            w == 4 && h == 25 &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == w * h);
    }

    snprintf(label, sizeof(label), "party%d slot%d champion icon populated",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 20);

    {
        int hand;
        for (hand = 0; hand < 2; ++hand) {
            snprintf(label, sizeof(label),
                     "party%d slot%d hand%d slot-box graphic available",
                     partyCount, slot, hand);
            ok &= expect_true(label,
                M11_GameView_GetV1StatusHandSlotGraphic(game, slot, hand) != 0);
            snprintf(label, sizeof(label),
                     "party%d slot%d hand%d slot-box populated",
                     partyCount, slot, hand);
            ok &= expect_true(label,
                M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                        &x, &y, &w, &h) &&
                w == 18 && h == 18 &&
                count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 60);
        }
    }
    return ok;
}

static int check_slot_populated_before_clear(const unsigned char* fb,
                                             int slot,
                                             int partyCount) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label),
             "party%d slot%d pre-clear status box populated",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 500);

    snprintf(label, sizeof(label),
             "party%d slot%d pre-clear champion icon populated",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 20);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label),
                 "party%d slot%d pre-clear stat%d bar populated",
                 partyCount, slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == w * h);
    }

    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label),
                 "party%d slot%d pre-clear hand%d box populated",
                 partyCount, slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) > 60);
    }

    return ok;
}

static int check_empty_slot(const M11_GameViewState* game,
                            const unsigned char* fb,
                            int slot,
                            int partyCount) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label), "party%d slot%d status box empty",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    snprintf(label, sizeof(label), "party%d slot%d name color unavailable",
             partyCount, slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusNameColor(game, slot), -1);

    snprintf(label, sizeof(label), "party%d slot%d champion icon empty",
             partyCount, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "party%d slot%d stat%d bar empty",
                 partyCount, slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label),
                 "party%d slot%d hand%d slot-box graphic unavailable",
                 partyCount, slot, hand);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusHandSlotGraphic(game,
                                                                 slot,
                                                                 hand),
                         0);
        snprintf(label, sizeof(label), "party%d slot%d hand%d zone empty",
                 partyCount, slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    return ok;
}

static int check_party_count(M11_GameViewState* game,
                             unsigned char* fb,
                             int partyCount) {
    int ok = 1;
    int slot;
    seed_party(game, PROBE_CHAMPION_COUNT);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = partyCount; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_slot_populated_before_clear(fb, slot, partyCount);
    }

    seed_party(game, partyCount);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        if (slot < partyCount) {
            ok &= check_occupied_slot(game, fb, slot, partyCount);
        } else {
            ok &= check_empty_slot(game, fb, slot, partyCount);
        }
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int partyCount;
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

    seed_party(&game, PROBE_CHAMPION_COUNT);
    memset(fb, 0x0F, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    for (partyCount = 1; partyCount < PROBE_CHAMPION_COUNT; ++partyCount) {
        ok &= check_party_count(&game, fb, partyCount);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel partial-party pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
