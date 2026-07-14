/*
 * DM1 V2.1 special-palette presented-capture probe.
 *
 * TITLE.C F0437 changes palette state around source-owned indexed C001
 * pixels.  V2.1 must EPX those pixels, retain the selected presentation
 * resolution, and capture that resulting RGBA surface without substituting
 * the ordinary VGA row.
 */

#include "render_sdl_m11.h"
#include "screenshot_m11.h"
#include "vga_palette_pc34_compat.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define MAKE_DIR(path) _mkdir(path)
#define REMOVE_DIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MAKE_DIR(path) mkdir((path), 0755)
#define REMOVE_DIR(path) rmdir(path)
#endif

static int read_bmp_dimensions_and_pixel(const char* path,
                                         int* out_width,
                                         int* out_height,
                                         unsigned char out_rgb[3]) {
    unsigned char header[57];
    FILE* file;
    size_t read_count;

    if (!path || !out_rgb) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    read_count = fread(header, 1, sizeof(header), file);
    fclose(file);
    if (read_count != sizeof(header) || header[0] != 'B' || header[1] != 'M') {
        return 0;
    }
    if (out_width) {
        *out_width = (int)header[18] | ((int)header[19] << 8) |
                     ((int)header[20] << 16) | ((int)header[21] << 24);
    }
    if (out_height) {
        int height = (int)header[22] | ((int)header[23] << 8) |
                     ((int)header[24] << 16) | ((int)header[25] << 24);
        *out_height = height < 0 ? -height : height;
    }
    out_rgb[0] = header[56];
    out_rgb[1] = header[55];
    out_rgb[2] = header[54];
    return 1;
}

static void force_dummy_video_driver(void) {
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif
}

int main(void) {
    enum { kWidth = 1280, kHeight = 960 };
    const char* temp_dir;
    char root[1024];
    char output_dir[1024];
    char output_path[1280] = {0};
    static unsigned char framebuffer[M11_FB_BYTES];
    const unsigned char* expected;
    const unsigned char* presented;
    unsigned char bmp_rgb[3] = {0, 0, 0};
    unsigned char live_rgb[3] = {0, 0, 0};
    int presented_width = 0;
    int presented_height = 0;
    int bmp_width = 0;
    int bmp_height = 0;
    int ok;

    temp_dir = getenv("TMPDIR");
    if (!temp_dir || !temp_dir[0]) {
        temp_dir = ".";
    }
    snprintf(root, sizeof(root), "%s/firestaff-dm1-v21-special-capture-%lu",
             temp_dir, (unsigned long)SDL_GetTicks());
    if (MAKE_DIR(root) != 0) {
        perror("mkdir");
        return 1;
    }
    snprintf(output_dir, sizeof(output_dir), "%s/nested/captures", root);
    force_dummy_video_driver();
    if (!SDL_Init(SDL_INIT_VIDEO) ||
        M11_Render_Init(kWidth, kHeight, M11_SCALE_FIT) != M11_RENDER_OK) {
        fprintf(stderr, "renderer init failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    M11_Render_SetScaleFilter(M11_SCALE_FILTER_LINEAR);

    memset(framebuffer, M11_FB_ENCODE(0x0Cu, 0u), sizeof(framebuffer));
    if (M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, kWidth, kHeight,
            VGA_PALETTE_PC34_SPECIAL_TITLE) != M11_RENDER_OK) {
        fprintf(stderr, "V2.1 special-palette present failed\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    presented = M11_Render_GetPresentedRGBA(&presented_width, &presented_height);
    expected = F9011_VGA_GetSpecialColorRgb_Compat(
        0x0Cu, VGA_PALETTE_PC34_SPECIAL_TITLE);
    if (presented) {
        live_rgb[0] = presented[0];
        live_rgb[1] = presented[1];
        live_rgb[2] = presented[2];
    }
    ok = presented && expected && presented_width == kWidth &&
         presented_height == kHeight && live_rgb[0] == expected[0] &&
         live_rgb[1] == expected[1] && live_rgb[2] == expected[2] &&
         M11_Screenshot_CapturePresentedRGBA(output_dir, output_path,
                                             (int)sizeof(output_path)) &&
         read_bmp_dimensions_and_pixel(output_path, &bmp_width, &bmp_height,
                                       bmp_rgb) &&
         bmp_width == kWidth && bmp_height == kHeight &&
         bmp_rgb[0] == expected[0] && bmp_rgb[1] == expected[1] &&
         bmp_rgb[2] == expected[2];

    M11_Render_Shutdown();
    SDL_Quit();
    if (output_path[0] != '\0') {
        remove(output_path);
    }
    REMOVE_DIR(output_dir);
    snprintf(output_dir, sizeof(output_dir), "%s/nested", root);
    REMOVE_DIR(output_dir);
    REMOVE_DIR(root);
    if (!ok) {
        fprintf(stderr, "V2.1 special-palette presented capture mismatch\n");
        return 1;
    }
    printf("PASS DM1 V2.1 special-palette selected-resolution capture\n");
    return 0;
}
