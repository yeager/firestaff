#ifndef FIRESTAFF_REDMCSB_F0655_F0656_VIEWPORT_BITMAP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0655_F0656_VIEWPORT_BITMAP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB BASE.C F0655/F0656, IBM-PC I34E/I34M path.
 *
 * A bitmap points at its first pixel.  Its signed 16-bit pixel width and
 * height immediately precede that pointer, as defined by DEFS.H:3444-3445.
 * Pixel storage, F0635 layout data, and F0132 rendering stay owned by the
 * caller's real-data renderer.
 */
enum {
    REDMCSB_F0655_F0656_PC34_NO_TRANSPARENCY = -1,
    REDMCSB_F0655_F0656_PC34_NO_FLIP = 0,
    REDMCSB_F0655_F0656_PC34_FLIP_HORIZONTAL = 1
};

typedef void (*redmcsb_f0655_f0656_video_blit_pc34_compat)(
    void *context,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    const int16_t xyz[4],
    int16_t source_x,
    int16_t source_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip);

/* Mirrors F0635_'s truth value plus its XYZ/X/Y output contract. */
typedef int (*redmcsb_f0656_resolve_viewport_zone_pc34_compat)(
    void *context,
    const uint8_t *bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *x,
    int16_t *y);

typedef struct redmcsb_f0655_f0656_renderer_pc34_compat {
    redmcsb_f0655_f0656_video_blit_pc34_compat video_blit;
    redmcsb_f0656_resolve_viewport_zone_pc34_compat resolve_viewport_zone;
    void *context;
    uint8_t *viewport_bitmap;
    int16_t viewport_pixel_width;
} redmcsb_f0655_f0656_renderer_pc34_compat;

/* Executes F0615 then the F0655 PC F0132 call. Returns 1 on dispatch. */
int redmcsb_f0655_copy_bitmap_and_flip_pc34_compat(
    const redmcsb_f0655_f0656_renderer_pc34_compat *renderer,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    int16_t flip);

/* Executes F0635 followed by F0656's PC F0132 call. Returns 1 on dispatch. */
int redmcsb_f0656_blit_bitmap_to_viewport_zone_with_transparency_pc34_compat(
    const redmcsb_f0655_f0656_renderer_pc34_compat *renderer,
    const uint8_t *bitmap,
    int16_t zone_index,
    int16_t transparent_color);

const char *redmcsb_f0655_f0656_viewport_bitmap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
