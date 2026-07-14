#include "redmcsb_f8213_set_single_pixel_c25_pc34_compat.h"

bool redmcsb_f8213_set_single_pixel_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count, int16_t pixel_index,
    uint8_t color, uint8_t viewport_color_index_offset)
{
    size_t index;

    if (aperture == NULL || pixel_index < 0) {
        return false;
    }
    index = (size_t)pixel_index;
    if (index >= aperture_byte_count) {
        return false;
    }

    /* VIDEODRV.C:1052-1062: AL receives G8177, ORs colour, then stosb. */
    aperture[index] = (uint8_t)(viewport_color_index_offset | color);
    return true;
}

const char *redmcsb_f8213_set_single_pixel_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:1052-1062 C25 F8213: add pixel index to "
           "G8134, OR G8177 viewport colour offset with the colour, and "
           "store one aperture byte.";
}
