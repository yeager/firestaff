/*
 * firestaff_m11_integer_scale_input_map_probe.c
 *
 * M11 integer-multiple scale-mode (M11_SCALE_1X..M11_SCALE_4X) input
 * mapping probe.
 *
 * Background. M11_Render_ComputePresentationRect (src/engine/render_sdl_m11.c)
 * has six documented scale modes: 1X/2X/3X/4X (integer multiples),
 * FIT (aspect-preserving fit), and STRETCH (aspect-preserving fill).
 * The 1X..4X branch produces a rect of exactly contentW*factor x
 * contentH*factor, centered in the window; for a 320x200 source that
 * is (320,200) / (640,400) / (960,600) / (1280,800).  The display-aspect
 * mode and integerScaling flag are intentionally ignored in this branch.
 *
 * Existing M11 coverage locks down FIT (with and without integerScaling)
 * and STRETCH in tests/test_m11_display_aspect_present_rect.c and the
 * M11 Phase A probe (probes/m11/firestaff_m11_phase_a_probe.c INV_A14
 * / INV_A14B).  Neither test exercises MapPointToFramebuffer against
 * 1X..4X, so a regression that swapped to a fractional fit for the
 * 2X/3X/4X paths could silently re-center (or scale) the rect without
 * any green test failing.  This probe locks the integer-multiple
 * input-mapping surface end-to-end.
 *
 * Invariants (all data-free, no game data, no SDL window required):
 *
 *   INV_IS01  M11_SCALE_1X at 320x200 / 640x400 / 1920x1080 / 3840x2160
 *             produces centered rects of exactly (320,200) with content
 *             aspect 8:5 (the integer-multiple branch ignores the
 *             M11_DISPLAY_ASPECT_* mode and integerScaling flag).
 *   INV_IS02  M11_SCALE_2X produces 640x400 centered rects.
 *   INV_IS03  M11_SCALE_3X produces 960x600 centered rects.
 *   INV_IS04  M11_SCALE_4X produces 1280x800 centered rects.
 *   INV_IS05  For all four integer modes at 1920x1080, every window
 *             pixel outside the centered rect is rejected by
 *             MapPointToFramebuffer (returns 0, leaves out slots at
 *             caller sentinel).
 *   INV_IS06  For all four integer modes at 1920x1080, the rect
 *             center maps back to fb=(159,99)/(160,100) (rounded)
 *             and the rect corners map back to fb=(0,0) /
 *             (319,199).  This is the input mapping round-trip that
 *             a real DM1 click relies on.
 *   INV_IS07  For M11_SCALE_2X at 1920x1080, a scaled-window click
 *             at the center of the rect routes through
 *             TOUCHCLICK_Compat_HitTestWithButton to the dungeon
 *             screen-relative zone (screen dispatch path), so a
 *             regression that re-centered the 2X rect by 1 px cannot
 *             silently land the click in a non-source zone.
 *   INV_IS08  The 2X movement-arrow row (248,135 turn_left /
 *             276,135 forward / 305,135 turn_right / 248,157 left /
 *             276,157 backward / 305,157 right) round-trips through
 *             the integer-multiple scaled rect, matching the FIT
 *             sibling coverage in test_m11_display_aspect_present_rect.c.
 *   INV_IS09  M11_SCALE_1X input map is the identity: a 320x200
 *             window click at (159,99) maps to fb=(159,99); a click
 *             at (0,0) maps to fb=(0,0); a click at (319,199) maps
 *             to fb=(319,199); a click at (-1, -1) or (320, 200)
 *             is rejected (return 0).
 *   INV_IS10  Cross-aspect-mode invariants: the 2X rect at 1920x1080
 *             stays (640,400) centered at (640,340) regardless of
 *             whether displayAspectMode is 4_3 / 16_9 / CONTENT, and
 *             regardless of whether integerScaling is 0 or 1 (the
 *             integer-multiple branch ignores both).  This locks the
 *             "ignore aspect/integerScaling for 1X..4X" contract that
 *             the render code relies on.
 *
 * Source-locks:
 *   - src/engine/render_sdl_m11.c M11_Render_ComputePresentationRect
 *     1X..4X switch arm (factor = scaleMode + 1, w = contentW*factor,
 *     h = contentH*factor; centered with x = (windowW-w)/2).
 *   - src/engine/render_sdl_m11.c M11_Render_MapPointToFramebuffer
 *     (rejects on windowX < rectX || windowY < rectY ||
 *      windowX >= rectX+rectW || windowY >= rectY+rectH; otherwise
 *      *outFbX = (localX * contentW) / rectW with clamp to
 *      [0, contentW-1]).
 *   - ReDMCSB COMMAND.C F0358/F0359 movement-arrow table + G0448 zone
 *     table (mirrors test_m11_display_aspect_present_rect.c coverage
 *     for the FIT path; this probe adds the 1X..4X round-trip).
 *
 * Exit code: 0 if every invariant PASSes, 1 otherwise.
 *
 * If $SDL_VIDEODRIVER is not set, the probe defaults it to "dummy" so
 * CI / headless runs do not require a display server.  This probe never
 * calls M11_Render_Init (it is pure data-free integer-math coverage of
 * the public compute-rect + map-point entry points), so no video driver
 * is required at all; the setenv is preserved as a defensive default so
 * the probe can be linked into a wider render-aware harness without
 * failing on a host without $DISPLAY.
 */

#include "render_sdl_m11.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# define m11_setenv(k, v) _putenv_s((k), (v))
#else
# include <stdlib.h>
# define m11_setenv(k, v) setenv((k), (v), 0)
#endif

typedef struct {
    int total;
    int passed;
} InvTally;

static void record(InvTally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg ? msg : "");
    } else {
        printf("FAIL %s %s\n", id, msg ? msg : "");
    }
}

static int scaled_window_coord(int rectStart, int rectSize, int logical, int logicalSize) {
    return rectStart + ((logical * rectSize) + (rectSize / 2)) / logicalSize;
}

static void check_centered_rect(InvTally* t,
                                const char* surfaceName,
                                int windowW,
                                int windowH,
                                int scaleMode,
                                int expectedW,
                                int expectedH,
                                int expectedX,
                                int expectedY) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int rc = M11_Render_ComputePresentationRect(windowW,
                                                windowH,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                scaleMode,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &rectX,
                                                &rectY,
                                                &rectW,
                                                &rectH);
    char msg[160];
    snprintf(msg, sizeof(msg),
             "%s window=%dx%d scale=%d rect=(%d,%d,%d,%d)",
             surfaceName, windowW, windowH, scaleMode,
             rectX, rectY, rectW, rectH);
    record(t, "INV_IS_rect",
           rc == M11_RENDER_OK &&
               rectX == expectedX &&
               rectY == expectedY &&
               rectW == expectedW &&
               rectH == expectedH,
           msg);
}

static void check_off_reject(InvTally* t,
                             const char* id,
                             int windowW,
                             int windowH,
                             int scaleMode) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int fbX = -123;
    int fbY = -456;
    int rc = M11_Render_ComputePresentationRect(windowW,
                                                windowH,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                scaleMode,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &rectX,
                                                &rectY,
                                                &rectW,
                                                &rectH);
    if (rc != M11_RENDER_OK) {
        record(t, id, 0, "ComputePresentationRect failed for off-reject setup");
        return;
    }
    int leftMiss = M11_Render_MapPointToFramebuffer(rectX - 1,
                                                   rectY + rectH / 2,
                                                   windowW,
                                                   windowH,
                                                   M11_FB_WIDTH,
                                                   M11_FB_HEIGHT,
                                                   scaleMode,
                                                   0,
                                                   M11_DISPLAY_ASPECT_CONTENT,
                                                   &fbX,
                                                   &fbY);
    int rightMiss = M11_Render_MapPointToFramebuffer(rectX + rectW,
                                                    rectY + rectH / 2,
                                                    windowW,
                                                    windowH,
                                                    M11_FB_WIDTH,
                                                    M11_FB_HEIGHT,
                                                    scaleMode,
                                                    0,
                                                    M11_DISPLAY_ASPECT_CONTENT,
                                                    &fbX,
                                                    &fbY);
    int topMiss = M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                                  rectY - 1,
                                                  windowW,
                                                  windowH,
                                                  M11_FB_WIDTH,
                                                  M11_FB_HEIGHT,
                                                  scaleMode,
                                                  0,
                                                  M11_DISPLAY_ASPECT_CONTENT,
                                                  &fbX,
                                                  &fbY);
    int botMiss = M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                                   rectY + rectH,
                                                   windowW,
                                                   windowH,
                                                   M11_FB_WIDTH,
                                                   M11_FB_HEIGHT,
                                                   scaleMode,
                                                   0,
                                                   M11_DISPLAY_ASPECT_CONTENT,
                                                   &fbX,
                                                   &fbY);
    record(t, id,
           leftMiss == 0 && rightMiss == 0 && topMiss == 0 && botMiss == 0 &&
               fbX == -123 && fbY == -456,
           "off-rect window pixels rejected with sentinel preservation");
}

static void check_round_trip_corners(InvTally* t,
                                     const char* id,
                                     int windowW,
                                     int windowH,
                                     int scaleMode) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int fbX = -1;
    int fbY = -1;
    int rc = M11_Render_ComputePresentationRect(windowW,
                                                windowH,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                scaleMode,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &rectX,
                                                &rectY,
                                                &rectW,
                                                &rectH);
    if (rc != M11_RENDER_OK) {
        record(t, id, 0, "ComputePresentationRect failed for round-trip setup");
        return;
    }

    /* Top-left corner of the rect -> fb=(0,0). */
    int tl = M11_Render_MapPointToFramebuffer(rectX,
                                             rectY,
                                             windowW,
                                             windowH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             scaleMode,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY);
    int tlOk = (tl == 1) && (fbX == 0) && (fbY == 0);

    /* Bottom-right corner of the rect -> fb=(319,199). */
    int br = M11_Render_MapPointToFramebuffer(rectX + rectW - 1,
                                             rectY + rectH - 1,
                                             windowW,
                                             windowH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             scaleMode,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY);
    int brOk = (br == 1) && (fbX == M11_FB_WIDTH - 1) && (fbY == M11_FB_HEIGHT - 1);

    /* Center of the rect -> fb=(159,99) or (160,100) depending on rounding.
     * Use the same scaled_window_coord helper the FIT-row coverage uses
     * to compute the matching window pixel, then verify the inverse
     * mapping lands within +/-1 of the framebuffer center. */
    int centerWindowX = rectX + rectW / 2;
    int centerWindowY = rectY + rectH / 2;
    int cc = M11_Render_MapPointToFramebuffer(centerWindowX,
                                              centerWindowY,
                                              windowW,
                                              windowH,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              scaleMode,
                                              0,
                                              M11_DISPLAY_ASPECT_CONTENT,
                                              &fbX,
                                              &fbY);
    int centerOk = (cc == 1) &&
                   (fbX >= (M11_FB_WIDTH / 2) - 1) &&
                   (fbX <= (M11_FB_WIDTH / 2) + 1) &&
                   (fbY >= (M11_FB_HEIGHT / 2) - 1) &&
                   (fbY <= (M11_FB_HEIGHT / 2) + 1);

    record(t, id,
           tlOk && brOk && centerOk,
           "rect corners round-trip to framebuffer corners and center");
}

static void check_movement_arrow_at_2x(InvTally* t,
                                       int windowW,
                                       int windowH,
                                       int sourceX,
                                       int sourceY,
                                       int expectedCommand,
                                       int expectedZone,
                                       const char* arrowName) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int windowX = -1;
    int windowY = -1;
    int fbX = -1;
    int fbY = -1;
    TouchClickZonePc34Compat hit;
    int rc = M11_Render_ComputePresentationRect(windowW,
                                                windowH,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                M11_SCALE_2X,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &rectX,
                                                &rectY,
                                                &rectW,
                                                &rectH);
    char msg[160];
    if (rc != M11_RENDER_OK) {
        snprintf(msg, sizeof(msg),
                 "%s: ComputePresentationRect failed for window=%dx%d",
                 arrowName, windowW, windowH);
        record(t, "INV_IS_arrow", 0, msg);
        return;
    }
    windowX = scaled_window_coord(rectX, rectW, sourceX, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, sourceY, M11_FB_HEIGHT);
    int mapRc = M11_Render_MapPointToFramebuffer(windowX,
                                                windowY,
                                                windowW,
                                                windowH,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                M11_SCALE_2X,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &fbX,
                                                &fbY);
    int hitOk = (mapRc == 1) &&
                (fbX == sourceX) &&
                (fbY == sourceY) &&
                (TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                                      fbY,
                                                      TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                      &hit) == 1) &&
                (hit.commandId == (unsigned int)expectedCommand) &&
                (hit.zoneIndex == (unsigned int)expectedZone) &&
                (hit.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
    snprintf(msg, sizeof(msg),
             "%s window=%dx%d 2X source=(%d,%d) -> window=(%d,%d) -> fb=(%d,%d) cmd=%u zone=%u",
             arrowName, windowW, windowH, sourceX, sourceY, windowX, windowY,
             fbX, fbY, expectedCommand, expectedZone);
    record(t, "INV_IS_arrow", hitOk, msg);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    m11_setenv("SDL_VIDEODRIVER", "dummy");

    InvTally t = {0, 0};
    printf("# firestaff_m11_integer_scale_input_map_probe\n");
    printf("# sourceEvidence: M11_Render_ComputePresentationRect 1X..4X branch (factor=scaleMode+1), M11_Render_MapPointToFramebuffer (src/engine/render_sdl_m11.c), ReDMCSB COMMAND.C F0358/F0359 + G0448 movement arrow table.\n");

    /* ---------- INV_IS01..INV_IS04: rect math for 1X..4X at multiple
     * window sizes.  Every integer-multiple mode produces a rect that
     * is exactly (320,200) / (640,400) / (960,600) / (1280,800),
     * centered in the window with x = (windowW - rectW) / 2,
     * y = (windowH - rectH) / 2.
     *
     * Window sizes chosen to mirror the FIT coverage in
     * test_m11_display_aspect_present_rect.c plus four additional
     * 4K surfaces where the rect would land at large letterbox
     * margins (a regression in centering math would be loudest here). */

    /* 1X = (320,200). */
    check_centered_rect(&t, "1x@320x200", 320, 200, M11_SCALE_1X,
                        320, 200, 0, 0);
    check_centered_rect(&t, "1x@640x400", 640, 400, M11_SCALE_1X,
                        320, 200, 160, 100);
    check_centered_rect(&t, "1x@1920x1080", 1920, 1080, M11_SCALE_1X,
                        320, 200, 800, 440);
    check_centered_rect(&t, "1x@3840x2160", 3840, 2160, M11_SCALE_1X,
                        320, 200, 1760, 980);

    /* 2X = (640,400). */
    check_centered_rect(&t, "2x@640x400", 640, 400, M11_SCALE_2X,
                        640, 400, 0, 0);
    check_centered_rect(&t, "2x@1280x720", 1280, 720, M11_SCALE_2X,
                        640, 400, 320, 160);
    check_centered_rect(&t, "2x@1920x1080", 1920, 1080, M11_SCALE_2X,
                        640, 400, 640, 340);
    check_centered_rect(&t, "2x@3840x2160", 3840, 2160, M11_SCALE_2X,
                        640, 400, 1600, 880);

    /* 3X = (960,600). */
    check_centered_rect(&t, "3x@960x600", 960, 600, M11_SCALE_3X,
                        960, 600, 0, 0);
    check_centered_rect(&t, "3x@1920x1080", 1920, 1080, M11_SCALE_3X,
                        960, 600, 480, 240);
    check_centered_rect(&t, "3x@3840x2160", 3840, 2160, M11_SCALE_3X,
                        960, 600, 1440, 780);

    /* 4X = (1280,800). */
    check_centered_rect(&t, "4x@1280x800", 1280, 800, M11_SCALE_4X,
                        1280, 800, 0, 0);
    check_centered_rect(&t, "4x@1920x1080", 1920, 1080, M11_SCALE_4X,
                        1280, 800, 320, 140);
    check_centered_rect(&t, "4x@3840x2160", 3840, 2160, M11_SCALE_4X,
                        1280, 800, 1280, 680);

    /* ---------- INV_IS05: off-rect rejection for all four modes at
     * a single window (1920x1080), plus the caller's sentinel-value
     * preservation rule. */
    check_off_reject(&t, "INV_IS05_off_1x_1920x1080",
                     1920, 1080, M11_SCALE_1X);
    check_off_reject(&t, "INV_IS05_off_2x_1920x1080",
                     1920, 1080, M11_SCALE_2X);
    check_off_reject(&t, "INV_IS05_off_3x_1920x1080",
                     1920, 1080, M11_SCALE_3X);
    check_off_reject(&t, "INV_IS05_off_4x_1920x1080",
                     1920, 1080, M11_SCALE_4X);

    /* ---------- INV_IS06: rect-corner round-trip at 1920x1080. */
    check_round_trip_corners(&t, "INV_IS06_corners_1x_1920x1080",
                             1920, 1080, M11_SCALE_1X);
    check_round_trip_corners(&t, "INV_IS06_corners_2x_1920x1080",
                             1920, 1080, M11_SCALE_2X);
    check_round_trip_corners(&t, "INV_IS06_corners_3x_1920x1080",
                             1920, 1080, M11_SCALE_3X);
    check_round_trip_corners(&t, "INV_IS06_corners_4x_1920x1080",
                             1920, 1080, M11_SCALE_4X);

    /* ---------- INV_IS07/INV_IS08: 2X movement-arrow round-trip at
     * 1920x1080.  The 2X rect at 1920x1080 is (640,400) centered at
     * (640,340).  Source: ReDMCSB COMMAND.C G0448 movement arrow
     * table, mirroring test_m11_display_aspect_present_rect.c
     * check_scaled_dm1_command + check_integer_scaled_movement_arrow. */
    check_movement_arrow_at_2x(&t, 1920, 1080, 248, 135,
                               1, 70u - 2u, "turn_left_C068");
    check_movement_arrow_at_2x(&t, 1920, 1080, 276, 135,
                               3, 70u, "forward_C070");
    check_movement_arrow_at_2x(&t, 1920, 1080, 305, 135,
                               2, 70u - 1u, "turn_right_C069");
    check_movement_arrow_at_2x(&t, 1920, 1080, 248, 157,
                               6, 70u + 3u, "left_C073");
    check_movement_arrow_at_2x(&t, 1920, 1080, 276, 157,
                               5, 70u + 2u, "backward_C072");
    check_movement_arrow_at_2x(&t, 1920, 1080, 305, 157,
                               4, 70u + 1u, "right_C071");

    /* ---------- INV_IS09: M11_SCALE_1X input map is the identity when
     * the window exactly matches the framebuffer.  This is the only
     * integer-multiple mode that does not add letterbox margins; a
     * regression that accidentally swapped 1X to FIT for a 320x200
     * window would still produce the same rect but would route the
     * map through a different code path. */
    {
        int fbX = -1;
        int fbY = -1;
        int ok = 1;
        /* (0,0) -> (0,0). */
        if (M11_Render_MapPointToFramebuffer(0,
                                             0,
                                             320,
                                             200,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_1X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY) != 1 ||
            fbX != 0 || fbY != 0) {
            ok = 0;
        }
        /* (319,199) -> (319,199). */
        if (M11_Render_MapPointToFramebuffer(319,
                                             199,
                                             320,
                                             200,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_1X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY) != 1 ||
            fbX != 319 || fbY != 199) {
            ok = 0;
        }
        /* (159,99) -> ~(159,99) center. */
        if (M11_Render_MapPointToFramebuffer(159,
                                             99,
                                             320,
                                             200,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_1X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY) != 1 ||
            fbX != 159 || fbY != 99) {
            ok = 0;
        }
        /* (-1,-1) rejected. */
        if (M11_Render_MapPointToFramebuffer(-1,
                                             -1,
                                             320,
                                             200,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_1X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY) != 0) {
            ok = 0;
        }
        /* (320,200) rejected (one past the corner). */
        if (M11_Render_MapPointToFramebuffer(320,
                                             200,
                                             320,
                                             200,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_1X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &fbX,
                                             &fbY) != 0) {
            ok = 0;
        }
        record(&t, "INV_IS09_1x_identity",
               ok,
               "M11_SCALE_1X at 320x200 is identity for in-rect points and rejects out-of-rect");
    }

    /* ---------- INV_IS10: cross-aspect + cross-integerScaling
     * invariants for 2X at 1920x1080.  The integer-multiple branch
     * ignores both the display-aspect mode and the integerScaling
     * flag, so the (640,400) rect at (640,340) must hold across all
     * six (3 aspect x 2 integerScaling) combinations.  A regression
     * that pulled the integer-scaling flag into the 2X branch (so
     * that 2X at 1920x1080 started snapping to a 960x600 multiple
     * when integerScaling=1) would otherwise pass the rect-math
     * check at integerScaling=0 and silently break the 4K
     * presentation contract. */
    {
        int aspects[] = {M11_DISPLAY_ASPECT_4_3,
                         M11_DISPLAY_ASPECT_16_9,
                         M11_DISPLAY_ASPECT_CONTENT};
        int intScales[] = {0, 1};
        int ai;
        int ii;
        int ok = 1;
        for (ai = 0; ai < 3; ++ai) {
            for (ii = 0; ii < 2; ++ii) {
                int rectX = -1;
                int rectY = -1;
                int rectW = -1;
                int rectH = -1;
                int rc = M11_Render_ComputePresentationRect(1920,
                                                            1080,
                                                            M11_FB_WIDTH,
                                                            M11_FB_HEIGHT,
                                                            M11_SCALE_2X,
                                                            intScales[ii],
                                                            aspects[ai],
                                                            &rectX,
                                                            &rectY,
                                                            &rectW,
                                                            &rectH);
                if (rc != M11_RENDER_OK ||
                    rectX != 640 || rectY != 340 ||
                    rectW != 640 || rectH != 400) {
                    ok = 0;
                }
            }
        }
        record(&t, "INV_IS10_2x_cross_aspect_integerScale",
               ok,
               "2X rect at 1920x1080 is identical across 4_3/16_9/CONTENT and integerScaling=0/1");
    }

    /* ---------- INV_IS10b: same cross-aspect + cross-integerScaling
     * invariants for 3X and 4X at 1920x1080.  The integer-multiple
     * branch must remain aspect/integerScaling-agnostic. */
    {
        int aspects[] = {M11_DISPLAY_ASPECT_4_3,
                         M11_DISPLAY_ASPECT_16_9,
                         M11_DISPLAY_ASPECT_CONTENT};
        int intScales[] = {0, 1};
        int modes[] = {M11_SCALE_3X, M11_SCALE_4X};
        int expectedRects[][4] = {
            {480, 240, 960, 600},  /* 3X at 1920x1080 */
            {320, 140, 1280, 800}, /* 4X at 1920x1080 */
        };
        int mi;
        int ai;
        int ii;
        int ok = 1;
        for (mi = 0; mi < 2; ++mi) {
            for (ai = 0; ai < 3; ++ai) {
                for (ii = 0; ii < 2; ++ii) {
                    int rectX = -1;
                    int rectY = -1;
                    int rectW = -1;
                    int rectH = -1;
                    int rc = M11_Render_ComputePresentationRect(1920,
                                                                1080,
                                                                M11_FB_WIDTH,
                                                                M11_FB_HEIGHT,
                                                                modes[mi],
                                                                intScales[ii],
                                                                aspects[ai],
                                                                &rectX,
                                                                &rectY,
                                                                &rectW,
                                                                &rectH);
                    if (rc != M11_RENDER_OK ||
                        rectX != expectedRects[mi][0] ||
                        rectY != expectedRects[mi][1] ||
                        rectW != expectedRects[mi][2] ||
                        rectH != expectedRects[mi][3]) {
                        ok = 0;
                    }
                }
            }
        }
        record(&t, "INV_IS10b_3x_4x_cross_aspect_integerScale",
               ok,
               "3X/4X rects at 1920x1080 stay aspect/integerScaling-agnostic");
    }

    /* ---------- INV_IS11: clamp guard for the corner pixel.  A click
     * one pixel past the bottom-right corner of the rect should be
     * rejected, not clamped to (319,199).  This is the literal
     * MapPointToFramebuffer guard `windowX >= rectX + rectW`. */
    {
        int rectX = -1;
        int rectY = -1;
        int rectW = -1;
        int rectH = -1;
        int fbX = -1;
        int fbY = -1;
        int rc = M11_Render_ComputePresentationRect(1920,
                                                    1080,
                                                    M11_FB_WIDTH,
                                                    M11_FB_HEIGHT,
                                                    M11_SCALE_4X,
                                                    0,
                                                    M11_DISPLAY_ASPECT_CONTENT,
                                                    &rectX,
                                                    &rectY,
                                                    &rectW,
                                                    &rectH);
        int ok = (rc == M11_RENDER_OK);
        if (ok) {
            int br = M11_Render_MapPointToFramebuffer(rectX + rectW,
                                                     rectY + rectH,
                                                     1920,
                                                     1080,
                                                     M11_FB_WIDTH,
                                                     M11_FB_HEIGHT,
                                                     M11_SCALE_4X,
                                                     0,
                                                     M11_DISPLAY_ASPECT_CONTENT,
                                                     &fbX,
                                                     &fbY);
            ok = (br == 0);
        }
        record(&t, "INV_IS11_corner_overshoot_reject",
               ok,
               "pixel exactly at rectX+rectW x rectY+rectH is rejected (no clamp leakage)");
    }

    printf("summary: %d/%d invariants passed\n", t.passed, t.total);
    if (t.passed != t.total) {
        return 1;
    }
    return 0;
}
