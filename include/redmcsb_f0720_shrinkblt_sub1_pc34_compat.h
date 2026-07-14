/*
 * ReDMCSB BLTSHRNK.C F0720_ShrinkBLT_Sub1, PC 3.4 route.
 *
 * This is the no-palette-change inner loop used by
 * F0129_VIDEO_BlitShrinkWithPaletteChanges.  Pixels are packed as two
 * four-bit values per byte; source and destination offsets are in pixels.
 */
#ifndef FIRESTAFF_REDMCSB_F0720_SHRINKBLT_SUB1_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0720_SHRINKBLT_SUB1_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Writes pairs of sampled source pixels to destination.  The source phase
 * starts at source_pixel_step_6bit / 2 and advances by that step per pixel,
 * exactly as BLTSHRNK.C F0720 does.  Destination width is measured in
 * pixels; its caller supplies an even width in the original blit path.
 */
void redmcsb_f0720_shrinkblt_sub1_pc34_compat(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t source_pixel_offset,
    uint16_t destination_pixel_offset,
    uint16_t source_pixel_step_6bit,
    uint16_t destination_pixel_width);

const char *redmcsb_f0720_shrinkblt_sub1_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
