#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdint.h>

static uint8_t f0680_read_source_nibble(
    const uint8_t *source,
    size_t source_pixel_index)
{
    const uint8_t packed = source[source_pixel_index / 2u];

    if ((source_pixel_index & 1u) == 0u) {
        return (uint8_t)(packed >> 4);
    }
    return (uint8_t)(packed & 0x0fu);
}

bool redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset)
{
    size_t index;
    size_t source_pixel_count;

    if (source == 0 || vga_aperture == 0 || vga_aperture->bytes == 0 ||
        (viewport_color_index_offset & 0x0fu) != 0u ||
        source_byte_count > (SIZE_MAX / 2u)) {
        return false;
    }
    source_pixel_count = source_byte_count * 2u;
    if (source_pixel_index > source_pixel_count ||
        pixel_count > source_pixel_count - source_pixel_index ||
        destination_pixel_index > vga_aperture->byte_count ||
        pixel_count > vga_aperture->byte_count - destination_pixel_index) {
        return false;
    }

    for (index = 0u; index < pixel_count; index++) {
        const uint8_t source_pixel = f0680_read_source_nibble(
            source,
            source_pixel_index + index);

        vga_aperture->bytes[destination_pixel_index + index] =
            (uint8_t)(viewport_color_index_offset | source_pixel);
    }
    return true;
}

bool F0680_CopyPixelsToScreenWithoutTransparency(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset)
{
    return redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
        source,
        source_byte_count,
        source_pixel_index,
        vga_aperture,
        destination_pixel_index,
        pixel_count,
        viewport_color_index_offset);
}

const char *redmcsb_f0680_copy_pixels_to_screen_source_evidence_pc34(void)
{
    return "ReDMCSB ANIMIMG.C:269 "
           "F0680_CopyPixelsToScreenWithoutTransparency";
}
