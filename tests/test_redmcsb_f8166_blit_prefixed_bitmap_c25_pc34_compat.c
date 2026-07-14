#include "redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat.h"

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
    const uint8_t prefixed_bitmap[] = {
        3U, 0U, 2U, 0U, 0x42U, 0x01U,
        0x31U, 0x32U, 0x33U, 0x41U, 0x42U, 0x43U};
    uint8_t aperture_bytes[960];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};

    memset(aperture_bytes, 0xEE, sizeof(aperture_bytes));
    expect_true("F8166 prefixed playback",
                redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat(
                    prefixed_bitmap, sizeof(prefixed_bitmap), &aperture));
    expect_byte("first row first pixel", aperture_bytes[322], 0x31U);
    expect_byte("first row last pixel", aperture_bytes[324], 0x33U);
    expect_byte("second row first pixel", aperture_bytes[642], 0x41U);
    expect_byte("second row last pixel", aperture_bytes[644], 0x43U);
    expect_byte("outside payload retained", aperture_bytes[321], 0xEEU);

    aperture_bytes[322] = 0xAAU;
    expect_true("truncated payload rejected",
                !redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat(
                    prefixed_bitmap, sizeof(prefixed_bitmap) - 1U, &aperture));
    expect_byte("rejected payload leaves aperture", aperture_bytes[322], 0xAAU);

    if (strstr(redmcsb_f8166_blit_prefixed_bitmap_source_evidence_pc34(),
               "VIDEODRV.C:3731-3802") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8166 PC 3.4 C25 prefixed aperture playback");
    return 0;
}
