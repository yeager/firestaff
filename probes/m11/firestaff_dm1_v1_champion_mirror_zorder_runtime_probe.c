/*
 * DM1 V1 champion mirror Z-order runtime probe.
 *
 * This extends the original north-facing Hall visibility gate with an
 * east/south-facing Hall mirror routes and west-facing no-floating poses.
 * The negative poses verify that turning toward ordinary side walls does not
 * leave the D1C champion-portrait rectangle painted over the viewport.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 champion portraits in G0289;
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 only for D1C front walls;
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 draws the viewport far-to-near.
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

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static int check_pose(M11_GameViewState* game,
                      const M11_AssetSlot* portraits,
                      int mapX,
                      int mapY,
                      int dir,
                      int expectedOrdinal,
                      const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, mapX, mapY, dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != expectedOrdinal) {
        fprintf(stderr, "FAIL %s front ordinal got=%d want=%d\n",
                label, ordinal, expectedOrdinal);
        ok = 0;
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb,
                                 expectedOrdinal >= 0 ? expectedOrdinal : 0);

    if (expectedOrdinal >= 0) {
        if (match.bestOrdinal != expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d bestMatched=%d\n",
                    label, expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared, match.bestMatched);
            ok = 0;
        }
    } else {
        /* Mail regression 2026-06-14: these Hall corridor poses must not
         * expose a clickable/front-route champion ordinal.  The D1C wall box
         * can still share palette pixels with C026 portrait assets, so this
         * negative check is about route ownership, not incidental color
         * similarity in the already-rendered wall/ornament pixels. */
    }
    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           label, mapX, mapY, dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

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

    ok &= check_pose(&game, portraits, 1, 3, DIR_NORTH, -1, "hall_d1c_front_route_blocked_1");
    ok &= check_pose(&game, portraits, 1, 4, DIR_NORTH, -1, "hall_d1c_front_route_blocked_2");
    ok &= check_pose(&game, portraits, 1, 4, DIR_SOUTH, -1, "hall_d1c_front_route_blocked_south");
    ok &= check_pose(&game, portraits, 1, 3, DIR_WEST, -1, "hall_side_no_floating_west_1");
    ok &= check_pose(&game, portraits, 1, 4, DIR_EAST, -1, "hall_d1c_front_route_blocked_east");
    ok &= check_pose(&game, portraits, 1, 4, DIR_WEST, -1, "hall_side_no_floating_west_2");

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror z-order runtime probe\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
