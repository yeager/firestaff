/*
 * firestaff_dm1_v1_hall_of_champions_portrait_14_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 14
 *   route   after_party_shuffle (the C040 panel select -> C160 resurrect
 *          confirm cycle mutates championCount; the D1C portrait rect
 *          must keep painting the source ordinal after the party has
 *          been augmented, and must stop painting it once the matching
 *          C127 sensor has been disabled by F0282)
 *   aspect  portrait_rect_position
 *
 * The DM1 PC 3.4 C026 champion-portrait atlas is an 8x3 grid of 32x29
 * portraits (256x87 pixels total, ordinals 0..23).  Ordinal 14 sits at
 * row 1, column 6 of the atlas:
 *
 *     srcX = (14 & 7) * 32 = 192
 *     srcY = (14 >> 3) * 29 =  29
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNGEON.C:2608-2612, DUNVIEW.C:3913-3928
 * and DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 * = { 96, 127, 35, 63 }):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * Why "after_party_shuffle" is its own route
 * ------------------------------------------
 * The "after_party_shuffle" route is a deliberate mirror
 * resurrection / rearm state slice:
 *
 *   1. Park the party at the (1, 19) DIR_NORTH pose that exposes
 *      the C127 sensor with sensorData = 14.  The pose is OOB on
 *      the south edge of map 0 (the Hall of Champions is 18x19) and
 *      is reached the same way the sibling south_return probe parks
 *      it (firestaff_dm1_v1_hall_of_champions_portrait_14_south_
 *      return_rect_probe.c).
 *
 *   2. Pixel-prove the D1C portrait rect (96, 35, 32, 29) carries
 *      the ordinal-14 atlas cell at >= 90% match.  This is the
 *      baseline.
 *
 *   3. Move the party to (1, 2) DIR_NORTH, the canonical HALK
 *      (ordinal 1) C127 sensor, run M11_GameView_SelectFrontMirror
 *      Candidate + M11_GameView_ConfirmMirrorCandidate(reincarnate=0)
 *      to append HALK to the party (championCount: 0 -> 1, active
 *      champion rotates to slot 0).  This is the resurrection /
 *      rearm flow that F0280 + F0282 (REVIVE.C) drive; the post-
 *      state is the "shuffled party" this slice is named for.
 *
 *   4. Move back to (1, 19) DIR_NORTH and re-render.  The D1C
 *      portrait rect must STILL carry ordinal 14.  The route is
 *      not consumed by the HALK recruit, so the LEYLA portrait
 *      must still paint at (96, 35, 32, 29).  This is the core
 *      invariant: shuffling the party does not bleach the D1C
 *      rect of a different route's C127 sensor.
 *
 *   5. Repeat for a second recruit (WUUF, ordinal 13) to push
 *      championCount to 2, then re-render at (1, 19) DIR_NORTH.
 *      Same invariant: ordinal 14 still paints the D1C rect.
 *
 *   6. Finally, recruit the LEYLA route itself (ordinal 14) via
 *      M11_GameView_SelectFrontMirrorCandidate +
 *      M11_GameView_ConfirmMirrorCandidate(reincarnate=0).  The
 *      C127 sensor on (1, 18) south wall is reset to sensorType=0
 *      by m11_disable_front_mirror_route (which mirrors
 *      ReDMCSB REVIVE.C F0282).  The front-mirror ordinal must
 *      drop to -1 and the D1C portrait rect must no longer
 *      carry ordinal-14 pixels above the 30% drift threshold
 *      (the rect falls back to the C346 D1C wall-ornament
 *      placeholder, with the C026 blit suppressed by the
 *      m11_draw_dm1_front_champion_portrait guard at
 *      m11_game_view.c:13960).
 *
 * Source evidence:
 *   - DUNGEON.C:2573  C127 sensor cell match against view dir
 *   - DUNGEON.C:2608-2612  C127 sensorData -> G0289 portrait ordinal
 *   - DUNVIEW.C:3913-3928  D1C C026 portrait blit at {96,35}
 *   - DUNVIEW.C:525  G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 }
 *   - DUNVIEW.C:3916-3919  C026_GRAPHIC_CHAMPION_PORTRAITS,
 *                          "A portrait is 32x29 pixels"
 *   - DUNVIEW.C:8318-8618  F0128 far-to-near viewport redraw order
 *   - COORD.C:1693-1749  PC34 viewport origin and portrait dims
 *   - DEFS.H:821-826  M027_PORTRAIT_X / M028_PORTRAIT_Y macro math
 *   - MOVESENS.C:1501-1503  F0280 sensorData -> candidate ordinal
 *   - REVIVE.C F0280  materialize candidate from sensorData
 *   - REVIVE.C F0282  C160 resurrect path (confirmed);
 *                     C127 sensorType reset on confirm
 *   - m11_draw_dm1_front_mirror_route  BUG-120/121 panel guard
 *   - m11_draw_dm1_front_champion_portrait  C026 blit with
 *                     transparentColor=1 (m11_game_view.c:13952)
 *   - m11_disable_front_mirror_route  REVIVE.C F0282 C127 disable
 *   - M11_GameView_SelectFrontMirrorCandidate  M11 entry for
 *                     REVIVE.C F0280 (championCount++)
 *   - M11_GameView_ConfirmMirrorCandidate  M11 entry for
 *                     REVIVE.C F0282 C160 / C161
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_
 *     rect_probe.c                       (LEYLA baseline rect proof)
 *   firestaff_dm1_v1_hall_of_champions_portrait_14_redraw_after_
 *     candidate_runtime_probe.c          (C040 panel state path)
 *   firestaff_dm1_v1_champion_mirror_leylla_ordinal14_unreachable_
 *     probe.c                            (route-absent data proof)
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe.c
 *                                          (16-pose ordinal map)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe.c
 *                                          (positive (1,2)N + (1,5)N zones)
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from; this is a
 *     runtime-correctness check, not a pixel-for-pixel match against
 *     an external DOSBox reference.
 *   - The probe parks the party directly at (1, 19) DIR_NORTH and
 *     at the canonical recruit poses, the same way the existing
 *     south_return and actual-pose probes do.  No in-game walk
 *     path is claimed.
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
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109 = { 96, 127, 35, 63 }). */
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* Source-locked C026 atlas dimensions (DUNVIEW.C:3916-3919):
     * "A portrait is 32x29 pixels", 8 columns x 3 rows. */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 14 in the C026 atlas: (14 & 7) * 32 = 192,
     *                                 (14 >> 3) * 29 =  29. */
    ORDINAL_14_COL = 14 & 7,
    ORDINAL_14_ROW = 14 >> 3,
    ORDINAL_14_SRC_X = ORDINAL_14_COL * 32,
    ORDINAL_14_SRC_Y = ORDINAL_14_ROW * 29,
    /* Source-locked DM1 GRAPHICS.DAT asset slot for the champion
     * portrait atlas.  C026 = 26 (DUNVIEW.C:3916-3919). */
    M11_GFX_CHAMPION_PORTRAITS = 26,
    /* Match thresholds.  The D1C rect (96, 35, 32, 29) must carry
     * the ordinal-14 atlas cell at >= 90% on a live C127 sensor
     * route, and must drop to <= 30% drift after the matching
     * sensor has been disabled by F0282.  The 30% drift threshold
     * matches the existing south_return + redraw_after_candidate
     * sibling probes. */
    CORRECT_MATCH_PCT = 90,
    WRONG_MATCH_PCT = 30,
    /* The ordinal-14 cell at atlas (192, 29, 32, 29) must be a
     * defined portrait (>= 100 opaque pixels).  An unused slot
     * would fail this gate before any pixel test runs. */
    ATLAS_OPAQUE_MIN = 100,
    /* Side wall sample zones - the no-floating proof checks that
     * the portrait sprite pixels do not bleed into the left/right
     * side walls of the D1C cell band.  The portrait band is the
     * union of the cutout (96, 35, 32, 29) and a 1-pixel border
     * above and below to catch any wall-ornament frame leak. */
    SIDE_WALL_LEFT_X = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    PORTRAIT_WARM_THRESHOLD = 30,
    /* Slice target ordinal.  See file header for atlas math. */
    TARGET_ORDINAL = 14,
    /* The two pre-shuffle recruits used to drive the party-shuffle
     * mutation.  We pick HALK (ordinal 1) because it is the
     * canonical DM1 V1 C127 sensor on (1, 1) cell with sensorData=1
     * (visible from (1, 2) DIR_NORTH, see
     * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe)
     * and WUUF (ordinal 13) because it is the next south_return
     * route on the corridor (visible from (1, 5) DIR_SOUTH).  Both
     * sensors ship in real DM1 V1 DUNGEON.DAT and do not need any
     * seeding. */
    PRE_SHUFFLE_ORDINAL_1 = 1,
    PRE_SHUFFLE_ORDINAL_2 = 13,
    /* The canonical party pose for ordinal 14 is the (1, 19)
     * DIR_NORTH parked pose.  See
     * firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_
     * rect_probe.c for the full C127 sensor / visible-wall-side
     * explanation; the same parked pose is used here. */
    LEYLA_POSE_MAPX = 1,
    LEYLA_POSE_MAPY = 19,
    LEYLA_POSE_DIR = 0, /* DIR_NORTH */
    HALK_POSE_MAPX = 1,
    HALK_POSE_MAPY = 2,
    HALK_POSE_DIR = 0, /* DIR_NORTH */
    WUUF_POSE_MAPX = 1,
    WUUF_POSE_MAPY = 5,
    WUUF_POSE_DIR = 2  /* DIR_SOUTH */
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count distinct palette indices in a framebuffer rectangle. */
static int rect_distinct(const unsigned char* fb,
                         int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (!seen[idx]) { seen[idx] = 1; ++n; }
        }
    }
    return n;
}

/* Count "warm" pixels in a framebuffer rectangle.  C026 portrait
 * sprites use the warm palette set {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}
 * (green / red / orange / peach / yellow / blue) for skin tones,
 * clothing, and backgrounds.  Grey-stone wall texture uses indices
 * 0x01, 0x02, 0x0D.  This is the same warm-pixel test as
 * firestaff_dm1_v1_hall_of_champions_portrait_14_redraw_after_
 * candidate_runtime_probe.c. */
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

/* Compare the C026 portrait atlas cell for the requested ordinal
 * to the framebuffer D1C portrait rectangle.  Returns matched
 * percent (0..100), or -1 if the asset is missing.  The blit uses
 * transparentColor = 1 (m11_game_view.c:13952), so C026 pixels
 * with palette index 1 leave the wall-niche background visible and
 * are skipped on the source side. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
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
            if (src == 1) continue; /* transparent */
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W + (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count non-transparent source pixels in the C026 atlas cell for the
 * requested ordinal.  Used to verify ordinal 14 is a defined
 * portrait in the atlas (i.e. not blank / unused). */
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

/* Park the party at the (1, 19) DIR_NORTH pose that exposes the
 * (1, 18) south-wall C127 sensor with sensorData=14.  The pose
 * is OOB on the south edge of map 0 (the Hall of Champions is
 * 18x19); the engine reaches the sensor by sampling the world
 * past the south map edge, the same way the south_return sibling
 * probe parks the party. */
static void park_leyla_pose(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = LEYLA_POSE_MAPX;
    state->world.party.mapY = LEYLA_POSE_MAPY;
    state->world.party.direction = LEYLA_POSE_DIR;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

/* Park the party at a known C127 sensor pose.  Used to drive the
 * pre-shuffle recruits (HALK at (1, 2) DIR_NORTH, WUUF at (1, 5)
 * DIR_SOUTH). */
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

/* Drive a full C127 recruit cycle at the current party pose.
 *   - M11_GameView_SelectFrontMirrorCandidate -> F0280:
 *       materializes a candidate from the C127 sensorData and
 *       appends it to the party (championCount++).
 *   - M11_GameView_ConfirmMirrorCandidate(reincarnate=0) -> F0282
 *       C160 resurrect path: closes the C040 panel, disables
 *       the matching C127 sensor (m11_disable_front_mirror_route),
 *       and promotes the appended slot to activeChampionIndex.
 * Returns 1 on success, 0 on any failure.  The probe treats
 * the post-state as the "shuffled party" the slice is named for. */
static int recruit_at_current_pose(M11_GameViewState* state,
                                   int expectedOrdinal,
                                   const char* label) {
    int rc;
    int expectedCount = state->world.party.championCount + 1;
    int preOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
    if (preOrdinal != expectedOrdinal) {
        printf("  FAIL: %s front-mirror ordinal = %d (want %d)\n",
               label, preOrdinal, expectedOrdinal);
        ++g_fail;
        return 0;
    }
    rc = M11_GameView_SelectFrontMirrorCandidate(state);
    if (rc != 1) {
        printf("  FAIL: %s SelectFrontMirrorCandidate = %d\n", label, rc);
        ++g_fail;
        return 0;
    }
    if (state->world.party.championCount != expectedCount) {
        printf("  FAIL: %s post-Select championCount=%d want=%d\n",
               label, state->world.party.championCount, expectedCount);
        ++g_fail;
        return 0;
    }
    rc = M11_GameView_ConfirmMirrorCandidate(state, 0);
    if (rc != 1) {
        printf("  FAIL: %s ConfirmMirrorCandidate = %d\n", label, rc);
        ++g_fail;
        return 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(state) != -1) {
        printf("  FAIL: %s route not disabled after confirm\n", label);
        ++g_fail;
        return 0;
    }
    if (state->world.party.championCount != expectedCount) {
        printf("  FAIL: %s post-Confirm championCount=%d want=%d\n",
               label, state->world.party.championCount, expectedCount);
        ++g_fail;
        return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int ordinal14Opaque;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBaseline[FB_W * FB_H];
    unsigned char fbAfterHalk[FB_W * FB_H];
    unsigned char fbAfterWulf[FB_W * FB_H];
    unsigned char fbAfterLeyla[FB_W * FB_H];
    int matchBaseline, matchAfterHalk, matchAfterWulf, matchAfterLeyla;
    int ordBaseline, ordAfterHalk, ordAfterWulf, ordAfterLeyla;
    int warmBaseline, warmAfterHalk, warmAfterWulf, warmAfterLeyla;
    int leftSideBaseline, leftSideAfterHalk, leftSideAfterWulf, leftSideAfterLeyla;
    int rightSideBaseline, rightSideAfterHalk, rightSideAfterWulf, rightSideAfterLeyla;
    int distinctBaseline, distinctAfterHalk, distinctAfterWulf, distinctAfterLeyla;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-14 / after_party_shuffle / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GFX_CHAMPION_PORTRAITS);
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads: %ux%u (graphic %d = C026)",
                 portraits->width, portraits->height,
                 M11_GFX_CHAMPION_PORTRAITS);
        CHECK(portraits->width >= ATLAS_W && portraits->height >= ATLAS_H, msg);
    }

    /* ----------------------------------------------------------------
     * Group A - Atlas math + ordinal 14 cell sanity
     * ----------------------------------------------------------------
     * Verify the C026 atlas math for ordinal 14 (the slice target)
     * matches COORD.C / DEFS.H:821-826, and that the atlas cell is
     * a defined portrait (not blank).  This is the same gate as
     * the sibling redraw_after_candidate + south_return probes. */
    printf("\n[Group A] C026 atlas math for ordinal 14\n");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 col = %d (expected 6)", ORDINAL_14_COL);
        CHECK(ORDINAL_14_COL == 6, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 row = %d (expected 1)", ORDINAL_14_ROW);
        CHECK(ORDINAL_14_ROW == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 srcX=%d, srcY=%d within %dx%d atlas",
                 ORDINAL_14_SRC_X, ORDINAL_14_SRC_Y, ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_14_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_14_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }
    ordinal14Opaque = atlas_cell_opaque_count(portraits, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas cell has >= %d opaque pixels "
                 "(got %d) - defined portrait, not blank",
                 TARGET_ORDINAL, ATLAS_OPAQUE_MIN, ordinal14Opaque);
        CHECK(ordinal14Opaque >= ATLAS_OPAQUE_MIN, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - Baseline (empty party) ordinal 14 proof
     * ----------------------------------------------------------------
     * Park the party at (1, 19) DIR_NORTH and confirm the D1C
     * portrait rect (96, 35, 32, 29) carries the ordinal-14 atlas
     * cell.  The pose is OOB on the south edge of map 0; the
     * engine reaches the (1, 18) south-wall C127 sensor by
     * sampling the world past the south map edge, the same way
     * the south_return sibling probe does. */
    printf("\n[Group B] baseline: empty party at (1, 19) DIR_NORTH\n");
    park_leyla_pose(&state);
    state.world.party.championCount = 0;

    ordBaseline = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1, 19) DIR_NORTH = %d (expected %d)",
                 ordBaseline, TARGET_ORDINAL);
        CHECK(ordBaseline == TARGET_ORDINAL, msg);
    }

    /* Sanity-check the public D1C wall-ornament zone helper
     * (DUNVIEW.C G0205 coordSet 5 / index 12 -> C346 D1C frame
     * (80, 29, 64, 43)) and verify the inner portrait rect is
     * inside it. */
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "(DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35, 32, 29) inside the D1C "
                 "wall ornament zone");
        CHECK(96 >= ornX && 96 + D1C_PORTRAIT_W <= ornX + ornW &&
              35 >= ornY && 35 + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    memset(fbBaseline, 0, sizeof(fbBaseline));
    M11_GameView_Draw(&state, fbBaseline, FB_W, FB_H);
    matchBaseline = match_portrait_at_rect(portraits, fbBaseline, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect matches ordinal %d >= %d%% (got %d%%)",
                 TARGET_ORDINAL, CORRECT_MATCH_PCT, matchBaseline);
        CHECK(matchBaseline >= CORRECT_MATCH_PCT, msg);
    }
    warmBaseline = rect_warm_count(fbBaseline,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect has >= %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, warmBaseline);
        CHECK(warmBaseline >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    distinctBaseline = rect_distinct(fbBaseline,
                                     D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                     D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect has >= 4 distinct palette indices "
                 "(got %d)", distinctBaseline);
        CHECK(distinctBaseline >= 4, msg);
    }

    /* No-floating proof: the side walls of the D1C portrait band
     * must not carry the portrait's warm pixels.  This is the
     * source-locked "no float on side walls" invariant from
     * DUNVIEW.C:8318-8618 F0128 far-to-near redraw. */
    leftSideBaseline = rect_warm_count(fbBaseline,
                                       SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                       SIDE_WALL_LEFT_W,
                                       PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideBaseline = rect_warm_count(fbBaseline,
                                        SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_RIGHT_W,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline left side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideBaseline);
        CHECK(leftSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline right side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideBaseline);
        CHECK(rightSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - Recruit HALK (ordinal 1) at (1, 2) DIR_NORTH
     * ----------------------------------------------------------------
     * Drive the F0280 + F0282 C160 resurrect path to append HALK
     * to the party.  This is the first "party shuffle": the
     * party goes from 0 -> 1 champion, activeChampionIndex
     * rotates to 0, and the (1, 1) C127 sensor with
     * sensorData=1 is reset to sensorType=0 by
     * m11_disable_front_mirror_route (REVIVE.C F0282). */
    printf("\n[Group C] recruit HALK at (1, 2) DIR_NORTH (party 0 -> 1)\n");
    park_pose(&state, HALK_POSE_MAPX, HALK_POSE_MAPY, HALK_POSE_DIR);
    state.world.party.championCount = 0;
    if (recruit_at_current_pose(&state, PRE_SHUFFLE_ORDINAL_1, "HALK")) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK championCount = %d, activeChampionIndex = %d",
                 state.world.party.championCount,
                 state.world.party.activeChampionIndex);
        CHECK(state.world.party.championCount == 1, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - After HALK shuffle, ordinal 14 still visible
     * ----------------------------------------------------------------
     * Move back to (1, 19) DIR_NORTH and re-render.  The D1C
     * portrait rect must STILL carry the ordinal-14 atlas cell
     * (match >= 90%, warm pixels >= threshold, side walls
     * clean).  This is the core invariant of the slice: the
     * HALK recruit on (1, 1) does not bleach the LEYLA rect at
     * (96, 35, 32, 29). */
    printf("\n[Group D] after HALK recruit: (1, 19) DIR_NORTH still shows ordinal 14\n");
    park_leyla_pose(&state);
    /* championCount is left at 1 from Group C; do not zero it. */

    ordAfterHalk = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK front-mirror ordinal at (1, 19) DIR_NORTH = %d "
                 "(expected %d)",
                 ordAfterHalk, TARGET_ORDINAL);
        CHECK(ordAfterHalk == TARGET_ORDINAL, msg);
    }

    memset(fbAfterHalk, 0, sizeof(fbAfterHalk));
    M11_GameView_Draw(&state, fbAfterHalk, FB_W, FB_H);
    matchAfterHalk = match_portrait_at_rect(portraits, fbAfterHalk, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK D1C rect matches ordinal %d >= %d%% (got %d%%)",
                 TARGET_ORDINAL, CORRECT_MATCH_PCT, matchAfterHalk);
        CHECK(matchAfterHalk >= CORRECT_MATCH_PCT, msg);
    }
    warmAfterHalk = rect_warm_count(fbAfterHalk,
                                    D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                    D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK D1C rect has >= %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, warmAfterHalk);
        CHECK(warmAfterHalk >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    distinctAfterHalk = rect_distinct(fbAfterHalk,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK D1C rect has >= 4 distinct palette indices "
                 "(got %d)", distinctAfterHalk);
        CHECK(distinctAfterHalk >= 4, msg);
    }
    leftSideAfterHalk = rect_warm_count(fbAfterHalk,
                                        SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_LEFT_W,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterHalk = rect_warm_count(fbAfterHalk,
                                         SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_RIGHT_W,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK left side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterHalk);
        CHECK(leftSideAfterHalk < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-HALK right side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterHalk);
        CHECK(rightSideAfterHalk < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - Recruit WUUF (ordinal 13) at (1, 5) DIR_SOUTH
     * ----------------------------------------------------------------
     * Drive the F0280 + F0282 path again to push championCount
     * from 1 -> 2.  activeChampionIndex rotates to slot 1.
     * The (1, 6) south-wall C127 sensor with sensorData=13 is
     * reset to sensorType=0 by m11_disable_front_mirror_route. */
    printf("\n[Group E] recruit WUUF at (1, 5) DIR_SOUTH (party 1 -> 2)\n");
    park_pose(&state, WUUF_POSE_MAPX, WUUF_POSE_MAPY, WUUF_POSE_DIR);
    if (recruit_at_current_pose(&state, PRE_SHUFFLE_ORDINAL_2, "WUUF")) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF championCount = %d, activeChampionIndex = %d",
                 state.world.party.championCount,
                 state.world.party.activeChampionIndex);
        CHECK(state.world.party.championCount == 2, msg);
    }

    /* ----------------------------------------------------------------
     * Group F - After WUUF shuffle, ordinal 14 still visible
     * ----------------------------------------------------------------
     * Move back to (1, 19) DIR_NORTH and re-render.  Same
     * invariant as Group D, now with a 2-champion party.  The
     * D1C portrait rect must STILL carry ordinal 14. */
    printf("\n[Group F] after WUUF recruit: (1, 19) DIR_NORTH still shows ordinal 14\n");
    park_leyla_pose(&state);

    ordAfterWulf = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF front-mirror ordinal at (1, 19) DIR_NORTH = %d "
                 "(expected %d)",
                 ordAfterWulf, TARGET_ORDINAL);
        CHECK(ordAfterWulf == TARGET_ORDINAL, msg);
    }

    memset(fbAfterWulf, 0, sizeof(fbAfterWulf));
    M11_GameView_Draw(&state, fbAfterWulf, FB_W, FB_H);
    matchAfterWulf = match_portrait_at_rect(portraits, fbAfterWulf, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF D1C rect matches ordinal %d >= %d%% (got %d%%)",
                 TARGET_ORDINAL, CORRECT_MATCH_PCT, matchAfterWulf);
        CHECK(matchAfterWulf >= CORRECT_MATCH_PCT, msg);
    }
    warmAfterWulf = rect_warm_count(fbAfterWulf,
                                    D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                    D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF D1C rect has >= %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, warmAfterWulf);
        CHECK(warmAfterWulf >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    distinctAfterWulf = rect_distinct(fbAfterWulf,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF D1C rect has >= 4 distinct palette indices "
                 "(got %d)", distinctAfterWulf);
        CHECK(distinctAfterWulf >= 4, msg);
    }
    leftSideAfterWulf = rect_warm_count(fbAfterWulf,
                                        SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_LEFT_W,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterWulf = rect_warm_count(fbAfterWulf,
                                         SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_RIGHT_W,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF left side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterWulf);
        CHECK(leftSideAfterWulf < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-WUUF right side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterWulf);
        CHECK(rightSideAfterWulf < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group G - Recruit LEYLA (ordinal 14) at (1, 19) DIR_NORTH
     * ----------------------------------------------------------------
     * Drive the F0280 + F0282 path a third time to recruit the
     * LEYLA route itself.  championCount goes 2 -> 3,
     * activeChampionIndex rotates to slot 2, and the (1, 18)
     * south-wall C127 sensor with sensorData=14 is reset to
     * sensorType=0 by m11_disable_front_mirror_route. */
    printf("\n[Group G] recruit LEYLA at (1, 19) DIR_NORTH (party 2 -> 3)\n");
    park_leyla_pose(&state);
    if (recruit_at_current_pose(&state, TARGET_ORDINAL, "LEYLA")) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA championCount = %d, activeChampionIndex = %d",
                 state.world.party.championCount,
                 state.world.party.activeChampionIndex);
        CHECK(state.world.party.championCount == 3, msg);
    }

    /* ----------------------------------------------------------------
     * Group H - After LEYLA recruit, route is disabled and the
     * D1C rect must no longer carry the ordinal-14 portrait.
     * ----------------------------------------------------------------
     * The C127 sensor has been reset to sensorType=0 by F0282, so
     * the engine's m11_draw_dm1_front_champion_portrait guard at
     * m11_game_view.c:13960 suppresses the C026 blit (the rect
     * falls back to the C346 D1C wall-ornament placeholder). */
    printf("\n[Group H] after LEYLA recruit: route disabled, D1C rect drops ordinal 14\n");

    ordAfterLeyla = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA front-mirror ordinal at (1, 19) DIR_NORTH = %d "
                 "(expected -1)",
                 ordAfterLeyla);
        CHECK(ordAfterLeyla == -1, msg);
    }

    memset(fbAfterLeyla, 0, sizeof(fbAfterLeyla));
    M11_GameView_Draw(&state, fbAfterLeyla, FB_W, FB_H);
    matchAfterLeyla = match_portrait_at_rect(portraits, fbAfterLeyla, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA D1C rect does NOT match ordinal %d <= %d%% "
                 "(got %d%%) - portrait is no longer painted",
                 TARGET_ORDINAL, WRONG_MATCH_PCT, matchAfterLeyla);
        CHECK(matchAfterLeyla <= WRONG_MATCH_PCT, msg);
    }
    warmAfterLeyla = rect_warm_count(fbAfterLeyla,
                                     D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                     D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA D1C rect has < %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, warmAfterLeyla);
        CHECK(warmAfterLeyla < PORTRAIT_WARM_THRESHOLD, msg);
    }
    distinctAfterLeyla = rect_distinct(fbAfterLeyla,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA D1C rect distinct palette count <= "
                 "baseline (off=%d, on=%d)",
                 distinctBaseline, distinctAfterLeyla);
        CHECK(distinctAfterLeyla <= distinctBaseline, msg);
    }
    leftSideAfterLeyla = rect_warm_count(fbAfterLeyla,
                                         SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_LEFT_W,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterLeyla = rect_warm_count(fbAfterLeyla,
                                          SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_RIGHT_W,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA left side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterLeyla);
        CHECK(leftSideAfterLeyla < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-LEYLA right side wall warm pixels < %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterLeyla);
        CHECK(rightSideAfterLeyla < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group I - Cross-check: every (1, 19) DIR_NORTH frame for
     * ordinal 14 also rejects the four adjacent atlas cells.
     * ----------------------------------------------------------------
     * The adjacent cells (5,1), (7,1), (6,0), (6,2) share
     * background-palette indices with (6,1) on the local C026
     * strip, so a single 30% cut-point reliably catches "wrong
     * atlas cell" while leaving headroom for the correct one. */
    printf("\n[Group I] cross-check: ordinal 14 specifically, not adjacent atlas cells\n");
    {
        const int kAdjacents[4] = {
            TARGET_ORDINAL - 1, /* (col 5, row 1) */
            TARGET_ORDINAL + 1, /* (col 7, row 1) */
            TARGET_ORDINAL - 8, /* (col 6, row 0) */
            TARGET_ORDINAL + 8  /* (col 6, row 2) */
        };
        const char* kAdjLabels[4] = {
            "ordinal-1 (col 5, row 1)",
            "ordinal+1 (col 7, row 1)",
            "ordinal-8 (col 6, row 0)",
            "ordinal+8 (col 6, row 2)"
        };
        int i;
        for (i = 0; i < 4; ++i) {
            int pctAdj = match_portrait_at_rect(portraits, fbBaseline,
                                                kAdjacents[i]);
            char msg[200];
            if (pctAdj < 0) continue;
            snprintf(msg, sizeof(msg),
                     "baseline D1C rect does NOT match %s <= %d%% "
                     "(got %d%%)",
                     kAdjLabels[i], WRONG_MATCH_PCT, pctAdj);
            CHECK(pctAdj < WRONG_MATCH_PCT, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group J - Re-park sanity check
     * ----------------------------------------------------------------
     * The LEYLA route is now disabled in the world, so we cannot
     * expect the rect to still carry ordinal 14.  The relevant
     * invariant is that re-parking at (1, 19) DIR_NORTH yields
     * frontMirrorOrdinal=-1, which we already proved in Group H.
     * This group just confirms the parked pose did not
     * accidentally re-enable the C127 sensor on (1, 18). */
    printf("\n[Group J] re-park sanity check\n");
    park_leyla_pose(&state);
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "re-park (1, 19) DIR_NORTH ordinal = %d (expected -1; "
                 "LEYLA C127 sensor was disabled by F0282)",
                 ord);
        CHECK(ord == -1, msg);
    }

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
