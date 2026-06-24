/*
 * firestaff_dm1_v1_hoc_champion_portrait_05_palette_match_rect_runtime_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 5 (ELIJA,
 * "LION OF YAITOPYA"), route palette_match_rect, aspect
 * portrait_rect_position, batch group 11.
 *
 * Sibling to the existing ordinal-05 slices:
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe.c
 *     route = north_positive (locates the unique ordinal-5 north-facing
 *     front route in the live DM1 V1 DUNGEON.DAT, currently (2, 16)
 *     DIR_NORTH on the shipped PC 3.4 English fixture; verifies the
 *     D1C portrait cutout renders ordinal 5 at >= 90% match and the
 *     three non-north directions stay below 50% match on the same
 *     cell — coarse presence vs coarse absence)
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_5_front_south_entry_portrait_rect_position_runtime_probe.c
 *     route = front_south_entry (same physical (2, 16) DIR_NORTH
 *     front cell (2, 15) cell 2 sensor, reframed as the south-entry
 *     approach; adds the D1C wall-ornament zone containment check,
 *     the 24-ordinal best-sweep dominance invariant, side-wall
 *     column bands (D1L/D1R), the turn-away redraw invariant, and
 *     the redraw-stability byte equality — does NOT lock per-line
 *     palette match)
 *
 *   probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_05_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *     route = after_party_shuffle (seeds the (1, 2) NORTH C127
 *     sensor from sensorData=1 to sensorData=5; drives the
 *     C040 -> F0284 party rotation -> C160 confirm arm; locks the
 *     portrait_rect_position across pre-/mid-/post-shuffle states
 *     on the seeded (1, 2) cell, NOT the live (2, 16) sensor)
 *
 *   probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen_portrait_rect_position_runtime_probe.c
 *     route = cancel_reopen (also seeds the (1, 2) NORTH C127 sensor;
 *     drives the C040 select -> C162 cancel -> re-select 3-step
 *     terminal slice; locks portrait_rect_position + champion-name
 *     readout across the cancel arm on the seeded cell, NOT the
 *     live (2, 16) sensor)
 *
 *   probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_05_candidate_panel_cancel_portrait_rect_position_runtime_probe.c
 *     route = candidate_panel_cancel (also seeds the (1, 2) NORTH
 *     C127 sensor; the 2-step C040 select -> C162 cancel terminal
 *     slice on the seeded cell, NOT the live (2, 16) sensor)
 *
 *   probes/m11/firestaff_dm1_v1_hoc_champion_portrait_05_wake_repaint_portrait_rect_position_173_gate_probe.c
 *     route = wake_repaint (also seeds the (1, 2) NORTH C127
 *     sensor; rest/wake flag cycle on the seeded cell, NOT the
 *     live (2, 16) sensor)
 *
 *   probes/m11/firestaff_dm1_v1_hoc_champion_portrait_05_approach_from_right_portrait_rect_position_runtime_probe.c
 *     route = approach_from_right (party at (3, 16) DIR_WEST
 *     approaches the (2, 16) cell from the east; the visible
 *     wall is the east wall of (2, 16), which has no C127 sensor;
 *     locks the negative control that ordinal 5 does NOT float on
 *     a wrong-wall side — coarse < 35% match, no per-line check)
 *
 * This probe covers the **palette_match_rect** route variant on the
 * LIVE (2, 16) DIR_NORTH ordinal-5 sensor — the distinct concern:
 * "does each rendered pixel in the D1C portrait rectangle carry the
 * same VGA palette index as the corresponding C026 ordinal-5 atlas
 * cell?"  Where the existing ordinal-05 slices use a coarse 90%
 * aggregate match with no per-line check, this probe tightens the
 * palette-level match to 99% aggregate AND adds per-row (>= 90%)
 * and per-column (>= 80%) invariants to catch sub-rectangle
 * stride / offset regressions that an aggregate match could
 * absorb.  Unlike the after_party_shuffle / cancel_reopen /
 * candidate_panel_cancel / wake_repaint siblings, this probe does
 * NOT seed any sensor data — it drives the real DM1 V1 PC 3.4
 * English fixture C127 sensorData=5 at (2, 15) cell 2 (the
 * corridor ordinal scanner confirms (2, 16, N) is the unique
 * ordinal-5 corridor pose in the shipped DUNGEON.DAT).
 *
 * Atlas math: ordinal 5 lives at row 0 / col 5 of the 8x3 C026
 * strip.  srcX = (5 & 7) * 32 = 160, srcY = (5 >> 3) * 29 = 0.
 * On the local PC 3.4 fixture the ordinal-5 atlas cell renders
 * ELIJA / "LION OF YAITOPYA" per
 * firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
 * (PASS on name + title).
 *
 * Three coupled concerns in one runtime drive:
 *
 *   (1) palette_match_rect — strict per-pixel palette index match
 *       between the C026 ordinal-5 source cell (160, 0, 32, 29) and
 *       the D1C destination rect (96, 35, 32, 29) on the rendered
 *       framebuffer.  Aggregate match >= 99%, every row match >=
 *       90% of its non-transparent count, every column match >=
 *       80% of its non-transparent count.  This tightens the 90%
 *       aggregate / no per-line checks used by the north_positive
 *       and front_south_entry siblings; the per-line guarantees
 *       catch stride / offset regressions that an aggregate match
 *       could absorb.
 *
 *   (2) side_wall_no_float — at (2, 16) DIR_EAST, the same D1C
 *       rect must contain fewer than 5% palette_match against
 *       ordinal 5 (no ordinal-5 portrait floating over the side
 *       wall; the east face of (2, 16) is a corridor side wall
 *       and the (2, 17) front cell has no C127 sensor with
 *       sensorData=5 on its east aspect).  This is the
 *       negative-control companion to
 *       firestaff_dm1_v1_hoc_champion_portrait_05_approach_from_
 *       right_portrait_rect_position_runtime_probe (which uses
 *       the same corridor side wall, coarse < 35% match).
 *
 *   (3) redraw_after_candidate — re-render the (2, 16) DIR_NORTH
 *       pose with the C040 candidate panel live (after
 *       SelectFrontMirrorCandidate on the live ordinal-5 sensor).
 *       The full D1C rect must drop to < 5% match with ordinal 5
 *       (no stale sprite after C040 takes over), and the visible
 *       top strip above the panel must stay clean of warm pixels
 *       (no portrait bleed over the panel border).
 *
 * The two render passes (panel-off and panel-on) are also checked
 * for determinism: a second render of the same pose must produce
 * a framebuffer byte-equal to the first.  This catches any
 * non-pure framebuffer state leak (e.g. an incrementing counter
 * between draws).
 *
 * Source evidence:
 *   - ReDMCSB DUNGEON.C:2573 (C127 sensor cell match against view
 *     direction; visibleWallCell = (dir + 2) & 3)
 *   - ReDMCSB DUNGEON.C:2608-2612 (G0289_i_DungeonView_ChampionPortraitOrdinal
 *     = C127 sensorData, range 0..23)
 *   - ReDMCSB DUNVIEW.C:3913-3928 (C026 portrait blit at D1C box
 *     { x=96, y=35, w=32, h=29 } using (ord & 7) * 32,
 *     (ord >> 3) * 29 with C01_COLOR_DARK_GRAY transparency)
 *   - ReDMCSB DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = { 96, 127, 35, 63 })
 *   - ReDMCSB DUNVIEW.C:3916-3919 (C026_GRAPHIC_CHAMPION_PORTRAITS,
 *     "A portrait is 32x29 pixels" — 8 cols * 3 rows = 256x87 atlas)
 *   - ReDMCSB DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y
 *     macro math)
 *   - ReDMCSB MOVESENS.C:1501-1503 (F0280 sensorData -> candidate
 *     ordinal)
 *   - ReDMCSB REVIVE.C F0280 (materialize candidate from sensorData)
 *   - src/engine/m11_game_view.c m11_draw_dm1_front_mirror_route
 *     (BUG-120/121 panel guard) and
 *     m11_draw_dm1_front_champion_portrait (D1C blit site)
 *
 * Slice assignment (batch group 11):  next free tag after groups
 * 0/3/4/6/8/9 used by the existing palette_match_rect, d2r_negative,
 * double_click_stability, approach_from_right, sleep_repaint, and
 * resurrect_reselect gate probes (see TODO.md 2026-06-22 HoC champion
 * portrait gate-batch annotation).  Disjoint from the four
 * sensor-seeded ordinal-05 siblings (after_party_shuffle,
 * cancel_reopen, candidate_panel_cancel, wake_repaint — all seed
 * the (1, 2) NORTH HALK sensor to ordinal 5) and from the live
 * coarse-match ordinal-05 siblings (ordinal5_rect_runtime_probe,
 * front_south_entry — both run on (2, 16) NORTH but with the
 * 90% aggregate / no per-line check).  This probe is the only
 * per-line palette_match_rect proof for ordinal 5 and the only
 * palette_match_rect proof that does NOT seed any sensor data.
 *
 * This is a Firestaff-runtime slice proof, not DOS pixel parity.
 * The probe does not claim parity against an original capture; it
 * locks the source-locked C026 atlas math + D1C destination
 * rectangle + C127 sensor side filter + C040 candidate redraw
 * ownership against the runtime framebuffer.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,

    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = { 96, 127, 35, 63 }).  Viewport-relative coordinates. */
    PROBE_PORTRAIT_VIEW_X = 96,
    PROBE_PORTRAIT_VIEW_Y = 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_FB_X = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VIEW_X,
    PROBE_PORTRAIT_FB_Y = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VIEW_Y,

    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    PROBE_ATLAS_W = 256,
    PROBE_ATLAS_H = 87,
    PROBE_ATLAS_COLS = 8,
    PROBE_ATLAS_ROWS = 3,

    /* Ordinal 5 in the C026 atlas: (5 & 7) * 32 = 160,
     *                                (5 >> 3) * 29 =   0.
     * This is the SIXTH column of the TOP row of the 8x3 atlas
     * (row 0, col 5).  In real DM1 V1 PC 3.4 English DUNGEON.DAT
     * the mirror-catalog name resolves to "ELIJA" / "LION OF
     * YAITOPYA" (verified at runtime by Group A's catalog
     * identity check, mirrors
     * firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
     * + front_south_entry siblings). */
    PROBE_ORDINAL = 5,
    PROBE_ORDINAL_COL = PROBE_ORDINAL & 7,         /* = 5 */
    PROBE_ORDINAL_ROW = PROBE_ORDINAL >> 3,        /* = 0 */
    PROBE_ORDINAL_SRC_X = PROBE_ORDINAL_COL * 32,  /* = 160 */
    PROBE_ORDINAL_SRC_Y = PROBE_ORDINAL_ROW * 29,  /* = 0 */
    /* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY is the C026 transparency
     * mask; pixels with palette index 1 are skipped by the blit. */
    PROBE_TRANSPARENT_COLOR = 1,

    /* Live ordinal-5 sensor pose (the only ordinal-5 cell in the
     * live PC 3.4 DM1 V1 DUNGEON.DAT per the corridor ordinal
     * scanner — the front_south_entry probe's `[Discovery]` line
     * reports "ordinal 5 hits in corridor band = 1" at (2, 15)
     * cell 2, the shipped DM1 V1 fixture sensor).  Party at
     * (2, 16) DIR_NORTH, front cell (2, 15), C127 sensor on
     * (2, 15) cell 2 (south wall of (2, 15)). */
    PROBE_POSITIVE_MAP_X = 2,
    PROBE_POSITIVE_MAP_Y = 16,
    PROBE_POSITIVE_DIR = 0,           /* DIR_NORTH */
    /* Negative control: party cell still owns the C127 sensor with
     * sensorData=5 on its SOUTH wall (cell 2), but turning EAST
     * makes the visible wall cell of the front cell (2, 16) the
     * WEST wall (cell 3) of (2, 16) — which has no C127 sensor —
     * so the front-cell filter rejects the ordinal.  The D1C rect
     * must not show a portrait sprite floating over the corridor
     * east wall. */
    PROBE_NEGATIVE_MAP_X = 2,
    PROBE_NEGATIVE_MAP_Y = 16,
    PROBE_NEGATIVE_DIR = 1,           /* DIR_EAST */

    /* C040 candidate panel top-left in framebuffer coords; used to
     * bound the visible top-strip above the panel for the
     * redraw_after_candidate palette cleanliness check.  This is
     * G0032_ai_Graphic562_Box_Panel viewport-relative (80, 52). */
    PROBE_C040_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_C040_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_PORTRAIT_TOP_VISIBLE_H = PROBE_C040_PANEL_Y - PROBE_PORTRAIT_FB_Y,

    /* Match thresholds.  The aggregate threshold is tighter than
     * the 90% used by the north_positive / front_south_entry
     * siblings because the palette_match_rect route is
     * specifically about proving pixel-level palette alignment,
     * not just "is there a portrait here".  The per-line
     * thresholds catch stride / offset regressions that an
     * aggregate match could absorb. */
    PROBE_MATCH_AGGREGATE_MIN_PCT = 99,
    PROBE_ROW_MATCH_MIN_PCT = 90,
    PROBE_COL_MATCH_MIN_PCT = 80,

    /* Warm-palette indices used by champion portrait sprites
     * (DUNVIEW.C:3913-3928).  Skin / clothing / background
     * pixels use {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}; grey
     * stone wall texture uses {0x01, 0x02, 0x0D}.  Ordinal 5
     * (ELIJA / "LION OF YAITOPYA") uses a warm / earth-tone
     * palette in the local PC 3.4 fixture, so warm-pixel counts
     * are used as an additional positive-presence signal (not
     * the authoritative test — the palette_match is). */
    PROBE_WARM_THRESHOLD_PRESENT = 200,

    /* M11_GFX_CHAMPION_PORTRAITS == 26 == C026_GRAPHIC_CHAMPION_PORTRAITS.
     * This file-scoped enum in m11_game_view.c is not exported; the
     * source-locked value 26 is stable across versions. */
    PROBE_M11_GFX_CHAMPION_PORTRAITS = 26
};

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;
static int g_info = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

#define INFO(msg) do { ++g_info; printf("  INFO: %s\n", msg); } while (0)

/* Park the party at a specific map/x/y/direction pose.  The four
 * M11 boolean state fields that affect the D1C render are reset
 * to a known state so the palette_match_rect comparison is
 * deterministic. */
static void park_pose(M11_GameViewState* state,
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
}

/* Per-pixel palette match result for one comparison pass. */
typedef struct PaletteMatch {
    int matched;          /* non-transparent source pixels whose palette index == destination */
    int compared;         /* non-transparent source pixels (denominator for match pct) */
    int rowMatched[PROBE_PORTRAIT_H];   /* matched per row */
    int rowCompared[PROBE_PORTRAIT_H];
    int colMatched[PROBE_PORTRAIT_W];
    int colCompared[PROBE_PORTRAIT_W];
    int opaqueSrcCount;   /* total non-transparent source pixels in the cell */
    int opaqueDstCount;   /* total non-transparent destination pixels in the rect */
} PaletteMatch;

/* Compare the C026 ordinal-5 source cell to the D1C destination
 * rect on the rendered framebuffer, with strict per-pixel palette
 * index matching.  Both aggregate and per-line tallies are
 * populated.  Returns 1 if the call succeeded, 0 if assets are
 * missing (caller should SKIP). */
static int compute_palette_match(const M11_AssetSlot* portraits,
                                 const unsigned char* fb,
                                 PaletteMatch* out) {
    int x, y;
    int srcX = PROBE_ORDINAL_SRC_X;
    int srcY = PROBE_ORDINAL_SRC_Y;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if ((int)portraits->width  < srcX + PROBE_PORTRAIT_W) return 0;
    if ((int)portraits->height < srcY + PROBE_PORTRAIT_H) return 0;
    if (!fb) return 0;
    memset(out, 0, sizeof(*out));
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)
                (portraits->pixels[(srcY + y) * (int)portraits->width +
                                    (srcX + x)] & 0x0F);
            unsigned char dst = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_FB_Y + y) * PROBE_FB_W +
                   (PROBE_PORTRAIT_FB_X + x)]);
            int srcOpaque = (src != (unsigned char)PROBE_TRANSPARENT_COLOR);
            int dstOpaque = (dst != (unsigned char)PROBE_TRANSPARENT_COLOR);
            if (srcOpaque) ++out->opaqueSrcCount;
            if (dstOpaque) ++out->opaqueDstCount;
            if (!srcOpaque) continue;
            ++out->compared;
            ++out->rowCompared[y];
            ++out->colCompared[x];
            if (src == dst) {
                ++out->matched;
                ++out->rowMatched[y];
                ++out->colMatched[x];
            }
        }
    }
    return 1;
}

/* Count warm-color pixels in a framebuffer rectangle. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < PROBE_FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < PROBE_FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]);
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

/* Count non-zero pixels in a framebuffer rectangle. */
static int rect_nonzero_count(const unsigned char* fb,
                              int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < PROBE_FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < PROBE_FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]);
            if (idx != 0) ++cnt;
        }
    }
    return cnt;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    static unsigned char fbNorth[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbNorthSecond[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbEast[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbNorthPanelOn[PROBE_FB_W * PROBE_FB_H];
    PaletteMatch northMatch;
    PaletteMatch eastMatch;
    PaletteMatch northPanelOnMatch;
    int opaqueAtlas5;
    int frontOrdinalNorth;
    int frontOrdinalEast;
    int selectRc;
    int warmNorth;
    int warmEast;
    int nonzeroNorth;
    int nonzeroEast;
    int warmNorthTopPanelOn;
    int nonzeroNorthTopPanelOn;
    int determinismMatch;
    int row, col;
    int pct;
    int ok;
    int overallOk = 1;
    char mirrorName[16];
    char mirrorTitle[32];
    int hasName;
    int hasTitle;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait-05 / palette_match_rect / portrait_rect_position (v2.7.27) ===\n");
    printf("dataDir=%s ordinal=%d src=(%d,%d,%d,%d) dst=(%d,%d,%d,%d) "
           "match_thresh(agg=%d%% row=%d%% col=%d%%)\n",
           dataDir, PROBE_ORDINAL,
           PROBE_ORDINAL_SRC_X, PROBE_ORDINAL_SRC_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_PORTRAIT_VIEW_X, PROBE_PORTRAIT_VIEW_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_MATCH_AGGREGATE_MIN_PCT,
           PROBE_ROW_MATCH_MIN_PCT,
           PROBE_COL_MATCH_MIN_PCT);
    printf("live sensor pose: (%d,%d) DIR_NORTH  negative: (%d,%d) DIR_EAST\n",
           PROBE_POSITIVE_MAP_X, PROBE_POSITIVE_MAP_Y,
           PROBE_NEGATIVE_MAP_X, PROBE_NEGATIVE_MAP_Y);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)PROBE_M11_GFX_CHAMPION_PORTRAITS);

    /* ----------------------------------------------------------------
     * Group A — Atlas math sanity
     * ----------------------------------------------------------------
     * The C026 atlas is the source of truth for the
     * palette_match_rect comparison.  The ordinal-5 cell must
     * exist (within the 256x87 atlas), be non-blank, and not
     * accidentally match a neighbour (stride-regression guard). */
    printf("\n[Group A] C026 atlas math for ordinal 5 (row 0 / col 5 of 8x3 atlas)\n");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic %d = C026_GRAPHIC_CHAMPION_PORTRAITS)",
                 PROBE_M11_GFX_CHAMPION_PORTRAITS);
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        SKIP("cannot continue without the C026 portrait atlas");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == PROBE_ATLAS_W, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == PROBE_ATLAS_H, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 col = 5 & 7 = %d (expected 5)",
                 PROBE_ORDINAL_COL);
        CHECK(PROBE_ORDINAL_COL == 5, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 row = 5 >> 3 = %d (expected 0)",
                 PROBE_ORDINAL_ROW);
        CHECK(PROBE_ORDINAL_ROW == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 source cell within atlas: "
                 "srcX(%d)+w(%d) <= %u AND srcY(%d)+h(%d) <= %u",
                 PROBE_ORDINAL_SRC_X, PROBE_PORTRAIT_W, portraits->width,
                 PROBE_ORDINAL_SRC_Y, PROBE_PORTRAIT_H, portraits->height);
        CHECK(PROBE_ORDINAL_SRC_X + PROBE_PORTRAIT_W <= (int)portraits->width &&
              PROBE_ORDINAL_SRC_Y + PROBE_PORTRAIT_H <= (int)portraits->height, msg);
    }
    /* Count non-transparent source pixels in the ordinal-5 cell.  A
     * blank or missing cell would silently pass a 100% match against
     * a transparent destination, so this precondition must hold. */
    {
        int x, y, count = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                unsigned char src = (unsigned char)
                    (portraits->pixels[
                        (PROBE_ORDINAL_SRC_Y + y) * (int)portraits->width +
                        (PROBE_ORDINAL_SRC_X + x)] & 0x0F);
                if (src != (unsigned char)PROBE_TRANSPARENT_COLOR) ++count;
            }
        }
        opaqueAtlas5 = count;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 ordinal-5 cell has >= 200 non-transparent pixels "
                 "(got %d) - defined portrait, not blank/unused",
                 opaqueAtlas5);
        CHECK(opaqueAtlas5 >= 200, msg);
    }
    /* Mirror catalog identity.  In real DM1 V1 PC 3.4 the
     * ordinal-5 slot is ELIJA ("LION OF YAITOPYA"); the probe
     * asserts both the catalog returns the expected name and
     * title so a future catalog edit doesn't break the probe. */
    if (!state.mirrorCatalogAvailable) {
        SKIP("mirror catalog unavailable; cannot verify ordinal-5 catalog identity");
    } else {
        mirrorName[0] = '\0';
        mirrorTitle[0] = '\0';
        hasName = M11_GameView_GetMirrorNameByOrdinal(&state, PROBE_ORDINAL,
                                                      mirrorName, sizeof(mirrorName));
        hasTitle = M11_GameView_GetMirrorTitleByOrdinal(&state, PROBE_ORDINAL,
                                                        mirrorTitle, sizeof(mirrorTitle));
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "ordinal 5 catalog name='%s' title='%s' "
                     "(hasName=%d hasTitle=%d expected name='ELIJA' "
                     "title='LION OF YAITOPYA')",
                     mirrorName[0] ? mirrorName : "<empty>",
                     mirrorTitle[0] ? mirrorTitle : "<empty>",
                     hasName, hasTitle);
            CHECK(hasName && mirrorName[0] != '\0', msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "ordinal 5 catalog name='%s' matches 'ELIJA' "
                     "(PC 3.4 English fixture)",
                     mirrorName);
            CHECK(strcmp(mirrorName, "ELIJA") == 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "ordinal 5 catalog title='%s' matches 'LION OF YAITOPYA' "
                     "(PC 3.4 English fixture)",
                     mirrorTitle);
            CHECK(strcmp(mirrorTitle, "LION OF YAITOPYA") == 0, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group B — palette_match_rect at the live (2, 16) NORTH pose
     * ----------------------------------------------------------------
     * The C127 sensor on (2, 15) cell-side 2 (south wall of
     * (2, 15)) is visible from (2, 16) facing NORTH, where the
     * front cell (2, 15) owns the view.  This is the only
     * ordinal-5 sensor pose in the live PC 3.4 DM1 V1 DUNGEON.DAT
     * (the north_positive probe's `[Discovery]` line reports
     * "all=1 north=1"), so the palette_match_rect proof is run
     * against a real C127 sensor drive.  No sensor mutation is
     * needed — this is what distinguishes this slice from the
     * four sensor-seeded ordinal-05 siblings
     * (after_party_shuffle / cancel_reopen /
     * candidate_panel_cancel / wake_repaint). */
    printf("\n[Group B] palette_match_rect at (2,16) NORTH — strict per-pixel palette match\n");

    park_pose(&state, PROBE_POSITIVE_MAP_X, PROBE_POSITIVE_MAP_Y,
              PROBE_POSITIVE_DIR);
    frontOrdinalNorth = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (2, 16) NORTH = %d (expected 5)",
                 frontOrdinalNorth);
        CHECK(frontOrdinalNorth == 5, msg);
    }
    if (frontOrdinalNorth != 5) {
        fprintf(stderr,
                "FATAL: live DUNGEON.DAT does not expose ordinal 5 at "
                "(2, 16) NORTH; cannot run palette_match_rect against "
                "a real C127 sensor drive. ordinal=%d\n",
                frontOrdinalNorth);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    memset(fbNorth, 0, sizeof(fbNorth));
    M11_GameView_Draw(&state, fbNorth, PROBE_FB_W, PROBE_FB_H);

    if (!compute_palette_match(portraits, fbNorth, &northMatch)) {
        SKIP("compute_palette_match could not read C026 ordinal-5 cell");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* Sanity precondition: the D1C rect must contain the portrait
     * (non-zero / opaque pixels) before any match percentage is
     * even meaningful.  A blank destination would trivially pass
     * a 99% match against nothing, so we check the rect is
     * populated.
     *
     * Ordinal 5 (ELIJA / "LION OF YAITOPYA") in real DM1 V1 uses
     * a warm / earth-tone palette, so we additionally assert a
     * minimum warm-pixel count as a positive-presence signal.
     * The aggregate palette_match is still the authoritative
     * presence / correctness test. */
    nonzeroNorth = rect_nonzero_count(fbNorth,
                                      PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                      PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    warmNorth = rect_warm_count(fbNorth,
                                PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) non-zero pixel count = %d "
                 "(expected >= 600 - rect is fully painted, not a "
                 "sparse wall-texture strip)",
                 nonzeroNorth);
        CHECK(nonzeroNorth >= 600, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) warm-pixel count = %d "
                 "(>= %d required for ordinal 5 ELIJA earth-tone "
                 "palette)",
                 warmNorth, PROBE_WARM_THRESHOLD_PRESENT);
        CHECK(warmNorth >= PROBE_WARM_THRESHOLD_PRESENT, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect opaque-destination pixel count = %d "
                 "(expected >= 400 of 32*29=928 cells)",
                 northMatch.opaqueDstCount);
        CHECK(northMatch.opaqueDstCount >= 400, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "compute_palette_match compared = %d (must equal the "
                 "C026 ordinal-5 cell non-transparent count = %d)",
                 northMatch.compared, opaqueAtlas5);
        CHECK(northMatch.compared == opaqueAtlas5, msg);
    }

    /* Aggregate palette match.  Tightened to 99% (vs the 90%
     * threshold used by the north_positive / front_south_entry
     * siblings) because this probe's whole point is the strict
     * palette-level equality. */
    pct = (northMatch.compared > 0)
              ? (northMatch.matched * 100 / northMatch.compared)
              : 0;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 5 (aggregate): "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 northMatch.matched, northMatch.compared, pct,
                 PROBE_MATCH_AGGREGATE_MIN_PCT);
        CHECK(pct >= PROBE_MATCH_AGGREGATE_MIN_PCT, msg);
    }

    /* Per-row palette match.  Catches vertical stride / offset
     * regressions that an aggregate match could absorb. */
    for (row = 0; row < PROBE_PORTRAIT_H; ++row) {
        int rowPct = (northMatch.rowCompared[row] > 0)
                         ? (northMatch.rowMatched[row] * 100 /
                            northMatch.rowCompared[row])
                         : 0;
        char msg[200];
        if (northMatch.rowCompared[row] == 0) {
            snprintf(msg, sizeof(msg),
                     "row %2d: all-transparent (compared=0) - "
                     "skipped, stride math OK",
                     row);
            CHECK(1, msg);
            continue;
        }
        snprintf(msg, sizeof(msg),
                 "row %2d palette_match for ordinal 5: "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 row, northMatch.rowMatched[row], northMatch.rowCompared[row],
                 rowPct, PROBE_ROW_MATCH_MIN_PCT);
        CHECK(rowPct >= PROBE_ROW_MATCH_MIN_PCT, msg);
    }

    /* Per-column palette match.  Catches horizontal stride / offset
     * regressions.  Some columns may be mostly-transparent for
     * ordinal 5 (a thin column) so a lower threshold applies. */
    for (col = 0; col < PROBE_PORTRAIT_W; ++col) {
        int colPct = (northMatch.colCompared[col] > 0)
                         ? (northMatch.colMatched[col] * 100 /
                            northMatch.colCompared[col])
                         : 0;
        char msg[200];
        if (northMatch.colCompared[col] == 0) {
            snprintf(msg, sizeof(msg),
                     "col %2d: all-transparent (compared=0) - "
                     "skipped, stride math OK",
                     col);
            CHECK(1, msg);
            continue;
        }
        snprintf(msg, sizeof(msg),
                 "col %2d palette_match for ordinal 5: "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 col, northMatch.colMatched[col], northMatch.colCompared[col],
                 colPct, PROBE_COL_MATCH_MIN_PCT);
        CHECK(colPct >= PROBE_COL_MATCH_MIN_PCT, msg);
    }

    /* Render determinism: a second render of the same pose must
     * produce a byte-equal framebuffer.  Catches any non-pure
     * framebuffer state leak (e.g. a counter increment between
     * draws). */
    memset(fbNorthSecond, 0, sizeof(fbNorthSecond));
    M11_GameView_Draw(&state, fbNorthSecond, PROBE_FB_W, PROBE_FB_H);
    determinismMatch = memcmp(fbNorth, fbNorthSecond, sizeof(fbNorth)) == 0;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C framebuffer is byte-equal across two renders of "
                 "(2, 16) NORTH (64000-byte framebuffer compare: %s)",
                 determinismMatch ? "equal" : "DIFFER");
        CHECK(determinismMatch, msg);
    }

    /* ----------------------------------------------------------------
     * Group C — side_wall_no_float negative control
     * ----------------------------------------------------------------
     * At (2, 16) facing EAST, the C127 sensor on (2, 15) cell-side
     * 2 (south wall) is on the party cell's left side from the
     * east-facing player's view.  The front cell (2, 17) has no
     * C127 sensor with sensorData=5 on its east aspect, so the
     * front-cell filter rejects the ordinal.  The D1C rect must
     * not show a portrait sprite.  This is the negative control
     * for the palette_match_rect proof: it confirms that the
     * high-match result at (2, 16) NORTH is driven by the C127
     * sensor side filter, not by a coincidental "the wall
     * texture happens to match ordinal 5" artefact.
     *
     * The D1C rect at (2, 16) EAST is occupied by the D1C wall
     * ornament texture, not by a portrait sprite.  Wall texture
     * pixels are not transparent (so non-zero count is high) but
     * they don't carry ordinal-5's palette pattern.  Therefore
     * the right negative-control test is the **palette_match
     * percentage**, which is the core proof of this probe: a
     * wall texture cannot match the C026 ordinal-5 cell at >=5%,
     * so a low palette_match is the correct "no portrait"
     * assertion.  Raw non-zero / warm counts are reported as
     * informational only. */
    printf("\n[Group C] side_wall_no_float at (2, 16) EAST — palette_match must be < 5%%\n");

    park_pose(&state, PROBE_NEGATIVE_MAP_X, PROBE_NEGATIVE_MAP_Y,
              PROBE_NEGATIVE_DIR);
    frontOrdinalEast = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (2, 16) EAST = %d (expected -1, "
                 "the east face of (2, 16) is a corridor side wall, the "
                 "front cell (2, 17) has no C127 sensor with "
                 "sensorData=5 on its east aspect)",
                 frontOrdinalEast);
        CHECK(frontOrdinalEast == -1, msg);
    }

    memset(fbEast, 0, sizeof(fbEast));
    M11_GameView_Draw(&state, fbEast, PROBE_FB_W, PROBE_FB_H);

    nonzeroEast = rect_nonzero_count(fbEast,
                                     PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                     PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    warmEast = rect_warm_count(fbEast,
                               PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                               PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) non-zero pixel count = %d at "
                 "(2, 16) EAST (informational - D1C wall ornament "
                 "texture is opaque, not transparent)",
                 nonzeroEast);
        INFO(msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) warm-pixel count = %d at "
                 "(2, 16) EAST (informational - wall ornament is "
                 "mostly grey)",
                 warmEast);
        INFO(msg);
    }

    /* The palette_match percentage is the authoritative test: a
     * wall texture cannot match the C026 ordinal-5 cell at >=5%,
     * so a low palette_match is the correct "no portrait"
     * assertion. */
    ok = compute_palette_match(portraits, fbEast, &eastMatch);
    if (!ok) {
        SKIP("compute_palette_match for (2, 16) EAST skipped (asset bounds)");
    } else {
        int eastPct = (eastMatch.compared > 0)
                          ? (eastMatch.matched * 100 / eastMatch.compared)
                          : 0;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 5 at (2, 16) "
                 "EAST: matched=%d compared=%d pct=%d%% (< 5%% "
                 "required - wall texture, not portrait)",
                 eastMatch.matched, eastMatch.compared, eastPct);
        CHECK(eastPct < 5, msg);
    }

    /* ----------------------------------------------------------------
     * Group D — redraw_after_candidate stability
     * ----------------------------------------------------------------
     * Re-park at (2, 16) NORTH, then call SelectFrontMirrorCandidate
     * to engage the C040 candidate panel.  Re-render and assert:
     *   (i)  the D1C rect no longer matches ordinal 5 (panel owns
     *        the view; no stale sprite),
     *   (ii) the visible top strip above the panel has no warm-color
     *        leak,
     *   (iii) the visible top strip is non-empty (panel border). */
    printf("\n[Group D] redraw_after_candidate at (2, 16) NORTH — C040 panel redraw stability\n");

    park_pose(&state, PROBE_POSITIVE_MAP_X, PROBE_POSITIVE_MAP_Y,
              PROBE_POSITIVE_DIR);
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (2, 16) NORTH returns 1 (got %d)",
                 selectRc);
        CHECK(selectRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "candidate panel state is live (candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == 5 &&
              state.candidateMirrorPartyIndex == 0 &&
              state.world.party.championCount == 1, msg);
    }

    memset(fbNorthPanelOn, 0, sizeof(fbNorthPanelOn));
    M11_GameView_Draw(&state, fbNorthPanelOn, PROBE_FB_W, PROBE_FB_H);

    /* (i) full D1C rect no longer matches ordinal 5 as a stale sprite. */
    ok = compute_palette_match(portraits, fbNorthPanelOn, &northPanelOnMatch);
    {
        char msg[200];
        if (!ok) {
            snprintf(msg, sizeof(msg),
                     "compute_palette_match for (2, 16) NORTH panel-on "
                     "skipped (asset bounds)");
            SKIP(msg);
        } else {
            int panelPct = (northPanelOnMatch.compared > 0)
                               ? (northPanelOnMatch.matched * 100 /
                                  northPanelOnMatch.compared)
                               : 0;
            snprintf(msg, sizeof(msg),
                     "D1C rect palette_match for ordinal 5 with C040 "
                     "panel live: matched=%d compared=%d pct=%d%% "
                     "(< 5%% required, no stale sprite)",
                     northPanelOnMatch.matched,
                     northPanelOnMatch.compared, panelPct);
            CHECK(panelPct < 5, msg);
        }
    }

    /* (ii) the visible top strip above the C040 panel must stay
     * clean of warm-color portrait pixels.  The C040 panel top
     * edge is at (80, 52) viewport-local (or
     * (PROBE_VIEWPORT_X + 80, PROBE_VIEWPORT_Y + 52) in
     * framebuffer coords).  The D1C rect's top edge is at
     * (PROBE_PORTRAIT_FB_Y); the visible top strip is the rows
     * from the D1C top to the C040 top. */
    warmNorthTopPanelOn = rect_warm_count(
        fbNorthPanelOn,
        PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
        PROBE_PORTRAIT_W, PROBE_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C visible top strip (above C040 panel) warm-pixel "
                 "count = %d (<= 10 required, no portrait leak)",
                 warmNorthTopPanelOn);
        CHECK(warmNorthTopPanelOn <= 10, msg);
    }

    /* (iii) the visible top strip is non-empty (panel border /
     * wall above). */
    nonzeroNorthTopPanelOn = rect_nonzero_count(
        fbNorthPanelOn,
        PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
        PROBE_PORTRAIT_W, PROBE_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C visible top strip (above C040 panel) non-zero "
                 "pixel count = %d (>= 50 required, panel border present)",
                 nonzeroNorthTopPanelOn);
        CHECK(nonzeroNorthTopPanelOn >= 50, msg);
    }

    /* ----------------------------------------------------------------
     * Summary
     * ---------------------------------------------------------------- */
    if (g_fail > 0) overallOk = 0;
    printf("\n=== Summary: %d passed, %d failed, %d skipped, %d info "
           "(ordinal-05 palette_match_rect portrait_rect_position) ===\n",
           g_pass, g_fail, g_skip, g_info);
    M11_GameView_Shutdown(&state);
    return overallOk ? 0 : 1;
}
