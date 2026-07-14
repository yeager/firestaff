#include "redmcsb_f8152_vidrv_fill_box_pc34_compat.h"

static bool redmcsb_f8152_range_fits(size_t offset, size_t count, size_t limit)
{
    return offset <= limit && count <= limit - offset;
}

bool redmcsb_f8152_vidrv_fill_box_pc34_compat(
    RedmcsbF8152C25VgaAperturePc34Compat *aperture,
    const RedmcsbF8152BoxPc34Compat *box,
    uint8_t color, uint8_t viewport_color_index_offset)
{
    int32_t width;
    int32_t height;
    int32_t row;
    uint8_t filled_color;

    if (aperture == NULL || aperture->bytes == NULL || box == NULL || box->left < 0 ||
        box->top < 0 || box->right < box->left || box->bottom < box->top ||
        box->right >= (int16_t)REDMCSB_F8152_SCREEN_STRIDE_PIXELS_PC34) {
        return false;
    }

    width = (int32_t)box->right - (int32_t)box->left + 1;
    height = (int32_t)box->bottom - (int32_t)box->top + 1;
    filled_color = (uint8_t)(viewport_color_index_offset | color);
    if (!redmcsb_f8152_range_fits(
            ((size_t)box->bottom * REDMCSB_F8152_SCREEN_STRIDE_PIXELS_PC34) +
                (size_t)box->left,
            (size_t)width, aperture->byte_count)) {
        return false;
    }
    for (row = 0; row < height; ++row) {
        size_t offset = ((size_t)(box->top + row) *
                         REDMCSB_F8152_SCREEN_STRIDE_PIXELS_PC34) +
                        (size_t)box->left;
        size_t index;

        for (index = 0U; index < (size_t)width; ++index) {
            aperture->bytes[offset + index] = filled_color;
        }
    }
    return true;
}

const char *redmcsb_f8152_vidrv_fill_box_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3127-3161 F8152 and :1192-1208 C25 F8137: "
           "inclusive box, top*320+left row offset, 320-byte stride, "
           "and G8177 viewport offset ORed into every color byte.";
}
