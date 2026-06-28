/*
 * DM1 V1 champion panel live/dead status-box matrix runtime probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders deterministic all-live and
 * one-dead-at-a-time party states through the real M11 V1 draw stack, and
 * checks that every C151..C154 status box switches between the live fill path
 * and the C008 GRAPHICS.DAT dead-box asset without leaving stale pixels or
 * drawing live HP/stamina/mana bars over dead champions. It does not claim
 * original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-842 draws live status boxes by
 *   filling C151..C154 with C12, but blits C008 when CurrentHealth is zero.
 *   ReDMCSB CHAMDRAW.C F0292 lines 879-905 reaches the live name/stat/bar
 *   children only after the dead branch jumps to T0292042.
 *   ReDMCSB CHAMDRAW.C F0287 lines 72-155 draws bottom-anchored live
 *   HP/stamina/mana bars, which must not overdraw C008 dead status boxes.
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
    PROBE_STALE_PIXEL = M11_FB_ENCODE(0, 15)
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

static int count_raw_pixel(const unsigned char* fb,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char rawPixel) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] == rawPixel) {
                ++count;
            }
        }
    }
    return count;
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
    champ->portraitIndex = direction & 3;
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

static void seed_party(M11_GameViewState* game) {
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", DIR_NORTH, 100, 80, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", DIR_EAST, 75, 58, 30);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", DIR_SOUTH, 50, 39, 12);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", DIR_WEST, 25, 17, 9);
}

static void set_dead_slot(M11_GameViewState* game, int deadSlot) {
    int slot;
    static const unsigned short hp[PROBE_CHAMPION_COUNT] = { 100, 75, 50, 25 };
    static const unsigned short stamina[PROBE_CHAMPION_COUNT] = { 80, 58, 39, 17 };
    static const unsigned short mana[PROBE_CHAMPION_COUNT] = { 60, 30, 12, 9 };

    game->world.party.activeChampionIndex = deadSlot == 0 ? 1 : 0;
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        struct ChampionState_Compat* champ = &game->world.party.champions[slot];
        champ->hp.current = slot == deadSlot ? 0u : hp[slot];
        champ->stamina.current = slot == deadSlot ? 0u : stamina[slot];
        champ->mana.current = slot == deadSlot ? 0u : mana[slot];
    }
}

static int check_live_status_box(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int slot) {
    int x, y, w, h;
    int fillColor = M11_GameView_GetV1StatusBoxFillColor();
    int nameColor = M11_GameView_GetV1StatusNameColor(game, slot);
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d live status base graphic", slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusBoxBaseGraphic(game, slot), 0);
    snprintf(label, sizeof(label), "slot%d live status zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                      w == 67 && h == 29);
    if (!ok) return 0;

    snprintf(label, sizeof(label), "slot%d live status no stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);
    snprintf(label, sizeof(label), "slot%d live fill remains visible", slot);
    ok &= expect_true(label,
                      count_color(fb, PROBE_FB_W, x, y, w, h, fillColor) > 300);
    snprintf(label, sizeof(label), "slot%d live name color present", slot);
    ok &= expect_true(label, nameColor >= 0 &&
                      count_color(fb, PROBE_FB_W, x, y, w, h, nameColor) > 0);
    return ok;
}

static int check_dead_status_box(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int slot) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1StatusBoxBaseGraphic(game, slot);
    const M11_AssetSlot* asset;
    int expected = 0;
    int matched = 0;
    int yy;
    int stat;
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d dead status base graphic", slot);
    ok &= expect_int(label, gfx, M11_GameView_GetV1DeadStatusBoxGraphicId());
    snprintf(label, sizeof(label), "slot%d dead status zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                      w == 67 && h == 29);
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)gfx);
    snprintf(label, sizeof(label), "slot%d dead status C008 asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 67 && asset->height == 29);
    if (!ok || !asset || !asset->pixels) return 0;

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (xx > 0 && xx < w - 1 && yy > 0 && yy < h - 1) {
                continue;
            }
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) {
                continue;
            }
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) {
                ++matched;
            }
        }
    }

    snprintf(label, sizeof(label), "slot%d dead status C008 perimeter", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 85);
    snprintf(label, sizeof(label), "slot%d dead status no stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);

    for (stat = 0; stat < 3; ++stat) {
        int bx, by, bw, bh;
        int fillColor = M11_GameView_GetV1ChampionBarColor(slot);
        snprintf(label, sizeof(label), "slot%d dead stat%d bar zone", slot, stat);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusBarZone(slot, stat,
                                                          &bx, &by, &bw, &bh) &&
                          bw == 4 && bh == 25);
        snprintf(label, sizeof(label), "slot%d dead stat%d no live bar overdraw",
                 slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, bx, by, bw, bh, fillColor),
                         0);
    }
    printf("slot%d dead status gfx=%d perimeter=%d/%d\n",
           slot, gfx, matched, expected);
    return ok;
}

static int render_and_check_matrix(M11_GameViewState* game,
                                   unsigned char* fb,
                                   int deadSlot) {
    int slot;
    int ok = 1;

    set_dead_slot(game, deadSlot);
    memset(fb, PROBE_STALE_PIXEL, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        if (slot == deadSlot) {
            ok &= check_dead_status_box(game, fb, slot);
        } else {
            ok &= check_live_status_box(game, fb, slot);
        }
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int deadSlot;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP could not open selected DM1 V1 game view from %s\n",
               dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    if (!game.assetsAvailable) {
        printf("SKIP DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
               dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    seed_party(&game);
    ok &= render_and_check_matrix(&game, fb, -1);
    for (deadSlot = 0; deadSlot < PROBE_CHAMPION_COUNT; ++deadSlot) {
        ok &= render_and_check_matrix(&game, fb, deadSlot);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel status-box live/dead matrix probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
