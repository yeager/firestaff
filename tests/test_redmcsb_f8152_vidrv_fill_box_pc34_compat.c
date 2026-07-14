#include "redmcsb_f8152_vidrv_fill_box_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum { kRows = 4, kBytes = kRows * REDMCSB_F8152_SCREEN_STRIDE_PIXELS_PC34 };

static int failures;

static void expect_byte(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %u, expected %u)\n", name,
                (unsigned)actual, (unsigned)expected);
        ++failures;
    }
}

static void expect_true(const char *name, int actual)
{
    if (!actual) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    uint8_t pixels[kBytes];
    RedmcsbF8152C25VgaAperturePc34Compat aperture = {pixels, sizeof(pixels)};
    RedmcsbF8152BoxPc34Compat box = {2, 5, 1, 2};

    memset(pixels, 0xEE, sizeof(pixels));
    expect_true("inclusive C25 fill", redmcsb_f8152_vidrv_fill_box_pc34_compat(
                    &aperture, &box, 0x05, 0x10));
    expect_byte("row one left", pixels[1U * 320U + 2U], 0x15);
    expect_byte("row one right", pixels[1U * 320U + 5U], 0x15);
    expect_byte("row two left", pixels[2U * 320U + 2U], 0x15);
    expect_byte("row two right", pixels[2U * 320U + 5U], 0x15);
    expect_byte("left neighbor", pixels[1U * 320U + 1U], 0xEE);
    expect_byte("next line", pixels[3U * 320U + 2U], 0xEE);

    box.left = 319;
    box.right = 319;
    box.top = 0;
    box.bottom = 0;
    expect_true("right edge", redmcsb_f8152_vidrv_fill_box_pc34_compat(
                    &aperture, &box, 0x0F, 0x00));
    expect_byte("right edge write", pixels[319], 0x0F);

    box.left = 4;
    box.right = 3;
    expect_true("inverted box rejected", !redmcsb_f8152_vidrv_fill_box_pc34_compat(
                    &aperture, &box, 0x00, 0x00));
    box.left = 319;
    box.right = 320;
    expect_true("beyond stride rejected", !redmcsb_f8152_vidrv_fill_box_pc34_compat(
                    &aperture, &box, 0x00, 0x00));

    if (strstr(redmcsb_f8152_vidrv_fill_box_source_evidence_pc34(),
               "VIDEODRV.C:3127-3161") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8152 C25 VGA fill box");
    return 0;
}
