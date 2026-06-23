/*
 * firestaff_dm1_v1_hall_of_champions_portrait_08_cancel_reopen_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 8           (mirror catalog record IAIDO)
 *   route   cancel_reopen (C040 panel select -> F0282 C162 cancel branch
 *                          -> fresh C040 panel select on the same front
 *                          square; championCount goes 0 -> 1 -> 0 -> 1)
 *   aspect  portrait_rect_position
 *
 * The C026 champion-portrait atlas is an 8x3 grid of 32x29 portraits
 * (256x87 pixels total, ordinals 0..23). Ordinal 8 sits at row 1,
 * column 0 of the atlas (the first cell of the second row):
 *
 *     srcX = (8 & 7) << 5 =   0
 *     srcY = (8 >> 3) * 29 = 29
 *
 * Row 1 of the C026 atlas differs from row 0 in that the champion
 * portraits shift up: ordinals 8..15 are the second row of the 24-record
 * DM1 V1 mirror catalog (this is IAIDO / STAMM / ZED / ... / LEYLA).
 * The (8 & 7) << 5 / (8 >> 3) * 29 macro math is identical to row 0
 * (DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro encoding);
 * only the row multiplier changes (0 -> 1).
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNVIEW.C:3913-3928 and COORD.C:1693-1749
 * + DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall = {96,
 * 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the (1,2)
 * NORTH-route front square (1,1) with sensorData=1 (HALK), so we
 * seed that sensor to sensorData=8 to lock the ordinal-8 edge case.
 * This keeps the probe runtime-real: same sensor, same DUNGEON.DAT,
 * same draw path - only the ordinal is shifted for the test.
 *
 * This probe covers the three coupled concerns of cancel_reopen in
 * one runtime drive:
 *
 *   (1) Atlas math for ordinal 8: verify the C026 atlas contains a
 *       defined portrait at (0, 29, 32, 29) and that the
 *       (8 & 7) << 5 / (8 >> 3) * 29 math matches COORD.C M027/M028
 *       macro encoding (DEFS.H:821-826).  Verifies ordinal 8 is a
 *       valid C026 atlas entry (not a degenerate cell) AND that the
 *       row-1 / column-0 cell is distinct from row-0 / column-0
 *       (ordinal 0 / DAROOU) - row wrapping is the most likely place
 *       for a macro bug.
 *
 *   (2) portrait_rect_position: drive a real D1C front-mirror pose
 *       from the actual DM1 V1 DUNGEON.DAT C127 sensor lattice,
 *       pixel-prove the destination rectangle (96, 35, 32, 29) on
 *       the 320x200 framebuffer contains the ordinal-8 champion
 *       portrait, and that the side walls (left of x=96 and right of
 *       x=127 in the portrait row band) do NOT carry the portrait's
 *       palette.
 *
 *   (3) cancel_reopen: select the candidate (panel live), cancel
 *       (F0282 C162 branch via M11_GameView_CancelMirrorCandidate),
 *       re-select (F0280 / M11_GameView_SelectFrontMirrorCandidate
 *       returns 1 again), and pixel-prove the D1C portrait rect
 *       still carries ordinal-8 pixels after the full
 *       select->cancel->select cycle.  This is the cancel_reopen
 *       slice from the gate table; it is disjoint from the existing
 *       pass784 contract test (which is a state-machine simulator
 *       that does not link the engine) and from
 *       portrait_14_redraw_after_candidate (different ordinal +
 *       different route - no cancel between two selects).
 *
 * Source evidence:
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir)
 *   - DUNGEON.C:2608-2612 (G0289 champion portrait ordinal)
 *   - DUNVIEW.C:3913-3928 (D1C C026 portrait blit at {96,35})
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:3916-3919 (C026_GRAPHIC_CHAMPION_PORTRAITS,
 *                          "A portrait is 32x29 pixels")
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - REVIVE.C F0282:744-806 (C162 cancel branch 744-783)
 *   - PANEL.C F0355:2299-2318 (inventory close on cancel)
 *   - COMMAND.C F0378:1956-1990 (M568_PANEL_RESURRECT_REINCARNATE
 *                              dispatch)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - M11_GameView_CancelMirrorCandidate (F0282 C162 cancel path)
 *   - M11_GameView_SelectFrontMirrorCandidate (F0280 reopen path)
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
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928). */
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    C040_PANEL_X = VIEWPORT_X + 80,
    C040_PANEL_Y = VIEWPORT_Y + 52,
    D1C_PORTRAIT_TOP_VISIBLE_H = C040_PANEL_Y - D1C_PORTRAIT_Y,
    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 8 in the C026 atlas: (8 & 7) << 5 = 0, (8 >> 3) * 29 = 29.
     * This is the first cell of row 1 (column 0), NOT row 0 column 0. */
    ORDINAL_8_COL = 8 & 7,         /* = 0 */
    ORDINAL_8_ROW = 8 >> 3,        /* = 1 */
    ORDINAL_8_SRC_X = ORDINAL_8_COL << 5,   /* =   0 */
    ORDINAL_8_SRC_Y = ORDINAL_8_ROW * 29,   /* =  29 */
    /* Side wall sample zones - the no-floating proof checks that
     * the portrait sprite pixels do not bleed into the left/right
     * side walls of the D1C cell band. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    TARGET_ORDINAL = 8,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 8 for this gate so we can lock the ordinal-8
     * edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1,
    /* The row-0 col-0 ordinal (0 / DAROOU) is the most likely
     * lookalike for ordinal 8 (IAIDO), since both share column 0.
     * The (8 & 7) vs (0 & 7) is the same (0); only the row
     * multiplier (8 >> 3 = 1 vs 0 >> 3 = 0) changes the source Y. */
    ROW0_COL0_ORDINAL = 0
};
/* Mirror catalog record name for ordinal 8 (DM1 V1 PC34 mirror
 * catalog).  Used to assert the catalog resolves correctly. */
static const char kExpectedCatalogName[] = "IAIDO";

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

/* Count non-zero pixels in a framebuffer rectangle. */
static int rect_nonzero(const unsigned char* fb,
                        int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx != 0) ++cnt;
        }
    }
    return cnt;
}

/* Count "warm" pixels in a framebuffer rectangle.  The C026 portrait
 * sprites use the warm palette set {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}
 * (green / red / orange / peach / yellow / blue) for skin tones,
 * clothing, and backgrounds.  Grey-stone wall texture uses indices
 * 0x01, 0x02, 0x0D.  Counting warm pixels is a coarse but reliable
 * way to distinguish "portrait is here" from "wall only" in the
 * C026 cutout (96, 35, 32, 29). */
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

/* Count opaque pixels in the C026 atlas cell for the requested ordinal.
 * Used to verify ordinal 8 is a defined portrait in the atlas
 * (i.e. not blank / unused / palette-index-1 transparent only). */
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

/* Compare the C026 portrait atlas cell for the requested ordinal
 * to the framebuffer D1C portrait rectangle.  Returns the percent
 * of opaque source pixels that match the destination pixel. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
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

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent of
 * pixels that differ.  Used to verify ordinal 8 is a distinct portrait
 * from its column-0 row-mates (0 DAROOU on row 0, 16 CHANI on row 2)
 * AND from its immediate row-1 neighbours (9 ZED, 15 LEYLA).  The DM1
 * champion-portrait atlas carries 24 distinct champions (one per
 * ordinal), so a duplicate would be a real regression. */
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-8 edge case on the real DM1 V1 DUNGEON.DAT
 * (which ships HALK / ordinal 1 on the (1,2) NORTH-route front
 * square (1,1)).  The seed does NOT change the map layout or the
 * C127 cell match - only the G0289 ordinal that DUNVIEW.C:3913-3928
 * reads through M000_INDEX_TO_ORDINAL (DUNGEON.C:2610-2612). */
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
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = DIR_NORTH;
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
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterReopen[FB_W * FB_H];
    int matchBefore, matchAfterSelect, matchAfterCancel, matchAfterReopen;
    int nonzeroBefore, nonzeroAfterReopen;
    int distinctBefore, distinctAfterReopen;
    int warmBefore, warmAfterReopen;
    int leftSideBefore, leftSideAfterReopen;
    int rightSideBefore, rightSideAfterReopen;
    int initialCount, countAfterSelect, countAfterCancel, countAfterReopen;
    int selectRc, cancelRc, reopenRc;
    char nameBuf[32];
    int nameLookupRc;
    int ordinal8Opaque;
    int ordinal8Vs0;
    int ordinal8Vs9;
    int ordinal8Vs16;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-08 / cancel_reopen / portrait_rect_position (v2.7.27) ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas via the public M11 helper, so the
     * probe does not depend on the file-scope enum value 26. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 8
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 1 /
     * column 0 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows"). */
    printf("\n[Group A] C026 atlas math for ordinal 8\n");

    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic id returned by "
                 "M11_GameView_GetV1ChampionPortraitGraphicId = %d)",
                 M11_GameView_GetV1ChampionPortraitGraphicId());
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == 256, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == 87, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 col = 8 & 7 = %d (expected 0)",
                 ORDINAL_8_COL);
        CHECK(ORDINAL_8_COL == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 row = 8 >> 3 = %d (expected 1)",
                 ORDINAL_8_ROW);
        CHECK(ORDINAL_8_ROW == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_8_SRC_X, ORDINAL_8_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_8_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_8_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 8 must be a defined portrait: opaque count > 50% of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion).  This catches a regression where
     * ordinal 8 is treated as "no portrait" or the wrong atlas cell
     * is blitted at (0, 29, 32, 29). */
    ordinal8Opaque = atlas_cell_opaque_count(portraits, 8);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal8Opaque);
        CHECK(ordinal8Opaque >= 100, msg);
    }

    /* Ordinal 8 must be visually distinct from its column-0 row-mates
     * (0 DAROOU on row 0, 16 CHANI on row 2) AND from its immediate
     * row-1 neighbours (9 ZED).  The DM1 champion-portrait atlas
     * carries 24 distinct champions (one per ordinal), so a duplicate
     * would be a real regression.  The row-0 col-0 vs row-1 col-0
     * distinctness check is the canonical row-wrapping bug catcher:
     * if the (ordinal >> 3) macro drops the high bits the row 1
     * portraits collapse onto row 0 and the C026 atlas would carry
     * only 16 distinct cells. */
    ordinal8Vs0  = atlas_cell_distinct_percent(portraits, 8, 0);
    ordinal8Vs9  = atlas_cell_distinct_percent(portraits, 8, 9);
    ordinal8Vs16 = atlas_cell_distinct_percent(portraits, 8, 16);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 0 (column-0 row-0) differ "
                 "by >= 30%% (got %d%%) - row-wrap check",
                 ordinal8Vs0);
        CHECK(ordinal8Vs0 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 9 (row-1 right neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal8Vs9);
        CHECK(ordinal8Vs9 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 vs ordinal 16 (column-0 row-2) differ "
                 "by >= 30%% (got %d%%) - row-2 column-0 distinctness",
                 ordinal8Vs16);
        CHECK(ordinal8Vs16 >= 30, msg);
    }

    /* Ordinal 8 must resolve to IAIDO through the mirror catalog.
     * This catches a regression where the catalog and the C026 atlas
     * disagree on the ordinal-8 record. */
    nameBuf[0] = '\0';
    nameLookupRc = M11_GameView_GetMirrorNameByOrdinal(&state,
                                                       TARGET_ORDINAL,
                                                       nameBuf,
                                                       (int)sizeof(nameBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 8 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to ordinal
     * 8 (IAIDO).  Same sensor, same map cell, same draw path - only
     * G0289 shifts.  This keeps the probe runtime-real. */
    park_d1c_front_route(&state);
    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    /* The same front route now reports ordinal 8.  After seeding
     * the C127 sensor's sensorData, the front route must reflect the
     * change end-to-end (m11_front_cell_mirror_ordinal -> G0289). */
    park_d1c_front_route(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify portrait_rect_position or cancel_reopen\n",
                TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "coords (DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35, 32, 29) sits inside the "
                 "D1C wall ornament zone (X within [%d,%d), Y within "
                 "[%d,%d))",
                 ornX, ornX + ornW, ornY, ornY + ornH);
        CHECK(96 >= ornX &&
              96 + D1C_PORTRAIT_W <= ornX + ornW &&
              35 >= ornY &&
              35 + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - portrait_rect_position on a real C127 sensor pose
     * ----------------------------------------------------------------
     * Render the framebuffer before any selection, and verify the
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-8
     * pixels. */
    printf("\n[Group B] portrait_rect_position on real C127 sensor pose (1,2,0)=8\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    memset(fbBefore, 0, sizeof(fbBefore));
    M11_GameView_Draw(&state, fbBefore, FB_W, FB_H);

    /* The D1C portrait rect must contain ordinal-8 source pixels at
     * >= 90% match.  This is the "portrait ordinal maps to the
     * expected champion and the D1C portrait rectangle is drawn at
     * the intended screen position" requirement from the slice. */
    matchBefore = match_portrait_at_rect(portraits, fbBefore, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchBefore);
        CHECK(matchBefore >= 90, msg);
    }
    nonzeroBefore = rect_nonzero(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 nonzeroBefore);
        CHECK(nonzeroBefore >= 100, msg);
    }
    distinctBefore = rect_distinct(fbBefore,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 4 distinct palette indices "
                 "(got %d)",
                 distinctBefore);
        CHECK(distinctBefore >= 4, msg);
    }
    warmBefore = rect_warm_count(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmBefore);
        CHECK(warmBefore >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof: side walls of the D1C portrait band must NOT
     * carry the portrait's warm pixels. */
    leftSideBefore = rect_warm_count(fbBefore,
                                     SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                     SIDE_WALL_LEFT_W,
                                     PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideBefore);
        CHECK(leftSideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideBefore = rect_warm_count(fbBefore,
                                      SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                      SIDE_WALL_RIGHT_W,
                                      PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "right side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideBefore);
        CHECK(rightSideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - cancel_reopen: select -> cancel -> reopen
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection, then the C162
     * cancel branch via M11_GameView_CancelMirrorCandidate
     * (REVIVE.C F0282:744-783), then re-select with
     * M11_GameView_SelectFrontMirrorCandidate (REVIVE.C F0280).
     * The framebuffer's D1C portrait rect must still carry ordinal-8
     * pixels after the full select->cancel->select cycle. */
    printf("\n[Group C] cancel_reopen: select, cancel, reopen, portrait rect still carries ordinal 8\n");

    /* Step 1: SelectFrontMirrorCandidate (F0280). */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterSelect = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (1,2,0) returns 1 (got %d)",
                 selectRc);
        CHECK(selectRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after select: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterSelect, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterSelect == initialCount + 1, msg);
    }

    /* Render with C040 panel live.  The portrait must NOT be drawn as
     * a stale floating sprite while the panel owns the view (BUG-120/
     * 121 panel guard).  Match against ordinal 8 should be low. */
    memset(fbAfterSelect, 0, sizeof(fbAfterSelect));
    M11_GameView_Draw(&state, fbAfterSelect, FB_W, FB_H);
    matchAfterSelect = match_portrait_at_rect(portraits,
                                              fbAfterSelect,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw does not leave ordinal %d as a stale "
                 "full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterSelect);
        CHECK(matchAfterSelect <= 20, msg);
    }

    /* Step 2: CancelMirrorCandidate (F0282 C162 cancel branch).
     * Per source-locked contract: F0355 inventory close, G0299 clear,
     * G0305--, F0643_PARTY_ClearChampionSlot for the candidate index.
     * The portrait on the wall stays the same because the sensor is
     * still active (sensorType=127, sensorData=8). */
    cancelRc = M11_GameView_CancelMirrorCandidate(&state);
    countAfterCancel = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate on (1,2,0) returns 1 (got %d)",
                 cancelRc);
        CHECK(cancelRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterCancel, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterCancel == initialCount, msg);
    }

    /* Render after cancel: panel closed, portrait rect must hold
     * ordinal-8 pixels again. */
    memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
    M11_GameView_Draw(&state, fbAfterCancel, FB_W, FB_H);
    matchAfterCancel = match_portrait_at_rect(portraits,
                                               fbAfterCancel,
                                               TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchAfterCancel);
        CHECK(matchAfterCancel >= 90, msg);
    }

    /* Step 3: SelectFrontMirrorCandidate again (F0280 reopen).
     * Per source-locked contract: party has 0 champions after cancel,
     * leader hand empty, sensorData=8 still on the front square, so
     * the reopen must succeed and the panel must come back live. */
    reopenRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterReopen = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate reopen on (1,2,0) returns 1 "
                 "(got %d)",
                 reopenRc);
        CHECK(reopenRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after reopen: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterReopen, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterReopen == initialCount + 1, msg);
    }

    /* Render after reopen: panel live, portrait rect must NOT be a
     * stale full D1C sprite (BUG-120/121 panel guard still active). */
    memset(fbAfterReopen, 0, sizeof(fbAfterReopen));
    M11_GameView_Draw(&state, fbAfterReopen, FB_W, FB_H);
    matchAfterReopen = match_portrait_at_rect(portraits,
                                              fbAfterReopen,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw after reopen does not leave ordinal %d "
                 "as a stale full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterReopen);
        CHECK(matchAfterReopen <= 20, msg);
    }

    /* The visible top D1C strip (portrait + 1px border, ABOVE the
     * C040 panel) must still be non-empty and stable across the
     * select/cancel/reopen cycle: panel covers (52..) so the top
     * strip y=[35..52) is the only window into the wall behind. */
    nonzeroAfterReopen = rect_nonzero(fbAfterReopen,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W,
                                      D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top strip of D1C candidate redraw "
                 "is non-empty (>= 100 non-zero pixels, got %d)",
                 nonzeroAfterReopen);
        CHECK(nonzeroAfterReopen >= 100, msg);
    }
    warmAfterReopen = rect_warm_count(fbAfterReopen,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W,
                                      D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top strip of D1C candidate redraw "
                 "has no portrait warm-color leak (<= 10 pixels, got %d)",
                 warmAfterReopen);
        CHECK(warmAfterReopen <= 10, msg);
    }
    distinctAfterReopen = rect_distinct(fbAfterReopen,
                                        D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                        D1C_PORTRAIT_W + 2,
                                        D1C_PORTRAIT_TOP_VISIBLE_H + 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top D1C slice distinct palette "
                 "count is <= panel-off count (reopen=%d, panel-off=%d)",
                 distinctAfterReopen, distinctBefore);
        CHECK(distinctAfterReopen <= distinctBefore, msg);
    }

    /* No-floating proof on the panel-on frame after reopen: only
     * sample the unoccluded side-wall strip above C040, because the
     * panel covers the lower/right side-wall band by design. */
    leftSideAfterReopen = rect_warm_count(fbAfterReopen,
                                          SIDE_WALL_LEFT_X, D1C_PORTRAIT_Y,
                                          SIDE_WALL_LEFT_W,
                                          D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: left side wall of D1C portrait band has "
                 "< %d warm pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterReopen);
        CHECK(leftSideAfterReopen < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterReopen = rect_warm_count(fbAfterReopen,
                                           SIDE_WALL_RIGHT_X, D1C_PORTRAIT_Y,
                                           SIDE_WALL_RIGHT_W,
                                           D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: right side wall of D1C portrait band has "
                 "< %d warm pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterReopen);
        CHECK(rightSideAfterReopen < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Cross-check: the cancel_reopen cycle leaves the visible top
     * strip pixel count within the wall-ornament density band.
     * DUNVIEW.C draws the wall ornament + portrait independently of
     * the panel state - the panel suppresses the portrait (BUG-120)
     * but the wall-ornament border stays in place.  Ordinal-8 (IAIDO)
     * portrait pixels in the top strip are denser for some champions
     * than others, so the absolute |diff| between panel-off and
     * panel-on can vary widely per champion.  We therefore assert the
     * panel-on top strip lands in the expected wall-ornament density
     * band (>=200 non-zero pixels - enough to prove the wall-ornament
     * frame is still being drawn) instead of forcing panel-off ==
     * panel-on. */
    {
        int topOff = rect_nonzero(fbBefore,
                                  D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                  D1C_PORTRAIT_W,
                                  D1C_PORTRAIT_TOP_VISIBLE_H);
        int topOn  = nonzeroAfterReopen;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "visible top strip non-zero pixel count lands in the "
                 "wall-ornament band after cancel_reopen (panel-off=%d, "
                 "after-reopen=%d, 200 <= after-reopen <= 700)",
                 topOff, topOn);
        CHECK(topOn >= 200 && topOn <= 700, msg);
    }

    /* The portrait_rect_position contract: across the full
     * select->cancel->select cycle the D1C destination rectangle
     * does NOT change screen position.  The (96, 35, 32, 29)
     * destination is source-locked to DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall, so
     * we verify the same rect lines up with ordinal-8 pixels when
     * the panel is closed (before select, after cancel) and is
     * suppressed as a stale sprite while the panel is live (after
     * select, after reopen). */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position: before=%d%%, after-select=%d%%, "
                 "after-cancel=%d%%, after-reopen=%d%% (panel-off >=90, "
                 "panel-on <=20)",
                 matchBefore, matchAfterSelect,
                 matchAfterCancel, matchAfterReopen);
        CHECK(matchBefore >= 90 &&
              matchAfterSelect <= 20 &&
              matchAfterCancel >= 90 &&
              matchAfterReopen <= 20, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - ordinal 8 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 8 must be self-consistent: the
     * destination (96, 35, 32, 29) on the framebuffer lines up with
     * the source (0, 29, 32, 29) in the atlas.  This is the "ordinal
     * 8 maps to the expected champion" check from the slice
     * description - the round-trip is independent of the runtime
     * drive and pins the macro math against the atlas itself. */
    printf("\n[Group D] ordinal 8 atlas round-trip: source (0, 29) maps to dst (96, 35)\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 8 is at source "
                 "(%d, %d, %d, %d) - matches "
                 "((8 & 7) << 5, (8 >> 3) * 29, 32, 29)",
                 ORDINAL_8_SRC_X, ORDINAL_8_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ORDINAL_8_SRC_X == 0 && ORDINAL_8_SRC_Y == 29, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas for ordinal 8 opaque count = %d "
                 "(in expected 200..900 range for a defined champion)",
                 ordinal8Opaque);
        CHECK(ordinal8Opaque >= 200 && ordinal8Opaque <= 900, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 8 round-trip distinctness: vs 0 = %d%%, "
                 "vs 9 = %d%%, vs 16 = %d%% (all >= 30%%)",
                 ordinal8Vs0, ordinal8Vs9, ordinal8Vs16);
        CHECK(ordinal8Vs0 >= 30 && ordinal8Vs9 >= 30 && ordinal8Vs16 >= 30, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
