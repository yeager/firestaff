#include "dm1_v1_fmtowns_jdm_font.h"
#include "dm1_v1_fmtowns_text_geometry.h"

int dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(uint8_t b) {
    return (b >= 0x81u && b <= 0x9fu) || (b >= 0xe0u && b <= 0xfcu);
}

int dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(uint8_t l, uint8_t t) {
    if (!dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(l)) return 0;
    return (t >= 0x40u && t <= 0x7eu) || (t >= 0x80u && t <= 0xfcu);
}

unsigned int dm1_v1_fmtowns_jdm_font_render_string_pc34(
        const uint8_t *ascii_raster,
        dm1_v1_fmtowns_jdm_sjis_draw_fn sjis_draw,
        void *sjis_draw_user,
        uint8_t *fb, int fb_width, int fb_height, int fb_stride,
        int dst_x, int dst_y, uint8_t fg, uint8_t bg,
        const uint8_t *label_bytes, size_t label_len) {
    unsigned int painted = 0;
    int cursor = dst_x;
    size_t i;
    if (!ascii_raster || !fb || !label_bytes) return 0;
    for (i = 0; i < label_len; ) {
        uint8_t b = label_bytes[i];
        if (dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(b) &&
            i + 1 < label_len &&
            dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(b, label_bytes[i+1])) {
            /* Shift-JIS pair. Advance cursor by 2 * CHAR_X_WID
             * (full-width glyph). Try callback; fail-closed if
             * none supplied. */
            if (sjis_draw) {
                int rc = sjis_draw(sjis_draw_user, fb, fb_width, fb_height,
                                   fb_stride, cursor, dst_y, fg, bg,
                                   b, label_bytes[i+1]);
                if (rc > 0) ++painted;
            }
            cursor += DM1_V1_FMTOWNS_CHAR_X_WID * 2;
            i += 2;
        } else if (b < 0x80u) {
            /* ASCII glyph via the shared font rasteriser. */
            if (cursor + DM1_V1_FMTOWNS_CHAR_X_SIZE > fb_width) break;
            if (dm1_v1_fmtowns_font_rasterise_glyph_pc34(
                    ascii_raster, fb, fb_width, fb_height, fb_stride,
                    cursor, dst_y, fg, bg, 0, b)) {
                ++painted;
            }
            cursor += DM1_V1_FMTOWNS_CHAR_X_WID;
            i += 1;
        } else {
            /* Non-ASCII byte outside a valid SJIS pair — skip. */
            i += 1;
        }
    }
    return painted;
}
