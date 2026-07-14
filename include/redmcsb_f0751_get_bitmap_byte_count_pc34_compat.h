/*
 * ReDMCSB STARTUP2.C F0751_GetBitmapByteCount, PC 3.4 I34M route.
 *
 * The original indexes G2005_GraphicWidthHeight and applies the PC packed
 * four-bit bitmap byte-count macro to that entry's width and height.
 */
#ifndef FIRESTAFF_REDMCSB_F0751_GET_BITMAP_BYTE_COUNT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0751_GET_BITMAP_BYTE_COUNT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redmcsb_f0751_graphic_width_height_pc34 {
    int16_t width;
    int16_t height;
} redmcsb_f0751_graphic_width_height_pc34;

/*
 * Returns the PC 3.4 M103_BITMAP_BYTE_COUNT value for graphic_index.
 * As in STARTUP2.C, the caller supplies a valid table and graphic index;
 * F0751 itself does not perform pointer or bounds validation.
 */
uint16_t redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
    const redmcsb_f0751_graphic_width_height_pc34 *graphic_width_height,
    int16_t graphic_index);

const char *redmcsb_f0751_get_bitmap_byte_count_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
