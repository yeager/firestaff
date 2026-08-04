#ifndef FIRESTAFF_DM2_V1_GFX_STR_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GFX_STR_PC34_COMPAT_H

/*
 * dm2_v1_gfx_str_pc34_compat.h — DM2 text/string rendering module.
 *
 * Ports the 19 functions from skproject c_gfx_str.cpp:
 *   c_stringdata::init, DM2_QUERY_FONT, DM2_QUERY_STR_METRICS,
 *   DM2_DRAW_STRING, DM2_DRAW_STRONG_TEXT, DM2_DRAW_BUTTON_STR,
 *   DM2_DRAW_NAME_STR, DM2_DRAW_VP_STR, DM2_DRAW_GUIDED_STR,
 *   DM2_PRINT_SYSERR_TEXT, DM2_DRAW_VP_RC_STR, DM2_DRAW_LOCAL_TEXT,
 *   DM2_FORMAT_SKSTR, DM2_QUERY_GDAT_TEXT, DM2_DRAW_TEXT_TO_BACKBUFF,
 *   DM2_gfxstr_3929_04e2, DM2_gfxstr_24a5_0732,
 *   DM2_DISPLAY_HINT_TEXT, DM2_SCROLLBOX_MESSAGE.
 *
 * All public functions use callback-based architecture.
 *
 * Source: skproject/SKWINSPX/src/v4/c_gfx_str.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * State struct — corresponds to c_stringdata fields
 * ======================================================================== */

typedef struct DM2_V1_GfxStrState {
    uint8_t font[24];      /* decoded glyph pixels */
    int16_t strx;
    int16_t stry;
    int16_t strxplus;      /* line height advance, default 7 */
    const uint8_t *strptr; /* font data pointer */
    int16_t gfxstrw1;      /* font height, default 6 */
    int16_t gfxstrw2;      /* inter-char gap, default 1 */
    int16_t gfxstrw3;      /* baseline adjust, default 1 */
    int16_t gfxstrw4;      /* char advance width, default 6 */
} DM2_V1_GfxStrState;

/* ========================================================================
 * Rect type (shared with gui_draw)
 * ======================================================================== */

/* Forward — avoid redefinition if gui_draw header already included */
#ifndef DM2_V1_RECT_DEFINED
#define DM2_V1_RECT_DEFINED
typedef struct DM2_V1_Rect DM2_V1_Rect;
#endif

/* ========================================================================
 * Callback struct — external dependencies
 * ======================================================================== */

typedef struct DM2_V1_GfxStrCallbacks {
    /* ---- Screen / bitmap access ---- */
    uint8_t *(*get_dm2screen)(void *ctx);
    uint8_t *(*get_bitmap_ptr)(void *ctx, int32_t id);
    int16_t (*get_bitmap_width)(void *ctx, int32_t id);
    uint8_t *(*get_pictbuff)(void *ctx);

    /* ---- Blitting ---- */
    void (*blit)(void *ctx, uint8_t *src, uint8_t *dst,
                 int16_t sx, int16_t sy, int16_t sw, int16_t sh,
                 int16_t dx, int16_t dy, int16_t srcw, int16_t dstw);
    void (*fill)(void *ctx, uint8_t *dst, int16_t x, int16_t y,
                 int16_t w, int16_t h, int16_t dstw, uint8_t color);
    void (*blit_within_screen)(void *ctx, int16_t sx, int16_t sy,
                               int16_t w, int16_t h,
                               int16_t dx, int16_t dy);
    void (*blit_toscreen)(void *ctx, uint8_t *src,
                          int16_t sx, int16_t sy, int16_t sw, int16_t sh,
                          int16_t dx, int16_t dy, int16_t srcw);

    /* ---- Rect queries ---- */
    bool (*query_blit_rect)(void *ctx, int32_t id,
                            int16_t *x, int16_t *y,
                            int16_t *w, int16_t *h,
                            int16_t *srcx, int16_t *srcy);
    bool (*query_expanded_rect)(void *ctx, int32_t id,
                                int16_t *x, int16_t *y,
                                int16_t *w, int16_t *h);
    bool (*query_topleft_of_rect)(void *ctx, int32_t id,
                                  int16_t *x, int16_t *y);

    /* ---- Button group rects ---- */
    void (*adjust_buttongroup_rects)(void *ctx, int32_t buttongroup);

    /* ---- dballoc bitmap access ---- */
    uint8_t *(*get_bmp)(void *ctx, int32_t id, int16_t *width);

    /* ---- Palette ---- */
    uint8_t (*get_palette_entry)(void *ctx, int32_t idx);
    uint8_t (*palette_to_ui8)(void *ctx, uint8_t color);
    uint8_t (*ui8_to_palette)(void *ctx, uint8_t color);

    /* ---- Mouse ---- */
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);

    /* ---- String utilities ---- */
    void (*str_copy)(void *ctx, char *dst, const char *src);
    void (*str_cat)(void *ctx, char *dst, const char *src);
    int32_t (*str_len)(void *ctx, const char *s);
    void (*copy_memory)(void *ctx, void *dst, const void *src, int32_t len);

    /* ---- GDAT queries ---- */
    bool (*query_gdat_entry_data_buff)(void *ctx, int32_t cls, int32_t sub,
                                       int32_t idx, uint8_t *buf,
                                       int32_t buflen);
    int32_t (*query_gdat_entry_data_length)(void *ctx, int32_t cls,
                                            int32_t sub, int32_t idx);

    /* ---- Numeric conversion ---- */
    void (*ltoa10)(void *ctx, int32_t value, char *buf);

    /* ---- FORMAT_SKSTR context variables ---- */
    const uint8_t *(*get_v1e0988)(void *ctx);   /* format buffer 1 */
    const uint8_t *(*get_v1e097c)(void *ctx);   /* format buffer 2 */
    uint8_t (*get_v1d70c5)(void *ctx);           /* format flag */
    int16_t (*get_v1e0218)(void *ctx);           /* format index */
    const char *(*get_hero_name)(void *ctx, int32_t idx);

    /* ---- Rendering helpers ---- */
    void (*fill_entire_pict)(void *ctx, uint8_t *bmp, uint8_t color);
    void (*scrollbox_message)(void *ctx);

    /* ---- gfx_main cross-references ---- */
    void (*gfxstr_3929_0914)(void *ctx, int16_t x, int16_t y,
                              int16_t w, int16_t h);
    void (*gfxstr_3929_0929)(void *ctx, int16_t x, int16_t y,
                              int16_t w, int16_t h);

    /* ---- Temp rect / palette ---- */
    int32_t (*tmprects_alloc)(void *ctx, int32_t count);
    void (*copy_small_palette)(void *ctx, int32_t src, int32_t dst);
    int32_t (*query_b073)(void *ctx, int32_t idx);
} DM2_V1_GfxStrCallbacks;

/* ========================================================================
 * Public functions
 * ======================================================================== */

/* c_stringdata::init — initialize string state defaults.
 * Source: c_gfx_str.cpp c_stringdata::init() */
void dm2_v1_gfx_str_init(DM2_V1_GfxStrState *state);

/* DM2_QUERY_FONT — decode font glyph into pixel array.
 * Source: c_gfx_str.cpp DM2_QUERY_FONT */
void dm2_v1_gfx_str_query_font(DM2_V1_GfxStrState *state,
                                uint8_t char_code, uint8_t fg_color,
                                uint8_t bg_color);

/* DM2_QUERY_STR_METRICS — measure string pixel dimensions.
 * Source: c_gfx_str.cpp DM2_QUERY_STR_METRICS */
void dm2_v1_gfx_str_query_str_metrics(const DM2_V1_GfxStrState *state,
                                       const char *text,
                                       int16_t *width_out,
                                       int16_t *height_out);

/* DM2_DRAW_STRING — draw string to pixel buffer.
 * Source: c_gfx_str.cpp DM2_DRAW_STRING */
void dm2_v1_gfx_str_draw_string(DM2_V1_GfxStrState *state,
                                 uint8_t *dst, int16_t dstw,
                                 int16_t x, int16_t y,
                                 uint8_t fg_color, uint8_t bg_color,
                                 const char *text,
                                 const DM2_V1_GfxStrCallbacks *cb,
                                 void *ctx);

/* DM2_DRAW_STRONG_TEXT — draw bold/shadow text.
 * Source: c_gfx_str.cpp DM2_DRAW_STRONG_TEXT */
void dm2_v1_gfx_str_draw_strong_text(DM2_V1_GfxStrState *state,
                                      uint8_t *dst, int16_t dstw,
                                      int16_t x, int16_t y,
                                      uint8_t fg_color, uint8_t bg_color,
                                      const char *text,
                                      const DM2_V1_GfxStrCallbacks *cb,
                                      void *ctx);

/* DM2_DRAW_BUTTON_STR — draw string on button.
 * Source: c_gfx_str.cpp DM2_DRAW_BUTTON_STR */
void dm2_v1_gfx_str_draw_button_str(DM2_V1_GfxStrState *state,
                                     int32_t buttongroup, int32_t rect_id,
                                     uint8_t fg_color, uint8_t bg_color,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx);

/* DM2_DRAW_NAME_STR — draw name string on button.
 * Source: c_gfx_str.cpp DM2_DRAW_NAME_STR */
void dm2_v1_gfx_str_draw_name_str(DM2_V1_GfxStrState *state,
                                   int32_t buttongroup, int32_t rect_id,
                                   uint8_t fg_color, uint8_t bg_color,
                                   const char *text,
                                   const DM2_V1_GfxStrCallbacks *cb,
                                   void *ctx);

/* DM2_DRAW_VP_STR — draw to viewport bitmap.
 * Source: c_gfx_str.cpp DM2_DRAW_VP_STR */
void dm2_v1_gfx_str_draw_vp_str(DM2_V1_GfxStrState *state,
                                  int16_t x, int16_t y,
                                  uint8_t color, const char *text,
                                  const DM2_V1_GfxStrCallbacks *cb,
                                  void *ctx);

/* DM2_DRAW_GUIDED_STR — word-wrapped viewport text.
 * Source: c_gfx_str.cpp DM2_DRAW_GUIDED_STR */
void dm2_v1_gfx_str_draw_guided_str(DM2_V1_GfxStrState *state,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx);

/* DM2_PRINT_SYSERR_TEXT — draw to screen directly.
 * Source: c_gfx_str.cpp DM2_PRINT_SYSERR_TEXT */
void dm2_v1_gfx_str_print_syserr_text(DM2_V1_GfxStrState *state,
                                       int16_t x, int16_t y,
                                       uint8_t fg_color, uint8_t bg_color,
                                       const char *text,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx);

/* DM2_DRAW_VP_RC_STR — draw text in rect.
 * Source: c_gfx_str.cpp DM2_DRAW_VP_RC_STR */
void dm2_v1_gfx_str_draw_vp_rc_str(DM2_V1_GfxStrState *state,
                                     int32_t rect_id, uint8_t color,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx);

/* DM2_DRAW_LOCAL_TEXT — draw strong text to bitmap.
 * Source: c_gfx_str.cpp DM2_DRAW_LOCAL_TEXT */
void dm2_v1_gfx_str_draw_local_text(DM2_V1_GfxStrState *state,
                                     int32_t rect_id, uint8_t fg_color,
                                     uint8_t bg_color, const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx);

/* DM2_FORMAT_SKSTR — format DM2 string with substitutions.
 * Source: c_gfx_str.cpp DM2_FORMAT_SKSTR */
void dm2_v1_gfx_str_format_skstr(const char *src, char *dest,
                                  const DM2_V1_GfxStrCallbacks *cb,
                                  void *ctx);

/* DM2_QUERY_GDAT_TEXT — query game data text with decryption.
 * Source: c_gfx_str.cpp DM2_QUERY_GDAT_TEXT */
bool dm2_v1_gfx_str_query_gdat_text(DM2_V1_GfxStrState *state,
                                     int32_t cls, int32_t sub,
                                     int32_t idx, char *buf,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx);

/* DM2_DRAW_TEXT_TO_BACKBUFF — draw to backbuffer.
 * Source: c_gfx_str.cpp DM2_DRAW_TEXT_TO_BACKBUFF */
void dm2_v1_gfx_str_draw_text_to_backbuff(DM2_V1_GfxStrState *state,
                                            int32_t rect_id, int16_t y_off,
                                            const char *text,
                                            const DM2_V1_GfxStrCallbacks *cb,
                                            void *ctx);

/* DM2_gfxstr_3929_04e2 — word-wrap helper.
 * Source: c_gfx_str.cpp DM2_gfxstr_3929_04e2 */
int32_t dm2_v1_gfx_str_word_wrap(const DM2_V1_GfxStrState *state,
                                  const char *src, char *dest,
                                  int32_t pos, int32_t max_width);

/* DM2_gfxstr_24a5_0732 — draw uppercase text.
 * Source: c_gfx_str.cpp DM2_gfxstr_24a5_0732 */
void dm2_v1_gfx_str_draw_uppercase(DM2_V1_GfxStrState *state,
                                    int16_t x, int16_t y,
                                    const char *text,
                                    const DM2_V1_GfxStrCallbacks *cb,
                                    void *ctx);

/* DM2_DISPLAY_HINT_TEXT — scrollbox hint display.
 * Source: c_gfx_str.cpp DM2_DISPLAY_HINT_TEXT */
void dm2_v1_gfx_str_display_hint_text(DM2_V1_GfxStrState *state,
                                       int32_t palette_idx,
                                       const char *text,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx);

/* DM2_SCROLLBOX_MESSAGE — scroll message box.
 * Source: c_gfx_str.cpp DM2_SCROLLBOX_MESSAGE */
void dm2_v1_gfx_str_scrollbox_message(DM2_V1_GfxStrState *state,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GFX_STR_PC34_COMPAT_H */
