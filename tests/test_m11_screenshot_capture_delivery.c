#include "screenshot_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3] = {{{0}}};

static unsigned char s_framebuffer[320 * 200];
static int s_indexed_palette_active = 0;

const unsigned char* M11_Render_GetFramebuffer(void) {
    return s_framebuffer;
}

int M11_Render_GetPaletteLevel(void) {
    return 0;
}

int M11_Render_CopyIndexedPaletteRgb6(uint8_t out_rgb6[256][3]) {
    if (!s_indexed_palette_active || !out_rgb6) return 0;
    memset(out_rgb6, 0, 256 * 3);
    out_rgb6[3][0] = 63;
    out_rgb6[3][1] = 31;
    out_rgb6[3][2] = 0;
    return 1;
}

const unsigned char* M11_Render_GetPresentedRGBA(int* outWidth, int* outHeight) {
    if (outWidth) *outWidth = 0;
    if (outHeight) *outHeight = 0;
    return NULL;
}

int main(void) {
    char root[] = "/tmp/firestaff-screenshot-delivery-XXXXXX";
    char outputDir[1024];
    char first[1280];
    char second[1280];
    unsigned char rgba[] = {
        1, 2, 3, 255, 4, 5, 6, 255,
        7, 8, 9, 255, 10, 11, 12, 255
    };
    struct stat firstStat;
    struct stat secondStat;
    FILE* captured = NULL;
    unsigned char header[54];
    unsigned char first_pixel[3];

    if (!mkdtemp(root)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(outputDir, sizeof(outputDir), "%s/nested/presented", root);
    if (!M11_Screenshot_CaptureRGBA(rgba, 2, 2, outputDir, first, (int)sizeof(first)) ||
        !M11_Screenshot_CaptureRGBA(rgba, 2, 2, outputDir, second, (int)sizeof(second))) {
        fprintf(stderr, "capture failed\n");
        return 1;
    }
    if (strcmp(first, second) == 0 || stat(first, &firstStat) != 0 ||
        stat(second, &secondStat) != 0 || firstStat.st_size <= 54 ||
        secondStat.st_size <= 54) {
        fprintf(stderr, "capture paths or BMPs invalid\n");
        return 1;
    }
    memset(s_framebuffer, 3, sizeof(s_framebuffer));
    s_indexed_palette_active = 1;
    if (!M11_Screenshot_CaptureCurrent(outputDir, first, (int)sizeof(first)) ||
        !(captured = fopen(first, "rb")) ||
        fread(header, 1, sizeof(header), captured) != sizeof(header) ||
        fread(first_pixel, 1, sizeof(first_pixel), captured) != sizeof(first_pixel)) {
        fprintf(stderr, "indexed palette capture failed\n");
        if (captured) fclose(captured);
        return 1;
    }
    fclose(captured);
    /* BMP stores BGR. RGB6 (63,31,0) expands to RGB8 (255,125,0). */
    if (first_pixel[0] != 0 || first_pixel[1] != 125 || first_pixel[2] != 255) {
        fprintf(stderr, "indexed palette was not preserved in capture\n");
        return 1;
    }
    remove(first);
    remove(second);
    rmdir(outputDir);
    snprintf(outputDir, sizeof(outputDir), "%s/nested", root);
    rmdir(outputDir);
    rmdir(root);
    return 0;
}
