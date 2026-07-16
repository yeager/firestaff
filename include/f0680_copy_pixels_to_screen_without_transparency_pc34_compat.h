/*
 * Bounded caller-owned adapter for ReDMCSB ANIMIMG.C:269
 * F0680_CopyPixelsToScreenWithoutTransparency.
 */
#ifndef FIRESTAFF_F0680_COPY_PIXELS_TO_SCREEN_WITHOUT_TRANSPARENCY_PC34_COMPAT_H
#define FIRESTAFF_F0680_COPY_PIXELS_TO_SCREEN_WITHOUT_TRANSPARENCY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Expands pixel_count packed 4-bit source pixels to one 8-bit destination
 * byte apiece. Source pixel zero is the high nibble of source[0]. Every
 * source value, including zero, is copied. Returns 1 on success and 0 when
 * either complete requested range is invalid; rejected calls write nothing.
 */
int f0680_copy_pixels_to_screen_without_transparency_pc34_compat(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count);

/* Exact source locator retained for standalone provenance checks. */
const char *f0680_copy_pixels_to_screen_without_transparency_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
