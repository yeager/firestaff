#include "render_sdl_m11.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <stdio.h>

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
                                             M11_DISPLAY_ASPECT_CONTENT + 1,
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
                                             M11_DISPLAY_ASPECT_CONTENT + 1,
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
    check_map_edges(1512, 982, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(3024, 1964, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(3600, 2092, M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT);
    check_map_edges(1920, 1080, M11_SCALE_FIT, 1, M11_DISPLAY_ASPECT_4_3);
    check_integer_scaled_content_input_gate();
    check_arg_validation_invariants();
    check_macbook_retina_drawable_rect_regression();
    check_sdl3_pixel_size_event_keeps_logical_mouse_space();

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
