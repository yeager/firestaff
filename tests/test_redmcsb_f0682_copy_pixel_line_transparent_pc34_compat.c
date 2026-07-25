#include "redmcsb_f0682_copy_pixel_line_transparent_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_transparent_pixels_preserve_destination(void)
{
    const uint8_t source[] = {0xA3U, 0x0AU, 0x5FU};
    (void)source;
    uint8_t aperture_bytes[] = {0xD0U, 0xD1U, 0xD2U, 0xD3U, 0xD4U, 0xD5U};
    const uint8_t expected[] = {0xD0U, 0x1AU, 0x13U, 0xD3U, 0x1AU, 0xD5U};
    (void)expected;
    RedmcsbF0682C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
        source, sizeof(source), 0U, &aperture, 1U, 4U, 0U, 0x10U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_odd_source_index_and_odd_pixel_count(void)
{
    const uint8_t source[] = {0x1EU, 0x4EU, 0x70U};
    (void)source;
    uint8_t aperture_bytes[] = {0x41U, 0x42U, 0x43U, 0x44U, 0x45U};
    const uint8_t expected[] = {0x41U, 0x04U, 0x43U, 0x07U, 0x00U};
    (void)expected;
    RedmcsbF0682C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
        source, sizeof(source), 1U, &aperture, 0U, 5U, 0xEU, 0U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_color_offset_uses_raw_or_semantics(void)
{
    const uint8_t source[] = {0x2FU};
    (void)source;
    uint8_t aperture_bytes[] = {0x99U, 0x88U};
    RedmcsbF0682C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
        source, sizeof(source), 0U, &aperture, 0U, 2U, 0xFU, 0x21U));
    assert(aperture_bytes[0] == 0x23U);
    assert(aperture_bytes[1] == 0x88U);
}

static void test_invalid_range_does_not_mutate_destination(void)
{
    const uint8_t source[] = {0x12U};
    (void)source;
    uint8_t aperture_bytes[] = {0xA5U, 0xA5U};
    const uint8_t unchanged[] = {0xA5U, 0xA5U};
    (void)unchanged;
    RedmcsbF0682C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(!redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
        source, sizeof(source), 1U, &aperture, 1U, 2U, 0U, 0x10U));
    assert(memcmp(aperture_bytes, unchanged, sizeof(unchanged)) == 0);
}

int main(void)
{
    test_transparent_pixels_preserve_destination();
    test_odd_source_index_and_odd_pixel_count();
    test_color_offset_uses_raw_or_semantics();
    test_invalid_range_does_not_mutate_destination();
    assert(strstr(redmcsb_f0682_copy_pixel_line_transparent_source_evidence_pc34(),
                  "VIDEODRV.C:2377-2460") != NULL);
    puts("PASS redmcsb_f0682_copy_pixel_line_transparent_pc34_compat");
    return 0;
}
