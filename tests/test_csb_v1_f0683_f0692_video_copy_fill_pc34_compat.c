#include "f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat.h"
#include "redmcsb_f0692_fillbox_image3_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_f0683_flips_and_skips_transparent_pixels(void)
{
    const uint8_t source[] = { 0x12u, 0x03u, 0x45u };
    uint8_t destination[] = { 0xaau, 0xaau, 0xaau };
    const uint8_t expected[] = { 0x43u, 0xa2u, 0x1au };

    CHECK(F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(
        source,
        sizeof(source),
        0u,
        destination,
        sizeof(destination),
        0u,
        5u,
        0u) == 1);
    CHECK(memcmp(destination, expected, sizeof(destination)) == 0);
    return 0;
}

static int test_f0683_odd_destination_start_preserves_other_nibbles(void)
{
    const uint8_t source[] = { 0x12u, 0x34u };
    uint8_t destination[] = { 0xabu, 0xcdu, 0xefu };
    const uint8_t expected[] = { 0xa4u, 0x32u, 0x1fu };

    CHECK(F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(
        source,
        sizeof(source),
        0u,
        destination,
        sizeof(destination),
        1u,
        4u,
        0x0fu) == 1);
    CHECK(memcmp(destination, expected, sizeof(destination)) == 0);
    return 0;
}

static int test_f0683_invalid_transparent_color_rejects_without_mutation(void)
{
    const uint8_t source[] = { 0x12u, 0x34u };
    uint8_t destination[] = { 0x55u, 0x66u };
    const uint8_t before[] = { 0x55u, 0x66u };

    CHECK(F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(
        source,
        sizeof(source),
        0u,
        destination,
        sizeof(destination),
        0u,
        4u,
        0x10u) == 0);
    CHECK(memcmp(destination, before, sizeof(destination)) == 0);
    return 0;
}

static int test_f0683_wrapper_matches_compat_entrypoint(void)
{
    const uint8_t source[] = { 0x98u, 0x76u };
    uint8_t wrapper_destination[] = { 0u, 0u };
    uint8_t compat_destination[] = { 0u, 0u };

    CHECK(F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(
        source,
        sizeof(source),
        0u,
        wrapper_destination,
        sizeof(wrapper_destination),
        0u,
        4u,
        0x0fu) == 1);
    CHECK(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
        source,
        sizeof(source),
        0u,
        compat_destination,
        sizeof(compat_destination),
        0u,
        4u,
        0x0fu) == 1);
    CHECK(memcmp(wrapper_destination,
                 compat_destination,
                 sizeof(wrapper_destination)) == 0);
    return 0;
}

static int test_f0692_fills_inclusive_box_low_nibble_color(void)
{
    uint8_t bitmap[] = {
        0x00u, 0x00u,
        0x00u, 0x00u,
        0x00u, 0x00u
    };
    const uint8_t expected[] = {
        0x00u, 0x00u,
        0x07u, 0x77u,
        0x00u, 0x00u
    };
    const int16_t box[4] = { 1, 3, 1, 1 };

    CHECK(F0692_FillBox(bitmap, sizeof(bitmap), 2u, 3u, box, 0x1237u) == 1);
    CHECK(memcmp(bitmap, expected, sizeof(bitmap)) == 0);
    return 0;
}

static int test_f0692_invalid_box_rejects_without_mutation(void)
{
    uint8_t bitmap[] = { 0x12u, 0x34u, 0x56u, 0x78u };
    const uint8_t before[] = { 0x12u, 0x34u, 0x56u, 0x78u };
    const int16_t box[4] = { 3, 1, 0, 0 };

    CHECK(F0692_FillBox(bitmap, sizeof(bitmap), 2u, 2u, box, 0x000fu) == 0);
    CHECK(memcmp(bitmap, before, sizeof(bitmap)) == 0);
    return 0;
}

static int test_source_evidence_names_f0683_and_f0692(void)
{
    const char *f0683 =
        f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence();
    const char *f0692 =
        redmcsb_f0692_fillbox_image3_pc34_compat_source_evidence();

    CHECK(f0683 != 0);
    CHECK(strstr(f0683,
                 "F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally") != 0);
    CHECK(strstr(f0683, "IMAGE3.C:612-829") != 0);
    CHECK(f0692 != 0);
    CHECK(strstr(f0692, "F0692_FillBox") != 0);
    CHECK(strstr(f0692, "CEDT027.C:1124") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_f0683_flips_and_skips_transparent_pixels() == 0);
    CHECK(test_f0683_odd_destination_start_preserves_other_nibbles() == 0);
    CHECK(test_f0683_invalid_transparent_color_rejects_without_mutation() == 0);
    CHECK(test_f0683_wrapper_matches_compat_entrypoint() == 0);
    CHECK(test_f0692_fills_inclusive_box_low_nibble_color() == 0);
    CHECK(test_f0692_invalid_box_rejects_without_mutation() == 0);
    CHECK(test_source_evidence_names_f0683_and_f0692() == 0);
    return 0;
}
