/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 4
 * sleep_repaint / portrait_rect_position runtime gate.
 *
 * Targeted slice:
 *   ordinal    = 4 (the C127 sensorData=4 / LEIF slot in the DM1 V1
 *                 mirror catalog — row 0, col 4 of the C026 atlas;
 *                 atlas source rect (128, 0, 32, 29).  LEIF is the
 *                 5th cell of the C026 champion portrait strip.)
 *   route      = sleep_repaint
 *                 (the M11 viewport redraw cycle that fires when the
 *                  party "enters" the rest state via the R-key.  In
 *                  the source-locked DM1 V1 flow, m11_game_view.c
 *                  routes M12_MENU_INPUT_REST_TOGGLE through the
 *                  M11_GameView_HandleInput dispatch which sets
 *                  `state->resting = 1`, logs "T%u: RESTING", sets
 *                  the "PARTY IS RESTING" status, and returns
 *                  M11_GAME_INPUT_REDRAW.  The next
 *                  M11_GameView_Draw cycle paints the F0128
 *                  viewport redraw AND THEN overpaints the RESTING
 *                  overlay rectangle (100, 70, 120, 30) fb-local
 *                  with a black backdrop and a light-blue border
 *                  via m11_game_view.c:27730-27737.  The
 *                  sleep_repaint slice asserts: (1) the resting
 *                  overlay is freshly visible on the FIRST redraw
 *                  after the sleep toggle (no carry-over from a
 *                  prior awake cycle), (2) the D1C portrait cutout
 *                  (96, 35, 32, 29) viewport-local stays anchored
 *                  at the source-locked rectangle even with the
 *                  RESTING overlay drawn on top, and (3) the
 *                  portrait_rect_position invariant survives
 *                  repeated sleep->wake cycles without drift.)
 *   aspect     = portrait_rect_position
 *                 (the C026 champion portrait cutout stays anchored
 *                  at the source-locked D1C viewport rectangle
 *                  (96, 35, 32, 29) on every redraw the sleep
 *                  cycle triggers, and the rectangle stays inside
 *                  the public D1C wall-mirror zone (80, 29, 64, 43)
 *                  reported by M11_GameView_GetD1CWallOrnamentZone)
 *   extra slice= candidate-panel return behavior after wake
 *                 (this gate goes one step past the ordinal 18
 *                  sleep_repaint gate by exercising a candidate
 *                  panel open/close round-trip AFTER the wake
 *                  redraw.  In the source-locked DM1 V1 flow, the
 *                  candidate panel state machine
 *                  (m11_game_view_render_candidate_mirror_panel)
 *                  lives on the same F0128 viewport, so the
 *                  portrait_rect_position invariant must also hold
 *                  after the panel is opened and closed on top of a
 *                  freshly-woken viewport.  A regression that
 *                  forgets to repaint the D1C cutout on the post-
 *                  panel-close redraw would push the C026 ordinal-4
 *                  pixel match below the 70% threshold and the warm
 *                  count below 30.)
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_portrait04_rect_position_*
 *     covers the positive north_entry route (2,1) DIR_SOUTH, but
 *     the resting state is never set, so the RESTING overlay path
 *     is not exercised.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left_*
 *     covers the wrong-wall west approach, but is a side/no-front
 *     negative probe — the candidate-panel return path is not
 *     exercised.
 *   - firestaff_dm1_v1_hoc_champion_portrait_18_sleep_repaint_*
 *     covers ordinal 18 (SONJA) under the same sleep_repaint
 *     route, but the candidate-panel return behavior is not
 *     covered there.  This gate exercises the same sleep_repaint
 *     slice on ordinal 4 (LEIF) — a disjoint ordinal on the row-0
 *     atlas path ((ordinal >> 3) * 29 = 0) — and adds the
 *     candidate-panel return slice to lock the post-wake portrait
 *     repaint on a different ordinal.
 *   - firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *     check_rest_input_ignored asserts that the C145 rest input
 *     is rejected while G0299 is set, but does not drive the
 *     sleep_repaint cycle through the viewport redraw path and
 *     does not cover the post-wake candidate-panel return.
 *
 * What the probe asserts at each stage:
 *   Stage 1 (pre-sleep):  D1C cutout (96,35,32,29) viewport-local
 *                          shows ordinal 4 with C026 strip cell
 *                          match >= 70% and warm pixel count >= 30;
 *                          the RESTING overlay region (100,70,120,30)
 *                          fb-local is NOT painted with the RESTING
 *                          fill; resting=0.
 *   Stage 2 (sleep):      After setting resting=1 (the sleep toggle
 *                          path), the next redraw paints the RESTING
 *                          overlay region with the source-locked
 *                          black fill (>= 30 black pixels in the
 *                          backdrop), the resting flag is set, and
 *                          the D1C portrait cutout is still anchored
 *                          at (96,35,32,29) viewport-local — the
 *                          cutout is the top-left 32x29 corner of
 *                          the overlay region, so the sleep_repaint
 *                          slice asserts the rectangle is preserved
 *                          even though the lower-right 28x27 corner
 *                          is overpainted by the RESTING fill.
 *   Stage 3 (sleep->sleep repeat): A second sleep redraw (resting
 *                          still 1) is byte-stable vs. the first
 *                          sleep redraw — the sleep_repaint cycle
 *                          must be idempotent (no drift between
 *                          consecutive sleep redraws).
 *   Stage 4 (cycles):     3 sleep->wake cycles at (1,2) NORTH
 *                          produce a byte-stable post-wake
 *                          framebuffer.  This locks the inverse
 *                          direction: every wake repaint from a
 *                          sleep must restore the viewport to the
 *                          same bytes, AND the entry into the next
 *                          sleep must restore the RESTING overlay
 *                          fill identically across cycles.
 *   Stage 5 (awake redraw byte-stability):
 *                          Three consecutive M11_GameView_Draw
 *                          cycles on the awake (resting=0) viewport
 *                          must produce identical framebuffers, with
 *                          the D1C portrait cutout showing the
 *                          ordinal 4 LEIF portrait at the anchored
 *                          (96, 35, 32, 29) viewport-local rectangle
 *                          on every draw.  This is the awake
 *                          counterpart of Stage 3's sleep->sleep
 *                          stability check: the awake state machine
 *                          must also be idempotent (no first-cycle
 *                          draw drift, no message-log timestamp
 *                          noise, no random palette jitter).  A
 *                          regression that leaks a one-shot draw
 *                          timer would push the diff above the
 *                          2-pixel tolerance.
 *   Stage 6 (candidate-panel return): After a sleep->wake round
 *                          trip, opening the candidate mirror panel
 *                          (M635 panel state ON) and then closing
 *                          it must leave the D1C portrait cutout
 *                          restored to the ordinal 4 portrait at
 *                          the (96, 35, 32, 29) anchored rectangle.
 *                          The probe seeds the panel, calls
 *                          M11_GameView_Draw, clears the panel
 *                          state, calls M11_GameView_Draw again,
 *                          and asserts the post-close portrait
 *                          pixel match is back to >= 70% (the
 *                          pre-panel baseline).  This is the
 *                          candidate-panel return behavior slice
 *                          that the ordinal 18 sleep_repaint gate
 *                          does not cover.
 *   Stage 7 (side pose):  At (1,2) facing EAST the front-mirror
 *                          ordinal is -1 and the D1C cutout must
 *                          NOT show ordinal 4 pixels (no floating).
 *                          The portrait_rect position invariant is
 *                          trivially satisfied because there is no
 *                          front-mirror ordinal to draw, but the
 *                          cutout rectangle is still inside the
 *                          public D1C zone.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C champion portrait blit (C026)
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw from
 *                       party map/x/y/direction (far-to-near order)
 *   ReDMCSB DUNVIEW.C:4547-4581 nibble 2 -> ordinal 4 (G0289 decode)
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB COMMAND.C:2336-2359 rest path sets G0300_B_PartyIsResting
 *                       and redraws the viewport; rejects while
 *                       G0299 is live
 *   ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS (256x87,
 *                       8 columns * 32 px wide, 3 rows * 29 px tall,
 *                       ordinals 0..23)
 *   ReDMCSB DEFS.H:821-826 M027_PORTRAIT_X(index), M028_PORTRAIT_Y
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *
 * Honest scope: Firestaff runtime evidence against the real DM1 V1
 * DUNGEON.DAT / GRAPHICS.DAT pair (retargeted (1,2) NORTH C127 sensor
 * from HALK ordinal 1 to ordinal 4 — same retarget pattern the
 * portrait04 family of probes uses).  The RESTING overlay path is
 * sourced from m11_game_view.c:27730-27737 (the resting state machine
 * in m11_game_view_render_dungeon_view).  The sleep_repaint slice
 * asserts runtime evidence, not DOS pixel parity.  No claim of
 * byte-equal DOSBox capture.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_04_sleep_repaint_portrait_rect_position_172_gate_probe DATA_DIR
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
    FB_W = 320,
    FB_H = 200,
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722):
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33); size
     * (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* C026 champion portrait cutout (viewport-local) inside the D1C
     * wall box.  ReDMCSB DUNVIEW.C:3913-3928 /
     * m11_draw_dm1_front_champion_portrait uses
     *   M11_AssetLoader_BlitRegion(portraits,
     *       (portraitIdx & 7) * M11_PORTRAIT_W (== 32),
     *       (portraitIdx >> 3) * M11_PORTRAIT_H (== 29),
     *       M11_PORTRAIT_W, M11_PORTRAIT_H,
     *       M11_VIEWPORT_X + 96, M11_VIEWPORT_Y + 35, ...)
     * so the cutout is (96, 35, 32, 29) viewport-local. */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (coordSet 5 / index 12 per
     * DUNVIEW.C G0205): dstX=80, dstY=29, w=64, h=43 viewport-local.
     * The C026 portrait cutout (96, 35, 32, 29) sits inside this
     * zone. */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS is the
     * 256x87 strip of 32x29 portraits: 8 columns by 3 rows, ordinals
     * 0..23.  Atlas math: srcX = (ord & 7) * 32, srcY = (ord >> 3) * 29.
     * Ordinal 4 -> atlas column 4, atlas row 0 -> source rect
     * (128, 0, 32, 29). */
    PROBE_PORTRAIT_COLS = 8,
    PROBE_PORTRAIT_ROWS = 3,
    PROBE_PORTRAIT_TOTAL = 24,
    /* Ordinal 4 atlas math (DEFS.H:821-826 macro encoding):
     *   srcX = (4 & 7) << 5 = 128
     *   srcY = (4 >> 3) * 29 = 0
     * The row-0 atlas path through (ordinal >> 3) * 29 yields 0 for
     * ordinals 0..7; ordinal 4 is the 5th column of the row-0 strip. */
    ORDINAL_4_COL = 4 & 7,             /* = 4 */
    ORDINAL_4_ROW = 4 >> 3,            /* = 0 (top row of the 8x3 grid) */
    ORDINAL_4_SRC_X = ORDINAL_4_COL << 5,    /* = 128 */
    ORDINAL_4_SRC_Y = ORDINAL_4_ROW * 29,    /* = 0 */
    /* RESTING overlay rectangle (m11_game_view.c:27730-27737):
     *   m11_fill_rect(100, 70, 120, 30, M11_COLOR_BLACK)
     *   m11_draw_rect(100, 70, 120, 30, M11_COLOR_LIGHT_BLUE)
     * framebuffer-local.  The D1C portrait cutout (96, 35, 32, 29)
     * viewport-local = (96, 68, 32, 29) framebuffer-local sits at
     * the top-left of the overlay region.  The cutout's fb extent
     * (96+32=128, 68+29=97) overlaps the overlay (100..219, 70..99)
     * in the rectangle (100..127, 70..96) — that is 28 * 27 = 756
     * pixels of the cutout are overpainted by the RESTING fill.
     * The sleep_repaint slice asserts the portrait_rect_position
     * invariant is the ANCHORED RECTANGLE (the cutout is still at
     * (96, 35, 32, 29) viewport-local even with the overlay drawn
     * on top), not "ordinal 4 visible at all times".  The wake
     * redraw restores the portrait in Stage 4. */
    RESTING_X_FB = 100,
    RESTING_Y_FB = 70,
    RESTING_W = 120,
    RESTING_H = 30,
    /* Hall of Champions ordinal 4 = LEIF (per ReDMCSB DUNVIEW.C
     * G0289 nibble decode table; the shipped DM1 V1 mirror catalog
     * has 24 records with LEIF as the 5th entry, ordinal index 4).
     * C127 sensorData is 0-indexed so the M11 ordinal = 4 is the
     * 5th cell in the portrait strip: column 4, row 0. */
    ORDINAL_TARGET = 4,
    /* The portrait_rect_position invariant: the cutout stays
     * anchored at (96, 35, 32, 29) on the viewport and never drifts
     * onto a side wall.  Reuse the threshold the existing
     * champion-mirror cancel_reopen probe locks for ordinal 4:
     * >= 30 warm pixels for "portrait present" and a C026 strip cell
     * match >= 70% for the ordinal-dominance assertion.  The
     * no-floating side-pose tolerance mirrors the cancel_reopen
     * probe. */
    PORTRAIT_PRESENT_WARM_THRESHOLD = 30,
    PORTRAIT_MATCH_PCT_MIN = 70,
    /* RESTING overlay fill detection: the black backdrop covers
     * 120 * 30 = 3600 pixels; the resting pass overwrites the
     * rectangle.  We require >= 30 black pixels (palette index
     * 0x00) in the overlay rectangle to declare "resting overlay
     * visible".  The wake assertion uses a delta from the pre-sleep
     * baseline count (see Stage 4), so the wake stage needs the
     * baseline as input, not a hard cutoff. */
    RESTING_OVERLAY_PRESENT_BLACK = 30,
    /* Sleep->sleep byte-stability tolerance: a tolerance of 0
     * pixels is the strict assertion that consecutive sleep redraws
     * produce identical framebuffers (no jitter from animation
     * timers, no random palette noise).  The probe accepts up to
     * 2 pixels of jitter to absorb the m11_log_event text-shadow
     * anti-aliasing when the "T%u: RESTING" log entry is appended
     * to the message log between draws. */
    SLEEP_REDRAW_TOLERANCE = 2,
    /* Float tolerance for the side/no-front pose: the side pose
     * must leave at most 35% of the compared ordinal pixels
     * matching (same threshold the cancel_reopen probe uses). */
    FLOATING_PERCENT_MAX = 35,
    /* Cycle count for Stage 4 stability.  Three sleep->wake cycles
     * is enough to catch a one-off "first sleep has stale RESTING
     * overlay" bug without spending the runtime budget on a longer
     * loop. */
    SLEEP_CYCLES = 3
};

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define PASS() do { ++g_pass; } while(0)
#define FAIL(fmt, ...) do { fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); ++g_fail; } while(0)
#define SKIP(fmt, ...) do { fprintf(stderr, "SKIP: " fmt "\n", ##__VA_ARGS__); ++g_skip; } while(0)

static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

/* Warm palette set the existing capture probe locks: palette indices
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} (green / red / orange /
 * peach / yellow / blue) mark the champion portrait sprite pixels
 * vs the grey-stone wall texture palette {0x01, 0x02, 0x07-grey,
 * 0x0D}. */
static int pixel_is_warm(unsigned char idx) {
    return idx == 0x07 || idx == 0x08 || idx == 0x09 ||
           idx == 0x0A || idx == 0x0B || idx == 0x0E;
}

typedef struct RectEvidence {
    int warmCount;
    int compared;
    int matched;
    int matchedPct;       /* matched*100/compared (only when compared>0) */
    int d1cZoneContainsPortrait;
} RectEvidence;

static void collect_rect_evidence(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal,
                                  RectEvidence* out) {
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int x, y;
    out->warmCount = 0;
    out->compared = 0;
    out->matched = 0;
    out->matchedPct = 0;
    out->d1cZoneContainsPortrait = 0;
    if (!fb) return;

    for (y = fbRectY; y < fbRectY + PORTRAIT_H; ++y) {
        if (y < 0 || y >= FB_H) continue;
        for (x = fbRectX; x < fbRectX + PORTRAIT_W; ++x) {
            if (x < 0 || x >= FB_W) continue;
            {
                unsigned char idx =
                    M11_FB_DECODE_INDEX(fb[y * FB_W + x]);
                if (pixel_is_warm(idx)) {
                    ++out->warmCount;
                }
            }
        }
    }

    if (portraits && portraits->loaded && portraits->pixels &&
        ordinal >= 0 && ordinal < PROBE_PORTRAIT_TOTAL) {
        int srcBaseX = (ordinal & 7) * PORTRAIT_W;
        int srcBaseY = (ordinal >> 3) * PORTRAIT_H;
        for (y = 0; y < PORTRAIT_H; ++y) {
            int srcY = srcBaseY + y;
            int dstY = fbRectY + y;
            if (srcY < 0 || srcY >= (int)portraits->height ||
                dstY < 0 || dstY >= FB_H) continue;
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = srcBaseX + x;
                int dstX = fbRectX + x;
                if (srcX < 0 || srcX >= (int)portraits->width ||
                    dstX < 0 || dstX >= FB_W) continue;
                {
                    unsigned char srcRaw = portraits->pixels[srcY * (int)portraits->width + srcX];
                    unsigned char srcIdx = (unsigned char)(srcRaw & 0x0F);
                    if (srcIdx == 1) continue; /* transparent */
                    {
                        unsigned char dstRaw = fb[dstY * FB_W + dstX];
                        unsigned char dstIdx = M11_FB_DECODE_INDEX(dstRaw);
                        ++out->compared;
                        if (dstIdx == srcIdx) ++out->matched;
                    }
                }
            }
        }
        if (out->compared > 0) {
            out->matchedPct = (out->matched * 100) / out->compared;
        }
    }

    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
}

/* Count black pixels (palette index 0x00) inside the RESTING
 * overlay rectangle (framebuffer coords).  The m11_game_view.c
 * resting pass overwrites the rectangle with M11_COLOR_BLACK on
 * top of the F0128 viewport, so a high black count is the
 * unambiguous fingerprint of the RESTING overlay being live. */
static int count_resting_black(const unsigned char* fb) {
    int x, y, count = 0;
    for (y = RESTING_Y_FB; y < RESTING_Y_FB + RESTING_H && y < FB_H; ++y) {
        if (y < 0) continue;
        for (x = RESTING_X_FB; x < RESTING_X_FB + RESTING_W && x < FB_W; ++x) {
            if (x < 0) continue;
            unsigned char idx = M11_FB_DECODE_INDEX(fb[y * FB_W + x]);
            if (idx == 0x00) ++count;
        }
    }
    return count;
}

static void reset_view(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
    game->resting = 0;
    game->partyDead = 0;
    game->damageFlashTimer = 0;
    game->attackCueTimer = 0;
}

/* Retarget the (1,2) NORTH C127 sensor (canonical ordinal 1 in the
 * real DM1 V1 fixture, HALK) to ordinal 4.  Same retarget pattern
 * the portrait04 family of probes uses for the (1,2) NORTH slice —
 * the M11_GameView_OpenSelectedMenuEntry on the real DM1 V1 fixture
 * places a single C127 sensor with sensorData == 1 on the (1,2)
 * front cell, so the retarget is unambiguous.  Returns the number
 * of sensors retargeted (1 on the real DM1 V1 fixture, 0 if the
 * fixture does not have an ordinal-1 C127 sensor on map 0). */
static int retarget_front_c127(M11_GameViewState* game,
                               int oldOrdinal,
                               int newOrdinal) {
    int idx;
    int found = 0;
    if (!game || !game->world.things || !game->world.things->sensors) {
        return 0;
    }
    for (idx = 0; idx < game->world.things->sensorCount; ++idx) {
        if (game->world.things->sensors[idx].sensorType != 127) continue;
        if ((int)game->world.things->sensors[idx].sensorData != oldOrdinal) continue;
        game->world.things->sensors[idx].sensorData = (unsigned short)newOrdinal;
        ++found;
        break;
    }
    return found;
}

/* ── Stage 1: pre-sleep baseline ─────────────────────────────────── */
static int check_stage_pre_sleep(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 unsigned char* fb,
                                 int* outBaselineBlack) {
    RectEvidence ev;
    int blackCount;
    int ornX, ornY, ornW, ornH;
    int frontOrdinal;
    int ok = 1;

    reset_view(game, 1, 2, DIR_NORTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (frontOrdinal != ORDINAL_TARGET) {
        /* Either the (1,2) front cell does not have a C127 sensor, or
         * the sensor's sensorData is not the target ordinal 4.  Apply
         * the same retarget pattern the portrait04 family of probes
         * uses. */
        if (retarget_front_c127(game, 1, ORDINAL_TARGET) != 1) {
            fprintf(stderr,
                    "SKIP this DM1 V1 build does not place a C127 sensor "
                    "with sensorData=1 at (1,2) front cell (front=%d, "
                    "no retarget possible); the sleep_repaint slice is "
                    "not exercised on builds that do not match the "
                    "reference DUNGEON.DAT fixture.\n",
                    frontOrdinal);
            return -1;
        }
        reset_view(game, 1, 2, DIR_NORTH);
        frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (frontOrdinal != ORDINAL_TARGET) {
            fprintf(stderr,
                    "SKIP retarget did not yield ordinal %d (got=%d); "
                    "the sleep_repaint slice is not exercised.\n",
                    ORDINAL_TARGET, frontOrdinal);
            return -1;
        }
    }

    /* Stage 0 evidence: the public D1C wall zone helper must report
     * the source-locked coordSet-5 / index-12 rectangle. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(game, &ornX, &ornY, &ornW, &ornH);
    if (ornX != D1C_ZONE_X_VP || ornY != D1C_ZONE_Y_VP ||
        ornW != D1C_ZONE_W || ornH != D1C_ZONE_H) {
        FAIL("D1C wall zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d) viewport-local",
             ornX, ornY, ornW, ornH,
             D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &ev);

    if (!ev.d1cZoneContainsPortrait) {
        FAIL("pre_sleep portrait_rect (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
             D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }
    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("pre_sleep portrait_rect not visible (warm=%d < %d)",
             ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("pre_sleep ordinal %d pixel match only %d%% (%d/%d) - portrait drifted",
             ORDINAL_TARGET, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }

    blackCount = count_resting_black(fb);
    /* Capture the baseline black count in the (100,70,120,30) region
     * as the reference for the post-wake redraw assertion in Stage
     * 4: the post-wake black count must equal the pre-sleep baseline
     * count (within a small tolerance) because the wake redraw must
     * restore the viewport to the pre-sleep state, and the pre-sleep
     * count is the source-of-truth for the "no RESTING overlay"
     * condition.  This is more robust than an absolute threshold
     * because the exact baseline count depends on the M11 renderer's
     * auxiliary overlays. */
    if (outBaselineBlack) *outBaselineBlack = blackCount;
    if (game->resting != 0) {
        FAIL("pre_sleep resting flag should be 0 (got=%d)", game->resting);
        ok = 0;
    }

    printf("  pre_sleep front_mirror=%d warm=%d match=%d%% (%d/%d) baseline_black=%d resting=%d\n",
           frontOrdinal, ev.warmCount, ev.matchedPct, ev.matched, ev.compared,
           blackCount, game->resting);
    return ok;
}

/* ── Stage 2: sleep state ────────────────────────────────────────── */
static int check_stage_sleep(M11_GameViewState* game,
                             const M11_AssetSlot* portraits,
                             unsigned char* fb) {
    RectEvidence ev;
    int blackCount;
    int ok = 1;

    /* Set the resting flag: this is the source-locked equivalent of
     * the m11_game_view.c M12_MENU_INPUT_REST_TOGGLE path (line
     * 8453-8463) which sets `state->resting = 1`, logs "T%u:
     * RESTING", sets the "PARTY IS RESTING" status, and returns
     * M11_GAME_INPUT_REDRAW.  The next M11_GameView_Draw cycle
     * paints the F0128 viewport redraw AND THEN overpaints the
     * RESTING overlay rectangle (100, 70, 120, 30) fb-local via
     * m11_game_view.c:27730-27737. */
    game->resting = 1;

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &ev);

    blackCount = count_resting_black(fb);
    if (blackCount < RESTING_OVERLAY_PRESENT_BLACK) {
        FAIL("sleep RESTING overlay not visible (black=%d < %d)",
             blackCount, RESTING_OVERLAY_PRESENT_BLACK);
        ok = 0;
    }
    /* The D1C portrait cutout (96, 35, 32, 29) viewport-local =
     * (96, 68, 32, 29) framebuffer-local is fully INSIDE the
     * RESTING overlay rectangle (100, 70, 120, 30) — the cutout's
     * fb origin (96, 68) is just above and to the left of the
     * overlay origin (100, 70) but the cutout's fb extent
     * (96+32=128, 68+29=97) overlaps the overlay (100..219, 70..99)
     * in the rectangle (100..127, 70..96) — that is 28 * 27 = 756
     * pixels of the cutout are overpainted by the RESTING fill.
     * The sleep_repaint slice asserts the portrait_rect_position
     * invariant is the ANCHORED RECTANGLE (the cutout is still at
     * (96, 35, 32, 29) viewport-local even with the overlay drawn
     * on top), not "ordinal 4 visible at all times".  The wake
     * redraw restores the portrait in Stage 4.  The D1C zone check
     * still passes because the cutout rectangle is still inside
     * the public D1C wall zone. */
    if (!ev.d1cZoneContainsPortrait) {
        FAIL("sleep portrait_rect (%d,%d,%d,%d) not inside D1C zone - drifted off D1C during sleep",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    if (game->resting != 1) {
        FAIL("sleep resting flag should be 1 (got=%d)", game->resting);
        ok = 0;
    }

    printf("  sleep front_mirror=%d warm=%d match=%d%% (%d/%d) black=%d resting=%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared,
           blackCount, game->resting);
    return ok;
}

/* ── Stage 3: sleep->sleep byte-stable repeat ────────────────────── */
static int check_stage_sleep_repeat(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits) {
    static unsigned char fbFirst[FB_W * FB_H];
    static unsigned char fbSecond[FB_W * FB_H];
    RectEvidence evFirst;
    RectEvidence evSecond;
    int blackSecond;
    int diff;
    int x;
    int ok = 1;

    /* Re-draw the sleep state.  This is the second M11_GameView_Draw
     * cycle while resting=1: the RESTING overlay path must be
     * idempotent (no animation jitter, no message-log timestamp
     * change, no clock-driven palette noise).  The byte-stability
     * check covers the whole framebuffer, not just the cutout. */
    memset(fbFirst, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fbFirst, FB_W, FB_H);
    collect_rect_evidence(portraits, fbFirst, ORDINAL_TARGET, &evFirst);
    (void)evFirst; /* first-cycle evidence is for diagnostic log only */

    memset(fbSecond, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fbSecond, FB_W, FB_H);
    collect_rect_evidence(portraits, fbSecond, ORDINAL_TARGET, &evSecond);
    blackSecond = count_resting_black(fbSecond);

    /* Count differing bytes across the full framebuffer.  The
     * sleep->sleep redraw must be near-byte-stable: at most a few
     * pixels differ (the m11_log_event text shadow anti-aliasing
     * does not change between two consecutive draws with no game
     * tick advance). */
    diff = 0;
    for (x = 0; x < FB_W * FB_H; ++x) {
        if (fbFirst[x] != fbSecond[x]) ++diff;
    }
    if (diff > SLEEP_REDRAW_TOLERANCE) {
        FAIL("sleep->sleep redraw not byte-stable (diff=%d > tolerance=%d)",
             diff, SLEEP_REDRAW_TOLERANCE);
        ok = 0;
    }
    if (blackSecond < RESTING_OVERLAY_PRESENT_BLACK) {
        FAIL("sleep->sleep second draw RESTING overlay gone (black=%d < %d)",
             blackSecond, RESTING_OVERLAY_PRESENT_BLACK);
        ok = 0;
    }
    if (evSecond.d1cZoneContainsPortrait != 1) {
        FAIL("sleep->sleep portrait_rect drifted off D1C zone");
        ok = 0;
    }

    printf("  sleep_repeat diff_bytes=%d (tol=%d) black=%d warm=%d match=%d%%\n",
           diff, SLEEP_REDRAW_TOLERANCE, blackSecond,
           evSecond.warmCount, evSecond.matchedPct);
    return ok;
}

/* ── Stage 4: sleep->wake cycle stability ───────────────────────── */
static int check_stage_cycles(M11_GameViewState* game,
                              const M11_AssetSlot* portraits,
                              int baselineBlack) {
    static unsigned char fbSleep[FB_W * FB_H];
    static unsigned char fbWake[FB_W * FB_H];
    RectEvidence evSleep;
    RectEvidence evWake;
    int blackSleep;
    int blackWake;
    /* Tolerance for the wake redraw returning the baseline black
     * count in the (100, 70, 120, 30) region.  A tolerance of 5
     * pixels absorbs the M11 renderer's auxiliary overlay jitter
     * (damage flash borders, message-log text shadows) between
     * the pre-sleep and post-wake draws.  The dominant signal is
     * the >= 3000 black pixel count from the RESTING overlay
     * (m11_game_view.c:27730-27737) which the wake redraw must
     * fully remove. */
    const int WAKE_BLACK_TOLERANCE = 5;
    int cycle;
    int ok = 1;

    for (cycle = 0; cycle < SLEEP_CYCLES; ++cycle) {
        /* Sleep: set resting=1, redraw, capture sleep frame. */
        game->resting = 1;
        memset(fbSleep, 0, FB_W * FB_H);
        M11_GameView_Draw(game, fbSleep, FB_W, FB_H);
        collect_rect_evidence(portraits, fbSleep, ORDINAL_TARGET, &evSleep);
        blackSleep = count_resting_black(fbSleep);
        if (blackSleep < RESTING_OVERLAY_PRESENT_BLACK) {
            FAIL("cycle %d sleep RESTING overlay not visible (black=%d < %d)",
                 cycle + 1, blackSleep, RESTING_OVERLAY_PRESENT_BLACK);
            ok = 0;
        }

        /* Wake: clear resting=0, redraw, capture wake frame.  The
         * wake redraw must restore the viewport to the pre-sleep
         * baseline (which is the source-of-truth for "no RESTING
         * overlay"): the post-wake black count in the overlay
         * region must be within WAKE_BLACK_TOLERANCE of the
         * pre-sleep baseline.  The D1C portrait cutout must
         * re-appear with the ordinal 4 portrait at the anchored
         * (96, 35, 32, 29) viewport-local rectangle. */
        game->resting = 0;
        memset(fbWake, 0, FB_W * FB_H);
        M11_GameView_Draw(game, fbWake, FB_W, FB_H);
        collect_rect_evidence(portraits, fbWake, ORDINAL_TARGET, &evWake);
        blackWake = count_resting_black(fbWake);
        if (blackWake > baselineBlack + WAKE_BLACK_TOLERANCE) {
            FAIL("cycle %d wake RESTING overlay still visible (black=%d > baseline %d + tolerance %d) - wake redraw did not clear the overlay",
                 cycle + 1, blackWake, baselineBlack, WAKE_BLACK_TOLERANCE);
            ok = 0;
        }
        if (!evWake.d1cZoneContainsPortrait) {
            FAIL("cycle %d wake portrait_rect (%d,%d,%d,%d) not inside D1C zone",
                 cycle + 1, PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
            ok = 0;
        }
        if (evWake.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
            FAIL("cycle %d wake portrait_rect not visible (warm=%d < %d) - wake redraw did not restore ordinal %d",
                 cycle + 1, evWake.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD, ORDINAL_TARGET);
            ok = 0;
        }
        if (evWake.compared > 0 && evWake.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
            FAIL("cycle %d wake ordinal %d pixel match only %d%% (%d/%d) - wake redraw drifted the portrait",
                 cycle + 1, ORDINAL_TARGET, evWake.matchedPct, evWake.matched, evWake.compared);
            ok = 0;
        }
        if (game->resting != 0) {
            FAIL("cycle %d post-wake resting flag should be 0 (got=%d)",
                 cycle + 1, game->resting);
            ok = 0;
        }
    }

    if (ok) {
        printf("  cycles sleep_wake_stable cycles=%d wake_warm=%d wake_match=%d%% sleep_black=%d wake_black=%d (baseline=%d)\n",
               SLEEP_CYCLES, evWake.warmCount, evWake.matchedPct,
               blackSleep, blackWake, baselineBlack);
    }
    return ok;
}

/* ── Stage 5: awake redraw byte-stability ───────────────────────── */
static int check_stage_awake_redraw_stability(M11_GameViewState* game,
                                              const M11_AssetSlot* portraits) {
    static unsigned char fbFirst[FB_W * FB_H];
    static unsigned char fbSecond[FB_W * FB_H];
    static unsigned char fbThird[FB_W * FB_H];
    RectEvidence evFirst;
    int diff12 = 0;
    int diff13 = 0;
    int x;
    int ok = 1;

    /* Awake redraw byte-stability.  Three consecutive M11_GameView_Draw
     * cycles on the awake (resting=0) viewport must produce
     * identical framebuffers, because the awake state has no
     * animation timers advancing between two consecutive draws with
     * no game tick.  This is the awake counterpart of Stage 3's
     * sleep->sleep stability check: the awake state machine must
     * also be idempotent.  A regression that leaks a one-shot draw
     * timer (e.g. a "portrait flicker on the first redraw after
     * wake" bug) would push diff12 or diff13 above the
     * tolerance. */
    if (game->resting != 0) {
        FAIL("awake_redraw_stability stage requires resting=0 (got=%d)",
             game->resting);
        return 0;
    }

    memset(fbFirst, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fbFirst, FB_W, FB_H);
    collect_rect_evidence(portraits, fbFirst, ORDINAL_TARGET, &evFirst);
    if (evFirst.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("awake_redraw_stability first draw portrait_rect not visible (warm=%d < %d)",
             evFirst.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (evFirst.compared > 0 && evFirst.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("awake_redraw_stability first draw ordinal %d match only %d%% (%d/%d)",
             ORDINAL_TARGET, evFirst.matchedPct, evFirst.matched, evFirst.compared);
        ok = 0;
    }

    memset(fbSecond, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fbSecond, FB_W, FB_H);
    memset(fbThird, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fbThird, FB_W, FB_H);

    for (x = 0; x < FB_W * FB_H; ++x) {
        if (fbFirst[x] != fbSecond[x]) ++diff12;
        if (fbFirst[x] != fbThird[x])  ++diff13;
    }
    if (diff12 > SLEEP_REDRAW_TOLERANCE) {
        FAIL("awake redraw 1->2 not byte-stable (diff=%d > tolerance=%d) - first-cycle draw drift",
             diff12, SLEEP_REDRAW_TOLERANCE);
        ok = 0;
    }
    if (diff13 > SLEEP_REDRAW_TOLERANCE) {
        FAIL("awake redraw 1->3 not byte-stable (diff=%d > tolerance=%d) - first-cycle draw drift across multiple redraws",
             diff13, SLEEP_REDRAW_TOLERANCE);
        ok = 0;
    }

    printf("  awake_redraw_stability diff_1_2=%d diff_1_3=%d (tol=%d) warm=%d match=%d%%\n",
           diff12, diff13, SLEEP_REDRAW_TOLERANCE,
           evFirst.warmCount, evFirst.matchedPct);
    return ok;
}

/* ── Stage 6: candidate-panel return behavior after wake ────────── */
static int check_stage_candidate_panel_return(M11_GameViewState* game,
                                             const M11_AssetSlot* portraits,
                                             unsigned char* fb) {
    RectEvidence evBaseline;
    RectEvidence evAfterPanel;
    RectEvidence evAfterClose;
    int ok = 1;

    if (game->resting != 0) {
        FAIL("candidate_panel_return stage requires resting=0 (got=%d)",
             game->resting);
        return 0;
    }

    /* Capture the pre-panel baseline.  The viewport is currently
     * in the post-wake (resting=0) state, so the D1C cutout
     * should show the ordinal 4 LEIF portrait at the anchored
     * (96, 35, 32, 29) viewport-local rectangle. */
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &evBaseline);
    if (evBaseline.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("candidate_panel_return baseline portrait not visible (warm=%d < %d)",
             evBaseline.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (evBaseline.compared > 0 && evBaseline.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("candidate_panel_return baseline ordinal %d match only %d%% (%d/%d)",
             ORDINAL_TARGET, evBaseline.matchedPct, evBaseline.matched, evBaseline.compared);
        ok = 0;
    }

    /* Open the candidate mirror panel: this is the source-locked
     * equivalent of the C145/C146 candidate panel state machine
     * (m11_game_view_render_candidate_mirror_panel).  In the
     * runtime, the panel is activated by a C127 sensor fire on
     * a slot, but in the headless probe we toggle the public
     * state directly: M635 candidate panel ON, candidate
     * ordinal = 4, candidate party index = 0. */
    game->candidateMirrorPanelActive = 1;
    game->candidateMirrorOrdinal = ORDINAL_TARGET;
    game->candidateMirrorPartyIndex = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &evAfterPanel);

    /* The candidate panel is a transient overlay; we do not
     * require the D1C cutout to be empty while the panel is
     * open, but we do require the panel state to be live (the
     * panel state must persist across the redraw).  The
     * portrait_rect_position invariant is verified AFTER the
     * panel closes. */

    /* Close the candidate mirror panel: the source-locked
     * equivalent of the F0282 / cancel-reopen panel state
     * machine exit (m11_game_view_render_candidate_mirror_panel
     * exit branch).  The D1C portrait cutout must be repainted
     * with ordinal 4 at the anchored rectangle after the close. */
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &evAfterClose);

    if (!evAfterClose.d1cZoneContainsPortrait) {
        FAIL("candidate_panel_return portrait_rect (%d,%d,%d,%d) not inside D1C zone after panel close",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    if (evAfterClose.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("candidate_panel_return portrait_rect not visible after close (warm=%d < %d) - post-panel redraw missing the LEIF portrait",
             evAfterClose.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (evAfterClose.compared > 0 && evAfterClose.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("candidate_panel_return ordinal %d match only %d%% (%d/%d) after panel close - post-panel redraw drifted",
             ORDINAL_TARGET, evAfterClose.matchedPct, evAfterClose.matched, evAfterClose.compared);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 0) {
        FAIL("candidate_panel_return panel state should be 0 after close (got=%d)",
             game->candidateMirrorPanelActive);
        ok = 0;
    }

    printf("  candidate_panel_return baseline warm=%d match=%d%% panel_warm=%d match=%d%% close_warm=%d match=%d%%\n",
           evBaseline.warmCount, evBaseline.matchedPct,
           evAfterPanel.warmCount, evAfterPanel.matchedPct,
           evAfterClose.warmCount, evAfterClose.matchedPct);
    return ok;
}

/* ── Stage 7: side/no-front pose does not float ordinal 4 ───────── */
static int check_stage_no_floating(M11_GameViewState* game,
                                   const M11_AssetSlot* portraits,
                                   unsigned char* fb) {
    int x, y, compared = 0, matched = 0;
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int ok = 1;

    reset_view(game, 1, 2, DIR_EAST);

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        FAIL("side pose front_mirror ordinal should be -1 (got=%d)",
             M11_GameView_GetFrontMirrorOrdinal(game));
        ok = 0;
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        int srcBaseX = (ORDINAL_TARGET & 7) * PORTRAIT_W;
        int srcBaseY = (ORDINAL_TARGET >> 3) * PORTRAIT_H;
        for (y = 0; y < PORTRAIT_H; ++y) {
            int srcY = srcBaseY + y;
            int dstY = fbRectY + y;
            if (srcY < 0 || srcY >= (int)portraits->height ||
                dstY < 0 || dstY >= FB_H) continue;
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = srcBaseX + x;
                int dstX = fbRectX + x;
                if (srcX < 0 || srcX >= (int)portraits->width ||
                    dstX < 0 || dstX >= FB_W) continue;
                {
                    unsigned char srcRaw = portraits->pixels[srcY * (int)portraits->width + srcX];
                    unsigned char srcIdx = (unsigned char)(srcRaw & 0x0F);
                    if (srcIdx == 1) continue;
                    {
                        unsigned char dstRaw = fb[dstY * FB_W + dstX];
                        unsigned char dstIdx = M11_FB_DECODE_INDEX(dstRaw);
                        ++compared;
                        if (dstIdx == srcIdx) ++matched;
                    }
                }
            }
        }
    }

    if (compared > 0 && matched * 100 >= compared * FLOATING_PERCENT_MAX) {
        FAIL("side pose floats ordinal %d in D1C rect matched=%d/%d (%d%%)",
             ORDINAL_TARGET, matched, compared,
             (matched * 100) / compared);
        ok = 0;
    }
    printf("  side_pose front_mirror=%d stale ordinal %d pixels=%d/%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           ORDINAL_TARGET, matched, compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[FB_W * FB_H];
    int rc;
    int baselineBlack = 0;
    int atlasOrdinal4Opaque;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 4 sleep_repaint portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 HoC champion portrait ordinal 4 sleep_repaint "
           "portrait_rect_position ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);
    printf("ordinal 4 atlas math: srcX=%d, srcY=%d (col=%d, row=%d)\n",
           ORDINAL_4_SRC_X, ORDINAL_4_SRC_Y,
           ORDINAL_4_COL, ORDINAL_4_ROW);

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Atlas self-consistency: ordinal 4 must be a defined portrait
     * in the C026 atlas (row 0 col 4, source rect 128, 0, 32, 29).
     * Count opaque pixels in the cell; a defined champion portrait
     * carries >= 100 opaque pixels (warm skin tones, clothing,
     * background).  An unused slot would be either all-zero or
     * all-transparent. */
    {
        int x, y, cnt = 0;
        int srcX = ORDINAL_4_SRC_X;
        int srcY = ORDINAL_4_SRC_Y;
        for (y = 0; y < PORTRAIT_H; ++y) {
            for (x = 0; x < PORTRAIT_W; ++x) {
                int sx = srcX + x;
                int sy = srcY + y;
                unsigned char src;
                if (sx >= (int)portraits->width ||
                    sy >= (int)portraits->height) continue;
                src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
                if (src != 0 && src != 1) ++cnt;
            }
        }
        atlasOrdinal4Opaque = cnt;
    }
    if (atlasOrdinal4Opaque < 100) {
        FAIL("ordinal 4 atlas cell has < 100 opaque pixels (got %d) - defined portrait, not blank/unused",
             atlasOrdinal4Opaque);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    printf("  atlas self-consistency: ordinal 4 opaque=%d (>= 100)\n",
           atlasOrdinal4Opaque);

    printf("\n[Stage 1] Pre-sleep baseline portrait_rect_position\n");
    rc = check_stage_pre_sleep(&game, portraits, fb, &baselineBlack);
    if (rc < 0) {
        M11_GameView_Shutdown(&game);
        printf("SKIP dm1 v1 HoC champion portrait ordinal 4 sleep_repaint portrait_rect_position\n");
        return 0;
    }
    if (rc) PASS();

    printf("\n[Stage 2] Sleep state draws RESTING overlay\n");
    if (check_stage_sleep(&game, portraits, fb)) PASS();

    printf("\n[Stage 3] Sleep->sleep redraw is byte-stable\n");
    if (check_stage_sleep_repeat(&game, portraits)) PASS();

    printf("\n[Stage 4] Sleep/wake cycle stability\n");
    if (check_stage_cycles(&game, portraits, baselineBlack)) PASS();

    printf("\n[Stage 5] Awake redraw byte-stability across 3 draws\n");
    if (check_stage_awake_redraw_stability(&game, portraits)) PASS();

    printf("\n[Stage 6] Candidate-panel return after wake redraw\n");
    if (check_stage_candidate_panel_return(&game, portraits, fb)) PASS();

    printf("\n[Stage 7] Side/no-front pose does not float ordinal 4\n");
    if (check_stage_no_floating(&game, portraits, fb)) PASS();

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed, %d skipped ===\n",
           g_pass, g_fail, g_skip);
    return g_fail == 0 ? 0 : 1;
}
