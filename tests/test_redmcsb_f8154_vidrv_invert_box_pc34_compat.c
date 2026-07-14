#include "redmcsb_f8154_vidrv_invert_box_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum { kRows = 4, kBytes = kRows * REDMCSB_F8154_SCREEN_STRIDE_PIXELS_PC34 };

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
    RedmcsbF8154C25VgaAperturePc34Compat aperture = {pixels, sizeof(pixels)};

    memset(pixels, 0x13, sizeof(pixels));
    expect_true("inclusive rectangle", redmcsb_f8154_vidrv_invert_box_pc34_compat(
                    &aperture, 2, 4, 1, 2));
    expect_byte("row one left", pixels[1U * 320U + 2U], 0x17);
    expect_byte("row one right", pixels[1U * 320U + 4U], 0x17);
    expect_byte("row two left", pixels[2U * 320U + 2U], 0x17);
    expect_byte("outside x", pixels[1U * 320U + 1U], 0x13);
    expect_byte("outside y", pixels[3U * 320U + 2U], 0x13);

    expect_true("second invert restores", redmcsb_f8154_vidrv_invert_box_pc34_compat(
                    &aperture, 2, 4, 1, 2));
    expect_byte("restored", pixels[1U * 320U + 2U], 0x13);

    memcpy(before, pixels, sizeof(pixels));
    expect_true("reversed source loop no-op", redmcsb_f8154_vidrv_invert_box_pc34_compat(
                    &aperture, 5, 4, 1, 2));
    if (memcmp(before, pixels, sizeof(pixels)) != 0) {
        fprintf(stderr, "FAIL: reversed box changed aperture\n");
        ++failures;
    }
    expect_true("out of stride rejected", !redmcsb_f8154_vidrv_invert_box_pc34_compat(
                    &aperture, 319, 320, 0, 0));

    if (strstr(redmcsb_f8154_vidrv_invert_box_source_evidence_pc34(),
               "VIDEODRV.C:3187-3241") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8154 C25 VGA invert box");
    return 0;
}
