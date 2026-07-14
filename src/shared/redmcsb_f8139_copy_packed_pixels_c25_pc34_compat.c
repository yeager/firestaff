#include "redmcsb_f8139_copy_packed_pixels_c25_pc34_compat.h"

static bool redmcsb_f8139_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

bool redmcsb_f8139_copy_packed_pixels_c25_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    uint16_t source_pixel_index, uint8_t *aperture,
    size_t aperture_byte_count, uint16_t destination_pixel_index,
    uint16_t pixel_count, uint8_t viewport_color_index_offset)
{
    const size_t source_start = (size_t)source_pixel_index;
    const size_t destination_start = (size_t)destination_pixel_index;
    const size_t count = (size_t)pixel_count;
    size_t pixel;

    if (source == NULL || aperture == NULL || source_byte_count > SIZE_MAX / 2U ||
        !redmcsb_f8139_range_fits(source_start, count, source_byte_count * 2U) ||
        !redmcsb_f8139_range_fits(destination_start, count, aperture_byte_count)) {
        return false;
    }

    /* VIDEODRV.C:1261-1276 advances one output byte per packed source pixel. */
    for (pixel = 0U; pixel < count; ++pixel) {
        const size_t source_index = source_start + pixel;
        const uint8_t packed = source[source_index >> 1U];
        const uint8_t nibble = (source_index & 1U) != 0U
                                   ? (uint8_t)(packed & 0x0FU)
                                   : (uint8_t)(packed >> 4U);

        aperture[destination_start + pixel] =
            (uint8_t)(viewport_color_index_offset | nibble);
    }
    return true;
}

const char *redmcsb_f8139_copy_packed_pixels_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:1224-1280 C25 F8139: start at G2159 plus "
           "source_pixel_index/2, emit high then low packed nibbles according "
           "to source parity, OR each with G8177, and advance G8134 one byte.";
}
