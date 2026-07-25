#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_even_source_and_count(void)
{
    const uint8_t source[] = {0x12U, 0xA0U, 0xFEU};
    (void)source;
    uint8_t aperture_bytes[] = {0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU};
    const uint8_t expected[] = {0xCCU, 0x11U, 0x12U, 0x1AU, 0x10U, 0xCCU};
    (void)expected;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 0U, &aperture, 1U, 4U, 0x10U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_odd_source_and_odd_count(void)
{
    const uint8_t source[] = {0xABU, 0xCDU, 0xE0U};
    (void)source;
    uint8_t aperture_bytes[] = {0x77U, 0x77U, 0x77U, 0x77U, 0x77U};
    const uint8_t expected[] = {0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x00U};
    (void)expected;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 1U, &aperture, 0U, 5U, 0x00U));
    assert(memcmp(aperture_bytes, expected, sizeof(expected)) == 0);
}

static void test_single_pixel_at_each_packed_boundary(void)
{
    const uint8_t source[] = {0x3CU};
    (void)source;
    uint8_t aperture_bytes[] = {0xEEU, 0xEEU};
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 0U, &aperture, 1U, 1U, 0x10U));
    assert(aperture_bytes[0] == 0xEEU);
    assert(aperture_bytes[1] == 0x13U);

    assert(redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 1U, &aperture, 0U, 1U, 0x10U));
    assert(aperture_bytes[0] == 0x1CU);
}

static void test_rejection_is_atomic(void)
{
    const uint8_t source[] = {0x12U};
    (void)source;
    uint8_t aperture_bytes[] = {0xA5U, 0xA5U};
    const uint8_t unchanged[] = {0xA5U, 0xA5U};
    (void)unchanged;
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        aperture_bytes, sizeof(aperture_bytes)};
    (void)aperture;

    assert(!redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 1U, &aperture, 1U, 2U, 0x10U));
    assert(memcmp(aperture_bytes, unchanged, sizeof(unchanged)) == 0);
    assert(!redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source, sizeof(source), 0U, &aperture, 0U, 1U, 0x11U));
    assert(memcmp(aperture_bytes, unchanged, sizeof(unchanged)) == 0);
}

int main(void)
{
    test_even_source_and_count();
    test_odd_source_and_odd_count();
    test_single_pixel_at_each_packed_boundary();
    test_rejection_is_atomic();
    assert(strstr(redmcsb_f0680_copy_pixels_to_screen_source_evidence_pc34(),
                  "IMAGE5.C:936-1010") != NULL);
    puts("PASS redmcsb_f0680_copy_pixels_to_screen_pc34_compat");
    return 0;
}
