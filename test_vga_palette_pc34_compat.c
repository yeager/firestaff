#include <stdio.h>
#include <string.h>
#include "vga_palette_pc34_compat.h"

static int test_brightest_palette(void) {
        const unsigned char* rgb;
        int i;

        /* Color 0 should be black */
        rgb = F9010_VGA_GetColorRgb_Compat(0, 0);
        if (rgb == 0) return 1;
        if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) return 2;

        /* Color 15 should be white */
        rgb = F9010_VGA_GetColorRgb_Compat(15, 0);
        if (rgb == 0) return 3;
        if (rgb[0] != 255 || rgb[1] != 255 || rgb[2] != 255) return 4;

        /* Color 4 should be dark red */
        rgb = F9010_VGA_GetColorRgb_Compat(4, 0);
        if (rgb == 0) return 5;
        if (rgb[0] != 170 || rgb[1] != 0 || rgb[2] != 0) return 6;

        /* All 16 colors at level 0 should be non-null */
        for (i = 0; i < 16; i++) {
                rgb = F9010_VGA_GetColorRgb_Compat((unsigned char)i, 0);
                if (rgb == 0) return 10 + i;
        }

        /* Out-of-range color should return null */
        rgb = F9010_VGA_GetColorRgb_Compat(16, 0);
        if (rgb != 0) return 30;

        /* Out-of-range palette level should return null */
        rgb = F9010_VGA_GetColorRgb_Compat(0, 6);
        if (rgb != 0) return 31;

        return 0;
}

static int test_darkest_palette(void) {
        const unsigned char* rgb;
        int i;

        /* Level 5 (darkest): all colors should be (0,0,0) */
        for (i = 0; i < 16; i++) {
                rgb = F9010_VGA_GetColorRgb_Compat((unsigned char)i, 5);
                if (rgb == 0) return 40 + i;
                if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) return 60 + i;
        }

        return 0;
}

static int test_palette_monotonic_brightness(void) {
        int color;
        unsigned int level;

        /* For each color, brightness should be non-increasing as level increases */
        for (color = 1; color < 16; color++) {
                for (level = 1; level < 6; level++) {
                        const unsigned char* prev;
                        const unsigned char* curr;
                        unsigned int prevSum;
                        unsigned int currSum;

                        prev = F9010_VGA_GetColorRgb_Compat((unsigned char)color, level - 1);
                        curr = F9010_VGA_GetColorRgb_Compat((unsigned char)color, level);
                        if (prev == 0 || curr == 0) return 80;
                        prevSum = (unsigned int)prev[0] + prev[1] + prev[2];
                        currSum = (unsigned int)curr[0] + curr[1] + curr[2];
                        if (currSum > prevSum) return 100 + color;
                }
        }

        return 0;
}

static int test_brightest_matches_array(void) {
        int i;

        /* G9010_auc_VgaPaletteBrightest_Compat should match level 0 of the full array */
        for (i = 0; i < 16; i++) {
                if (memcmp(G9010_auc_VgaPaletteBrightest_Compat[i],
                           G9010_auc_VgaPaletteAll_Compat[0][i], 3) != 0) {
                        return 120 + i;
                }
        }

        return 0;
}

int main(void) {
        int rc;

        rc = test_brightest_palette();
        if (rc != 0) {
                printf("FAIL test_brightest_palette rc=%d\n", rc);
                return rc;
        }

        rc = test_darkest_palette();
        if (rc != 0) {
                printf("FAIL test_darkest_palette rc=%d\n", rc);
                return rc;
        }

        rc = test_palette_monotonic_brightness();
        if (rc != 0) {
                printf("FAIL test_palette_monotonic_brightness rc=%d\n", rc);
                return rc;
        }

        rc = test_brightest_matches_array();
        if (rc != 0) {
                printf("FAIL test_brightest_matches_array rc=%d\n", rc);
                return rc;
        }

        printf("PASS vga_palette_pc34_compat: all tests passed\n");
        return 0;
}
