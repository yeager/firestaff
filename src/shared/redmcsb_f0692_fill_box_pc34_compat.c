#include "redmcsb_f0692_fill_box_pc34_compat.h"

#include <limits.h>

static bool range_fits(size_t start, size_t count, size_t limit)
{
    return start <= limit && count <= limit - start;
}

/* ReDMCSB IMAGE4.C:48-93 F0685_IMG3_LineColorFilling, expressed without
 * the original 16-bit assembly. */
static void fill_packed_pixels(uint8_t *bitmap,
                               size_t pixel_index,
                               uint8_t color,
                               size_t pixel_count)
{
    color &= 0x0fU;
    while (pixel_count-- != 0U) {
        const size_t byte_index = pixel_index >> 1U;

        if ((pixel_index & 1U) == 0U) {
            bitmap[byte_index] =
                (uint8_t)((bitmap[byte_index] & 0x0fU) | (color << 4U));
        } else {
            bitmap[byte_index] =
                (uint8_t)((bitmap[byte_index] & 0xf0U) | color);
        }
        pixel_index++;
    }
}

bool redmcsb_f0692_fill_box_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    const int16_t zone[4],
    int16_t color,
    uint16_t row_width_pixels)
{
    size_t rounded_row_width;
    size_t left;
    size_t top;
    size_t width;
    size_t height;
    size_t row;

    if (bitmap == NULL || zone == NULL || row_width_pixels == 0U ||
        zone[0] < 0 || zone[1] < 0 || zone[2] <= 0 || zone[3] <= 0) {
        return false;
    }

    /* ReDMCSB DEFS.H:3419 M104_EVEN_INTEGER(P2363_i_Width). */
    rounded_row_width = ((size_t)row_width_pixels + 1U) & ~(size_t)1U;
    left = (size_t)zone[0];
    top = (size_t)zone[1];
    width = (size_t)zone[2];
    height = (size_t)zone[3];

    if (left > rounded_row_width || width > rounded_row_width - left ||
        top > SIZE_MAX / rounded_row_width ||
        height > (SIZE_MAX / rounded_row_width) - top) {
        return false;
    }

    for (row = 0U; row < height; ++row) {
        const size_t pixel_index = (top + row) * rounded_row_width + left;
        const size_t final_pixel_index = pixel_index + width;
        const size_t first_byte = pixel_index >> 1U;
        const size_t last_byte = (final_pixel_index - 1U) >> 1U;

        if (final_pixel_index < pixel_index ||
            !range_fits(first_byte, last_byte - first_byte + 1U,
                        bitmap_size)) {
            return false;
        }
    }

    for (row = 0U; row < height; ++row) {
        const size_t pixel_index = (top + row) * rounded_row_width + left;

        fill_packed_pixels(bitmap, pixel_index, (uint8_t)color, width);
    }
    return true;
}

const char *redmcsb_f0692_fill_box_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE3.C:1166-1203 F0692_FillBox; MEDIA709_I34E_I34M_P31J "
           "with EXETYPE C03_GAME selects M708/M709 zone width and height and "
           "M706/M704 top-left at lines 1194-1198. DEFS.H:172-176 defines the "
           "PC zone layout {left, top, width, height}; DEFS.H:3419 rounds the "
           "row width up to even pixels; IMAGE4.C:48-93 supplies the packed "
           "4bpp F0685 fill operation.";
}
