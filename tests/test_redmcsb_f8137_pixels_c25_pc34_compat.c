#include "redmcsb_f8137_pixels_c25_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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
    uint8_t aperture[16];

    memset(aperture, 0xEE, sizeof(aperture));
    expect_true("odd C25 span",
                redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 3, 0x05U, 3, 0x10U));
    expect_byte("odd first", aperture[3], 0x15U);
    expect_byte("odd middle", aperture[4], 0x15U);
    expect_byte("odd last", aperture[5], 0x15U);
    expect_byte("prefix retained", aperture[2], 0xEEU);
    expect_byte("suffix retained", aperture[6], 0xEEU);

    expect_true("zero count is source-safe no-op",
                redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 7, 0x0FU, 0, 0x00U));
    expect_byte("zero count retained", aperture[7], 0xEEU);

    aperture[15] = 0xA5U;
    expect_true("out of range rejected",
                !redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 15, 0x00U, 2, 0x10U));
    expect_true("negative count rejected",
                !redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
                    aperture, sizeof(aperture), 0, 0x00U, -1, 0x10U));
    expect_byte("rejected fill retained", aperture[15], 0xA5U);

    if (strstr(redmcsb_f8137_pixels_c25_source_evidence_pc34(),
               "VIDEODRV.C:1065-1210") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8137 PC 3.4 C25 aperture fill");
    return 0;
}
