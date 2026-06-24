/*
 * DM1 V1 Hall of Champions portrait 07 side_wall_negative probe (gate 271).
 *
 * Slice:
 *   ordinal 7   TIGGY / TAMAL, C026 champion portrait atlas cell 7
 *               (column 7, row 0 of the 8x3 strip).
 *   route       side_wall_negative
 *   batch group 11.
 *   aspect      no-floating negative behavior around the three
 *               wrong-wall approaches to the ordinal-7 D1C portrait
 *               cell on the Hall corridor return leg.
 *
 * Why this slice exists:
 *
 *   The existing ordinal-7 gates already lock the south_return route
 *   ((2, 17) facing SOUTH, the only cell on map 0 with C127 sensorData
 *   = 7 in the local PC 3.4 DUNGEON.DAT):
 *     - firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe.c
 *       (front_north_entry framing; catalog + resurrect + side-wall warm_count)
 *     - firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe.c
 *       (south_return framing; A..E contracts including stale-positive
 *        south -> west transition).
 *     - firestaff_dm1_v1_hoc_champion_portrait_07_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *     - firestaff_dm1_v1_hoc_champion_portrait_07_turn_away_return_portrait_rect_position_137_gate_probe.c
 *     - firestaff_dm1_v1_hall_champion_portrait_07_east_walkpath_rect_position_runtime_probe.c
 *     - firestaff_dm1_v1_hall_champion_portrait_07_walkpath_from_stairs_runtime_probe.c
 *
 *   None of those gates verifies that a stale ordinal-7 portrait
 *   painted at (2, 17) facing SOUTH is correctly overpainted by the
 *   side-wall redraw when the party steps to a wrong-wall approach
 *   without going through an intervening forward tick.  This gate
 *   closes that lane by:
 *     (a) seeding (2, 17, SOUTH) with a real ordinal-7 blit (>= 90%
 *         pixel match in the D1C portrait rect),
 *     (b) transitioning to each of the three wrong-wall side poses
 *         ((1, 17) EAST, (3, 17) WEST, (2, 18) NORTH) without
 *         clearing the framebuffer in between,
 *     (c) asserting the D1C portrait rect and both side-wall bands
 *         (left 0..79 and right 144..223) lose the ordinal-7 sprite
 *         (stale-positive leak < 35%, symmetric with the side_wall_negative
 *         envelopes locked by ordinal-4 / ordinal-21 siblings).
 *
 *   The 35% stale-leak tolerance is the same envelope locked by the
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe and
 *   reused by the ordinal-21 side_wall_negative gate (gate 189).
 *
 *   This gate deliberately uses a stale-positive seed rather than a
 *   blank-frame probe to verify the F0128_DUNGEONVIEW_Draw_CPSF far-to-near
 *   redraw (ReDMCSB DUNVIEW.C:8318-8542) actually overpaints the side
 *   walls; a blank-frame probe would pass even if the side walls
 *   never redraw at all.
 *
 *   The three side poses are the only source-visible wrong-wall
 *   approaches to (2, 17) on map 0 of the local PC 3.4 DUNGEON.DAT
 *   that keep the party adjacent to the front cell while facing
 *   away from it.  They mirror the (1,2) EAST / (3,2) WEST / (2,3)
 *   NORTH triple the ordinal-4 LEIF side_wall_negative gate uses.
 *
 * Source evidence (ReDMCSB):
 *   DUNGEON.C:2573         visible-wall sensor filter (M011_CELL(sensor)
 *                          - partyDirection + 3, walls only).
 *   DUNGEON.C:2608-2612    stores the C127 sensorData ordinal in G0289.
 *   DUNVIEW.C:3913-3928    blits C026_GRAPHIC_CHAMPION_PORTRAITS into
 *                          the D1C rect (96, 35, 32, 29) viewport
 *                          coords (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                          from DUNVIEW.C:525).
 *   DUNVIEW.C:8318-8542    F0128_DUNGEONVIEW_Draw_CPSF far-to-near
 *                          square redraw, the path that overpaints
 *                          stale portrait pixels on side walls.
 *   MOVESENS.C:1501-1503   passes the C127 sensorData to F0280.
 *   REVIVE.C F0280         materializes the candidate from sensorData.
 *
 * Honest scope:
 *   - This probe is Firestaff runtime evidence against the local DM1 V1
 *     asset set.  It does NOT claim original DOS pixel parity.
 *   - The 90% positive-match threshold and 35% stale-leak threshold are
 *     Firestaff runtime heuristics calibrated against the shipped DM1 V1
 *     DUNGEON.DAT, not paired-against-original metrics.
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

    /* DUNVIEW.C:3913-3928 D1C portrait rect in viewport coords:
     *   x = 96, y = 35, w = 32, h = 29. */
    D1C_PORTRAIT_VX = 96,
    D1C_PORTRAIT_VY = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_TRANSPARENT = 1,
    /* Wall-ornament backdrop palette index (DUNVIEW.C G0205 coordSet 5
     * / index 12 = C346 champion-mirror frame); skipped so the wall
     * ornament does not poison the C026 match count. */
    PORTRAIT_WALL_BACKDROP = 12,

    TARGET_ORDINAL = 7,
    POSITIVE_MATCH_PCT = 90,
    STALE_MATCH_PCT = 35,

    /* Source-visible wrong-wall side approaches to the ordinal-7
     * front cell (2, 17).  Confirmed by the ordinal-7 south_return gate
     * and the firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
     * "no-floating at (2, 17) for non-front directions" check. */
    POSITIVE_X = 2,
    POSITIVE_Y = 17,
    POSITIVE_DIR = DIR_SOUTH,

    /* The two side-wall bands on the D1C viewport.  They sit outside
     * the C346 wall-ornament box (80, 29, 64, 43) and the C026 D1C
     * cutout (96, 35, 32, 29) so a sliding portrait sprite would be
     * picked up by the per-x scan. */
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
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
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

/* C026 atlas match for ordinal `ordinal` against the framebuffer cutout
 * anchored at viewport (dstVx, dstVy).  Transparency (C01 dark gray)
 * and wall-ornament backdrop palette indices are skipped so the
 * underlying wall pixels do not poison the match count.  Mirrors the
 * `match_ordinal_at` helper used by the ordinal-21 side_wall_negative
 * gate (gate 189) and the ordinal-4 side_wall_negative probe. */
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
            if (src == PORTRAIT_TRANSPARENT ||
                src == PORTRAIT_WALL_BACKDROP) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(
                fb[(VIEWPORT_Y + dstVy + y) * FB_W +
                   (VIEWPORT_X + dstVx + x)]);
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

/* Slide the C026 ordinal across [bandX .. bandX+bandW-PORTRAIT_W]
 * along the D1C portrait row (y = D1C_PORTRAIT_VY) and report the
 * best-match (bandX, pct) pair.  Lets us catch a portrait sprite that
 * drifted sideways off the C346 wall ornament frame into the visible
 * left or right wall band. */
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

static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    char msg[240];
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, TARGET_ORDINAL,
                                              name, (int)sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, TARGET_ORDINAL,
                                               title, (int)sizeof(title));
    snprintf(msg, sizeof(msg),
             "ordinal 7 resolves to TIGGY / TAMAL (got %s / %s)",
             name, title);
    CHECK(strcmp(name, "TIGGY") == 0 &&
          strcmp(title, "TAMAL") == 0, msg);
    return strcmp(name, "TIGGY") == 0 &&
           strcmp(title, "TAMAL") == 0;
}

static int draw_positive_anchor(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                unsigned char* fb,
                                const char* label) {
    MatchStats stats;
    int pct;
    int ord;
    char msg[260];

    set_pose(game, POSITIVE_X, POSITIVE_Y, POSITIVE_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "%s positive anchor (2,17,SOUTH) returns ordinal 7 (got %d)",
             label, ord);
    CHECK(ord == TARGET_ORDINAL, msg);

    M11_GameView_Draw(game, fb, FB_W, FB_H);
    stats = match_ordinal_at(portraits, fb, D1C_PORTRAIT_VX,
                             D1C_PORTRAIT_VY, TARGET_ORDINAL);
    pct = match_pct(stats);
    snprintf(msg, sizeof(msg),
             "%s D1C portrait rect at (96,35) matches ordinal 7 >= %d%% "
             "(got %d%%, %d/%d)",
             label, POSITIVE_MATCH_PCT, pct, stats.matched, stats.compared);
    CHECK(stats.compared > 0 && pct >= POSITIVE_MATCH_PCT, msg);

    return ord == TARGET_ORDINAL && stats.compared > 0 &&
           pct >= POSITIVE_MATCH_PCT;
}

static void check_negative_after_stale_positive(M11_GameViewState* game,
                                                const M11_AssetSlot* portraits,
                                                unsigned char* fb,
                                                const NegativePose* pose) {
    int ord;
    MatchStats center;
    SideScan left;
    SideScan right;
    int centerPct;
    char msg[280];

    printf("\n[negative] %s (%d,%d,dir=%d)\n",
           pose->label, pose->x, pose->y, pose->dir);

    memset(fb, 0, (size_t)FB_W * (size_t)FB_H);
    if (!draw_positive_anchor(game, portraits, fb, "stale_seed")) {
        SKIP("positive ordinal-7 anchor unavailable; "
             "stale-positive redraw not meaningful");
        return;
    }

    /* Turn the party to the wrong-wall side pose WITHOUT clearing the
     * framebuffer.  This is the slice that exercises
     * F0128_DUNGEONVIEW_Draw_CPSF's far-to-near side-wall redraw: the
     * D1C rect must lose the seeded ordinal-7 pixels even though the
     * positive blit is still sitting in the framebuffer memory. */
    set_pose(game, pose->x, pose->y, pose->dir);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "%s front mirror ordinal == -1 under DUNGEON.C visible-wall filter (got %d)",
             pose->label, ord);
    CHECK(ord == -1, msg);

    M11_GameView_Draw(game, fb, FB_W, FB_H);

    center = match_ordinal_at(portraits, fb, D1C_PORTRAIT_VX,
                              D1C_PORTRAIT_VY, TARGET_ORDINAL);
    centerPct = match_pct(center);
    snprintf(msg, sizeof(msg),
             "%s stale-positive D1C rect is overpainted < %d%% "
             "(got %d%%, %d/%d)",
             pose->label, STALE_MATCH_PCT, centerPct,
             center.matched, center.compared);
    CHECK(center.compared > 0 && centerPct < STALE_MATCH_PCT, msg);

    left = scan_side_band(portraits, fb, SIDE_LEFT_X, SIDE_LEFT_W,
                          TARGET_ORDINAL);
    snprintf(msg, sizeof(msg),
             "%s left side-wall band has no sliding ordinal-7 portrait < %d%% "
             "(best x=%d got %d%%, %d/%d)",
             pose->label, STALE_MATCH_PCT, left.bestX, left.bestPct,
             left.bestMatched, left.compared);
    CHECK(left.compared > 0 && left.bestPct < STALE_MATCH_PCT, msg);

    right = scan_side_band(portraits, fb, SIDE_RIGHT_X, SIDE_RIGHT_W,
                           TARGET_ORDINAL);
    snprintf(msg, sizeof(msg),
             "%s right side-wall band has no sliding ordinal-7 portrait < %d%% "
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
    /* The three source-visible wrong-wall approaches around the
     * ordinal-7 D1C portrait cell (2, 17) on map 0.  Mirrors the
     * (1,2) EAST / (3,2) WEST / (2,3) NORTH triple the ordinal-4 LEIF
     * side_wall_negative gate uses against the LEIF chamber. */
    static const NegativePose kNegatives[] = {
        {1, 17, DIR_EAST,  "tiggy_front_cell_west_side_wrong_wall"},
        {3, 17, DIR_WEST,  "tiggy_front_cell_east_side_wrong_wall"},
        {2, 18, DIR_NORTH, "tiggy_front_cell_south_side_wrong_wall"}
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

    printf("=== DM1 V1 HoC champion portrait 07 side_wall_negative gate 271 ===\n");
    printf("=== (batch group 11) ===\n");
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

    (void)check_catalog_identity(&game);

    printf("\n[positive] canonical TIGGY / TAMAL D1C rect liveness\n");
    memset(fb, 0, sizeof(fb));
    if (!draw_positive_anchor(&game, portraits, fb, "canonical")) {
        SKIP("fixture mismatch: ordinal-7 TIGGY anchor is not available");
        M11_GameView_Shutdown(&game);
        printf("=== Summary: %d passed, %d failed, %d skipped ===\n",
               g_pass, g_fail, g_skip);
        return g_fail == 0 ? 0 : 1;
    }

    for (i = 0; i < (int)(sizeof(kNegatives) / sizeof(kNegatives[0])); ++i) {
        check_negative_after_stale_positive(&game, portraits, fb,
                                            &kNegatives[i]);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed, %d skipped ===\n",
           g_pass, g_fail, g_skip);
    return g_fail == 0 ? 0 : 1;
}
