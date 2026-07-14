#include "redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat.h"

#include <string.h>

static uint16_t redmcsb_f8166_read_le16(const uint8_t *source)
{
    return (uint16_t)source[0] | ((uint16_t)source[1] << 8U);
}

static bool redmcsb_f8166_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

bool redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat(
    const uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture)
{
    size_t width;
    size_t height;
    size_t destination_offset;
    size_t pixel_byte_count;
    size_t row;

    if (prefixed_bitmap == NULL || aperture == NULL || aperture->bytes == NULL ||
        prefixed_bitmap_byte_count < REDMCSB_F8165_C25_HEADER_BYTES_PC34) {
        return false;
    }
    width = (size_t)redmcsb_f8166_read_le16(prefixed_bitmap);
    height = (size_t)redmcsb_f8166_read_le16(prefixed_bitmap + 2U);
    destination_offset = (size_t)redmcsb_f8166_read_le16(prefixed_bitmap + 4U);
    if (height != 0U && width > SIZE_MAX / height) return false;
    pixel_byte_count = width * height;
    if (pixel_byte_count >
        prefixed_bitmap_byte_count - REDMCSB_F8165_C25_HEADER_BYTES_PC34) {
        return false;
    }
    if (!redmcsb_f8166_range_fits(destination_offset, width, aperture->byte_count) ||
        (height != 0U &&
         !redmcsb_f8166_range_fits(
             destination_offset + (height - 1U) *
                 REDMCSB_F8165_C25_SCREEN_STRIDE_PC34,
             width, aperture->byte_count))) {
        return false;
    }

    /* VIDEODRV.C:3769-3796 copies each prefix payload row to A000h. */
    for (row = 0U; row < height; ++row) {
        memcpy(aperture->bytes + destination_offset +
                   row * REDMCSB_F8165_C25_SCREEN_STRIDE_PC34,
               prefixed_bitmap + REDMCSB_F8165_C25_HEADER_BYTES_PC34 +
                   row * width,
               width);
    }
    return true;
}

const char *redmcsb_f8166_blit_prefixed_bitmap_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3731-3802 C25 F8166: read width, height and "
           "destination offset from the three-word F8165 prefix, then copy "
           "raw payload rows to A000h with a 320-byte stride.";
}
