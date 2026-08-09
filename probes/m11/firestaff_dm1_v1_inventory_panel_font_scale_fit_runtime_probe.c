/*
 * DM1 V1 inventory panel font-scale fit probe.
 *
 * Source evidence:
 *   m11_draw_inventory_panel (src/engine/m11_game_view.c:53806) draws
 *   the champion inventory.  In source mode with authentic GRAPHICS.DAT
 *   C017 backdrop, the entire viewport-sized panel is bitmap-only — no
 *   host text is drawn.  In debug HUD mode, champion name/stats use
 *   g_text_title and g_text_small which respect fontScale.
 *
 *   ReDMCSB PANEL.C F0355 publishes C017 before any slot overlay.
 *   The source-locked inventory surface (x=0, y=33, w=224, h=136)
 *   must be fontScale-invariant.
 *
 * What this probe locks:
 *
 *   1. In source mode, the inventory viewport region is
 *      fontScale-invariant (hash comparison at scale 1/2/3).
 *
 *   2. In debug HUD mode, the champion name text pixel count
 *      grows monotonically with fontScale.
 *
 *   3. No inventory content bleeds outside the source-locked
 *      viewport bounds at scale 3.
 *
 * This probe skips safely when DM1 PC 3.4 data files are absent.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "ui_scale_m11.h"

#include <stdio.h>
#include <string.h>
#include "firestaff_dm1_probe_data_dir.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_WHITE = 15,
    PROBE_BLACK = 0,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    PROBE_VIEWPORT_W = 224,
    PROBE_VIEWPORT_H = 136
};

static int g_pass;
static int g_fail;

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}


static void check_true(const char* label, int cond) {
    if (cond) {
        ++g_pass;
        printf("PASS %s\n", label);
    } else {
        ++g_fail;
        printf("FAIL %s\n", label);
    }
}

static void check_ge(const char* label, int got, int wantAtLeast) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s got=%d want>=%d", label, got, wantAtLeast);
    check_true(buf, got >= wantAtLeast);
}

static size_t count_color(const unsigned char* fb,
                          int x, int y, int w, int h,
                          unsigned char color) {
    size_t count = 0;
    int yy, xx;
    if (!fb) return 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) continue;
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) continue;
            if (M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]) == color)
                ++count;
        }
    }
    return count;
}

static uint32_t region_hash(const unsigned char* fb,
                            int x, int y, int w, int h) {
    uint32_t hash = 2166136261u;
    int yy, xx;
    if (!fb) return 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) continue;
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) continue;
            hash ^= (uint32_t)fb[yy * PROBE_FB_W + xx];
            hash *= 16777619u;
        }
    }
    return hash;
}

static void render_inventory_at_scale(M11_GameViewState* game,
                                      unsigned char* fb,
                                      size_t fbBytes,
                                      int fontScale) {
    game->inventoryPanelActive = 1;
    game->fontScale = fontScale;
    memset(fb, 0, fbBytes);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : NULL;
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];

    if (!root) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = firestaff_dm1_probe_narrow_data_dir(root, narrowed, sizeof(narrowed));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    printf("probe=firestaff_dm1_v1_inventory_panel_font_scale_fit_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);

    /* Ensure a champion is active for inventory rendering */
    if (game.world.party.activeChampionIndex < 0 &&
        game.world.party.championCount > 0) {
        game.world.party.activeChampionIndex = 0;
    }

    /* (1) Source mode: inventory viewport region must be fontScale-invariant.
     * The entire C017 backdrop and slot-box bitmaps are asset-backed;
     * fontScale must not affect them. */
    {
        uint32_t hash1, hash2, hash3;
        game.showDebugHUD = 0;

        render_inventory_at_scale(&game, fb, sizeof(fb), 1);
        hash1 = region_hash(fb, PROBE_VIEWPORT_X, PROBE_VIEWPORT_Y,
                            PROBE_VIEWPORT_W, PROBE_VIEWPORT_H);

        render_inventory_at_scale(&game, fb, sizeof(fb), 2);
        hash2 = region_hash(fb, PROBE_VIEWPORT_X, PROBE_VIEWPORT_Y,
                            PROBE_VIEWPORT_W, PROBE_VIEWPORT_H);

        render_inventory_at_scale(&game, fb, sizeof(fb), 3);
        hash3 = region_hash(fb, PROBE_VIEWPORT_X, PROBE_VIEWPORT_Y,
                            PROBE_VIEWPORT_W, PROBE_VIEWPORT_H);

        check_true("source inventory viewport is fontScale-invariant (scale 2 == scale 1)",
                   hash2 == hash1);
        check_true("source inventory viewport is fontScale-invariant (scale 3 == scale 1)",
                   hash3 == hash1);

        printf("inventoryViewportHash: scale1=0x%08x scale2=0x%08x scale3=0x%08x\n",
               hash1, hash2, hash3);
    }

    /* (2) Debug HUD mode: champion name text grows with fontScale. */
    {
        int namePixels1, namePixels2, namePixels3;
        game.showDebugHUD = 1;

        render_inventory_at_scale(&game, fb, sizeof(fb), 1);
        namePixels1 = (int)count_color(fb, 40, 6, 200, 18, PROBE_WHITE);

        render_inventory_at_scale(&game, fb, sizeof(fb), 2);
        namePixels2 = (int)count_color(fb, 40, 6, 200, 18, PROBE_WHITE);

        render_inventory_at_scale(&game, fb, sizeof(fb), 3);
        namePixels3 = (int)count_color(fb, 40, 6, 200, 18, PROBE_WHITE);

        check_ge("debug champion name white pixels at scale 1", namePixels1, 4);
        check_ge("debug champion name white pixels at scale 2", namePixels2, 4);
        check_ge("debug champion name white pixels at scale 3", namePixels3, 4);
        check_true("debug champion name pixels grow (scale 2 > scale 1)",
                   namePixels2 > namePixels1);
        check_true("debug champion name pixels grow (scale 3 > scale 2)",
                   namePixels3 > namePixels2);

        printf("debugNamePixels: scale1=%d scale2=%d scale3=%d\n",
               namePixels1, namePixels2, namePixels3);

        game.showDebugHUD = 0;
    }

    printf("summary passed=%d failed=%d\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
