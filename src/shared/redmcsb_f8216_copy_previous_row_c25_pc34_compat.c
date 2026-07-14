#include "redmcsb_f8216_copy_previous_row_c25_pc34_compat.h"

static bool redmcsb_f8216_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

bool redmcsb_f8216_copy_previous_row_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count,
    int16_t destination_offset, int16_t pixel_count)
{
    size_t destination;
    size_t count;
    size_t pixel;

    if (aperture == NULL || destination_offset < (int16_t)REDMCSB_F8216_SCREEN_STRIDE_PC34 ||
        pixel_count < 0) {
        return false;
    }
    destination = (size_t)destination_offset;
    count = (size_t)pixel_count;
    if (!redmcsb_f8216_range_fits(destination, count, aperture_byte_count) ||
        !redmcsb_f8216_range_fits(
            destination - REDMCSB_F8216_SCREEN_STRIDE_PC34, count,
            aperture_byte_count)) {
        return false;
    }

    /* VIDEODRV.C:1454-1472 uses cld/movs forward, not overlap-safe memmove. */
    for (pixel = 0U; pixel < count; ++pixel) {
        aperture[destination + pixel] =
            aperture[destination - REDMCSB_F8216_SCREEN_STRIDE_PC34 + pixel];
    }
    return true;
}

const char *redmcsb_f8216_copy_previous_row_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:1450-1473 C25 F8216: destination starts at "
           "P3200, source is destination-320, and cld/movs copies P3201 bytes "
           "forward in byte/word units.";
}
