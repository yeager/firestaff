#include "redmcsb_f8165_prefixed_bitmap_c25_pc34_compat.h"

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

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %zu, expected %zu)\n", name, actual, expected);
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
    uint8_t aperture_bytes[960];
    uint8_t output[12];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    size_t byte_count = 0U;

    memset(aperture_bytes, 0, sizeof(aperture_bytes));
    aperture_bytes[322] = 0x31U;
    aperture_bytes[323] = 0x32U;
    aperture_bytes[324] = 0x33U;
    aperture_bytes[642] = 0x41U;
    aperture_bytes[643] = 0x42U;
    aperture_bytes[644] = 0x43U;

    expect_true("operation zero reports size",
                redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
                    NULL, 2U, 1, 3, 2U, 0, NULL, 0U, &byte_count));
    expect_size("operation zero byte count", byte_count, 12U);

    memset(output, 0xEE, sizeof(output));
    byte_count = 77U;
    expect_true("capture aperture rectangle",
                redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
                    &aperture, 2U, 1, 3, 2U, 1, output, sizeof(output),
                    &byte_count));
    expect_size("capture leaves source-undefined result", byte_count, 77U);
    expect_byte("header width lo", output[0], 3U);
    expect_byte("header height lo", output[2], 2U);
    expect_byte("header source offset lo", output[4], 0x42U);
    expect_byte("header source offset hi", output[5], 0x01U);
    expect_byte("first row first pixel", output[6], 0x31U);
    expect_byte("first row last pixel", output[8], 0x33U);
    expect_byte("second row first pixel", output[9], 0x41U);
    expect_byte("second row last pixel", output[11], 0x43U);

    memset(output, 0xEE, sizeof(output));
    expect_true("insufficient output rejected",
                !redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
                    &aperture, 2U, 1, 3, 2U, 1, output, 11U, &byte_count));
    expect_byte("rejected capture leaves output", output[0], 0xEEU);

    if (strstr(redmcsb_f8165_prefixed_bitmap_source_evidence_pc34(),
               "VIDEODRV.C:3657-3729") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8165 PC 3.4 C25 prefixed aperture bitmap");
    return 0;
}
