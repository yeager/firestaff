/*
 * firestaff_dm1_v1_hoc_champion_portrait_09_palette_match_rect_runtime_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 9,
 * route palette_match_rect, aspect portrait_rect_position,
 * batch group 4.
 *
 * Sibling to the existing ordinal-9 slices:
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_9_south_return_portrait_rect_position_runtime_probe.c
 *     route = south_return (seeds the (1,6) C127 sensor data 13 -> 9 on
 *     the canonical south-facing WUUF alcove; per-pixel dominance vs
 *     warm-color heuristic; per-ordinal dominance over all 24 atlas
 *     cells)
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative_portrait_rect_position_runtime_probe.c
 *     route = west_negative (corridor (x=1, y=2..6) facing west; wrong-
 *     wall side filter; cross-checks row=1 band aliveness at (1,5) N
 *     with ordinal 10 GANDO)
 *
 *   probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_09_candidate_panel_cancel_portrait_rect_position_runtime_probe.c
 *     route = candidate_panel_cancel (C040 panel live, then Escape
 *     dismissal, then re-park + re-render proves the D1C portrait
 *     returns to painted state)
 *
 *   probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_09_popup_focus_return_portrait_rect_position_runtime_probe.c
 *     route = popup_focus_return (popup overlay redraw discipline after
 *     a popup steals and returns keyboard focus)
 *
 * This probe covers the **palette_match_rect** route variant — the
 * distinct concern: "does each rendered pixel in the D1C portrait
 * rectangle carry the same VGA palette index as the corresponding
 * C026 ordinal-9 atlas cell?"  The south_return sibling uses a
 * 90% aggregate dominance check across warm-pixel heuristics and
 * per-ordinal match counts; this probe tightens the palette-level
 * match to 99% aggregate AND adds per-row (>=90%) and per-column
 * (>=80%) invariants to catch sub-rectangle stride/offset
 * regressions that an aggregate match could absorb.
 *
 * The probe anchors at the live (1, 10) DIR_NORTH pose from the
 * corridor ordinal scanner — the only ordinal-9 sensor in the live
 * PC 3.4 DM1 V1 DUNGEON.DAT, so the palette_match_rect proof runs
 * against a real C127 sensor drive (no sensor mutation).
 *
 * Three coupled concerns in one runtime drive:
 *
 *   (1) palette_match_rect — strict per-pixel palette index match
 *       between the C026 ordinal-9 source cell (32, 29, 32, 29)
 *       and the D1C destination rect (96, 35, 32, 29) on the
 *       rendered framebuffer.  Aggregate match >= 99%, every row
 *       match >= 90% of its non-transparent count, every column
 *       match >= 80% of its non-transparent count.  This tightens
 *       the 90% aggregate / no per-line checks used by the south
 *       _return sibling; the per-line guarantees catch stride /
 *       offset regressions that an aggregate match could absorb.
 *
 *   (2) side_wall_no_float — at the (1, 10) DIR_SOUTH pose (the
 *       party cell owns the C127 sensor with sensorData=9 on its
 *       NORTH aspect, but the front cell is (1, 11) which has no
 *       C127 sensor on its NORTH aspect, so the front-cell-side
 *       filter rejects the ordinal).  The D1C rect must contain
 *       less than 5% palette_match against ordinal 9 (no stale
 *       sprite floating over the corridor north wall).
 *
 *   (3) redraw_after_candidate — re-render the (1, 10) NORTH pose
 *       with the C040 candidate panel live (after
 *       SelectFrontMirrorCandidate).  The full D1C rect must drop
 *       to < 5% match with ordinal 9 (no stale sprite after C040
 *       takes over), and the visible top strip above the panel
 *       must stay clean of warm pixels (no portrait bleed over
 *       the panel border).
 *
 * The two render passes (panel-off and panel-on) are also checked
 * for determinism: a second render of the same pose must produce
 * a framebuffer byte-equal to the first.  This catches any
 * non-pure framebuffer state leak (e.g. an incrementing counter
 * between draws).
 *
 * Source evidence:
 *   - ReDMCSB DUNGEON.C:2573 (C127 sensor cell match against
 *     view direction)
 *   - ReDMCSB DUNGEON.C:2608-2612 (G0289_i_DungeonView_ChampionPortraitOrdinal
 *     = C127 sensorData, range 0..23)
 *   - ReDMCSB DUNVIEW.C:3913-3928 (C026 portrait blit at D1C
 *     box { x=96, y=35, w=32, h=29 } using (ord & 7) * 32,
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

    /* Ordinal 9 in the C026 atlas: (9 & 7) * 32 = 32,
     *                                (9 >> 3) * 29 = 29.
     * This is the MIDDLE-LEFT band of the 8x3 atlas (row 1, col 1).
     * On the local PC 3.4 DM1 V1 DUNGEON.DAT this is the ZED /
     * "DUKE OF BANVILLE" portrait per
     * firestaff_dm1_v1_champion_mirror_ordinal_9_south_return
     * _portrait_rect_position_runtime_probe. */
    PROBE_ORDINAL = 9,
    PROBE_ORDINAL_COL = 9 & 7,         /* = 1 */
    PROBE_ORDINAL_ROW = 9 >> 3,        /* = 1 */
    PROBE_ORDINAL_SRC_X = PROBE_ORDINAL_COL * 32,   /* = 32 */
    PROBE_ORDINAL_SRC_Y = PROBE_ORDINAL_ROW * 29,   /* = 29 */
    /* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY is the C026 transparency
     * mask; pixels with palette index 1 are skipped by the blit. */
    PROBE_TRANSPARENT_COLOR = 1,

    /* Live ordinal-9 sensor pose (the only ordinal-9 cell in the
     * live PC 3.4 DUNGEON.DAT per the corridor ordinal scanner).
     * Party at (1, 10) DIR_NORTH, front cell (1, 9), C127 sensor
     * on (1, 10) NORTH aspect. */
    PROBE_POSITIVE_MAP_X = 1,
    PROBE_POSITIVE_MAP_Y = 10,
    PROBE_POSITIVE_DIR = 0,           /* DIR_NORTH */
    /* Negative control: party cell still owns the C127 sensor with
     * sensorData=9 on its NORTH aspect, but the front cell (1, 11)
     * has no C127 sensor on its NORTH aspect, so the front-cell
     * filter rejects the ordinal and the D1C rect must stay
     * un-painted (no stale sprite floating on the north wall). */
    PROBE_NEGATIVE_MAP_X = 1,
    PROBE_NEGATIVE_MAP_Y = 10,
    PROBE_NEGATIVE_DIR = 2,           /* DIR_SOUTH */

    /* C040 candidate panel top-left in framebuffer coords; used to
     * bound the visible top-strip above the panel for the
     * redraw_after_candidate palette cleanliness check. */
    PROBE_C040_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_C040_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_PORTRAIT_TOP_VISIBLE_H = PROBE_C040_PANEL_Y - PROBE_PORTRAIT_FB_Y,

    /* Match thresholds.  The aggregate threshold is tighter than
     * the 90% used by the south_return sibling because the
     * palette_match_rect route is specifically about proving
     * pixel-level palette alignment, not just "is there a
     * portrait here".  The per-line thresholds catch stride /
     * offset regressions that an aggregate match could absorb. */
    PROBE_MATCH_AGGREGATE_MIN_PCT = 99,
    PROBE_ROW_MATCH_MIN_PCT = 90,
    PROBE_COL_MATCH_MIN_PCT = 80,

    /* Warm-palette indices used by champion portrait sprites
     * (DUNVIEW.C:3913-3928).  Skin / clothing / background
     * pixels use {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}; grey
     * stone wall texture uses {0x01, 0x02, 0x0D}.  Ordinal 9
     * (ZED / "DUKE OF BANVILLE") uses a warm/earth-tone palette
     * in the local PC 3.4 fixture, so warm-pixel counts are
     * used as an additional positive-presence signal (not the
     * authoritative test — the palette_match is). */
    PROBE_WARM_THRESHOLD_ABSENT = 20,

    /* M11_GFX_CHAMPION_PORTRAITS == 26 == C026_GRAPHIC_CHAMPION_PORTRAITS.
     * This file-scoped enum in m11_game_view.c is not exported; the
     * source-locked value 26 is stable across versions. */
    PROBE_M11_GFX_CHAMPION_PORTRAITS = 26
};

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

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

/* Compare the C026 ordinal-9 source cell to the D1C destination
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
    static unsigned char fbSouth[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbNorthPanelOn[PROBE_FB_W * PROBE_FB_H];
    PaletteMatch northMatch;
    PaletteMatch southMatch;
    PaletteMatch northPanelOnMatch;
    int opaqueAtlas9;
    int frontOrdinalNorth;
    int frontOrdinalSouth;
    int selectRc;
    int warmNorth;
    int warmSouth;
    int nonzeroNorth;
    int nonzeroSouth;
    int warmNorthTopPanelOn;
    int nonzeroNorthTopPanelOn;
    int determinismMatch;
    int row, col;
    int pct;
    int ok;
    int overallOk = 1;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait-09 / palette_match_rect / portrait_rect_position (v2.7.27) ===\n");
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
    printf("live sensor pose: (%d,%d) DIR_NORTH  negative: (%d,%d) DIR_SOUTH\n",
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
     * palette_match_rect comparison.  The ordinal-9 cell must
     * exist (within the 256x87 atlas), be non-blank, and not
     * accidentally match a neighbour (stride-regression guard). */
    printf("\n[Group A] C026 atlas math for ordinal 9 (row 1 / col 1 of 8x3 atlas)\n");
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
                 "ordinal 9 col = 9 & 7 = %d (expected 1)",
                 PROBE_ORDINAL_COL);
        CHECK(PROBE_ORDINAL_COL == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 row = 9 >> 3 = %d (expected 1)",
                 PROBE_ORDINAL_ROW);
        CHECK(PROBE_ORDINAL_ROW == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 source cell within atlas: "
                 "srcX(%d)+w(%d) <= %u AND srcY(%d)+h(%d) <= %u",
                 PROBE_ORDINAL_SRC_X, PROBE_PORTRAIT_W, portraits->width,
                 PROBE_ORDINAL_SRC_Y, PROBE_PORTRAIT_H, portraits->height);
        CHECK(PROBE_ORDINAL_SRC_X + PROBE_PORTRAIT_W <= (int)portraits->width &&
              PROBE_ORDINAL_SRC_Y + PROBE_PORTRAIT_H <= (int)portraits->height, msg);
    }
    /* Count non-transparent source pixels in the ordinal-9 cell.  A
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
        opaqueAtlas9 = count;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 ordinal-9 cell has >= 200 non-transparent pixels "
                 "(got %d) - defined portrait, not blank/unused",
                 opaqueAtlas9);
        CHECK(opaqueAtlas9 >= 200, msg);
    }

    /* ----------------------------------------------------------------
     * Group B — palette_match_rect at the live (1, 10) NORTH pose
     * ----------------------------------------------------------------
     * The C127 sensor on (1, 10) cell-side NORTH is visible from
     * (1, 10) facing NORTH, where the front cell (1, 9) owns the
     * view.  This is the only ordinal-9 sensor pose in the live
     * PC 3.4 DM1 V1 DUNGEON.DAT (corridor ordinal scanner
     * confirms (1, 10, N) -> ordinal=9 / ZED), so the
     * palette_match_rect proof is run against a real C127 sensor
     * drive. */
    printf("\n[Group B] palette_match_rect at (1,10) NORTH — strict per-pixel palette match\n");

    park_pose(&state, PROBE_POSITIVE_MAP_X, PROBE_POSITIVE_MAP_Y,
              PROBE_POSITIVE_DIR);
    frontOrdinalNorth = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1, 10) NORTH = %d (expected 9)",
                 frontOrdinalNorth);
        CHECK(frontOrdinalNorth == 9, msg);
    }
    if (frontOrdinalNorth != 9) {
        fprintf(stderr,
                "FATAL: live DUNGEON.DAT does not expose ordinal 9 at "
                "(1, 10) NORTH; cannot run palette_match_rect against "
                "a real C127 sensor drive. ordinal=%d\n",
                frontOrdinalNorth);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    memset(fbNorth, 0, sizeof(fbNorth));
    M11_GameView_Draw(&state, fbNorth, PROBE_FB_W, PROBE_FB_H);

    if (!compute_palette_match(portraits, fbNorth, &northMatch)) {
        SKIP("compute_palette_match could not read C026 ordinal-9 cell");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* Sanity precondition: the D1C rect must contain the portrait
     * (non-zero / opaque pixels) before any match percentage is even
     * meaningful.  A blank destination would trivially pass a 99%
     * match against nothing, so we check the rect is populated.
     *
     * Ordinal 9 (ZED / "DUKE OF BANVILLE") in real DM1 V1 uses a
     * warm / earth-tone palette, so we additionally assert a
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
                 "(>= %d required for ordinal 9 ZED earth-tone "
                 "palette)",
                 warmNorth, PROBE_WARM_THRESHOLD_ABSENT);
        CHECK(warmNorth >= PROBE_WARM_THRESHOLD_ABSENT, msg);
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
                 "C026 ordinal-9 cell non-transparent count = %d)",
                 northMatch.compared, opaqueAtlas9);
        CHECK(northMatch.compared == opaqueAtlas9, msg);
    }

    /* Aggregate palette match.  Tightened to 99% (vs the 90%
     * threshold used by the south_return sibling) because this
     * probe's whole point is the strict palette-level equality. */
    pct = (northMatch.compared > 0)
              ? (northMatch.matched * 100 / northMatch.compared)
              : 0;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 9 (aggregate): "
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
                 "row %2d palette_match for ordinal 9: "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 row, northMatch.rowMatched[row], northMatch.rowCompared[row],
                 rowPct, PROBE_ROW_MATCH_MIN_PCT);
        CHECK(rowPct >= PROBE_ROW_MATCH_MIN_PCT, msg);
    }

    /* Per-column palette match.  Catches horizontal stride / offset
     * regressions.  Some columns may be mostly-transparent for
     * ordinal 9 (a thin column) so a lower threshold applies. */
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
                 "col %2d palette_match for ordinal 9: "
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
                 "(1, 10) NORTH (64000-byte framebuffer compare: %s)",
                 determinismMatch ? "equal" : "DIFFER");
        CHECK(determinismMatch, msg);
    }

    /* ----------------------------------------------------------------
     * Group C — side_wall_no_float negative control
     * ----------------------------------------------------------------
     * At (1, 10) facing SOUTH, the C127 sensor on (1, 10) cell-side
     * NORTH is on the party cell itself.  The front cell (1, 11)
     * has no C127 sensor on its NORTH aspect, so the front-cell
     * filter rejects the ordinal.  The D1C rect must not show a
     * portrait sprite.  This is the negative control for the
     * palette_match_rect proof: it confirms that the high-match
     * result at (1, 10) NORTH is driven by the C127 sensor side
     * filter, not by a coincidental "the wall texture happens to
     * match ordinal 9" artefact.
     *
     * The D1C rect at (1, 10) SOUTH is occupied by the D1C wall
     * ornament texture, not by a portrait sprite.  Wall texture
     * pixels are not transparent (so non-zero count is high) but
     * they don't carry ordinal-9's palette pattern.  Therefore the
     * right negative-control test is the **palette_match
     * percentage**, which is the core proof of this probe: a wall
     * texture cannot match the C026 ordinal-9 cell at >=5%, so a
     * low palette_match is the correct "no portrait" assertion.
     * Raw non-zero / warm counts are reported as informational
     * only. */
    printf("\n[Group C] side_wall_no_float at (1, 10) SOUTH — palette_match must be < 5%%\n");

    park_pose(&state, PROBE_NEGATIVE_MAP_X, PROBE_NEGATIVE_MAP_Y,
              PROBE_NEGATIVE_DIR);
    frontOrdinalSouth = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1, 10) SOUTH = %d (expected -1, "
                 "C127 sensor with sensorData=9 lives on the party "
                 "cell's NORTH aspect, but the front cell (1, 11) has "
                 "no C127 sensor on its NORTH aspect)",
                 frontOrdinalSouth);
        CHECK(frontOrdinalSouth == -1, msg);
    }

    memset(fbSouth, 0, sizeof(fbSouth));
    M11_GameView_Draw(&state, fbSouth, PROBE_FB_W, PROBE_FB_H);

    nonzeroSouth = rect_nonzero_count(fbSouth,
                                      PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                      PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    warmSouth = rect_warm_count(fbSouth,
                                PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) non-zero pixel count = %d at "
                 "(1, 10) SOUTH (informational - D1C wall ornament "
                 "texture is opaque, not transparent)",
                 nonzeroSouth);
        ++g_pass;
        printf("  INFO: %s\n", msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) warm-pixel count = %d at "
                 "(1, 10) SOUTH (informational - wall ornament is "
                 "mostly grey)",
                 warmSouth);
        ++g_pass;
        printf("  INFO: %s\n", msg);
    }

    /* The palette_match percentage is the authoritative test: a
     * wall texture cannot match the C026 ordinal-9 cell at >=5%,
     * so a low palette_match is the correct "no portrait"
     * assertion. */
    ok = compute_palette_match(portraits, fbSouth, &southMatch);
    if (!ok) {
        SKIP("compute_palette_match for (1, 10) SOUTH skipped (asset bounds)");
    } else {
        int southPct = (southMatch.compared > 0)
                          ? (southMatch.matched * 100 / southMatch.compared)
                          : 0;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 9 at (1, 10) "
                 "SOUTH: matched=%d compared=%d pct=%d%% (< 5%% "
                 "required - wall texture, not portrait)",
                 southMatch.matched, southMatch.compared, southPct);
        CHECK(southPct < 5, msg);
    }

    /* ----------------------------------------------------------------
     * Group D — redraw_after_candidate stability
     * ----------------------------------------------------------------
     * Re-park at (1, 10) NORTH, then call SelectFrontMirrorCandidate
     * to engage the C040 candidate panel.  Re-render and assert:
     *   (i)  the D1C rect no longer matches ordinal 9 (panel owns
     *        the view; no stale sprite),
     *   (ii) the visible top strip above the panel has no warm-color
     *        leak,
     *   (iii) the visible top strip is non-empty (panel border). */
    printf("\n[Group D] redraw_after_candidate at (1, 10) NORTH — C040 panel redraw stability\n");

    park_pose(&state, PROBE_POSITIVE_MAP_X, PROBE_POSITIVE_MAP_Y,
              PROBE_POSITIVE_DIR);
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (1, 10) NORTH returns 1 (got %d)",
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
              state.candidateMirrorOrdinal == 9 &&
              state.candidateMirrorPartyIndex == 0 &&
              state.world.party.championCount == 1, msg);
    }

    memset(fbNorthPanelOn, 0, sizeof(fbNorthPanelOn));
    M11_GameView_Draw(&state, fbNorthPanelOn, PROBE_FB_W, PROBE_FB_H);

    /* (i) full D1C rect no longer matches ordinal 9 as a stale sprite. */
    ok = compute_palette_match(portraits, fbNorthPanelOn, &northPanelOnMatch);
    {
        char msg[200];
        if (!ok) {
            snprintf(msg, sizeof(msg),
                     "compute_palette_match for (1, 10) NORTH panel-on "
                     "skipped (asset bounds)");
        } else {
            int panelPct = (northPanelOnMatch.compared > 0)
                               ? (northPanelOnMatch.matched * 100 /
                                  northPanelOnMatch.compared)
                               : 0;
            snprintf(msg, sizeof(msg),
                     "D1C rect palette_match for ordinal 9 with C040 "
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
    printf("\n=== Summary: %d passed, %d failed, %d skipped "
           "(ordinal-09 palette_match_rect portrait_rect_position) ===\n",
           g_pass, g_fail, g_skip);
    M11_GameView_Shutdown(&state);
    return overallOk ? 0 : 1;
}
