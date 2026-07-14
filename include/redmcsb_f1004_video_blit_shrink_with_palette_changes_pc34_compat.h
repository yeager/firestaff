#ifndef FIRESTAFF_REDMCSB_F1004_VIDEO_BLIT_SHRINK_WITH_PALETTE_CHANGES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1004_VIDEO_BLIT_SHRINK_WITH_PALETTE_CHANGES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB BLTSHRNK.C:1556-1595, MEDIA458_P20JA_P20JB.
 *
 * Source and destination are packed 4bpp images: the even pixel is the high
 * nibble of each byte. Each row is rounded up to an even pixel width. All
 * dimensions must be positive, destination_pixel_width must be even, and both
 * buffers must cover their respective even-rounded row strides.
 * palette_changes may be NULL; otherwise it points to at least 16 bytes. This
 * function deliberately performs no validation, matching F1004's valid-input
 * contract.
 */
void F1004_VIDEO_BlitShrinkWithPaletteChanges_PC34(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    uint16_t source_pixel_width,
    uint16_t source_height,
    int16_t destination_pixel_width,
    int16_t destination_height,
    const uint8_t *palette_changes);

const char *redmcsb_f1004_video_blit_shrink_with_palette_changes_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1004_VIDEO_BLIT_SHRINK_WITH_PALETTE_CHANGES_PC34_COMPAT_H */
