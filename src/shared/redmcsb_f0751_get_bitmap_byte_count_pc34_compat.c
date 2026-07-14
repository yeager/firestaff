#include "redmcsb_f0751_get_bitmap_byte_count_pc34_compat.h"

uint16_t redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
    const redmcsb_f0751_graphic_width_height_pc34 *graphic_width_height,
    int16_t graphic_index)
{
    const redmcsb_f0751_graphic_width_height_pc34 *graphic =
        &graphic_width_height[graphic_index];
    const uint16_t rounded_width =
        (uint16_t)(((uint16_t)graphic->width + UINT16_C(1)) & UINT16_C(0xfffe));
    const uint16_t bytes_per_row = (uint16_t)(rounded_width >> 1);

    /* M103_BITMAP_BYTE_COUNT for the PC 3.4 I34M two-pixel stride. */
    return (uint16_t)((uint32_t)bytes_per_row * (uint16_t)graphic->height);
}

const char *redmcsb_f0751_get_bitmap_byte_count_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C:465-472 defines F0751 as "
           "M103_BITMAP_BYTE_COUNT(G2005_GraphicWidthHeight[index].Width, "
           "G2005_GraphicWidthHeight[index].Height); DEFS.H:3466 provides "
           "the PC I34M two-pixels-per-byte formula.";
}
