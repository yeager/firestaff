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

        /* Color 4 should be cyan (0,219,219) — NOT dark red */
        rgb = F9010_VGA_GetColorRgb_Compat(4, 0);
        if (rgb == 0) return 5;
        if (rgb[0] != 0 || rgb[1] != 219 || rgb[2] != 219) return 6;

        /* Color 1 should be gray (109,109,109) — NOT dark blue */
        rgb = F9010_VGA_GetColorRgb_Compat(1, 0);
        if (rgb == 0) return 7;
        if (rgb[0] != 109 || rgb[1] != 109 || rgb[2] != 109) return 8;

        /* Color 8 should be red (255,0,0) */
        rgb = F9010_VGA_GetColorRgb_Compat(8, 0);
        if (rgb == 0) return 9;
        if (rgb[0] != 255 || rgb[1] != 0 || rgb[2] != 0) return 10;

        /* Color 14 should be blue (0,0,255) — NOT gray */
        rgb = F9010_VGA_GetColorRgb_Compat(14, 0);
        if (rgb == 0) return 11;
        if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 255) return 12;

        /* All 16 colors at level 0 should be non-null */
        for (i = 0; i < 16; i++) {
                rgb = F9010_VGA_GetColorRgb_Compat((unsigned char)i, 0);
                if (rgb == 0) return 20 + i;
        }

        /* Out-of-range color should return null */
        rgb = F9010_VGA_GetColorRgb_Compat(16, 0);
        if (rgb != 0) return 40;

        /* Out-of-range palette level should return null */
        rgb = F9010_VGA_GetColorRgb_Compat(0, 6);
        if (rgb != 0) return 41;

        return 0;
}

static int test_cyan_invariant(void) {
        unsigned int level;

        /* Color 4 (Cyan) must be (0,219,219) at ALL 6 brightness levels */
        for (level = 0; level < 6; level++) {
                const unsigned char* rgb;
                rgb = F9010_VGA_GetColorRgb_Compat(4, level);
                if (rgb == 0) return 50 + (int)level;
                if (rgb[0] != 0 || rgb[1] != 219 || rgb[2] != 219) {
                        printf("  cyan_invariant FAIL at level %u: (%u,%u,%u)\n",
                               level, rgb[0], rgb[1], rgb[2]);
                        return 60 + (int)level;
                }
        }

        return 0;
}

static int test_darkest_palette(void) {
        const unsigned char* rgb;

        /* Level 5 (darkest): NOT all-zero in original.
           Color 0 (black) is zero, color 4 (cyan) is non-zero. */
        rgb = F9010_VGA_GetColorRgb_Compat(0, 5);
        if (rgb == 0) return 70;
        if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) return 71;

        /* Color 4 at level 5: cyan is still (0,219,219) */
        rgb = F9010_VGA_GetColorRgb_Compat(4, 5);
        if (rgb == 0) return 72;
        if (rgb[0] != 0 || rgb[1] != 219 || rgb[2] != 219) return 73;

        /* Color 8 (Red) at level 5: (73,0,0) — NOT zero */
        rgb = F9010_VGA_GetColorRgb_Compat(8, 5);
        if (rgb == 0) return 74;
        if (rgb[0] != 73 || rgb[1] != 0 || rgb[2] != 0) return 75;

        /* Color 15 (White) at level 5: (73,73,73) — NOT zero */
        rgb = F9010_VGA_GetColorRgb_Compat(15, 5);
        if (rgb == 0) return 76;
        if (rgb[0] != 73 || rgb[1] != 73 || rgb[2] != 73) return 77;

        return 0;
}

static int test_palette_monotonic_brightness(void) {
        int color;
        unsigned int level;

        /* For each color, brightness should be non-increasing as level increases */
        for (color = 0; color < 16; color++) {
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
                        if (currSum > prevSum) {
                                /* Exception: color 4 (cyan) is invariant — equal is fine */
                                if (color == 4 && currSum == prevSum) continue;
                                printf("  monotonic FAIL: color %d level %u: sum %u > prev %u\n",
                                       color, level, currSum, prevSum);
                                return 100 + color;
                        }
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

static int test_specific_level_values(void) {
        const unsigned char* rgb;

        /* LIGHT1 color 1 should be (73,73,73) = VGA6 (18,18,18) */
        rgb = F9010_VGA_GetColorRgb_Compat(1, 1);
        if (rgb == 0) return 130;
        if (rgb[0] != 73 || rgb[1] != 73 || rgb[2] != 73) return 131;

        /* LIGHT2 color 12 should be (0,0,0) — goes dark before other grays */
        rgb = F9010_VGA_GetColorRgb_Compat(12, 2);
        if (rgb == 0) return 132;
        if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) return 133;

        /* LIGHT3 color 3 (Brown) should be (36,0,0) = VGA6 (9,0,0) */
        rgb = F9010_VGA_GetColorRgb_Compat(3, 3);
        if (rgb == 0) return 134;
        if (rgb[0] != 36 || rgb[1] != 0 || rgb[2] != 0) return 135;

        /* LIGHT5 color 7 (Green) should be (0,36,0) = VGA6 (0,9,0) */
        rgb = F9010_VGA_GetColorRgb_Compat(7, 5);
        if (rgb == 0) return 136;
        if (rgb[0] != 0 || rgb[1] != 36 || rgb[2] != 0) return 137;

        return 0;
}

int main(void) {
        int rc;

        rc = test_brightest_palette();
        if (rc != 0) {
                printf("FAIL test_brightest_palette rc=%d\n", rc);
                return rc;
        }

        rc = test_cyan_invariant();
        if (rc != 0) {
                printf("FAIL test_cyan_invariant rc=%d\n", rc);
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

        rc = test_specific_level_values();
        if (rc != 0) {
                printf("FAIL test_specific_level_values rc=%d\n", rc);
                return rc;
        }

        printf("PASS vga_palette_pc34_compat: all tests passed (original VGA DAC palette)\n");
        return 0;
}
