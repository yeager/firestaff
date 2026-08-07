#ifndef DM1_V1_FMTOWNS_JDM_FONT_H
#define DM1_V1_FMTOWNS_JDM_FONT_H

#include <stddef.h>
#include <stdint.h>
#include "dm1_v1_fmtowns_font_rasteriser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked JDM (Japanese DM1) font resolution contract.
 *
 * Byte-verified 2026-08-07:
 *   * JDATA/GRAPHICS.DAT asset 557 is BYTE-IDENTICAL to
 *     DATA/GRAPHICS.DAT asset 557 (768 bytes; 6 rows x 128 ASCII
 *     glyphs). The Japanese disc uses the same ASCII font for
 *     Latin characters as the English disc.
 *   * The disc contains NO game-owned Shift-JIS glyph bitmap
 *     for multi-byte kanji. TownsOS TBIOS provides those glyphs
 *     via the platform font ROM. Prior parity evidence documents
 *     this at parity-evidence/csb_fmtowns_f31j_text_owner.md.
 *
 * This module formalises the JDM text-render contract:
 *   * ASCII bytes (0x00..0x7f)  -> render via the ASCII font
 *     (same byte-verified raster as English EDM).
 *   * Shift-JIS lead byte (0x81..0x9f or 0xe0..0xfc) followed by
 *     a valid trail byte (0x40..0x7e or 0x80..0xfc) -> Shift-JIS
 *     glyph. TownsOS TBIOS is the ONLY authorised source; no
 *     Firestaff-shipped synthetic Japanese glyph is permitted.
 *   * A shipping consumer without TBIOS integration renders NO
 *     pixels for Shift-JIS pairs and returns 0 (fail-closed).
 */

/* Return 1 if `lead_byte` is a valid Shift-JIS lead. */
int dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(uint8_t lead_byte);

/* Return 1 if the 2-byte sequence (lead, trail) is a valid
 * Shift-JIS pair (per the standard reserved-range rules). */
int dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(
    uint8_t lead_byte, uint8_t trail_byte);

/* Render a JDM label (mix of ASCII and Shift-JIS) into an M11
 * 8bpp framebuffer. ASCII glyphs use the caller-supplied ASCII
 * raster (byte-identical to `dm1_v1_fmtowns_font_rasteriser`
 * expectations); Shift-JIS pairs fail-closed (advance cursor by
 * CHAR_X_WID * 2 without painting) unless the caller supplies a
 * `sjis_glyph_draw` callback that renders a real TBIOS-provided
 * glyph. Returns count of ASCII glyphs painted; Shift-JIS pairs
 * do not contribute unless the callback returns non-zero. */
typedef int (*dm1_v1_fmtowns_jdm_sjis_draw_fn)(
    void *user, uint8_t *fb, int fb_width, int fb_height, int fb_stride,
    int dst_x, int dst_y, uint8_t fg, uint8_t bg,
    uint8_t lead, uint8_t trail);

unsigned int dm1_v1_fmtowns_jdm_font_render_string_pc34(
    const uint8_t                          *ascii_raster,
    dm1_v1_fmtowns_jdm_sjis_draw_fn         sjis_draw,
    void                                    *sjis_draw_user,
    uint8_t                                 *fb,
    int                                      fb_width,
    int                                      fb_height,
    int                                      fb_stride,
    int                                      dst_x,
    int                                      dst_y,
    uint8_t                                  fg,
    uint8_t                                  bg,
    const uint8_t                           *label_bytes,
    size_t                                   label_len);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_JDM_FONT_H */
