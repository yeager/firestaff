/*
 * DM1 V1 Hall of Champions champion mirror ordinal 5 placement probe.
 *
 * This is a narrow runtime gate for the front_north_entry /
 * portrait_rect_position slice.  It uses hash-verified real DM1 data,
 * finds the north-facing front-cell C127 route whose sensorData is
 * portrait ordinal 5, and verifies that M11_GameView_Draw blits the
 * C026 portrait strip cell into the source D1C rectangle:
 * viewport-local (96,35)..(127,63), framebuffer (96,68)..(127,96).
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:525 defines G0109 as {96,127,35,63}
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 portrait into D1C
 *   ReDMCSB DUNVIEW.C:8318-8618 draws the viewport far-to-near
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C F0280 materializes the candidate from sensorData
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    DIR_NORTH_COMPAT = 0,
    TARGET_ORDINAL = 5,
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29
};

typedef struct ProbePose {
    int x;
    int y;
    int direction;
} ProbePose;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int portrait_match_percent(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal,
                                  int* outCompared) {
    int x;
    int y;
    int matched = 0;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        if (outCompared) *outCompared = 0;
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src;
            unsigned char dst;
            if (srcX < 0 || srcY < 0 ||
                srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    if (outCompared) {
        *outCompared = compared;
    }
    return compared > 0 ? (matched * 100 / compared) : 0;
}

static int find_ordinal5_north_route(M11_GameViewState* game,
                                     ProbePose* outPose,
                                     int* outAllDirectionsCount,
                                     int* outNorthCount) {
    const struct DungeonMapDesc_Compat* map;
    int x;
    int y;
    int dir;
    int foundAll = 0;
    int foundNorth = 0;
    if (!game || !game->active || !game->world.dungeon ||
        game->world.dungeon->header.mapCount <= 0) {
        return 0;
    }
    map = &game->world.dungeon->maps[0];
    game->world.party.mapIndex = 0;
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            for (dir = 0; dir < 4; ++dir) {
                int ordinal;
                game->world.party.mapX = x;
                game->world.party.mapY = y;
                game->world.party.direction = dir;
                ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ordinal == TARGET_ORDINAL) {
                    ++foundAll;
                    if (dir == DIR_NORTH_COMPAT) {
                        ++foundNorth;
                        if (outPose) {
                            outPose->x = x;
                            outPose->y = y;
                            outPose->direction = dir;
                        }
                    }
                }
            }
        }
    }
    if (outAllDirectionsCount) {
        *outAllDirectionsCount = foundAll;
    }
    if (outNorthCount) {
        *outNorthCount = foundNorth;
    }
    return foundNorth == 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    ProbePose pose = {-1, -1, -1};
    unsigned char fb[FB_W * FB_H];
    char name[32];
    char title[64];
    int allCount = 0;
    int northCount = 0;
    int compared = 0;
    int pct;
    int dir;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_mirror_ordinal5_rect_runtime_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;
    game.candidateMirrorPanelActive = 0;
    game.candidateMirrorOrdinal = -1;
    game.candidateMirrorPartyIndex = -1;

    printf("=== DM1 V1 champion mirror ordinal 5 D1C rect probe ===\n");
    printf("dataDir=%s\n", dataDir);

    CHECK(M11_GameView_GetMirrorCatalogCount(&game) >= 24,
          "mirror catalog exposes the 24 Hall of Champions records");
    CHECK(M11_GameView_GetMirrorNameByOrdinal(&game, TARGET_ORDINAL,
                                              name, (int)sizeof(name)) > 0,
          "ordinal 5 maps to a named champion");
    CHECK(M11_GameView_GetMirrorTitleByOrdinal(&game, TARGET_ORDINAL,
                                               title, (int)sizeof(title)) > 0,
          "ordinal 5 maps to a champion title");
    CHECK(strcmp(name, "ELIJA") == 0,
          "ordinal 5 maps to champion ELIJA");
    CHECK(strcmp(title, "LION OF YAITOPYA") == 0,
          "ordinal 5 maps to title LION OF YAITOPYA");
    printf("  INFO: ordinal 5 champion=%s title=%s\n", name, title);

    CHECK(find_ordinal5_north_route(&game, &pose, &allCount, &northCount),
          "exactly one north-facing front route exposes ordinal 5");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 has no extra non-north front routes (all=%d north=%d)",
                 allCount, northCount);
        CHECK(allCount == northCount, msg);
    }
    printf("  INFO: ordinal 5 north pose map=0 x=%d y=%d dir=%d\n",
           pose.x, pose.y, pose.direction);

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                      (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    CHECK(portraits && portraits->loaded && portraits->pixels,
          "C026 champion portrait strip is loaded from GRAPHICS.DAT");

    game.world.party.mapIndex = 0;
    game.world.party.mapX = pose.x;
    game.world.party.mapY = pose.y;
    game.world.party.direction = DIR_NORTH_COMPAT;
    CHECK(M11_GameView_GetFrontMirrorOrdinal(&game) == TARGET_ORDINAL,
          "found north pose resolves front mirror ordinal 5 before draw");
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    pct = portrait_match_percent(portraits, fb, TARGET_ORDINAL, &compared);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rectangle (96,35)-(127,63) matches ordinal 5 C026 pixels >= 90%% (got %d%% of %d)",
                 pct, compared);
        CHECK(pct >= 90 && compared > 300, msg);
    }

    for (dir = 0; dir < 4; ++dir) {
        if (dir == DIR_NORTH_COMPAT) {
            continue;
        }
        game.world.party.mapIndex = 0;
        game.world.party.mapX = pose.x;
        game.world.party.mapY = pose.y;
        game.world.party.direction = dir;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        pct = portrait_match_percent(portraits, fb, TARGET_ORDINAL, &compared);
        {
            char msg[180];
            snprintf(msg, sizeof(msg),
                     "same-cell non-north direction %d does not float ordinal 5 in D1C rect (route=%d match=%d%%)",
                     dir, M11_GameView_GetFrontMirrorOrdinal(&game), pct);
            CHECK(M11_GameView_GetFrontMirrorOrdinal(&game) != TARGET_ORDINAL &&
                  pct < 50,
                  msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
