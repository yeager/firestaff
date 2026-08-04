#ifndef FIRESTAFF_DM2_V1_GFX_MAIN_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GFX_MAIN_PC34_COMPAT_H

/*
 * dm2_v1_gfx_main_pc34_compat.h — DM2 main graphics operations.
 *
 * Ports the 16 functions from skproject c_gfx_main.cpp: backbuffer
 * init, rect fill, halftone, fade, blit-to-screen, scrollbox line
 * advance/draw, stretch blit for viewport transitions, and drawings
 * completed.
 *
 * Source: skproject/SKWINSPX/src/v4/c_gfx_main.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants (c_gfx_main.cpp) ────────────────────────────────────── */

#define DM2_GFX_ORIG_SWIDTH      320
#define DM2_GFX_ORIG_SHEIGHT     200
#define DM2_GFX_BACKBUFFER_W     0xe0   /* 224 */
#define DM2_GFX_BACKBUFFER_H     0x88   /* 136 */
#define DM2_GFX_STRETCH_DELTAX   0x67
#define DM2_GFX_STRETCH_START    0x3200
#define DM2_GFX_STRETCH_END      0xdc00

/* ── Rect helper ───────────────────────────────────────────────────── */

typedef struct DM2_V1_GfxRect {
    int16_t x, y, w, h;
} DM2_V1_GfxRect;

/* ── GFX system state ──────────────────────────────────────────────── */

typedef struct DM2_V1_GfxMainState {
    uint8_t *bitmapptr;
    int16_t  backbuff1;
    int16_t  backbuff2;
    int16_t  backbuffrect_x, backbuffrect_y, backbuffrect_w, backbuffrect_h;
    int16_t  backbuffer_w;   /* 0xe0 = 224 */
    int16_t  backbuffer_h;   /* 0x88 = 136 */
    uint8_t *pictbuff;
    int32_t  disable_video;
} DM2_V1_GfxMainState;

/* ── Bitmap header (minimal, for get/set accessors) ────────────────── */

typedef struct DM2_V1_BmpHeader {
    int16_t width;
    int16_t height;
    int16_t res;
} DM2_V1_BmpHeader;

/* ── Callback vtable ───────────────────────────────────────────────── */

typedef struct DM2_V1_GfxMainCallbacks {
    /* Blitter operations */
    void (*blit)(void *ctx, uint8_t *dst, const uint8_t *src,
                 const DM2_V1_GfxRect *rect);
    void (*fill)(void *ctx, uint8_t *dst, const DM2_V1_GfxRect *rect,
                 uint8_t pixel);
    void (*blit_within_screen)(void *ctx, const DM2_V1_GfxRect *src_rect,
                               int16_t dst_x, int16_t dst_y);

    /* Rect queries */
    void (*query_expanded_rect)(void *ctx, int16_t query_id,
                                DM2_V1_GfxRect *out);
    void (*query_blit_rect)(void *ctx, int16_t query_id,
                            DM2_V1_GfxRect *out);

    /* Mouse */
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);

    /* Palette */
    uint8_t (*get_palette_entry)(void *ctx, int16_t index);
    uint8_t (*palette_to_ui8)(void *ctx, int16_t palette_index);
    uint8_t (*ui8_to_pixel)(void *ctx, uint8_t ui8val);
    void    (*select_palette_set)(void *ctx, int16_t set_index);

    /* Bitmap allocation (dballoc) */
    uint8_t *(*get_bmp)(void *ctx, int16_t id);
    void     (*dballoc_free)(void *ctx, int16_t id);

    /* Bitmap header accessors */
    int16_t (*get_bmpheader_width)(void *ctx, uint8_t *bmp);
    int16_t (*get_bmpheader_height)(void *ctx, uint8_t *bmp);
    int16_t (*get_bmpheader_res)(void *ctx, uint8_t *bmp);
    void    (*set_bmpheader_width)(void *ctx, uint8_t *bmp, int16_t w);
    void    (*set_bmpheader_height)(void *ctx, uint8_t *bmp, int16_t h);
    void    (*set_bmpheader_res)(void *ctx, uint8_t *bmp, int16_t r);

    /* Screen buffer */
    uint8_t *(*get_dm2screen)(void *ctx);

    /* Text drawing (delegates to gfx_str) */
    void (*draw_string)(void *ctx, uint8_t *dst, int16_t x, int16_t y,
                        uint8_t color, const char *text);
    void (*print_syserr_text)(void *ctx, const char *text);

    /* Self-reference fill for internal use */
    void (*fill_entire_pict)(void *ctx, uint8_t *bmp, uint8_t pixel);

    /* Scrollbox */
    const char *(*scrollbox_message)(void *ctx, int16_t line_index);

    /* Mouse blocking (driver) */
    void (*driver_blockmouseinput)(void *ctx);
    void (*driver_unblockmouseinput)(void *ctx);

    /* MBlitter (for _specialblit) */
    void (*mblitter_blit)(void *ctx, uint8_t *dst, const uint8_t *src,
                          const DM2_V1_GfxRect *rect);
    void (*mblitter_blit_hline_stretched)(void *ctx, uint8_t *dst,
                                          const uint8_t *src,
                                          int16_t src_x, int16_t dst_x,
                                          int16_t width, int16_t y,
                                          int32_t stretch_frac);

    /* Game state accessors */
    int32_t (*get_gametick)(void *ctx);
    int16_t (*get_dialog2)(void *ctx);

    /* DDAT field accessors (skproject variable addresses) */
    int32_t (*get_v1e025c)(void *ctx);
    void    (*set_v1e025c)(void *ctx, int32_t val);
    int16_t (*get_v1e01d8)(void *ctx);
    int16_t (*get_v1e141e)(void *ctx);
    void    (*set_v1e141e)(void *ctx, int16_t val);
    int16_t (*get_v1e1420)(void *ctx);
    void    (*set_v1e1420)(void *ctx, int16_t val);
    int16_t (*get_v1e1408)(void *ctx);
    int16_t (*get_v1d2746)(void *ctx);
    int16_t (*get_v1d2744)(void *ctx);
    int16_t (*get_v1d70ea)(void *ctx);
    int16_t (*get_v1e141c)(void *ctx);
    void    (*set_v1e141c)(void *ctx, int16_t val);

    /* String metrics (strdat) */
    int16_t (*strxplus)(void *ctx, const char *text);
    int16_t (*gfxstrw1)(void *ctx);
    int16_t (*gfxstrw3)(void *ctx);

    /* DRV_screen256 pixel pointer */
    uint8_t *(*DRV_screen256)(void *ctx);

    /* Callback context */
    void *ctx;
} DM2_V1_GfxMainCallbacks;

/* ── Public API ────────────────────────────────────────────────────── */

/* c_gfx_system::init() — initialize gfx system state */
void dm2_v1_gfx_main_init(DM2_V1_GfxMainState *st);

/* DM2_INIT_BACKBUFF() — initialize backbuffer (0xe0 x 0x88) */
void dm2_v1_gfx_main_init_backbuff(DM2_V1_GfxMainState *st,
                                    const DM2_V1_GfxMainCallbacks *cb);

/* DM2_DRAW_GAMELOAD_DIALOGUE_TO_SCREEN() */
void dm2_v1_gfx_main_draw_gameload_dialogue(DM2_V1_GfxMainState *st,
                                             const DM2_V1_GfxMainCallbacks *cb);

/* DM2_gfxmain_3929_0914() — advance scrollbox line */
void dm2_v1_gfx_main_advance_scrollbox(DM2_V1_GfxMainState *st,
                                        const DM2_V1_GfxMainCallbacks *cb);

/* DM2_gfxmain_3929_0929(color, text) — draw scrollbox line */
void dm2_v1_gfx_main_draw_scrollbox_line(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          uint8_t color,
                                          const char *text);

/* DM2_gfxmain_0b36_0cbe(buttongroup, alloc) — blit button rects */
void dm2_v1_gfx_main_blit_button_rects(DM2_V1_GfxMainState *st,
                                        const DM2_V1_GfxMainCallbacks *cb,
                                        int16_t buttongroup,
                                        int16_t alloc);

/* DM2_FILL_BACKBUFF_RECT(rect, pixel) */
void dm2_v1_gfx_main_fill_backbuff_rect(DM2_V1_GfxMainState *st,
                                         const DM2_V1_GfxMainCallbacks *cb,
                                         const DM2_V1_GfxRect *rect,
                                         uint8_t pixel);

/* DM2_FILL_SCREEN_RECT(query, pixel) */
void dm2_v1_gfx_main_fill_screen_rect(DM2_V1_GfxMainState *st,
                                       const DM2_V1_GfxMainCallbacks *cb,
                                       int16_t query_id,
                                       uint8_t pixel);

/* DM2_FILL_FULLSCREEN(rect, pixel) */
void dm2_v1_gfx_main_fill_fullscreen(DM2_V1_GfxMainState *st,
                                      const DM2_V1_GfxMainCallbacks *cb,
                                      const DM2_V1_GfxRect *rect,
                                      uint8_t pixel);

/* DM2_FILL_ENTIRE_PICT(bmp, pixel) */
void dm2_v1_gfx_main_fill_entire_pict(DM2_V1_GfxMainState *st,
                                       const DM2_V1_GfxMainCallbacks *cb,
                                       uint8_t *bmp,
                                       uint8_t pixel);

/* DM2_FILL_HALFTONE_RECTV(rect) — static, checkerboard grey-out */
void dm2_v1_gfx_main_fill_halftone_rectv(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          const DM2_V1_GfxRect *rect);

/* DM2_FILL_HALFTONE_RECTI(query) — halftone by query id */
void dm2_v1_gfx_main_fill_halftone_recti(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          int16_t query_id);

/* DM2_FADE_SCREEN(mode) — fade screen */
void dm2_v1_gfx_main_fade_screen(DM2_V1_GfxMainState *st,
                                  const DM2_V1_GfxMainCallbacks *cb,
                                  int16_t mode);

/* blit_toscreen(...) — blit source to screen with disable_video guard */
void dm2_v1_gfx_main_blit_toscreen(DM2_V1_GfxMainState *st,
                                    const DM2_V1_GfxMainCallbacks *cb,
                                    const uint8_t *src,
                                    const DM2_V1_GfxRect *rect);

/* _specialblit(dest, src, rect, stretched) — stretch blit */
void dm2_v1_gfx_main_specialblit(DM2_V1_GfxMainState *st,
                                  const DM2_V1_GfxMainCallbacks *cb,
                                  uint8_t *dest,
                                  const uint8_t *src,
                                  const DM2_V1_GfxRect *rect,
                                  int16_t stretched);

/* DM2_DRAWINGS_COMPLETED() — finalize viewport drawings */
void dm2_v1_gfx_main_drawings_completed(DM2_V1_GfxMainState *st,
                                         const DM2_V1_GfxMainCallbacks *cb);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GFX_MAIN_PC34_COMPAT_H */
