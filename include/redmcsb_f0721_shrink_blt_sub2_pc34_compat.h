#ifndef FIRESTAFF_REDMCSB_F0721_SHRINK_BLT_SUB2_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0721_SHRINK_BLT_SUB2_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB BLTSHRNK.C F0721, PC 3.4 I34E/I34M palette-change path. */
void redmcsb_f0721_shrink_blt_sub2_pc34_compat(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    const uint8_t *palette_changes,
    int16_t source_row_offset,
    uint16_t destination_pixel_offset,
    uint16_t source_pixel_ratio,
    uint16_t destination_pixel_width);

#ifdef __cplusplus
}
#endif

#endif
