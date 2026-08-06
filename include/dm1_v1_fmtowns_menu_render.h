#ifndef DM1_V1_FMTOWNS_MENU_RENDER_H
#define DM1_V1_FMTOWNS_MENU_RENDER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Consumer that binds every source-locked FM Towns DM1 menu module
 * (regions, dynamenu, dyna_buttons EN/JA, text_geometry, egb_shim)
 * into an M11 framebuffer paint.
 *
 * This module ships only the operations that are fully byte-verified
 * against EDM.EXP:
 *   - Region lookup (regions 10 + 11).
 *   - Panel-colour selection from the 8-byte DYNAMENU record.
 *   - Rectangle fill through the software EGB shim.
 *   - Per-slot label walk via GET_LABEL semantics.
 *
 * The per-glyph rasterisation from the loaded 768-byte font raster
 * is delegated to a caller-supplied glyph_draw callback so this
 * module never guesses source-bit ordering. Once the font raster
 * layout is round-trip-verified against a decoded EDM.EXP sample,
 * a companion module can supply the concrete callback; this file
 * intentionally does not invent one.
 */

typedef enum {
    DM1_V1_FMTOWNS_MENU_LANG_EN = 0,
    DM1_V1_FMTOWNS_MENU_LANG_JA = 1
} dm1_v1_fmtowns_menu_lang_t;

/* Called by the menu-render harness for each visible label slot.
 * `label_bytes` points at a NUL-terminated byte string owned by
 * DYNA_BUTTONS (English) or its Shift-JIS Japanese sibling. `slot`
 * is 0..2 (three-slot DYNAMENU). Rectangle (dst_x, dst_y) is the
 * text baseline within the panel. `fg`/`bg` are the M11 palette
 * indices selected by DRAW_DMENU. `user` is the harness's passthrough
 * pointer.
 *
 * The callback must not invent pixels: if the font raster layout is
 * not yet known good, it should paint nothing and return 0. */
typedef int (*dm1_v1_fmtowns_menu_glyph_draw_fn)(
    void *user, uint8_t *fb, int fb_width, int fb_height, int fb_stride,
    int dst_x, int dst_y, uint8_t fg, uint8_t bg,
    dm1_v1_fmtowns_menu_lang_t lang, unsigned int slot,
    const char *label_bytes);

/* Configuration for one menu render. All fields are consumed
 * read-only; the harness does not mutate them. */
typedef struct {
    dm1_v1_fmtowns_menu_lang_t language;
    const uint8_t             *dynamenu_record;   /* 8 bytes */
    const uint8_t             *menu_font_raster;  /* 768 bytes or NULL */
    dm1_v1_fmtowns_menu_glyph_draw_fn glyph_draw;
    void                      *glyph_draw_user;
    uint8_t                    label_fg;
    uint8_t                    label_bg;
} dm1_v1_fmtowns_menu_render_config_t;

/* Result summary. Fields are populated whether or not the render
 * succeeded so tests can observe partial progress. */
typedef struct {
    int      panel_filled;
    size_t   panel_pixels_written;
    uint8_t  panel_colour;
    int      panel_x1, panel_y1, panel_x2, panel_y2;
    unsigned int label_slots_visited;
    unsigned int label_slots_drawn;
    int      label_slot_returns[3];
} dm1_v1_fmtowns_menu_render_result_t;

/* Paint the FM Towns dynamic menu into `fb` (an 8bpp indexed
 * framebuffer of size fb_width x fb_height, row stride fb_stride).
 * Uses byte-verified region and colour constants; per-glyph
 * rasterisation is delegated to config->glyph_draw (may be NULL).
 * Returns 1 if the panel was filled, 0 on failure. `result` may be
 * NULL; if provided it is populated with per-step diagnostics. */
int dm1_v1_fmtowns_menu_render_pc34(
    uint8_t                                    *fb,
    int                                         fb_width,
    int                                         fb_height,
    int                                         fb_stride,
    const dm1_v1_fmtowns_menu_render_config_t  *config,
    dm1_v1_fmtowns_menu_render_result_t        *result);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_MENU_RENDER_H */
