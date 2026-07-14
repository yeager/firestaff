#include "redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum { kRows = 4, kBytes = kRows * REDMCSB_F8155_SCREEN_STRIDE_PIXELS_PC34 };

static int failures;

static void expect_byte(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %02X, expected %02X)\n", name,
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
    uint8_t before[kBytes];
    RedmcsbF8155C25VgaAperturePc34Compat aperture = {pixels, sizeof(pixels)};

    memset(pixels, 0x1B, sizeof(pixels));
    expect_true("inclusive hatch", redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat(
                    &aperture, 1, 3, 1, 2));
    /* (x^y) even clears, odd parity preserves. */
    expect_byte("row one x1 clear", pixels[1U * 320U + 1U], 0x00);
    expect_byte("row one x2 retain", pixels[1U * 320U + 2U], 0x1B);
    expect_byte("row one x3 clear", pixels[1U * 320U + 3U], 0x00);
    expect_byte("row two x1 retain", pixels[2U * 320U + 1U], 0x1B);
    expect_byte("row two x2 clear", pixels[2U * 320U + 2U], 0x00);
    expect_byte("outside box", pixels[1U * 320U], 0x1B);

    memcpy(before, pixels, sizeof(pixels));
    expect_true("reversed loop no-op", redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat(
                    &aperture, 4, 3, 1, 2));
    if (memcmp(before, pixels, sizeof(pixels)) != 0) {
        fprintf(stderr, "FAIL: reversed bounds changed aperture\n");
        ++failures;
    }
    expect_true("invalid stride rejected", !redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat(
                    &aperture, 319, 320, 0, 0));

    if (strstr(redmcsb_f8155_vidrv_hatch_screen_box_source_evidence_pc34(),
               "VIDEODRV.C:3243-3300") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8155 C25 VGA hatch box");
    return 0;
}
