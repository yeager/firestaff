/*
 * Source-faithful PC 3.4 C25_VGA implementation boundary for
 * F0680_CopyPixelsToScreenWithoutTransparency.
 */
#ifndef FIRESTAFF_REDMCSB_F0680_COPY_PIXELS_TO_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0680_COPY_PIXELS_TO_SCREEN_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The C25 VGA branch addresses segment A000h as one byte per displayed
 * pixel. This is deliberately an aperture contract, not an RGBA surface or
 * a planar-VGA abstraction.
 */
typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF0680C25VgaAperturePc34Compat;

/*
 * Copies pixel_count packed 4bpp source pixels to the byte-addressed C25
 * VGA aperture. Source pixel zero is the high nibble of source[0]; odd
 * source pixel indices begin at the low nibble. Each output byte is
 * viewport_color_index_offset | source_nibble, matching IMAGE5.C's OR.
 *
 * The PC 3.4 caller state uses a high-nibble colour offset (0x00 or 0x10),
 * so offsets with a populated low nibble are rejected. Invalid input leaves
 * the aperture unchanged.
 */
bool redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset);

bool F0680_CopyPixelsToScreenWithoutTransparency(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset);

const char *redmcsb_f0680_copy_pixels_to_screen_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
