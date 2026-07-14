#include "f0680_copy_pixels_to_screen_without_transparency_pc34_compat.h"

int f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count)
{
    size_t source_end;
    size_t destination_end;
    size_t pixel_offset;

    if (source == NULL || destination == NULL ||
        source_pixel_index > SIZE_MAX - pixel_count ||
        destination_pixel_index > SIZE_MAX - pixel_count) {
        return 0;
    }

    source_end = source_pixel_index + pixel_count;
    destination_end = destination_pixel_index + pixel_count;
    if (source_bytes > SIZE_MAX / 2u || source_end > source_bytes * 2u ||
        destination_end > destination_bytes) {
        return 0;
    }

    for (pixel_offset = 0u; pixel_offset < pixel_count; ++pixel_offset) {
        const size_t source_pixel = source_pixel_index + pixel_offset;
        const uint8_t packed = source[source_pixel / 2u];

        destination[destination_pixel_index + pixel_offset] =
            (source_pixel & 1u) == 0u ? (uint8_t)(packed >> 4) :
                                         (uint8_t)(packed & 0x0fu);
    }

    return 1;
}

const char *f0680_copy_pixels_to_screen_without_transparency_pc34_compat_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/ANIMIMG.C:269 "
           "F0680_CopyPixelsToScreenWithoutTransparency; IMAGE5.C:5";
}
