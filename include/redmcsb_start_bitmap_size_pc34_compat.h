#ifndef FIRESTAFF_REDMCSB_START_BITMAP_SIZE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_START_BITMAP_SIZE_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB BMPSIZE.C F0459_START_GetScaledBitmapByteCount, PC34/I34
 * family. The caller supplies positive bitmap pixel dimensions and the
 * original 5-bit scale value. This reports allocation bytes only. */
int16_t F0459_START_GetScaledBitmapByteCount_PC34(
    int16_t pixel_width,
    int16_t pixel_height,
    int16_t scale);

#endif
