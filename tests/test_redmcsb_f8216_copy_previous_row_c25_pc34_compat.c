#include "redmcsb_f8216_copy_previous_row_c25_pc34_compat.h"

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
    uint8_t aperture[1000];

    memset(aperture, 0xEE, sizeof(aperture));
    aperture[0] = 0x11U;
    aperture[1] = 0x22U;
    aperture[2] = 0x33U;
    expect_true("previous row copy",
                redmcsb_f8216_copy_previous_row_c25_pc34_compat(
                    aperture, sizeof(aperture), 320, 3));
    expect_byte("first copied byte", aperture[320], 0x11U);
    expect_byte("second copied byte", aperture[321], 0x22U);
    expect_byte("third copied byte", aperture[322], 0x33U);

    /* Forward movs propagation when a >320-byte span overlaps its source. */
    memset(aperture, 0xEE, sizeof(aperture));
    aperture[0] = 0x51U;
    aperture[320] = 0xA2U;
    expect_true("forward overlap copy",
                redmcsb_f8216_copy_previous_row_c25_pc34_compat(
                    aperture, sizeof(aperture), 320, 321));
    expect_byte("overlap initial destination", aperture[320], 0x51U);
    expect_byte("overlap propagated byte", aperture[640], 0x51U);

    aperture[320] = 0x77U;
    expect_true("underflow rejected",
                !redmcsb_f8216_copy_previous_row_c25_pc34_compat(
                    aperture, sizeof(aperture), 319, 1));
    expect_byte("rejected copy leaves destination", aperture[320], 0x77U);

    if (strstr(redmcsb_f8216_copy_previous_row_source_evidence_pc34(),
               "VIDEODRV.C:1450-1473") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8216 PC 3.4 C25 previous-row aperture copy");
    return 0;
}
