#include "redmcsb_f8167_f8168_mouse_pointer_c25_pc34_compat.h"

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
    uint8_t aperture_bytes[64000];
    uint8_t prefixed_bitmap[REDMCSB_F8165_C25_HEADER_BYTES_PC34 + 18U * 18U];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    size_t row;

    memset(aperture_bytes, 0xEE, sizeof(aperture_bytes));
    for (row = 0U; row < 5U; ++row) {
        aperture_bytes[(195U + row) * 320U + 310U] = (uint8_t)(0x30U + row);
        aperture_bytes[(195U + row) * 320U + 319U] = (uint8_t)(0x70U + row);
    }
    expect_true("F8167 captures clamped cursor rectangle",
                redmcsb_f8167_capture_mouse_pointer_c25_pc34_compat(
                    &aperture, 310, 195, prefixed_bitmap,
                    sizeof(prefixed_bitmap)));
    expect_byte("prefix width", prefixed_bitmap[0], 10U);
    expect_byte("prefix height", prefixed_bitmap[2], 5U);
    expect_byte("captured first pixel", prefixed_bitmap[6], 0x30U);
    expect_byte("captured last row last pixel", prefixed_bitmap[55], 0x74U);

    memset(aperture_bytes, 0x00, sizeof(aperture_bytes));
    expect_true("F8168 restores cursor rectangle",
                redmcsb_f8168_restore_mouse_pointer_c25_pc34_compat(
                    prefixed_bitmap, 56U, &aperture));
    expect_byte("restored first pixel", aperture_bytes[195U * 320U + 310U], 0x30U);
    expect_byte("restored last row last pixel", aperture_bytes[199U * 320U + 319U], 0x74U);
    expect_byte("outside restore retained", aperture_bytes[195U * 320U + 309U], 0U);

    expect_true("negative coordinate rejected",
                !redmcsb_f8167_capture_mouse_pointer_c25_pc34_compat(
                    &aperture, -1, 0, prefixed_bitmap, sizeof(prefixed_bitmap)));
    if (strstr(redmcsb_f8167_f8168_mouse_pointer_source_evidence_pc34(),
               "VIDEODRV.C:3804-3835") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8167/F8168 PC 3.4 C25 mouse pointer snapshot");
    return 0;
}
