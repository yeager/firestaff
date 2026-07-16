#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_source_named_wrapper_expands_packed_nibbles(void)
{
    const uint8_t source[] = { 0x12u, 0x30u, 0xabu };
    uint8_t destination[8] = {
        0xeeu, 0xeeu, 0xeeu, 0xeeu,
        0xeeu, 0xeeu, 0xeeu, 0xeeu
    };
    const uint8_t expected[8] = {
        0xeeu, 0x11u, 0x12u, 0x13u,
        0x10u, 0xeeu, 0xeeu, 0xeeu
    };
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        destination,
        sizeof(destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        0u,
        &aperture,
        1u,
        4u,
        0x10u) == true);
    CHECK(memcmp(destination, expected, sizeof(destination)) == 0);
    return 0;
}

static int test_odd_source_pixel_start_uses_low_nibble_first(void)
{
    const uint8_t source[] = { 0x12u, 0x34u };
    uint8_t destination[4] = { 0u, 0u, 0u, 0u };
    const uint8_t expected[4] = { 0x02u, 0x03u, 0x04u, 0u };
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        destination,
        sizeof(destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        1u,
        &aperture,
        0u,
        3u,
        0x00u) == true);
    CHECK(memcmp(destination, expected, sizeof(destination)) == 0);
    return 0;
}

static int test_invalid_color_offset_rejects_without_mutation(void)
{
    const uint8_t source[] = { 0x12u };
    uint8_t destination[2] = { 0x44u, 0x55u };
    const uint8_t before[2] = { 0x44u, 0x55u };
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        destination,
        sizeof(destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        0u,
        &aperture,
        0u,
        2u,
        0x11u) == false);
    CHECK(memcmp(destination, before, sizeof(destination)) == 0);
    return 0;
}

static int test_source_bounds_reject_without_mutation(void)
{
    const uint8_t source[] = { 0x12u };
    uint8_t destination[3] = { 0x66u, 0x77u, 0x88u };
    const uint8_t before[3] = { 0x66u, 0x77u, 0x88u };
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        destination,
        sizeof(destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        1u,
        &aperture,
        0u,
        2u,
        0x00u) == false);
    CHECK(memcmp(destination, before, sizeof(destination)) == 0);
    return 0;
}

static int test_destination_bounds_reject_without_mutation(void)
{
    const uint8_t source[] = { 0x12u, 0x34u };
    uint8_t destination[3] = { 0x99u, 0xaau, 0xbbu };
    const uint8_t before[3] = { 0x99u, 0xaau, 0xbbu };
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {
        destination,
        sizeof(destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        0u,
        &aperture,
        2u,
        2u,
        0x00u) == false);
    CHECK(memcmp(destination, before, sizeof(destination)) == 0);
    return 0;
}

static int test_wrapper_matches_compat_entrypoint(void)
{
    const uint8_t source[] = { 0x45u, 0x60u };
    uint8_t wrapper_destination[4] = { 0u, 0u, 0u, 0u };
    uint8_t compat_destination[4] = { 0u, 0u, 0u, 0u };
    RedmcsbF0680C25VgaAperturePc34Compat wrapper_aperture = {
        wrapper_destination,
        sizeof(wrapper_destination)
    };
    RedmcsbF0680C25VgaAperturePc34Compat compat_aperture = {
        compat_destination,
        sizeof(compat_destination)
    };

    CHECK(F0680_CopyPixelsToScreenWithoutTransparency(
        source,
        sizeof(source),
        0u,
        &wrapper_aperture,
        1u,
        3u,
        0x10u) == true);
    CHECK(redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source,
        sizeof(source),
        0u,
        &compat_aperture,
        1u,
        3u,
        0x10u) == true);
    CHECK(memcmp(wrapper_destination,
                 compat_destination,
                 sizeof(wrapper_destination)) == 0);
    return 0;
}

static int test_source_evidence_names_f0680(void)
{
    const char *evidence =
        redmcsb_f0680_copy_pixels_to_screen_source_evidence_pc34();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0680_CopyPixelsToScreenWithoutTransparency") != 0);
    CHECK(strstr(evidence, "ANIMIMG.C:269") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_expands_packed_nibbles() == 0);
    CHECK(test_odd_source_pixel_start_uses_low_nibble_first() == 0);
    CHECK(test_invalid_color_offset_rejects_without_mutation() == 0);
    CHECK(test_source_bounds_reject_without_mutation() == 0);
    CHECK(test_destination_bounds_reject_without_mutation() == 0);
    CHECK(test_wrapper_matches_compat_entrypoint() == 0);
    CHECK(test_source_evidence_names_f0680() == 0);
    return 0;
}
