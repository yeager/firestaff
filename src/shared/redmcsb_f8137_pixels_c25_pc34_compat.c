#include "redmcsb_f8137_pixels_c25_pc34_compat.h"

bool redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count, int16_t pixel_index,
    uint8_t color, int16_t pixel_count, uint8_t viewport_color_index_offset)
{
    size_t start;
    size_t count;
    size_t pixel;
    uint8_t aperture_color;

    if (aperture == NULL || pixel_index < 0 || pixel_count < 0) {
        return false;
    }
    start = (size_t)pixel_index;
    count = (size_t)pixel_count;
    if (start > aperture_byte_count || count > aperture_byte_count - start) {
        return false;
    }

    /* VIDEODRV.C:1200-1208 emits the same AL/AH byte for every C25 pixel. */
    aperture_color = (uint8_t)(viewport_color_index_offset | color);
    for (pixel = 0U; pixel < count; ++pixel) {
        aperture[start + pixel] = aperture_color;
    }
    return true;
}

const char *redmcsb_f8137_pixels_c25_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:1065-1210 C25 F8137: add pixel index to "
           "G8134, form G8177 OR color in AL/AH, and stosb/stosw the requested "
           "byte-addressed aperture span.";
}
