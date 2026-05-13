#include "render_sdl_m11.h"

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

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("m11_display_aspect_present_rect: ok");
    return 0;
}
