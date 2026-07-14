#include "redmcsb_f0720_shrinkblt_sub1_pc34_compat.h"

void redmcsb_f0720_shrinkblt_sub1_pc34_compat(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t source_pixel_offset,
    uint16_t destination_pixel_offset,
    uint16_t source_pixel_step_6bit,
    uint16_t destination_pixel_width)
{
    uint16_t source_phase = (uint16_t)(source_pixel_step_6bit >> 1);
    uint16_t destination_pixel = destination_pixel_offset;
    const uint32_t destination_limit =
        (uint32_t)destination_pixel_offset + destination_pixel_width;

    while (destination_limit > destination_pixel) {
        uint16_t source_pixel = (uint16_t)((source_phase >> 6) +
                                           source_pixel_offset);
        uint8_t source_byte = bitmap_source[source_pixel >> 1];
        uint8_t destination_high_nibble;

        source_phase = (uint16_t)(source_phase + source_pixel_step_6bit);
        if ((source_pixel & UINT16_C(1)) != 0U) {
            destination_high_nibble = (uint8_t)(source_byte << 4);
        } else {
            destination_high_nibble = (uint8_t)(source_byte & UINT8_C(0xf0));
        }

        source_pixel = (uint16_t)((source_phase >> 6) + source_pixel_offset);
        source_byte = bitmap_source[source_pixel >> 1];
        source_phase = (uint16_t)(source_phase + source_pixel_step_6bit);
        if ((source_pixel & UINT16_C(1)) != 0U) {
            bitmap_destination[destination_pixel >> 1] =
                (uint8_t)(destination_high_nibble | (source_byte & UINT8_C(0x0f)));
        } else {
            bitmap_destination[destination_pixel >> 1] =
                (uint8_t)(destination_high_nibble | (source_byte >> 4));
        }
        destination_pixel = (uint16_t)(destination_pixel + 2U);
    }
}

const char *redmcsb_f0720_shrinkblt_sub1_source_evidence_pc34(void)
{
    return "ReDMCSB BLTSHRNK.C F0720_ShrinkBLT_Sub1: PC 3.4 expanded "
           "source samples two packed 4-bit pixels with a 6-bit fixed-point "
           "source phase and writes one destination byte per iteration.";
}
