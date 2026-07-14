#include "redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat.h"

static bool redmcsb_f8155_range_fits(size_t offset, size_t count, size_t limit)
{
    return offset <= limit && count <= limit - offset;
}

bool redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat(
    RedmcsbF8155C25VgaAperturePc34Compat *aperture,
    int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    int32_t width;
    int32_t row;

    if (aperture == NULL || aperture->bytes == NULL || x1 < 0 || y1 < 0) {
        return false;
    }
    /* The source's inclusive loops have no iterations for reversed bounds. */
    if (x2 < x1 || y2 < y1) {
        return true;
    }
    if (x2 >= (int16_t)REDMCSB_F8155_SCREEN_STRIDE_PIXELS_PC34) {
        return false;
    }
    width = (int32_t)x2 - (int32_t)x1 + 1;
    if (!redmcsb_f8155_range_fits(
            ((size_t)y2 * REDMCSB_F8155_SCREEN_STRIDE_PIXELS_PC34) + (size_t)x1,
            (size_t)width, aperture->byte_count)) {
        return false;
    }

    for (row = (int32_t)y1; row <= (int32_t)y2; ++row) {
        size_t offset = ((size_t)row * REDMCSB_F8155_SCREEN_STRIDE_PIXELS_PC34) +
                        (size_t)x1;
        int32_t column;
        for (column = (int32_t)x1; column <= (int32_t)x2; ++column) {
            if (((column ^ row) & 1) == 0) {
                aperture->bytes[offset] = 0U;
            }
            ++offset;
        }
    }
    return true;
}

const char *redmcsb_f8155_vidrv_hatch_screen_box_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3243-3300 C25_VGA F8155: each inclusive "
           "A000h box pixel is retained for odd (x^y), otherwise set to zero.";
}
