/*
 * DM1 V1 champion mirror visibility runtime probe.
 *
 * This is a narrow Hall of Champions regression for the 2026-06-14 mail
 * report where champion portraits appeared as a clickable row in the front
 * viewport.  These front Hall corridor poses must not expose the synthetic
 * D1C front mirror route:
 *   (map 0, x=1, y=3, NORTH) -> no front mirror ordinal
 *   (map 0, x=1, y=4, NORTH) -> no front mirror ordinal
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell to front-wall aspect;
 *   ReDMCSB DUNGEON.C:2608-2612 sets G0289 for C127 champion portraits;
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 at the fixed D1C wall box;
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 draws the viewport from party pose.
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
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) {
        return dataDir;
    }
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static MirrorMatch match_front_portrait(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
                unsigned char src =
                    (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                           (PROBE_PORTRAIT_X + x)]);
                if (src == 1) {
                    continue;
                }
                ++compared;
                if (dst == src) {
                    ++matched;
                }
            }
        }
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

static int check_mirror(M11_GameViewState* game,
                        const M11_AssetSlot* portraits,
                        int mapX,
                        int mapY,
                        int expectedOrdinal,
                        const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    MirrorMatch match;
    char routeLabel[96];
    int ok = 1;
    int frontOrdinal;

    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;

    snprintf(routeLabel, sizeof(routeLabel), "%s front mirror ordinal", label);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int(routeLabel, frontOrdinal, expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb, expectedOrdinal);

    /* Negative route guard only.  The wall box can share palette pixels with
     * portrait assets, so incidental C026 color similarity is reported but
     * not treated as a live portrait route. */
    printf("%s route=%d best=%d matched=%d/%d\n",
           label, frontOrdinal, match.bestOrdinal,
           match.expectedMatched, match.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowedDataDir[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(argv[1], narrowedDataDir, sizeof(narrowedDataDir));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= check_mirror(&game, portraits, 1, 3, -1, "hall_start_front_route_blocked");
    ok &= check_mirror(&game, portraits, 1, 4, -1, "hall_corridor_front_route_blocked");

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror visibility runtime probe\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
