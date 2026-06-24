/*
 * DM1 V1 Hall of Champions portrait 04 side_wall_negative probe.
 *
 * Slice:
 *   ordinal 4   LEIF, C026 champion portrait atlas cell 4
 *   route       side_wall_negative
 *   aspect      no-floating negative behavior around the side walls
 *
 * Existing ordinal-4 probes lock the canonical D1C portrait rectangle
 * (2,1,SOUTH), the (1,2,EAST) wrong-wall D1C cutout, the sealed x=2
 * chamber path, south_return redraw stability, and candidate-panel
 * close behavior.  This probe keeps a narrower contract: the three
 * source-visible wrong-wall approaches around the LEIF chamber
 * ((1,2,EAST), (3,2,WEST), and (2,3,NORTH)) must not show an ordinal-4
 * portrait in the D1C cutout or sliding into either side-wall band.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 filters C127 sensors by visible wall side.
 *   ReDMCSB DUNGEON.C:2608-2612 stores the C127 sensorData ordinal.
 *   ReDMCSB DUNVIEW.C:525 defines
 *     G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026_GRAPHIC_CHAMPION_PORTRAITS
 *     into that D1C box after the C346 wall ornament.
 *   ReDMCSB DUNVIEW.C:8318-8542 redraws viewport squares far-to-near,
 *     so side walls must overpaint any stale portrait when the front
 *     cell has no matching C127 sensor.
 *
 * This is Firestaff runtime evidence against the local DM1 V1 asset
 * set.  It does not claim original DOS pixel parity.
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
    VIEWPORT_W = 224,

    D1C_PORTRAIT_VX = 96,
    D1C_PORTRAIT_VY = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_TRANSPARENT = 1,

    TARGET_ORDINAL = 4,
    POSITIVE_MATCH_PCT = 90,
    STALE_MATCH_PCT = 35,

    SIDE_LEFT_X = 0,
    SIDE_LEFT_W = 80,
    SIDE_RIGHT_X = 144,
    SIDE_RIGHT_W = 80
};

typedef struct MatchStats {
    int matched;
    int compared;
} MatchStats;

typedef struct SideScan {
    int bestPct;
    int bestX;
    int bestMatched;
    int compared;
} SideScan;

typedef struct NegativePose {
    int x;
    int y;
    int dir;
    const char* label;
} NegativePose;

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

static MatchStats match_ordinal_at(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int dstVx,
                                   int dstVy,
                                   int ordinal) {
    MatchStats out;
    int x;
    int y;
    int srcX0;
    int srcY0;
    memset(&out, 0, sizeof(out));
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return out;
    }
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    if (srcX0 + PORTRAIT_W > (int)portraits->width ||
        srcY0 + PORTRAIT_H > (int)portraits->height) {
        return out;
    }
    if (dstVx < 0 || dstVy < 0 ||
        dstVx + PORTRAIT_W > VIEWPORT_W ||
        dstVy + PORTRAIT_H > FB_H - VIEWPORT_Y) {
        return out;
    }

    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)
                (portraits->pixels[(srcY0 + y) * (int)portraits->width +
                                   (srcX0 + x)] & 0x0F);
            unsigned char dst;
            if (src == PORTRAIT_TRANSPARENT) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(
                fb[(VIEWPORT_Y + dstVy + y) * FB_W + (VIEWPORT_X + dstVx + x)]);
            ++out.compared;
            if (dst == src) {
                ++out.matched;
            }
        }
    }
    return out;
}

static int match_pct(MatchStats stats) {
    return stats.compared > 0 ? (stats.matched * 100 / stats.compared) : 0;
}

static SideScan scan_side_band(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int bandX,
                               int bandW,
                               int ordinal) {
    SideScan out;
    int x;
    memset(&out, 0, sizeof(out));
    out.bestX = -1;
    for (x = bandX; x <= bandX + bandW - PORTRAIT_W; ++x) {
        MatchStats stats = match_ordinal_at(portraits, fb, x,
                                            D1C_PORTRAIT_VY, ordinal);
        int pct = match_pct(stats);
        if (pct > out.bestPct || out.bestX < 0) {
            out.bestPct = pct;
            out.bestX = x;
            out.bestMatched = stats.matched;
            out.compared = stats.compared;
        }
    }
    return out;
}

static int check_positive_anchor(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 unsigned char* fb) {
    MatchStats stats;
    int pct;
    int ord;
    char name[32];
    char msg[240];

    set_pose(game, 2, 1, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "canonical LEIF anchor (2,1,SOUTH) returns ordinal 4 (got %d)",
             ord);
    CHECK(ord == TARGET_ORDINAL, msg);

    memset(name, 0, sizeof(name));
    snprintf(msg, sizeof(msg),
             "mirror catalog ordinal 4 resolves to LEIF");
    CHECK(M11_GameView_GetMirrorNameByOrdinal(game, TARGET_ORDINAL,
                                              name, (int)sizeof(name)) > 0 &&
          strcmp(name, "LEIF") == 0,
          msg);

    memset(fb, 0, (size_t)FB_W * (size_t)FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    stats = match_ordinal_at(portraits, fb, D1C_PORTRAIT_VX,
                             D1C_PORTRAIT_VY, TARGET_ORDINAL);
    pct = match_pct(stats);
    snprintf(msg, sizeof(msg),
             "D1C portrait rect at (96,35) matches ordinal 4 >= %d%% "
             "(got %d%%, %d/%d)",
             POSITIVE_MATCH_PCT, pct, stats.matched, stats.compared);
    CHECK(stats.compared > 0 && pct >= POSITIVE_MATCH_PCT, msg);

    return ord == TARGET_ORDINAL && stats.compared > 0 &&
           pct >= POSITIVE_MATCH_PCT;
}

static void check_negative_pose(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                unsigned char* fb,
                                const NegativePose* pose) {
    int ord;
    MatchStats center;
    SideScan left;
    SideScan right;
    int centerPct;
    char msg[260];

    printf("\n[negative] %s (%d,%d,dir=%d)\n",
           pose->label, pose->x, pose->y, pose->dir);
    set_pose(game, pose->x, pose->y, pose->dir);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "%s front mirror ordinal == -1 under DUNGEON.C visible-wall filter (got %d)",
             pose->label, ord);
    CHECK(ord == -1, msg);

    memset(fb, 0, (size_t)FB_W * (size_t)FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    center = match_ordinal_at(portraits, fb, D1C_PORTRAIT_VX,
                              D1C_PORTRAIT_VY, TARGET_ORDINAL);
    centerPct = match_pct(center);
    snprintf(msg, sizeof(msg),
             "%s D1C center rect does not retain ordinal 4 < %d%% "
             "(got %d%%, %d/%d)",
             pose->label, STALE_MATCH_PCT, centerPct,
             center.matched, center.compared);
    CHECK(center.compared > 0 && centerPct < STALE_MATCH_PCT, msg);

    left = scan_side_band(portraits, fb, SIDE_LEFT_X, SIDE_LEFT_W,
                          TARGET_ORDINAL);
    snprintf(msg, sizeof(msg),
             "%s left side-wall band has no sliding ordinal-4 portrait < %d%% "
             "(best x=%d got %d%%, %d/%d)",
             pose->label, STALE_MATCH_PCT, left.bestX, left.bestPct,
             left.bestMatched, left.compared);
    CHECK(left.compared > 0 && left.bestPct < STALE_MATCH_PCT, msg);

    right = scan_side_band(portraits, fb, SIDE_RIGHT_X, SIDE_RIGHT_W,
                           TARGET_ORDINAL);
    snprintf(msg, sizeof(msg),
             "%s right side-wall band has no sliding ordinal-4 portrait < %d%% "
             "(best x=%d got %d%%, %d/%d)",
             pose->label, STALE_MATCH_PCT, right.bestX, right.bestPct,
             right.bestMatched, right.compared);
    CHECK(right.compared > 0 && right.bestPct < STALE_MATCH_PCT, msg);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[FB_W * FB_H];
    static const NegativePose kNegatives[] = {
        {1, 2, DIR_EAST,  "leif_west_side_wrong_wall"},
        {3, 2, DIR_WEST,  "leif_east_side_wrong_wall"},
        {2, 3, DIR_NORTH, "leif_south_side_wrong_wall"}
    };
    int i;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC champion portrait 04 side_wall_negative ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "SKIP: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        SKIP("GRAPHICS.DAT C026 champion portrait strip unavailable");
        M11_GameView_Shutdown(&game);
        printf("=== Summary: %d passed, %d failed, %d skipped ===\n",
               g_pass, g_fail, g_skip);
        return 0;
    }

    printf("\n[positive] canonical LEIF D1C rect liveness\n");
    if (!check_positive_anchor(&game, portraits, fb)) {
        SKIP("fixture mismatch: ordinal-4 LEIF anchor is not available");
        M11_GameView_Shutdown(&game);
        printf("=== Summary: %d passed, %d failed, %d skipped ===\n",
               g_pass, g_fail, g_skip);
        return g_fail == 0 ? 0 : 1;
    }

    for (i = 0; i < (int)(sizeof(kNegatives) / sizeof(kNegatives[0])); ++i) {
        check_negative_pose(&game, portraits, fb, &kNegatives[i]);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed, %d skipped ===\n",
           g_pass, g_fail, g_skip);
    return g_fail == 0 ? 0 : 1;
}
