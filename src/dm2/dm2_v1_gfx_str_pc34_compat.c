/*
 * dm2_v1_gfx_str_pc34_compat.c — DM2 text/string rendering module.
 *
 * Ports 19 functions from skproject c_gfx_str.cpp to pure C with
 * callback-based architecture for external dependencies.
 *
 * Source: skproject/SKWINSPX/src/v4/c_gfx_str.cpp
 */

#include "dm2_v1_gfx_str_pc34_compat.h"

#include <string.h>

/* ── c_stringdata::init ────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp c_stringdata::init() */

void dm2_v1_gfx_str_init(DM2_V1_GfxStrState *state)
{
    if (state == NULL) return;
    memset(state->font, 0, sizeof(state->font));
    state->strx = 0;
    state->stry = 0;
    state->strxplus = 7;    /* default line height advance */
    state->strptr = NULL;
    state->gfxstrw1 = 6;   /* font height */
    state->gfxstrw2 = 1;   /* inter-char gap */
    state->gfxstrw3 = 1;   /* baseline adjust */
    state->gfxstrw4 = 6;   /* char advance width */
}

/* ── DM2_QUERY_FONT ───────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_QUERY_FONT
 *
 * Decodes a font glyph from the font data (strptr) into the 24-byte
 * pixel array (state->font).  The font data is a 1-bit-per-pixel
 * bitmap; each glyph is gfxstrw1 rows of 1 byte each.  Bits are
 * expanded to fg_color / bg_color. */

void dm2_v1_gfx_str_query_font(DM2_V1_GfxStrState *state,
                                uint8_t char_code, uint8_t fg_color,
                                uint8_t bg_color)
{
    const uint8_t *glyph_data;
    int row, col;
    int font_h, font_w;
    int idx;

    if (state == NULL || state->strptr == NULL) return;

    font_h = state->gfxstrw1;
    font_w = state->gfxstrw4;
    if (font_h > 6) font_h = 6;
    if (font_w > 4) font_w = 4;

    /* Source: each glyph starts at char_code * gfxstrw1 bytes into font data.
     * c_gfx_str.cpp: eaxb = character, edxb = fg, ebxb = bg.
     * Font data: 1 byte per row, MSB-first pixel order. */
    glyph_data = state->strptr + (int32_t)char_code * font_h;

    memset(state->font, bg_color, sizeof(state->font));

    idx = 0;
    for (row = 0; row < font_h && idx < 24; row++) {
        uint8_t bits = glyph_data[row];
        for (col = 0; col < font_w && idx < 24; col++) {
            /* MSB first: bit 7 is leftmost pixel */
            if (bits & (0x80u >> col)) {
                state->font[idx] = fg_color;
            }
            idx++;
        }
    }
}

/* ── DM2_QUERY_STR_METRICS ────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_QUERY_STR_METRICS
 *
 * Measures string pixel dimensions.  Width = sum of char widths +
 * inter-char gaps.  Height = font height + baseline adjust.
 * Newlines ('\n') advance to next line. */

void dm2_v1_gfx_str_query_str_metrics(const DM2_V1_GfxStrState *state,
                                       const char *text,
                                       int16_t *width_out,
                                       int16_t *height_out)
{
    int16_t max_w = 0;
    int16_t cur_w = 0;
    int16_t lines = 1;
    const char *p;

    if (state == NULL || text == NULL) {
        if (width_out) *width_out = 0;
        if (height_out) *height_out = 0;
        return;
    }

    for (p = text; *p != '\0'; p++) {
        if (*p == '\n') {
            if (cur_w > max_w) max_w = cur_w;
            cur_w = 0;
            lines++;
        } else {
            if (cur_w > 0) {
                cur_w += state->gfxstrw2; /* inter-char gap */
            }
            cur_w += state->gfxstrw4; /* char advance width */
        }
    }
    if (cur_w > max_w) max_w = cur_w;

    if (width_out) *width_out = max_w;
    if (height_out) *height_out = (int16_t)(lines * state->strxplus);
}

/* ── DM2_DRAW_STRING ──────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_STRING
 *
 * Draws each character by decoding the glyph via QUERY_FONT then
 * blitting the font[] array to the destination bitmap. */

void dm2_v1_gfx_str_draw_string(DM2_V1_GfxStrState *state,
                                 uint8_t *dst, int16_t dstw,
                                 int16_t x, int16_t y,
                                 uint8_t fg_color, uint8_t bg_color,
                                 const char *text,
                                 const DM2_V1_GfxStrCallbacks *cb,
                                 void *ctx)
{
    const char *p;
    int16_t cx, cy;
    int16_t font_h, font_w;

    if (state == NULL || dst == NULL || text == NULL) return;

    cx = x;
    cy = y;
    font_h = state->gfxstrw1;
    font_w = state->gfxstrw4;

    for (p = text; *p != '\0'; p++) {
        if (*p == '\n') {
            cx = x;
            cy += state->strxplus;
            continue;
        }

        dm2_v1_gfx_str_query_font(state, (uint8_t)*p, fg_color, bg_color);

        /* Blit the decoded glyph to destination */
        if (cb != NULL && cb->blit != NULL) {
            cb->blit(ctx, state->font, dst,
                     0, 0, font_w, font_h,
                     cx, cy, font_w, dstw);
        }

        cx += font_w + state->gfxstrw2;
    }
}

/* ── DM2_DRAW_STRONG_TEXT ─────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_STRONG_TEXT
 *
 * Draws text with shadow effect: first draw at (x+1, y+1) in bg_color,
 * then draw at (x, y) in fg_color. */

void dm2_v1_gfx_str_draw_strong_text(DM2_V1_GfxStrState *state,
                                      uint8_t *dst, int16_t dstw,
                                      int16_t x, int16_t y,
                                      uint8_t fg_color, uint8_t bg_color,
                                      const char *text,
                                      const DM2_V1_GfxStrCallbacks *cb,
                                      void *ctx)
{
    if (state == NULL || text == NULL) return;

    /* Shadow pass: draw at offset (1,1) with bg_color as fg */
    dm2_v1_gfx_str_draw_string(state, dst, dstw,
                                (int16_t)(x + 1), (int16_t)(y + 1),
                                bg_color, 0, text, cb, ctx);

    /* Foreground pass: draw at (x, y) with fg_color, transparent bg (0) */
    dm2_v1_gfx_str_draw_string(state, dst, dstw,
                                x, y, fg_color, 0, text, cb, ctx);
}

/* ── DM2_DRAW_BUTTON_STR ─────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_BUTTON_STR
 *
 * Draws a string on a button.  Gets the button rect via
 * adjust_buttongroup_rects, queries the blit rect, then draws
 * strong text centered on the rect. */

void dm2_v1_gfx_str_draw_button_str(DM2_V1_GfxStrState *state,
                                     int32_t buttongroup, int32_t rect_id,
                                     uint8_t fg_color, uint8_t bg_color,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx)
{
    int16_t rx, ry, rw, rh, srcx, srcy;
    int16_t tw, th;
    uint8_t *bmp;
    int16_t bmpw;

    if (state == NULL || cb == NULL || text == NULL) return;

    if (cb->adjust_buttongroup_rects)
        cb->adjust_buttongroup_rects(ctx, buttongroup);

    if (cb->query_blit_rect == NULL) return;
    if (!cb->query_blit_rect(ctx, rect_id, &rx, &ry, &rw, &rh, &srcx, &srcy))
        return;

    /* Get bitmap for drawing */
    bmp = NULL;
    bmpw = 0;
    if (cb->get_bmp)
        bmp = cb->get_bmp(ctx, rect_id, &bmpw);
    if (bmp == NULL) {
        bmp = cb->get_dm2screen ? cb->get_dm2screen(ctx) : NULL;
        bmpw = 320; /* default screen width */
    }
    if (bmp == NULL) return;

    /* Measure and center text */
    dm2_v1_gfx_str_query_str_metrics(state, text, &tw, &th);
    rx = (int16_t)(rx + (rw - tw) / 2);
    ry = (int16_t)(ry + (rh - th) / 2);

    dm2_v1_gfx_str_draw_strong_text(state, bmp, bmpw,
                                     rx, ry, fg_color, bg_color,
                                     text, cb, ctx);
}

/* ── DM2_DRAW_NAME_STR ────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_NAME_STR
 *
 * Like DRAW_BUTTON_STR but left-aligned instead of centered. */

void dm2_v1_gfx_str_draw_name_str(DM2_V1_GfxStrState *state,
                                   int32_t buttongroup, int32_t rect_id,
                                   uint8_t fg_color, uint8_t bg_color,
                                   const char *text,
                                   const DM2_V1_GfxStrCallbacks *cb,
                                   void *ctx)
{
    int16_t rx, ry, rw, rh, srcx, srcy;
    int16_t th;
    uint8_t *bmp;
    int16_t bmpw;

    if (state == NULL || cb == NULL || text == NULL) return;

    if (cb->adjust_buttongroup_rects)
        cb->adjust_buttongroup_rects(ctx, buttongroup);

    if (cb->query_blit_rect == NULL) return;
    if (!cb->query_blit_rect(ctx, rect_id, &rx, &ry, &rw, &rh, &srcx, &srcy))
        return;

    bmp = NULL;
    bmpw = 0;
    if (cb->get_bmp)
        bmp = cb->get_bmp(ctx, rect_id, &bmpw);
    if (bmp == NULL) {
        bmp = cb->get_dm2screen ? cb->get_dm2screen(ctx) : NULL;
        bmpw = 320;
    }
    if (bmp == NULL) return;

    dm2_v1_gfx_str_query_str_metrics(state, text, NULL, &th);
    ry = (int16_t)(ry + (rh - th) / 2);

    dm2_v1_gfx_str_draw_strong_text(state, bmp, bmpw,
                                     rx, ry, fg_color, bg_color,
                                     text, cb, ctx);
}

/* ── DM2_DRAW_VP_STR ──────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_VP_STR
 *
 * Draws text to the viewport bitmap (pictbuff). */

void dm2_v1_gfx_str_draw_vp_str(DM2_V1_GfxStrState *state,
                                  int16_t x, int16_t y,
                                  uint8_t color, const char *text,
                                  const DM2_V1_GfxStrCallbacks *cb,
                                  void *ctx)
{
    uint8_t *pictbuff;

    if (state == NULL || cb == NULL || text == NULL) return;
    if (cb->get_pictbuff == NULL) return;

    pictbuff = cb->get_pictbuff(ctx);
    if (pictbuff == NULL) return;

    /* Source: draws with color as fg, 0 as bg on the viewport bitmap.
     * Viewport bitmap width is 224 pixels in DM2. */
    dm2_v1_gfx_str_draw_string(state, pictbuff, 224,
                                x, y, color, 0, text, cb, ctx);
}

/* ── DM2_DRAW_GUIDED_STR ─────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_GUIDED_STR
 *
 * Word-wrapped text rendering to viewport.  Uses strx/stry from state
 * as current position, advances stry by strxplus per line. */

void dm2_v1_gfx_str_draw_guided_str(DM2_V1_GfxStrState *state,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx)
{
    uint8_t *pictbuff;
    const char *p;
    const char *word_start;
    int16_t word_w;
    int16_t vp_width = 224; /* DM2 viewport width */

    if (state == NULL || cb == NULL || text == NULL) return;
    if (cb->get_pictbuff == NULL) return;

    pictbuff = cb->get_pictbuff(ctx);
    if (pictbuff == NULL) return;

    p = text;
    while (*p != '\0') {
        /* Find next word boundary */
        word_start = p;
        word_w = 0;
        while (*p != '\0' && *p != ' ' && *p != '\n') {
            if (word_w > 0) word_w += state->gfxstrw2;
            word_w += state->gfxstrw4;
            p++;
        }

        /* Check if word fits on current line */
        if (state->strx + word_w > vp_width && state->strx > 0) {
            state->strx = 0;
            state->stry += state->strxplus;
        }

        /* Draw the word character by character */
        while (word_start < p) {
            dm2_v1_gfx_str_query_font(state, (uint8_t)*word_start, 1, 0);
            if (cb->blit) {
                cb->blit(ctx, state->font, pictbuff,
                         0, 0, state->gfxstrw4, state->gfxstrw1,
                         state->strx, state->stry,
                         state->gfxstrw4, vp_width);
            }
            state->strx += state->gfxstrw4 + state->gfxstrw2;
            word_start++;
        }

        /* Handle space/newline */
        if (*p == ' ') {
            state->strx += state->gfxstrw4;
            p++;
        } else if (*p == '\n') {
            state->strx = 0;
            state->stry += state->strxplus;
            p++;
        }
    }
}

/* ── DM2_PRINT_SYSERR_TEXT ────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_PRINT_SYSERR_TEXT
 *
 * Draws text directly to the screen buffer (dm2screen).  Used for
 * system error messages that bypass normal rendering. */

void dm2_v1_gfx_str_print_syserr_text(DM2_V1_GfxStrState *state,
                                       int16_t x, int16_t y,
                                       uint8_t fg_color, uint8_t bg_color,
                                       const char *text,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx)
{
    uint8_t *screen;

    if (state == NULL || cb == NULL || text == NULL) return;
    if (cb->get_dm2screen == NULL) return;

    screen = cb->get_dm2screen(ctx);
    if (screen == NULL) return;

    /* Source: draws directly to screen at 320 width */
    dm2_v1_gfx_str_draw_string(state, screen, 320,
                                x, y, fg_color, bg_color,
                                text, cb, ctx);
}

/* ── DM2_DRAW_VP_RC_STR ──────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_VP_RC_STR
 *
 * Draws text centered within a viewport rect. */

void dm2_v1_gfx_str_draw_vp_rc_str(DM2_V1_GfxStrState *state,
                                     int32_t rect_id, uint8_t color,
                                     const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx)
{
    int16_t rx, ry, rw, rh;
    int16_t tw, th;

    if (state == NULL || cb == NULL || text == NULL) return;

    if (cb->query_expanded_rect == NULL) return;
    if (!cb->query_expanded_rect(ctx, rect_id, &rx, &ry, &rw, &rh))
        return;

    dm2_v1_gfx_str_query_str_metrics(state, text, &tw, &th);

    /* Center horizontally within rect */
    rx = (int16_t)(rx + (rw - tw) / 2);

    dm2_v1_gfx_str_draw_vp_str(state, rx, ry, color, text, cb, ctx);
}

/* ── DM2_DRAW_LOCAL_TEXT ──────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_LOCAL_TEXT
 *
 * Draws strong text to a bitmap identified by rect_id. */

void dm2_v1_gfx_str_draw_local_text(DM2_V1_GfxStrState *state,
                                     int32_t rect_id, uint8_t fg_color,
                                     uint8_t bg_color, const char *text,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx)
{
    int16_t rx, ry, rw, rh, srcx, srcy;
    uint8_t *bmp;
    int16_t bmpw;

    if (state == NULL || cb == NULL || text == NULL) return;

    if (cb->query_blit_rect == NULL) return;
    if (!cb->query_blit_rect(ctx, rect_id, &rx, &ry, &rw, &rh, &srcx, &srcy))
        return;

    bmp = NULL;
    bmpw = 0;
    if (cb->get_bmp)
        bmp = cb->get_bmp(ctx, rect_id, &bmpw);
    if (bmp == NULL) return;

    dm2_v1_gfx_str_draw_strong_text(state, bmp, bmpw,
                                     rx, ry, fg_color, bg_color,
                                     text, cb, ctx);
}

/* ── DM2_FORMAT_SKSTR ─────────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_FORMAT_SKSTR
 *
 * The original does not use alphabetic .Za--.Zz escapes.  Its .Z directives
 * contain three decimal digits (0..28) and resolve through shared ddat/party/
 * GDAT state (c_gfx_str.cpp:290-557).  This narrow public adapter has no
 * authenticated owner for that complete state.  Keep every byte literal until
 * that owner is bound: inventing hero names, buffers, or newlines would turn
 * an original text record into synthetic runtime text. */

void dm2_v1_gfx_str_format_skstr(const char *src, char *dest,
                                  const DM2_V1_GfxStrCallbacks *cb,
                                  void *ctx)
{
    const char *s;
    char *d;

    if (src == NULL || dest == NULL) {
        if (dest) dest[0] = '\0';
        return;
    }

    s = src;
    d = dest;

    (void)cb;
    (void)ctx;
    while (*s != '\0')
        *d++ = *s++;

    *d = '\0';
}

/* ── DM2_QUERY_GDAT_TEXT ──────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_QUERY_GDAT_TEXT
 *
 * Queries game data for a text entry, decrypts it with XOR decryption
 * loop, then passes through FORMAT_SKSTR. */

bool dm2_v1_gfx_str_query_gdat_text(DM2_V1_GfxStrState *state,
                                     int32_t cls, int32_t sub,
                                     int32_t idx, char *buf,
                                     const DM2_V1_GfxStrCallbacks *cb,
                                     void *ctx)
{
    uint8_t raw[256];
    char formatted[256];
    int32_t len;
    int32_t i;
    uint8_t key;

    if (buf == NULL) return false;
    buf[0] = '\0';

    if (state == NULL || cb == NULL) return false;
    if (cb->query_gdat_entry_data_length == NULL) return false;
    if (cb->query_gdat_entry_data_buff == NULL) return false;

    len = cb->query_gdat_entry_data_length(ctx, cls, sub, idx);
    if (len <= 0 || len > (int32_t)sizeof(raw) - 1) return false;

    if (!cb->query_gdat_entry_data_buff(ctx, cls, sub, idx, raw, len))
        return false;

    /* Source: c_gfx_str.cpp DM2_QUERY_GDAT_TEXT decryption loop.
     * XOR each byte with rotating key starting from (len & 0xFF). */
    key = (uint8_t)(len & 0xFF);
    for (i = 0; i < len; i++) {
        raw[i] ^= key;
        key = (uint8_t)(key + 3);
    }
    raw[len] = '\0';

    /* Pass through FORMAT_SKSTR for variable substitution */
    dm2_v1_gfx_str_format_skstr((const char *)raw, formatted, cb, ctx);

    /* Copy to output buffer */
    {
        const char *s = formatted;
        char *d = buf;
        while (*s != '\0') *d++ = *s++;
        *d = '\0';
    }

    return true;
}

/* ── DM2_DRAW_TEXT_TO_BACKBUFF ────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DRAW_TEXT_TO_BACKBUFF
 *
 * Draws text to the backbuffer using a rect for positioning. */

void dm2_v1_gfx_str_draw_text_to_backbuff(DM2_V1_GfxStrState *state,
                                            int32_t rect_id, int16_t y_off,
                                            const char *text,
                                            const DM2_V1_GfxStrCallbacks *cb,
                                            void *ctx)
{
    int16_t rx, ry;
    uint8_t *screen;

    if (state == NULL || cb == NULL || text == NULL) return;

    if (cb->query_topleft_of_rect == NULL) return;
    if (!cb->query_topleft_of_rect(ctx, rect_id, &rx, &ry)) return;

    if (cb->get_dm2screen == NULL) return;
    screen = cb->get_dm2screen(ctx);
    if (screen == NULL) return;

    ry = (int16_t)(ry + y_off);

    dm2_v1_gfx_str_draw_string(state, screen, 320,
                                rx, ry, 1, 0, text, cb, ctx);
}

/* ── DM2_gfxstr_3929_04e2 — word-wrap helper ─────────────────────── */
/* Source: c_gfx_str.cpp DM2_gfxstr_3929_04e2
 *
 * Copies characters from src to dest starting at pos, wrapping at
 * max_width pixel boundary.  Returns updated position in dest.
 * Inserts newlines at word boundaries when line exceeds max_width. */

int32_t dm2_v1_gfx_str_word_wrap(const DM2_V1_GfxStrState *state,
                                  const char *src, char *dest,
                                  int32_t pos, int32_t max_width)
{
    int32_t line_w = 0;
    int32_t last_space_pos = -1;
    int32_t dp;
    const char *s;

    if (state == NULL || src == NULL || dest == NULL) return pos;

    dp = pos;
    s = src;

    while (*s != '\0') {
        int16_t char_w = state->gfxstrw4 + state->gfxstrw2;

        if (*s == ' ') {
            last_space_pos = dp;
        }

        if (line_w + state->gfxstrw4 > max_width && last_space_pos >= 0) {
            /* Replace last space with newline */
            dest[last_space_pos] = '\n';
            line_w = 0;
            /* Recount width from after the space */
            {
                int32_t k;
                for (k = last_space_pos + 1; k < dp; k++) {
                    line_w += char_w;
                }
            }
            last_space_pos = -1;
        }

        dest[dp++] = *s;
        line_w += char_w;

        if (*s == '\n') {
            line_w = 0;
            last_space_pos = -1;
        }

        s++;
    }

    dest[dp] = '\0';
    return dp;
}

/* ── DM2_gfxstr_24a5_0732 — draw uppercase text ─────────────────── */
/* Source: c_gfx_str.cpp DM2_gfxstr_24a5_0732
 *
 * Converts text to uppercase then draws it to the viewport. */

void dm2_v1_gfx_str_draw_uppercase(DM2_V1_GfxStrState *state,
                                    int16_t x, int16_t y,
                                    const char *text,
                                    const DM2_V1_GfxStrCallbacks *cb,
                                    void *ctx)
{
    char upper[256];
    int i;

    if (state == NULL || text == NULL) return;

    for (i = 0; text[i] != '\0' && i < 255; i++) {
        char c = text[i];
        if (c >= 'a' && c <= 'z')
            upper[i] = (char)(c - 32);
        else
            upper[i] = c;
    }
    upper[i] = '\0';

    dm2_v1_gfx_str_draw_vp_str(state, x, y, 1, upper, cb, ctx);
}

/* ── DM2_DISPLAY_HINT_TEXT ────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_DISPLAY_HINT_TEXT
 *
 * Displays a hint text in the scrollbox area.  Sets up palette,
 * fills the pict buffer, draws text, then triggers scrollbox. */

void dm2_v1_gfx_str_display_hint_text(DM2_V1_GfxStrState *state,
                                       int32_t palette_idx,
                                       const char *text,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx)
{
    uint8_t *pictbuff;

    if (state == NULL || cb == NULL || text == NULL) return;

    /* Source: copies palette, fills pict, draws guided string,
     * then calls scrollbox_message. */
    if (cb->copy_small_palette)
        cb->copy_small_palette(ctx, palette_idx, 0);

    if (cb->get_pictbuff == NULL) return;
    pictbuff = cb->get_pictbuff(ctx);
    if (pictbuff == NULL) return;

    if (cb->fill_entire_pict)
        cb->fill_entire_pict(ctx, pictbuff, 0);

    /* Reset guided string position */
    state->strx = 0;
    state->stry = 0;

    dm2_v1_gfx_str_draw_guided_str(state, text, cb, ctx);

    if (cb->scrollbox_message)
        cb->scrollbox_message(ctx);
}

/* ── DM2_SCROLLBOX_MESSAGE ────────────────────────────────────────── */
/* Source: c_gfx_str.cpp DM2_SCROLLBOX_MESSAGE
 *
 * Scrolls the message box area.  Delegates to the scrollbox_message
 * callback and gfx_main helpers for screen update. */

void dm2_v1_gfx_str_scrollbox_message(DM2_V1_GfxStrState *state,
                                       const DM2_V1_GfxStrCallbacks *cb,
                                       void *ctx)
{
    if (state == NULL || cb == NULL) return;

    /* Source: c_gfx_str.cpp DM2_SCROLLBOX_MESSAGE calls gfxstr_3929_0914
     * and gfxstr_3929_0929 for scroll region management, then
     * blit_within_screen to scroll pixels. */
    if (cb->gfxstr_3929_0914)
        cb->gfxstr_3929_0914(ctx, 0, 0, 224, 14);

    if (cb->blit_within_screen)
        cb->blit_within_screen(ctx, 0, 7, 224, 7, 0, 0);

    if (cb->gfxstr_3929_0929)
        cb->gfxstr_3929_0929(ctx, 0, 7, 224, 7);

    if (cb->scrollbox_message)
        cb->scrollbox_message(ctx);
}
