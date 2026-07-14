#include "redmcsb_f0682_copy_pixel_line_transparent_pc34_compat.h"

static bool redmcsb_f0682_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

bool redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0682C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color,
    uint8_t viewport_color_index_offset)
{
    size_t source_pixel_capacity;
    size_t pixel_index;

    if (source == NULL || vga_aperture == NULL || vga_aperture->bytes == NULL ||
        source_byte_count > SIZE_MAX / 2U) {
        return false;
    }

    source_pixel_capacity = source_byte_count * 2U;
    if (!redmcsb_f0682_range_fits(source_pixel_index, pixel_count,
                                   source_pixel_capacity) ||
        !redmcsb_f0682_range_fits(destination_pixel_index, pixel_count,
                                   vga_aperture->byte_count)) {
        return false;
    }

    for (pixel_index = 0U; pixel_index < pixel_count; ++pixel_index) {
        const size_t source_index = source_pixel_index + pixel_index;
        const uint8_t packed = source[source_index >> 1U];
        const uint8_t pixel = (source_index & 1U) == 0U
                                  ? (uint8_t)(packed >> 4U)
                                  : (uint8_t)(packed & 0x0FU);

        if (pixel != transparent_color) {
            vga_aperture->bytes[destination_pixel_index + pixel_index] =
                (uint8_t)(viewport_color_index_offset | pixel);
        }
    }

    return true;
}

const char *redmcsb_f0682_copy_pixel_line_transparent_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:2377-2460 "
           "F0682_CopyPixelLineToScreenWithTransparency C25_VGA: packed "
           "source nibbles are read high-first, values equal to P2328 are "
           "not written, and every other value is ORed with "
           "G8177_c_ViewportColorIndexOffset before the A000h byte write.";
}
