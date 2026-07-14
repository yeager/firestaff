#ifndef FIRESTAFF_REDMCSB_F0691_DRAW_COMPRESSED_IMG3_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0691_DRAW_COMPRESSED_IMG3_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IMAGE3.C F0691_IMG_ExpandGraphicToScreen, MEDIA707_I34E_I34M,
 * EXETYPE == C03_GAME (PC 3.4).
 *
 * graphic starts with little-endian signed 16-bit width and height, followed
 * by six packed-nibble local-palette entries and IMG3 commands. pixel_line is
 * caller-owned packed 4bpp scratch storage: its left pixel is the high nibble.
 * The decoder deliberately keeps skipped pixels unchanged, as F0691 does by
 * not calling F0685 for command 6. Every completed source row is delivered to
 * sink through the F0690-equivalent callback. There is no clipping path in
 * the original routine; this bounded contract therefore requires the complete
 * image to fit on the 320-pixel PC screen row.
 */
typedef void (*redmcsb_f0691_img3_pc34_sink)(
    void *context,
    const uint8_t *packed_pixel_line,
    size_t destination_pixel_index,
    size_t pixel_count);

/* Returns 1 when a complete, bounded F0691 decode reaches every source pixel.
 * Returns 0 for malformed or out-of-contract input and never calls sink in
 * that case. pixel_line is only modified after the compressed stream validates.
 */
int redmcsb_f0691_draw_compressed_img3_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_size,
    int16_t x,
    int16_t y,
    uint8_t *pixel_line,
    size_t pixel_line_size,
    redmcsb_f0691_img3_pc34_sink sink,
    void *sink_context);

#ifdef __cplusplus
}
#endif

#endif
