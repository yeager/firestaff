/*
 * DM1 V1 champion panel repaint-delta runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders a deterministic party,
 * mutates one champion's HUD-visible state, then redraws into the same
 * framebuffer. The gate proves the party HUD/status-box/champion panel
 * pixels reflect the second state instead of retaining stale first-frame
 * bar, hand-slot, or champion-icon pixels. It does not claim original DOS
 * screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-905 owns live C151..C154 status-box,
 *   name, bar, wound, and action-hand redraws.
 *   ReDMCSB CHAMDRAW.C F0287 lines 72-156 draws bottom-anchored 4x25
 *   HP/stamina/mana bars using C12 blank pixels above the colored fill.
 *   ReDMCSB CHAMDRAW.C F0291 lines 632-651 draws C033/C034/C035 hand-slot
 *   boxes, with C035 overriding the wounded C01 action-hand route.
 *   ReDMCSB CHAMDRAW.C F0622 prepares the direction-relative C028 champion
 *   icon bitmap used by the top-right party icon panel.
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
    PROBE_CHAMPION_COUNT = 4
};

typedef struct PixelRectStats {
    int nonBlack;
    int checksum;
} PixelRectStats;

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

static PixelRectStats rect_stats(const unsigned char* fb,
                                 int width,
                                 int x,
                                 int y,
                                 int w,
                                 int h) {
    PixelRectStats stats;
    int yy;
    stats.nonBlack = 0;
    stats.checksum = 0;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            int px = (int)px_index(fb, width, x + xx, y + yy);
            if (px != 0) {
                ++stats.nonBlack;
            }
            stats.checksum = (stats.checksum * 33) ^ (px + xx * 17 + yy * 31);
        }
    }
    return stats;
}

static int expected_fill_height(int current, int maximum, int fullHeight) {
    long scaled;
    if (current <= 0 || maximum <= 0 || fullHeight <= 0) {
        return 0;
    }
    scaled = (long)fullHeight * (long)current / (long)maximum;
    if (scaled < 1) {
        scaled = 1;
    }
    if (scaled > fullHeight) {
        scaled = fullHeight;
    }
    return (int)scaled;
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

static void seed_party(M11_GameViewState* game) {
    int slot;
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
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        game->championDamageTimer[slot] = 0;
    }
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 80, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 70, 50, 40);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 50, 40, 30);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 30, 20, 10);
}

static void mutate_slot0_for_repaint(M11_GameViewState* game) {
    struct ChampionState_Compat* champ = &game->world.party.champions[0];
    champ->direction = DIR_EAST;
    champ->hp.current = 25;
    champ->stamina.current = 10;
    champ->mana.current = 0;
    champ->wounds = 0x0003u;
    game->actingChampionOrdinal = 1;
}

static int check_bar_after_repaint(const M11_GameViewState* game,
                                   const unsigned char* fb,
                                   int stat) {
    const struct ChampionState_Compat* champ = &game->world.party.champions[0];
    int current[3];
    int maximum[3];
    int x, y, w, h;
    int fillHeight;
    int blankHeight;
    int fillColor = M11_GameView_GetV1ChampionBarColor(0);
    int blankColor = M11_GameView_GetV1StatusBarBlankColor();
    int ok = 1;
    char label[128];

    current[0] = champ->hp.current;
    current[1] = champ->stamina.current;
    current[2] = champ->mana.current;
    maximum[0] = champ->hp.maximum;
    maximum[1] = champ->stamina.maximum;
    maximum[2] = champ->mana.maximum;

    snprintf(label, sizeof(label), "repaint stat%d bar zone", stat);
    ok &= expect_true(label, M11_GameView_GetV1StatusBarZone(0, stat,
                                                             &x, &y, &w, &h) &&
                             w == 4 && h == 25);
    if (!ok) {
        return 0;
    }

    fillHeight = expected_fill_height(current[stat], maximum[stat], h);
    blankHeight = h - fillHeight;
    if (blankHeight > 0) {
        snprintf(label, sizeof(label), "repaint stat%d blank top", stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, x, y, w, blankHeight,
                                     blankColor),
                         w * blankHeight);
    }
    if (fillHeight > 0) {
        snprintf(label, sizeof(label), "repaint stat%d colored bottom", stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y + blankHeight, w, fillHeight,
                                     fillColor),
                         w * fillHeight);
    } else {
        snprintf(label, sizeof(label), "repaint stat%d no stale fill", stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, x, y, w, h, fillColor),
                         0);
    }
    return ok;
}

static int check_slot_box_perimeter(const M11_GameViewState* game,
                                    const unsigned char* fb,
                                    int hand,
                                    int expectedGfx) {
    int gfx = M11_GameView_GetV1StatusHandSlotGraphic(game, 0, hand);
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int x, y, w, h;
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "repaint hand%d graphic id", hand);
    ok &= expect_int(label, gfx, expectedGfx);
    snprintf(label, sizeof(label), "repaint hand%d box zone", hand);
    ok &= expect_true(label, M11_GameView_GetV1StatusHandSlotBoxZone(
                                  0, hand, &x, &y, &w, &h) &&
                             w == 18 && h == 18);
    snprintf(label, sizeof(label), "repaint hand%d box asset", hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 18 && asset->height == 18);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (xx > 0 && xx < 17 && yy > 0 && yy < 17) {
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
    snprintf(label, sizeof(label), "repaint hand%d perimeter pixels", hand);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 85);
    printf("hand%d repaint gfx=%d perimeter=%d/%d\n",
           hand, gfx, matched, expected);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PixelRectStats iconBefore;
    PixelRectStats iconAfter;
    PixelRectStats handBefore[2];
    PixelRectStats handAfter[2];
    int iconX, iconY, iconW, iconH;
    int handX[2], handY[2], handW[2], handH[2];
    int firstIconSource;
    int secondIconSource;
    int stat;
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
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("slot0 champion icon zone",
                      M11_GameView_GetV1ChampionIconZone(0, &iconX, &iconY,
                                                         &iconW, &iconH) &&
                      iconW == 19 && iconH == 14);
    ok &= expect_true("slot0 ready hand zone",
                      M11_GameView_GetV1StatusHandSlotBoxZone(0, 0,
                                                              &handX[0],
                                                              &handY[0],
                                                              &handW[0],
                                                              &handH[0]) &&
                      handW[0] == 18 && handH[0] == 18);
    ok &= expect_true("slot0 action hand zone",
                      M11_GameView_GetV1StatusHandSlotBoxZone(0, 1,
                                                              &handX[1],
                                                              &handY[1],
                                                              &handW[1],
                                                              &handH[1]) &&
                      handW[1] == 18 && handH[1] == 18);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    firstIconSource = M11_GameView_GetV1ChampionIconSourceIndex(&game, 0);
    iconBefore = rect_stats(fb, PROBE_FB_W, iconX, iconY, iconW, iconH);
    handBefore[0] = rect_stats(fb, PROBE_FB_W,
                               handX[0], handY[0], handW[0], handH[0]);
    handBefore[1] = rect_stats(fb, PROBE_FB_W,
                               handX[1], handY[1], handW[1], handH[1]);

    mutate_slot0_for_repaint(&game);
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    secondIconSource = M11_GameView_GetV1ChampionIconSourceIndex(&game, 0);
    iconAfter = rect_stats(fb, PROBE_FB_W, iconX, iconY, iconW, iconH);
    handAfter[0] = rect_stats(fb, PROBE_FB_W,
                              handX[0], handY[0], handW[0], handH[0]);
    handAfter[1] = rect_stats(fb, PROBE_FB_W,
                              handX[1], handY[1], handW[1], handH[1]);

    ok &= expect_true("slot0 champion icon source changed",
                      firstIconSource >= 0 &&
                      secondIconSource >= 0 &&
                      firstIconSource != secondIconSource);
    ok &= expect_true("slot0 champion icon pixels repainted",
                      iconBefore.checksum != iconAfter.checksum &&
                      iconAfter.nonBlack > 20);

    for (stat = 0; stat < 3; ++stat) {
        ok &= check_bar_after_repaint(&game, fb, stat);
    }

    ok &= check_slot_box_perimeter(&game, fb, 0,
                                   M11_GameView_GetV1SlotBoxWoundedGraphicId());
    ok &= check_slot_box_perimeter(&game, fb, 1,
                                   M11_GameView_GetV1SlotBoxActingHandGraphicId());
    ok &= expect_true("slot0 ready hand pixels repainted",
                      handBefore[0].checksum != handAfter[0].checksum &&
                      handAfter[0].nonBlack > 60);
    ok &= expect_true("slot0 action hand pixels repainted",
                      handBefore[1].checksum != handAfter[1].checksum &&
                      handAfter[1].nonBlack > 60);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel repaint-delta runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
