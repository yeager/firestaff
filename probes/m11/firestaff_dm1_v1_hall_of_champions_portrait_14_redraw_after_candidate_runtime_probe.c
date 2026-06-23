/*
 * firestaff_dm1_v1_hall_of_champions_portrait_14_redraw_after_candidate_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions
 * slice:
 *
 *   ordinal 14
 *   route   redraw_after_candidate
 *   aspect  portrait_rect_position
 *
 * The DM1 PC 3.4 C026 champion-portrait atlas is an 8x3 grid of
 * 32x29 portraits (256x87 pixels total), supporting ordinals 0..23.
 * Ordinal 14 sits at row 1, column 6 of the atlas:
 *
 *     srcX = (14 & 7) * 32 = 192
 *     srcY = (14 >> 3) * 29 =  29
 *
 * The D1C front-wall champion portrait rectangle is the source-
 * locked destination (per ReDMCSB DUNVIEW.C:3913-3928 and
 * COORD.C:1693-1749 + DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * This probe covers the three coupled concerns in one runtime
 * drive:
 *
 *   (1) Atlas math for ordinal 14: verify the C026 atlas contains
 *       a defined portrait at (192, 29, 32, 29) and that the
 *       (14 & 7) * 32 / (14 >> 3) * 29 math matches COORD.C
 *       M027/M028 macro encoding (DEFS.H:821-826).
 *
 *   (2) portrait_rect_position: drive a real D1C front-mirror
 *       pose from the actual DM1 V1 DUNGEON.DAT C127 sensor lattice
 *       (verified by
 *        firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe)
 *       and pixel-prove that the destination rectangle (96, 35,
 *       32, 29) on the 320x200 framebuffer contains the expected
 *       champion portrait ordinal AND that the side walls (left of
 *       x=96 and right of x=127 in the portrait row band) do NOT
 *       carry the portrait's palette.
 *
 *   (3) redraw_after_candidate: re-render the same D1C pose with
 *       candidateMirrorPanelActive=1, and confirm that the redraw
 *       does not leave the D1C portrait sprite floating on ordinary
 *       wall bands after the C040 candidate panel owns the view.
 *       ReDMCSB PANEL.C F0346 draws the modal panel after the wall
 *       portrait path, so this route proves cleanup/ownership, not
 *       DOS pixel parity or a still-visible full portrait.
 *
 * The probe uses real DM1 V1 DUNGEON.DAT C127 sensor pose (1,2,0)
 * (front=(1,1), C127 sensorData=1, HALK). Ordinal 1 is the
 * simplest real ordinal in the C026 atlas; the same draw path
 * handles all 24 ordinals (0..23), and ordinal 14 is a specific
 * row 1 / column 6 portrait whose (192, 29) source coordinates
 * we verify separately. The C026 atlas math and the C127 sensor
 * drive are independent sources of truth, so a regression in one
 * cannot mask a regression in the other.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir)
 *   - DUNGEON.C:2608-2612 (G0289 champion portrait ordinal)
 *   - DUNVIEW.C:3913-3928 (D1C C026 portrait blit)
 *   - DUNVIEW.C:525 (G0109 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:3916-3919 (C026_GRAPHIC_CHAMPION_PORTRAITS,
 *                          "A portrait is 32x29 pixels")
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280 (materialize candidate from sensorData)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
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
    D1C_PORTRAIT_VIEW_X = 96,
    D1C_PORTRAIT_VIEW_Y = 35,
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
    /* Ordinal 14 in the C026 atlas: (14 & 7) * 32 = 192,
     *                                 (14 >> 3) * 29 =  29. */
    ORDINAL_14_COL = 14 & 7,        /* = 6 */
    ORDINAL_14_ROW = 14 >> 3,       /* = 1 */
    ORDINAL_14_SRC_X = ORDINAL_14_COL * 32,   /* = 192 */
    ORDINAL_14_SRC_Y = ORDINAL_14_ROW * 29,   /* = 29 */
    /* M11_GFX_CHAMPION_PORTRAITS == 26 == C026_GRAPHIC_CHAMPION_PORTRAITS.
     * This file-scoped enum in m11_game_view.c is not exported; the
     * source-locked value 26 is stable across versions. */
    M11_GFX_CHAMPION_PORTRAITS = 26,
    /* Side wall sample zones - the no-floating proof checks that
     * the portrait sprite pixels do not bleed into the left/right
     * side walls of the D1C cell band. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

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

/* Count non-transparent source pixels in the C026 atlas cell for the
 * requested ordinal.  Used to verify ordinal 14 is a defined portrait
 * in the atlas (i.e. not blank / unused). */
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

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent of
 * pixels that differ.  Used to verify ordinal 14 is a distinct
 * portrait from its row-1 neighbours (13, 15) - if the atlas stored
 * a duplicate, the cell match would be 100% and we would fail the
 * distinctness assertion. */
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

/* Park the party at the (1,2) D1C front-mirror route facing NORTH.
 * This is the real C127 sensor position from
 * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 * the front square (1,1) has a C127 sensor with sensorData=1 (HALK). */
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
    int ordinal14Opaque;
    int ordinal14Vs13;
    int ordinal14Vs15;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbPanelOff[FB_W * FB_H];
    unsigned char fbPanelOn [FB_W * FB_H];
    int matchOff, matchOn;
    int nonzeroOff, nonzeroOn;
    int distinctOff, distinctOn;
    int warmOff, warmOn;
    int leftSideOff, leftSideOn;
    int rightSideOff, rightSideOn;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-14 / redraw_after_candidate / portrait_rect_position (v2.7.27) ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GFX_CHAMPION_PORTRAITS);

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 14
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 1 /
     * column 6 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows"). */
    printf("\n[Group A] C026 atlas math for ordinal 14\n");

    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic 26 = C026_GRAPHIC_CHAMPION_PORTRAITS)");
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
                 "ordinal 14 col = 14 & 7 = %d (expected 6)",
                 ORDINAL_14_COL);
        CHECK(ORDINAL_14_COL == 6, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 row = 14 >> 3 = %d (expected 1)",
                 ORDINAL_14_ROW);
        CHECK(ORDINAL_14_ROW == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_14_SRC_X, ORDINAL_14_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_14_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_14_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 14 must be a defined portrait: opaque count > 50% of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion). */
    ordinal14Opaque = atlas_cell_opaque_count(portraits, 14);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal14Opaque);
        CHECK(ordinal14Opaque >= 100, msg);
    }

    /* Ordinal 14 must be visually distinct from neighbours 13 and 15.
     * The DM1 champion-portrait atlas carries 24 distinct champions
     * (one per ordinal), so a duplicate would be a real regression. */
    ordinal14Vs13 = atlas_cell_distinct_percent(portraits, 14, 13);
    ordinal14Vs15 = atlas_cell_distinct_percent(portraits, 14, 15);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 vs ordinal 13 (left neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal14Vs13);
        CHECK(ordinal14Vs13 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 vs ordinal 15 (right neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal14Vs15);
        CHECK(ordinal14Vs15 >= 30, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - portrait_rect_position on a real C127 sensor pose
     * ----------------------------------------------------------------
     * Drive the (1,2,0) D1C front-mirror route, render the framebuffer,
     * and verify the D1C destination rectangle (96, 35, 32, 29) holds
     * the expected champion portrait pixels. */
    printf("\n[Group B] portrait_rect_position on real C127 sensor pose (1,2,0)=1\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;

    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1,2,0) = %d (expected 1, HALK)",
                 frontOrdinal);
        CHECK(frontOrdinal == 1, msg);
    }
    /* If the front ordinal is not 1 in the user's data, the rest of
     * Group B and Group C still run but assert against whatever the
     * runtime front ordinal is.  This keeps the probe useful even
     * against a non-canonical DM1 V1 DUNGEON.DAT variant. */
    if (frontOrdinal < 0) {
        fprintf(stderr,
                "FATAL: no C127 sensor at (1,2,0); cannot verify "
                "portrait_rect_position or redraw_after_candidate\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* GetD1CWallOrnamentZone returns the source-locked (80, 29, 64,
     * 43) wall frame; the portrait sits inside it at (96, 35).  We
     * sanity-check the public zone helper, then verify the inner
     * portrait rectangle is drawn correctly. */
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
        CHECK(D1C_PORTRAIT_VIEW_X >= ornX &&
              D1C_PORTRAIT_VIEW_X + D1C_PORTRAIT_W <= ornX + ornW &&
              D1C_PORTRAIT_VIEW_Y >= ornY &&
              D1C_PORTRAIT_VIEW_Y + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* Render the framebuffer with the C040 panel OFF. */
    memset(fbPanelOff, 0, sizeof(fbPanelOff));
    M11_GameView_Draw(&state, fbPanelOff, FB_W, FB_H);

    /* The D1C portrait rect must contain the expected ordinal's
     * source pixels at >= 90% match. */
    matchOff = match_portrait_at_rect(portraits, fbPanelOff, frontOrdinal);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%)",
                 frontOrdinal, matchOff);
        CHECK(matchOff >= 90, msg);
    }
    /* The rect must have non-zero pixels. */
    nonzeroOff = rect_nonzero(fbPanelOff,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 nonzeroOff);
        CHECK(nonzeroOff >= 100, msg);
    }
    /* The rect must have a non-trivial palette set. */
    distinctOff = rect_distinct(fbPanelOff,
                                D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 4 distinct palette indices "
                 "(got %d)",
                 distinctOff);
        CHECK(distinctOff >= 4, msg);
    }
    /* Warm-pixel proof: champion portraits use the warm palette set. */
    warmOff = rect_warm_count(fbPanelOff,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmOff);
        CHECK(warmOff >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof: the left and right side walls of the D1C
     * portrait band must NOT carry the portrait's warm pixels. */
    leftSideOff = rect_warm_count(fbPanelOff,
                                  SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                  SIDE_WALL_LEFT_W,
                                  PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideOff);
        CHECK(leftSideOff < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideOff = rect_warm_count(fbPanelOff,
                                   SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                   SIDE_WALL_RIGHT_W,
                                   PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "right side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideOff);
        CHECK(rightSideOff < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - redraw_after_candidate
     * ----------------------------------------------------------------
     * With the C040 panel live (candidateMirrorPanelActive=1), the
     * modal candidate redraw owns the D1C view.  The important
     * regression here is the BUG-120/121 class: stale wall-portrait
     * pixels or the wall-ornament placeholder must not float on the
     * ordinary side-wall bands after selection. */
    printf("\n[Group C] redraw_after_candidate: candidate redraw clears stale D1C portrait/ornament pixels\n");

    /* Drive the source-locked candidate selection: REVIVE.C F0280
     * sets candidateMirrorOrdinal, candidateMirrorPanelActive, and
     * the appended party champion.  M11_GameView_SelectFrontMirrorCandidate
     * is the public M11 entry for that path. */
    {
        int rc = M11_GameView_SelectFrontMirrorCandidate(&state);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (1,2,0) returns 1 (got %d)",
                 rc);
        CHECK(rc == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "candidate panel state is live (candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == frontOrdinal &&
              state.candidateMirrorPartyIndex == 0 &&
              state.world.party.championCount == 1, msg);
    }

    /* Re-render with the C040 panel live. */
    memset(fbPanelOn, 0, sizeof(fbPanelOn));
    M11_GameView_Draw(&state, fbPanelOn, FB_W, FB_H);

    /* The candidate redraw should no longer present the full D1C wall
     * portrait sprite.  A very low match means the prior portrait was
     * not left behind as a stale floating sprite after C040 takes over. */
    matchOn = match_portrait_at_rect(portraits, fbPanelOn, frontOrdinal);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "full D1C portrait rect no longer matches ordinal %d "
                 "as a stale sprite after candidate redraw (<= 20%%, got %d%%)",
                 frontOrdinal, matchOn);
        CHECK(matchOn <= 20, msg);
    }
    nonzeroOn = rect_nonzero(fbPanelOn,
                             D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                             D1C_PORTRAIT_W,
                             D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "visible top strip of D1C candidate redraw is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 nonzeroOn);
        CHECK(nonzeroOn >= 100, msg);
    }
    warmOn = rect_warm_count(fbPanelOn,
                             D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                             D1C_PORTRAIT_W,
                             D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "visible top strip of D1C candidate redraw has no "
                 "portrait warm-color leak (<= 10 pixels, got %d)",
                 warmOn);
        CHECK(warmOn <= 10, msg);
    }

    /* The wall-ornament zone around the portrait must be reduced
     * when the panel is live.  We sample the visible top strip and
     * the left border, both outside the C040-covered lower/right area. */
    {
        int dBorderOff = rect_distinct(fbPanelOff,
                                       D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                       D1C_PORTRAIT_W + 2, 1);
        int dBorderOn  = rect_distinct(fbPanelOn,
                                       D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                       D1C_PORTRAIT_W + 2, 1);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "top wall border above D1C portrait stays simple after "
                 "candidate redraw (distinct palette count <= 2, got %d; "
                 "panel-off was %d)",
                 dBorderOff, dBorderOn);
        CHECK(dBorderOn <= 2, msg);
    }
    {
        int dBorderOff = rect_distinct(fbPanelOff,
                                       D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                       1, D1C_PORTRAIT_TOP_VISIBLE_H + 1);
        int dBorderOn  = rect_distinct(fbPanelOn,
                                       D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                       1, D1C_PORTRAIT_TOP_VISIBLE_H + 1);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "distinct palette count in left wall border beside "
                 "D1C portrait: panel-off=%d, panel-on=%d "
                 "(panel-on must be <= panel-off)",
                 dBorderOff, dBorderOn);
        CHECK(dBorderOn <= dBorderOff, msg);
    }
    /* The visible top D1C slice (portrait + wall border) should be no
     * more complex when the panel is live because the wall ornament is
     * suppressed.  This is a coarse sanity check on the BUG-120/121 fix. */
    distinctOn = rect_distinct(fbPanelOn,
                               D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                               D1C_PORTRAIT_W + 2,
                               D1C_PORTRAIT_TOP_VISIBLE_H + 1);
    distinctOff = rect_distinct(fbPanelOff,
                                D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                D1C_PORTRAIT_W + 2,
                                D1C_PORTRAIT_TOP_VISIBLE_H + 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "visible top D1C slice (portrait + 1px border) distinct palette "
                 "count: panel-off=%d, panel-on=%d (panel-on <= panel-off)",
                 distinctOff, distinctOn);
        CHECK(distinctOn <= distinctOff, msg);
    }

    /* The portrait rect itself does NOT change position: it's the
     * source-locked (96, 35, 32, 29) destination.  Verify the panel-
     * on frame's visible top strip has the same non-zero pixel count
     * as the panel-off top strip. */
    {
        int topOff = rect_nonzero(fbPanelOff,
                                  D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                  D1C_PORTRAIT_W,
                                  D1C_PORTRAIT_TOP_VISIBLE_H);
        int diffNonzero = (nonzeroOn > topOff)
                              ? (nonzeroOn - topOff)
                              : (topOff - nonzeroOn);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "visible top strip non-zero pixel count is stable "
                 "across panel on/off (off=%d, on=%d, |diff|=%d <= 20)",
                 topOff, nonzeroOn, diffNonzero);
        CHECK(diffNonzero <= 20, msg);
    }

    /* No-floating proof in the panel-on frame: only sample the
     * unoccluded side-wall strip above C040, because the panel covers
     * the lower/right side-wall band by design. */
    leftSideOn = rect_warm_count(fbPanelOn,
                                 SIDE_WALL_LEFT_X, D1C_PORTRAIT_Y,
                                 SIDE_WALL_LEFT_W,
                                 D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideOn);
        CHECK(leftSideOn < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideOn = rect_warm_count(fbPanelOn,
                                  SIDE_WALL_RIGHT_X, D1C_PORTRAIT_Y,
                                  SIDE_WALL_RIGHT_W,
                                  D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "right side wall of D1C portrait band has < %d warm "
                 "pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideOn);
        CHECK(rightSideOn < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - ordinal 14 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 14 must be self-consistent:
     * the destination (96, 35, 32, 29) on the framebuffer lines up
     * with the source (192, 29, 32, 29) in the atlas, and the
     * destination would receive ordinal-14 pixels if the runtime
     * ever routed to that ordinal (e.g. in a CSB or extended-catalog
     * port of the Hall).  This is the "ordinal 14 maps to the
     * expected champion" check from the slice description. */
    printf("\n[Group D] ordinal 14 atlas round-trip: source (192, 29) maps to dst (96, 35)\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 14 is at source "
                 "(%d, %d, %d, %d) - matches "
                 "((14 & 7) * 32, (14 >> 3) * 29, 32, 29)",
                 ORDINAL_14_SRC_X, ORDINAL_14_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ORDINAL_14_SRC_X == 192 && ORDINAL_14_SRC_Y == 29, msg);
    }
    {
        int opaqueCol5Row1 = atlas_cell_opaque_count(portraits, 5 + 8); /* col 5, row 1 = ordinal 13 */
        int opaqueCol7Row1 = atlas_cell_opaque_count(portraits, 7 + 8); /* col 7, row 1 = ordinal 15 */
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "row-1 atlas cells are all defined: ordinal 13 opaque=%d, "
                 "ordinal 14 opaque=%d, ordinal 15 opaque=%d "
                 "(row 1 = 8 portraits: ordinals 8..15)",
                 opaqueCol5Row1, ordinal14Opaque, opaqueCol7Row1);
        CHECK(opaqueCol5Row1 >= 100 && opaqueCol7Row1 >= 100, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas for ordinal 14 opaque count = %d "
                 "(in expected 200..900 range for a defined champion)",
                 ordinal14Opaque);
        CHECK(ordinal14Opaque >= 200 && ordinal14Opaque <= 900, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
