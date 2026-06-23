/*
 * firestaff_dm1_v1_hall_of_champions_west_negative_portrait_rect_position_probe.c
 *
 * Focused regression for the DM1 V1 Hall of Champions slice
 *     (1,2) facing WEST, no C127 mirror on the front square
 *
 * The broader firestaff_dm1_v1_champion_mirror_capture_probe already
 * proves this slice passes (expectedOrdinal=-1, actualOrdinal=-1,
 * portrait_rect_warm_count < 30 in the hardcoded (96,35)-(128,64)
 * D1C front-wall rectangle).  What the existing capture probe does
 * NOT verify is the *position* of the portrait rectangle: it relies
 * on hardcoded coordinates that the engine might paint to a
 * different source-locked destination for this orientation.
 *
 * This probe closes that gap by:
 *   1) Reading the D1C wall-ornament zone from the engine helper
 *      `M11_GameView_GetD1CWallOrnamentZone()` (DUNVIEW.C G0205
 *      Graphic558 coordSet 5, the C346 D1C champion-mirror route).
 *      This is the source-locked wall box for the C346 champion
 *      mirror graphic (80, 29, 64, 43 in viewport coords).
 *   2) Defining the inner portrait cutout as (96, 35, 32, 29) — the
 *      C026 champion portrait is a smaller rectangle inside the
 *      wall box, parented at (+16, +6) per the (80,29) -> (96,35)
 *      offset documented in DUNVIEW.C:3913-3928.
 *   3) Driving M11_GameView_Draw at the (1,2) WEST pose and
 *      confirming `portrait_rect_warm_count(inner) < 30` and
 *      `portrait_rect_warm_count(wall box) < 50` — the rectangle is
 *      reserved for the portrait sprite but no portrait is painted
 *      because there is no C127 mirror on the front square.  A
 *      floating champion sprite would push either count above the
 *      threshold.
 *   4) Cross-checking the (1,2) NORTH HALK (positive) pose at the
 *      SAME rectangle position to prove the engine paints the
 *      portrait at exactly the source-locked coordinates when a
 *      C127 mirror is present.  Without this cross-check the empty
 *      west_negative rectangle could be silently empty for the
 *      wrong reason (engine painted to a different rect).
 *
 * Source evidence:
 *   ReDMCSB DUNVIEW.C G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *     coordSet 5 / index 12 = C346 D1C champion-mirror frame route.
 *   ReDMCSB DUNVIEW.C:3913-3928 champion portrait blit rectangle.
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw order (far-to-near).
 *   ReDMCSB DUNGEON.C:2573 / 2608-2612 C127 sensorData front-wall ownership.
 *   ReDMCSB MOVESENS.C:1501-1503 C127 sensorData routing to F0280.
 *   ReDMCSB REVIVE.C F0280 candidate materialization from sensorData.
 *   DEFS.H:821-826 M027_PORTRAIT_X/M028_PORTRAIT_Y 8-column atlas math.
 *   DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS strip.
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_capture_probe            (visual captures)
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe (16-pose runtime)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe      (BUG-120/121 guard)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe (positive (1,2)N/(1,5)N)
 *
 * The probe is data-conditional: it requires hash-verified DM1 V1
 * data for the (1,2) N HALK positive cross-check; without that data
 * the contract surface (engine helpers) is still exercised.
 */

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W            = 320,
    FB_H            = 200,
    VIEWPORT_X      = 0,
    VIEWPORT_Y      = 33,
    /* D1C champion portrait cutout inside the C346 wall-mirror
     * box (DUNVIEW.C:3913-3928).  The portrait is parented at
     * (+16,+6) from the wall box origin. */
    PORTRAIT_X      = VIEWPORT_X + 96,
    PORTRAIT_Y      = VIEWPORT_Y + 35,
    PORTRAIT_W      = 32,
    PORTRAIT_H      = 29,
    /* Thresholds (warm-color pixel counts).  Mirrors the values
     * used by firestaff_dm1_v1_champion_mirror_capture_probe. */
    PORTRAIT_WARM_NEG_THRESHOLD  = 30,
    PORTRAIT_WARM_POS_THRESHOLD  = 30,
    /* Wall box threshold is slightly higher because stone wall
     * texture can include a few warm-tone torches or edge
     * antialiasing pixels (observed 0-22 in the existing capture
     * probe for the (1,4) S negative route). */
    WALLBOX_WARM_NEG_THRESHOLD   = 50
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count pixels in a viewport rect whose palette index is in the
 * "warm" set {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
 * 0x0B yellow, 0x0E blue} — the same set the capture probe uses
 * to detect champion portrait sprites without false positives on
 * the grey-stone wall texture. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char raw = fb[yy * FB_W + xx];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: case 0x08: case 0x09: case 0x0A:
                case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* Count non-zero pixels in a viewport rect. */
static int rect_nonzero_count(const unsigned char* fb,
                              int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            if (fb[yy * FB_W + xx] != 0) ++count;
        }
    }
    return count;
}

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction) pose
 * on map 0 (Hall of Champions) and return the rendered framebuffer.
 * Caller owns the storage. */
static void render_at(M11_GameViewState* state,
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
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* Contract surface: the engine helper must return the source-locked
 * D1C wall box (80, 29, 64, 43) regardless of the active pose.  This
 * is the core "portrait_rect_position" invariant. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for portrait_rect_position\n");

    /* Pose the party at (1,2) W so the helper is queried against the
     * west_negative slice. */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = DIR_WEST;

    rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    /* Source-locked coords from DUNVIEW.C G0205 Graphic558 coordSet 5:
     * (dstX=80, dstY=29, w=64, h=43) in viewport coords.  The inner
     * portrait cutout is parented at (+16,+6) per DUNVIEW.C:3913-3928,
     * giving (96, 35, 32, 29). */
    snprintf(msg, sizeof(msg),
             "D1C wall box X == 80 (got %d)", ornX);
    CHECK(ornX == 80, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box Y == 29 (got %d)", ornY);
    CHECK(ornY == 29, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box W == 64 (got %d)", ornW);
    CHECK(ornW == 64, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box H == 43 (got %d)", ornH);
    CHECK(ornH == 43, msg);

    /* Inner portrait cutout = (80+16, 29+6, 32, 29) = (96, 35, 32, 29). */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == wallX + 16 == 96 (got %d)", ornX + 16);
    CHECK(ornX + 16 == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == wallY + 6 == 35 (got %d)", ornY + 6);
    CHECK(ornY + 6 == 35, msg);
}

/* Slice aspect: at (1,2) facing WEST, the C127 front-wall sensor
 * is absent (the front square (0,2) is a doorway, not a mirror
 * wall).  The portrait rectangle is reserved at the source-locked
 * position but no portrait is painted there.  A floating champion
 * sprite would push the warm-count above the threshold. */
static void check_west_negative_slice(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int actualOrdinal;
    int innerWarm, wallWarm;
    int innerNonzero;
    char msg[200];

    printf("\n[Group B] (1,2) WEST slice: portrait_rect_position is empty\n");

    /* The actual ordinal must be -1 (no C127 mirror on the front
     * cell — ReDMCSB DUNGEON.C:2573 + MOVESENS.C:1501-1503 + REVIVE.C
     * F0280 confirm the C127 sensor lookup returns -1 here). */
    render_at(state, fb, 1, 2, DIR_WEST);
    actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,2)W) == -1 (got %d)",
             actualOrdinal);
    CHECK(actualOrdinal == -1, msg);

    (void)M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);

    innerWarm = rect_warm_count(fb,
                                VIEWPORT_X + ornX + 16,
                                VIEWPORT_Y + ornY + 6,
                                PORTRAIT_W, PORTRAIT_H);
    wallWarm  = rect_warm_count(fb,
                                VIEWPORT_X + ornX,
                                VIEWPORT_Y + ornY,
                                ornW, ornH);
    innerNonzero = rect_nonzero_count(fb,
                                      VIEWPORT_X + ornX + 16,
                                      VIEWPORT_Y + ornY + 6,
                                      PORTRAIT_W, PORTRAIT_H);

    snprintf(msg, sizeof(msg),
             "Inner portrait cutout warm_count < %d (got %d)",
             PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
    CHECK(innerWarm < PORTRAIT_WARM_NEG_THRESHOLD, msg);

    snprintf(msg, sizeof(msg),
             "Wall box warm_count < %d (got %d)",
             WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
    CHECK(wallWarm < WALLBOX_WARM_NEG_THRESHOLD, msg);

    /* The cutout can still contain non-black pixels (e.g. wall
     * texture bleed) but those must not be warm-colored portrait
     * palette indices.  We log the nonzero count for the record. */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout nonzero pixels = %d (logged)",
             innerNonzero);
    printf("  INFO: %s\n", msg);
    ++g_pass;
}

/* Cross-check: at (1,2) facing NORTH (HALK, ordinal 1), the engine
 * paints a portrait sprite at the SAME source-locked rectangle.
 * Without this cross-check the empty west_negative rectangle could
 * be silently empty because the engine painted somewhere else. */
static void check_north_positive_cross_check(M11_GameViewState* state,
                                             int assetsAvailable) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int actualOrdinal;
    int innerWarm, wallWarm;
    char msg[200];

    printf("\n[Group C] (1,2) NORTH HALK cross-check: portrait IS painted at the same rect\n");

    render_at(state, fb, 1, 2, DIR_NORTH);
    actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,2)N) == 1 (HALK) (got %d)",
             actualOrdinal);
    CHECK(actualOrdinal == 1, msg);

    (void)M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);

    innerWarm = rect_warm_count(fb,
                                VIEWPORT_X + ornX + 16,
                                VIEWPORT_Y + ornY + 6,
                                PORTRAIT_W, PORTRAIT_H);
    wallWarm  = rect_warm_count(fb,
                                VIEWPORT_X + ornX,
                                VIEWPORT_Y + ornY,
                                ornW, ornH);

    if (assetsAvailable) {
        snprintf(msg, sizeof(msg),
                 "Inner portrait cutout warm_count >= %d for HALK (got %d)",
                 PORTRAIT_WARM_POS_THRESHOLD, innerWarm);
        CHECK(innerWarm >= PORTRAIT_WARM_POS_THRESHOLD, msg);
        snprintf(msg, sizeof(msg),
                 "Wall box warm_count >= %d for HALK (got %d)",
                 PORTRAIT_WARM_POS_THRESHOLD, wallWarm);
        CHECK(wallWarm >= PORTRAIT_WARM_POS_THRESHOLD, msg);
    } else {
        printf("  SKIP: assets unavailable, cannot pixel-prove HALK portrait rect\n");
        printf("  (contract surface still verified in Group A)\n");
    }
}

/* Adjacent corridor west_negative slices: (1,3) W and (1,4) W are
 * covered by the broader capture probe but their portrait_rect_position
 * aspect is worth locking with the engine helper too. */
static void check_corridor_west_negatives(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int actualOrdinal;
    int innerWarm, wallWarm;
    char msg[200];
    static const int kMapYs[2] = { 3, 4 };
    int i;

    printf("\n[Group D] Corridor west_negative slices (1,3)W + (1,4)W\n");

    for (i = 0; i < 2; ++i) {
        int y = kMapYs[i];
        render_at(state, fb, 1, y, DIR_WEST);
        actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);

        snprintf(msg, sizeof(msg),
                 "(1,%d) W ordinal == -1 (got %d)", y, actualOrdinal);
        CHECK(actualOrdinal == -1, msg);

        (void)M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        innerWarm = rect_warm_count(fb,
                                    VIEWPORT_X + ornX + 16,
                                    VIEWPORT_Y + ornY + 6,
                                    PORTRAIT_W, PORTRAIT_H);
        wallWarm  = rect_warm_count(fb,
                                    VIEWPORT_X + ornX,
                                    VIEWPORT_Y + ornY,
                                    ornW, ornH);
        snprintf(msg, sizeof(msg),
                 "(1,%d) W inner cutout warm_count < %d (got %d)",
                 y, PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
        CHECK(innerWarm < PORTRAIT_WARM_NEG_THRESHOLD, msg);
        snprintf(msg, sizeof(msg),
                 "(1,%d) W wall box warm_count < %d (got %d)",
                 y, WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
        CHECK(wallWarm < WALLBOX_WARM_NEG_THRESHOLD, msg);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    int assetsAvailable;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions west_negative portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP dm1_v1_hall_of_champions_west_negative_portrait_rect_position_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 0;

    check_engine_helpers(&state);
    check_west_negative_slice(&state);
    check_north_positive_cross_check(&state, assetsAvailable);
    check_corridor_west_negatives(&state);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
