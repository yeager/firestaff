#include "f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    const uint8_t source[] = {0xa5u, 0x20u, 0x7eu};
    const uint8_t source_with_f[] = {0xfau, 0x31u};
    uint8_t destination[4];
    uint8_t before[4];
    int ok = 1;

    memcpy(destination, (const uint8_t[]){0xbcu, 0xdeu, 0xf1u, 0x23u}, sizeof(destination));
    ok &= check(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, sizeof(source), 0u, destination, sizeof(destination), 1u, 6u,
                    0u) == 1 &&
                    memcmp(destination, (const uint8_t[]){0xbeu, 0x7eu, 0x25u, 0xa3u},
                           sizeof(destination)) == 0,
                "reverses a packed even-source span into an odd destination span and preserves transparent nibbles");

    memcpy(destination, (const uint8_t[]){0x12u, 0x34u, 0x56u, 0x78u}, sizeof(destination));
    ok &= check(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, sizeof(source), 1u, destination, sizeof(destination), 0u, 4u,
                    0u) == 1 &&
                    memcmp(destination, (const uint8_t[]){0x72u, 0x25u, 0x56u, 0x78u},
                           sizeof(destination)) == 0,
                "handles odd source alignment and an odd count without disturbing pixels outside the span");

    memcpy(destination, (const uint8_t[]){0x4cu, 0x8du, 0x21u, 0x43u}, sizeof(destination));
    ok &= check(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source_with_f, sizeof(source_with_f), 0u, destination, sizeof(destination),
                    0u, 4u, 0x0fu) == 1 &&
                    memcmp(destination, (const uint8_t[]){0x13u, 0xadu, 0x21u, 0x43u},
                           sizeof(destination)) == 0,
                "uses the supplied nonzero transparent color for either nibble of a packed byte");

    memset(destination, 0xa5, sizeof(destination));
    memcpy(before, destination, sizeof(destination));
    ok &= check(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, 2u, 0u, destination, sizeof(destination), 0u, 5u, 0u) == 0 &&
                    f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, sizeof(source), 0u, destination, 2u, 0u, 5u, 0u) == 0 &&
                    f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, sizeof(source), 0u, destination, sizeof(destination), 0u, 1u,
                    0x10u) == 0 &&
                    memcmp(destination, before, sizeof(destination)) == 0,
                "rejects incomplete ranges and colors beyond the original lookup table without mutation");
    ok &= check(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    NULL, sizeof(source), 0u, destination, sizeof(destination), 0u, 1u, 0u) ==
                    0 &&
                    f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
                    source, sizeof(source), 0u, NULL, sizeof(destination), 0u, 1u, 0u) == 0,
                "rejects missing caller-owned bitmaps");
    ok &= check(strstr(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence(),
                       "IMAGE3.C:612-829") != NULL &&
                    strstr(f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence(),
                           "I34E PC 3.4") != NULL,
                "source evidence identifies the exact PC 3.4 function body and branch");

    if (!ok) {
        return 1;
    }
    puts("PASS f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat");
    return 0;
}
