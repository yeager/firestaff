/*
 * firestaff_dm1_v1_hoc_champion_portrait_22_approach_from_right_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   ordinal 22             (mirror catalog record GOTHMOG, atlas col 6
 *                           row 2 -- the bottom row of the 8x3 C026
 *                           strip; ordinal 22 is an UNTITLED champion
 *                           per the DM1 V1 PC 3.4 catalog)
 *   route  approach_from_right: approach the ordinal-22 C127 sensor
 *                              cell (2, 6) from the right (north) side.
 *                              The sensor is anchored to the EAST wall
 *                              of (2, 6) (M011_CELL=1) -- the player at
 *                              (3, 6) DIR_WEST sees the front cell's
 *                              EAST wall (visibleWallCell=(W+2)&3=1)
 *                              under DUNGEON.C:2573, which matches the
 *                              sensor's M011_CELL=1.  Right-side poses
 *                              try to view the sensor cell from the
 *                              wrong side or from the wrong cell:
 *
 *                                - (3, 5) DIR_WEST     front=(2, 5)
 *                                  wrong cell, right wall side
 *                                - (3, 7) DIR_WEST     front=(2, 7)
 *                                  wrong cell, right wall side
 *                                - (3, 6) DIR_SOUTH    front=(3, 7)
 *                                  right cell, wrong wall side
 *                                - (3, 6) DIR_EAST     front=(4, 6)
 *                                  right cell, wrong wall side
 *
 *                              The (3, 6) DIR_NORTH pose is NOT in
 *                              this band -- it fires ordinal 11 (a
 *                              different champion's mirror, on the
 *                              (3, 5) front cell's C127 sensor),
 *                              so it is not a wrong-side pose for
 *                              ordinal 22.  The (3, 6) DIR_SOUTH
 *                              pose is the in-place rotation analog
 *                              that does not trigger a different
 *                              mirror ordinal in DM1 V1.
 *
 *                              The D1C portrait_rect_position
 *                              (96, 35, 32, 29) in viewport coords
 *                              (DUNVIEW.C:525 G0109 portrait box
 *                              {96, 127, 35, 63}, source-locked inner
 *                              cutout) must therefore contain no
 *                              ordinal-22 pixels at any of the
 *                              right-side approach poses.  A regression
 *                              that re-paints ordinal 22 over the side
 *                              walls would push the C026 ordinal-22
 *                              pixel match above the 35% wrong-ordinal
 *                              drift threshold the existing per-ordinal
 *                              west_negative / south_return probes lock.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 + portrait atlas math (col 6 row 2,
 *                                 source rect (192, 58, 32, 29)) and the
 *                                 +1 (3, 6, W) positive cross-check and
 *                                 +1 atlas round-trip.
 *
 * The probe is the source-visible right-side counterpart of
 * firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime_probe
 * (which covers the (1, 2, N) front-wall + sensor-rewrite seed on the
 * HALK corridor cell) and
 * firestaff_dm1_v1_hall_of_champions_portrait_22_redraw_after_candidate
 * _runtime_probe (which covers the +1 (1, 2, N) redraw-after-candidate
 * slice on the HALK corridor cell).  Neither probe covers the
 * approach_from_right band on the natural ordinal-22 (3, 6, W) route
 * that the actual_pose probe locks for this DM1 V1 DUNGEON.DAT fixture.
 *
 * The shipped DM1 V1 PC 3.4 DUNGEON.DAT exposes ordinal 22 at exactly
 * one pose: (3, 6) DIR_WEST.  The exhaustAny-pose probe at
 * firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime
 * _probe.c locked this via the 16x16 cell x 4 direction scan and
 * reported exactly one hit ("HIT: ordinal 22 at pose=(map=0, x=3,
 * y=6, dir=3)").  This probe narrows the geographic coverage to the
 * right-side approach band around that natural route and locks the
 * no-floating invariant for ordinal 22.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 maps M011_CELL(sensor) against view direction
 *     (PC 3.4 I34E builds, M552=5; visibleWallCell = (dir+2)&3)
 *   - DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   - DUNVIEW.C:3913-3928 / 8522-8533 C026 portrait blit into
 *     G0109 portrait box (D1C only -- M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:3916-3919 C026_GRAPHIC_CHAMPION_PORTRAITS,
 *     "A portrait is 32x29 pixels"
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29
 *   - COORD.C:1693-1749 PC34 viewport origin
 *   - DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro math
 *   - MOVESENS.C:1501-1503 sensorData -> F0280 candidate ordinal
 *   - REVIVE.C F0280:124-132 C040 empty-leader candidate gate
 *   - DEFS.H:1284 C127_SENSOR_WALL_CHAMPION_PORTRAIT=127
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   - m11_front_cell_mirror_ordinal (src/engine/m11_game_view.c:11652)
 *   - m11_draw_dm1_front_champion_portrait
 *     (src/engine/m11_game_view.c:13952 -- D1C only)
 *   - m11_get_front_cell (src/engine/m11_game_view.c:11708)
 *   - m11_disable_front_mirror_route
 *     (src/engine/m11_game_view.c:7898 -- F0282 sensor disable)
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - exhaustAny-pose 16x16 scan that locks (3, 6, W) as the only
 *       ordinal-22 pose.  This probe assumes that result and adds
 *       the right-side approach band.
 *   firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime_probe
 *     - the ordinal-22 front_north_entry seed slice on the (1, 2, N)
 *       HALK corridor cell.  Covers atlas math, front_north_entry
 *       ordinal, ANY-pose discovery on map 0, no-floating on the
 *       (1, y) corridor cells, and the (1, 2, N) C127 sensor-rewrite
 *       seed from HALK to GOTHMOG.  Does NOT cover the right-side
 *       approach band on the natural (3, 6, W) ordinal-22 route.
 *   firestaff_dm1_v1_hall_of_champions_portrait_22_redraw_after_candidate_runtime_probe
 *     - the +1 (1, 2, N) redraw_after_candidate slice on the HALK
 *       corridor cell.  Covers atlas round-trip, C026 strip math,
 *       redraw stability, side-wall no-floating, candidate panel
 *       suppression.  Does NOT cover the right-side approach band
 *       on the natural (3, 6, W) ordinal-22 route.
 *   firestaff_dm1_v1_hoc_champion_portrait_05_approach_from_right_portrait_rect_position_runtime_probe
 *     - the ordinal-5 (ELIJA) right-side approach slice.  Tests
 *       poses that try to view the (1, 1) NORTH sensor cell from
 *       the wrong side.  Ordinal 22's sensor is on (2, 6) EAST wall,
 *       a different route geometry -- the right-side poses are
 *       different (right of the EAST wall is NORTH, so right-side
 *       poses are (3, 5, W), (3, 7, W), (3, 6, N), (3, 6, E)).
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left_portrait_rect_position_runtime_probe
 *     - the ordinal-4 (LEIF) left-side approach slice on the
 *       (1, 2, E) corridor anchor.  Disjoint geometry and pose.
 *
 * Honesty:
 *   This is Firestaff deterministic runtime evidence.  It does NOT
 *   claim DOS pixel parity because no paired original DM1 PC 3.4
 *   screenshot covers the ordinal-22 (3, 6) wall from the (3, 5)
 *   west or (3, 7) west poses (those are non-source-visible wrong-
 *   side approaches).  The probe drives real Firestaff game-view
 *   state through the same M11 input pipeline the live game uses,
 *   and the no-floating pixel contract is computed against the
 *   local C026 strip pulled from the same GRAPHICS.DAT the runtime
 *   is drawing from.
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
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109).  Inner cutout is the (96, 35, 32, 29)
     * portion of the (80, 29, 64, 43) wall ornament box. */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* The C026 champion portrait atlas is 256x87 pixels (8 cols x
     * 3 rows of 32x29 portraits, DUNVIEW.C:3916-3919).  Ordinal 22
     * sits at (col=6, row=2) which the C027 macro math from
     * DEFS.H:821-826 computes as ((22 & 7) << 5, (22 >> 3) * 29) =
     * (192, 58).  This is the BOTTOM row of the 8x3 atlas; row 2
     * is the only row where srcY + 29 == atlasH (87). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    ORDINAL_22_COL = 22 & 7,
    ORDINAL_22_ROW = 22 >> 3,
    ORDINAL_22_SRC_X = ORDINAL_22_COL << 5,
    ORDINAL_22_SRC_Y = ORDINAL_22_ROW * 29,
    /* Canonical ordinal-22 route in shipped DM1 V1 PC 3.4 DUNGEON.DAT
     * (verified by firestaff_dm1_v1_champion_mirror_actual_pose_runtime
     * _probe and the ordinal-22 front_north_entry probe's ANY-pose
     * scan): (3, 6) DIR_WEST=3.  The sensor is on the EAST wall of
     * the front cell (2, 6) (visibleWallCell = (W+2)&3 = 1 = EAST). */
    CANONICAL_MAP_X = 3,
    CANONICAL_MAP_Y = 6,
    CANONICAL_DIR = 3,                /* DIR_WEST */
    TARGET_ORDINAL = 22,
    /* The wrong-ordinal drift threshold the existing per-ordinal
     * west_negative / south_return / east_walkpath probes lock.
     * Above 35% means a stale portrait sprite is floating on a
     * wall that should not carry the ordinal.  Same threshold
     * firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative
     * _portrait_rect_position_runtime_probe locks. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* Cross-check threshold for the +1 (3, 6, W) positive route:
     * the D1C cutout must carry the ordinal-22 portrait at
     * >= 90% pixel match. */
    POSITIVE_ORDINAL_MATCH_PCT = 90,
    /* Match floor for the corridor / surround to prove the
     * framebuffer is alive at the right-side approach poses. */
    RECT_ALIVE_DISTINCT = 3,
    /* Dark-gray transparency mask (DUNVIEW.C:3916, C01_COLOR_DARK_GRAY). */
    PORTRAIT_TRANSPARENT = 1
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Mirror catalog record name for ordinal 22.  GOTHMOG is a real
 * champion entry in the DM1 V1 PC 3.4 mirror catalog but is UNTITLED
 * (no title in the catalog), per the ordinal-22 front_north_entry
 * probe's Group D check. */
static const char kExpectedCatalogName[] = "GOTHMOG";

/* Pixel-match a single 32x29 C026 cell against the D1C portrait
 * cutout.  Returns matched-percent (0..100) or -1 if the asset is
 * missing.  Source pixels with palette index 1 (C01_COLOR_DARK_GRAY
 * blitter transparent) are skipped so the wall-niche background
 * bleed does not skew the match.  Same algorithm the per-ordinal
 * west_negative probes and the ordinal-5 approach_from_right probe
 * use. */
static int match_portrait_cell(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0, compared = 0, x, y;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if ((int)portraits->width < ATLAS_W || (int)portraits->height < ATLAS_H) {
        return -1;
    }
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = M11_FB_DECODE_INDEX(
                fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            if (src == PORTRAIT_TRANSPARENT) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count opaque (non-zero, non-transparent) pixels in the C026
 * atlas cell for the requested ordinal.  Used to lock the
 * ordinal-22 atlas cell is a defined portrait and not an unused
 * slot. */
static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * PORTRAIT_W;
    srcY = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int sx = srcX + x, sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width || sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != PORTRAIT_TRANSPARENT) ++cnt;
        }
    }
    return cnt;
}

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent
 * of pixels that differ.  Used to lock ordinal 22 is a distinct
 * portrait from its row-2 neighbours (21, 23).  The DM1 champion-
 * portrait atlas carries 24 distinct champions, so a duplicate
 * would be a real regression. */
static int atlas_cell_distinct_percent(const M11_AssetSlot* portraits,
                                       int ordinalA, int ordinalB) {
    int x, y, compared = 0, different = 0;
    int srcAX, srcAY, srcBX, srcBY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcAX = (ordinalA & 7) * PORTRAIT_W;
    srcAY = (ordinalA >> 3) * PORTRAIT_H;
    srcBX = (ordinalB & 7) * PORTRAIT_W;
    srcBY = (ordinalB >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char a = (unsigned char)(
                portraits->pixels[(srcAY + y) * (int)portraits->width + (srcAX + x)] & 0x0F);
            unsigned char b = (unsigned char)(
                portraits->pixels[(srcBY + y) * (int)portraits->width + (srcBX + x)] & 0x0F);
            ++compared;
            if (a != b) ++different;
        }
    }
    return (compared > 0) ? (different * 100 / compared) : 0;
}

/* Count distinct non-zero palette indices in a viewport rect.
 * Used to prove the right-side approach pose still has rendered
 * content (floor / wall / door frame) so the empty D1C cutout
 * cannot be explained away by "the framebuffer was never
 * painted".  Same helper the ordinal-5 approach_from_right probe
 * uses. */
static int rect_distinct_nonzero(const unsigned char* fb,
                                 int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int n = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx != 0 && !seen[idx]) { seen[idx] = 1; ++n; }
        }
    }
    return n;
}

/* Drive M11_GameView_Draw at the given pose and render the result
 * into the supplied framebuffer.  The candidate / inventory state
 * is reset so the no-floating contract is clean.  Same helper
 * the ordinal-5 approach_from_right probe uses. */
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
    memset(fb, 0, (size_t)FB_W * (size_t)FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int ord;
    int pct, distinct, nameRc, titleRc;
    char nameBuf[32];
    char titleBuf[32];
    int ordinal22Opaque, ordinal22Vs21, ordinal22Vs23;
    unsigned char fbNorthOfWallWest[FB_W * FB_H];
    unsigned char fbSouthOfWallWest[FB_W * FB_H];
    unsigned char fbOnCellSouth[FB_W * FB_H];
    unsigned char fbOnCellEast[FB_W * FB_H];
    unsigned char fbPositiveWest[FB_W * FB_H];
    /* Right-side approach band.  Each entry is a pose that fails
     * to expose the ordinal-22 sensor on (2, 6) EAST wall, either
     * because the front cell is wrong or because the visible wall
     * side is wrong.  The engine must return -1 and the D1C cutout
     * must not contain ordinal-22 pixels.  Pattern mirrors the
     * ordinal-5 approach_from_right probe's four-pose band.
     *
     * Note: (3, 6) DIR_NORTH is intentionally NOT in this band --
     * that pose fires ordinal 11 because the (3, 5) front cell has
     * its own C127 sensor (sensorData=11).  It is not a wrong-side
     * pose for ordinal 22; it is a different champion's mirror
     * route.  The (3, 6) DIR_SOUTH pose is the closest analog that
     * also tests an in-place rotation but does not trigger a
     * different mirror ordinal in DM1 V1. */
    struct {
        int mapX;
        int mapY;
        int dir;
        const char* label;
    } rightApproach[] = {
        /* (3, 5) DIR_WEST: front cell (2, 5), wrong cell (correct
         * sensor cell is (2, 6)).  Same facing as canonical (W),
         * but one row north. */
        {3, 5, 3 /* DIR_WEST  */,  "approach_from_right_3_5_W"},
        /* (3, 7) DIR_WEST: front cell (2, 7), wrong cell (correct
         * sensor cell is (2, 6)).  Same facing as canonical (W),
         * but one row south. */
        {3, 7, 3 /* DIR_WEST  */,  "approach_from_right_3_7_W"},
        /* (3, 6) DIR_SOUTH: front cell (3, 7), wrong cell and
         * wrong wall side.  Player turns to face south from the
         * canonical cell. */
        {3, 6, 2 /* DIR_SOUTH */,  "approach_from_right_3_6_S"},
        /* (3, 6) DIR_EAST: front cell (4, 6), wrong cell AND
         * wrong wall side.  Player turns to face east from the
         * canonical cell. */
        {3, 6, 1 /* DIR_EAST  */,  "approach_from_right_3_6_E"},
    };
    int i;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-22 / approach_from_right / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
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
    state.inventoryPanelActive = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas "
                "(got %p %ux%u, need >= %dx%d)\n",
                (const void*)portraits,
                portraits ? portraits->width : 0,
                portraits ? portraits->height : 0,
                ATLAS_W, ATLAS_H);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group A - C026 atlas math for ordinal 22
     * ----------------------------------------------------------------
     * Lock the (col=6, row=2) source rect (192, 58, 32, 29), the
     * opaque-pixel floor, the row-2 neighbour distinctness, and
     * the catalog resolution to GOTHMOG (untitled).  Identical
     * to the Group A checks in the ordinal-22 front_north_entry
     * probe so the right-side approach slice inherits the same
     * atlas self-consistency contract. */
    printf("\n[Group A] C026 atlas math for ordinal 22 (row 2 / col 6 of 8x3 atlas)\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == 256, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == 87, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 col = 22 & 7 = %d (expected 6)",
                 ORDINAL_22_COL);
        CHECK(ORDINAL_22_COL == 6, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 row = 22 >> 3 = %d (expected 2)",
                 ORDINAL_22_ROW);
        CHECK(ORDINAL_22_ROW == 2, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 srcX = %d, srcY = %d (within 256x87 atlas)",
                 ORDINAL_22_SRC_X, ORDINAL_22_SRC_Y);
        CHECK(ORDINAL_22_SRC_X + PORTRAIT_W <= ATLAS_W &&
              ORDINAL_22_SRC_Y + PORTRAIT_H <= ATLAS_H, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 source cell bottom exactly reaches atlas height: "
                 "srcY(%d) + portraitH(%d) == atlasH(%u) -- row 2 is the last "
                 "row of the 8x3 atlas",
                 ORDINAL_22_SRC_Y, PORTRAIT_H, portraits->height);
        CHECK(ORDINAL_22_SRC_Y + PORTRAIT_H == (int)portraits->height, msg);
    }

    ordinal22Opaque = atlas_cell_opaque_count(portraits, 22);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 atlas cell opaque count = %d "
                 "(>= 100, defined portrait)",
                 ordinal22Opaque);
        CHECK(ordinal22Opaque >= 100, msg);
    }
    ordinal22Vs21 = atlas_cell_distinct_percent(portraits, 22, 21);
    ordinal22Vs23 = atlas_cell_distinct_percent(portraits, 22, 23);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 vs ordinal 21 (left row-2 neighbour) differ "
                 "by %d%% (>= 30%%)",
                 ordinal22Vs21);
        CHECK(ordinal22Vs21 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 vs ordinal 23 (right row-2 neighbour) differ "
                 "by %d%% (>= 30%%)",
                 ordinal22Vs23);
        CHECK(ordinal22Vs23 >= 30, msg);
    }

    nameBuf[0] = '\0';
    titleBuf[0] = '\0';
    nameRc = M11_GameView_GetMirrorNameByOrdinal(&state, TARGET_ORDINAL,
                                                 nameBuf, (int)sizeof(nameBuf));
    titleRc = M11_GameView_GetMirrorTitleByOrdinal(&state, TARGET_ORDINAL,
                                                   titleBuf, (int)sizeof(titleBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog ordinal 22 = \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameRc > 0 && strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog ordinal 22 title = \"%s\" (expected empty "
                 "-- GOTHMOG is untitled in the DM1 V1 catalog)",
                 titleBuf[0] ? titleBuf : "");
        CHECK(titleRc == 0 || titleBuf[0] == '\0', msg);
    }

    /* ----------------------------------------------------------------
     * Group B - Confirm the natural canonical (3, 6, W) route
     * ----------------------------------------------------------------
     * The shipped PC 3.4 English DUNGEON.DAT places the ordinal-22
     * C127 sensor on the EAST wall of (2, 6), visible from (3, 6)
     * DIR_WEST.  Confirm the front-mirror ordinal at the canonical
     * pose equals ordinal 22 before measuring the right-side
     * approach band.  This proves the natural (3, 6, W) route
     * exposes ordinal 22 -- the route the right-side approach band
     * is being measured against. */
    printf("\n[Group B] Confirm canonical (3, 6, W) natural route exposes ordinal 22\n");
    {
        state.world.party.mapIndex = 0;
        state.world.party.mapX = CANONICAL_MAP_X;
        state.world.party.mapY = CANONICAL_MAP_Y;
        state.world.party.direction = CANONICAL_DIR;
        ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3, 6) DIR_WEST front mirror ordinal = %d (expected %d)",
                     ord, TARGET_ORDINAL);
            CHECK(ord == TARGET_ORDINAL, msg);
        }
        if (ord != TARGET_ORDINAL) {
            fprintf(stderr,
                    "FATAL: front ordinal did not lock to %d at the natural "
                    "route; cannot verify approach_from_right\n", TARGET_ORDINAL);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone.  Same helper the ordinal-5 approach_from
     * _right probe uses. */
    {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)",
                 rc);
        CHECK(rc == 1, msg);
        snprintf(msg, sizeof(msg),
                 "D1C wall box = (%d, %d, %d, %d) (expected 80, 29, 64, 43)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) sits inside the D1C wall box");
        CHECK(96 >= ornX && 96 + PORTRAIT_W <= ornX + ornW &&
              35 >= ornY && 35 + PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - approach_from_right negative-route pixel contract
     * ----------------------------------------------------------------
     * For each right-side approach pose, M11_GameView_GetFrontMirror
     * Ordinal must return -1 (either the front cell is wrong or the
     * visible wall side does not match the sensor's M011_CELL=1
     * EAST), and the D1C portrait cutout (96, 35, 32, 29) must not
     * carry ordinal-22 pixels.  A regression that paints the
     * ordinal-22 portrait on a side wall would push the C026 pixel
     * match above the 35% drift threshold. */
    printf("\n[Group C] approach_from_right negative-route pixel contract\n");
    for (i = 0; i < (int)(sizeof(rightApproach) / sizeof(rightApproach[0])); ++i) {
        unsigned char* fb;
        switch (i) {
            case 0: fb = fbNorthOfWallWest;  break;
            case 1: fb = fbSouthOfWallWest;  break;
            case 2: fb = fbOnCellSouth;      break;
            default: fb = fbOnCellEast;      break;
        }
        render_at(&state, fb,
                  rightApproach[i].mapX, rightApproach[i].mapY,
                  rightApproach[i].dir);
        ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s: front mirror ordinal = %d (expected -1)",
                     rightApproach[i].label, ord);
            CHECK(ord == -1, msg);
        }
        /* The corridor / wall / floor must still have rendered
         * content at the right-side approach pose, so an empty D1C
         * cutout cannot be explained away by "the framebuffer was
         * never painted". */
        distinct = rect_distinct_nonzero(fb,
                                         VIEWPORT_X + 0,
                                         VIEWPORT_Y + 30,
                                         96, 60);
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "%s: left half of viewport has >= %d distinct non-zero "
                     "palette indices (got %d) - framebuffer is alive, empty "
                     "portrait cutout is meaningful",
                     rightApproach[i].label, RECT_ALIVE_DISTINCT, distinct);
            CHECK(distinct >= RECT_ALIVE_DISTINCT, msg);
        }
        /* The D1C portrait cutout must NOT carry ordinal-22 pixels. */
        pct = match_portrait_cell(portraits, fb, TARGET_ORDINAL);
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "%s: D1C cutout ordinal-22 pixel match = %d%% "
                     "(expected < %d%%, no floating portrait on right side)",
                     rightApproach[i].label, pct, WRONG_ORDINAL_MATCH_PCT);
            CHECK(pct >= 0 && pct < WRONG_ORDINAL_MATCH_PCT, msg);
        }
        printf("  INFO: %s map=(%d,%d) dir=%d ord=%d match=%d%% distinct=%d\n",
               rightApproach[i].label,
               rightApproach[i].mapX, rightApproach[i].mapY, rightApproach[i].dir,
               ord, pct, distinct);
    }

    /* ----------------------------------------------------------------
     * Group D - +1 (3, 6, W) positive-route cross-check
     * ----------------------------------------------------------------
     * Render the canonical (3, 6, W) pose where the C127 sensor's
     * M011_CELL=1 (EAST) matches the visible wall side.  The D1C
     * cutout must carry ordinal-22 pixels at >= 90% match.  This
     * is the cross-check that proves the right-side approach band
     * is being measured against the same sensor and that the empty
     * right-side rectangle is not silently dead. */
    printf("\n[Group D] +1 (3, 6, W) positive-route cross-check\n");
    render_at(&state, fbPositiveWest,
              CANONICAL_MAP_X, CANONICAL_MAP_Y, CANONICAL_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(3, 6) DIR_WEST front mirror ordinal = %d (expected %d)",
                 ord, TARGET_ORDINAL);
        CHECK(ord == TARGET_ORDINAL, msg);
    }
    pct = match_portrait_cell(portraits, fbPositiveWest, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(3, 6) DIR_WEST D1C cutout ordinal-22 match = %d%% "
                 "(expected >= %d%%, positive route paints ordinal 22)",
                 pct, POSITIVE_ORDINAL_MATCH_PCT);
        CHECK(pct >= POSITIVE_ORDINAL_MATCH_PCT, msg);
    }
    distinct = rect_distinct_nonzero(fbPositiveWest,
                                     VIEWPORT_X + 0,
                                     VIEWPORT_Y + 30,
                                     96, 60);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(3, 6) DIR_WEST left half of viewport has >= %d distinct "
                 "non-zero palette indices (got %d)",
                 RECT_ALIVE_DISTINCT, distinct);
        CHECK(distinct >= RECT_ALIVE_DISTINCT, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - ordinal 22 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 22 must be self-consistent
     * and the catalog/atlas pair must agree on the (col=6, row=2)
     * source rect.  Same shape as the ordinal-22 front_north_entry
     * probe's Group D so the right-side approach slice inherits the
     * same round-trip contract. */
    printf("\n[Group E] ordinal 22 atlas round-trip\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 22 is at source "
                 "(%d, %d, %d, %d) (matches ((22&7)<<5, (22>>3)*29, 32, 29))",
                 ORDINAL_22_SRC_X, ORDINAL_22_SRC_Y,
                 PORTRAIT_W, PORTRAIT_H);
        CHECK(ORDINAL_22_SRC_X == 192 && ORDINAL_22_SRC_Y == 58, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 opaque count = %d (in expected 200..900 range "
                 "for a defined champion)",
                 ordinal22Opaque);
        CHECK(ordinal22Opaque >= 200 && ordinal22Opaque <= 900, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 distinctness: vs 21 = %d%%, vs 23 = %d%% "
                 "(both >= 30%%)",
                 ordinal22Vs21, ordinal22Vs23);
        CHECK(ordinal22Vs21 >= 30 && ordinal22Vs23 >= 30, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
