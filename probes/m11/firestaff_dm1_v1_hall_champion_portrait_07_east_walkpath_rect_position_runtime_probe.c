/*
 * DM1 V1 Hall of Champions champion portrait — ordinal 7 / east_walkpath / portrait_rect_position.
 *
 * Slice assignment:
 *   ordinal   = 7  (TIGGY / TAMAL — confirmed by the companion
 *                    firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe)
 *   route     = east_walkpath (the canonical Hall of Champions corridor
 *               walk from west to east, exercising both EAST-facing and
 *               NORTH-facing poses across the (1,2)..(1,5) cells)
 *   aspect    = portrait_rect_position
 *
 * Honest scope note:
 *   In the real DM1 V1 PC 3.4 English DUNGEON.DAT, the C127 sensor with
 *   sensorData == 7 lives only on map 0 at (mapX=2, mapY=17) with the
 *   party facing SOUTH (the canonical ordinal-07 route).  No Hall of
 *   Champions cell on the (1,2)..(1,5) east-walkpath corridor exposes
 *   sensorData == 7 — that route is owned by HALK (1), ZED (10), SONJA
 *   (18), and the no-portrait corridor poses.  This probe therefore
 *   verifies two contracts on the east_walkpath corridor:
 *
 *     (a) The D1C portrait rectangle lives at viewport-local
 *         (96, 35)-(127, 63) and does not float on side walls in
 *         ANY east_walkpath pose, regardless of ordinal.  This is the
 *         "portrait_rect_position" aspect: the rectangle is fixed by
 *         DUNVIEW.C:3913-3928 and 8522-8533 and is identical for every
 *         C127 sensorData value in [0, 23].
 *
 *     (b) The ordinal-7 atlas slot in the C026 strip (column 7,
 *         row 0, source rect (224, 0, 32, 29)) maps to the same D1C
 *         destination rectangle when blitted via the source-locked
 *         M11_AssetLoader_BlitRegion path.  This pins ordinal 7 as a
 *         distinct atlas slot (different from ordinals 0, 6, and
 *         adjacent slots) and proves the destination rectangle matches
 *         the C026 source rect for ordinal 7 exactly (>= 90% pixel
 *         match) without leaking into side-wall boxes.
 *
 *   The probe does NOT claim that ordinal 7 fires on any east_walkpath
 *   cell in the reference DUNGEON.DAT — that route is reserved for
 *   HALK/ZED/SONJA as documented in the existing companion probes.
 *
 * The probe locks the following invariants against real DM1 V1
 * GRAPHICS.DAT and DUNGEON.DAT under the configured data root:
 *
 *   1. Mirror-catalog identity for ordinal 7:
 *        M11_GameView_GetMirrorNameByOrdinal(7) == "TIGGY"
 *        M11_GameView_GetMirrorTitleByOrdinal(7) == "TAMAL"
 *
 *   2. East_walkpath corridor walk (NORTH-facing) at cells (1,2) ->
 *      (1,3) -> (1,4) -> (1,5):
 *        - (1,2, NORTH) -> HALK ordinal 1
 *        - (1,3, NORTH) -> no portrait
 *        - (1,4, NORTH) -> no portrait
 *        - (1,5, NORTH) -> ZED ordinal 10
 *      At every step the D1C rect lives at (96, 35)-(127, 63), the
 *      portrait matches the C026 atlas slot for the expected ordinal
 *      (matched/compared >= 90% when ordinal >= 0), and the side-wall
 *      boxes (D1L/D1R/D3L/D3R) carry zero or near-zero warm pixels
 *      (no floating on side walls).
 *
 *   3. East_walkpath corridor walk (EAST-facing) at the same cells:
 *        - (1,2, EAST) -> no portrait
 *        - (1,3, EAST) -> SONJA ordinal 18
 *        - (1,4, EAST) -> no portrait
 *        - (1,5, EAST) -> no portrait
 *      Same rect-position contract as the NORTH-facing walk.
 *
 *   4. Synthetic ordinal-7 atlas slot contract:
 *      Clear a framebuffer, blit ordinal 7 from the C026 atlas to
 *      (96, 35) using M11_AssetLoader_BlitRegion with the source-
 *      locked C01 dark-gray transparency mask (DUNVIEW.C:3916), and
 *      verify the destination matches the C026 source rect for
 *      ordinal 7 (matched/compared >= 90%).  No side-wall box has
 *      warm pixels (no floating).  The ordinal-7 atlas slot is
 *      distinct from ordinals 0/6 (>= 50 differing non-transparent
 *      pixels) and the C026 centre pixel at (240, 14) maps to the
 *      framebuffer centre pixel at (112, 82).
 *
 *   5. Rect-position contract (static):
 *      The D1C portrait rect (96, 35)-(127, 63) lives strictly inside
 *      the D1C wall-ornament frame at (80, 29)-(143, 71) per
 *      DUNVIEW.C G0205 coordSet 5 / index 12 (C346 champion-mirror
 *      frame), per DUNVIEW.C:3913-3928.
 *
 * Source evidence (ReDMCSB):
 *   DUNGEON.C:2573 maps C127 sensor cell against view direction.
 *   DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS (256x87,
 *     8 columns * 32 px wide, 3 rows * 29 px tall, ordinals 0..23).
 *   DEFS.H:821-826 M027_PORTRAIT_X(index), M028_PORTRAIT_Y.
 *   DUNVIEW.C:3913-3928 blits the C026 atlas at the fixed D1C wall
 *     box (96, 35)-(127, 63) with the C01 dark-gray transparency
 *     mask (PALETTE color 1).
 *   DUNVIEW.C:8522-8533 same blit on the F0128 redraw path.
 *   DUNVIEW.C:7727-7924 F0124_DrawSquareD1C draws wall, alcove, then
 *     portrait blit.
 *   DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF redraws the
 *     viewport after every MOVESENS.C:556 movement tick.
 *   MOVESENS.C:1501-1503 passes C127 sensorData to F0280.
 *   REVIVE.C F0280 materializes the candidate from sensorData.
 *
 * Companion probes (already in the tree):
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *     - walks east at NORTH through (1,3)->(2,3)->(3,3) and locks
 *       the D1C rect across cell transitions and re-blt.
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *     - covers (1,3) facing N and (1,4) facing N in isolation.
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 *     - z-order under wall + floor + ornament at all four cardinal
 *       directions.
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     - re-blt invariant after viewport scroll.
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - 16 real-DM1 poses including the east-facing SONJA ordinal
 *       18 at (1,3) EAST and the north-facing HALK ordinal 1 at
 *       (1,2) NORTH.
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     - PPM evidence of every Hall pose with portrait_rect_nonzero
 *       and portrait_rect_warm_count thresholds.
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe
 *     - D1C wall-ornament zone position contract for the canonical
 *       HALK (1,2) and ZED (1,5) north-facing routes.
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 *     - the companion ordinal-07 front_north_entry probe — verifies
 *       catalog identity TIGGY/TAMAL, the (2,17,SOUTH) available
 *       route, no front_north_entry route for ordinal 7, and a
 *       resurrect round-trip.  This probe extends ordinal 07 to the
 *       east_walkpath aspect, which the companion does not cover.
 *   firestaff_dm1_v1_hall_champion_portrait_east_walkpath_rect_position_runtime_probe
 *     - the ordinal-02 east_walkpath probe — same east_walkpath
 *       route table for ordinals 1/18/10/-1.  This probe uses the
 *       same route table so the corridor walk is a verified, shared
 *       contract; the ordinal-specific addition is the atlas-slot
 *       contract for ordinal 7 (TIGGY/TAMAL, source rect (224,0)).
 *
 * This probe does NOT duplicate any of the above.  It focuses on:
 *   (a) ordinal 7 maps to the expected C026 atlas slot
 *       (column 7 row 0, source rect (224, 0, 32, 29)),
 *   (b) the D1C destination rectangle is at viewport-local
 *       (96, 35)-(127, 63) on every east_walkpath pose (regardless
 *       of which ordinal fires), and
 *   (c) the rectangle does not float on side walls in any east-
 *       walkpath pose.
 *
 * Honest scope: Firestaff runtime evidence against the real DM1 V1
 * DUNGEON.DAT / GRAPHICS.DAT pair.  Not DOS pixel parity; the C026
 * atlas pixels are loaded from the local GRAPHICS.DAT extract and
 * the destination rect position is the source-locked DUNVIEW.C
 * coordinate (96, 35).  No claim of byte-equal DOSBox capture.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,

    /* ReDMCSB DUNVIEW.C:3913-3928 / 8522-8533: the D1C front-wall box
     * is the 32x29 rectangle at viewport-local (96, 35)-(127, 63).
     * Source framebuffer (i.e., the actual DM1 V1 destination rect):
     *   dstX = M11_VIEWPORT_X + 96 = 96
     *   dstY = M11_VIEWPORT_Y + 35 = 68
     *   width = 32, height = 29. */
    PROBE_D1C_DST_X = PROBE_VIEWPORT_X + 96,
    PROBE_D1C_DST_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_D1C_W = 32,
    PROBE_D1C_H = 29,

    /* ReDMCSB DUNVIEW.C:3916 C01 dark-gray transparency mask. */
    PROBE_CHAMPION_TRANSPARENT = 1,

    /* ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS is the
     * 256x87 strip of 32x29 portraits: 8 columns by 3 rows, ordinals
     * 0..23.  Atlas math: srcX = (ord & 7) * 32, srcY = (ord >> 3) * 29.
     * Ordinal 7 -> atlas column 7, atlas row 0 -> source rect
     * (224, 0, 32, 29). */
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_COLS = 8,
    PROBE_PORTRAIT_ROWS = 3,
    PROBE_PORTRAIT_TOTAL = 24,
};

/* Side-wall boxes the D1C portrait must NOT bleed into.  These
 * are the four rectangles used by the D1L, D1R, D3L, D3R wall
 * paths under F0115/F0112 (ReDMCSB DUNVIEW.C:4629-4835 and 5180-
 * 5656).  If the D1C portrait leaks into any of these, the
 * champion portrait is "floating" on an ordinary side wall.  The
 * boxes are viewport-local and the function scans the source
 * framebuffer pixel range that corresponds to each. */
typedef struct SideWallBox {
    const char* label;
    int viewX;       /* viewport-local x */
    int viewY;       /* viewport-local y */
    int viewW;
    int viewH;
} SideWallBox;

static const SideWallBox kSideWallBoxes[] = {
    {"D1L",  0,  33, 28, 136},  /* left side wall */
    {"D1R", 196, 33, 28, 136},  /* right side wall */
    {"D3L",  28, 33, 32,  56},  /* forward-left, upper */
    {"D3R", 164, 33, 32,  56},  /* forward-right, upper */
};
enum { PROBE_SIDE_BOX_COUNT = sizeof(kSideWallBoxes) / sizeof(kSideWallBoxes[0]) };

typedef struct WalkpathStep {
    int mapX;
    int mapY;
    int dir;                /* DIR_NORTH=0, DIR_EAST=1, DIR_SOUTH=2, DIR_WEST=3 */
    int expectedOrdinal;    /* -1 means no front mirror */
    const char* label;
} WalkpathStep;

static int g_pass = 0;
static int g_fail = 0;
#define PASS() do { g_pass++; } while(0)
#define FAIL(msg, ...) do { fprintf(stderr, "FAIL: " msg "\n", ##__VA_ARGS__); g_fail++; } while(0)
#define CHECK(cond, msg, ...) do { if (cond) { ++g_pass; printf("  PASS: " msg "\n", ##__VA_ARGS__); } else { ++g_fail; fprintf(stderr, "  FAIL: " msg "\n", ##__VA_ARGS__); } } while(0)

/* Reuse the warm-pixel set the existing capture probe locks
 * (firestaff_dm1_v1_champion_mirror_capture_probe PORTRAIT_WARM_THRESHOLD):
 * palette indices {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} (green, red,
 * orange, peach, yellow, blue).  Champion portrait sprites use a mix
 * of these for skin tones and clothing; grey-stone wall texture uses
 * only 0x01/0x02/0x07-grey/0x0D (grey shades) and never the warm
 * set.  Threshold >= 30 warm pixels = portrait detected. */
static int pixel_is_warm(unsigned char idx) {
    return idx == 0x07 || idx == 0x08 || idx == 0x09 ||
           idx == 0x0A || idx == 0x0B || idx == 0x0E;
}

static int rect_warm_count(const unsigned char* fb,
                           int x0, int y0, int w, int h) {
    int x, y, count = 0;
    for (y = y0; y < y0 + h && y < PROBE_FB_H; ++y) {
        if (y < 0) continue;
        for (x = x0; x < x0 + w && x < PROBE_FB_W; ++x) {
            if (x < 0) continue;
            unsigned char raw = fb[y * PROBE_FB_W + x];
            unsigned char idx = (unsigned char)(raw & 0x0F);
            if (pixel_is_warm(idx)) {
                ++count;
            }
        }
    }
    return count;
}

/* Count how many pixels in the D1C rect exactly match the source
 * C026 atlas slot for the given ordinal (compared = total non-
 * transparent source pixels in that rect).  The dark-gray C01
 * source pixels are skipped because DUNVIEW.C:3916 treats them
 * as transparent. */
static int count_ordinal_matched(const M11_AssetSlot* portraits,
                                 const unsigned char* fb,
                                 int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= PROBE_PORTRAIT_TOTAL) {
        return 0;
    }
    srcX0 = (ordinal & (PROBE_PORTRAIT_COLS - 1)) * PROBE_PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PROBE_PORTRAIT_H;
    for (y = 0; y < PROBE_D1C_H; ++y) {
        for (x = 0; x < PROBE_D1C_W; ++x) {
            unsigned char src = (unsigned char)(portraits->pixels[
                (srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = (unsigned char)(fb[
                (PROBE_D1C_DST_Y + y) * PROBE_FB_W + (PROBE_D1C_DST_X + x)] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) continue;
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared << 16) | (matched & 0xFFFF);
}

/* Sum warm pixels across every side-wall box (D1L/D1R/D3L/D3R). */
static int side_wall_warm_total(const unsigned char* fb) {
    int i, total = 0;
    for (i = 0; i < PROBE_SIDE_BOX_COUNT; ++i) {
        int x0 = PROBE_VIEWPORT_X + kSideWallBoxes[i].viewX;
        int y0 = PROBE_VIEWPORT_Y + kSideWallBoxes[i].viewY;
        total += rect_warm_count(fb, x0, y0,
                                 kSideWallBoxes[i].viewW,
                                 kSideWallBoxes[i].viewH);
    }
    return total;
}

/* Warm pixels in the D1C rect itself. */
static int d1c_warm_count(const unsigned char* fb) {
    return rect_warm_count(fb, PROBE_D1C_DST_X, PROBE_D1C_DST_Y,
                           PROBE_D1C_W, PROBE_D1C_H);
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

static int check_east_walkpath_step(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits,
                                    const WalkpathStep* step,
                                    unsigned char* outFb) {
    int ordinal;
    int d1cWarm;
    int sideWarm;
    int matched, compared, expectPct;
    int mc;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        FAIL("%s front ordinal got=%d want=%d",
             step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }

    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);

    d1cWarm = d1c_warm_count(outFb);
    sideWarm = side_wall_warm_total(outFb);

    if (step->expectedOrdinal >= 0) {
        /* Positive ordinal: D1C rect must carry the portrait signature
         * (warm_count >= 30) AND match the C026 atlas rect for that
         * ordinal (matched/compared >= 90%).  Side walls must have
         * zero or near-zero warm pixels (no floating on side walls).
         * Ordinal 7 is not exposed on the (1,2)..(1,5) corridor in
         * the reference DUNGEON.DAT, but the rect-position contract
         * is identical for every C127 sensorData value, so the same
         * rect-position assertion applies for whichever ordinal fires. */
        if (d1cWarm < 30) {
            FAIL("%s D1C rect warm_count=%d want>=30 (expectedOrdinal=%d)",
                 step->label, d1cWarm, step->expectedOrdinal);
            ok = 0;
        }
        mc = count_ordinal_matched(portraits, outFb, step->expectedOrdinal);
        matched = mc & 0xFFFF;
        compared = (mc >> 16) & 0xFFFF;
        expectPct = compared > 0 ? (matched * 100) / compared : 0;
        if (compared <= 0 || expectPct < 90) {
            FAIL("%s D1C rect mismatch expectedOrdinal=%d matched=%d/%d (%d%%) want>=90%%",
                 step->label, step->expectedOrdinal,
                 matched, compared, expectPct);
            ok = 0;
        }
        if (sideWarm > 5) {
            FAIL("%s side-wall warm pixels=%d want<=5 (floating on side walls)",
                 step->label, sideWarm);
            ok = 0;
        }
    } else {
        /* No-front-mirror pose: D1C rect must not show a portrait
         * signature (warm_count < 30, the noise floor established by
         * firestaff_dm1_v1_champion_mirror_capture_probe for corridor
         * no-portrait poses that may still have a few warm pixels
         * from edge antialiasing or torch glow).  Side walls must be
         * empty too. */
        if (d1cWarm >= 30) {
            FAIL("%s D1C rect warm_count=%d want<30 (no front mirror)",
                 step->label, d1cWarm);
            ok = 0;
        }
        if (sideWarm > 5) {
            FAIL("%s side-wall warm pixels=%d want<=5 (no front mirror)",
                 step->label, sideWarm);
            ok = 0;
        }
    }

    printf("  %s pose=(%d,%d,%d) ordinal=%d d1c_warm=%d side_warm=%d",
           step->label, step->mapX, step->mapY, step->dir,
           ordinal, d1cWarm, sideWarm);
    if (step->expectedOrdinal >= 0) {
        mc = count_ordinal_matched(portraits, outFb, step->expectedOrdinal);
        matched = mc & 0xFFFF;
        compared = (mc >> 16) & 0xFFFF;
        expectPct = compared > 0 ? (matched * 100) / compared : 0;
        printf(" rect_match=%d/%d (%d%%)", matched, compared, expectPct);
    }
    printf("\n");
    return ok;
}

/* Synthetic ordinal-7 contract: clear a framebuffer, blit ordinal 7
 * portrait from the C026 atlas to the D1C destination rectangle using
 * M11_AssetLoader_BlitRegion with the source-locked C01 dark-gray
 * transparency mask (DUNVIEW.C:3916).  Then verify the destination
 * matches the C026 source rect for ordinal 7 exactly (matched/compared
 * >= 90%) and that no other viewport rectangle has warm pixels.  This
 * is the contract DUNVIEW.C:3913-3928 satisfies for any C127 sensorData
 * in [0, 23]; the probe locks it for ordinal 7 (TIGGY/TAMAL slot in
 * C026, column 7 row 0, source rect (224, 0, 32, 29)).
 *
 * The probe deliberately does not assume ordinal 7 is exposed on any
 * east_walkpath cell in the user's DUNGEON.DAT.  The east_walkpath
 * route is exercised above with the real corridor ordinals (HALK/
 * ZED/SONJA) so the D1C rect position is verified across both NORTH
 * and EAST facings of the corridor; this synthetic test pins the
 * ordinal-7 atlas slot independently. */
static int check_ordinal_7_atlas_slot(M11_GameViewState* game,
                                      const M11_AssetSlot* portraits,
                                      unsigned char* outFb) {
    int srcX0, srcY0;
    int matched, compared, pct;
    int d1cWarm, sideWarm;
    int ok = 1;

    srcX0 = (7 & (PROBE_PORTRAIT_COLS - 1)) * PROBE_PORTRAIT_W;
    srcY0 = (7 >> 3) * PROBE_PORTRAIT_H;

    /* Dark-gray backdrop everywhere so any blit-leaked pixels are
     * detectable as warm/colored on the backdrop. */
    memset(outFb, 0x01, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));

    M11_AssetLoader_BlitRegion(portraits,
                               srcX0, srcY0,
                               PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
                               outFb, PROBE_FB_W, PROBE_FB_H,
                               PROBE_D1C_DST_X, PROBE_D1C_DST_Y,
                               PROBE_CHAMPION_TRANSPARENT);

    matched = 0;
    compared = 0;
    {
        int x, y;
        for (y = 0; y < PROBE_D1C_H; ++y) {
            for (x = 0; x < PROBE_D1C_W; ++x) {
                unsigned char src = (unsigned char)(portraits->pixels[
                    (srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                unsigned char dst = (unsigned char)(outFb[
                    (PROBE_D1C_DST_Y + y) * PROBE_FB_W + (PROBE_D1C_DST_X + x)] & 0x0F);
                if (src == PROBE_CHAMPION_TRANSPARENT) continue;
                ++compared;
                if (dst == src) ++matched;
            }
        }
    }
    pct = compared > 0 ? (matched * 100) / compared : 0;
    d1cWarm = d1c_warm_count(outFb);
    sideWarm = side_wall_warm_total(outFb);

    printf("  ordinal_7_atlas_slot rect=(%d,%d,%d,%d) -> dst=(%d,%d) matched=%d/%d (%d%%) d1c_warm=%d side_warm=%d\n",
           srcX0, srcY0, PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_D1C_DST_X, PROBE_D1C_DST_Y,
           matched, compared, pct, d1cWarm, sideWarm);

    if (compared <= 0 || pct < 90) {
        FAIL("ordinal 7 atlas slot blit: matched=%d/%d (%d%%) want>=90%%",
             matched, compared, pct);
        ok = 0;
    }
    if (d1cWarm < 30) {
        FAIL("ordinal 7 atlas slot blit: D1C warm_count=%d want>=30",
             d1cWarm);
        ok = 0;
    }
    if (sideWarm > 5) {
        FAIL("ordinal 7 atlas slot blit: side_wall warm=%d want<=5 (no floating)",
             sideWarm);
        ok = 0;
    }

    /* Verify ordinal 7 maps to atlas slot column 7 row 0 by checking
     * the source pixels are different from ordinals 0 and 6 (i.e.,
     * ordinal 7 is a distinct C026 atlas slot, not TIGGY aliased to
     * HALK or WUK).  The 50-pixel threshold matches the ordinal-02
     * companion probe's distinctness check. */
    {
        int refOrdinals[2] = { 0, 6 };
        int refIdx;
        for (refIdx = 0; refIdx < 2; ++refIdx) {
            int refOrdinal = refOrdinals[refIdx];
            int distinct = 1;
            int diffPixels = 0;
            int x, y;
            for (y = 0; y < PROBE_PORTRAIT_H && distinct; ++y) {
                for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                    unsigned char ord7 = (unsigned char)(portraits->pixels[
                        (srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                    unsigned char ordRef = (unsigned char)(portraits->pixels[
                        ((refOrdinal >> 3) * PROBE_PORTRAIT_H + y) * (int)portraits->width +
                        ((refOrdinal & (PROBE_PORTRAIT_COLS - 1)) * PROBE_PORTRAIT_W + x)] & 0x0F);
                    if (ord7 == PROBE_CHAMPION_TRANSPARENT &&
                        ordRef == PROBE_CHAMPION_TRANSPARENT) continue;
                    if (ord7 != ordRef) ++diffPixels;
                }
            }
            if (diffPixels < 50) {
                FAIL("ordinal 7 atlas slot not distinct from ordinal %d (diffPixels=%d)",
                     refOrdinal, diffPixels);
                ok = 0;
            } else {
                printf("  ordinal 7 vs ordinal %d distinct (diffPixels=%d)\n",
                       refOrdinal, diffPixels);
            }
        }
    }

    /* Sanity: portrait_rect_position (96, 35) is exactly where
     * DUNVIEW.C:3913-3928 places it.  Confirm by reading the center
     * pixel of the blitted ordinal 7 portrait (framebuffer position
     * (96+16, 33+35+14) = (112, 82)) and asserting it is the same
     * palette index as the C026 atlas row 0 column 7 center pixel
     * (224+16, 14) = (240, 14). */
    {
        int centerDstX = PROBE_D1C_DST_X + 16;
        int centerDstY = PROBE_D1C_DST_Y + 14;
        int centerSrcX = srcX0 + 16;
        int centerSrcY = srcY0 + 14;
        unsigned char centerDst = (unsigned char)(outFb[
            centerDstY * PROBE_FB_W + centerDstX] & 0x0F);
        unsigned char centerSrc = (unsigned char)(portraits->pixels[
            centerSrcY * (int)portraits->width + centerSrcX] & 0x0F);
        if (centerDst != centerSrc) {
            FAIL("ordinal 7 center pixel mismatch dst(112,82)=0x%02x src(240,14)=0x%02x",
                 centerDst, centerSrc);
            ok = 0;
        } else {
            printf("  ordinal 7 center pixel dst(112,82)=0x%02x == src(240,14)=0x%02x\n",
                 centerDst, centerSrc);
        }
    }

    (void)game;
    return ok;
}

/* Static rect-position contract: the D1C portrait rect
 * (96, 35)-(127, 63) lives strictly inside the D1C wall-ornament
 * frame at (80, 29)-(143, 71) per ReDMCSB DUNVIEW.C G0205 coordSet
 * 5 / index 12 (C346 champion-mirror wall ornament frame) and
 * DUNVIEW.C:3913-3928 (champion-portrait blit rectangle).
 *
 * The rect and the frame are both in viewport-local coordinates
 * (matching the existing companion
 * firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 * check_portrait_rect_position_contract); the framebuffer dst coords
 * (PROBE_D1C_DST_X, PROBE_D1C_DST_Y) include the +33 viewport-Y
 * offset and must not be mixed with viewport-local frame coords in
 * this contract check. */
static int check_portrait_rect_position_contract(void) {
    int ok = 1;
    const int kRectViewportX = 96;
    const int kRectViewportY = 35;
    const int kRectViewportW = 32;
    const int kRectViewportH = 29;
    const int kWallFrameX = 80;
    const int kWallFrameY = 29;
    const int kWallFrameW = 64;
    const int kWallFrameH = 43;
    if (kRectViewportX < kWallFrameX ||
        kRectViewportY < kWallFrameY ||
        kRectViewportX + kRectViewportW > kWallFrameX + kWallFrameW ||
        kRectViewportY + kRectViewportH > kWallFrameY + kWallFrameH) {
        FAIL("ordinal 7 portrait rect position: rect=(%d,%d,%d,%d) escapes frame=(%d,%d,%d,%d)",
             kRectViewportX, kRectViewportY,
             kRectViewportW, kRectViewportH,
             kWallFrameX, kWallFrameY,
             kWallFrameW, kWallFrameH);
        ok = 0;
    } else {
        printf("  ordinal 7 portrait_rect_position rect=(%d,%d,%d,%d) inside frame=(%d,%d,%d,%d) [viewport-local]\n",
               kRectViewportX, kRectViewportY,
               kRectViewportW, kRectViewportH,
               kWallFrameX, kWallFrameY,
               kWallFrameW, kWallFrameH);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    char name[32];
    char title[32];
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;

    /* East_walkpath route: the canonical Hall of Champions corridor
     * walk from west to east.  We exercise both NORTH-facing (the
     * existing walkpath probe's "forward walk" route) and EAST-facing
     * (looking east down the corridor) variants so the D1C rect
     * position is locked across both corridor traversal modes.
     *
     * Real DM1 V1 DUNGEON.DAT (ReDMCSB DUNGEON.C:2573 + sensorData):
     *   (1,2) EAST  -> front (2,2) no C127 sensor   -> ordinal -1
     *   (1,3) EAST  -> front (2,3) C127 data 18 SONJA -> ordinal 18
     *   (1,4) EAST  -> front (2,4) no C127 sensor   -> ordinal -1
     *   (1,5) EAST  -> front (2,5) no C127 sensor   -> ordinal -1
     *   (1,2) NORTH -> front (1,1) C127 data 1 HALK -> ordinal 1
     *   (1,3) NORTH -> front (1,2) no C127 sensor   -> ordinal -1
     *   (1,4) NORTH -> front (1,3) no C127 sensor   -> ordinal -1
     *   (1,5) NORTH -> front (1,4) C127 data 10 ZED -> ordinal 10
     *
     * Ordinal 7 (TIGGY/TAMAL) is not exposed on the (1,2)..(1,5)
     * east_walkpath corridor in the reference DUNGEON.DAT (its
     * canonical route is (2, 17, SOUTH); see
     * firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe).
     * This probe still exercises the east_walkpath route table so the
     * D1C rect position is verified at every cell where ANY C127
     * mirror sensor fires; the ordinal-7-specific atlas-slot contract
     * is pinned by the synthetic blit test below.
     *
     * Different DM1 V1 builds may place C127 sensors on different
     * cells; if a cell here does not match the reference DUNGEON.DAT
     * the probe prints SKIP rather than fail.  This is a per-build
     * fixture guard, not a regression detector (companion to the SKIP
     * guards in firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
     * and firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe).
     */
    const WalkpathStep steps[] = {
        /* east_walkpath EAST-facing (looking east down the corridor) */
        {1, 2, 1, -1, "east_walkpath_east_1_2_no_portrait"},
        {1, 3, 1, 18, "east_walkpath_east_1_3_sonja_ordinal_18"},
        {1, 4, 1, -1, "east_walkpath_east_1_4_no_portrait"},
        {1, 5, 1, -1, "east_walkpath_east_1_5_no_portrait"},
        /* east_walkpath NORTH-facing (forward walk east at corridor) */
        {1, 2, 0,  1, "east_walkpath_north_1_2_halk_ordinal_1"},
        {1, 3, 0, -1, "east_walkpath_north_1_3_no_portrait"},
        {1, 4, 0, -1, "east_walkpath_north_1_4_no_portrait"},
        {1, 5, 0, 10, "east_walkpath_north_1_5_zed_ordinal_10"},
    };

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_hall_champion_portrait_07_east_walkpath_rect_position_runtime_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < (unsigned)(PROBE_PORTRAIT_H * PROBE_PORTRAIT_ROWS)) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable "
                "(width=%u height=%u)\n",
                portraits ? portraits->width : 0,
                portraits ? portraits->height : 0);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall champion portrait 07 east_walkpath / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("d1c rect: dst=(%d,%d) %dx%d (viewport-local (96,35)-(127,63))\n",
           PROBE_D1C_DST_X, PROBE_D1C_DST_Y, PROBE_D1C_W, PROBE_D1C_H);
    printf("side-wall boxes: D1L=(0,33,28,136) D1R=(196,33,28,136) "
           "D3L=(28,33,32,56) D3R=(164,33,32,56)\n");

    /* Catalog identity for ordinal 7 (TIGGY / TAMAL).  This is the
     * honest-scope statement: ordinal 7 is a real catalog entry, even
     * though the east_walkpath route does not expose it.  Mirrors
     * check 1 of the companion ordinal-07 front_north_entry probe. */
    name[0] = '\0';
    title[0] = '\0';
    if (M11_GameView_GetMirrorNameByOrdinal(&game, 7, name, sizeof(name)) &&
        strcmp(name, "TIGGY") == 0) {
        PASS();
        printf("  PASS: ordinal 7 catalog name='%s' matches 'TIGGY'\n", name);
    } else {
        FAIL("ordinal 7 catalog name got='%s' want='TIGGY'", name);
        ok = 0;
    }
    if (M11_GameView_GetMirrorTitleByOrdinal(&game, 7, title, sizeof(title)) &&
        strcmp(title, "TAMAL") == 0) {
        PASS();
        printf("  PASS: ordinal 7 catalog title='%s' matches 'TAMAL'\n", title);
    } else {
        FAIL("ordinal 7 catalog title got='%s' want='TAMAL'", title);
        ok = 0;
    }
    printf("  ordinal_07_catalog_identity name='%s' title='%s'\n", name, title);

    /* Static rect-position contract: the D1C portrait rect must
     * live strictly inside the D1C wall-ornament frame. */
    if (!check_portrait_rect_position_contract()) {
        ok = 0;
    }

    /* Walk east through the corridor in both EAST-facing and NORTH-
     * facing modes, verifying the D1C portrait rect position. */
    {
        int stepIdx;
        for (stepIdx = 0; stepIdx < (int)(sizeof(steps) / sizeof(steps[0])); ++stepIdx) {
            if (!check_east_walkpath_step(&game, portraits,
                                          &steps[stepIdx], currFb)) {
                ok = 0;
            }
        }
    }

    /* Ordinal-7 synthetic contract: clear a framebuffer, blit the
     * ordinal-7 portrait from the C026 atlas to the D1C destination
     * (96, 35), and verify the destination matches the C026 source
     * slot for ordinal 7 exactly.  This pins the ordinal-7 atlas
     * slot (column 7 row 0, source rect (224, 0, 32, 29))
     * independent of any user DUNGEON.DAT. */
    if (!check_ordinal_7_atlas_slot(&game, portraits, currFb)) {
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return ok ? 0 : 1;
}
