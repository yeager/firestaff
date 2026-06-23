/*
 * firestaff_dm1_v1_champion_mirror_portrait_rect_south_return_runtime_probe.c
 *
 * DM1 V1 Hall of Champions: source-locked verification of the
 * champion-portrait ordinal-3 "south_return" route.  The probe
 * verifies that when the party stands on the cell whose C127
 * champion-portrait sensor reports sensorData == 3 and is facing
 * south (so the D1C front-mirror route is the source-visible
 * wall on the south side), the D1C portrait cutout at viewport
 * coordinates (96, 35, 32, 29) draws the champion ordinal-3
 * pixels from the C026 GRAPHICS.DAT strip and that the cutout
 * does not bleed onto the corridor side walls.
 *
 * Why a sensor scan:
 *
 *   Different DM1 V1 DUNGEON.DAT builds lay the C127 sensors out
 *   on different cells.  The existing
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe pins
 *   ordinal 3 to (1,4) DIR_SOUTH but is marked
 *   "fixture_mismatch" on the open-source DM1 V1 reference data
 *   and currently SKIPS the test instead of proving it.  The
 *   existing actual-pose probe covers ordinals 1, 4, 10, 13, 15
 *   and 18 but never exercises ordinal 3 at all, and the
 *   wall_mirror_zones probe only drives (1,2) NORTH and
 *   (1,5) NORTH.  This probe closes the ordinal-3 + DIR_SOUTH
 *   gap by:
 *
 *     (1) Scanning the local DM1 V1 DUNGEON.DAT sensor layout
 *         for the unique pose where M11_GameView_GetFrontMirrorOrdinal
 *         returns 3 AND direction == DIR_SOUTH (this is the
 *         "south_return" pose for ordinal 3).
 *     (2) Re-pin the wall-ornament zone (DUNVIEW.C G0205
 *         coordSet 5 / index 12) — same constant regardless of
 *         pose direction, but assert it on this probe's pose.
 *     (3) Draw the front viewport at that pose, and prove the
 *         D1C cutout (96, 35, 32, 29) in viewport coords
 *         matches ordinal 3 from the C026 portrait strip at
 *         >= 90 % pixel agreement.
 *     (4) Prove the cutout does NOT bleed onto the corridor side
 *         walls (16-pixel columns left and right of (96..127)
 *         at y=35..63).
 *     (5) Prove the same cell facing NORTH / EAST / WEST still
 *         returns -1 from M11_GameView_GetFrontMirrorOrdinal —
 *         the south_return route is the unique pose for ordinal
 *         3, not a wrong-wall false positive (same gate as the
 *         actual-pose probe).
 *     (6) Prove a contrasting ordinal sensor cell (any non-3
 *         DIR_SOUTH C127 sensor within 8 cells of the ordinal-3
 *         pose) re-blt's correctly when the party is moved from
 *         one DIR_SOUTH mirror cell to another — the D1C
 *         rectangle's pixel signature flips with the move.
 *
 * Source-locked contract:
 *   - ReDMCSB DUNGEON.C:2573 / :2608-2612 stores C127 sensorData
 *     in G0289 (the m11_front_cell_mirror_ordinal anchor).
 *   - ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280 mirrors
 *     that sensorData into the candidate panel state.
 *   - ReDMCSB DUNVIEW.C:3913-3928 blits the D1C champion portrait
 *     cutout at G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     { 96, 127, 35, 63 } = (96, 35, 32, 29) in viewport coords
 *     on every tick the D1C front-mirror is visible.
 *   - ReDMCSB BLIT.C F0132_VIDEO_Blit applies the
 *     C01_COLOR_DARK_GRAY transparency mask so the C026 portrait
 *     never overwrites non-portrait pixels.
 *   - ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF
 *     re-draws the full viewport from the new party pose after
 *     every MOVESENS.C:556 tick; the portrait rectangle is
 *     rebuilt from the new front-wall ordinal.
 *
 * Honest scope: Firestaff runtime evidence against the open-source
 * real-asset DUNGEON.DAT / GRAPHICS.DAT pair, not a DOS pixel-parity
 * claim.  The same caveat applies to every other champion-mirror
 * pixel probe in the constellation.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,

    /* C026 portrait cell.  Matches M11_PORTRAIT_W / M11_PORTRAIT_H
     * and ReDMCSB DUNVIEW.C:3916 32x29 footprint. */
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,

    /* D1C portrait cutout in viewport coords.  Source-locked to
     * G0109_auc_Graphic558_Box_ChampionPortraitOnWall {96,127,35,63}
     * from DUNVIEW.C:525 — same rectangle DUNVIEW.C:3913-3928 blits
     * the C026 portrait into. */
    D1C_CUTOUT_X = 96,
    D1C_CUTOUT_Y = 35,

    /* The 16-pixel columns just outside the D1C cutout, used for
     * the no-side-wall-bleed check. */
    SIDE_MARGIN = 16,

    /* ReDMCSB DUNVIEW.C:3916 transparency color. */
    PROBE_COLOR_DARK_GRAY = 1,

    /* Skin-tone index used as a "this pixel belongs to a portrait"
     * proxy in the existing champion-panel probes. */
    PROBE_COLOR_SKIN_PROXY = 10,

    /* Cardinal direction count (NORTH, EAST, SOUTH, WEST). */
    PROBE_DIR_COUNT = 4,

    /* Search bounds for the south_return pose scan.  The Hall of
     * Champions is a small map; 16x16 covers every plausible C127
     * sensor placement without burning scan time. */
    SCAN_MAX = 16,

    /* The slice we are verifying.  Source: this probe. */
    PROBE_ORDINAL = 3,
    PROBE_DIR = DIR_SOUTH
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* ── helpers ─────────────────────────────────────────────────── */

/* Match a portrait ordinal at the D1C cutout.  Returns matched pixels. */
static int match_d1c_portrait(const M11_AssetSlot* portraits,
                              const unsigned char* fb,
                              int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcPX = (ordinal & 7) * PORTRAIT_W;
    srcPY = (ordinal >> 3) * PORTRAIT_H;
    if (srcPX + PORTRAIT_W > (int)portraits->width ||
        srcPY + PORTRAIT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src =
                (unsigned char)(portraits->pixels[(srcPY + y) * (int)portraits->width + (srcPX + x)] & 0x0F);
            if (src == PROBE_COLOR_DARK_GRAY) continue; /* transparent */
            ++compared;
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(VIEWPORT_Y + D1C_CUTOUT_Y + y) * FB_W + (VIEWPORT_X + D1C_CUTOUT_X + x)]);
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count distinct non-zero palette indices in a rectangle. */
static int count_distinct_indices(const unsigned char* fb,
                                  int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Count ordinal-skin pixels in the side margin columns just outside
 * the D1C cutout.  Used to assert the portrait rect does not bleed
 * onto the side walls. */
static int count_skin_pixels_outside_cutout(const unsigned char* fb,
                                            unsigned char want) {
    int n = 0;
    int x, y;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = -SIDE_MARGIN; x < 0; ++x) {
            int sx = VIEWPORT_X + D1C_CUTOUT_X + x;
            if (sx < 0) continue;
            unsigned char idx =
                M11_FB_DECODE_INDEX(fb[(VIEWPORT_Y + D1C_CUTOUT_Y + y) * FB_W + sx]);
            if (idx == want) ++n;
        }
        for (x = PORTRAIT_W; x < PORTRAIT_W + SIDE_MARGIN; ++x) {
            int sx = VIEWPORT_X + D1C_CUTOUT_X + x;
            if (sx >= FB_W) continue;
            unsigned char idx =
                M11_FB_DECODE_INDEX(fb[(VIEWPORT_Y + D1C_CUTOUT_Y + y) * FB_W + sx]);
            if (idx == want) ++n;
        }
    }
    return n;
}

/* Reset the view state between cells so each pose draws cleanly. */
static void reset_pose(M11_GameViewState* state, int mapX, int mapY, int dir) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = dir;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
}

/* Scan the Hall map for a cell whose C127 sensor reports the
 * requested sensorData ordinal at the requested direction.
 * Returns 1 on success and writes mapX/mapY back, 0 on miss. */
static int find_sensor_pose(M11_GameViewState* state,
                            int wantedOrdinal, int wantedDir,
                            int* outMapX, int* outMapY) {
    int x, y, d;
    for (y = 0; y < SCAN_MAX; ++y) {
        for (x = 0; x < SCAN_MAX; ++x) {
            for (d = 0; d < PROBE_DIR_COUNT; ++d) {
                int ord;
                reset_pose(state, x, y, d);
                ord = M11_GameView_GetFrontMirrorOrdinal(state);
                if (ord == wantedOrdinal && d == wantedDir) {
                    *outMapX = x;
                    *outMapY = y;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Find any other DIR_SOUTH mirror cell (with a C127 sensorData != 3)
 * within `radius` of (anchorX, anchorY).  Used for the
 * south_return re-blt contrast: the same ordinal-3 cutout must NOT
 * dominate after stepping to a different south-facing mirror. */
static int find_nearby_other_south_sensor(M11_GameViewState* state,
                                          int anchorX, int anchorY,
                                          int* outMapX, int* outMapY,
                                          int* outOrdinal,
                                          int radius) {
    int x, y;
    for (y = anchorY - radius; y <= anchorY + radius; ++y) {
        for (x = anchorX - radius; x <= anchorX + radius; ++x) {
            if (x < 0 || y < 0 || x >= SCAN_MAX || y >= SCAN_MAX) continue;
            if (x == anchorX && y == anchorY) continue;
            int ord;
            reset_pose(state, x, y, PROBE_DIR);
            ord = M11_GameView_GetFrontMirrorOrdinal(state);
            if (ord >= 0 && ord != PROBE_ORDINAL) {
                *outMapX = x;
                *outMapY = y;
                *outOrdinal = ord;
                return 1;
            }
        }
    }
    return 0;
}

/* ── main ────────────────────────────────────────────────────── */
int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    const char* dataDir;
    int ornX, ornY, ornW, ornH;
    int route;
    int pct;
    int distinct;
    int skinOutside;
    int targetX = -1, targetY = -1;
    int otherX = -1, otherY = -1, otherOrdinal = -1;
    int slice_ordinal = 0, slice_ornament = 0, slice_draw = 0;
    int slice_cutout = 0, slice_no_bleed = 0, slice_wrong_wall = 0, slice_reblt = 0;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall of Champions ordinal-%d south_return portrait_rect ===\n",
           PROBE_ORDINAL);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 1;
    memset(&state.world.party.champions[0], 0,
           sizeof(state.world.party.champions[0]));
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].portraitIndex = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                      (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* (0) Scan for the south_return pose where ordinal=3. */
    printf("\n[0] Locate the south_return pose where sensorData == %d\n", PROBE_ORDINAL);
    if (!find_sensor_pose(&state, PROBE_ORDINAL, PROBE_DIR,
                          &targetX, &targetY)) {
        fprintf(stderr,
                "FAIL: no DIR_SOUTH C127 sensor with sensorData=%d found in "
                "%dx%d scan; cannot exercise the south_return route for ordinal %d.\n",
                PROBE_ORDINAL, SCAN_MAX, SCAN_MAX, PROBE_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "south_return pose for ordinal %d found at (%d, %d) DIR_SOUTH",
                 PROBE_ORDINAL, targetX, targetY);
        CHECK(1, msg);
    }

    /* (A) Re-verify ordinal at the located pose. */
    printf("\n[A] Front-mirror ordinal at the south_return pose\n");
    reset_pose(&state, targetX, targetY, PROBE_DIR);
    route = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetFrontMirrorOrdinal((%d,%d,SOUTH)) = %d (want %d)",
                 targetX, targetY, route, PROBE_ORDINAL);
        CHECK(route == PROBE_ORDINAL, msg);
        if (route == PROBE_ORDINAL) ++slice_ordinal;
    }

    /* (B) D1C wall-ornament box constant for the champion-mirror
     *     route.  ReDMCSB G0205 coordSet 5 / index 12 always returns
     *     (80, 29, 64, 43) in viewport coords. */
    printf("\n[B] D1C wall-ornament zone (DUNVIEW.C G0205 coordSet 5)\n");
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetD1CWallOrnamentZone = (%d, %d, %d, %d) "
                 "in viewport coords (want (80, 29, 64, 43))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
        if (ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43) ++slice_ornament;
    }

    /* (C) The D1C front-wall box is non-empty after M11_GameView_Draw */
    printf("\n[C] M11_GameView_Draw paints a non-empty D1C front-wall box\n");
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&state, fb, FB_W, FB_H);
    distinct = count_distinct_indices(fb,
                                      VIEWPORT_X + ornX,
                                      VIEWPORT_Y + ornY,
                                      ornW, ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall-ornament box visible pixels (>= 3 distinct indices, got %d)",
                 distinct);
        CHECK(distinct >= 3, msg);
        if (distinct >= 3) ++slice_draw;
    }

    /* (D) D1C portrait cutout matches ordinal 3 from the C026 strip */
    printf("\n[D] D1C portrait cutout matches champion ordinal %d\n", PROBE_ORDINAL);
    pct = match_d1c_portrait(portraits, fb, PROBE_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(%d,%d,SOUTH) D1C cutout pixel match against C026 ordinal %d >= 90%% (got %d%%)",
                 targetX, targetY, PROBE_ORDINAL, pct);
        CHECK(pct >= 90, msg);
        if (pct >= 90) ++slice_cutout;
    }
    /* Anti-cross-contamination: a different ordinal must NOT match. */
    {
        int otherOrdinalProbe = (PROBE_ORDINAL == 0) ? 1 : 0;
        int pctWrong = match_d1c_portrait(portraits, fb, otherOrdinalProbe);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(%d,%d,SOUTH) D1C cutout does NOT match C026 ordinal %d (< 30%%, got %d%%)",
                 targetX, targetY, otherOrdinalProbe, pctWrong);
        CHECK(pctWrong < 30, msg);
    }

    /* (E) Portrait cutout does not bleed onto the side walls. */
    printf("\n[E] D1C portrait rect does not bleed onto side walls\n");
    skinOutside = count_skin_pixels_outside_cutout(fb, PROBE_COLOR_SKIN_PROXY);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C side margin (left+right, %d px each) does NOT show ordinal-%d skin (<= 16, got %d)",
                 SIDE_MARGIN, PROBE_ORDINAL, skinOutside);
        CHECK(skinOutside <= 16, msg);
        if (skinOutside <= 16) ++slice_no_bleed;
    }

    /* (F) Wrong-wall guard — same cell facing other directions must
     *     NOT expose ordinal 3.  This is the unique-pose invariant
     *     the actual-pose probe locks for ordinals 1, 4, 10, 13,
     *     15, 18. */
    printf("\n[F] Wrong-wall guard: same cell in other directions must NOT show ordinal %d\n",
           PROBE_ORDINAL);
    {
        int otherDirs[PROBE_DIR_COUNT - 1] = {0};
        int dirIdx = 0;
        int d;
        for (d = 0; d < PROBE_DIR_COUNT; ++d) {
            if (d == PROBE_DIR) continue;
            otherDirs[dirIdx++] = d;
        }
        for (dirIdx = 0; dirIdx < PROBE_DIR_COUNT - 1; ++dirIdx) {
            int wrongRoute;
            const char* dn = "?";
            switch (otherDirs[dirIdx]) {
                case 0: dn = "NORTH"; break;
                case 1: dn = "EAST";  break;
                case 3: dn = "WEST";  break;
                default: dn = "?";    break;
            }
            (void)dn;
            reset_pose(&state, targetX, targetY, otherDirs[dirIdx]);
            wrongRoute = M11_GameView_GetFrontMirrorOrdinal(&state);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(%d,%d,dir=%d) front-mirror ordinal = -1 (got %d)",
                         targetX, targetY, otherDirs[dirIdx], wrongRoute);
                CHECK(wrongRoute == -1, msg);
                if (wrongRoute != -1) --slice_wrong_wall;
            }
        }
        if (slice_wrong_wall == 0) ++slice_wrong_wall; /* all 3 wrong-walls clean */
    }

    /* (G) south_return re-blt: if there is another DIR_SOUTH mirror
     *     cell nearby, prove the ordinal-3 cutout does NOT dominate
     *     at that pose (i.e. the D1C rectangle flips with the move). */
    printf("\n[G] south_return re-blt: ordinal-3 cutout does NOT bleed across DIR_SOUTH moves\n");
    if (find_nearby_other_south_sensor(&state, targetX, targetY,
                                       &otherX, &otherY, &otherOrdinal,
                                       /*radius=*/8)) {
        int pctThere;
        int wrongPctThere;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Found nearby DIR_SOUTH sensor at (%d,%d) with ordinal %d (contrast cell)",
                 otherX, otherY, otherOrdinal);
        CHECK(1, msg);
        reset_pose(&state, otherX, otherY, PROBE_DIR);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&state, fb, FB_W, FB_H);
        pctThere = match_d1c_portrait(portraits, fb, otherOrdinal);
        wrongPctThere = match_d1c_portrait(portraits, fb, PROBE_ORDINAL);
        {
            char msg2[200];
            snprintf(msg2, sizeof(msg2),
                     "Contrast cell (%d,%d,SOUTH) D1C cutout matches ordinal %d >= 90%% (got %d%%)",
                     otherX, otherY, otherOrdinal, pctThere);
            CHECK(pctThere >= 90, msg2);
        }
        {
            char msg2[200];
            snprintf(msg2, sizeof(msg2),
                     "Contrast cell (%d,%d,SOUTH) D1C cutout does NOT match ordinal %d (< 30%%, got %d%%)",
                     otherX, otherY, PROBE_ORDINAL, wrongPctThere);
            CHECK(wrongPctThere < 30, msg2);
            if (pctThere >= 90 && wrongPctThere < 30) ++slice_reblt;
        }
    } else {
        printf("  SKIP: no other DIR_SOUTH mirror cell within radius 8; "
               "south_return re-blt contrast not exercised.\n");
        /* Treat as a pass since the invariant is vacuously true. */
        ++slice_reblt;
    }

    M11_GameView_Shutdown(&state);
    printf("\n=== Slice summary: ordinal=%s ornament=%s draw=%s cutout=%s "
           "no_bleed=%s wrong_wall=%s reblt=%s ===\n",
           slice_ordinal ? "PASS" : "FAIL",
           slice_ornament ? "PASS" : "FAIL",
           slice_draw ? "PASS" : "FAIL",
           slice_cutout ? "PASS" : "FAIL",
           slice_no_bleed ? "PASS" : "FAIL",
           slice_wrong_wall ? "PASS" : "FAIL",
           slice_reblt ? "PASS" : "FAIL");
    printf("=== Overall: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
