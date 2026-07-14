#include "redmcsb_f8161_vidrv_blit_viewport_pc34_compat.h"

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
    uint8_t bitmap[224];
    uint8_t aperture_bytes[960];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    RedmcsbF8151BoxPc34Compat box = {2, 5, 1, 2};

    memset(bitmap, 0, sizeof(bitmap));
    bitmap[0] = 0x12U;
    bitmap[1] = 0x34U;
    bitmap[112] = 0x56U;
    bitmap[113] = 0x78U;
    memset(aperture_bytes, 0xEE, sizeof(aperture_bytes));

    expect_true("F8161 viewport blit",
                redmcsb_f8161_vidrv_blit_viewport_pc34_compat(
                    bitmap, sizeof(bitmap), &aperture, &box));
    expect_byte("first source pixel gets viewport bank", aperture_bytes[322], 0x11U);
    expect_byte("second source pixel gets viewport bank", aperture_bytes[323], 0x12U);
    expect_byte("fourth source pixel gets viewport bank", aperture_bytes[325], 0x14U);
    expect_byte("second source row", aperture_bytes[642], 0x15U);
    expect_byte("outside viewport box retained", aperture_bytes[321], 0xEEU);

    box.right = 226;
    expect_true("source-bound validation",
                !redmcsb_f8161_vidrv_blit_viewport_pc34_compat(
                    bitmap, sizeof(bitmap), &aperture, &box));

    if (strstr(redmcsb_f8161_vidrv_blit_viewport_source_evidence_pc34(),
               "VIDEODRV.C:3566-3577") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8161 PC 3.4 C25 viewport blit");
    return 0;
}
