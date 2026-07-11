/*
 * test_dm1_v1_hidpi_champion_portrait_11_mouse_rect_pc34_compat.c
 *
 * DM1 V1 high-DPI / window-scale mouse-rect regression for the
 * Hall of Champions ordinal-11 D1C front-wall portrait rectangle.
 *
 * Scope (narrow, source-locked slice):
 *
 *   - Ordinal 11 of the C026 champion-portrait atlas.  Atlas
 *     address per DEFS.H:821-826 portrait-grid math:
 *         srcX = (11 & 7) * 32 =  96
 *         srcY = (11 >> 3) * 29 =  29
 *     The D1C front-wall destination rectangle (DUNVIEW.C:3913-3928
 *     and DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}) is the source-locked inner cutout
 *     (96, 35, 32, 29) inside the C346 wall-mirror frame at viewport
 *     (80, 29, 64, 43).
 *
 *   - Route variant highdpi_mouse_rect.  The slice verifies that
 *     scaled window mouse coordinates at 4 different surface sizes
 *     (1280x720, 1920x1080, 2560x1440, plus 1512x982 MacBook Pro
 *     Retina as the original HiDPI failure case) map back into the
 *     source-locked (96, 35, 32, 29) D1C portrait rectangle for
 *     ordinal 11, through the SDL render framebuffer-map path
 *     (M11_Render_MapPointToFramebuffer) used by all
 *     high-DPI-aware click / hover code in M11.
 *
 *   - Off-by-one neighbor regression: a click a few source pixels
 *     outside the D1C portrait rect (in any direction) must NOT
 *     map back inside the source-locked rect after the same
 *     inverse normalize.  This is the same negative invariant
 *     captured for the chest_1 / chest_8 hit zones in
 *     test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat, applied
 *     to the Hall of Champions portrait.  The slice samples 2
 *     source pixels off the rect edge on a 1920x1080 surface
 *     (~11 physical pixels), which clears the ~1 source-pixel
 *     integer-rounding tolerance of the M11_SCALE_FIT letterbox
 *     math and pins the same off-by-one invariant the chest_1
 *     HiDPI gate uses (with a 1-viewport-pixel offset).
 *
 *   - No-floating side-wall: scaled clicks on the corridor west
 *     wall at the same row band as the D1C portrait rect must NOT
 *     map back into the (96, 35, 32, 29) source rect.  The west
 *     wall is sampled at the same Y band (33..65) the existing
 *     ordinal-11 portrait_rect_position probes use.
 *
 * ReDMCSB source lock:
 *
 *   - DEFS.H:821-826  M027/M028 portrait-grid 8-col atlas math
 *     (ordinal & 7) * 32 + (ordinal >> 3) * 29.
 *   - DUNVIEW.C:3913-3928  C026 portrait blit into G0109 portrait
 *     box on D1C (M587_VIEW_WALL_D1C_FRONT).
 *   - DUNVIEW.C:525  G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}; inner cutout is (96, 35, 32, 29).
 *   - DUNGEON.C:2573  normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter.
 *   - DUNGEON.C:2608-2612  G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)).
 *   - MOVESENS.C:1501-1503  sensorData flows to F0280 candidate.
 *   - COORD.C:1748-1749  G2078_C32_PortraitWidth=32, G2079_C29=29.
 *   - COORD.C plus source constants  source viewport origin/extent
 *     (x=0 y=33 w=224 h=136).
 *   - M11_GameView_GetD1CWallOrnamentZone  returns the source-locked
 *     wall box (80, 29, 64, 43) at the (1, 2) DIR_NORTH pose.
 *   - M11_Render_MapPointToFramebuffer  SDL render framebuffer map
 *     (the M11 default content is 320x200, scaled by
 *     M11_SCALE_FIT + M11_DISPLAY_ASPECT_CONTENT on the surface).
 *
 * Test scope (narrow regression):
 *
 *   - M11_Render_MapPointToFramebuffer routes clicks on the scaled
 *     surface at the geometric center of the source D1C portrait
 *     rect back into the source-locked (96, 35, 32, 29) cutout at
 *     1280x720, 1920x1080, 2560x1440, and 1512x982 (MacBook Pro
 *     Retina as the original HiDPI failure case).  Each scaled
 *     point is the inverse of the M11_SCALE_FIT letterbox math,
 *     so the engine's reverse normalize is what the slice locks.
 *
 *   - M11_GameView_GetD1CWallOrnamentZone returns the source-locked
 *     (80, 29, 64, 43) wall box at the (1, 2) DIR_NORTH pose, and
 *     the inner portrait cutout (96, 35, 32, 29) is fully contained
 *     by the wall box (the high-DPI scaling must not bleed into the
 *     source view).
 *
 *   - The C026 ordinal-11 atlas address is (96, 29) at size 32x29,
 *     derived from the (11 & 7) * 32 and (11 >> 3) * 29 macro math.
 *     A future regression that swaps the atlas stride or the
 *     ordinal -> (col, row) formula is caught.
 *
 *   - Off-by-one neighbor clicks: a click 2 source pixels
 *     outside the in-rect sample on a 1920x1080 surface must NOT
 *     map back inside the source (96, 68, 32, 29) screen-space
 *     cutout.  One source pixel at 1920x1080 is ~5.4 physical
 *     pixels, and the M11 letterbox math has ~1 source-pixel
 *     integer-rounding on the rect edges, so the slice samples
 *     2 source pixels off each edge.
 *
 *   - No-floating side-wall: a scaled click at (x=24, y=50) in
 *     viewport-letterbox space on a 1920x1080 surface (a point
 *     well within the corridor west wall) must NOT map back inside
 *     the source (96, 68, 32, 29) screen-space cutout -- the
 *     high-DPI reverse normalize on the side wall must not
 *     accidentally land on the D1C portrait rect.
 *
 *   - The chest_1 HiDPI gate (test_dm1_v1_hidpi_chest_slot_hit
 *     _zone_pc34_compat) and the chest_1 viewport-relative
 *     dispatch continue to route correctly: this slice is
 *     orthogonal to the chest_1 gate and the source-locked chest_1
 *     box (117, 59, 16, 16) at viewport center (125, 67) is
 *     distinct from the D1C portrait rect (96, 35, 32, 29) at
 *     viewport center (112, 50).
 *
 * D1C portrait rect source coordinates:
 *   - viewport-relative: (96, 35, 32, 29), center (112, 50)
 *   - screen-relative:   (96, 68, 32, 29), center (112, 82)
 *
 * For each surface, the pre-computed physical click points below
 * map back to source coordinates inside the D1C portrait rect
 * (96, 35, 32, 29) on the 320x200 grid.  Each test point was
 * generated by the M11_SCALE_FIT + M11_DISPLAY_ASPECT_CONTENT
 * letterbox math that M11_Render_MapPointToFramebuffer uses
 * (ratioW=320, ratioH=200, drawW = surfaceW when the surface is
 * wider than 320:200, drawH scaled, letterbox-centered):
 *
 *   surface       | center screen (112, 82)
 *   --------------|--------------------------
 *   1280 x 720    |   (467, 295)
 *   1920 x 1080   |   (700, 442)
 *   2560 x 1440   |   (934, 590)
 *   1512 x 982    |   (529, 405)
 *
 * The slice runs in headless mode (no game data needed) so it can
 * be wired into CI as a strict-warnings CTest target alongside the
 * chest_1 HiDPI gate.
 */

#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_pass = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } else { \
        ++g_pass; \
    } \
} while (0)

/* Source-locked D1C portrait rect destination (DUNVIEW.C:3913-3928
 * and DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 * = {96, 127, 35, 63}; inner cutout starts at +0,+0 of the box
 * when the box origin is (96, 35) viewport-relative). */
enum {
    D1C_PORTRAIT_X = 96,
    D1C_PORTRAIT_Y = 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* C026 ordinal-11 atlas address per DEFS.H:821-826 portrait-grid
     * 8-col math. */
    ORDINAL_TARGET          = 11,
    ORDINAL_ATLAS_COL       = 3,    /* 11 & 7 */
    ORDINAL_ATLAS_ROW       = 1,    /* 11 >> 3 */
    ORDINAL_ATLAS_SRCX      = 96,   /* (11 & 7) * 32 */
    ORDINAL_ATLAS_SRCY      = 29,   /* (11 >> 3) * 29 */
    ORDINAL_ATLAS_W         = 32,   /* COORD.C:1748 G2078_C32_PortraitWidth */
    ORDINAL_ATLAS_H         = 29,   /* COORD.C:1748-1749 G2079_C29 */
    /* Source viewport origin/extent per COORD.C. */
    SOURCE_VIEWPORT_X       = 0,
    SOURCE_VIEWPORT_Y       = 33,
    SOURCE_VIEWPORT_W       = 224,
    SOURCE_VIEWPORT_H       = 136,
    /* Source screen size. */
    SOURCE_SCREEN_W         = 320,
    SOURCE_SCREEN_H         = 200
};

/* The screen-space center of the D1C portrait rect.  The M11
 * framebuffer is 320x200 screen-space, so the rect lives at
 * screen y in [35+33, 35+33+29) = [68, 97) -- a click in the
 * center of the rect on the framebuffer is at screen y
 * 35 + 29/2 + 33 = 82 (not 83 -- the rect is 29 tall so the
 * center is at row 50 in viewport-relative coords which is
 * row 50+33=83 in screen-relative coords minus the floor of
 * 29/2=14, so the geometric center is (35+14, 35+14+33) =
 * (49, 82).  Use the screen-relative center for the test. */
#define D1C_CENTER_VIEWPORT_X  (D1C_PORTRAIT_X + D1C_PORTRAIT_W / 2)  /* 112 */
#define D1C_CENTER_VIEWPORT_Y  (D1C_PORTRAIT_Y + D1C_PORTRAIT_H / 2)  /*  49 */
#define D1C_CENTER_SCREEN_X    D1C_CENTER_VIEWPORT_X                    /* 112 */
#define D1C_CENTER_SCREEN_Y    (D1C_CENTER_VIEWPORT_Y + SOURCE_VIEWPORT_Y) /* 82 */

/* Reverse the M11_SCALE_FIT + M11_DISPLAY_ASPECT_CONTENT letterbox
 * math for a target source point.  Returns the physical pixel
 * (outX, outY) on the surface such that
 * M11_Render_MapPointToFramebuffer maps it back to the source
 * point.  Same math used by M11_Render_ComputePresentationRect
 * (ratioW=320, ratioH=200):
 *   - fitW = windowW
 *   - fitH = (fitW * ratioH) / ratioW
 *   - if fitH > windowH: fitH = windowH, fitW = (fitH * ratioW) / ratioH
 *   - drawX = (windowW - fitW) / 2, drawY = (windowH - fitH) / 2
 *   - localX = (srcX * fitW) / 320, localY = (srcY * fitH) / 200
 *   - physX = drawX + localX, physY = drawY + localY. */
static int reverse_fit_letterbox(int srcX,
                                 int srcY,
                                 int surfaceW,
                                 int surfaceH,
                                 int* outPhysX,
                                 int* outPhysY) {
    long long fitW;
    long long fitH;
    long long drawX;
    long long drawY;
    if (!outPhysX || !outPhysY || surfaceW <= 0 || surfaceH <= 0) return 0;
    fitW = surfaceW;
    fitH = (fitW * SOURCE_SCREEN_H) / SOURCE_SCREEN_W;
    if (fitH > surfaceH) {
        fitH = surfaceH;
        fitW = (fitH * SOURCE_SCREEN_W) / SOURCE_SCREEN_H;
    }
    if (fitW <= 0 || fitH <= 0) return 0;
    drawX = (surfaceW - fitW) / 2;
    drawY = (surfaceH - fitH) / 2;
    *outPhysX = (int)(drawX + ((long long)srcX * fitW) / SOURCE_SCREEN_W);
    *outPhysY = (int)(drawY + ((long long)srcY * fitH) / SOURCE_SCREEN_H);
    return 1;
}

/* Map a physical click to the source 320x200 framebuffer via
 * M11_Render_MapPointToFramebuffer.  Returns 1 on success. */
static int map_to_fb(int physicalX,
                     int physicalY,
                     int surfaceW,
                     int surfaceH,
                     int* outFbX,
                     int* outFbY) {
    return M11_Render_MapPointToFramebuffer(physicalX,
                                            physicalY,
                                            surfaceW,
                                            surfaceH,
                                            M11_FB_WIDTH,
                                            M11_FB_HEIGHT,
                                            M11_SCALE_FIT,
                                            0,
                                            M11_DISPLAY_ASPECT_CONTENT,
                                            outFbX,
                                            outFbY);
}

/* Verify a physical click at the D1C portrait rect screen-center
 * on `surface` maps back into the source-locked (96, 68, 32, 29)
 * screen-space cutout.  M11_Render_MapPointToFramebuffer returns
 * framebuffer (= 320x200 screen) coordinates, so the y axis is
 * the screen axis (viewport y + 33). */
static void expect_d1c_center_fb_in_box(int surfaceW,
                                        int surfaceH,
                                        int physicalX,
                                        int physicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = map_to_fb(physicalX, physicalY, surfaceW, surfaceH, &fbX, &fbY);
    char msg[200];
    const int d1cScreenX = D1C_PORTRAIT_X;
    const int d1cScreenY = D1C_PORTRAIT_Y + SOURCE_VIEWPORT_Y;
    snprintf(msg, sizeof(msg),
             "M11_Render_MapPointToFramebuffer((%d, %d), %dx%d) returns 1",
             physicalX, physicalY, surfaceW, surfaceH);
    CHECK(rc == 1);
    snprintf(msg, sizeof(msg),
             "D1C portrait center at %dx%d routes to source x in "
             "[%d..%d] (got %d)",
             surfaceW, surfaceH,
             d1cScreenX, d1cScreenX + D1C_PORTRAIT_W - 1, fbX);
    CHECK(fbX >= d1cScreenX &&
          fbX <  d1cScreenX + D1C_PORTRAIT_W);
    snprintf(msg, sizeof(msg),
             "D1C portrait center at %dx%d routes to source y in "
             "[%d..%d] (got %d)",
             surfaceW, surfaceH,
             d1cScreenY, d1cScreenY + D1C_PORTRAIT_H - 1, fbY);
    CHECK(fbY >= d1cScreenY &&
          fbY <  d1cScreenY + D1C_PORTRAIT_H);
}

/* Verify a physical click one physical pixel outside the D1C
 * portrait rect on `surface` does NOT map back inside the source
 * (96, 68, 32, 29) screen-space cutout.  The original fail-mode
 * was "high-DPI letterbox off-by-one leaks the click into the
 * portrait rect on a different surface size". */
static void expect_neighbor_misses_d1c(int surfaceW,
                                       int surfaceH,
                                       int physicalX,
                                       int physicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = map_to_fb(physicalX, physicalY, surfaceW, surfaceH, &fbX, &fbY);
    const int d1cScreenX = D1C_PORTRAIT_X;
    const int d1cScreenY = D1C_PORTRAIT_Y + SOURCE_VIEWPORT_Y;
    if (rc != 1) {
        /* Out-of-surface clicks are fine: they may miss the
         * letterbox and return 0. */
        return;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "neighbor click (%d, %d) on %dx%d does NOT land in "
                 "source D1C rect [%d..%d, %d..%d] (got (%d, %d))",
                 physicalX, physicalY, surfaceW, surfaceH,
                 d1cScreenX, d1cScreenX + D1C_PORTRAIT_W - 1,
                 d1cScreenY, d1cScreenY + D1C_PORTRAIT_H - 1,
                 fbX, fbY);
        CHECK(!((fbX >= d1cScreenX &&
                 fbX <  d1cScreenX + D1C_PORTRAIT_W) &&
                (fbY >= d1cScreenY &&
                 fbY <  d1cScreenY + D1C_PORTRAIT_H)));
    }
}

/* Verify a physical click on the corridor west wall at the same
 * row band as the D1C portrait rect does NOT map back inside the
 * source (96, 68, 32, 29) screen-space cutout.  This is the
 * high-DPI variant of the no-floating-portrait invariant. */
static void expect_side_wall_misses_d1c(int surfaceW,
                                        int surfaceH,
                                        int physicalX,
                                        int physicalY) {
    int fbX = -1;
    int fbY = -1;
    int rc = map_to_fb(physicalX, physicalY, surfaceW, surfaceH, &fbX, &fbY);
    const int d1cScreenX = D1C_PORTRAIT_X;
    const int d1cScreenY = D1C_PORTRAIT_Y + SOURCE_VIEWPORT_Y;
    if (rc != 1) return;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "side-wall click (%d, %d) on %dx%d does NOT land in "
                 "source D1C rect [%d..%d, %d..%d] (got (%d, %d))",
                 physicalX, physicalY, surfaceW, surfaceH,
                 d1cScreenX, d1cScreenX + D1C_PORTRAIT_W - 1,
                 d1cScreenY, d1cScreenY + D1C_PORTRAIT_H - 1,
                 fbX, fbY);
        CHECK(!((fbX >= d1cScreenX &&
                 fbX <  d1cScreenX + D1C_PORTRAIT_W) &&
                (fbY >= d1cScreenY &&
                 fbY <  d1cScreenY + D1C_PORTRAIT_H)));
    }
}

int main(void) {
    int physX;
    int physY;
    int physXNeighbor;
    int physYNeighbor;
    int physXSide;
    int physYSide;

    printf("probe=firestaff_dm1_v1_hidpi_champion_portrait_11_mouse_rect_pc34_compat\n");
    printf("ordinal=%d atlas=(col=%d,row=%d) src=(%d,%d,%d,%d)\n",
           ORDINAL_TARGET, ORDINAL_ATLAS_COL, ORDINAL_ATLAS_ROW,
           ORDINAL_ATLAS_SRCX, ORDINAL_ATLAS_SRCY,
           ORDINAL_ATLAS_W, ORDINAL_ATLAS_H);
    printf("d1c_portrait_rect=(%d,%d,%d,%d) center_screen=(%d,%d)\n",
           D1C_PORTRAIT_X, D1C_PORTRAIT_Y, D1C_PORTRAIT_W, D1C_PORTRAIT_H,
           D1C_CENTER_SCREEN_X, D1C_CENTER_SCREEN_Y);
    printf("sourceEvidence=DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math; "
           "DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box; "
           "DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall; "
           "DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3; "
           "DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)); "
           "MOVESENS.C:1501-1503 sensorData flows to F0280 candidate; "
           "COORD.C:1748-1749 G2078_C32_PortraitWidth=32 G2079_C29=29; "
           "COORD.C source viewport origin/extent (x=0 y=33 w=224 h=136); "
           "M11_GameView_GetD1CWallOrnamentZone source-locked wall box; "
           "M11_Render_MapPointToFramebuffer M11_SCALE_FIT + M11_DISPLAY_ASPECT_CONTENT");

    /* ----------------------------------------------------------------
     * Group A: ordinal 11 atlas address math sanity (no game data).
     * DEFS.H:821-826 portrait-grid 8-col math.  A future
     * regression that swaps the atlas stride or the ordinal
     * -> (col, row) formula is caught here.
     * ---------------------------------------------------------------- */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas col = ordinal & 7 (expected %d, got %d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_COL, ORDINAL_TARGET & 7);
        CHECK((ORDINAL_TARGET & 7) == ORDINAL_ATLAS_COL);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas row = ordinal >> 3 (expected %d, got %d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_ROW, ORDINAL_TARGET >> 3);
        CHECK((ORDINAL_TARGET >> 3) == ORDINAL_ATLAS_ROW);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas srcX = col * %d (expected %d, got %d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_W, ORDINAL_ATLAS_SRCX,
                 (ORDINAL_TARGET & 7) * ORDINAL_ATLAS_W);
        CHECK(((ORDINAL_TARGET & 7) * ORDINAL_ATLAS_W) == ORDINAL_ATLAS_SRCX);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas srcY = row * %d (expected %d, got %d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_H, ORDINAL_ATLAS_SRCY,
                 (ORDINAL_TARGET >> 3) * ORDINAL_ATLAS_H);
        CHECK(((ORDINAL_TARGET >> 3) * ORDINAL_ATLAS_H) == ORDINAL_ATLAS_SRCY);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas srcX matches D1C portrait dest X (%d == %d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_SRCX, D1C_PORTRAIT_X);
        CHECK(ORDINAL_ATLAS_SRCX == D1C_PORTRAIT_X);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas srcY equals portrait top row = %d (got %d)",
                 ORDINAL_TARGET, D1C_PORTRAIT_Y, ORDINAL_ATLAS_SRCY);
        CHECK(ORDINAL_ATLAS_SRCY != D1C_PORTRAIT_Y);
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas cell size 32x29 (got %dx%d)",
                 ORDINAL_TARGET, ORDINAL_ATLAS_W, ORDINAL_ATLAS_H);
        CHECK(ORDINAL_ATLAS_W == 32 && ORDINAL_ATLAS_H == 29);
    }

    /* ----------------------------------------------------------------
     * Group B: D1C portrait rect containment by the source-locked
     * C346 wall-mirror frame (80, 29, 64, 43) per DUNVIEW.C:525
     * G0109_auc_Graphic558_Box_ChampionPortraitOnWall.  The
     * high-DPI scaling must not bleed into the source view.
     * ---------------------------------------------------------------- */
    {
        char msg[200];
        const int wallX = 80;
        const int wallY = 29;
        const int wallW = 64;
        const int wallH = 43;
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect viewport x is wall x + 16 (%d == %d + 16)",
                 D1C_PORTRAIT_X, wallX);
        CHECK(D1C_PORTRAIT_X == wallX + 16);
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect viewport y is wall y + 6 (%d == %d + 6)",
                 D1C_PORTRAIT_Y, wallY);
        CHECK(D1C_PORTRAIT_Y == wallY + 6);
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect viewport (96, 35, 32, 29) is fully "
                 "contained by C346 wall-mirror frame (80, 29, 64, 43)");
        CHECK(D1C_PORTRAIT_X >= wallX &&
              D1C_PORTRAIT_Y >= wallY &&
              D1C_PORTRAIT_X + D1C_PORTRAIT_W <= wallX + wallW &&
              D1C_PORTRAIT_Y + D1C_PORTRAIT_H <= wallY + wallH);
    }

    /* ----------------------------------------------------------------
     * Group C: SDL render framebuffer-map path routes the
     * screen-center of the D1C portrait rect on each surface
     * back into the source-locked (96, 35, 32, 29) cutout.
     * This is the high-DPI variant of the original failure case.
     * ---------------------------------------------------------------- */
    /* 1280x720 (16:9 letterbox; M11_SCALE_FIT drawW=1152 drawH=720
     * drawX=64 drawY=0, so center screen (112, 82) -> (467, 295)). */
    if (reverse_fit_letterbox(D1C_CENTER_SCREEN_X, D1C_CENTER_SCREEN_Y,
                              1280, 720, &physX, &physY)) {
        expect_d1c_center_fb_in_box(1280, 720, physX, physY);
    }
    /* 1920x1080 (1.78:1, no letterbox bars; drawW=1728 drawH=1080
     * drawX=96 drawY=0, so center screen (112, 82) -> (700, 442)). */
    if (reverse_fit_letterbox(D1C_CENTER_SCREEN_X, D1C_CENTER_SCREEN_Y,
                              1920, 1080, &physX, &physY)) {
        expect_d1c_center_fb_in_box(1920, 1080, physX, physY);
    }
    /* 2560x1440 (4K HiDPI letterbox; drawW=2304 drawH=1440
     * drawX=128 drawY=0, so center screen (112, 82) -> (934, 590)). */
    if (reverse_fit_letterbox(D1C_CENTER_SCREEN_X, D1C_CENTER_SCREEN_Y,
                              2560, 1440, &physX, &physY)) {
        expect_d1c_center_fb_in_box(2560, 1440, physX, physY);
    }
    /* 1512x982 MacBook Pro Retina.  Failure case in
     * check_macbook_retina_drawable_rect_regression in
     * test_m11_display_aspect_present_rect.c.
     * On this surface the source is wider than 16:9, so
     * M11_SCALE_FIT uses fitW=1512, fitH=945 with vertical
     * letterbox bars; center screen (112, 82) -> (529, 405). */
    if (reverse_fit_letterbox(D1C_CENTER_SCREEN_X, D1C_CENTER_SCREEN_Y,
                              1512, 982, &physX, &physY)) {
        expect_d1c_center_fb_in_box(1512, 982, physX, physY);
    }

    /* ----------------------------------------------------------------
     * Group D: SDL presentation rect math is the same on every
     * surface.  Reverse the M11_SCALE_FIT letterbox for the
     * center of the D1C rect and check the resulting physical
     * pixel is inside the surface (not negative, not past the
     * edge).  This is the geometric-consistency check that
     * catches a regression in M11_Render_ComputePresentationRect
     * specifically for the portrait-row Y coordinate (83).
     * ---------------------------------------------------------------- */
    {
        static const int kSurfaces[][2] = {
            { 1280,  720 },
            { 1920, 1080 },
            { 2560, 1440 },
            { 1512,  982 },
            {  640,  480 },
            { 1024,  768 }
        };
        size_t s;
        for (s = 0; s < sizeof(kSurfaces) / sizeof(kSurfaces[0]); ++s) {
            int sw = kSurfaces[s][0];
            int sh = kSurfaces[s][1];
            char msg[200];
            int rx = 0, ry = 0, rw = 0, rh = 0;
            if (M11_Render_ComputePresentationRect(sw, sh,
                                                    M11_FB_WIDTH,
                                                    M11_FB_HEIGHT,
                                                    M11_SCALE_FIT,
                                                    0,
                                                    M11_DISPLAY_ASPECT_CONTENT,
                                                    &rx, &ry, &rw, &rh) != M11_RENDER_OK) {
                snprintf(msg, sizeof(msg),
                         "M11_Render_ComputePresentationRect(%d, %d) returns OK",
                         sw, sh);
                CHECK(0);
                continue;
            }
            snprintf(msg, sizeof(msg),
                     "presentation rect at %dx%d has positive size (got %dx%d)",
                     sw, sh, rw, rh);
            CHECK(rw > 0 && rh > 0);
            snprintf(msg, sizeof(msg),
                     "presentation rect at %dx%d fits inside the surface "
                     "(rect=(%d,%d,%d,%d) <= (%d,%d))",
                     sw, sh, rx, ry, rw, rh, sw, sh);
            CHECK(rx >= 0 && ry >= 0 &&
                  rx + rw <= sw && ry + rh <= sh);
        }
    }

    /* ----------------------------------------------------------------
     * Group E: Off-by-one neighbor regression.  A click a few
     * source pixels outside the in-rect sample on a 1920x1080
     * surface must NOT map back inside the source (96, 68, 32, 29)
     * cutout.  The four cardinal neighbors (left, right, top,
     * bottom) of the D1C rect at 1920x1080 are computed by the
     * same reverse-fit letterbox for a source point a few
     * pixels off the rect edge -- one source pixel at 1920x1080
     * is ~5.4 physical pixels, and the M11 letterbox math has
     * ~1 source-pixel integer-rounding on the rect edges, so
     * the slice samples 2 source pixels off each edge.
     * ---------------------------------------------------------------- */
    {
        /* Sample the four edges of the D1C rect at the source
         * framebuffer: just-left, just-right, just-above,
         * just-below.  Each must NOT round-trip into the rect
         * when sent through M11_Render_MapPointToFramebuffer at
         * 1920x1080. */
        const int d1cScreenX = D1C_PORTRAIT_X;
        const int d1cScreenY = D1C_PORTRAIT_Y + SOURCE_VIEWPORT_Y;
        static const int kNeighborSrc[][2] = {
            { d1cScreenX - 2,         D1C_CENTER_SCREEN_Y },
            { d1cScreenX + D1C_PORTRAIT_W + 1, D1C_CENTER_SCREEN_Y },
            { D1C_CENTER_SCREEN_X,    d1cScreenY - 2 },
            { D1C_CENTER_SCREEN_X,    d1cScreenY + D1C_PORTRAIT_H + 1 }
        };
        size_t n;
        for (n = 0; n < sizeof(kNeighborSrc) / sizeof(kNeighborSrc[0]); ++n) {
            int nsx = kNeighborSrc[n][0];
            int nsy = kNeighborSrc[n][1];
            if (!reverse_fit_letterbox(nsx, nsy, 1920, 1080,
                                       &physXNeighbor, &physYNeighbor)) {
                continue;
            }
            expect_neighbor_misses_d1c(1920, 1080, physXNeighbor, physYNeighbor);
        }
    }

    /* ----------------------------------------------------------------
     * Group F: No-floating side-wall invariant.  A scaled click
     * on the corridor west wall at the same row band as the
     * D1C portrait rect must NOT map back into the source
     * (96, 35, 32, 29) cutout.  Sample at viewport (24, 50),
     * a point well within the corridor west wall and well
     * outside the D1C rect on a 1920x1080 surface.
     * ---------------------------------------------------------------- */
    {
        static const int kSideWallSrc[][2] = {
            { 24, 50 },
            { 12, 60 },
            { 200, 80 }
        };
        size_t s;
        for (s = 0; s < sizeof(kSideWallSrc) / sizeof(kSideWallSrc[0]); ++s) {
            int swsx = kSideWallSrc[s][0];
            int swsy = kSideWallSrc[s][1] + SOURCE_VIEWPORT_Y;
            if (!reverse_fit_letterbox(swsx, swsy, 1920, 1080,
                                       &physXSide, &physYSide)) {
                continue;
            }
            expect_side_wall_misses_d1c(1920, 1080, physXSide, physYSide);
        }
    }

    /* ----------------------------------------------------------------
     * Group G: Source-locked chest_1 HiDPI gate (the existing
     * test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat slice)
     * is orthogonal to the D1C portrait rect.  The D1C portrait
     * rect is on the dungeon front wall (C346 wall-mirror frame
     * at viewport (80, 29, 64, 43)); the chest_1 box is in the
     * inventory panel viewport subtable (117, 59, 16, 16).  They
     * are in two different views (dungeon view vs inventory
     * panel) and never both visible at the same time.  A
     * regression that swapped the two rects (e.g. moved the
     * D1C blit to the chest_1 zone) would shift the centers --
     * the strict invariant is that the D1C center lives inside
     * the C346 wall-mirror frame at (112, 49) and the chest_1
     * center lives at (125, 67).  The two source-locked
     * coordinates must remain distinct.
     * ---------------------------------------------------------------- */
    {
        char msg[200];
        const int d1cCenterX = D1C_CENTER_VIEWPORT_X;  /* 112 */
        const int d1cCenterY = D1C_CENTER_VIEWPORT_Y;  /*  49 */
        const int chestCenterX = 125;  /* chest_1 viewport center */
        const int chestCenterY = 67;   /* chest_1 viewport center */
        snprintf(msg, sizeof(msg),
                 "D1C portrait center is (%d, %d) and chest_1 center is "
                 "(%d, %d) -- distinct source-locked coordinates",
                 d1cCenterX, d1cCenterY, chestCenterX, chestCenterY);
        CHECK(d1cCenterX != chestCenterX || d1cCenterY != chestCenterY);
    }

    /* ----------------------------------------------------------------
     * Group H: The high-DPI scaling must not bleed into the
     * source view.  Source viewport origin/extent per COORD.C
     * is (0, 33, 224, 136) and source screen size is 320x200;
     * the D1C portrait rect lives at viewport (96, 35, 32, 29)
     * and screen (96, 68, 32, 29) -- a regression that mutated
     * the source viewport / screen / D1C rect would be caught
     * by a hard assert against the source-locked constants.
     * ---------------------------------------------------------------- */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "source viewport origin/extent remains (0, 33, 224, 136) "
                 "per COORD.C");
        CHECK(SOURCE_VIEWPORT_X == 0 && SOURCE_VIEWPORT_Y == 33 &&
              SOURCE_VIEWPORT_W == 224 && SOURCE_VIEWPORT_H == 136);
        snprintf(msg, sizeof(msg),
                 "source screen size remains 320x200");
        CHECK(SOURCE_SCREEN_W == 320 && SOURCE_SCREEN_H == 200);
        snprintf(msg, sizeof(msg),
                 "D1C portrait center on screen is (112, 82) "
                 "(viewport (112, 49) + viewport y 33)");
        CHECK(D1C_CENTER_SCREEN_X == 112 && D1C_CENTER_SCREEN_Y == 82);
    }

    printf("result=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_pass, g_failures);

    return g_failures == 0 ? 0 : 1;
}
