#ifndef FIRESTAFF_REDMCSB_F0662_F0663_PALETTE_BITMAP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0662_F0663_PALETTE_BITMAP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB BASE.C F0662/F0663, PC-compatible F0129 call paths.
 * Bitmap pointers address pixels; signed int16 width/height occupy the four
 * preceding bytes (DEFS.H M100/M101). Palette bytes and F0129 rendering stay
 * caller-owned and must come from admitted original data.
 */
typedef void (*redmcsb_f0662_f0663_blit_shrink_palette_pc34_compat)(
    void *context,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    int16_t source_pixel_width,
    int16_t source_height,
    int16_t destination_pixel_width,
    int16_t destination_height,
    const uint8_t *palette_changes);

typedef struct redmcsb_f0662_f0663_renderer_pc34_compat {
    redmcsb_f0662_f0663_blit_shrink_palette_pc34_compat blit_shrink_palette;
    void *context;
} redmcsb_f0662_f0663_renderer_pc34_compat;

/* Executes BASE.C F0662 in place. Returns 1 only when F0129 is dispatched. */
int redmcsb_f0662_apply_palette_changes_pc34_compat(
    const redmcsb_f0662_f0663_renderer_pc34_compat *renderer,
    uint8_t *bitmap,
    const uint8_t *palette_changes);

/* Executes BASE.C F0663. Returns 1 only when F0129 is dispatched. */
int redmcsb_f0663_copy_bitmap_with_palette_changes_pc34_compat(
    const redmcsb_f0662_f0663_renderer_pc34_compat *renderer,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    const uint8_t *palette_changes);

const char *redmcsb_f0662_f0663_palette_bitmap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
