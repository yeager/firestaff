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
    check_scaled_dm1_command(264, 126, 3, 70);
    check_scaled_letterbox_rejection();
    check_macbook_retina_drawable_rect_regression();
    check_sdl3_pixel_size_event_keeps_logical_mouse_space();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("m11_display_aspect_present_rect: ok");
    return 0;
}
