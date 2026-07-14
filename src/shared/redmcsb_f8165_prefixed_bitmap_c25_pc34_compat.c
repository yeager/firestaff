#include "redmcsb_f8165_prefixed_bitmap_c25_pc34_compat.h"

#include <string.h>

static bool redmcsb_f8165_range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

static void redmcsb_f8165_write_le16(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8U);
}

bool redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
    const RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    uint16_t x, int16_t y, int16_t width, uint16_t height, int16_t operation,
    uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    size_t *result_byte_count)
{
    size_t pixel_byte_count;
    size_t total_byte_count;
    size_t source_offset;
    size_t row;

    if (width < 0 || (size_t)width > SIZE_MAX / (size_t)height) {
        return false;
    }
    pixel_byte_count = (size_t)width * (size_t)height;
    if (pixel_byte_count > SIZE_MAX - REDMCSB_F8165_C25_HEADER_BYTES_PC34) {
        return false;
    }
    total_byte_count = REDMCSB_F8165_C25_HEADER_BYTES_PC34 + pixel_byte_count;
    /* VIDEODRV.C:3674-3683 returns before touching P2545_pi_ for operation 0. */
    if (operation == 0) {
        if (result_byte_count == NULL) return false;
        *result_byte_count = total_byte_count;
        return true;
    }

    if (aperture == NULL || aperture->bytes == NULL || prefixed_bitmap == NULL ||
        y < 0 || x > REDMCSB_F8165_C25_SCREEN_STRIDE_PC34 ||
        (size_t)width > REDMCSB_F8165_C25_SCREEN_STRIDE_PC34 - (size_t)x ||
        prefixed_bitmap_byte_count < total_byte_count) {
        return false;
    }
    source_offset = (size_t)y * REDMCSB_F8165_C25_SCREEN_STRIDE_PC34 + (size_t)x;
    if (!redmcsb_f8165_range_fits(source_offset, (size_t)width, aperture->byte_count) ||
        (height != 0U &&
         !redmcsb_f8165_range_fits(
             source_offset + ((size_t)height - 1U) *
                 REDMCSB_F8165_C25_SCREEN_STRIDE_PC34,
             (size_t)width, aperture->byte_count))) {
        return false;
    }

    /* VIDEODRV.C:3697-3719: width, height, A000h source offset, then rows. */
    redmcsb_f8165_write_le16(prefixed_bitmap, (uint16_t)width);
    redmcsb_f8165_write_le16(prefixed_bitmap + 2U, height);
    redmcsb_f8165_write_le16(prefixed_bitmap + 4U, (uint16_t)source_offset);
    for (row = 0U; row < (size_t)height; ++row) {
        memcpy(prefixed_bitmap + REDMCSB_F8165_C25_HEADER_BYTES_PC34 +
                   row * (size_t)width,
               aperture->bytes + source_offset +
                   row * REDMCSB_F8165_C25_SCREEN_STRIDE_PC34,
               (size_t)width);
    }
    return true;
}

const char *redmcsb_f8165_prefixed_bitmap_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3657-3729 C25 F8165: operation zero returns "
           "width*height+6; capture emits width, height, y*320+x and copies "
           "height raw A000h rows with a 320-byte stride.";
}
