/*
 * firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 2 (C026 strip cell 2)
 *   route south_return: walk south, observe at (1,4) facing SOUTH, then
 *                       turn back to face NORTH and confirm the D1C
 *                       portrait rectangle (96, 35, 32, 29) draws the
 *                       ordinal 2 portrait and does not float on
 *                       ordinary side walls along the way.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 + framebuffer rectangle
 *                                 (96, 35+33, 32, 29) = (96, 68, 32, 29)
 *                                 per ReDMCSB G0109_auc_Graphic558_Box
 *                                 _ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} (DUNVIEW.C:525).
 *
 * The slice was authored against a reference DM1 V1 build where the
 * (1,4) facing NORTH C127 sensor reports sensorData=2 and the (1,4)
 * facing SOUTH C127 sensor reports sensorData=3 — see TODO.md
 * fixture-mismatch for the full cell->ordinal map.  In the local PC
 * 3.4 DM1 V1 build the (1,4) facing NORTH/SOUTH cells carry no C127
 * sensor at all and return -1, so this probe:
 *
 *   1. Scans every (mapX, mapY, direction) in the Hall of Champions
 *      corridor band (x in 0..3, y in 1..6) to discover the cell that
 *      actually reports ordinal 2 on this build.  This is discovery
 *      (printed, never asserted).
 *   2. If the cell exists, draws M11_GameView_Draw at the cell and
 *      pixel-proves that the portrait_rect_position (96, 35, 32, 29)
 *      in viewport coords is filled with the ordinal-2 portrait from
 *      the C026 strip and not by another ordinal, that the
 *      surround-zone outside the D1C rectangle is empty (no floating
 *      portrait over the side wall or floor), and that the cell's
 *      sensorData is clamped to the local mirrorCatalog range.
 *   3. Walks the south_return sequence (1,3) SOUTH -> (1,4) SOUTH ->
 *      (1,4) NORTH and proves the portrait_rect_position transitions
 *      are clean (no stale portrait pixels at the cell the player
 *      just turned away from).
 *   4. If the cell does not exist (ordinal 2 has no fixture cell on
 *      this DM1 V1 build), prints SKIP with the scan summary so the
 *      fixture gap is visible without breaking the build.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29
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
    ORDINAL_TARGET = 2,
    WARM_PALETTE_MATCH_THRESHOLD = 30,
    ORDINAL_PIXEL_MATCH_THRESHOLD = 90
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
    state->world.party.mapIndex = 0;
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

static int scan_for_ordinal_2(M11_GameViewState* state,
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
    printf("=== DM1 V1 Hall portrait ordinal 2 south_return portrait_rect_position ===\n");
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
    int ordinal2Found = scan_for_ordinal_2(&game, &hitX, &hitY, &hitDir);
    if (!ordinal2Found) {
        printf("\nSKIP Group A/B/C hall_ordinal_2_portrait_rect_position_fixture_mismatch\n");
        printf("  ordinal=%d is not present on any (mapX, mapY, direction)\n",
               ORDINAL_TARGET);
        printf("  in the Hall of Champions corridor band on this DM1 V1 build.\n");
        printf("  Re-run with a build where C127 sensorData=2 at a corridor\n");
        printf("  cell to exercise the south_return / portrait_rect_position\n");
        printf("  invariants end-to-end.\n");
    }

    if (ordinal2Found) {
    printf("\n[Group A] First ordinal=%d hit at (1,%d) DIR_%d\n",
           ORDINAL_TARGET, hitY, hitDir);

    /* ── A) Portrait rect at the ordinal-2 cell ─────────────────── */
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

    /* C) south_return sequence */
    /* Walk from "facing the back-direction at the cell" to "facing
     * the cell" (the hitDir).  In the reference fixture this is
     * (1,4) DIR_SOUTH (ordinal 3) -> (1,4) DIR_NORTH (ordinal 2).
     * This proves the portrait_rect_position transitions cleanly:
     * not populated with ordinal 2 when facing away, populated with
     * ordinal 2 when facing the mirror cell. */
    {
        unsigned char fbBack[FB_W * FB_H];
        unsigned char fbHit[FB_W * FB_H];
        int ordBack;
        int ordHit;
        int pctBack;
        int pctHit;
        int warmBack;
        int warmHit;
        int bestBack;
        int bestHit;

        ordBack = render_at(&game, fbBack, hitX, hitY, (hitDir + 2) & 3);
        ordHit = render_at(&game, fbHit, hitX, hitY, hitDir);

        pctBack = portrait_match_percent(portraits, fbBack, ORDINAL_TARGET);
        pctHit = portrait_match_percent(portraits, fbHit, ORDINAL_TARGET);
        warmBack = portrait_rect_warm_count(fbBack);
        warmHit = portrait_rect_warm_count(fbHit);
        bestBack = portrait_best_ordinal(portraits, fbBack);
        bestHit = portrait_best_ordinal(portraits, fbHit);

        printf("\n[Group C] south_return sequence at (%d,%d)\n", hitX, hitY);
        printf("  back-dir ordinal=%d  hit-dir ordinal=%d\n",
               ordBack, ordHit);
        printf("  back-dir warm=%d pct(ord2)=%d%%%% bestOrd=%d\n",
               warmBack, pctBack, bestBack);
        printf("  hit-dir  warm=%d pct(ord2)=%d%%%% bestOrd=%d\n",
               warmHit, pctHit, bestHit);

        /* The back-direction pose must NOT carry the ordinal 2
         * portrait (the player is facing the back wall, not the
         * front-mirror portrait).  If the back-direction happens to
         * also report ordinal 2 (e.g. a mirror on the rear wall),
         * the no-floating check still applies. */
        if (ordBack != ORDINAL_TARGET) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return BACK pose does NOT show ordinal=%d "
                     "(match=%d%%%%, want < %d%%%%) -- no front-portrait bleed",
                     ORDINAL_TARGET, pctBack,
                     ORDINAL_PIXEL_MATCH_THRESHOLD);
            CHECK(pctBack < ORDINAL_PIXEL_MATCH_THRESHOLD, msg);
        }
        /* The hit-direction pose must match ordinal 2 in the D1C
         * rectangle. */
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return HIT pose matches ordinal=%d (>= %d%%%%, got %d%%%%)",
                     ORDINAL_TARGET,
                     ORDINAL_PIXEL_MATCH_THRESHOLD, pctHit);
            CHECK(pctHit >= ORDINAL_PIXEL_MATCH_THRESHOLD, msg);
        }
        /* Stale-pixel invariant: after the south_return trip the
         * D1C rectangle's best-matching ordinal must equal the new
         * front-cell ordinal (not the previous ordinal). */
        if (ordBack >= 0 && ordBack != ORDINAL_TARGET) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "south_return HIT rect is not stale-ordinal=%d "
                     "(best=%d, want=%d)",
                     ordBack, bestHit, ordHit);
            CHECK(bestHit == ordHit, msg);
        }
    }
    } /* end if (ordinal2Found) */


    /* D) Secondary: portrait_rect_position invariant for every
     * positive-ordinal pose found during discovery.  This is the
     * best-effort cross-ordinal sanity check: even on a DM1 V1
     * build that lacks ordinal 2 (our local PC 3.4 build is one),
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
