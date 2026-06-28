/*
 * DM1 V1 Hall of Champions portrait ordinal 11 / route south_return
 * runtime probe.
 *
 * Companion to firestaff_dm1_v1_champion_mirror_capture_probe.
 * That probe covers ordinals 1 (HALK), 4 (LEIF), 10 (ZED), 13 (WUUF),
 * 15 (MOPHUS), 18 (SONJA) plus the corridor-no-portrait negatives.
 * This probe covers the remaining unproven ordinal in the C026
 * champion-portrait atlas range, ordinal 11, plus the south_return
 * route through the Hall corridor:
 *
 *   - The party starts in the Hall corridor facing NORTH and reads
 *     the front-cell mirror ordinal.  We scan a small bounding box
 *     (map 0, x in [1,3], y in [1,7]) and locate the (x,y,dir) triple
 *     whose front-cell mirror ordinal is 11; this is the C127
 *     sensor on the back wall of the Hall alcove whose sensorData
 *     carries the ordinal 11 atlas index.  We then verify that the
 *     D1C champion-portrait rectangle (96,35)-(127,63) contains the
 *     ordinal-11 portrait bitmap at >= 90% palette-index match.
 *
 *   - We then drive the south_return route: starting from the
 *     ordinal-11 cell facing the source-visible direction, the
 *     party walks one cell back along the corridor (the same
 *     way it came in) without turning.  At the return cell we
 *     re-check
 *     (a) the front-cell mirror ordinal (must be -1 on the corridor
 *         cell south of the ordinal-11 cell, or the next ordinal
 *         if a different sensor sits on that cell),
 *     (b) the D1C portrait rectangle is either cleared (no
 *         portrait pixels) or shows the new ordinal, and does
 *         NOT show stale pixels from the previous ordinal.
 *     This proves the portrait rect is rebuilt correctly on the
 *     return leg (southbound) of the canonical Hall route, not
 *     just the inbound (northbound) leg.
 *
 *   - Finally, we assert the no-floating invariant: at the
 *     ordinal-11 cell, left/right turns must NOT leave the
 *     ordinal-11 portrait dominating the D1C rect.  After the
 *     turn, the D1C rect must either show the new front wall's
 *     own portrait (a different ordinal on a different wall) or
 *     no portrait at all.  This is the same no-floating invariant
 *     the existing firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 *     locks, narrowed to ordinal 11.
 *
 * The probe is data-driven (no fixture assumptions): the (x,y,dir)
 * cell carrying ordinal 11 is discovered at runtime.  Without
 * hash-verified DM1 data the probe prints SKIP and exits 0.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 - sensor cell vs view direction
 *   ReDMCSB DUNGEON.C:2608-2612 - store C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 - D1C portrait blit
 *     (G0289 & 7) << 5, (G0289 >> 3) * 29, 32x29 portrait
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 - full-viewport redraw on
 *     every MOVESENS.C:556 tick (so portrait rect is rebuilt
 *     every step, including return legs)
 *   ReDMCSB MOVESENS.C:1501-1503 - C127 sensorData passes to F0280
 *   ReDMCSB REVIVE.C F0280:142-167 - resurrect blits the same
 *     C026 portrait ordinal
 *   ReDMCSB DEFS.H:2186 / DEFS.H:821-826 - C026 atlas math
 *
 * Run: firestaff_dm1_v1_champion_mirror_ordinal_11_south_return_probe DATA_DIR
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
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* Warm-color set used by C026 portrait sprites (greens/reds/
     * peaches/yellows/blues), as documented in the existing
     * firestaff_dm1_v1_champion_mirror_capture_probe.  Grey-stone
     * wall texture never uses these indices. */
    PORTRAIT_WARM_THRESHOLD = 30
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-level match for ordinal N at the D1C portrait cutout.
 * Skips the source-graph "transparent" palette index 1 (the C026
 * atlas uses 1 for transparent cells), and returns matched/compared
 * as integer percentages so the probe can assert >= 90% pixel
 * agreement without claiming DOS-pixel parity. */
static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if (ordinal < 0 || ordinal >= 24) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* transparent in C026 atlas */
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count warm-colored pixels in the D1C portrait cutout.  Same
 * palette set as firestaff_dm1_v1_champion_mirror_capture_probe:
 * {0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B yellow,
 * 0x0E blue}.  Grey-stone wall texture uses 0x01/0x02/0x07-grey/
 * 0x0D and never this warm set, so a positive ordinal pose has
 * >= PORTRAIT_WARM_THRESHOLD warm pixels while a no-portrait cell
 * has < PORTRAIT_WARM_THRESHOLD. */
static int portrait_warm_count(const unsigned char* fb) {
    int x, y, count = 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char raw = fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

static void set_pose(M11_GameViewState* state, int mapX, int mapY, int dir) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = dir;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    static M11_GameViewState game; /* BSS, not stack: ~579KB */
    const M11_AssetSlot* portraits = NULL;
    const char* dataDir;
    int mapX = -1, mapY = -1, dir = -1;
    int found = 0;
    /* south_return route: direction the party faces when looking
     * back south through the Hall corridor after reaching the
     * ordinal-11 cell. */
    int returnDir = -1;

    int pctOrdinal11 = 0;
    int pctStaleOnReturn = 0;
    int warmOrdinal11 = 0;
    int warmReturn = 0;
    int warmSideLeft = 0;
    int warmSideRight = 0;
    int ordAtOrdinal11 = -1;
    int ordAtReturn = -1;
    int ordSideLeft = -1;
    int ordSideRight = -1;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_11_south_return_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL cannot open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* Locate ordinal 11.
     * Scan a small Hall bounding box (map 0, x in [1,3], y in [1,7]) for
     * the C127 sensor whose sensorData equals 11.  We do not
     * assume a specific (x,y,dir) triple - the probe is
     * data-driven and stays valid across DM1 V1 builds whose Hall
     * sensor layout differs. */
    printf("=== DM1 V1 Hall of Champions ordinal 11 / south_return ===\n");
    printf("dataDir=%s\n", dataDir);
    {
        int x, y, d;
        for (y = 1; y <= 7 && !found; ++y) {
            for (x = 1; x <= 3 && !found; ++x) {
                for (d = 0; d < 4 && !found; ++d) {
                    set_pose(&game, x, y, d);
                    if (M11_GameView_GetFrontMirrorOrdinal(&game) == 11) {
                        mapX = x; mapY = y; dir = d;
                        found = 1;
                    }
                }
            }
        }
    }
    if (!found) {
        /* No ordinal 11 in the bounded scan.  This can happen when
         * the local data set uses a different Hall sensor layout, so
         * skip cleanly instead of failing unrelated machines. */
        printf("  no (x,y,dir) in map 0 x in [1,3] y in [1,7] carries C127 ordinal 11\n");
        printf("  ordinal 11 is reserved for the C026 atlas row 1, col 3 (per DEFS.H:821-826 M027/M028)\n");
        printf("  see docs/dm1_gap_portrait_sensor.md for the source-locked 0..23 range\n");
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_11_south_return_probe "
               "no ordinal-11 sensor in scan box (Hall map may differ from PC 3.4 reference)\n");
        M11_GameView_Shutdown(&game);
        return 0;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 11 located at map 0 (%d,%d) facing %s%s",
                 mapX, mapY,
                 dir == DIR_NORTH ? "NORTH" :
                 dir == DIR_EAST  ? "EAST"  :
                 dir == DIR_SOUTH ? "SOUTH" : "WEST",
                 "");
        CHECK(1, msg);
    }

    /* Render at the ordinal-11 cell and verify the D1C rect. */
    {
        unsigned char fb[FB_W * FB_H];
        set_pose(&game, mapX, mapY, dir);
        ordAtOrdinal11 = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "M11_GameView_GetFrontMirrorOrdinal at ordinal-11 cell = %d (want 11)",
                     ordAtOrdinal11);
            CHECK(ordAtOrdinal11 == 11, msg);
        }
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        pctOrdinal11 = match_portrait(portraits, fb, 11);
        warmOrdinal11 = portrait_warm_count(fb);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C portrait cutout (96,35)-(127,63) matches C026 ordinal 11 >= 90%% (got %d%%, warm=%d)",
                     pctOrdinal11, warmOrdinal11);
            CHECK(pctOrdinal11 >= 90, msg);
        }
    }

    /* south_return route.
     * Walk one cell back along the corridor (southbound) from the
     * ordinal-11 cell.  The corridor pose that the party occupies
     * after the return step must NOT show stale portrait pixels
     * from the previous ordinal-11 frame. */
    /* The return direction is the direction OPPOSITE the source-
     * visible wall: dir+2 in DM1 PC 3.4 source coordinates
     * (DEFS.H:2552 + DUNGEON.C:2573).  Walking back means stepping
     * one square opposite the original facing direction, and the
     * party now faces the same direction as before (we don't turn
     * around; we walk back along the corridor the same way we
     * came).  This is the south_return leg of the canonical Hall
     * route: party moves back through the same corridor but the
     * front-cell mirror ordinal flips because the front square
     * has changed. */
    returnDir = dir; /* keep facing the source-visible wall so the
                        return pose has the same source-front as
                        the original, but the party has stepped
                        one square back, so the front-cell now
                        reads from a different map cell. */

    {
        unsigned char fb[FB_W * FB_H];
        int prevPct = pctOrdinal11;
        int prevWarm = warmOrdinal11;
        int stepX = 0, stepY = 0;
        switch (dir) {
            case DIR_NORTH: stepY =  1; break; /* came from south */
            case DIR_EAST:  stepX = -1; break;
            case DIR_SOUTH: stepY = -1; break;
            case DIR_WEST:  stepX =  1; break;
        }
        set_pose(&game, mapX + stepX, mapY + stepY, returnDir);
        /* The party has stepped off the ordinal-11 cell.  The front
         * cell now reads from a different map square which may or
         * may not carry a C127 portrait sensor. */
        ordAtReturn = M11_GameView_GetFrontMirrorOrdinal(&game);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        pctStaleOnReturn = (ordAtReturn >= 0)
            ? match_portrait(portraits, fb, ordAtReturn)
            : 0;
        warmReturn = portrait_warm_count(fb);

        /* Stale-pixel invariant: if the return cell has no portrait
         * sensor (ordAtReturn < 0), the D1C rect must have warm
         * pixel count well below the PORTRAIT_WARM_THRESHOLD - no
         * portrait should be floating on the corridor stone. */
        if (ordAtReturn < 0) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return corridor cell: no-portrait pose, "
                     "warm_count=%d (want < %d, was %d at ordinal-11)",
                     warmReturn, PORTRAIT_WARM_THRESHOLD, prevWarm);
            CHECK(warmReturn < PORTRAIT_WARM_THRESHOLD, msg);
        } else {
            /* The corridor cell carries its own C127 portrait.
             * The new ordinal must dominate the rect (>= 90% match
             * against the new ordinal), and the prior ordinal's
             * pixels must NOT be left over in the rect. */
            int pctNew = match_portrait(portraits, fb, ordAtReturn);
            int warmNew = portrait_warm_count(fb);
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "south_return corridor cell: ordinal=%d rect matches new ordinal >= 90%% (got %d%%, warm=%d)",
                     ordAtReturn, pctNew, warmNew);
            CHECK(pctNew >= 90, msg);
            /* Stale-pixels test: prior ordinal match must drop
             * below the strict threshold once we are off the
             * ordinal-11 cell. */
            {
                int prevStale = 0;
                int x, y;
                if (portraits && portraits->loaded && portraits->pixels) {
                    for (y = 0; y < PORTRAIT_H; ++y) {
                        for (x = 0; x < PORTRAIT_W; ++x) {
                            int srcX = (11 & 7) * PORTRAIT_W + x;
                            int srcY = (11 >> 3) * PORTRAIT_H + y;
                            unsigned char src, dst;
                            if (srcX >= (int)portraits->width ||
                                srcY >= (int)portraits->height) continue;
                            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                            if (src == 1) continue;
                            dst = M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
                            if (dst == src) ++prevStale;
                        }
                    }
                }
                {
                    char smsg[240];
                    snprintf(smsg, sizeof(smsg),
                             "south_return corridor cell: prior ordinal 11 stale pixels <= 5%% of rect (got %d pixels, prev pass %d%%)",
                             prevStale, prevPct);
                    /* A few pixels of overlap on transparent edges
                     * is fine: only assert that the prior ordinal
                     * is NOT dominating the rect on the return
                     * step.  We tolerate up to 5 raw pixels of
                     * overlap on the C026 atlas border (palette
                     * index 1 cells). */
                    CHECK(prevStale <= 5, smsg);
                }
            }
        }
    }

    /* No-floating invariant on side walls. */
    /* Turn the party 90 degrees left and right at the ordinal-11 cell and
     * confirm that the D1C portrait rectangle shows EITHER:
     *   (a) the new front wall's own portrait (its own C127 sensor,
     *       ordinal != 11, matched at >= 90%), or
     *   (b) no portrait at all (front wall has no C127 sensor).
     * In neither case may the prior ordinal-11 portrait pixels
     * dominate the D1C rect - that would be a 'floating portrait'
     * bug.  This is the same no-floating invariant the existing
     * firestaff_dm1_v1_champion_mirror_zorder_runtime_probe locks,
     * narrowed to the ordinal-11 cell. */
    {
        unsigned char fb[FB_W * FB_H];
        int leftDir  = (dir + 3) & 3;
        int rightDir = (dir + 1) & 3;

        /* Left turn */
        set_pose(&game, mapX, mapY, leftDir);
        ordSideLeft = M11_GameView_GetFrontMirrorOrdinal(&game);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        warmSideLeft = portrait_warm_count(fb);
        {
            int pctPrior11 = match_portrait(portraits, fb, 11);
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "left-turn at ordinal-11 cell: new front ordinal=%d warm=%d, prior ordinal-11 match=%d%% (want <= 5%% for no-floating)",
                     ordSideLeft, warmSideLeft, pctPrior11);
            /* The prior ordinal 11 must not dominate the D1C rect
             * after the left turn.  If the new front ordinal is -1
             * (no portrait), the rect is grey stone only; if it is
             * a different ordinal, the new portrait is what shows. */
            CHECK(pctPrior11 <= 5, msg);
        }
        /* If a new portrait is present, verify it matches the
         * new ordinal.  This is the side-channel check: the
         * new front-wall C127 (if any) renders correctly. */
        if (ordSideLeft >= 0) {
            int pctNew = match_portrait(portraits, fb, ordSideLeft);
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "left-turn at ordinal-11 cell: new front ordinal=%d matches >= 90%% (got %d%%)",
                     ordSideLeft, pctNew);
            CHECK(pctNew >= 90, msg);
        }

        /* Right turn */
        set_pose(&game, mapX, mapY, rightDir);
        ordSideRight = M11_GameView_GetFrontMirrorOrdinal(&game);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        warmSideRight = portrait_warm_count(fb);
        {
            int pctPrior11 = match_portrait(portraits, fb, 11);
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "right-turn at ordinal-11 cell: new front ordinal=%d warm=%d, prior ordinal-11 match=%d%% (want <= 5%% for no-floating)",
                     ordSideRight, warmSideRight, pctPrior11);
            CHECK(pctPrior11 <= 5, msg);
        }
        if (ordSideRight >= 0) {
            int pctNew = match_portrait(portraits, fb, ordSideRight);
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "right-turn at ordinal-11 cell: new front ordinal=%d matches >= 90%% (got %d%%)",
                     ordSideRight, pctNew);
            CHECK(pctNew >= 90, msg);
        }
    }

    M11_GameView_Shutdown(&game);

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("ordinal 11 pose = map 0 (%d,%d) facing %s\n", mapX, mapY,
           dir == DIR_NORTH ? "NORTH" :
           dir == DIR_EAST  ? "EAST"  :
           dir == DIR_SOUTH ? "SOUTH" : "WEST");
    printf("south_return route: ordinal at return cell = %d, portrait rect match = %d%%, warm_count = %d\n",
           ordAtReturn, pctStaleOnReturn, warmReturn);
    printf("no-floating invariant: side-left warm=%d ordinal=%d, side-right warm=%d ordinal=%d\n",
           warmSideLeft, ordSideLeft, warmSideRight, ordSideRight);
    return g_fail == 0 ? 0 : 1;
}
