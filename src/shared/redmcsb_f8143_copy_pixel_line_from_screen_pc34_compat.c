#include "redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat.h"

#include <stdint.h>

static bool redmcsb_f8143_range_fits(size_t offset, size_t count, size_t limit)
{
    return offset <= limit && count <= limit - offset;
}

bool redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
    const uint8_t *aperture, size_t aperture_byte_count,
    size_t source_pixel_index, uint8_t *destination,
    size_t destination_byte_count, size_t destination_pixel_index,
    size_t pixel_count)
{
    size_t last_destination_pixel;
    size_t index;

    if (pixel_count == 0U) {
        return true;
    }
    if (aperture == NULL || destination == NULL ||
        !redmcsb_f8143_range_fits(source_pixel_index, pixel_count, aperture_byte_count) ||
        destination_pixel_index > SIZE_MAX - (pixel_count - 1U)) {
        return false;
    }

    last_destination_pixel = destination_pixel_index + pixel_count - 1U;
    if ((last_destination_pixel >> 1U) >= destination_byte_count) {
        return false;
    }

    /*
     * VIDEODRV.C:1491-1524 writes an odd leading pixel, complete pairs, then
     * an odd trailing pixel. This pixel form is equivalent and retains the
     * untouched opposite nibble at both boundaries.
     */
    for (index = 0U; index < pixel_count; ++index) {
        size_t packed_pixel = destination_pixel_index + index;
        size_t packed_byte = packed_pixel >> 1U;
        uint8_t color = (uint8_t)(aperture[source_pixel_index + index] & 0x0FU);

        if ((packed_pixel & 1U) == 0U) {
            destination[packed_byte] =
                (uint8_t)((destination[packed_byte] & 0x0FU) | (color << 4U));
        } else {
            destination[packed_byte] =
                (uint8_t)((destination[packed_byte] & 0xF0U) | color);
        }
    }
    return true;
}

const char *redmcsb_f8143_copy_pixel_line_from_screen_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:1474-1527 C25_VGA F8143: aperture bytes are "
           "read low-nibble only, then packed high-first while retaining the "
           "unwritten boundary nibble.";
}
