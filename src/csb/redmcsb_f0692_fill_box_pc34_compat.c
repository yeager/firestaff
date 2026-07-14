#include "redmcsb_f0692_fill_box_pc34_compat.h"
#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"

bool redmcsb_f0692_fill_box_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    const int16_t zone[4],
    int16_t color,
    uint16_t row_width_pixels)
{
    size_t row_width;
    size_t left;
    size_t top;
    size_t width;
    size_t height;
    size_t row;

    if (bitmap == NULL || zone == NULL || color < 0 || color > 15 ||
        row_width_pixels == 0U || zone[0] < 0 || zone[1] < 0 ||
        zone[2] <= 0 || zone[3] <= 0) {
        return false;
    }
    row_width = ((size_t)row_width_pixels + 1U) & ~(size_t)1U;
    left = (size_t)zone[0];
    top = (size_t)zone[1];
    width = (size_t)zone[2];
    height = (size_t)zone[3];
    if (left > row_width || width > row_width - left ||
        top > SIZE_MAX / row_width || height > SIZE_MAX / row_width - top) {
        return false;
    }
    for (row = 0U; row < height; ++row) {
        if (!redmcsb_f0685_img3_line_fill_pc34_compat(
                bitmap, bitmap_size, (top + row) * row_width + left,
                (uint8_t)color, width)) {
            return false;
        }
    }
    return true;
}

const char *redmcsb_f0692_fill_box_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE3.C F0692_FillBox (1166-1201), PC I34E/I34M C03_GAME";
}
