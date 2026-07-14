#include "redmcsb_f0681_copy_pixel_line_flipped_pc34_compat.h"

#include <stdint.h>

static bool redmcsb_f0681_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

bool redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset)
{
    size_t source_pixel_capacity;
    size_t destination_offset;

    if (source == NULL || vga_aperture == NULL || vga_aperture->bytes == NULL ||
        (viewport_color_index_offset & 0x0FU) != 0U ||
        source_byte_count > SIZE_MAX / 2U) {
        return false;
    }

    source_pixel_capacity = source_byte_count * 2U;
    if (!redmcsb_f0681_range_fits(source_pixel_index, pixel_count,
                                   source_pixel_capacity) ||
        !redmcsb_f0681_range_fits(destination_pixel_index, pixel_count,
                                   vga_aperture->byte_count)) {
        return false;
    }

    for (destination_offset = 0U; destination_offset < pixel_count;
         ++destination_offset) {
        const size_t source_index = source_pixel_index +
            (pixel_count - 1U - destination_offset);
        const uint8_t packed_pixel = source[source_index >> 1U];
        const uint8_t source_nibble = (source_index & 1U) == 0U
            ? (uint8_t)(packed_pixel >> 4U)
            : (uint8_t)(packed_pixel & 0x0FU);

        vga_aperture->bytes[destination_pixel_index + destination_offset] =
            (uint8_t)(viewport_color_index_offset | source_nibble);
    }

    return true;
}

const char *redmcsb_f0681_copy_pixel_line_flipped_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE3.C:7-176 F0681 starts at source + count - 1 and "
           "writes the destination forward without a transparency branch; "
           "IMAGE5.C:936-1010 C25_VGA defines the coupled packed-nibble "
           "source and A000h byte-aperture representation used here.";
}
