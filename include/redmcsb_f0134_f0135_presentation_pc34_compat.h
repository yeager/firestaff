#ifndef FIRESTAFF_REDMCSB_F0134_F0135_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0134_F0135_PRESENTATION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Indexed presentation boundary shared by the DM1 and CSB PC 3.4 paths.
 *
 * F0134/F0135 operate on four-bit colour indices, never RGBA values.  The
 * caller supplies the already-admitted source-frame and its 16-colour VGA
 * palette.  This prevents a host-generated surface or a palette substituted
 * after decode from entering either primitive.
 */
typedef struct Redmcsb_F0134F0135_PresentationTargetPc34 {
    uint8_t *pixels;
    size_t pixel_count;
    int width;
    int height;
    int pitch;
    uint32_t source_frame_fingerprint;
    uint32_t palette_fingerprint;
    int source_frame_verified;
    int no_synthetic_fallback;
} Redmcsb_F0134F0135_PresentationTargetPc34;

/* Binds an indexed host target to a verified original frame and exact
 * 16xRGB palette. palette_bytes must be exactly 48 bytes. */
int redmcsb_f0134_f0135_presentation_target_bind_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *out_target,
    uint8_t *pixels,
    size_t pixel_count,
    int width,
    int height,
    int pitch,
    uint32_t source_frame_fingerprint,
    const uint8_t *palette,
    size_t palette_bytes,
    int source_frame_verified,
    int no_synthetic_fallback);

/* Exposes the admission check to adjacent ReDMCSB video-driver routines.
 * Those routines may mutate the indexed target, but must never bypass the
 * original-frame/palette ownership established by target_bind. */
int redmcsb_f0134_f0135_presentation_target_is_bound_pc34(
    const Redmcsb_F0134F0135_PresentationTargetPc34 *target);

/* VIDEO.C F0134: replace the complete admitted indexed target with one
 * palette index. A fill has no transparency or palette remap semantic. */
int redmcsb_f0134_fill_bitmap_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    uint8_t color);

/* FILLBOX.C F0135: inclusive {left,right,top,bottom} fill.  The host-side
 * presentation target clips the caller box to the admitted target before
 * writing. Bit 15 retains the original alternate-pixel mode. */
int redmcsb_f0135_fill_box_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    const int16_t box[4],
    uint16_t color);

/* Source-frame blit used by the same presentation boundary. It clips only
 * at the admitted target edge, preserves source indices, and skips exactly
 * transparent_color when it is in 0..15. Both frames must share the same
 * original palette fingerprint; this routine never fabricates a remap. */
int redmcsb_f0134_f0135_blit_indexed_pc34(
    Redmcsb_F0134F0135_PresentationTargetPc34 *target,
    const uint8_t *source_pixels,
    size_t source_pixel_count,
    int source_width,
    int source_height,
    uint32_t source_palette_fingerprint,
    int dst_x,
    int dst_y,
    int transparent_color);

const char *redmcsb_f0134_f0135_presentation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
