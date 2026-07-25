#include "redmcsb_f0681_copy_pixel_line_flipped_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_even_source_reverses_every_pixel(void)
{
    const uint8_t source[] = {0x12U, 0xA0U, 0xFEU};
    (void)source;
    uint8_t aperture_bytes[] = {0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU};
    const uint8_t expected[] = {0xCCU, 0x10U, 0x1AU, 0x12U, 0x11U, 0xCCU};
    (void)expected;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
        source, sizeof(source), 0U, &aperture, 1U, 4U, 0x10U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_odd_source_and_odd_length_preserve_nibble_order(void)
{
    const uint8_t source[] = {0xABU, 0xCDU, 0xE0U};
    (void)source;
    uint8_t aperture_bytes[] = {0x77U, 0x77U, 0x77U, 0x77U, 0x77U};
    const uint8_t expected[] = {0x00U, 0x0EU, 0x0DU, 0x0CU, 0x0BU};
    (void)expected;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
        source, sizeof(source), 1U, &aperture, 0U, 5U, 0x00U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_zero_is_opaque(void)
{
    const uint8_t source[] = {0x90U};
    (void)source;
    uint8_t aperture_bytes[] = {0x77U, 0x77U};
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
        source, sizeof(source), 0U, &aperture, 0U, 2U, 0x10U));
    assert(aperture_bytes[0] == 0x10U);
    assert(aperture_bytes[1] == 0x19U);
}

static void test_invalid_ranges_do_not_write(void)
{
    const uint8_t source[] = {0x12U};
    (void)source;
    uint8_t aperture_bytes[] = {0xA5U, 0xA5U};
    const uint8_t unchanged[] = {0xA5U, 0xA5U};
    (void)unchanged;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(!redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
        source, sizeof(source), 1U, &aperture, 1U, 2U, 0x10U));
    assert(memcmp(aperture_bytes, unchanged, sizeof(unchanged)) == 0);
    assert(!redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
        source, sizeof(source), 0U, &aperture, 0U, 1U, 0x11U));
    assert(memcmp(aperture_bytes, unchanged, sizeof(unchanged)) == 0);
}

int main(void)
{
    test_even_source_reverses_every_pixel();
    test_odd_source_and_odd_length_preserve_nibble_order();
    test_zero_is_opaque();
    test_invalid_ranges_do_not_write();
    assert(strstr(redmcsb_f0681_copy_pixel_line_flipped_source_evidence_pc34(),
                  "IMAGE3.C:7-176") != NULL);
    puts("PASS redmcsb_f0681_copy_pixel_line_flipped_pc34_compat");
    return 0;
}
