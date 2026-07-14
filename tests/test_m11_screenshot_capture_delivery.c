#include "screenshot_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3] = {{{0}}};

const unsigned char* M11_Render_GetFramebuffer(void) {
    return NULL;
}

int M11_Render_GetPaletteLevel(void) {
    return 0;
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
    remove(first);
    remove(second);
    rmdir(outputDir);
    snprintf(outputDir, sizeof(outputDir), "%s/nested", root);
    rmdir(outputDir);
    rmdir(root);
    return 0;
}
