#include "redmcsb_start_bitmap_size_pc34_compat.h"

int16_t F0459_START_GetScaledBitmapByteCount_PC34(
    int16_t pixel_width,
    int16_t pixel_height,
    int16_t scale) {
    int32_t scaled_width;
    int32_t scaled_height;
    int32_t byte_count;

    if (pixel_width <= 0 || pixel_height <= 0 || scale < 0) return 0;

    /* BMPSIZE.C: M078_SCALED_DIMENSION followed by the I34 M103 layout. */
    scaled_width = (((int32_t)pixel_width * scale) + (scale >> 1)) >> 5;
    scaled_height = (((int32_t)pixel_height * scale) + (scale >> 1)) >> 5;
    byte_count = (((scaled_width + 1) & ~1) >> 1) * scaled_height;

    return (int16_t)byte_count;
}
