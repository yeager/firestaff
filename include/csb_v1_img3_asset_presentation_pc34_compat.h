#ifndef CSB_V1_IMG3_ASSET_PRESENTATION_PC34_COMPAT_H
#define CSB_V1_IMG3_ASSET_PRESENTATION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef int (*csb_v1_img3_present_bitmap_pc34_compat)(
    void *context,
    const uint8_t *packed_pixels,
    uint16_t width,
    uint16_t height);

typedef struct {
    uint8_t *decoded_pixels;
    size_t decoded_pixel_byte_count;
    csb_v1_img3_present_bitmap_pc34_compat present_bitmap;
    void *present_context;
} csb_v1_img3_asset_presentation_pc34_compat;

/*
 * Decodes one caller-owned PC IMG3 record with ReDMCSB F0689 and presents it
 * only after a complete successful decode. The host owns both storage and the
 * display route; this adapter creates no pixels, palette, layout, or fallback.
 */
int csb_v1_img3_decode_and_present_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    const csb_v1_img3_asset_presentation_pc34_compat *presentation,
    uint16_t *out_width,
    uint16_t *out_height);

#endif
