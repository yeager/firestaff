#include "redmcsb_f0689_img3_expand_pc34_compat.h"
#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"
#include "redmcsb_f0686_copy_previous_line_pc34_compat.h"
#include "redmcsb_f0687_f0688_img3_pc34_compat.h"
bool redmcsb_f0689_img3_expand_even_pc34_compat(
    const uint8_t *source, size_t source_size, uint8_t *destination,
    size_t destination_size, uint16_t *out_width, uint16_t *out_height)
{
    redmcsb_f0687_img3_stream_pc34_compat stream;
    uint8_t palette[6], command, color;
    uint16_t width, height, count;
    size_t total, offset = 0U, index;

    if (source == NULL || destination == NULL || source_size < 7U ||
        destination_size > SIZE_MAX / 2U) return false;
    width = (uint16_t)(source[0] | ((uint16_t)source[1] << 8U));
    height = (uint16_t)(source[2] | ((uint16_t)source[3] << 8U));
    if (width == 0U || height == 0U || (width & 1U) != 0U) return false;
    if ((size_t)width > SIZE_MAX / (size_t)height) return false;
    total = (size_t)width * (size_t)height;
    if (destination_size * 2U < total) return false;

    stream.bytes = source;
    stream.byte_count = source_size;
    stream.pixel_index = 8U;
    for (index = 0U; index < 6U; ++index) {
        if (!redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &palette[index]))
            return false;
    }
    while (offset < total) {
        if (!redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &command))
            return false;
        if ((command & 7U) == 6U) {
            if ((command & 8U) != 0U) {
                if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(&stream, &count))
                    return false;
            } else {
                count = 1U;
            }
            if (offset < (size_t)width || count > total - offset ||
                !redmcsb_f0686_copy_previous_line_pc34_compat(
                    destination, destination_size, offset, offset - width, count))
                return false;
        } else {
            if ((command & 7U) < 6U) {
                color = palette[command & 7U];
            } else if (!redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &color)) {
                return false;
            }
            if ((command & 8U) != 0U) {
                if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(&stream, &count))
                    return false;
            } else {
                count = 1U;
            }
            if (count > total - offset ||
                !redmcsb_f0685_img3_line_fill_pc34_compat(
                    destination, destination_size, offset, color, count))
                return false;
        }
        offset += count;
    }
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return true;
}
const char *redmcsb_f0689_img3_expand_source_evidence_pc34(void){return "ReDMCSB IMAGE2.C F0689_IMG_ExpandGraphicToBitmap (339-469), PC I34E/I34M IMG3 even-stride branch";}
