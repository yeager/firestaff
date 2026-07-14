#include "redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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
    const uint8_t aperture[] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6};
    uint8_t packed[] = {0xEE, 0xEE, 0xEE, 0xEE};
    uint8_t before[sizeof(packed)];

    expect_true("even start full pairs",
                redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
                    aperture, sizeof(aperture), 0U, packed, sizeof(packed), 0U, 4U));
    expect_byte("pair 0", packed[0], 0x12);
    expect_byte("pair 1", packed[1], 0x34);
    expect_byte("outside pair", packed[2], 0xEE);

    memset(packed, 0xA5, sizeof(packed));
    expect_true("odd start and trailing boundary",
                redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
                    aperture, sizeof(aperture), 1U, packed, sizeof(packed), 1U, 4U));
    expect_byte("leading low nibble preserved high", packed[0], 0xA2);
    expect_byte("middle pair", packed[1], 0x34);
    expect_byte("trailing high nibble preserved low", packed[2], 0x55);

    memcpy(before, packed, sizeof(packed));
    expect_true("out of range rejected",
                !redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
                    aperture, sizeof(aperture), 5U, packed, sizeof(packed), 0U, 2U));
    if (memcmp(before, packed, sizeof(packed)) != 0) {
        fprintf(stderr, "FAIL: invalid input changed destination\n");
        ++failures;
    }
    expect_true("zero count", redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
                    NULL, 0U, 0U, NULL, 0U, 0U, 0U));

    if (strstr(redmcsb_f8143_copy_pixel_line_from_screen_source_evidence_pc34(),
               "VIDEODRV.C:1474-1527") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8143 C25 aperture packed readback");
    return 0;
}
