/*
 * dm2_v1_gfx_main_pc34_compat.c — DM2 main graphics operations.
 *
 * Ports the 16 functions from skproject c_gfx_main.cpp: backbuffer
 * management, rect fill, halftone overlay, fade, blit-to-screen,
 * scrollbox text, stretch blit, and drawings completed.
 *
 * Source: skproject/SKWINSPX/src/v4/c_gfx_main.cpp
 */

#include "dm2_v1_gfx_main_pc34_compat.h"

#include <string.h>

/* ── c_gfx_system::init() ─────────────────────────────────────────── */
/* Source: c_gfx_main.cpp — c_gfx_system::init() */

void dm2_v1_gfx_main_init(DM2_V1_GfxMainState *st)
{
    memset(st, 0, sizeof(*st));
    st->backbuffer_w = DM2_GFX_BACKBUFFER_W;
    st->backbuffer_h = DM2_GFX_BACKBUFFER_H;
    st->disable_video = 0;
}

/* ── DM2_INIT_BACKBUFF() ──────────────────────────────────────────── */
/* Source: c_gfx_main.cpp — DM2_INIT_BACKBUFF()
 * Allocates backbuffer bitmap via dballoc, sets bmpheader dimensions
 * to 0xe0 x 0x88, sets backbuffrect to cover the full area. */

void dm2_v1_gfx_main_init_backbuff(DM2_V1_GfxMainState *st,
                                    const DM2_V1_GfxMainCallbacks *cb)
{
    uint8_t *bmp;

    if (!cb->get_bmp || !cb->set_bmpheader_width ||
        !cb->set_bmpheader_height || !cb->set_bmpheader_res)
        return;

    bmp = cb->get_bmp(cb->ctx, st->backbuff1);
    if (!bmp)
        return;

    cb->set_bmpheader_width(cb->ctx, bmp, st->backbuffer_w);
    cb->set_bmpheader_height(cb->ctx, bmp, st->backbuffer_h);
    cb->set_bmpheader_res(cb->ctx, bmp, 4);

    st->bitmapptr = bmp;
    st->backbuffrect_x = 0;
    st->backbuffrect_y = 0;
    st->backbuffrect_w = st->backbuffer_w;
    st->backbuffrect_h = st->backbuffer_h;
}

/* ── DM2_DRAW_GAMELOAD_DIALOGUE_TO_SCREEN() ────────────────────────── */
/* Source: c_gfx_main.cpp — blit the game-load dialog bitmap to screen */

void dm2_v1_gfx_main_draw_gameload_dialogue(DM2_V1_GfxMainState *st,
                                             const DM2_V1_GfxMainCallbacks *cb)
{
    uint8_t *screen;
    uint8_t *bmp;
    DM2_V1_GfxRect rect;

    if (!cb->get_dm2screen || !cb->get_bmp || !cb->blit)
        return;

    screen = cb->get_dm2screen(cb->ctx);
    bmp = cb->get_bmp(cb->ctx, st->backbuff2);
    if (!screen || !bmp)
        return;

    rect.x = 0;
    rect.y = 0;
    rect.w = DM2_GFX_ORIG_SWIDTH;
    rect.h = DM2_GFX_ORIG_SHEIGHT;

    cb->blit(cb->ctx, screen, bmp, &rect);
}

/* ── DM2_gfxmain_3929_0914() — advance scrollbox line ─────────────── */
/* Source: c_gfx_main.cpp — shifts scrollbox messages up by one line.
 * Uses blit_within_screen to scroll the scrollbox area, then fills
 * the bottom line with background color. */

void dm2_v1_gfx_main_advance_scrollbox(DM2_V1_GfxMainState *st,
                                        const DM2_V1_GfxMainCallbacks *cb)
{
    DM2_V1_GfxRect src_rect, fill_rect;
    int16_t line_h;

    (void)st;

    if (!cb->get_v1e141e || !cb->set_v1e141e ||
        !cb->get_v1e1420 || !cb->set_v1e1420)
        return;

    /* Source: scrollbox area starts at query rect, line height from gfxstrw1 */
    line_h = cb->gfxstrw1 ? cb->gfxstrw1(cb->ctx) : 8;

    /* Shift visible area up by one line */
    if (cb->query_expanded_rect) {
        cb->query_expanded_rect(cb->ctx, 0x3929, &src_rect);

        if (cb->blit_within_screen) {
            cb->blit_within_screen(cb->ctx, &src_rect,
                                   src_rect.x,
                                   (int16_t)(src_rect.y - line_h));
        }

        /* Fill bottom line with black */
        fill_rect.x = src_rect.x;
        fill_rect.y = (int16_t)(src_rect.y + src_rect.h - line_h);
        fill_rect.w = src_rect.w;
        fill_rect.h = line_h;

        if (cb->fill) {
            uint8_t *screen = cb->get_dm2screen ? cb->get_dm2screen(cb->ctx) : NULL;
            if (screen)
                cb->fill(cb->ctx, screen, &fill_rect, 0);
        }
    }

    /* Advance scrollbox line counter */
    {
        int16_t cur = cb->get_v1e141e(cb->ctx);
        cb->set_v1e141e(cb->ctx, (int16_t)(cur + 1));
    }
}

/* ── DM2_gfxmain_3929_0929(color, text) — draw scrollbox line ──────── */
/* Source: c_gfx_main.cpp — draws a single scrollbox text line at the
 * current scrollbox position.  Delegates text rendering to draw_string
 * (gfx_str module). */

void dm2_v1_gfx_main_draw_scrollbox_line(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          uint8_t color,
                                          const char *text)
{
    DM2_V1_GfxRect rect;
    int16_t y_pos;
    uint8_t *screen;

    (void)st;

    if (!cb->draw_string || !cb->get_dm2screen || !cb->query_expanded_rect)
        return;

    cb->query_expanded_rect(cb->ctx, 0x3929, &rect);

    /* Current scrollbox Y from v1e1420 */
    y_pos = cb->get_v1e1420 ? cb->get_v1e1420(cb->ctx) : rect.y;

    screen = cb->get_dm2screen(cb->ctx);
    if (!screen)
        return;

    cb->draw_string(cb->ctx, screen, rect.x, y_pos, color, text);

    /* If print_syserr_text is available, also echo to system error log */
    if (cb->print_syserr_text && text)
        cb->print_syserr_text(cb->ctx, text);
}

/* ── DM2_gfxmain_0b36_0cbe(buttongroup, alloc) ────────────────────── */
/* Source: c_gfx_main.cpp — blit button group rectangles to screen */

void dm2_v1_gfx_main_blit_button_rects(DM2_V1_GfxMainState *st,
                                        const DM2_V1_GfxMainCallbacks *cb,
                                        int16_t buttongroup,
                                        int16_t alloc)
{
    uint8_t *bmp;
    uint8_t *screen;
    DM2_V1_GfxRect rect;

    (void)st;

    if (!cb->get_bmp || !cb->get_dm2screen || !cb->blit ||
        !cb->query_blit_rect)
        return;

    bmp = cb->get_bmp(cb->ctx, alloc);
    screen = cb->get_dm2screen(cb->ctx);
    if (!bmp || !screen)
        return;

    cb->query_blit_rect(cb->ctx, buttongroup, &rect);
    cb->blit(cb->ctx, screen, bmp, &rect);
}

/* ── DM2_FILL_BACKBUFF_RECT(rect, pixel) ──────────────────────────── */
/* Source: c_gfx_main.cpp — fill a rectangle in the backbuffer bitmap */

void dm2_v1_gfx_main_fill_backbuff_rect(DM2_V1_GfxMainState *st,
                                         const DM2_V1_GfxMainCallbacks *cb,
                                         const DM2_V1_GfxRect *rect,
                                         uint8_t pixel)
{
    if (!cb->fill || !st->bitmapptr)
        return;

    cb->fill(cb->ctx, st->bitmapptr, rect, pixel);
}

/* ── DM2_FILL_SCREEN_RECT(query, pixel) ───────────────────────────── */
/* Source: c_gfx_main.cpp — query a rect by id then fill on screen */

void dm2_v1_gfx_main_fill_screen_rect(DM2_V1_GfxMainState *st,
                                       const DM2_V1_GfxMainCallbacks *cb,
                                       int16_t query_id,
                                       uint8_t pixel)
{
    DM2_V1_GfxRect rect;
    uint8_t *screen;

    (void)st;

    if (!cb->query_expanded_rect || !cb->fill || !cb->get_dm2screen)
        return;

    cb->query_expanded_rect(cb->ctx, query_id, &rect);
    screen = cb->get_dm2screen(cb->ctx);
    if (!screen)
        return;

    cb->fill(cb->ctx, screen, &rect, pixel);
}

/* ── DM2_FILL_FULLSCREEN(rect, pixel) ─────────────────────────────── */
/* Source: c_gfx_main.cpp — fill arbitrary rect on the screen surface */

void dm2_v1_gfx_main_fill_fullscreen(DM2_V1_GfxMainState *st,
                                      const DM2_V1_GfxMainCallbacks *cb,
                                      const DM2_V1_GfxRect *rect,
                                      uint8_t pixel)
{
    uint8_t *screen;

    (void)st;

    if (!cb->fill || !cb->get_dm2screen)
        return;

    screen = cb->get_dm2screen(cb->ctx);
    if (!screen)
        return;

    cb->fill(cb->ctx, screen, rect, pixel);
}

/* ── DM2_FILL_ENTIRE_PICT(bmp, pixel) ─────────────────────────────── */
/* Source: c_gfx_main.cpp — fill entire bitmap using header dimensions */

void dm2_v1_gfx_main_fill_entire_pict(DM2_V1_GfxMainState *st,
                                       const DM2_V1_GfxMainCallbacks *cb,
                                       uint8_t *bmp,
                                       uint8_t pixel)
{
    DM2_V1_GfxRect rect;

    (void)st;

    if (!cb->fill || !cb->get_bmpheader_width || !cb->get_bmpheader_height || !bmp)
        return;

    rect.x = 0;
    rect.y = 0;
    rect.w = cb->get_bmpheader_width(cb->ctx, bmp);
    rect.h = cb->get_bmpheader_height(cb->ctx, bmp);

    cb->fill(cb->ctx, bmp, &rect, pixel);
}

/* ── DM2_FILL_HALFTONE_RECTV(rect) ────────────────────────────────── */
/* Source: c_gfx_main.cpp — checkerboard pattern grey-out.
 * Every other pixel (checkerboard) is set to 0, producing the DM2
 * halftone "greyed out" effect for disabled UI elements. */

void dm2_v1_gfx_main_fill_halftone_rectv(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          const DM2_V1_GfxRect *rect)
{
    uint8_t *screen;
    int16_t x, y;

    (void)st;

    if (!cb->get_dm2screen || !cb->DRV_screen256)
        return;

    screen = cb->DRV_screen256(cb->ctx);
    if (!screen)
        return;

    /* Checkerboard: zero every other pixel.
     * Source: c_gfx_main.cpp — iterates rect, (x+y)&1 == 0 => pixel = 0 */
    for (y = rect->y; y < rect->y + rect->h; y++) {
        for (x = rect->x; x < rect->x + rect->w; x++) {
            if (((x + y) & 1) == 0) {
                screen[y * DM2_GFX_ORIG_SWIDTH + x] = 0;
            }
        }
    }
}

/* ── DM2_FILL_HALFTONE_RECTI(query) ───────────────────────────────── */
/* Source: c_gfx_main.cpp — halftone by query id */

void dm2_v1_gfx_main_fill_halftone_recti(DM2_V1_GfxMainState *st,
                                          const DM2_V1_GfxMainCallbacks *cb,
                                          int16_t query_id)
{
    DM2_V1_GfxRect rect;

    if (!cb->query_expanded_rect)
        return;

    cb->query_expanded_rect(cb->ctx, query_id, &rect);
    dm2_v1_gfx_main_fill_halftone_rectv(st, cb, &rect);
}

/* ── DM2_FADE_SCREEN(mode) ────────────────────────────────────────── */
/* Source: c_gfx_main.cpp — fade screen.
 * mode 0: select palette set 1 (fade-to-black palette)
 * mode 1: select palette set 0 (normal palette) and fill screen black */

void dm2_v1_gfx_main_fade_screen(DM2_V1_GfxMainState *st,
                                  const DM2_V1_GfxMainCallbacks *cb,
                                  int16_t mode)
{
    (void)st;

    if (!cb->select_palette_set)
        return;

    if (mode == 0) {
        /* Fade to black: select the dark/fade palette set */
        cb->select_palette_set(cb->ctx, 1);
    } else {
        /* Restore: select normal palette, then fill screen black */
        cb->select_palette_set(cb->ctx, 0);

        if (cb->fill && cb->get_dm2screen) {
            uint8_t *screen = cb->get_dm2screen(cb->ctx);
            if (screen) {
                DM2_V1_GfxRect full;
                full.x = 0;
                full.y = 0;
                full.w = DM2_GFX_ORIG_SWIDTH;
                full.h = DM2_GFX_ORIG_SHEIGHT;
                cb->fill(cb->ctx, screen, &full, 0);
            }
        }
    }
}

/* ── blit_toscreen(...) ───────────────────────────────────────────── */
/* Source: c_gfx_main.cpp — blit source to screen with disable_video
 * guard.  Increments disable_video before blit, decrements after. */

void dm2_v1_gfx_main_blit_toscreen(DM2_V1_GfxMainState *st,
                                    const DM2_V1_GfxMainCallbacks *cb,
                                    const uint8_t *src,
                                    const DM2_V1_GfxRect *rect)
{
    uint8_t *screen;

    if (!cb->blit || !cb->get_dm2screen)
        return;

    st->disable_video++;

    screen = cb->get_dm2screen(cb->ctx);
    if (screen)
        cb->blit(cb->ctx, screen, src, rect);

    st->disable_video--;
}

/* ── _specialblit(dest, src, rect, stretched) ─────────────────────── */
/* Source: c_gfx_main.cpp — stretch blit for viewport transitions.
 * When stretched != 0, uses mblitter_blit_hline_stretched to apply
 * progressive stretch from STRETCH_START to STRETCH_END over the rect
 * height.  Otherwise delegates to mblitter_blit. */

void dm2_v1_gfx_main_specialblit(DM2_V1_GfxMainState *st,
                                  const DM2_V1_GfxMainCallbacks *cb,
                                  uint8_t *dest,
                                  const uint8_t *src,
                                  const DM2_V1_GfxRect *rect,
                                  int16_t stretched)
{
    (void)st;

    if (stretched == 0) {
        /* Normal blit */
        if (cb->mblitter_blit)
            cb->mblitter_blit(cb->ctx, dest, src, rect);
        return;
    }

    /* Stretched blit: iterate scanlines with progressive stretch factor.
     * Source: c_gfx_main.cpp — stretch from STRETCH_START, adding
     * STRETCH_DELTAX per line until STRETCH_END. */
    if (!cb->mblitter_blit_hline_stretched)
        return;

    {
        int16_t y;
        int32_t stretch_frac = DM2_GFX_STRETCH_START;

        for (y = rect->y; y < rect->y + rect->h; y++) {
            int16_t center_x = (int16_t)(rect->x + rect->w / 2);
            int16_t half_w = (int16_t)((rect->w * stretch_frac) >> 16);
            int16_t src_x = (int16_t)(center_x - half_w);
            int16_t dst_x = (int16_t)(center_x - half_w);
            int16_t width = (int16_t)(half_w * 2);

            cb->mblitter_blit_hline_stretched(cb->ctx, dest, src,
                                               src_x, dst_x,
                                               width, y,
                                               stretch_frac);

            stretch_frac += DM2_GFX_STRETCH_DELTAX;
            if (stretch_frac > DM2_GFX_STRETCH_END)
                stretch_frac = DM2_GFX_STRETCH_END;
        }
    }
}

/* ── DM2_DRAWINGS_COMPLETED() ─────────────────────────────────────── */
/* Source: c_gfx_main.cpp — finalize viewport drawings.
 * Blits the backbuffer to screen, then shows the mouse cursor. */

void dm2_v1_gfx_main_drawings_completed(DM2_V1_GfxMainState *st,
                                         const DM2_V1_GfxMainCallbacks *cb)
{
    if (st->bitmapptr && cb->blit && cb->get_dm2screen) {
        DM2_V1_GfxRect rect;
        uint8_t *screen;

        rect.x = st->backbuffrect_x;
        rect.y = st->backbuffrect_y;
        rect.w = st->backbuffrect_w;
        rect.h = st->backbuffrect_h;

        screen = cb->get_dm2screen(cb->ctx);
        if (screen) {
            st->disable_video++;
            cb->blit(cb->ctx, screen, st->bitmapptr, &rect);
            st->disable_video--;
        }
    }

    if (cb->show_mouse)
        cb->show_mouse(cb->ctx);
}
