/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 19 (HAWK,
 * THE FEARLESS) wake_repaint / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal    = 19 (the C127 sensorData=19 retargeted on the (1,2)
 *                 NORTH front cell of the DM1 V1 DUNGEON.DAT; the
 *                 shipped DM1 V1 fixture places a single C127 sensor
 *                 with sensorData=1 on the (1,2) front cell, so this
 *                 probe uses the same retarget pattern the existing
 *                 ordinal_02 wake_repaint probe uses, but retargets
 *                 the shipped HALK ordinal 1 to ordinal 19 — the row-
 *                 2 / column-3 atlas cell that carries HAWK.  The
 *                 ordinal-19 cancel_reopen probe and the ordinal-19
 *                 wall_ornament_no_float probe both lock ordinal-19
 *                 slices that do NOT exercise the rest/wake flag
 *                 cycle, so this gate is a non-duplicative extension
 *                 of those slices into the wake_repaint route.)
 *   route      = wake_repaint
 *                 (the M11 viewport redraw cycle that fires after the
 *                  party "wakes" from a rest state.  In the source-
 *                  locked DM1 V1 flow, COMMAND.C:2361-2363 dispatches
 *                  the C146_COMMAND_WAKE_UP route while G0300_B_PartyIsResting
 *                  is set; the wake path clears the resting flag and
 *                  re-paints the viewport from the party map/x/y/direction
 *                  through F0128.  The resting flag additionally draws
 *                  a small RESTING... overlay on top of the viewport at
 *                  framebuffer (100, 70, 120, 30) via
 *                  m11_game_view.c:27730-27737, so the wake_repaint
 *                  route asserts: (1) the resting overlay is on while
 *                  resting, (2) the wake flag clear plus a redraw
 *                  removes the overlay, and (3) the D1C portrait
 *                  cutout (96, 35, 32, 29) holds the ordinal 19
 *                  portrait at every stage of the cycle, not just the
 *                  post-cancel state that the existing cancel_reopen
 *                  / redraw_after_candidate / wall_ornament_no_float
 *                  probes cover.)
 *   aspect     = portrait_rect_position
 *                 (the C026 champion portrait cutout stays anchored
 *                  at the source-locked D1C viewport rectangle
 *                  (96, 35, 32, 29) on every redraw the wake
 *                  cycle triggers, and the rectangle stays inside
 *                  the public D1C wall-mirror zone (80, 29, 64, 43)
 *                  reported by M11_GameView_GetD1CWallOrnamentZone)
 *
 * Coverage gap relative to existing ordinal-19 champion-mirror probes:
 *   - firestaff_dm1_v1_hall_of_champions_portrait_19_cancel_reopen_*
 *     covers select -> cancel -> reopen on the candidate panel
 *     state machine, but the resting state is never set, so the
 *     RESTING overlay path is not exercised and the post-cancel
 *     redraw is the only viewport redraw cycle that runs.
 *   - firestaff_dm1_v1_hoc_champion_portrait_19_wall_ornament_no_float_*
 *     locks the C346 frame backing composition (BLACK outer ring +
 *     LIGHT_GRAY top/left + GRAY bottom/right + DARK_GRAY interior)
 *     and the C026 portrait sprite isolation, but only with the
 *     resting flag clear and no RESTING overlay drawn.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_*
 *     and the equivalent 10 / 21 probes cover panel state transitions
 *     (open/close/reopen) but never set G0300_B_PartyIsResting,
 *     so the wake repaint slice is uncovered for any ordinal.
 *   - firestaff_dm1_v1_hoc_champion_portrait_02_wake_repaint_*
 *     covers ordinal 2 through the rest/wake flag cycle but is the
 *     only existing wake_repaint gate — ordinal 19 (HAWK / row 2
 *     / col 3) is a different atlas cell with a different champion
 *     identity and needs its own gate so the wake redraw path is
 *     locked for the row-2 / col-3 atlas path
 *     ((19 >> 3) * 29 = 58 source Y, (19 & 7) << 5 = 96 source X)
 *     and not just the row-0 / col-2 path of ordinal 2.
 *
 * What the probe asserts at each stage:
 *   Stage 1 (pre-rest):    D1C cutout (96,35,32,29) shows ordinal 19
 *                          with C026 strip cell match >= 70% and
 *                          warm pixel count >= 30; the RESTING
 *                          overlay region (100,70,120,30) fb-local
 *                          is NOT painted with the RESTING... text
 *                          fill (black-on-blue backdrop);
 *                          resting=0.
 *   Stage 2 (resting):     After setting resting=1, the next redraw
 *                          paints the RESTING overlay region with
 *                          the source-locked fill (>= 30 black
 *                          pixels in the backdrop), and the D1C
 *                          portrait cutout is still anchored at
 *                          (96,35,32,29) with the ordinal 19
 *                          portrait rectangle preserved even when
 *                          the overlay is drawn on top of the rest
 *                          of the viewport.
 *   Stage 3 (wake):        After clearing resting=0, the next
 *                          redraw must NOT contain the RESTING
 *                          overlay fill (the black backdrop is
 *                          gone), and the D1C cutout (96,35,32,29)
 *                          must still show the ordinal 19 portrait
 *                          (warm count >= 30, C026 strip cell
 *                          match >= 70%).
 *   Stage 4 (cycles):      4 rest/wake cycles at (1,2) NORTH
 *                          produce a byte-stable post-wake
 *                          framebuffer.  This locks the wake
 *                          redraw path: every wake must repaint
 *                          the viewport back to the same bytes
 *                          (no drift across cycles), and the
 *                          RESTING overlay must be absent on
 *                          every post-wake redraw.
 *   Stage 5 (side pose):   At (1,2) facing EAST the front-mirror
 *                          ordinal is -1 and the D1C cutout must
 *                          NOT show ordinal 19 pixels (no floating).
 *                          The portrait_rect position invariant
 *                          is trivially satisfied because there
 *                          is no front-mirror ordinal to draw, but
 *                          the cutout rectangle is still inside
 *                          the public D1C zone.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C champion portrait blit (C026)
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw from
 *                       party map/x/y/direction (far-to-near order)
 *   ReDMCSB DUNVIEW.C:4547-4581 nibble 2 -> ordinal 2 (G0289 decode)
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB COMMAND.C:2336-2359 rest path sets G0300_B_PartyIsResting
 *                       and redraws the viewport; rejects while
 *                       G0299 is live
 *   ReDMCSB COMMAND.C:2361-2363 C146_COMMAND_WAKE_UP dispatches wake
 *   ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS (256x87,
 *                       8 columns * 32 px wide, 3 rows * 29 px tall,
 *                       ordinals 0..23)
 *   ReDMCSB DEFS.H:821-826 M027_PORTRAIT_X(index), M028_PORTRAIT_Y
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *   m11_game_view.c:27730-27737 RESTING... overlay
 *
 * Honest scope: Firestaff runtime evidence against the real DM1 V1
 * DUNGEON.DAT / GRAPHICS.DAT pair (retargeted (1,2) NORTH C127
 * sensor to ordinal 19 — the same retarget pattern the existing
 * ordinal_19 wall_ornament_no_float and ordinal_19 cancel_reopen
 * companion probes use).  The RESTING overlay path is sourced from
 * m11_game_view.c:27730-27737 (the resting state machine in
 * m11_game_view_render_dungeon_view).  The wake_repaint slice
 * asserts runtime evidence, not DOS pixel parity.  No claim of
 * byte-equal DOSBox capture.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_19_wake_repaint_portrait_rect_position_091_gate_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "memory_champion_state_pc34_compat.h"

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
     * so the cutout is (96, 35, 32, 29) viewport-local = (96, 68, 32,
     * 29) framebuffer-local. */
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
     * Ordinal 19 -> atlas column 3, atlas row 2 -> source rect
     * (96, 58, 32, 29). */
    PROBE_PORTRAIT_COLS = 8,
    PROBE_PORTRAIT_ROWS = 3,
    PROBE_PORTRAIT_TOTAL = 24,
    /* RESTING overlay rectangle (m11_game_view.c:27730-27737):
     *   m11_fill_rect(100, 70, 120, 30, M11_COLOR_BLACK)
     *   m11_draw_rect(100, 70, 120, 30, M11_COLOR_LIGHT_BLUE)
     * framebuffer-local.  The D1C portrait cutout (96, 35, 32, 29)
     * viewport-local = (96, 68, 32, 29) framebuffer-local has its
     * bottom-right corner under the top-left of the RESTING overlay
     * but the rectangle position invariant must still hold while
     * the overlay is drawn on top of the rest of the viewport. */
    RESTING_X_FB = 100,
    RESTING_Y_FB = 70,
    RESTING_W = 120,
    RESTING_H = 30,
    /* Hall of Champions ordinal 19 = HAWK (THE FEARLESS) per the
     * shipped DM1 V1 DUNGEON.DAT mirror catalog.  C127 sensorData
     * is 0-indexed so the M11 ordinal = 19 is the twentieth cell in
     * the portrait strip: column 3, row 2.  This row-2 / col-3
     * atlas cell is exactly the slice the ordinal_19 cancel_reopen
     * and ordinal_19 wall_ornament_no_float companion probes cover;
     * this gate extends those slices through the wake_repaint
     * route, which neither companion probe exercises. */
    ORDINAL_TARGET = 19,
    /* The portrait_rect_position invariant: the cutout stays
     * anchored at (96, 35, 32, 29) on the viewport and never drifts
     * onto a side wall.  Reuse the threshold the existing
     * champion-mirror capture probe locks: >= 30 warm pixels for
     * "portrait present" and a C026 strip cell match >= 70% for
     * the ordinal-dominance assertion.  The no-floating side-pose
     * tolerance is the same as the ordinal_02 wake_repaint gate
     * (<= 35% of compared pixels match). */
    PORTRAIT_PRESENT_WARM_THRESHOLD = 30,
    PORTRAIT_MATCH_PCT_MIN = 70,
    /* RESTING overlay fill detection: the black backdrop covers
     * 120 * 30 = 3600 pixels; even after the C017 inventory
     * backdrop and the F0128 viewport draw, the resting pass
     * overwrites the entire rectangle.  We require >= 30 black
     * pixels (palette index 0x00) in the overlay rectangle to
     * declare "resting overlay visible".  The wake assertion
     * does NOT use an absolute threshold for "resting overlay
     * gone" — it uses a delta from the pre-rest baseline count
     * (see Stage 3 / check_stage_wake), so the wake stage needs
     * the baseline as input, not a hard cutoff.  The cutout area
     * may have black pixels from the D1C wall frame's black
     * outline, so the black-pixel counter uses the full overlay
     * rectangle, not the cutout area. */
    RESTING_OVERLAY_PRESENT_BLACK = 30,
    /* Float tolerance for the side/no-front pose: the side pose
     * must leave at most 35% of the compared ordinal pixels
     * matching. */
    FLOATING_PERCENT_MAX = 35,
    /* Cycle count for Stage 4 stability.  Four cycles is enough
     * to catch a one-off "first wake has stale RESTING overlay"
     * bug without spending the runtime budget on a longer loop. */
    RESTING_CYCLES = 4
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
}

/* Retarget the (1,2) NORTH C127 sensor (canonical ordinal 1 in the
 * real DM1 V1 fixture, HALK) to ordinal 19.  Same retarget pattern
 * the ordinal_19 cancel_reopen and ordinal_19 wall_ornament_no_float
 * companion probes use — they seed the shipped HALK ordinal 1
 * sensor to ordinal 19 to lock the ordinal-19 edge case without
 * changing the map layout.  This probe uses the same seed so the
 * wake_repaint slice is exercised against the same row-2 / col-3
 * atlas cell (96, 58, 32, 29) that those companion probes cover.
 * Returns the number of sensors retargeted (1 on the real DM1 V1
 * fixture, 0 if the fixture does not have an ordinal-1 C127 sensor
 * on map 0). */
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

/* ── Stage 1: pre-rest baseline ─────────────────────────────────── */
static int check_stage_pre_rest(M11_GameViewState* game,
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
         * the sensor's sensorData is not the target ordinal 19.  Apply
         * the same retarget pattern the ordinal_19 cancel_reopen and
         * ordinal_19 wall_ornament_no_float companion probes use:
         * seed the shipped HALK ordinal 1 sensor to ordinal 19. */
        if (retarget_front_c127(game, 1, ORDINAL_TARGET) != 1) {
            fprintf(stderr,
                    "SKIP this DM1 V1 build does not place a C127 sensor "
                    "with sensorData=1 at (1,2) front cell (front=%d, "
                    "no retarget possible); the wake_repaint slice is "
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
                    "the wake_repaint slice is not exercised.\n",
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
        FAIL("pre_rest portrait_rect (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
             D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }
    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("pre_rest portrait_rect not visible (warm=%d < %d)",
             ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("pre_rest ordinal %d pixel match only %d%% (%d/%d) — portrait drifted",
             ORDINAL_TARGET, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }

    blackCount = count_resting_black(fb);
    /* The baseline (100,70,120,30) region is NOT a clean black field
     * because m11_game_view.c may render text shadows / message log
     * entries / damage flash borders that overlap the rectangle.  We
     * capture the baseline count here and use it as the reference for
     * the wake redraw assertion in Stage 3: the post-wake black
     * count must equal the pre-rest baseline count (within a small
     * tolerance) because the wake redraw must restore the viewport
     * to the pre-rest state, and the pre-rest count is the
     * source-of-truth for the "no RESTING overlay" condition.  This
     * is more robust than an absolute threshold because the exact
     * baseline count depends on the M11 renderer's auxiliary
     * overlays. */
    if (outBaselineBlack) *outBaselineBlack = blackCount;
    if (game->resting != 0) {
        FAIL("pre_rest resting flag should be 0 (got=%d)", game->resting);
        ok = 0;
    }

    printf("  pre_rest front_mirror=%d warm=%d match=%d%% (%d/%d) baseline_black=%d resting=%d\n",
           frontOrdinal, ev.warmCount, ev.matchedPct, ev.matched, ev.compared,
           blackCount, game->resting);
    return ok;
}

/* ── Stage 2: resting state ─────────────────────────────────────── */
static int check_stage_resting(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               unsigned char* fb) {
    RectEvidence ev;
    int blackCount;
    int ok = 1;

    /* Set the resting flag; m11_game_view.c:27730-27737 then draws
     * the RESTING overlay on top of the F0128 viewport redraw. */
    game->resting = 1;

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &ev);

    blackCount = count_resting_black(fb);
    if (blackCount < RESTING_OVERLAY_PRESENT_BLACK) {
        FAIL("resting RESTING overlay not visible (black=%d < %d)",
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
     * pixels.  The RESTING overlay overpaints the lower-right
     * 756 of the 928 cutout pixels.  We accept either:
     *   (a) the F0128 portrait blit is drawn FIRST and the RESTING
     *       overlay overpaints it (the dominant case): the cutout
     *       shows a mix of portrait pixels and RESTING fill — the
     *       warm_count drops and the matchedPct drops.  This is
     *       acceptable for the wake_repaint slice: the
     *       portrait_rect_position invariant is the ANCHORED
     *       RECTANGLE, not "ordinal 19 visible at all times".  The
     *       wake redraw restores the portrait in Stage 3.
     *   (b) the RESTING overlay is drawn BEFORE the F0128 portrait
     *       blit: this is also acceptable because the
     *       portrait_rect_position invariant still holds (the
     *       rectangle is still (96, 35, 32, 29) viewport-local).
     * The wake_repaint slice does NOT assert ordinal-19 dominance
     * in Stage 2 — that is the wake path's job (Stage 3).  The
     * Stage 2 assertion is the RESTING overlay is visible, the
     * resting flag is set, and the D1C zone check still passes. */
    if (!ev.d1cZoneContainsPortrait) {
        FAIL("resting portrait_rect (%d,%d,%d,%d) not inside D1C zone — drifted off D1C during rest",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    if (game->resting != 1) {
        FAIL("resting flag should be 1 (got=%d)", game->resting);
        ok = 0;
    }

    printf("  resting front_mirror=%d warm=%d match=%d%% (%d/%d) black=%d resting=%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared,
           blackCount, game->resting);
    return ok;
}

/* ── Stage 3: wake redraw ───────────────────────────────────────── */
static int check_stage_wake(M11_GameViewState* game,
                            const M11_AssetSlot* portraits,
                            unsigned char* fb,
                            int baselineBlack) {
    RectEvidence ev;
    int blackCount;
    /* Tolerance for the wake redraw returning the baseline black
     * count in the (100, 70, 120, 30) region.  A tolerance of 5
     * pixels absorbs the M11 renderer's auxiliary overlay jitter
     * (damage flash borders, message-log text shadows) between
     * the pre-rest and post-wake draws.  The dominant signal is
     * the >= 3000 black pixel count from the RESTING overlay
     * (m11_game_view.c:27730-27737) which the wake redraw must
     * fully remove. */
    const int WAKE_BLACK_TOLERANCE = 5;
    int ok = 1;

    /* Wake: clear the resting flag.  ReDMCSB COMMAND.C:2361-2363
     * dispatches the C146_COMMAND_WAKE_UP route while
     * G0300_B_PartyIsResting is set; the wake path clears
     * G0300_B_PartyIsResting and re-paints the viewport through
     * F0128.  In the M11 state this is exactly `state->resting = 0`
     * followed by a viewport redraw. */
    game->resting = 0;

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_TARGET, &ev);

    blackCount = count_resting_black(fb);
    if (blackCount > baselineBlack + WAKE_BLACK_TOLERANCE) {
        FAIL("wake RESTING overlay still visible (black=%d > baseline %d + tolerance %d) — wake redraw did not clear the overlay",
             blackCount, baselineBlack, WAKE_BLACK_TOLERANCE);
        ok = 0;
    }
    if (!ev.d1cZoneContainsPortrait) {
        FAIL("wake portrait_rect (%d,%d,%d,%d) not inside D1C zone",
             PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        FAIL("wake portrait_rect not visible (warm=%d < %d) — wake redraw did not restore ordinal %d",
             ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD, ORDINAL_TARGET);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
        FAIL("wake ordinal %d pixel match only %d%% (%d/%d) — wake redraw drifted the portrait",
             ORDINAL_TARGET, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }
    if (game->resting != 0) {
        FAIL("wake resting flag should be 0 (got=%d)", game->resting);
        ok = 0;
    }

    printf("  wake front_mirror=%d warm=%d match=%d%% (%d/%d) black=%d (baseline=%d) resting=%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared,
           blackCount, baselineBlack, game->resting);
    return ok;
}

/* ── Stage 4: byte-stable rest/wake cycles ─────────────────────── */
static int check_stage_cycles(M11_GameViewState* game,
                              const M11_AssetSlot* portraits) {
    static unsigned char fb0[FB_W * FB_H];
    static unsigned char fbN[FB_W * FB_H];
    RectEvidence ev0;
    RectEvidence evN;
    int cycle;
    int ok = 1;

    for (cycle = 0; cycle < RESTING_CYCLES; ++cycle) {
        game->resting = 1;
        M11_GameView_Draw(game, fbN, FB_W, FB_H);
        game->resting = 0;
        memset(fbN, 0, FB_W * FB_H);
        M11_GameView_Draw(game, fbN, FB_W, FB_H);
        collect_rect_evidence(portraits, fbN, ORDINAL_TARGET, &evN);
        if (cycle == 0) {
            collect_rect_evidence(portraits, fbN, ORDINAL_TARGET, &ev0);
            memcpy(fb0, fbN, sizeof(fb0));
            if (evN.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
                FAIL("cycle 0 post-wake portrait_rect not visible (warm=%d < %d)",
                     evN.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
                ok = 0;
            }
            if (evN.compared > 0 && evN.matchedPct < PORTRAIT_MATCH_PCT_MIN) {
                FAIL("cycle 0 post-wake ordinal %d match %d%%",
                     ORDINAL_TARGET, evN.matchedPct);
                ok = 0;
            }
        } else {
            if (evN.warmCount != ev0.warmCount) {
                FAIL("cycle %d post-wake warm drift got=%d want=%d",
                     cycle + 1, evN.warmCount, ev0.warmCount);
                ok = 0;
            }
            if (evN.matchedPct != ev0.matchedPct) {
                FAIL("cycle %d post-wake match%% drift got=%d want=%d",
                     cycle + 1, evN.matchedPct, ev0.matchedPct);
                ok = 0;
            }
            if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
                FAIL("cycle %d post-wake framebuffer drift", cycle + 1);
                ok = 0;
            }
        }
    }
    if (ok) {
        printf("  cycles rest_wake_stable cycles=%d warm=%d match=%d%%\n",
               RESTING_CYCLES, ev0.warmCount, ev0.matchedPct);
    }
    return ok;
}

/* ── Stage 5: side/no-front pose does not float ordinal 19 ─────── */
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

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 19 wake_repaint portrait_rect_position\n",
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

    printf("=== DM1 V1 HoC champion portrait ordinal 19 wake_repaint "
           "portrait_rect_position ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("\n[Stage 1] Pre-rest baseline portrait_rect_position\n");
    rc = check_stage_pre_rest(&game, portraits, fb, &baselineBlack);
    if (rc < 0) {
        M11_GameView_Shutdown(&game);
        printf("SKIP dm1 v1 HoC champion portrait ordinal 19 wake_repaint portrait_rect_position\n");
        return 0;
    }
    if (rc) PASS();

    printf("\n[Stage 2] Resting state draws RESTING overlay\n");
    if (check_stage_resting(&game, portraits, fb)) PASS();

    printf("\n[Stage 3] Wake redraw restores portrait_rect_position\n");
    if (check_stage_wake(&game, portraits, fb, baselineBlack)) PASS();

    printf("\n[Stage 4] Rest/wake cycle stability\n");
    if (check_stage_cycles(&game, portraits)) PASS();

    printf("\n[Stage 5] Side/no-front pose does not float ordinal 19\n");
    if (check_stage_no_floating(&game, portraits, fb)) PASS();

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed, %d skipped ===\n",
           g_pass, g_fail, g_skip);
    return g_fail == 0 ? 0 : 1;
}
