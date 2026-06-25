/*
 * firestaff_m11_v1_nearest_cpu_present_probe.c
 *
 * Regression guard for the DM1 V1 original presentation path.  The 320x200
 * source framebuffer contains tiny details such as 32x29 champion portraits
 * and 8x8 wall-inscription glyphs; V1 must not rely on backend texture
 * stretching that can blur those pixels on SDL3/Metal/HiDPI paths.
 */

#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void check_true(const char* label, int ok) {
    if (!ok) {
        printf("FAIL %s\n", label);
        ++g_failures;
    } else {
        printf("PASS %s\n", label);
    }
}

static int same_rgba(const unsigned char* a, const unsigned char* b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static int block_is_solid(const unsigned char* rgba, int pitch, int x, int y) {
    const unsigned char* first = rgba + ((y * pitch + x) * 4);
    int yy;
    int xx;
    for (yy = 0; yy < 3; ++yy) {
        for (xx = 0; xx < 3; ++xx) {
            const unsigned char* p = rgba + (((y + yy) * pitch + (x + xx)) * 4);
            if (!same_rgba(first, p)) {
                return 0;
            }
        }
    }
    return 1;
}

int main(void) {
    unsigned char* fb;
    const unsigned char* rgba;
    int w = 0;
    int h = 0;
    int x;
    int y;
    int sx;
    int sy;
    int rc;

#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

    rc = M11_Render_Init(320, 200, M11_SCALE_STRETCH);
    check_true("render init", rc == M11_RENDER_OK);
    if (rc != M11_RENDER_OK) {
        return 1;
    }
    (void)M11_Render_SetWindowMode(M11_WINDOW_MODE_WINDOWED);
    (void)M11_Render_HandleResize(960, 600);
    (void)M11_Render_SetScaleMode(M11_SCALE_STRETCH);
    (void)M11_Render_SetScaleFilter(M11_SCALE_FILTER_NEAREST);

    fb = M11_Render_GetFramebuffer();
    check_true("framebuffer available", fb != NULL);
    if (!fb) {
        M11_Render_Shutdown();
        return 1;
    }

    memset(fb, 0, M11_FB_BYTES);
    for (y = 0; y < M11_FB_HEIGHT; ++y) {
        for (x = 0; x < M11_FB_WIDTH; ++x) {
            fb[y * M11_FB_WIDTH + x] = (unsigned char)(((x + y) & 1) ? 15 : 0);
        }
    }

    rc = M11_Render_PresentIndexed(fb, M11_FB_WIDTH, M11_FB_HEIGHT);
    check_true("present indexed nearest", rc == M11_RENDER_OK);
    rgba = M11_Render_GetPresentedRGBA(&w, &h);
    check_true("presented rgba available", rgba != NULL);
    check_true("presented width is CPU-upscaled 3x", w == 960);
    check_true("presented height is CPU-upscaled 3x", h == 600);

    if (rgba && w == 960 && h == 600) {
        int solid = 1;
        int adjacent_diff = 0;
        for (sy = 35; sy < 64; sy += 7) {
            for (sx = 96; sx < 128; sx += 8) {
                if (!block_is_solid(rgba, w, sx * 3, sy * 3)) {
                    solid = 0;
                }
                if (sx + 1 < M11_FB_WIDTH) {
                    const unsigned char* a = rgba + (((sy * 3) * w + sx * 3) * 4);
                    const unsigned char* b = rgba + (((sy * 3) * w + (sx + 1) * 3) * 4);
                    if (!same_rgba(a, b)) {
                        adjacent_diff = 1;
                    }
                }
            }
        }
        check_true("portrait-size source pixels become solid 3x3 blocks", solid);
        check_true("adjacent source pixels remain distinct, not blended", adjacent_diff);
    }

    M11_Render_Shutdown();
    if (g_failures) {
        printf("FAIL m11_v1_nearest_cpu_present failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS m11_v1_nearest_cpu_present\n");
    return 0;
}
