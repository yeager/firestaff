/* ReDMCSB IMAGE3.C F0684 C25 source-to-aperture blit. */
#ifndef FIRESTAFF_REDMCSB_F0684_BLIT_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0684_BLIT_C25_PC34_COMPAT_H

#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { REDMCSB_F0684_FLIP_NONE_PC34 = 0, REDMCSB_F0684_FLIP_HORIZONTAL_PC34 = 1,
       REDMCSB_F0684_FLIP_VERTICAL_PC34 = 2, REDMCSB_F0684_FLIP_BOTH_PC34 = 3 };

typedef struct { int16_t left, right, top, bottom; } RedmcsbF0684BoxPc34Compat;

bool redmcsb_f0684_blit_c25_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *destination,
    const RedmcsbF0684BoxPc34Compat *box, int16_t source_x,
    int16_t source_y, int16_t source_width, int16_t destination_width,
    int16_t transparent_color, int16_t flip, uint8_t viewport_color_index_offset);
const char *redmcsb_f0684_blit_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif
#endif
