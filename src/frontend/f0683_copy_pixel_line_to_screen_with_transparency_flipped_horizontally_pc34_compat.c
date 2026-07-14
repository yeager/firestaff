#include "f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat.h"

static uint8_t f0683_packed_pixel(const uint8_t *bitmap, size_t pixel_index)
{
    const uint8_t packed = bitmap[pixel_index / 2u];

    return (pixel_index & 1u) == 0u ? (uint8_t)(packed >> 4) :
                                        (uint8_t)(packed & 0x0fu);
}

static void f0683_set_packed_pixel(uint8_t *bitmap, size_t pixel_index, uint8_t pixel)
{
    uint8_t *packed = bitmap + pixel_index / 2u;

    if ((pixel_index & 1u) == 0u) {
        *packed = (uint8_t)((*packed & 0x0fu) | (uint8_t)(pixel << 4));
    } else {
        *packed = (uint8_t)((*packed & 0xf0u) | pixel);
    }
}

int f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color)
{
    size_t source_end;
    size_t destination_end;
    size_t pixel_offset;

    if (source == NULL || destination == NULL || transparent_color > 0x0fu ||
        source_pixel_index > SIZE_MAX - pixel_count ||
        destination_pixel_index > SIZE_MAX - pixel_count) {
        return 0;
    }

    source_end = source_pixel_index + pixel_count;
    destination_end = destination_pixel_index + pixel_count;
    if (source_bytes > SIZE_MAX / 2u || destination_bytes > SIZE_MAX / 2u ||
        source_end > source_bytes * 2u ||
        destination_end > destination_bytes * 2u) {
        return 0;
    }

    for (pixel_offset = 0u; pixel_offset < pixel_count; ++pixel_offset) {
        const uint8_t source_pixel = f0683_packed_pixel(
            source, source_pixel_index + pixel_count - pixel_offset - 1u);

        if (source_pixel != transparent_color) {
            f0683_set_packed_pixel(
                destination, destination_pixel_index + pixel_offset, source_pixel);
        }
    }

    return 1;
}

const char *f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/IMAGE3.C:612-829 "
           "F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally; "
           "MEDIA463_P20JA_P20JB_I34E_I34M_P31J / I34E PC 3.4; "
           "F0684_Blit:904-924";
}
