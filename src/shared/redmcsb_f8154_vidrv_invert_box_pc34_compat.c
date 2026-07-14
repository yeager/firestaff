#include "redmcsb_f8154_vidrv_invert_box_pc34_compat.h"

static bool redmcsb_f8154_range_fits(size_t offset, size_t count, size_t limit)
{
    return offset <= limit && count <= limit - offset;
}

bool redmcsb_f8154_vidrv_invert_box_pc34_compat(
    RedmcsbF8154C25VgaAperturePc34Compat *aperture,
    int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    int32_t width;
    int32_t row;

    if (aperture == NULL || aperture->bytes == NULL || x1 < 0 || y1 < 0) {
        return false;
    }
    /* The original's inclusive loops make reversed endpoints a no-op. */
    if (x2 < x1 || y2 < y1) {
        return true;
    }
    if (x2 >= (int16_t)REDMCSB_F8154_SCREEN_STRIDE_PIXELS_PC34) {
        return false;
    }
    width = (int32_t)x2 - (int32_t)x1 + 1;
    if (!redmcsb_f8154_range_fits(
            ((size_t)y2 * REDMCSB_F8154_SCREEN_STRIDE_PIXELS_PC34) + (size_t)x1,
            (size_t)width, aperture->byte_count)) {
        return false;
    }

    for (row = (int32_t)y1; row <= (int32_t)y2; ++row) {
        size_t offset = ((size_t)row * REDMCSB_F8154_SCREEN_STRIDE_PIXELS_PC34) +
                        (size_t)x1;
        size_t index;
        for (index = 0U; index < (size_t)width; ++index) {
            aperture->bytes[offset + index] ^= 0x04U;
        }
    }
    return true;
}

const char *redmcsb_f8154_vidrv_invert_box_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3187-3241 C25_VGA F8154: for each inclusive "
           "row y1..y2, A000h[(row*320)+x1..x2] is XORed with 0x04.";
}
