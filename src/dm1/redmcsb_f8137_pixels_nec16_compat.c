#include "redmcsb_f8137_pixels_nec16_compat.h"

void redmcsb_f8137_set_multiple_pixels_nec16_compat(
    uint8_t *video, size_t bytes, uint16_t pixel, uint8_t color,
    uint16_t count)
{
    size_t index;

    if (video == 0) return;
    color &= 0x0fU;
    for (index = 0U; index < count; ++index) {
        size_t current_pixel = (size_t)pixel + index;
        size_t byte_index = current_pixel / 2U;

        if (byte_index >= bytes) break;
        if ((current_pixel & 1U) != 0U) {
            video[byte_index] = (uint8_t)((video[byte_index] & 0xf0U) | color);
        } else {
            video[byte_index] = (uint8_t)((video[byte_index] & 0x0fU) | (color << 4));
        }
    }
}

const char *redmcsb_f8137_pixels_nec16_source_evidence(void)
{
    return "ReDMCSB NEC816.C:1804-1853; MEDIA457_P20JA";
}
