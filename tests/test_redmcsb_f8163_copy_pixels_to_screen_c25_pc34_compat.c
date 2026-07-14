#include "redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat.h"

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
    const uint8_t bitmap[] = {0x12U, 0x34U, 0x56U};
    uint8_t aperture_bytes[12];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};

    memset(aperture_bytes, 0xEE, sizeof(aperture_bytes));
    expect_true("F8163 delegates packed source span",
                redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat(
                    bitmap, sizeof(bitmap), 1U, 4U, 4U, &aperture, 0x10U));
    expect_byte("source low nibble first", aperture_bytes[4], 0x12U);
    expect_byte("second source nibble", aperture_bytes[5], 0x13U);
    expect_byte("third source nibble", aperture_bytes[6], 0x14U);
    expect_byte("fourth source nibble", aperture_bytes[7], 0x15U);
    expect_byte("outside span retained", aperture_bytes[3], 0xEEU);

    aperture_bytes[4] = 0xA5U;
    expect_true("out-of-range span rejected",
                !redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat(
                    bitmap, sizeof(bitmap), 5U, 4U, 2U, &aperture, 0x10U));
    expect_byte("rejected span leaves aperture", aperture_bytes[4], 0xA5U);

    if (strstr(redmcsb_f8163_copy_pixels_to_screen_source_evidence_pc34(),
               "VIDEODRV.C:3607-3646") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8163 PC 3.4 C25 source-bitmap aperture transfer");
    return 0;
}
