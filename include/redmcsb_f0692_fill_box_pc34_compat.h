#ifndef FIRESTAFF_REDMCSB_F0692_FILL_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0692_FILL_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IMAGE3.C F0692_FillBox, MEDIA709_I34E_I34M_P31J,
 * EXETYPE == C03_GAME (PC 3.4).
 *
 * zone is the PC zone layout { left, top, width, height }. bitmap stores two
 * 4bpp pixels per byte: the even pixel is the high nibble. As in IMAGE3.C,
 * row_width_pixels is rounded up to an even number before address arithmetic.
 */
bool redmcsb_f0692_fill_box_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    const int16_t zone[4],
    int16_t color,
    uint16_t row_width_pixels);

const char *redmcsb_f0692_fill_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0692_FILL_BOX_PC34_COMPAT_H */
