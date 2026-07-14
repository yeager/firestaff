/*
 * ReDMCSB PC 3.4 C25 VGA adapter for the flipped transparent F0683 route.
 *
 * IMAGE3.C:612-829 supplies the reverse packed-nibble traversal.  IMAGE5.C:
 * 936-1010 supplies the C25 VGA byte-aperture writes and raw palette-offset
 * OR operation used by the PC 3.4 renderer.
 */
#ifndef FIRESTAFF_REDMCSB_F0683_C25_VGA_FLIPPED_TRANSPARENT_COPY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0683_C25_VGA_FLIPPED_TRANSPARENT_COPY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copies pixel_count packed 4bpp source pixels in reverse order to the
 * byte-addressed C25 VGA aperture.  Pixel zero is the high nibble of source
 * byte zero.  A transparent pixel leaves its aperture byte untouched; every
 * other pixel is written as the unmasked original `pixel | color_offset`.
 *
 * Returns zero without mutation if a caller-owned range is incomplete.
 */
int redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
    const uint8_t *source_packed_4bpp,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *vga_a000_aperture,
    size_t aperture_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color,
    uint8_t color_offset);

const char *redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
