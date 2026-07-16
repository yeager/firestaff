/*
 * DM1 V1 spell-rune panel material runtime probe.
 *
 * Firestaff-side evidence only. With hash-verified local DM1 data, render an
 * open spell panel and bind its C009 background plus the two source-owned
 * C011 line copies to the live V1 framebuffer. No DOS screenshot-parity
 * claim is made.
 *
 * ReDMCSB source lock:
 *   CASTER.C F0394 lines 22-31 blits C009 into G0000 and clears it when
 *   inactive. F0394/F0396 then use C011 rows two and three for spell-area
 *   controls; DEFS.H C009/C011 records that C011's first row is not drawn.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_C009_DRAW_W = 87,
    PROBE_C009_DRAW_H = 25,
    PROBE_C011_W = 14,
    PROBE_C011_H = 12,
    PROBE_C011_ROW1_Y = 13,
    PROBE_C011_ROW2_Y = 26,
    PROBE_C011_DST_X = 224,
    PROBE_C011_DST_ROW1_Y = 50,
    PROBE_C011_DST_ROW2_Y = 62
};

static unsigned char pixel_index(const unsigned char* fb, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * PROBE_FB_W + x]);
}

static int expect_true(const char* label, int value) {
    if (!value) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int point_in_c011_copy(int x, int y) {
    int inX = x >= PROBE_C011_DST_X && x < PROBE_C011_DST_X + PROBE_C011_W;
    return inX && ((y >= PROBE_C011_DST_ROW1_Y &&
                    y < PROBE_C011_DST_ROW1_Y + PROBE_C011_H) ||
                   (y >= PROBE_C011_DST_ROW2_Y &&
                    y < PROBE_C011_DST_ROW2_Y + PROBE_C011_H));
}

/* m11_draw_v1_spell_area_overlay prints the six available rune glyphs after
 * C009/C011 material composition. They are source-font output, not C009
 * material, so exclude only their fixed F0394 control cells. */
static int point_in_available_rune_glyph(int x, int y) {
    int glyph;
    if (y < 58 || y >= 70) {
        return 0;
    }
    for (glyph = 0; glyph < 6; ++glyph) {
        int glyphX = 239 + glyph * 14;
        if (x >= glyphX && x < glyphX + 8) {
            return 1;
        }
    }
    return 0;
}

static int check_c009_background(const M11_AssetSlot* background,
                                 const unsigned char* fb,
                                 int dstX,
                                 int dstY) {
    int expected = 0;
    int matched = 0;
    int y;
    for (y = 0; y < PROBE_C009_DRAW_H; ++y) {
        int x;
        for (x = 0; x < PROBE_C009_DRAW_W; ++x) {
            int screenX = dstX + x;
            int screenY = dstY + y;
            unsigned char src;
            if (point_in_c011_copy(screenX, screenY) ||
                point_in_available_rune_glyph(screenX, screenY)) {
                continue;
            }
            src = (unsigned char)(background->pixels[
                y * (int)background->width + x] & 0x0F);
            ++expected;
            if (pixel_index(fb, screenX, screenY) == src) {
                ++matched;
            }
        }
    }
    printf("C009 unobscured material=%d/%d\n", matched, expected);
    return expected > 0 && matched == expected;
}

static int check_c011_row(const M11_AssetSlot* lines,
                          const unsigned char* fb,
                          int sourceY,
                          int destinationY) {
    int expected = 0;
    int matched = 0;
    int y;
    for (y = 0; y < PROBE_C011_H; ++y) {
        int x;
        for (x = 0; x < PROBE_C011_W; ++x) {
            unsigned char src = (unsigned char)(lines->pixels[
                (sourceY + y) * (int)lines->width + x] & 0x0F);
            ++expected;
            if (pixel_index(fb, PROBE_C011_DST_X + x, destinationY + y) == src) {
                ++matched;
            }
        }
    }
    printf("C011 sourceY=%d material=%d/%d\n", sourceY, matched, expected);
    return expected == PROBE_C011_W * PROBE_C011_H && matched == expected;
}

static void seed_open_spell_panel(M11_GameViewState* game) {
    struct ChampionState_Compat* champion;
    if (!game) return;
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    champion = &game->world.party.champions[0];
    champion->present = 1;
    champion->hp.current = 100;
    champion->hp.maximum = 100;
    champion->stamina.current = 80;
    champion->stamina.maximum = 80;
    champion->mana.current = 60;
    champion->mana.maximum = 60;
    memset(champion->name, ' ', sizeof(champion->name));
    memcpy(champion->name, "TIGGY", 5);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 1;
    game->spellRuneRow = 0;
    memset(&game->spellBuffer, 0, sizeof(game->spellBuffer));
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuInitOptions menuOptions;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    DM1_V1_SpellAreaRectPc34 spellRect;
    const M11_AssetSlot* background;
    const M11_AssetSlot* lines;
    int ok = 1;

    if (argc < 2 || !argv[1] || !argv[1][0]) {
        printf("SKIP dm1 spell-rune material probe: no DATA_DIR supplied\n");
        return 0;
    }
    dataDir = argv[1];
    memset(&menuOptions, 0, sizeof(menuOptions));
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, dataDir, "dm1", &menuOptions);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP dm1 spell-rune material probe: data unavailable at %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    if (!game.assetsAvailable) {
        printf("SKIP dm1 spell-rune material probe: GRAPHICS.DAT unavailable at %s\n",
               dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    seed_open_spell_panel(&game);
    memset(fb, 0xFF, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    spellRect = dm1_v1_spell_area_graphic_rect_pc34();
    background = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game.assetLoader,
        (unsigned int)DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34);
    lines = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game.assetLoader,
        (unsigned int)DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34);
    ok &= expect_true("C009 GRAPHICS.DAT background dimensions",
                      background && background->loaded && background->pixels &&
                      background->width >= PROBE_C009_DRAW_W &&
                      background->height >= PROBE_C009_DRAW_H);
    ok &= expect_true("C011 GRAPHICS.DAT lines dimensions",
                      lines && lines->loaded && lines->pixels &&
                      lines->width >= PROBE_C011_W &&
                      lines->height >= PROBE_C011_ROW2_Y + PROBE_C011_H);
    ok &= expect_true("C009 spell destination",
                      spellRect.x == PROBE_C011_DST_X && spellRect.y == 42 &&
                      spellRect.w == 96 && spellRect.h == 33);
    if (ok) {
        ok &= expect_true("C009 unobscured framebuffer material",
                          check_c009_background(background, fb, spellRect.x, spellRect.y));
        ok &= expect_true("C011 second source row framebuffer material",
                          check_c011_row(lines, fb, PROBE_C011_ROW1_Y,
                                         PROBE_C011_DST_ROW1_Y));
        ok &= expect_true("C011 third source row framebuffer material",
                          check_c011_row(lines, fb, PROBE_C011_ROW2_Y,
                                         PROBE_C011_DST_ROW2_Y));
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 spell-rune panel material runtime probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
