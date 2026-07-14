/* ReDMCSB NEC816.C F8151_VIDRV_02_Blit, PC 3.4 C25_VGA route. */
#ifndef FIRESTAFF_REDMCSB_F8151_VIDRV_BLIT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8151_VIDRV_BLIT_PC34_COMPAT_H

#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8151_FLIP_NONE_PC34 = 0,
    REDMCSB_F8151_FLIP_HORIZONTAL_PC34 = 1,
    REDMCSB_F8151_FLIP_VERTICAL_PC34 = 2,
    REDMCSB_F8151_FLIP_BOTH_PC34 = 3
};

typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} RedmcsbF8151BoxPc34Compat;

/*
 * Performs F8151's source-bitmap to C25 VGA-aperture branch. Source pixels
 * are packed 4bpp (high nibble first); destination bytes are the original
 * A000h aperture model. A negative transparent_color is opaque mode.
 *
 * This is deliberately the source-bitmap to screen branch only. The C25 VGA
 * reverse branch calls F8143, which packs aperture pixels into a bitmap and
 * remains a separately audited/readback mapping. C25 F0681/F0683 and F8144
 * have empty source bodies, so horizontal-flip routes intentionally preserve
 * the target rather than inventing a flipped image.
 */
bool redmcsb_f8151_vidrv_blit_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *destination,
    const RedmcsbF8151BoxPc34Compat *box,
    int16_t source_x, int16_t source_y,
    int16_t source_width, int16_t destination_width,
    int16_t transparent_color, int16_t flip,
    uint8_t viewport_color_index_offset);

const char *redmcsb_f8151_vidrv_blit_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
