#include "redmcsb_f8169_blacken_pixels_c25_pc34_compat.h"

bool redmcsb_f8169_blacken_all_pixels_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count,
    uint8_t viewport_color_index_offset,
    RedmcsbF8169PixelWritePc34Compat on_pixel_write, void *context)
{
    uint16_t lfsr_state;

    if (aperture == NULL || aperture_byte_count < REDMCSB_F8169_SCREEN_PIXELS_PC34 ||
        (viewport_color_index_offset & 0x0FU) != 0U) {
        return false;
    }

    lfsr_state = 1U;
    do {
        if (lfsr_state < REDMCSB_F8169_SCREEN_PIXELS_PC34) {
            aperture[lfsr_state] = viewport_color_index_offset;
            if (on_pixel_write != NULL) {
                on_pixel_write(context, lfsr_state, viewport_color_index_offset);
            }
        }
        lfsr_state = (lfsr_state & 1U) != 0U
            ? (uint16_t)((lfsr_state >> 1U) ^ 0xB400U)
            : (uint16_t)(lfsr_state >> 1U);
    } while (lfsr_state != 1U);

    /* VIDEODRV.C:3846 explicitly finishes the only omitted screen index. */
    aperture[0] = viewport_color_index_offset;
    if (on_pixel_write != NULL) on_pixel_write(context, 0U, viewport_color_index_offset);
    return true;
}

const char *redmcsb_f8169_blacken_pixels_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3833-3848 C25 F8169: seed LFSR at one, use "
           "(state>>1)^0xB400 for odd states, write only values below 64000, "
           "then explicitly blacken pixel zero through F8137.";
}
