#include "redmcsb_f8213_set_single_pixel_c25_pc34_compat.h"

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
    uint8_t aperture[8];

    memset(aperture, 0xEE, sizeof(aperture));
    expect_true("C25 aperture write",
                redmcsb_f8213_set_single_pixel_c25_pc34_compat(
                    aperture, sizeof(aperture), 3, 0x05U, 0x10U));
    expect_byte("viewport bank is ORed", aperture[3], 0x15U);
    expect_byte("other aperture bytes remain", aperture[2], 0xEEU);

    aperture[7] = 0xA5U;
    expect_true("negative index rejected",
                !redmcsb_f8213_set_single_pixel_c25_pc34_compat(
                    aperture, sizeof(aperture), -1, 0x0FU, 0x10U));
    expect_true("past-end index rejected",
                !redmcsb_f8213_set_single_pixel_c25_pc34_compat(
                    aperture, sizeof(aperture), 8, 0x0FU, 0x10U));
    expect_byte("rejected write retained", aperture[7], 0xA5U);

    if (strstr(redmcsb_f8213_set_single_pixel_source_evidence_pc34(),
               "VIDEODRV.C:1052-1062") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8213 PC 3.4 C25 aperture single-pixel write");
    return 0;
}
