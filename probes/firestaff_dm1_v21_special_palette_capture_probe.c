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
                                         int pixel_x,
                                         int pixel_y,
                                         int* out_width,
                                         int* out_height,
                                         unsigned char out_rgb[3]) {
    unsigned char header[54];
    FILE* file;
    size_t read_count;
    int width;
    int height;
    int pixel_offset;
    int row_bytes;
    int padded_row_bytes;
    long pixel_position;

    if (!path || !out_rgb) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    read_count = fread(header, 1, sizeof(header), file);
    if (read_count != sizeof(header) || header[0] != 'B' || header[1] != 'M') {
        fclose(file);
        return 0;
    }
    width = (int)header[18] | ((int)header[19] << 8) |
            ((int)header[20] << 16) | ((int)header[21] << 24);
    height = (int)header[22] | ((int)header[23] << 8) |
             ((int)header[24] << 16) | ((int)header[25] << 24);
    pixel_offset = (int)header[10] | ((int)header[11] << 8) |
                   ((int)header[12] << 16) | ((int)header[13] << 24);
    if (width <= 0 || height == 0 || pixel_x < 0 || pixel_x >= width ||
        pixel_y < 0 || pixel_y >= (height < 0 ? -height : height) ||
        pixel_offset < (int)sizeof(header)) {
        fclose(file);
        return 0;
    }
    row_bytes = width * 3;
    padded_row_bytes = (row_bytes + 3) & ~3;
    if (height > 0) {
        pixel_y = height - 1 - pixel_y;
    }
    pixel_position = (long)pixel_offset + (long)pixel_y * padded_row_bytes +
                     (long)pixel_x * 3;
    if (fseek(file, pixel_position, SEEK_SET) != 0 ||
        fread(out_rgb, 1, 3, file) != 3) {
        fclose(file);
        return 0;
    }
    fclose(file);
    {
        unsigned char blue = out_rgb[0];
        out_rgb[0] = out_rgb[2];
        out_rgb[2] = blue;
    }
    if (out_width) *out_width = width;
    if (out_height) *out_height = height < 0 ? -height : height;
    return 1;
}

static int rgb_matches(const unsigned char rgb[3], const unsigned char* expected) {
    return rgb && expected && rgb[0] == expected[0] &&
           rgb[1] == expected[1] && rgb[2] == expected[2];
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
    const unsigned char* presented;
    unsigned char bmp_rgb[4][3] = {{0}};
    const unsigned char* expected[4];
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

    enum { kSourceX = 80, kSourceY = 60, kOutputX = kSourceX * 4,
           kOutputYTop = kSourceY * 24 / 5,
           kOutputYBottom = ((kSourceY * 2 + 1) * 12 + 4) / 5 };

    /* This is an indexed edge relation, not replacement artwork. The source
     * TITLE palette remains authoritative while Scale2x resolves its four
     * source-preserving output pixels. */
    memset(framebuffer, M11_FB_ENCODE(0x0Cu, 0u), sizeof(framebuffer));
    framebuffer[(kSourceY - 1) * M11_FB_WIDTH + kSourceX] =
        M11_FB_ENCODE(0x0Eu, 0u);
    framebuffer[kSourceY * M11_FB_WIDTH + kSourceX - 1] =
        M11_FB_ENCODE(0x0Eu, 0u);
    framebuffer[kSourceY * M11_FB_WIDTH + kSourceX + 1] =
        M11_FB_ENCODE(0x02u, 0u);
    framebuffer[(kSourceY + 1) * M11_FB_WIDTH + kSourceX] =
        M11_FB_ENCODE(0x02u, 0u);
    if (M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, kWidth, kHeight,
            VGA_PALETTE_PC34_SPECIAL_TITLE) != M11_RENDER_OK) {
        fprintf(stderr, "V2.1 special-palette present failed\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    presented = M11_Render_GetPresentedRGBA(&presented_width, &presented_height);
    expected[0] = F9011_VGA_GetSpecialColorRgb_Compat(
        0x0Eu, VGA_PALETTE_PC34_SPECIAL_TITLE);
    expected[1] = F9011_VGA_GetSpecialColorRgb_Compat(
        0x0Cu, VGA_PALETTE_PC34_SPECIAL_TITLE);
    expected[2] = expected[1];
    expected[3] = F9011_VGA_GetSpecialColorRgb_Compat(
        0x02u, VGA_PALETTE_PC34_SPECIAL_TITLE);
    ok = presented && expected[0] && expected[1] && expected[3] &&
         presented_width == kWidth && presented_height == kHeight &&
         rgb_matches(presented + ((kOutputYTop * kWidth + kOutputX) * 4), expected[0]) &&
         rgb_matches(presented + ((kOutputYTop * kWidth + kOutputX + 2) * 4), expected[1]) &&
         rgb_matches(presented + ((kOutputYBottom * kWidth + kOutputX) * 4), expected[2]) &&
         rgb_matches(presented + ((kOutputYBottom * kWidth + kOutputX + 2) * 4), expected[3]) &&
         M11_Screenshot_CapturePresentedRGBA(output_dir, output_path,
                                             (int)sizeof(output_path)) &&
         read_bmp_dimensions_and_pixel(output_path, kOutputX, kOutputYTop,
                                       &bmp_width, &bmp_height, bmp_rgb[0]) &&
         bmp_width == kWidth && bmp_height == kHeight &&
         rgb_matches(bmp_rgb[0], expected[0]) &&
         read_bmp_dimensions_and_pixel(output_path, kOutputX + 2, kOutputYTop,
                                       NULL, NULL, bmp_rgb[1]) &&
         rgb_matches(bmp_rgb[1], expected[1]) &&
         read_bmp_dimensions_and_pixel(output_path, kOutputX, kOutputYBottom,
                                       NULL, NULL, bmp_rgb[2]) &&
         rgb_matches(bmp_rgb[2], expected[2]) &&
         read_bmp_dimensions_and_pixel(output_path, kOutputX + 2, kOutputYBottom,
                                       NULL, NULL, bmp_rgb[3]) &&
         rgb_matches(bmp_rgb[3], expected[3]);

    if (!ok) {
        fprintf(stderr, "V2.1 special-palette presented capture mismatch\n");
        M11_Render_Shutdown();
        SDL_Quit();
        if (output_path[0] != '\0') remove(output_path);
        REMOVE_DIR(output_dir);
        snprintf(output_dir, sizeof(output_dir), "%s/nested", root);
        REMOVE_DIR(output_dir);
        REMOVE_DIR(root);
        return 1;
    }
    M11_Render_Shutdown();
    SDL_Quit();
    if (output_path[0] != '\0') remove(output_path);
    REMOVE_DIR(output_dir);
    snprintf(output_dir, sizeof(output_dir), "%s/nested", root);
    REMOVE_DIR(output_dir);
    REMOVE_DIR(root);
    printf("PASS DM1 V2.1 special-palette selected-resolution capture\n");
    return 0;
}
