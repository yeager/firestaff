#include "csb_v1_img3_asset_presentation_pc34_compat.h"
#include "redmcsb_f0689_img3_expand_pc34_compat.h"

int csb_v1_img3_decode_and_present_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    const csb_v1_img3_asset_presentation_pc34_compat *presentation,
    uint16_t *out_width,
    uint16_t *out_height)
{
    uint16_t width;
    uint16_t height;

    if (presentation == 0 || presentation->decoded_pixels == 0 ||
        presentation->present_bitmap == 0 ||
        !redmcsb_f0689_img3_expand_even_pc34_compat(
            source, source_byte_count, presentation->decoded_pixels,
            presentation->decoded_pixel_byte_count, &width, &height)) {
        return 0;
    }

    if (!presentation->present_bitmap(
            presentation->present_context, presentation->decoded_pixels,
            width, height)) {
        return 0;
    }

    if (out_width != 0) {
        *out_width = width;
    }
    if (out_height != 0) {
        *out_height = height;
    }
    return 1;
}
