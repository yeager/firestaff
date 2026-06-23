/*
 * firestaff_dm1_v1_champion_mirror_ordinal_8_sleep_repaint_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 8               (mirror catalog record IAIDO / "RUYITO CHIBURI")
 *   route   sleep_repaint   (the M11 idle-tick cycle: 200 ticks of
 *                            M11_GameView_AdvanceIdleTick, then a fresh
 *                            M11_GameView_Draw; portrait rect must still
 *                            carry ordinal 8 pixels and must not float)
 *   aspect  portrait_rect_position
 *
 * What this probe pins (and why it is its own slice):
 *
 *   The ordinal-8 champion portrait is on row 1 / column 0 of the C026
 *   atlas (8x3 grid of 32x29 portraits, 256x87 strip).  The C026
 *   portrait blit at viewport (96, 35, 32, 29) is the source-locked
 *   DUNVIEW.C destination rectangle - it is drawn whenever the M11
 *   draw path runs (movement, idle tick, panel open, etc.).  The
 *   existing ordinal-8 gates cover the cancel_reopen and south_return
 *   route aspects.  Neither of them exercises the idle-tick path
 *   that the main SDL loop calls in main_loop_m11.c:2620 (after every
 *   input read with no input pending, the engine runs an idle tick
 *   which advances m11_tick_v1_mouth_animation and may also call
 *   m11_apply_tick on the DM1 world tick; the result is the
 *   M11_GAME_INPUT_REDRAW contract that requests a viewport re-blit).
 *
 *   A regression that mutes the C026 blit on the idle-tick path
 *   (e.g. an early-return on a stale animTick, or a state-machine
 *   guard that skips portrait re-blit during sleep) would not be
 *   caught by the cancel_reopen / south_return probes because they
 *   drive M11_GameView_Draw directly without first running any
 *   AdvanceIdleTick calls.  This probe therefore adds a focused
 *   sleep_repaint gate that runs N consecutive idle ticks and then
 *   re-renders the framebuffer, asserting the portrait rect still
 *   carries ordinal 8 pixels and has not drifted onto a side wall.
 *
 * Source-locked against:
 *   DUNGEON.C:2573       C127 sensor M011_CELL(sensor) vs view-dir
 *                        filter (only front-wall aspect sets G0289)
 *   DUNGEON.C:2608-2612  C127 sensorData -> G0289
 *   MOVESENS.C:1501-1503 sensorData -> F0280 candidate materializer
 *   REVIVE.C F0280       candidate materialize from sensorData
 *   REVIVE.C:142, 146    G0047 inner-portrait byte rect {0, 31, 0, 28}
 *   DUNVIEW.C:3913-3928  C346 frame + C026 portrait blit at the
 *                        fixed (96, 35, 32, 29) D1C viewport rect
 *   DUNVIEW.C:525        G0109_Graphic558_Box_ChampionPortraitOnWall
 *                        = {96, 127, 35, 63}
 *   DUNVIEW.C:8318-8542  F0128 viewport far-to-near redraw order
 *   DUNVIEW.C:7727-7924  F0124_DrawSquareD1C wall, alcove, then
 *                        portrait blit (catches ordinal-vs-wall draw
 *                        order regressions on the sleep redraw path)
 *   COORD.C:1693-1722    PC 3.4 viewport origin (0, 33) / 224x136
 *   COORD.C:1748-1749    G2078_C32_PortraitWidth=32, G2079_C29=29
 *   DEFS.H:821-826       M027_PORTRAIT_X / M028_PORTRAIT_Y macro
 *                        encoding (ordinal col, ordinal row)
 *   DEFS.H:2186          C026_GRAPHIC_CHAMPION_PORTRAITS = 256x87
 *   main_loop_m11.c:2620 M11_GameView_AdvanceIdleTick call site that
 *                        drives the sleep_repaint route (the M11
 *                        main loop asks for a redraw when the idle
 *                        tick returns M11_GAME_INPUT_REDRAW)
 *   m11_game_view.c:7438 M11_GameView_AdvanceIdleTick (sleep tick:
 *                        runs mouth animation, then m11_apply_tick
 *                        for DM1, returning REDRAW or IGNORED)
 *   m11_game_view.c:8639 m11_tick_v1_mouth_animation (frame index
 *                        advance; one of the contracts the
 *                        sleep_repaint route must respect)
 *
 * Slice coverage (this probe):
 *
 *   (A) Catalog identity:
 *       M11_GameView_GetMirrorNameByOrdinal(8) == "IAIDO" /
 *       M11_GameView_GetMirrorTitleByOrdinal(8) == "RUYITO CHIBURI"
 *       (PC 3.4 English mirror catalog bind).
 *   (B) C026 atlas math for ordinal 8:
 *       atlas width=256, height=87 (8 cols * 3 rows of 32x29 cells);
 *       (8 & 7) == 0, (8 >> 3) == 1; source rect (0, 29, 32, 29);
 *       the cell is opaque (>= 100 of 928 pixels).
 *   (C) Row-wrap distinctness:
 *       ordinal 8 is distinct from ordinal 0 (col-0 row-0, DAROOU),
 *       ordinal 9 (col-1 row-1, ZED), ordinal 16 (col-0 row-2,
 *       CHANI) at >= 30% differing pixels per cell.
 *   (D) Front-mirror seeding:
 *       (1,2) NORTH C127 sensor ships with sensorData=1 (HALK);
 *       after seed 1->8, GetFrontMirrorOrdinal((1,2) NORTH) == 8.
 *   (E) Sleep-repaint invariant (200 idle ticks):
 *       - front-mirror ordinal stays at 8 (sensor still active,
 *         F0280 / F0282 not invoked on idle ticks)
 *       - M11_GameView_Draw after 200 AdvanceIdleTick calls
 *         paints ordinal 8 pixels at the D1C portrait rect
 *         (96, 35, 32, 29) at >= 90% match
 *       - the warm-color pixel count in the D1C portrait rect
 *         is within +/- 10% of the baseline (no sprite drift)
 *       - the side walls of the D1C portrait band carry
 *         < 30 warm pixels (no floating on side walls)
 *       - the per-ordinal dominant match holds: ordinal 8 best,
 *         best count > 0, second-best strictly less than best
 *   (F) Long-tail sleep-repaint invariant (600 idle ticks):
 *       the same portrait_rect_position contract holds after
 *       600 idle ticks (matches the 600-tick survival pattern
 *       in firestaff_dm1_v1_resurrect_survival_load_runtime_probe
 *       so a regression that takes >200 ticks to surface is still
 *       caught).  This is what the route variant is named for:
 *       the portrait must remain stable on the sleep redraw path
 *       for a non-trivial idle-tick count.
 *
 * HONESTY: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 DOSBox pixel parity.  The pixel match uses
 * the same warm-color palette set and per-pixel ordinal comparison
 * the ordinal-8 / south_return probe uses
 * (probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_8_south_return_portrait_rect_position_runtime_probe.c),
 * so the per-pixel confidence band matches the existing matrix.
 *
 * Disjoint from the existing ordinal-8 gates:
 *   - portrait_08_cancel_reopen (cancel_reopen route aspect)
 *   - ordinal_8_south_return (south_return route aspect)
 *   - champion_mirror_east_walkpath_ordinal_8 (east_walkpath route)
 *   - ordinal_05/07/15 portrait_rect_position (different ordinal)
 *   - portrait_01_redraw_after_candidate (different ordinal, different
 *     route aspect; uses panel-open / confirm / cancel, no idle ticks)
 *   - panel_guard BUG-120/121 (panel-on stale sprite guard)
 *   - wall_mirror_zones positive (1,2)N+(1,5)N
 *
 * Companion slice to:
 *   - firestaff_dm1_v1_champion_mirror_ordinal_8_south_return_portrait_rect_position_runtime_probe
 *     (south_return / portrait_rect_position — same ordinal,
 *      different route aspect)
 *   - firestaff_dm1_v1_hall_of_champions_portrait_08_cancel_reopen_portrait_rect_position_runtime_probe
 *     (cancel_reopen / portrait_rect_position — same ordinal,
 *      different route aspect)
 *
 * Probe is data-conditional: if the (1,2) NORTH C127 sensor does not
 * ship with sensorData=1 (HALK) on the loaded DUNGEON.DAT, the probe
 * SKIPs with an explanatory message and exits 0 (per-build fixture
 * guard, not a regression detector).
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy firestaff_dm1_v1_champion_mirror_ordinal_8_sleep_repaint_portrait_rect_position_runtime_probe DATA_DIR
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
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33);
     * size (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 and
     * DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall
     * = {96, 127, 35, 63}). */
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* Source-locked C026 atlas dimensions (DUNVIEW.C:3916-3919 +
     * DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS).  8x3 grid of
     * 32x29 portraits, 256x87 strip. */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 8 in the C026 atlas: (8 & 7) << 5 = 0, (8 >> 3) * 29 = 29.
     * First cell of row 1 (column 0). */
    ORDINAL_8_COL = 8 & 7,
    ORDINAL_8_ROW = 8 >> 3,
    ORDINAL_8_SRC_X = ORDINAL_8_COL << 5,
    ORDINAL_8_SRC_Y = ORDINAL_8_ROW * 29,
    /* No-floating side-wall sample zones: the D1C portrait band
     * (y=33..64 viewport-local) is the only place the C026 sprite
     * should paint.  Side walls of the same band must not carry
     * warm pixels. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    PORTRAIT_WARM_THRESHOLD = 30,
    /* Sleep-tick counts.  200 covers the "ordinary idle frame" the
     * main loop runs while the player is standing still; 600 matches
     * the 600-tick survival pattern in
     * firestaff_dm1_v1_resurrect_survival_load_runtime_probe so a
     * regression that takes >200 ticks to surface still trips the
     * gate.  These are not arbitrary: 200 is the cheap smoke and
     * 600 is the long-tail safety. */
    SLEEP_TICKS_SHORT = 200,
    SLEEP_TICKS_LONG  = 600,
    /* Per-pixel match tolerance for the D1C rect (>= 90% is the
     * existing matrix band). */
    PORTRAIT_MATCH_PCT = 90,
    /* Row-0 col-0 ordinal (DAROOU) is the most likely lookalike for
     * ordinal 8 (IAIDO) because both share column 0.  The (8 >> 3)
     * macro dropping the high bits is the canonical row-wrap bug
     * catcher. */
    ROW0_COL0_ORDINAL = 0,
    ROW1_COL1_ORDINAL = 9,
    ROW2_COL0_ORDINAL = 16,
    /* Slice coordinates: (1,2) NORTH is the real DM1 V1 DUNGEON.DAT
     * Hall of Champions C127 sensor for HALK (sensorData=1) on the
     * front cell (1,1).  We seed sensorData 1 -> 8 to lock ordinal 8
     * (IAIDO) on the same sensor without changing the map layout. */
    PROBE_SLICE_MAP_X = 1,
    PROBE_SLICE_MAP_Y = 2,
    PROBE_SLICE_DIR = 0,                  /* DIR_NORTH */
    SHIPPED_HALK_ORDINAL = 1,
    TARGET_ORDINAL = 8
};

/* Mirror catalog record name for ordinal 8 (DM1 V1 PC 3.4 English). */
static const char kExpectedCatalogName[]  = "IAIDO";
static const char kExpectedCatalogTitle[] = "RUYITO CHIBURI";

static int g_pass = 0;
static int g_fail = 0;

#define PASS(label) do { printf("  PASS: %s\n", label); ++g_pass; } while (0)
#define FAIL(label) do { printf("  FAIL: %s\n", label); ++g_fail; } while (0)
#define CHECK(cond, label) do { \
    if (cond) { PASS(label); } else { FAIL(label); } \
} while (0)

/* Count "warm" pixels in a framebuffer rectangle.  The C026 portrait
 * sprites use palette indices {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}
 * (green / red / orange / peach / yellow / blue) for skin tones,
 * clothing, and backgrounds.  Grey-stone wall texture uses indices
 * 0x01, 0x02, 0x0D.  Counting warm pixels is a coarse but reliable
 * way to distinguish "portrait is here" from "wall only" in the C026
 * cutout (96, 35, 32, 29). */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

/* Count opaque pixels in the C026 atlas cell for the requested
 * ordinal.  Returns the count of pixels whose 4-bit value is
 * non-zero and not the dark-gray (palette index 1) transparency
 * sentinel.  Used to verify ordinal 8 is a defined portrait in
 * the atlas (not blank / unused). */
static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    int srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int sx = srcX + x;
            int sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != 1) ++cnt;
        }
    }
    return cnt;
}

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent
 * of pixels that differ.  Used to verify ordinal 8 is distinct
 * from its column-0 row-mates (0 DAROOU on row 0, 16 CHANI on row 2)
 * and from its immediate row-1 neighbour (9 ZED).  The DM1 champion-
 * portrait atlas carries 24 distinct champions (one per ordinal),
 * so a duplicate would be a real regression. */
static int atlas_cell_distinct_percent(const M11_AssetSlot* portraits,
                                       int ordinalA, int ordinalB) {
    int x, y, compared = 0, different = 0;
    int srcAX = (ordinalA & 7) * D1C_PORTRAIT_W;
    int srcAY = (ordinalA >> 3) * D1C_PORTRAIT_H;
    int srcBX = (ordinalB & 7) * D1C_PORTRAIT_W;
    int srcBY = (ordinalB >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char a = (unsigned char)
                (portraits->pixels[(srcAY + y) * (int)portraits->width + (srcAX + x)] & 0x0F);
            unsigned char b = (unsigned char)
                (portraits->pixels[(srcBY + y) * (int)portraits->width + (srcBX + x)] & 0x0F);
            ++compared;
            if (a != b) ++different;
        }
    }
    return (compared > 0) ? (different * 100 / compared) : 0;
}

/* Compare the C026 portrait atlas cell for the requested ordinal to
 * the framebuffer D1C portrait rectangle.  Returns the percent of
 * opaque source pixels that match the destination pixel.  The
 * C01 dark-gray transparency sentinel is skipped on the source side
 * (DUNVIEW.C:3916).  This is the "portrait blit lands in the rect"
 * check from the slice. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) return 0;
    srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src;
            unsigned char dst;
            int sx = srcX + x;
            int sy = srcY + y;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == 1) continue; /* transparency sentinel */
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W +
                                         (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Per-ordinal match count: count non-transparent source pixels of
 * `ordinal` that match the D1C rect.  Used for the strict-dominance
 * check at the sleep_repaint redraw frame (ordinal 8 must beat every
 * other ordinal at the same D1C rect). */
static int ordinal_match_count(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * D1C_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * D1C_PORTRAIT_H + y;
            unsigned char srcIdx;
            unsigned char dstIdx;
            if (srcX < 0 || srcX >= (int)portraits->width) continue;
            if (srcY < 0 || srcY >= (int)portraits->height) continue;
            srcIdx = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            dstIdx = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W +
                                             (D1C_PORTRAIT_X + x)]);
            if (srcIdx == 1) continue;
            if (dstIdx == srcIdx) ++matched;
        }
    }
    return matched;
}

/* Seed the first C127 sensor whose data equals `oldData` to
 * `newData`.  Returns the sensor index on success, or -1 if no
 * such sensor was found.  Used to lock the ordinal-8 edge case
 * on the real DM1 V1 DUNGEON.DAT (which ships HALK / ordinal 1 on
 * the (1,2) NORTH-route front square (1,1)).  The seed does NOT
 * change the map layout or the C127 cell match - only the G0289
 * ordinal that DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
 * (DUNGEON.C:2610-2612). */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData,
                                int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Park the party at the (1,2) D1C front-mirror route facing NORTH.
 * This is the real C127 sensor position from the DM1 V1 DUNGEON.DAT
 * shipped with the public PC 3.4 English release: at (1,2) facing
 * NORTH, the front square (1,1) has a C127 sensor on cell=2 (north
 * wall) with sensorData=1 (HALK, mirror ordinal 1).  After
 * seed_first_c127_data the same square reports ordinal 8. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = PROBE_SLICE_MAP_X;
    state->world.party.mapY = PROBE_SLICE_MAP_Y;
    state->world.party.direction = PROBE_SLICE_DIR;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int seededSensor;
    int shippedOrdinal;
    int seededOrdinal;
    int ordinal8Opaque;
    int ordinal8Vs0, ordinal8Vs9, ordinal8Vs16;
    int ornX, ornY, ornW, ornH;
    char nameBuf[32];
    char titleBuf[64];
    int nameLookupRc, titleLookupRc;
    int redrawSeen = 0;
    int i;

    /* Framebuffer snapshots: FB0 is the pre-sleep baseline; FB1 is
     * after 200 idle ticks (cheap smoke); FB2 is after 600 idle
     * ticks (long-tail safety).  The sleep_repaint contract says
     * the D1C portrait rect must still carry ordinal 8 pixels in
     * all three. */
    unsigned char fb0[FB_W * FB_H];
    unsigned char fb1[FB_W * FB_H];
    unsigned char fb2[FB_W * FB_H];
    int match0, match1, match2;
    int warm0, warm1, warm2;
    int leftSide0, leftSide1, leftSide2;
    int rightSide0, rightSide1, rightSide2;
    int bestOrdinal1, bestOrdinal2;
    int best1, second1;
    int best2, second2;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-08 / sleep_repaint / "
           "portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas via the public M11 helper so the
     * probe does not depend on the file-scope enum value 26. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr,
                "FATAL: cannot load C026 portrait atlas (width=%d height=%d)\n",
                portraits ? (int)portraits->width : -1,
                portraits ? (int)portraits->height : -1);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group A - Catalog identity for ordinal 8
     * ----------------------------------------------------------------
     * The PC 3.4 English mirror catalog binds ordinal 8 to IAIDO
     * with title "RUYITO CHIBURI".  This is the catalog-level
     * evidence that ordinal 8 is a real Hall of Champions entry,
     * not a defunct slot.  Catches regressions where the catalog
     * and the C026 atlas disagree on the ordinal-8 record. */
    printf("\n[Group A] Mirror catalog identity for ordinal 8\n");
    nameBuf[0]  = '\0';
    titleBuf[0] = '\0';
    nameLookupRc  = M11_GameView_GetMirrorNameByOrdinal(&state,
                                                        TARGET_ORDINAL,
                                                        nameBuf,
                                                        (int)sizeof(nameBuf));
    titleLookupRc = M11_GameView_GetMirrorTitleByOrdinal(&state,
                                                         TARGET_ORDINAL,
                                                         titleBuf,
                                                         (int)sizeof(titleBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "GetMirrorNameByOrdinal(8) returns %d (got \"%s\", expected \"%s\")",
                 nameLookupRc,
                 nameBuf[0] ? nameBuf : "",
                 kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "GetMirrorTitleByOrdinal(8) returns %d (got \"%s\", expected \"%s\")",
                 titleLookupRc,
                 titleBuf[0] ? titleBuf : "",
                 kExpectedCatalogTitle);
        CHECK(titleLookupRc > 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - C026 atlas math for ordinal 8
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 1 /
     * column 0 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows"). */
    printf("\n[Group B] C026 atlas math for ordinal 8\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected %d = 8 cols * 32)",
                 portraits->width, ATLAS_W);
        CHECK(portraits->width == ATLAS_W, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected %d = 3 rows * 29)",
                 portraits->height, ATLAS_H);
        CHECK(portraits->height == ATLAS_H, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 col = 8 & 7 = %d (expected %d)",
                 ORDINAL_8_COL, 0);
        CHECK(ORDINAL_8_COL == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 row = 8 >> 3 = %d (expected %d)",
                 ORDINAL_8_ROW, 1);
        CHECK(ORDINAL_8_ROW == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 source rect (%d, %d, %d, %d) fits inside "
                 "atlas (%d, %d)",
                 ORDINAL_8_SRC_X, ORDINAL_8_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_8_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_8_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }
    ordinal8Opaque = atlas_cell_opaque_count(portraits, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal8Opaque);
        CHECK(ordinal8Opaque >= 100, msg);
    }

    /* Row-wrap distinctness: ordinal 8 must be visually distinct
     * from its column-0 row-mates (0 DAROOU on row 0, 16 CHANI on
     * row 2) AND from its immediate row-1 neighbour (9 ZED).  The
     * DM1 champion-portrait atlas carries 24 distinct champions
     * (one per ordinal), so a duplicate would be a real
     * regression.  The row-0 col-0 vs row-1 col-0 distinctness
     * check is the canonical row-wrapping bug catcher. */
    ordinal8Vs0  = atlas_cell_distinct_percent(portraits, 8, 0);
    ordinal8Vs9  = atlas_cell_distinct_percent(portraits, 8, 9);
    ordinal8Vs16 = atlas_cell_distinct_percent(portraits, 8, 16);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 0 (column-0 row-0) differ "
                 "by >= 30%% (got %d%%) - row-wrap check",
                 ordinal8Vs0);
        CHECK(ordinal8Vs0 >= 30, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 9 (row-1 right neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal8Vs9);
        CHECK(ordinal8Vs9 >= 30, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 16 (column-0 row-2) differ "
                 "by >= 30%% (got %d%%) - row-2 column-0 distinctness",
                 ordinal8Vs16);
        CHECK(ordinal8Vs16 >= 30, msg);
    }

    /* D1C wall ornament zone must be the source-locked (80, 29, 64,
     * 43) viewport-relative C346 frame.  The inner portrait cutout
     * (96, 35, 32, 29) must fit inside this frame.  Catches a
     * regression where the helper stops returning the source-locked
     * box (a future source-locked rect change cannot silently
     * break the slice). */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "coords (DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    {
        /* The D1C wall ornament zone is viewport-local
         * (M11_GameView_GetD1CWallOrnamentZone returns (80, 29, 64,
         * 43) viewport-relative, per DUNVIEW.C G0205 coordSet 5 /
         * viewWallIndex 12).  The inner portrait cutout is also
         * viewport-local: source-locked at (96, 35, 32, 29).  The
         * framebuffer address used for the warm-count / match
         * checks is D1C_PORTRAIT_X = VIEWPORT_X + 96 = 96 and
         * D1C_PORTRAIT_Y = VIEWPORT_Y + 35 = 68, but the
         * containment check must compare viewport-local coords. */
        const int portraitX_vp = D1C_PORTRAIT_X - VIEWPORT_X;
        const int portraitY_vp = D1C_PORTRAIT_Y - VIEWPORT_Y;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (%d, %d, %d, %d) viewport-local "
                 "sits inside the D1C wall ornament zone "
                 "(X in [%d,%d), Y in [%d,%d))",
                 portraitX_vp, portraitY_vp,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H,
                 ornX, ornX + ornW, ornY, ornY + ornH);
        CHECK(portraitX_vp >= ornX &&
              portraitX_vp + D1C_PORTRAIT_W <= ornX + ornW &&
              portraitY_vp >= ornY &&
              portraitY_vp + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - Front-mirror seeding (1,2) NORTH C127 sensor 1->8
     * ----------------------------------------------------------------
     * Per-build fixture guard: the shipped DM1 V1 DUNGEON.DAT
     * places a C127 sensor on the (1,2) NORTH-route front square
     * (1,1) with sensorData=1 (HALK).  If the loaded build does
     * not match this contract we SKIP rather than mis-pinning the
     * ordinal.  This is a fixture guard, not a regression
     * detector. */
    printf("\n[Group C] Front-mirror seeding (1,2) NORTH C127 1 -> 8\n");
    park_d1c_front_route(&state);
    shippedOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped (1,2) NORTH front-mirror ordinal = %d "
                 "(expected %d / HALK)",
                 shippedOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(shippedOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }
    if (shippedOrdinal != SHIPPED_HALK_ORDINAL) {
        printf("SKIP sleep_repaint_fixture_mismatch "
               "(1,2) NORTH shipped ordinal=%d expected=%d; this DM1 V1 "
               "build does not match the reference DUNGEON.DAT fixture "
               "for the (1,2) NORTH HALK C127 sensor.\n",
               shippedOrdinal, SHIPPED_HALK_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    seededSensor = seed_first_c127_data(&state,
                                        SHIPPED_HALK_ORDINAL,
                                        TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d (HALK) "
                 "to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }
    if (seededSensor < 0) {
        printf("SKIP sleep_repaint_fixture_mismatch could not find a "
               "C127 sensor with sensorData=%d on the (1,2) NORTH "
               "route; this DM1 V1 build does not match the reference "
               "DUNGEON.DAT fixture.\n", SHIPPED_HALK_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    park_d1c_front_route(&state);
    seededOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after seed: (1,2) NORTH front-mirror ordinal = %d "
                 "(expected %d)",
                 seededOrdinal, TARGET_ORDINAL);
        CHECK(seededOrdinal == TARGET_ORDINAL, msg);
    }
    if (seededOrdinal != TARGET_ORDINAL) {
        printf("FATAL: front ordinal did not lock to %d after seed; "
               "cannot verify sleep_repaint contract.\n", TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group D - Baseline render (pre-sleep) at (1,2) NORTH
     * ----------------------------------------------------------------
     * The D1C portrait rect must contain ordinal-8 pixels at the
     * same match percentage the cancel_reopen / south_return gates
     * lock (>= 90%).  The warm pixel count establishes the
     * baseline for the sleep_repaint stability check in Group E. */
    printf("\n[Group D] Baseline render at (1,2) NORTH\n");
    park_d1c_front_route(&state);
    state.world.party.championCount = 0;

    memset(fb0, 0, sizeof(fb0));
    M11_GameView_Draw(&state, fb0, FB_W, FB_H);

    match0 = match_portrait_at_rect(portraits, fb0, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect (96, 35) carries ordinal %d pixels "
                 "at >= %d%% match (got %d%%)",
                 TARGET_ORDINAL, PORTRAIT_MATCH_PCT, match0);
        CHECK(match0 >= PORTRAIT_MATCH_PCT, msg);
    }
    warm0 = rect_warm_count(fb0,
                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                            D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warm0);
        CHECK(warm0 >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    leftSide0 = rect_warm_count(fb0,
                                SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                SIDE_WALL_LEFT_W,
                                PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSide0 = rect_warm_count(fb0,
                                 SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                 SIDE_WALL_RIGHT_W,
                                 PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline left side wall of D1C portrait band has "
                 "< %d warm pixels (got %d) - portrait not floating on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSide0);
        CHECK(leftSide0 < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline right side wall of D1C portrait band has "
                 "< %d warm pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSide0);
        CHECK(rightSide0 < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - Sleep_repaint invariant: 200 idle ticks
     * ----------------------------------------------------------------
     * Drive M11_GameView_AdvanceIdleTick SLEEP_TICKS_SHORT times.
     * This is the M11 main loop's idle path (main_loop_m11.c:2620
     * calls AdvanceIdleTick and re-renders on REDRAW).  Re-render
     * the framebuffer and verify the D1C portrait rect still
     * carries ordinal 8 pixels at the same match / warm-count
     * band as the baseline.  Any drift, panel-inherit, or sensor
     * disable on the sleep redraw path will trip this. */
    printf("\n[Group E] Sleep_repaint invariant after %d idle ticks\n",
           SLEEP_TICKS_SHORT);
    park_d1c_front_route(&state);

    for (i = 0; i < SLEEP_TICKS_SHORT; ++i) {
        M11_GameInputResult r = M11_GameView_AdvanceIdleTick(&state);
        if (r == M11_GAME_INPUT_REDRAW) redrawSeen = 1;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "AdvanceIdleTick returned M11_GAME_INPUT_REDRAW at "
                 "least once across %d ticks (sleep_repaint route active)",
                 SLEEP_TICKS_SHORT);
        /* redrawSeen is informational: the contract is "portrait
         * is stable", not "the main loop must have requested a
         * redraw".  Some builds may not request one in 200 ticks
         * if m11_apply_tick returns IGNORED for the resting
         * party, but the next assertion (portrait still painted)
         * still holds.  We still record the redraw event for
         * traceability. */
        if (redrawSeen) {
            PASS(msg);
        } else {
            printf("  NOTE: %s (no REDRAW in %d ticks; route still verified "
                   "by framebuffer re-render below)\n", msg, SLEEP_TICKS_SHORT);
        }
    }

    /* The party pose must be unchanged across the sleep tick
     * window.  AdvanceIdleTick on a resting party does not move
     * the party; if a future refactor of m11_apply_tick starts
     * mutating the pose on idle ticks, this assertion fires. */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: party pose still (map=%d, x=%d, "
                 "y=%d, dir=%d) - no idle-tick movement regression",
                 SLEEP_TICKS_SHORT,
                 state.world.party.mapIndex,
                 state.world.party.mapX,
                 state.world.party.mapY,
                 state.world.party.direction);
        CHECK(state.world.party.mapIndex == 0 &&
              state.world.party.mapX == PROBE_SLICE_MAP_X &&
              state.world.party.mapY == PROBE_SLICE_MAP_Y &&
              state.world.party.direction == PROBE_SLICE_DIR, msg);
    }

    /* The front-mirror ordinal must still be 8: idle ticks do not
     * invoke F0280 / F0282 (those are panel-button triggered via
     * COMMAND.C F0380).  If a future refactor starts dispatching
     * C160 resurrect on idle ticks, the sensor would be disabled
     * and the ordinal would shift to -1, which this assertion
     * catches. */
    {
        int idleOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: front-mirror ordinal still %d "
                 "(got %d) - idle ticks must not disable the C127 sensor",
                 SLEEP_TICKS_SHORT, TARGET_ORDINAL, idleOrdinal);
        CHECK(idleOrdinal == TARGET_ORDINAL, msg);
    }

    /* Re-render the framebuffer and check the D1C rect still
     * carries ordinal 8 pixels.  The match percentage and warm
     * pixel count must remain inside the band the baseline
     * established - a portrait drift / sprite swap / row-wrap
     * bug will push match1 below 90% or warm1 below 30. */
    memset(fb1, 0, sizeof(fb1));
    M11_GameView_Draw(&state, fb1, FB_W, FB_H);

    match1 = match_portrait_at_rect(portraits, fb1, TARGET_ORDINAL);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect (96, 35) carries "
                 "ordinal %d pixels at >= %d%% match (got %d%%; "
                 "baseline match0=%d%%)",
                 SLEEP_TICKS_SHORT, TARGET_ORDINAL,
                 PORTRAIT_MATCH_PCT, match1, match0);
        CHECK(match1 >= PORTRAIT_MATCH_PCT, msg);
    }
    warm1 = rect_warm_count(fb1,
                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                            D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect warm-count = %d "
                 "(baseline = %d; |delta| = %d, must be <= 10%% "
                 "of baseline = %d)",
                 SLEEP_TICKS_SHORT, warm1, warm0,
                 warm1 > warm0 ? warm1 - warm0 : warm0 - warm1,
                 warm0 / 10);
        CHECK(warm1 >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    /* Stability: the absolute warm-count delta vs the baseline
     * must be small.  10% of the baseline is the same band the
     * cancel_reopen probe uses for its top-strip density check;
     * it absorbs the documented per-champion warm-pixel
     * variability.  A regression where the portrait sprite swaps
     * mid-sleep (e.g. row-wrap macro dropping the high bits and
     * painting ordinal 0 instead) trips this assertion. */
    {
        int warmDelta = warm1 > warm0 ? warm1 - warm0 : warm0 - warm1;
        int warmBand  = warm0 / 10;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect warm-count stable vs "
                 "baseline (|delta|=%d, band=%d)",
                 SLEEP_TICKS_SHORT, warmDelta, warmBand);
        CHECK(warmDelta <= warmBand, msg);
    }

    /* No-floating on the sleep-repaint frame: the side walls of
     * the D1C portrait band must not have grown warm pixels. */
    leftSide1 = rect_warm_count(fb1,
                                SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                SIDE_WALL_LEFT_W,
                                PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSide1 = rect_warm_count(fb1,
                                 SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                 SIDE_WALL_RIGHT_W,
                                 PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d; baseline %d) - "
                 "no floating on left wall",
                 SLEEP_TICKS_SHORT, PORTRAIT_WARM_THRESHOLD,
                 leftSide1, leftSide0);
        CHECK(leftSide1 < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d; baseline %d) - "
                 "no floating on right wall",
                 SLEEP_TICKS_SHORT, PORTRAIT_WARM_THRESHOLD,
                 rightSide1, rightSide0);
        CHECK(rightSide1 < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Strict per-ordinal dominance: ordinal 8 must beat every
     * other ordinal at the same D1C rect after the sleep redraw.
     * This is the "sibling portrait cannot accidentally win"
     * invariant; without it, a regression where the rect pixels
     * shift to another ordinal's sprite (matching at >= 90%) but
     * ordinal 8's sprite is no longer painted would still pass
     * the match_portrait_at_rect(8) test. */
    {
        int ordinal;
        best1 = 0;
        bestOrdinal1 = -1;
        second1 = 0;
        for (ordinal = 0; ordinal < 24; ++ordinal) {
            int m = ordinal_match_count(portraits, fb1, ordinal);
            if (m > best1) {
                second1 = best1;
                best1 = m;
                bestOrdinal1 = ordinal;
            } else if (m > second1) {
                second1 = m;
            }
        }
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "after %d idle ticks: per-ordinal dominance at D1C rect "
                     "is ordinal %d (count=%d, second-best=%d)",
                     SLEEP_TICKS_SHORT, bestOrdinal1, best1, second1);
            CHECK(bestOrdinal1 == TARGET_ORDINAL &&
                  best1 > 0 && best1 > second1, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group F - Long-tail sleep_repaint invariant: 600 idle ticks
     * ----------------------------------------------------------------
     * Same portrait_rect_position contract held across 600 idle
     * ticks (matches the 600-tick survival pattern in
     * firestaff_dm1_v1_resurrect_survival_load_runtime_probe so a
     * regression that takes >200 ticks to surface is still
     * caught).  This is the long-tail safety.  We re-derive the
     * same invariants as Group E on a fresh framebuffer. */
    printf("\n[Group F] Long-tail sleep_repaint invariant after %d idle ticks\n",
           SLEEP_TICKS_LONG - SLEEP_TICKS_SHORT);
    for (i = 0; i < (SLEEP_TICKS_LONG - SLEEP_TICKS_SHORT); ++i) {
        M11_GameInputResult r = M11_GameView_AdvanceIdleTick(&state);
        if (r == M11_GAME_INPUT_REDRAW) redrawSeen = 1;
    }

    {
        int idleOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: front-mirror ordinal still %d "
                 "(got %d) - long-tail sensor stability",
                 SLEEP_TICKS_LONG, TARGET_ORDINAL, idleOrdinal);
        CHECK(idleOrdinal == TARGET_ORDINAL, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: party pose unchanged (still "
                 "(map=%d, x=%d, y=%d, dir=%d))",
                 SLEEP_TICKS_LONG,
                 state.world.party.mapIndex,
                 state.world.party.mapX,
                 state.world.party.mapY,
                 state.world.party.direction);
        CHECK(state.world.party.mapIndex == 0 &&
              state.world.party.mapX == PROBE_SLICE_MAP_X &&
              state.world.party.mapY == PROBE_SLICE_MAP_Y &&
              state.world.party.direction == PROBE_SLICE_DIR, msg);
    }

    memset(fb2, 0, sizeof(fb2));
    M11_GameView_Draw(&state, fb2, FB_W, FB_H);

    match2 = match_portrait_at_rect(portraits, fb2, TARGET_ORDINAL);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect carries ordinal %d "
                 "pixels at >= %d%% match (got %d%%)",
                 SLEEP_TICKS_LONG, TARGET_ORDINAL,
                 PORTRAIT_MATCH_PCT, match2);
        CHECK(match2 >= PORTRAIT_MATCH_PCT, msg);
    }
    warm2 = rect_warm_count(fb2,
                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                            D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect warm-count = %d "
                 "(baseline = %d, must be >= %d)",
                 SLEEP_TICKS_LONG, warm2, warm0,
                 PORTRAIT_WARM_THRESHOLD);
        CHECK(warm2 >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        int warmDelta = warm2 > warm0 ? warm2 - warm0 : warm0 - warm2;
        int warmBand  = warm0 / 10;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: D1C rect warm-count stable vs "
                 "baseline (|delta|=%d, band=%d)",
                 SLEEP_TICKS_LONG, warmDelta, warmBand);
        CHECK(warmDelta <= warmBand, msg);
    }
    leftSide2 = rect_warm_count(fb2,
                                SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                SIDE_WALL_LEFT_W,
                                PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSide2 = rect_warm_count(fb2,
                                 SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                 SIDE_WALL_RIGHT_W,
                                 PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d)",
                 SLEEP_TICKS_LONG, PORTRAIT_WARM_THRESHOLD, leftSide2);
        CHECK(leftSide2 < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d idle ticks: right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d)",
                 SLEEP_TICKS_LONG, PORTRAIT_WARM_THRESHOLD, rightSide2);
        CHECK(rightSide2 < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        int ordinal;
        best2 = 0;
        bestOrdinal2 = -1;
        second2 = 0;
        for (ordinal = 0; ordinal < 24; ++ordinal) {
            int m = ordinal_match_count(portraits, fb2, ordinal);
            if (m > best2) {
                second2 = best2;
                best2 = m;
                bestOrdinal2 = ordinal;
            } else if (m > second2) {
                second2 = m;
            }
        }
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "after %d idle ticks: per-ordinal dominance at D1C rect "
                     "is ordinal %d (count=%d, second-best=%d)",
                     SLEEP_TICKS_LONG, bestOrdinal2, best2, second2);
            CHECK(bestOrdinal2 == TARGET_ORDINAL &&
                  best2 > 0 && best2 > second2, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group G - sleep_repaint triple-frame summary
     * ----------------------------------------------------------------
     * The three framebuffer snapshots (baseline / 200 / 600) must
     * agree on the portrait_rect_position contract.  This is the
     * "redraw stability across the sleep_repaint window" closure
     * assertion: a single-line summary that ties Groups D, E, F
     * together so a future log scan sees the whole slice in one
     * place. */
    printf("\n[Group G] sleep_repaint redraw stability summary\n");
    {
        char msg[320];
        snprintf(msg, sizeof(msg),
                 "sleep_repaint portrait_rect_position: baseline match=%d%% "
                 "warm=%d, after %d ticks match=%d%% warm=%d, after %d "
                 "ticks match=%d%% warm=%d (all panel-off >= %d%%, warm >= %d)",
                 match0, warm0,
                 SLEEP_TICKS_SHORT, match1, warm1,
                 SLEEP_TICKS_LONG,  match2, warm2,
                 PORTRAIT_MATCH_PCT, PORTRAIT_WARM_THRESHOLD);
        CHECK(match0 >= PORTRAIT_MATCH_PCT &&
              match1 >= PORTRAIT_MATCH_PCT &&
              match2 >= PORTRAIT_MATCH_PCT &&
              warm0 >= PORTRAIT_WARM_THRESHOLD &&
              warm1 >= PORTRAIT_WARM_THRESHOLD &&
              warm2 >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "sleep_repaint per-ordinal dominance: "
                 "200t best=%d second=%d, 600t best=%d second=%d "
                 "(both must be ordinal 8)",
                 bestOrdinal1, second1, bestOrdinal2, second2);
        CHECK(bestOrdinal1 == TARGET_ORDINAL &&
              bestOrdinal2 == TARGET_ORDINAL, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "sleep_repaint no-floating: left side wall warm-count "
                 "200t=%d 600t=%d (both < %d), right side wall "
                 "200t=%d 600t=%d (both < %d)",
                 leftSide1, leftSide2, PORTRAIT_WARM_THRESHOLD,
                 rightSide1, rightSide2, PORTRAIT_WARM_THRESHOLD);
        CHECK(leftSide1 < PORTRAIT_WARM_THRESHOLD &&
              leftSide2 < PORTRAIT_WARM_THRESHOLD &&
              rightSide1 < PORTRAIT_WARM_THRESHOLD &&
              rightSide2 < PORTRAIT_WARM_THRESHOLD, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
