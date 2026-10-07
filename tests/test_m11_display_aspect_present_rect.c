#include "render_sdl_m11.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; } } while (0)

static void check_rect(int windowW,
                       int windowH,
                       int scaleMode,
                       int integerScaling,
                       int aspectMode,
                       int expectedX,
                       int expectedY,
                       int expectedW,
                       int expectedH) {
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;
    int rc = M11_Render_ComputePresentationRect(windowW,
                                                windowH,
                                                320,
                                                200,
                                                scaleMode,
                                                integerScaling,
                                                aspectMode,
                                                &x,
                                                &y,
                                                &w,
                                                &h);
    CHECK(rc == M11_RENDER_OK);
    CHECK(x == expectedX);
    CHECK(y == expectedY);
    CHECK(w == expectedW);
    CHECK(h == expectedH);
}

static int scaled_window_coord(int rectStart, int rectSize, int logical, int logicalSize) {
    return rectStart + ((logical * rectSize) + (rectSize / 2)) / logicalSize;
}

static void check_fill_window_mapping(void) {
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;
    int fbX = -1;
    int fbY = -1;

    CHECK(M11_Render_ComputeFillWindowPresentationRect(1512, 982,
                                                        &x, &y, &w, &h) ==
          M11_RENDER_OK);
    CHECK(x == 0 && y == 0 && w == 1512 && h == 982);
    CHECK(M11_Render_MapPointToFillWindowFramebuffer(0, 0, 1512, 982,
                                                      1920, 1080,
                                                      &fbX, &fbY) == 1);
    CHECK(fbX == 0 && fbY == 0);
    CHECK(M11_Render_MapPointToFillWindowFramebuffer(1511, 981, 1512, 982,
                                                      1920, 1080,
                                                      &fbX, &fbY) == 1);
    CHECK(fbX == 1918 && fbY == 1078);
    CHECK(M11_Render_MapPointToFillWindowFramebuffer(-1, 0, 1512, 982,
                                                      1920, 1080,
                                                      &fbX, &fbY) == 0);
}

static void check_map_edges(int windowW,
                            int windowH,
                            int scaleMode,
                            int integerScaling,
                            int aspectMode) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int fbX = -1;
    int fbY = -1;

    CHECK(M11_Render_ComputePresentationRect(windowW,
                                             windowH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             scaleMode,
                                             integerScaling,
                                             aspectMode,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);
    CHECK(rectW >= M11_FB_WIDTH);
    CHECK(rectH >= M11_FB_HEIGHT);

    CHECK(M11_Render_MapPointToFramebuffer(rectX,
                                           rectY,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           scaleMode,
                                           integerScaling,
                                           aspectMode,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 0);
    CHECK(fbY == 0);

    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW - 1,
                                           rectY + rectH - 1,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           scaleMode,
                                           integerScaling,
                                           aspectMode,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == M11_FB_WIDTH - 1);
    CHECK(fbY == M11_FB_HEIGHT - 1);

    if (rectX > 0) {
        CHECK(M11_Render_MapPointToFramebuffer(rectX - 1,
                                               rectY + rectH / 2,
                                               windowW,
                                               windowH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               scaleMode,
                                               integerScaling,
                                               aspectMode,
                                               &fbX,
                                               &fbY) == 0);
    }
    if (rectY > 0) {
        CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                               rectY - 1,
                                               windowW,
                                               windowH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               scaleMode,
                                               integerScaling,
                                               aspectMode,
                                               &fbX,
                                               &fbY) == 0);
    }
    if (rectX + rectW < windowW) {
        CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW,
                                               rectY + rectH / 2,
                                               windowW,
                                               windowH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               scaleMode,
                                               integerScaling,
                                               aspectMode,
                                               &fbX,
                                               &fbY) == 0);
    }
    if (rectY + rectH < windowH) {
        CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                               rectY + rectH,
                                               windowW,
                                               windowH,
                                               M11_FB_WIDTH,
                                               M11_FB_HEIGHT,
                                               scaleMode,
                                               integerScaling,
                                               aspectMode,
                                               &fbX,
                                               &fbY) == 0);
    }
}

static void check_scaled_dm1_command(int logicalX,
                                     int logicalY,
                                     int expectedCommand,
                                     int expectedZone) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int windowX;
    int windowY;
    int fbX = -1;
    int fbY = -1;
    TouchClickZonePc34Compat hit;
    int rc;

    rc = M11_Render_ComputePresentationRect(3600,
                                            2092,
                                            M11_FB_WIDTH,
                                            M11_FB_HEIGHT,
                                            M11_SCALE_FIT,
                                            0,
                                            M11_DISPLAY_ASPECT_CONTENT,
                                            &rectX,
                                            &rectY,
                                            &rectW,
                                            &rectH);
    CHECK(rc == M11_RENDER_OK);

    windowX = scaled_window_coord(rectX, rectW, logicalX, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, logicalY, M11_FB_HEIGHT);

    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           3600,
                                           2092,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == logicalX);
    CHECK(fbY == logicalY);

    /* Source route: ReDMCSB COMMAND.C G0448 movement arrow table. */
    CHECK(TOUCHCLICK_Compat_HitTestWithButton(
        fbX,
        fbY,
        TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
        &hit) == 1);
    CHECK(hit.commandId == (unsigned int)expectedCommand);
    CHECK(hit.zoneIndex == (unsigned int)expectedZone);
    CHECK(hit.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
}

static void check_scaled_letterbox_rejection(void) {
    int fbX = -1;
    int fbY = -1;

    CHECK(M11_Render_MapPointToFramebuffer(30,
                                           100,
                                           3600,
                                           2092,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
}

static void check_integer_scaled_content_input_gate(void) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int cellW = -1;
    int cellH = -1;
    int windowX = -1;
    int windowY = -1;
    int fbX = -1;
    int fbY = -1;
    TouchClickZonePc34Compat hit;

    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             1,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);
    CHECK(rectX == 160);
    CHECK(rectY == 40);
    CHECK(rectW == 1600);
    CHECK(rectH == 1000);
    CHECK((rectW % M11_FB_WIDTH) == 0);
    CHECK((rectH % M11_FB_HEIGHT) == 0);
    cellW = rectW / M11_FB_WIDTH;
    cellH = rectH / M11_FB_HEIGHT;
    CHECK(cellW == 5);
    CHECK(cellH == 5);

    windowX = scaled_window_coord(rectX, rectW, 264, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, 126, M11_FB_HEIGHT);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 264);
    CHECK(fbY == 126);
    /* Source route: ReDMCSB COMMAND.C G0448 movement arrow table. */
    CHECK(TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                              fbY,
                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                              &hit) == 1);
    CHECK(hit.commandId == 3u);
    CHECK(hit.zoneIndex == 70u);

    /* Integer scaling must give every physical pixel in a source cell the
     * same source coordinate before the ReDMCSB COMMAND.C G0448 hit-test.
     * This catches off-by-one regressions at movement-arrow boundaries. */
    windowX = rectX + (263 * cellW);
    windowY = rectY + (125 * cellH);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 263);
    CHECK(fbY == 125);
    CHECK(TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                              fbY,
                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                              &hit) == 1);
    CHECK(hit.commandId == 3u);
    CHECK(hit.zoneIndex == 70u);

    CHECK(M11_Render_MapPointToFramebuffer(windowX + cellW - 1,
                                           windowY + cellH - 1,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 263);
    CHECK(fbY == 125);

    CHECK(M11_Render_MapPointToFramebuffer(windowX - cellW,
                                           windowY,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 262);
    CHECK(fbY == 125);
    CHECK((TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                               fbY,
                                               TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                               &hit) == 0) ||
          hit.zoneIndex != 70u);

    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY - cellH,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 263);
    CHECK(fbY == 124);
    CHECK((TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                               fbY,
                                               TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                               &hit) == 0) ||
          hit.zoneIndex != 70u);

    windowX = rectX + (289 * cellW);
    windowY = rectY + (145 * cellH);
    CHECK(M11_Render_MapPointToFramebuffer(windowX + cellW - 1,
                                           windowY + cellH - 1,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 289);
    CHECK(fbY == 145);
    CHECK(TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                              fbY,
                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                              &hit) == 1);
    CHECK(hit.commandId == 3u);
    CHECK(hit.zoneIndex == 70u);

    CHECK(M11_Render_MapPointToFramebuffer(windowX + cellW,
                                           windowY,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 290);
    CHECK(fbY == 145);
    CHECK((TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                               fbY,
                                               TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                               &hit) == 0) ||
          hit.zoneIndex != 70u);

    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY + cellH,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 289);
    CHECK(fbY == 146);
    CHECK((TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                               fbY,
                                               TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                               &hit) == 0) ||
          hit.zoneIndex != 70u);

    windowX = scaled_window_coord(rectX, rectW, 319, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, 199, M11_FB_HEIGHT);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 319);
    CHECK(fbY == 199);

    CHECK(M11_Render_MapPointToFramebuffer(rectX - 1,
                                           rectY + rectH / 2,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW,
                                           rectY + rectH / 2,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY - 1,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY + rectH,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
}

/* Forward declare: see below for the helper that round-trips a single
 * movement-arrow click through any window-sized M11_SCALE_FIT + integer
 * scaling + content-aspect presentation path. */
static void check_integer_scaled_movement_arrow(int windowW,
                                                int windowH,
                                                int expectedRectX,
                                                int expectedRectY,
                                                int expectedRectW,
                                                int expectedRectH,
                                                int sourceX,
                                                int sourceY,
                                                int expectedCommand,
                                                int expectedZone);

static void check_integer_scaled_movement_arrows_at_resolution(int windowW,
                                                              int windowH,
                                                              int expectedRectX,
                                                              int expectedRectY,
                                                              int expectedRectW,
                                                              int expectedRectH,
                                                              const char* surfaceName) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int fbX = -1;
    int fbY = -1;
    int windowX;
    int windowY;

    /* The integer-scaling branch in M11_Render_ComputePresentationRect only
     * fires when (contentW * ratioH) == (contentH * ratioW); for
     * M11_DISPLAY_ASPECT_CONTENT with the 320x200 framebuffer, ratioW=320
     * and ratioH=200, so the predicate holds (320*200 == 200*320).  Lock
     * that the integer-scaled rect we expect is exactly what the function
     * returns, so a future regression that swaps to a fractional fit would
     * show up as a CHECK failure here instead of silently changing the
     * input-mapping surface. */
    CHECK(M11_Render_ComputePresentationRect(windowW,
                                             windowH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             1,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);
    CHECK(rectX == expectedRectX);
    CHECK(rectY == expectedRectY);
    CHECK(rectW == expectedRectW);
    CHECK(rectH == expectedRectH);

    printf("integer_scaled_movement_arrow_surface=%s window=%dx%d rect=(%d,%d,%d,%d)\n",
           surfaceName, windowW, windowH, rectX, rectY, rectW, rectH);

    /* Lock all six ReDMCSB COMMAND.C G0448 movement arrows at this
     * resolution so a future regression that hard-codes one arrow (or
     * accidentally maps the right column to the turn_right zone) cannot
     * pass while the other five silently drift.  Each click is forwarded
     * through the same scaled-window-coord helper used by the existing
     * scaled DM1 command path so the round-trip math stays consistent
     * with check_integer_scaled_content_input_gate above. */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        248,
                                        135,
                                        1,
                                        70u - 2u); /* turn_left -> C068 */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        276,
                                        135,
                                        3,
                                        70u); /* forward -> C070 */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        305,
                                        135,
                                        2,
                                        70u - 1u); /* turn_right -> C069 */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        248,
                                        157,
                                        6,
                                        70u + 3u); /* left -> C073 */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        276,
                                        157,
                                        5,
                                        70u + 2u); /* backward -> C072 */
    check_integer_scaled_movement_arrow(windowW,
                                        windowH,
                                        rectX,
                                        rectY,
                                        rectW,
                                        rectH,
                                        305,
                                        157,
                                        4,
                                        70u + 1u); /* right -> C071 */

    /* Letterbox edges must still be rejected at the integer-scaled rect,
     * even when the source framebuffer content (320x200) does not fill
     * the window.  Use the same one-pixel-off-the-edge sample points the
     * existing check_map_edges uses for the 4_3 integer-scaled path. */
    CHECK(M11_Render_MapPointToFramebuffer(rectX - 1,
                                           rectY + rectH / 2,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW,
                                           rectY + rectH / 2,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY - 1,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY + rectH,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);

    /* Source corner samples still hit the bottom-right source cell so a
     * regression in the integer-scaled branch cannot silently flip the
     * last visible cell into an out-of-bounds coordinate. */
    windowX = scaled_window_coord(rectX, rectW, M11_FB_WIDTH - 1, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, M11_FB_HEIGHT - 1, M11_FB_HEIGHT);
    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == M11_FB_WIDTH - 1);
    CHECK(fbY == M11_FB_HEIGHT - 1);
}

static void check_integer_scaled_movement_arrow(int windowW,
                                                int windowH,
                                                int expectedRectX,
                                                int expectedRectY,
                                                int expectedRectW,
                                                int expectedRectH,
                                                int sourceX,
                                                int sourceY,
                                                int expectedCommand,
                                                int expectedZone) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int windowX;
    int windowY;
    int fbX = -1;
    int fbY = -1;
    TouchClickZonePc34Compat hit;

    /* Verify the helper is fed the actual integer-scaled rect for this
     * surface, so the source-to-window mapping below uses the same rect
     * a real M11_Render_MapPointToFramebuffer call would observe. */
    CHECK(M11_Render_ComputePresentationRect(windowW,
                                             windowH,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             1,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);
    CHECK(rectX == expectedRectX);
    CHECK(rectY == expectedRectY);
    CHECK(rectW == expectedRectW);
    CHECK(rectH == expectedRectH);

    windowX = scaled_window_coord(rectX, rectW, sourceX, M11_FB_WIDTH);
    windowY = scaled_window_coord(rectY, rectH, sourceY, M11_FB_HEIGHT);

    CHECK(M11_Render_MapPointToFramebuffer(windowX,
                                           windowY,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == sourceX);
    CHECK(fbY == sourceY);

    /* Source route: ReDMCSB COMMAND.C G0448 movement arrow table. */
    CHECK(TOUCHCLICK_Compat_HitTestWithButton(fbX,
                                              fbY,
                                              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                              &hit) == 1);
    CHECK(hit.commandId == (unsigned int)expectedCommand);
    CHECK(hit.zoneIndex == (unsigned int)expectedZone);
    CHECK(hit.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
}

static void check_macbook_retina_drawable_rect_regression(void) {
    int logicalX = -1;
    int logicalY = -1;
    int logicalW = -1;
    int logicalH = -1;
    int drawableX = -1;
    int drawableY = -1;
    int drawableW = -1;
    int drawableH = -1;
    int fbX = -1;
    int fbY = -1;

    /* Regression for the MacBook "tiny view" report: SDL3 mouse events
     * are in logical window coordinates, but SDL_RenderTexture's dest rect
     * is in drawable pixels. A 1512x982 point MacBook window typically has
     * a 3024x1964 render output; presenting with the logical rect would
     * fill only the center quarter of the drawable. */
    CHECK(M11_Render_ComputePresentationRect(1512,
                                             982,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &logicalX,
                                             &logicalY,
                                             &logicalW,
                                             &logicalH) == M11_RENDER_OK);
    CHECK(M11_Render_ComputePresentationRect(3024,
                                             1964,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &drawableX,
                                             &drawableY,
                                             &drawableW,
                                             &drawableH) == M11_RENDER_OK);
    CHECK(logicalX == 0);
    CHECK(logicalY == 18);
    CHECK(logicalW == 1512);
    CHECK(logicalH == 945);
    CHECK(drawableX == 0);
    CHECK(drawableY == 37);
    CHECK(drawableW == 3024);
    CHECK(drawableH == 1890);
    CHECK(drawableW == logicalW * 2);
    CHECK(drawableH == logicalH * 2);
    CHECK(drawableW > 2900);

    CHECK(M11_Render_MapPointToFramebuffer(1511,
                                           981,
                                           1512,
                                           982,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(M11_Render_MapPointToFramebuffer(756,
                                           491,
                                           1512,
                                           982,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 160);
    CHECK(fbY == 100);
}

static int logical_coord_for_drawable_edge(int drawableEdge,
                                           int windowExtent,
                                           int drawableExtent) {
    return (int)(((long long)drawableEdge * windowExtent +
                  drawableExtent - 1) / drawableExtent);
}

static int logical_coord_before_drawable_edge(int drawableEdgeExclusive,
                                              int windowExtent,
                                              int drawableExtent) {
    return (int)(((long long)drawableEdgeExclusive * windowExtent - 1) /
                 drawableExtent);
}

static void check_drawable_input_mapping(int drawableW,
                                        int drawableH,
                                        int scaleMode,
                                        int integerScaling,
                                        const char* label) {
    const int windowW = 1512;
    const int windowH = 982;
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int fbX = -1, fbY = -1;
    int left, top, right, bottom;
    int centerX, centerY;

    CHECK(M11_Render_ComputeDrawablePresentationRect(
              windowW, windowH, drawableW, drawableH, 320, 200,
              scaleMode, integerScaling, M11_DISPLAY_ASPECT_CONTENT,
              &rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    if (rectW <= 0 || rectH <= 0) return;
    left = logical_coord_for_drawable_edge(rectX, windowW, drawableW);
    top = logical_coord_for_drawable_edge(rectY, windowH, drawableH);
    right = logical_coord_before_drawable_edge(rectX + rectW,
                                               windowW, drawableW);
    bottom = logical_coord_before_drawable_edge(rectY + rectH,
                                                windowH, drawableH);
    centerX = logical_coord_for_drawable_edge(rectX + rectW / 2,
                                              windowW, drawableW);
    centerY = logical_coord_for_drawable_edge(rectY + rectH / 2,
                                              windowH, drawableH);

    CHECK(M11_Render_MapPointToDrawableFramebuffer(
              left, top, windowW, windowH, drawableW, drawableH,
              320, 200, scaleMode, integerScaling,
              M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 1);
    CHECK(fbX >= 0 && fbX <= 2);
    CHECK(fbY >= 0 && fbY <= 2);

    CHECK(M11_Render_MapPointToDrawableFramebuffer(
              centerX, centerY, windowW, windowH, drawableW, drawableH,
              320, 200, scaleMode, integerScaling,
              M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 1);
    CHECK(fbX >= 158 && fbX <= 161);
    CHECK(fbY >= 98 && fbY <= 101);

    CHECK(M11_Render_MapPointToDrawableFramebuffer(
              right, bottom, windowW, windowH, drawableW, drawableH,
              320, 200, scaleMode, integerScaling,
              M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 1);
    CHECK(fbX >= 317 && fbX <= 319);
    CHECK(fbY >= 197 && fbY <= 199);

    /* Fixed-scale rectangles and FIT rectangles both keep their bars
     * noninteractive after converting logical input into drawable pixels. */
    if (left > 0) {
        CHECK(M11_Render_MapPointToDrawableFramebuffer(
                  left - 1, centerY, windowW, windowH, drawableW, drawableH,
                  320, 200, scaleMode, integerScaling,
                  M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 0);
    }
    if (right + 1 < windowW) {
        CHECK(M11_Render_MapPointToDrawableFramebuffer(
                  right + 1, centerY, windowW, windowH, drawableW, drawableH,
                  320, 200, scaleMode, integerScaling,
                  M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 0);
    }
    if (top > 0) {
        CHECK(M11_Render_MapPointToDrawableFramebuffer(
                  centerX, top - 1, windowW, windowH, drawableW, drawableH,
                  320, 200, scaleMode, integerScaling,
                  M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 0);
    }
    if (bottom + 1 < windowH) {
        CHECK(M11_Render_MapPointToDrawableFramebuffer(
                  centerX, bottom + 1, windowW, windowH, drawableW, drawableH,
                  320, 200, scaleMode, integerScaling,
                  M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 0);
    }
    printf("PASS drawable input mapping: %s (scale=%d integer=%d drawable=%dx%d)\n",
           label, scaleMode, integerScaling, drawableW, drawableH);
}

static void check_retina_fixed_scale_input_mapping(void) {
    int scale;
    /* Retina 2x in both axes, then an independent 2x/1.5x density ratio. */
    for (scale = M11_SCALE_1X; scale <= M11_SCALE_4X; ++scale) {
        check_drawable_input_mapping(3024, 1964, scale, 0,
                                     "retina 2x fixed scale");
        check_drawable_input_mapping(3024, 1964, scale, 1,
                                     "retina 2x fixed integer scale");
        check_drawable_input_mapping(3024, 1473, scale, 0,
                                     "independent X/Y density fixed scale");
        check_drawable_input_mapping(3024, 1473, scale, 1,
                                     "independent X/Y fixed integer scale");
    }
    check_drawable_input_mapping(3024, 1964, M11_SCALE_FIT, 0,
                                 "retina 2x FIT smooth");
    check_drawable_input_mapping(3024, 1964, M11_SCALE_FIT, 1,
                                 "retina 2x FIT integer");
    check_drawable_input_mapping(3024, 1473, M11_SCALE_FIT, 0,
                                 "independent X/Y density FIT");
    check_drawable_input_mapping(3024, 1964, M11_SCALE_STRETCH, 0,
                                 "retina 2x legacy stretch/FIT");
}

static void check_resize_before_event_mapping(void) {
    /* Inject a native-size snapshot that changed before SDL delivers its
     * resize event. This same resolver is used by the production refresh; a
     * stale cache pair rejects the edge click as outside its old presentation. */
    const int liveWindowW = 1512;
    const int liveWindowH = 982;
    const int liveDrawableW = 3024;
    const int liveDrawableH = 1964;
    const int staleWindowW = 1000;
    const int staleWindowH = 700;
    const int staleDrawableW = 2000;
    const int staleDrawableH = 1400;
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int fbX = -1, fbY = -1;
    int resolvedWindowW = -1, resolvedWindowH = -1;
    int resolvedDrawableW = -1, resolvedDrawableH = -1;
    int clickX;
    int clickY;

    CHECK(M11_Render_ResolveSdl3LiveDimensions(
              liveWindowW, liveWindowH, liveDrawableW, liveDrawableH,
              &resolvedWindowW, &resolvedWindowH,
              &resolvedDrawableW, &resolvedDrawableH) == M11_RENDER_OK);
    CHECK(resolvedWindowW == liveWindowW && resolvedWindowH == liveWindowH);
    CHECK(resolvedDrawableW == liveDrawableW &&
          resolvedDrawableH == liveDrawableH);
    CHECK(M11_Render_ResolveSdl3LiveDimensions(
              liveWindowW, liveWindowH, 0, liveDrawableH,
              &resolvedWindowW, &resolvedWindowH,
              &resolvedDrawableW, &resolvedDrawableH) ==
          M11_RENDER_ERR_INVALID_ARG);
    CHECK(resolvedWindowW == liveWindowW && resolvedWindowH == liveWindowH);
    CHECK(resolvedDrawableW == liveDrawableW &&
          resolvedDrawableH == liveDrawableH);
    CHECK(M11_Render_ComputeDrawablePresentationRect(
              resolvedWindowW, resolvedWindowH,
              resolvedDrawableW, resolvedDrawableH,
              320, 200, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT,
              &rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    if (rectW <= 0 || rectH <= 0) return;
    clickX = logical_coord_before_drawable_edge(rectX + rectW,
                                                resolvedWindowW, resolvedDrawableW);
    clickY = logical_coord_before_drawable_edge(rectY + rectH,
                                                resolvedWindowH, resolvedDrawableH);
    CHECK(M11_Render_MapPointToDrawableFramebuffer(
              clickX, clickY, resolvedWindowW, resolvedWindowH,
              resolvedDrawableW, resolvedDrawableH, 320, 200, M11_SCALE_FIT, 0,
              M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 1);
    CHECK(fbX >= 317 && fbX <= 319);
    CHECK(fbY >= 197 && fbY <= 199);
    CHECK(M11_Render_MapPointToDrawableFramebuffer(
              clickX, clickY, staleWindowW, staleWindowH,
              staleDrawableW, staleDrawableH, 320, 200, M11_SCALE_FIT, 0,
              M11_DISPLAY_ASPECT_CONTENT, &fbX, &fbY) == 0);
    printf("PASS resize-before-event injected live-dimension mapping\n");
}

#if SDL_VERSION_ATLEAST(3, 2, 0)
static int check_native_resize_snapshot(SDL_Window* window,
                                        int requestW,
                                        int requestH,
                                        const char* label) {
    int cacheW = M11_Render_GetWindowWidth();
    int cacheH = M11_Render_GetWindowHeight();
    int liveW = 0, liveH = 0, drawableW = 0, drawableH = 0;
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int expectedX = -1, expectedY = -1, expectedW = -1, expectedH = -1;
    int fbX = -1, fbY = -1;
    int clickX, clickY;
    int contentW = 0, contentH = 0;

    if (!SDL_SetWindowSize(window, requestW, requestH) ||
        !SDL_SyncWindow(window) ||
        !SDL_GetWindowSize(window, &liveW, &liveH) ||
        !SDL_GetRenderOutputSize(M11_Render_GetRenderer(),
                                 &drawableW, &drawableH) ||
        liveW <= 0 || liveH <= 0 || drawableW <= 0 || drawableH <= 0) {
        fprintf(stderr, "SKIP native resize probe %s: SDL size change/query failed: %s\n",
                label, SDL_GetError());
        return 0;
    }
    if ((liveW == cacheW && liveH == cacheH) ||
        M11_Render_GetWindowWidth() != liveW ||
        M11_Render_GetWindowHeight() != liveH) {
        fprintf(stderr, "SKIP native resize probe %s: native size did not change "
                        "independently of cached size (%dx%d -> %dx%d)\n",
                label, cacheW, cacheH, liveW, liveH);
        return 0;
    }
    CHECK(M11_Render_GetContentSize(&contentW, &contentH) == 1);
    CHECK(M11_Render_ComputeDrawablePresentationRect(
              liveW, liveH, drawableW, drawableH, contentW, contentH,
              M11_Render_GetScaleMode(), M11_Render_GetIntegerScaling(),
              M11_Render_GetDisplayAspectMode(),
              &expectedX, &expectedY, &expectedW, &expectedH) == M11_RENDER_OK);
    CHECK(M11_Render_GetPresentRect(&rectX, &rectY, &rectW, &rectH) ==
          M11_RENDER_OK);
    CHECK(rectX == expectedX && rectY == expectedY &&
          rectW == expectedW && rectH == expectedH);

    clickX = (int)(((int64_t)(rectX + rectW / 2) * liveW) / drawableW);
    clickY = (int)(((int64_t)(rectY + rectH / 2) * liveH) / drawableH);
    CHECK(M11_Render_MapWindowToFramebuffer(clickX, clickY, &fbX, &fbY) == 1);
    CHECK(fbX >= contentW / 2 - 2 && fbX <= contentW / 2 + 2);
    CHECK(fbY >= contentH / 2 - 2 && fbY <= contentH / 2 + 2);
    printf("PASS native resize before event: %s before=%dx%d live=%dx%d "
           "drawable=%dx%d rect=%d,%d %dx%d\n",
           label, cacheW, cacheH, liveW, liveH, drawableW, drawableH,
           rectX, rectY, rectW, rectH);
    return 1;
}

static void check_native_resize_before_event_optin(void) {
    const char* enabled = getenv("FIRESTAFF_M11_NATIVE_RESIZE_PROBE");
    SDL_Window* m11Window;
    SDL_Window* target = NULL;
    SDL_Window** windows;
    int count = 0;
    int i;
    int baseW = 0, baseH = 0;
    int pass = 1;

    if (!enabled || strcmp(enabled, "1") != 0) return;
    if (M11_Render_Init(900, 650, M11_SCALE_FIT) != M11_RENDER_OK) {
        fprintf(stderr, "SKIP native resize probe: M11_Render_Init failed\n");
        return;
    }
    if (!M11_Render_HasHostPresentationWindow()) {
        printf("SKIP native resize probe: SDL has no host presentation window\n");
        M11_Render_Shutdown();
        return;
    }
    m11Window = M11_Render_GetWindow();
    windows = SDL_GetWindows(&count);
    if (!windows) {
        fprintf(stderr, "SKIP native resize probe: SDL_GetWindows failed: %s\n",
                SDL_GetError());
        M11_Render_Shutdown();
        return;
    }
    for (i = 0; i < count; ++i) {
        const SDL_WindowID id = SDL_GetWindowID(windows[i]);
        if (id == SDL_GetWindowID(m11Window) &&
            SDL_GetWindowFromID(id) == windows[i]) {
            target = windows[i];
            break;
        }
    }
    SDL_free(windows);
    if (!target) {
        fprintf(stderr, "SKIP native resize probe: could not resolve M11 SDL window ID\n");
        M11_Render_Shutdown();
        return;
    }
    if ((SDL_GetWindowFlags(target) & SDL_WINDOW_MAXIMIZED) != 0) {
        if (!SDL_RestoreWindow(target) || !SDL_SyncWindow(target)) {
            fprintf(stderr, "SKIP native resize probe: could not restore window: %s\n",
                    SDL_GetError());
            M11_Render_Shutdown();
            return;
        }
    }
    if (!SDL_GetWindowSize(target, &baseW, &baseH) || baseW <= 0 || baseH <= 0) {
        fprintf(stderr, "SKIP native resize probe: invalid restored window size\n");
        M11_Render_Shutdown();
        return;
    }

    /* Do not pump events or call M11_Render_HandleResize between these
     * native changes and production queries; this is the resize-before-event
     * condition that previously split render and pointer dimensions. */
    if (!check_native_resize_snapshot(target, baseW + 160, baseH + 120, "grow")) {
        pass = 0;
    } else {
        int liveW = 0, liveH = 0;
        SDL_GetWindowSize(target, &liveW, &liveH);
        if (!check_native_resize_snapshot(target,
                                         liveW > 480 ? liveW - 120 : liveW + 120,
                                         liveH > 380 ? liveH - 100 : liveH + 100,
                                         "shrink")) {
            pass = 0;
        }
    }
    if (!pass) ++failures;
    M11_Render_Shutdown();
}
#else
static void check_native_resize_before_event_optin(void) {
    if (getenv("FIRESTAFF_M11_NATIVE_RESIZE_PROBE")) {
        printf("SKIP native resize probe: SDL 3.2+ window enumeration is required\n");
    }
}
#endif

static void check_sdl3_pixel_size_event_keeps_logical_mouse_space(void) {
    int windowW = -1;
    int windowH = -1;
    int renderW = -1;
    int renderH = -1;
    int fbX = -1;
    int fbY = -1;

    /* SDL3 sends mouse clicks in logical window coordinates, while
     * SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED reports the high-DPI drawable.
     * ReDMCSB entrance hit-tests must see the logical 1512x982 space;
     * otherwise a real MacBook click is mapped against 3024x1964 and can
     * miss every source door button. */
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_OK);
    CHECK(windowW == 1512);
    CHECK(windowH == 982);
    CHECK(renderW == 3024);
    CHECK(renderH == 1964);
    CHECK(M11_Render_MapPointToFramebuffer(756,
                                           491,
                                           windowW,
                                           windowH,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 1);
    CHECK(fbX == 160);
    CHECK(fbY == 100);

    CHECK(M11_Render_ResolveSdl3ResizeEvent(1280,
                                            800,
                                            960,
                                            540,
                                            1920,
                                            1080,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_OK);
    CHECK(windowW == 1280);
    CHECK(windowH == 800);
    CHECK(renderW == 1280);
    CHECK(renderH == 800);
}

static void check_sdl3_stale_pixel_resize_event_preserves_live_retina_pair(void) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_Event event;
    int beforeWindowW = 0, beforeWindowH = 0;
    int beforeDrawableW = 0, beforeDrawableH = 0;
    int resizedWindowW = 0, resizedWindowH = 0;
    int resizedDrawableW = 0, resizedDrawableH = 0;
    int afterWindowW = 0, afterWindowH = 0;
    int afterDrawableW = 0, afterDrawableH = 0;
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int expectedX = -1, expectedY = -1, expectedW = -1, expectedH = -1;
    SDL_Window *window = NULL;
    int live = 0;

    if (M11_Render_Init(900, 650, M11_SCALE_FIT) != M11_RENDER_OK) {
        fprintf(stderr, "SKIP SDL3 resize-event probe: renderer init failed\n");
        return;
    }
    live = M11_Render_HasHostPresentationWindow();
    window = M11_Render_GetWindow();
    CHECK(M11_Render_GetWindowAndDrawableSize(
              &beforeWindowW, &beforeWindowH,
              &beforeDrawableW, &beforeDrawableH) == 1);
    if (!live) {
        int headlessWindowW = 0, headlessWindowH = 0;
        int headlessDrawableW = 0, headlessDrawableH = 0;
        CHECK(beforeWindowW == 900);
        CHECK(beforeWindowH == 650);
        CHECK(M11_Render_HandleResize(900, 650) == M11_RENDER_OK);
        CHECK(M11_Render_GetWindowAndDrawableSize(
                  &headlessWindowW, &headlessWindowH,
                  &headlessDrawableW, &headlessDrawableH) == 1);
        CHECK(headlessWindowW == 900);
        CHECK(headlessWindowH == 650);
        CHECK(headlessDrawableW == 900);
        CHECK(headlessDrawableH == 650);
        printf("PASS SDL3 dummy resize retains explicit 900x650 test surface\n");
        M11_Render_Shutdown();
        return;
    }

#if SDL_VERSION_ATLEAST(3, 2, 0)
    /* Recreate the native-resize race: the window changes size, then an old
     * pixel notification arrives after SDL already exposes the new pair. */
    CHECK(SDL_RestoreWindow(window));
    CHECK(SDL_SetWindowSize(window, 900, 650));
    CHECK(SDL_SyncWindow(window));
    CHECK(SDL_GetWindowSize(window, &resizedWindowW, &resizedWindowH));
    CHECK(SDL_GetRenderOutputSize(M11_Render_GetRenderer(),
                                  &resizedDrawableW, &resizedDrawableH));
    CHECK(resizedWindowW > 0 && resizedWindowH > 0);
    CHECK(resizedDrawableW > 0 && resizedDrawableH > 0);

    /* Drain native resize notifications first so the injected pixel event is
     * the stale event under test, not another queued current-size event. */
    M11_Render_PumpEvents();
#else
    resizedWindowW = beforeWindowW;
    resizedWindowH = beforeWindowH;
    resizedDrawableW = beforeDrawableW;
    resizedDrawableH = beforeDrawableH;
#endif

    /* Deliver a stale pixel-size notification. The queued event must not
     * replace SDL's current logical window/drawable pair. */
    SDL_zero(event);
    event.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    event.window.windowID = SDL_GetWindowID(window);
    event.window.data1 = beforeDrawableW;
    event.window.data2 = beforeDrawableH;
    CHECK(SDL_PushEvent(&event));
    M11_Render_PumpEvents();
    CHECK(M11_Render_GetWindowAndDrawableSize(
              &afterWindowW, &afterWindowH,
              &afterDrawableW, &afterDrawableH) == 1);
    CHECK(afterWindowW == resizedWindowW);
    CHECK(afterWindowH == resizedWindowH);
    CHECK(afterDrawableW == resizedDrawableW);
    CHECK(afterDrawableH == resizedDrawableH);
    CHECK(M11_Render_GetPresentRect(&rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    CHECK(M11_Render_ComputeDrawablePresentationRect(
              afterWindowW, afterWindowH, afterDrawableW, afterDrawableH,
              320, 200, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT,
              &expectedX, &expectedY, &expectedW, &expectedH) == M11_RENDER_OK);
    CHECK(rectX == expectedX);
    CHECK(rectY == expectedY);
    CHECK(rectW == expectedW);
    CHECK(rectH == expectedH);
    printf("PASS SDL3 stale pixel resize preserves current pair %dx%d -> %dx%d\n",
           afterWindowW, afterWindowH, afterDrawableW, afterDrawableH);
    M11_Render_Shutdown();
#endif
}

static void check_arg_validation_invariants(void) {
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;
    int windowW = -1;
    int windowH = -1;
    int renderW = -1;
    int renderH = -1;

    /* Guard 1: a zero/negative content size is a hard validation
     * failure for M11_Render_ComputePresentationRect. SDL3 callers
     * fed with an uninitialised content rect (e.g. the V2 modern-
     * asset path before any modern bitmap declares its canvas size)
     * would otherwise compute fitW = (windowW * contentH) / contentW
     * and divide by zero. Source-lock: src/engine/render_sdl_m11.c
     * M11_Render_ComputePresentationRect INVALID_ARG branch for
     * contentW <= 0 || contentH <= 0 (return at the top of the
     * function body, before x/y/w/h are touched). */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             0,
                                             200,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             320,
                                             0,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             -1,
                                             -1,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);

    /* Guard 2: an unknown scale mode and an unknown display-aspect
     * mode are both hard validation failures. Source-lock:
     * m11_validate_scale + m11_validate_display_aspect in
     * src/engine/render_sdl_m11.c (the second guard at the top of
     * M11_Render_ComputePresentationRect). */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_STRETCH + 1,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             -7,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_32_9 + 1,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             -1,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);

    /* Guard 3: when the resolver rejects input via the content/scale/
     * aspect guards, the four out slots must remain at their caller-
     * supplied sentinel values (locked here to -1, mirroring the
     * sentinel wiring used throughout the other subtests). The
     * contentW <= 0 path returns before x/y/w/h are populated, so
     * the sentinel value must survive.  The invalid-scale and
     * invalid-aspect paths also return before the out-write block,
     * preserving the caller's sentinel.  A regression that wrote to
     * the out slots before the validation block would silently
     * corrupt the caller-side "did this resolve change anything?"
     * reasoning that the M11 launch handler relies on, so the
     * sentinel preservation is locked down here.  Source-lock: the
     * two early-return INVALID_ARG paths in M11_Render_ComputePresentationRect
     * which both sit above the `if (outX) *outX = x;` write block. */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             0,
                                             200,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(rectX == -1);
    CHECK(rectY == -1);
    CHECK(rectW == -1);
    CHECK(rectH == -1);

    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_STRETCH + 1,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(rectX == -1);
    CHECK(rectY == -1);
    CHECK(rectW == -1);
    CHECK(rectH == -1);

    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             0,
                                             M11_DISPLAY_ASPECT_32_9 + 1,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(rectX == -1);
    CHECK(rectY == -1);
    CHECK(rectW == -1);
    CHECK(rectH == -1);

    /* Guard 4: the NULL-pointer rule applies to all four out slots
     * of M11_Render_ResolveSdl3ResizeEvent. SDL3 fires
     * SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED both for legitimate user
     * resizes and during fullscreen transitions where the cached
     * liveRenderW/H is stale; a refactor that flattened the resize
     * resolver into M11_Render_ComputePresentationRect without
     * preserving the explicit NULL out-pointer check would crash on
     * the macOS fullscreen toggle path. Source-lock: the explicit
     * `!outWindowW || !outWindowH || !outRenderW || !outRenderH`
     * guard at the top of M11_Render_ResolveSdl3ResizeEvent in
     * src/engine/render_sdl_m11.c. */
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            NULL,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            NULL,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            NULL,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            NULL) == M11_RENDER_ERR_INVALID_ARG);

    /* Guard 5: non-positive event dimensions in M11_Render_ResolveSdl3ResizeEvent
     * are also a hard validation failure. A zero or negative
     * SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED pair is documented as
     * "window not yet realised" by SDL3; the resolver must surface
     * INVALID_ARG so the caller can fall back to the cached window
     * size rather than propagating (0,0) into the presentation rect.
     * Source-lock: the `eventW <= 0 || eventH <= 0` guard at the
     * top of M11_Render_ResolveSdl3ResizeEvent. */
    CHECK(M11_Render_ResolveSdl3ResizeEvent(0,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            0,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(M11_Render_ResolveSdl3ResizeEvent(-1,
                                            -1,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);

    /* Guard 6: when M11_Render_ResolveSdl3ResizeEvent rejects input,
     * the four out slots must remain at their caller-supplied sentinel
     * values (locked here to -1). A regression where the resolver
     * zeroed or partially populated the out slots on the error path
     * would silently break the caller-side "did this resolve change
     * anything?" reasoning that the M11 launch handler relies on.
     * Source-lock: M11_Render_ResolveSdl3ResizeEvent returns the
     * INVALID_ARG code BEFORE any *outX = ... assignment executes,
     * so the caller's sentinel must survive both the NULL out-pointer
     * and the non-positive event-dimension guards. */
    CHECK(M11_Render_ResolveSdl3ResizeEvent(3024,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            NULL,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(windowW == -1);
    CHECK(windowH == -1);
    CHECK(renderW == -1);
    CHECK(renderH == -1);

    CHECK(M11_Render_ResolveSdl3ResizeEvent(0,
                                            1964,
                                            1512,
                                            982,
                                            3024,
                                            1964,
                                            &windowW,
                                            &windowH,
                                            &renderW,
                                            &renderH) == M11_RENDER_ERR_INVALID_ARG);
    CHECK(windowW == -1);
    CHECK(windowH == -1);
    CHECK(renderW == -1);
    CHECK(renderH == -1);
}

static void check_map_point_rejection_invariants(void) {
    int fbX = -123;
    int fbY = -456;
    int rectX = -1;
    int rectY = -1;
    int rectW = -1;
    int rectH = -1;

    /* M11_Render_MapPointToFramebuffer is the public M11 input-scale
     * boundary used before ReDMCSB COMMAND.C F0358/F0359 style hit-tests
     * see a 320x200 source coordinate.  Its reject paths must be pure
     * failures: return 0 and leave the caller's output slots untouched so
     * stale click coordinates cannot leak into a later command dispatch. */
    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           0,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(fbX == -123);
    CHECK(fbY == -456);

    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           -1,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(fbX == -123);
    CHECK(fbY == -456);

    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_STRETCH + 1,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(fbX == -123);
    CHECK(fbY == -456);

    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_32_9 + 1,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(fbX == -123);
    CHECK(fbY == -456);

    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           NULL,
                                           &fbY) == 0);
    CHECK(fbY == -456);
    CHECK(M11_Render_MapPointToFramebuffer(100,
                                           100,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           NULL) == 0);
    CHECK(fbX == -123);

    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_FIT,
                                             1,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX,
                                             &rectY,
                                             &rectW,
                                             &rectH) == M11_RENDER_OK);
    CHECK(rectX == 160);
    CHECK(rectY == 40);
    CHECK(rectW == 1600);
    CHECK(rectH == 1000);
    CHECK(M11_Render_MapPointToFramebuffer(rectX - 1,
                                           rectY + rectH / 2,
                                           1920,
                                           1080,
                                           M11_FB_WIDTH,
                                           M11_FB_HEIGHT,
                                           M11_SCALE_FIT,
                                           1,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           &fbY) == 0);
    CHECK(fbX == -123);
    CHECK(fbY == -456);
}

int main(void) {
    check_rect(1920, 1080, M11_SCALE_STRETCH, 0, M11_DISPLAY_ASPECT_16_9,
               0, 0, 1920, 1080);
    check_rect(1920, 1080, M11_SCALE_STRETCH, 0, M11_DISPLAY_ASPECT_4_3,
               240, 0, 1440, 1080);
    check_rect(1280, 1024, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_16_9,
               0, 152, 1280, 720);
    check_rect(1280, 1024, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_4_3,
               0, 32, 1280, 960);
    check_rect(1920, 1080, M11_SCALE_FIT, 1, M11_DISPLAY_ASPECT_16_9,
               0, 0, 1920, 1080);
    check_rect(1920, 1080, M11_SCALE_FIT, 1, M11_DISPLAY_ASPECT_4_3,
               240, 0, 1440, 1080);
    check_rect(3600, 2092, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT,
               126, 0, 3347, 2092);
    check_rect(1920, 1080, M11_SCALE_FIT, 1, M11_DISPLAY_ASPECT_CONTENT,
               160, 40, 1600, 1000);
    check_scaled_dm1_command(264, 126, 3, 70);
    check_scaled_letterbox_rejection();
    check_fill_window_mapping();
    /* Per-game 16:10 and ultrawide preferences fit inside the output rather
     * than deforming the 320x200 source frame. */
    check_rect(1920, 1080, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_16_10,
               96, 0, 1728, 1080);
    check_rect(1920, 1080, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_32_9,
               0, 270, 1920, 540);
    check_map_edges(1512, 982, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(3024, 1964, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(3600, 2092, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(1920, 1080, M11_SCALE_FIT, 1, M11_DISPLAY_ASPECT_4_3);
    check_integer_scaled_content_input_gate();
    check_integer_scaled_movement_arrows_at_resolution(1280,
                                                       720,
                                                       160,
                                                       60,
                                                       960,
                                                       600,
                                                       "hd-720p");
    check_integer_scaled_movement_arrows_at_resolution(1920,
                                                       1080,
                                                       160,
                                                       40,
                                                       1600,
                                                       1000,
                                                       "full-hd-1080p");
    check_integer_scaled_movement_arrows_at_resolution(2560,
                                                       1440,
                                                       160,
                                                       20,
                                                       2240,
                                                       1400,
                                                       "qhd-1440p");
    check_arg_validation_invariants();
    check_map_point_rejection_invariants();
    check_macbook_retina_drawable_rect_regression();
    check_retina_fixed_scale_input_mapping();
    check_resize_before_event_mapping();
    check_native_resize_before_event_optin();
    check_sdl3_pixel_size_event_keeps_logical_mouse_space();
    check_sdl3_stale_pixel_resize_event_preserves_live_retina_pair();

    /* Wire the dead-code check_integer_scaled_movement_arrows_at_resolution
     * helper into main() so the M11_SCALE_FIT + integerScaling +
     * M11_DISPLAY_ASPECT_CONTENT movement-arrow round-trip is locked at
     * every common 16:9 + ultrawide + MacBook Retina surface. The helper
     * asserts the integer-scaled rect, then round-trips all six ReDMCSB
     * COMMAND.C G0448 movement arrows (C068 turn_left / C069 turn_right /
     * C070 forward / C071 right / C072 backward / C073 left), then the
     * four letterbox-edge rejection points, then the source corner sample
     * for that surface. The expected rect math is sourced from the
     * integer-scaling branch in M11_Render_ComputePresentationRect
     * (src/engine/render_sdl_m11.c:340-360): ratioW=320 / ratioH=200
     * (content-aspect), factor = min(windowW/320, windowH/200), fitW =
     * 320*factor, fitH = 200*factor, x = (windowW-fitW)/2, y =
     * (windowH-fitH)/2. 1920x1080 -> (160,40,1600,1000) is already
     * covered by check_integer_scaled_content_input_gate above; the four
     * resolutions here pin additional surfaces (1024p 5:4 monitor,
     * ultrawide 3600x2092, MacBook logical 1512x982, MacBook drawable
     * 3024x1964) without duplicating prior coverage. */
    check_integer_scaled_movement_arrows_at_resolution(1280,
                                                       1024,
                                                       0,
                                                       112,
                                                       1280,
                                                       800,
                                                       "desktop_5x4_1024p");
    check_integer_scaled_movement_arrows_at_resolution(1920,
                                                       1080,
                                                       160,
                                                       40,
                                                       1600,
                                                       1000,
                                                       "desktop_16x9_1080p");
    check_integer_scaled_movement_arrows_at_resolution(3600,
                                                       2092,
                                                       200,
                                                       46,
                                                       3200,
                                                       2000,
                                                       "ultrawide_3600x2092");
    check_integer_scaled_movement_arrows_at_resolution(1512,
                                                       982,
                                                       116,
                                                       91,
                                                       1280,
                                                       800,
                                                       "macbook_logical_1512x982");
    check_integer_scaled_movement_arrows_at_resolution(3024,
                                                       1964,
                                                       72,
                                                       82,
                                                       2880,
                                                       1800,
                                                       "macbook_drawable_3024x1964");

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("m11_display_aspect_present_rect: ok");
    return 0;
}
