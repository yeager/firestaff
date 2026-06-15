/*
 * DM1 V1 champion panel status-box asset-slice runtime probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, seeds deterministic live and dead
 * champion states, renders the party HUD through the real M11 draw stack, and
 * checks stable C151..C154 status-box pixels against the GRAPHICS.DAT-backed
 * status-box assets. It does not claim original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-842 draws live/dead champion status
 *   boxes and gates later live-only redraw lanes.
 *   ReDMCSB CHAMDRAW.C F0292 lines 879-905 draws the live name/bar children
 *   after the 67x29 status-box refresh.
 *   ReDMCSB COORD.C/layout-696 anchors C151..C154 as 67x29 boxes on a 69px
 *   stride, leaving a two-pixel black gutter between adjacent champions.
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
                  "WUUF", DIR_SOUTH, 0, 0, 0);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", DIR_WEST, 25, 17, 9);
}

static int check_live_status_box_slice(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int slot) {
    int x, y, w, h;
    int fillColor = M11_GameView_GetV1StatusBoxFillColor();
    int nameColor = M11_GameView_GetV1StatusNameColor(game, slot);
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d live status base graphic is fill path",
             slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusBoxBaseGraphic(game, slot), 0);
    snprintf(label, sizeof(label), "slot%d live status box zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                      w == 67 && h == 29);
    if (!ok) return 0;

    snprintf(label, sizeof(label), "slot%d live status overwrites stale pixels",
             slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);
    snprintf(label, sizeof(label), "slot%d live status fill remains visible",
             slot);
    ok &= expect_true(label,
                      count_color(fb, PROBE_FB_W, x, y, w, h, fillColor) > 300);
    snprintf(label, sizeof(label), "slot%d live name color present", slot);
    ok &= expect_true(label, nameColor >= 0 &&
                      count_color(fb, PROBE_FB_W, x, y, w, h, nameColor) > 0);
    return ok;
}

static int check_dead_status_box_asset_slice(const M11_GameViewState* game,
                                             const unsigned char* fb,
                                             int slot) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1StatusBoxBaseGraphic(game, slot);
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d dead status base graphic", slot);
    ok &= expect_int(label, gfx, M11_GameView_GetV1DeadStatusBoxGraphicId());
    snprintf(label, sizeof(label), "slot%d dead status box zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                      w == 67 && h == 29);
    snprintf(label, sizeof(label), "slot%d dead status asset loaded", slot);
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

    snprintf(label, sizeof(label),
             "slot%d dead status GRAPHICS.DAT perimeter match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 90);
    snprintf(label, sizeof(label), "slot%d dead status no stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);
    printf("slot%d dead status gfx=%d perimeter=%d/%d\n",
           slot, gfx, matched, expected);
    return ok;
}

static int check_status_box_gutters(const unsigned char* fb) {
    int ok = 1;
    int slot;
    char label[160];

    for (slot = 0; slot < PROBE_CHAMPION_COUNT - 1; ++slot) {
        int x, y, w, h;
        int nextX, nextY, nextW, nextH;
        int gutterX;
        int gutterW;
        snprintf(label, sizeof(label), "slot%d status zone for gutter", slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                          w == 67 && h == 29);
        snprintf(label, sizeof(label), "slot%d next status zone for gutter", slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusBoxZone(slot + 1,
                                                          &nextX, &nextY,
                                                          &nextW, &nextH) &&
                          nextY == y && nextW == 67 && nextH == 29);
        if (!ok) return 0;
        gutterX = x + w;
        gutterW = nextX - gutterX;
        snprintf(label, sizeof(label), "slot%d status gutter width", slot);
        ok &= expect_int(label, gutterW, 2);
        snprintf(label, sizeof(label), "slot%d status gutter black", slot);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     gutterX, y, gutterW, h, 0),
                         gutterW * h);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
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

    seed_party(&game);
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= check_live_status_box_slice(&game, fb, 0);
    ok &= check_live_status_box_slice(&game, fb, 1);
    ok &= check_dead_status_box_asset_slice(&game, fb, 2);
    ok &= check_live_status_box_slice(&game, fb, 3);
    ok &= check_status_box_gutters(fb);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel status-box asset-slice probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
