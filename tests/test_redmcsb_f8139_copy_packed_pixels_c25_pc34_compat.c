#include "redmcsb_f8139_copy_packed_pixels_c25_pc34_compat.h"

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
    const uint8_t source[] = {0x12U, 0xABU, 0xC0U};
    uint8_t aperture[8];

    memset(aperture, 0xEE, sizeof(aperture));
    expect_true("odd source begins with low nibble",
                redmcsb_f8139_copy_packed_pixels_c25_pc34_compat(
                    source, sizeof(source), 1U, aperture, sizeof(aperture),
                    2U, 4U, 0x10U));
    expect_byte("low nibble first", aperture[2], 0x12U);
    expect_byte("next high nibble", aperture[3], 0x1AU);
    expect_byte("next low nibble", aperture[4], 0x1BU);
    expect_byte("next high nibble", aperture[5], 0x1CU);
    expect_byte("outside span retained", aperture[1], 0xEEU);

    expect_true("source byte is ORed without colour normalization",
                redmcsb_f8139_copy_packed_pixels_c25_pc34_compat(
                    source, sizeof(source), 0U, aperture, sizeof(aperture),
                    0U, 1U, 0x03U));
    expect_byte("raw source OR semantics", aperture[0], 0x03U);

    aperture[7] = 0xA5U;
    expect_true("out of range rejected",
                !redmcsb_f8139_copy_packed_pixels_c25_pc34_compat(
                    source, sizeof(source), 5U, aperture, sizeof(aperture),
                    7U, 2U, 0x10U));
    expect_byte("rejected transfer retained", aperture[7], 0xA5U);

    if (strstr(redmcsb_f8139_copy_packed_pixels_source_evidence_pc34(),
               "VIDEODRV.C:1224-1280") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8139 PC 3.4 C25 packed-pixel aperture transfer");
    return 0;
}
