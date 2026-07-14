#include "redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat.h"

#include "redmcsb_f8140_scroll_message_area_up_pc34_compat.h"

static bool redmcsb_f8162_range_fits(size_t offset, size_t count, size_t limit)
{
    return offset <= limit && count <= limit - offset;
}

bool redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat(
    RedmcsbF8162VideoPagesPc34Compat *pages,
    const RedmcsbF8162BoxPc34Compat *box,
    uint16_t scroll_rows)
{
    size_t width_bytes;
    int32_t row;
    size_t plane_index;

    if (pages == NULL || box == NULL || pages->plane_count < REDMCSB_F8162_REQUIRED_PLANES_PC34 ||
        pages->plane_count > REDMCSB_F8162_MAX_PLANES_PC34 || box->right < box->left ||
        box->bottom < box->top || scroll_rows == 0U) {
        return false;
    }

    width_bytes = ((size_t)(box->right - box->left + 1)) >> 2U;
    if (width_bytes == 0U || width_bytes > REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34 ||
        (int32_t)box->top + (int32_t)scroll_rows > (int32_t)box->bottom) {
        return false;
    }

    for (plane_index = 0U; plane_index < pages->plane_count; ++plane_index) {
        if (pages->planes[plane_index] == NULL) {
            return false;
        }
    }

    for (row = (int32_t)box->top + (int32_t)scroll_rows;
         row <= (int32_t)box->bottom; ++row) {
        size_t source_offset = (size_t)row * REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34;
        size_t destination_offset = (size_t)(row - (int32_t)scroll_rows) *
                                    REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34;

        if (!redmcsb_f8162_range_fits(source_offset, width_bytes, pages->plane_byte_count) ||
            !redmcsb_f8162_range_fits(destination_offset, width_bytes, pages->plane_byte_count)) {
            return false;
        }
        for (plane_index = 0U; plane_index < pages->plane_count; ++plane_index) {
            redmcsb_f8140_scroll_message_area_up_pc34_compat(
                pages->planes[plane_index] + source_offset,
                pages->planes[plane_index] + destination_offset, width_bytes);
        }
    }
    return true;
}

const char *redmcsb_f8162_vidrv_scroll_message_area_up_source_evidence_pc34(void)
{
    return "ReDMCSB NEC816.C:2438-2466; F8162 transfers A8/B0/B8 and, for "
           "PC 3.4 NEC16 MEDIA472, E0 planes with 160-byte scanline stride.";
}
