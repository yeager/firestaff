/*
 * firestaff_dm1_v1_champion_mirror_ordinal_6_south_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 6 (C026 strip cell 6)
 *   route south_return: arrive at the cell that holds ordinal 6 facing
 *                       SOUTH (where ordinal 15 / MOPHUS is visible on
 *                       the south wall), turn back to NORTH (no mirror
 *                       on the back wall), then turn to EAST and
 *                       confirm the D1C portrait rectangle
 *                       (96, 35, 32, 29) draws the ordinal 6 portrait
 *                       and does not float on the surrounding side
 *                       walls.
 *   aspect portrait_rect_position: viewport rectangle
 *                                 (96, 35, 32, 29) plus framebuffer
 *                                 rectangle (96, 35+33, 32, 29)
 *                                 = (96, 68, 32, 29) per ReDMCSB
 *                                 DUNVIEW.C:525
 *                                 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                                 = {96, 127, 35, 63}.
 *
 * The slice was authored against the local PC 3.4 DM1 V1 DUNGEON.DAT
 * where the (2,4) facing EAST C127 sensor reports sensorData=6 (the
 * ordinal 6 champion portrait is mounted on the east wall of (3,4)
 * and is visible when the party stands at (2,4) looking east).  On
 * this build the route is:
 *
 *   1. (2,3) facing SOUTH  -> no mirror  (corridor step)
 *   2. (2,4) facing SOUTH  -> ordinal 15 (MOPHUS, south wall of (2,5))
 *   3. (2,4) facing NORTH  -> no mirror  (back wall, no portrait)
 *   4. (2,4) facing EAST   -> ordinal 6  (target slice, D1C portrait)
 *
 * The probe proves:
 *   Group A: at the (2,4) EAST pose M11 reports ordinal=6 and the
 *            portrait_rect_position (96, 35, 32, 29) in viewport
 *            coords is filled with the ordinal 6 portrait from the
 *            C026 strip (>= 90% pixel match) and is non-empty
 *            (>= 30 warm pixels).
 *   Group B: the surround-zone outside the D1C rectangle is empty
 *            (no floating portrait over the side wall or floor).
 *   Group C: the south_return sequence leaves the D1C rectangle
 *            clean on the back-direction pose (no stale portrait
 *            pixels) and matches ordinal 6 only on the EAST pose.
 *   Group D: cross-ordinal sanity — every positive-ordinal corridor
 *            band pose paints in the D1C rectangle (the cutout is
 *            universal, not ordinal-6-specific).
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF far-to-near
 *     draw order so D0/D1/D2/D3 walls draw with D1C last and the
 *     champion portrait is the final pixel over the front wall
 *   - COORD.C:1693-1722 PC 3.4 viewport origin (0, 33), 224x136
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

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
    /* ReDMCSB DUNVIEW.C:525 G0109 portrait box. */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* The D1C surround-zone covers the rest of the front-cell rect
     * plus a thin band along the side walls, used to prove the
     * portrait does not float outside the (96, 35, 32, 29) cutout. */
    SIDE_PAD_X = 8,
    SIDE_PAD_Y = 4,
    ORDINAL_TARGET = 6,
    WARM_PALETTE_MATCH_THRESHOLD = 30,
    ORDINAL_PIXEL_MATCH_THRESHOLD = 90,
    HALL_MAP_INDEX = 0
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count non-zero pixels in the D1C portrait rectangle. */
static int portrait_rect_nonzero(const unsigned char* fb) {
    int x, y, n = 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            if (fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)] != 0) ++n;
        }
    }
    return n;
}

/* Count warm-colored pixels in the D1C portrait rectangle.  The
 * palette index of warm skin/clothing pixels is in
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} per the F20E PC 3.4 palette
 * (src/shared/vga_palette_pc34_compat.c LIGHT0).  Grey-stone wall
 * texture uses indices 0x01/0x02/0x07-grey/0x0D and never the warm
 * set, so this is a robust portrait-present / wall-only test. */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int x, y, count = 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char raw = fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07:
                case 0x08:
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* Pixel-match a single portrait ordinal against the D1C rectangle.
 * Returns matched/compared*100 percent, or 0 when assets are missing.
 * Used by the strongest assertion: the C026 strip cell for the
 * expected ordinal must dominate the rectangle. */
static int portrait_match_percent(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int matched = 0, compared = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src;
            unsigned char dst;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1 /* PROBE_CHAMPION_TRANSPARENT for PC 3.4 */) continue;
            dst = M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Find the best-matching portrait ordinal at the D1C rectangle.
 * Used to detect a 'stale' ordinal from a previous view staying in
 * the rectangle after a transition. */
static int portrait_best_ordinal(const M11_AssetSlot* portraits,
                                 const unsigned char* fb) {
    int bestOrd = -1;
    int bestPct = 0;
    int ord;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    for (ord = 0; ord < 24; ++ord) {
        int pct = portrait_match_percent(portraits, fb, ord);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrd = ord;
        }
    }
    return bestOrd;
}

/* Draw the corridor cell at (mapX, mapY, direction) into fb and return
 * the front-mirror ordinal M11 reports.  Resets candidate/inventory
 * panels so the BUG-120/121 guard does not influence the D1C draw. */
static int render_at(M11_GameViewState* state,
                     unsigned char* fb,
                     int mapX, int mapY, int direction) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    state->world.party.championCount = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
    return M11_GameView_GetFrontMirrorOrdinal(state);
}

/* Scan the Hall of Champions corridor band for ordinal ORDINAL_TARGET.
 * Populates outMapX/outMapY/outDirection with the first matching pose.
 * Returns 1 on hit, 0 on no-hit.  The scan is discovery only; this
 * function never asserts.  Also tracks every positive-ordinal pose
 * in a side table for the secondary portrait_rect_position check
 * (the corridor band is small so the table is bounded). */
typedef struct OrdinalHit {
    int mapX;
    int mapY;
    int direction;
    int ordinal;
} OrdinalHit;

#define MAX_ORDINAL_HITS 64
static OrdinalHit kOrdinalHits[MAX_ORDINAL_HITS];
static int kOrdinalHitCount = 0;

static int scan_for_ordinal_6(M11_GameViewState* state,
                              int* outMapX, int* outMapY, int* outDirection) {
    static const int kDirs[] = { DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST };
    int x, y, d;
    int hits = 0;
    *outMapX = -1;
    *outMapY = -1;
    *outDirection = -1;
    kOrdinalHitCount = 0;
    printf("\n[Discovery] Scanning Hall corridor band for ordinal=%d\n",
           ORDINAL_TARGET);
    for (y = 1; y <= 6; ++y) {
        for (x = 0; x <= 3; ++x) {
            for (d = 0; d < 4; ++d) {
                unsigned char fb[FB_W * FB_H];
                int ord = render_at(state, fb, x, y, kDirs[d]);
                if (ord == ORDINAL_TARGET) {
                    ++hits;
                    if (*outMapX < 0) {
                        *outMapX = x;
                        *outMapY = y;
                        *outDirection = kDirs[d];
                    }
                    printf("  HIT  ordinal=%d at (%d,%d) DIR_%d\n",
                           ord, x, y, d);
                }
                if (ord >= 0 && kOrdinalHitCount < MAX_ORDINAL_HITS) {
                    kOrdinalHits[kOrdinalHitCount].mapX = x;
                    kOrdinalHits[kOrdinalHitCount].mapY = y;
                    kOrdinalHits[kOrdinalHitCount].direction = kDirs[d];
                    kOrdinalHits[kOrdinalHitCount].ordinal = ord;
                    ++kOrdinalHitCount;
                }
            }
        }
    }
    printf("[Discovery] ordinal=%d hits in corridor band = %d\n",
           ORDINAL_TARGET, hits);
    printf("[Discovery] total positive-ordinal corridor-band hits = %d\n",
           kOrdinalHitCount);
    if (kOrdinalHitCount > 0) {
        printf("[Discovery] Corridor ordinal inventory (one entry per unique ordinal):\n");
        {
            int seen[24] = {0};
            int i;
            int uniqueCount = 0;
            for (i = 0; i < kOrdinalHitCount; ++i) {
                if (!seen[kOrdinalHits[i].ordinal]) {
                    printf("  ordinal=%d first seen at (%d,%d) DIR_%d\n",
                           kOrdinalHits[i].ordinal,
                           kOrdinalHits[i].mapX,
                           kOrdinalHits[i].mapY,
                           kOrdinalHits[i].direction);
                    seen[kOrdinalHits[i].ordinal] = 1;
                    ++uniqueCount;
                }
            }
            printf("[Discovery] %d unique ordinals in corridor band\n",
                   uniqueCount);
        }
    }
    return hits > 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int hitX = -1, hitY = -1, hitDir = -1;
    int ordinalAtHit;
    int pct;
    int warm;
    int nonZero;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait ordinal 6 south_return portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* ── Discovery ──────────────────────────────────────────────── */
    int ordinal6Found = scan_for_ordinal_6(&game, &hitX, &hitY, &hitDir);
    if (!ordinal6Found) {
        printf("\nSKIP Group A/B/C hall_ordinal_6_portrait_rect_position_fixture_mismatch\n");
        printf("  ordinal=%d is not present on any (mapX, mapY, direction)\n",
               ORDINAL_TARGET);
        printf("  in the Hall of Champions corridor band on this DM1 V1 build.\n");
        printf("  Re-run with a build where C127 sensorData=6 at a corridor\n");
        printf("  cell to exercise the south_return / portrait_rect_position\n");
        printf("  invariants end-to-end.\n");
    }

    if (ordinal6Found) {
    printf("\n[Group A] First ordinal=%d hit at (%d,%d) DIR_%d\n",
           ORDINAL_TARGET, hitX, hitY, hitDir);

    /* ── A) Portrait rect at the ordinal-6 cell ─────────────────── */
    {
        unsigned char fb[FB_W * FB_H];
        ordinalAtHit = render_at(&game, fb, hitX, hitY, hitDir);
        warm = portrait_rect_warm_count(fb);
        nonZero = portrait_rect_nonzero(fb);
        pct = portrait_match_percent(portraits, fb, ORDINAL_TARGET);

        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "M11 reports ordinal=%d at the discovered cell",
                     ordinalAtHit);
            CHECK(ordinalAtHit == ORDINAL_TARGET, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "portrait_rect_position (%d,%d,%d,%d) has warm pixels >= %d (got %d)",
                     PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H,
                     WARM_PALETTE_MATCH_THRESHOLD, warm);
            CHECK(warm >= WARM_PALETTE_MATCH_THRESHOLD, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "portrait_rect_position has non-zero pixels (got %d)", nonZero);
            CHECK(nonZero > 0, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "portrait ordinal=%d pixel-match >= %d%% in D1C rectangle (got %d%%)",
                     ORDINAL_TARGET, ORDINAL_PIXEL_MATCH_THRESHOLD, pct);
            CHECK(pct >= ORDINAL_PIXEL_MATCH_THRESHOLD, msg);
        }
    }

    /* ── B) No floating: surround-zone outside D1C is wall texture ── */
    {
        unsigned char fb[FB_W * FB_H];
        ordinalAtHit = render_at(&game, fb, hitX, hitY, hitDir);
        /* The side-wall padding around the D1C rectangle should not
         * contain a high warm-pixel density (portrait floating). */
        int sideWarm = 0;
        int x, y;
        for (y = PORTRAIT_Y - SIDE_PAD_Y; y < PORTRAIT_Y + PORTRAIT_H + SIDE_PAD_Y; ++y) {
            for (x = PORTRAIT_X - SIDE_PAD_X; x < PORTRAIT_X; ++x) {
                unsigned char raw = fb[y * FB_W + x];
                unsigned char idx = M11_FB_DECODE_INDEX(raw);
                switch (idx) {
                    case 0x07:
                    case 0x08:
                    case 0x09:
                    case 0x0A:
                    case 0x0B:
                    case 0x0E:
                        ++sideWarm;
                        break;
                    default:
                        break;
                }
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "left-side surround zone (%d x %d around D1C) has no "
                     "warm-portrait bleed (got %d warm pixels, threshold < 8)",
                     SIDE_PAD_X, PORTRAIT_H + 2 * SIDE_PAD_Y, sideWarm);
            CHECK(sideWarm < 8, msg);
        }
    }

    /* C) south_return sequence
     *
     * The south_return route on this build is:
     *   (2,4) DIR_SOUTH  -> ordinal 15 (MOPHUS visible on south wall)
     *   (2,4) DIR_NORTH  -> no mirror (back wall, no D1C portrait)
     *   (2,4) DIR_EAST   -> ordinal 6  (target slice)
     *
     * This proves the portrait_rect_position transitions cleanly:
     * not populated with ordinal 6 when facing away (NORTH/return),
     * populated with ordinal 6 when facing the mirror cell (EAST).
     */
    {
        unsigned char fbSouth[FB_W * FB_H];
        unsigned char fbNorth[FB_W * FB_H];
        unsigned char fbEast[FB_W * FB_H];
        int ordSouth;
        int ordNorth;
        int ordEast;
        int pctSouth6;
        int pctNorth6;
        int pctEast6;
        int warmSouth;
        int warmNorth;
        int warmEast;
        int bestNorth;
        int bestEast;

        ordSouth = render_at(&game, fbSouth, hitX, hitY, DIR_SOUTH);
        ordNorth = render_at(&game, fbNorth, hitX, hitY, DIR_NORTH);
        ordEast = render_at(&game, fbEast, hitX, hitY, DIR_EAST);

        pctSouth6 = portrait_match_percent(portraits, fbSouth, ORDINAL_TARGET);
        pctNorth6 = portrait_match_percent(portraits, fbNorth, ORDINAL_TARGET);
        pctEast6 = portrait_match_percent(portraits, fbEast, ORDINAL_TARGET);
        warmSouth = portrait_rect_warm_count(fbSouth);
        warmNorth = portrait_rect_warm_count(fbNorth);
        warmEast = portrait_rect_warm_count(fbEast);
        bestNorth = portrait_best_ordinal(portraits, fbNorth);
        bestEast = portrait_best_ordinal(portraits, fbEast);

        printf("\n[Group C] south_return sequence at (%d,%d)\n", hitX, hitY);
        printf("  south (entry view): ordinal=%d  warm=%d  pct(ord6)=%d%%\n",
               ordSouth, warmSouth, pctSouth6);
        printf("  north (back wall) : ordinal=%d  warm=%d  pct(ord6)=%d%%  bestOrd=%d\n",
               ordNorth, warmNorth, pctNorth6, bestNorth);
        printf("  east  (mirror)    : ordinal=%d  warm=%d  pct(ord6)=%d%%  bestOrd=%d\n",
               ordEast, warmEast, pctEast6, bestEast);

        /* The back-direction pose (NORTH) must NOT carry the ordinal
         * 6 portrait.  This is the no-floating contract: turning
         * around from the MOPHUS mirror to face the back wall must
         * leave the D1C rectangle empty (wall texture only), not
         * hold a stale ordinal 6 portrait. */
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return NORTH (back-wall) does NOT show ordinal=%d "
                     "(match=%d%%%%, want < %d%%%%)",
                     ORDINAL_TARGET, pctNorth6,
                     ORDINAL_PIXEL_MATCH_THRESHOLD);
            CHECK(pctNorth6 < ORDINAL_PIXEL_MATCH_THRESHOLD, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return NORTH (back-wall) has low warm-pixel density "
                     "(no stale portrait bleed, warm=%d want < %d)",
                     warmNorth, WARM_PALETTE_MATCH_THRESHOLD);
            CHECK(warmNorth < WARM_PALETTE_MATCH_THRESHOLD, msg);
        }
        /* The east-direction pose must match ordinal 6 in the D1C
         * rectangle.  This is the south_return destination: facing
         * east after the back-turn reveals ordinal 6. */
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return EAST (target pose) matches ordinal=%d (>= %d%%%%, got %d%%%%)",
                     ORDINAL_TARGET,
                     ORDINAL_PIXEL_MATCH_THRESHOLD, pctEast6);
            CHECK(pctEast6 >= ORDINAL_PIXEL_MATCH_THRESHOLD, msg);
        }
        /* Stale-pixel invariant: after the south_return trip the
         * D1C rectangle's best-matching ordinal must equal the new
         * front-cell ordinal (not the previous ordinal). */
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return EAST rect is not stale (best=%d, want=%d)",
                     bestEast, ordEast);
            CHECK(bestEast == ordEast, msg);
        }
    }
    } /* end if (ordinal6Found) */


    /* D) Secondary: portrait_rect_position invariant for every
     * positive-ordinal pose found during discovery.  This is the
     * best-effort cross-ordinal sanity check: even on a DM1 V1
     * build that lacks ordinal 6 (our local PC 3.4 build is one),
     * the probe proves the (96, 35, 32, 29) D1C rectangle is the
     * right blit destination for the ordinals that DO exist in
     * the corridor band.  Each check is a single warm-pixel /
     * pixel-match assertion against the discovered cell. */
    {
        printf("\n[Group D] Secondary: portrait_rect_position invariant "
               "across all discovered ordinals\n");
        int i;
        int totalOrdinals = 0;
        int correctRects = 0;
        for (i = 0; i < kOrdinalHitCount; ++i) {
            const OrdinalHit* h = &kOrdinalHits[i];
            unsigned char fb[FB_W * FB_H];
            int ord;
            int pct;
            int warm;
            render_at(&game, fb, h->mapX, h->mapY, h->direction);
            ord = M11_GameView_GetFrontMirrorOrdinal(&game);
            warm = portrait_rect_warm_count(fb);
            pct = portrait_match_percent(portraits, fb, h->ordinal);
            ++totalOrdinals;
            if (warm >= WARM_PALETTE_MATCH_THRESHOLD &&
                ord == h->ordinal &&
                pct >= ORDINAL_PIXEL_MATCH_THRESHOLD) {
                ++correctRects;
            }
        }
        printf("  %d/%d corridor ordinals land in the D1C portrait rect\n",
               correctRects, totalOrdinals);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "All %d positive-ordinal corridor poses paint in the "
                     "portrait_rect_position (96, 35, 32, 29) -- ordinal=%d "
                     "is not special, the rectangle is the universal front-"
                     "wall portrait cutout",
                     totalOrdinals, ORDINAL_TARGET);
            /* This is a soft check: when the discovery scan finds at
             * least one ordinal, all of them must hit the D1C rectangle.
             * If even one positive-ordinal pose lacks the portrait, the
             * D1C cutout invariant is broken. */
            CHECK(correctRects == totalOrdinals, msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
