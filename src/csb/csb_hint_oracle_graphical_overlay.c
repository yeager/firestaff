/*
 * csb_hint_oracle_graphical_overlay.c
 *
 * Headless graphical boundary for CSB Hint Oracle pages. This is a
 * presentation-only framebuffer renderer: it consumes a staged decoded
 * page and draws a compact framed oracle panel. It does not inspect or
 * classify Utility Disk variants.
 */

#include "csb_hint_oracle_graphical_overlay.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

const char *csb_hint_oracle_overlay_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_OVERLAY_OK: return "OK";
    case CSB_HINT_ORACLE_OVERLAY_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_OVERLAY_ERR_NOT_LOADED: return "not-loaded";
    case CSB_HINT_ORACLE_OVERLAY_ERR_HINT_OUT_OF_RANGE: return "hint-out-of-range";
    case CSB_HINT_ORACLE_OVERLAY_ERR_DECODE: return "decode";
    case CSB_HINT_ORACLE_OVERLAY_ERR_GEOMETRY: return "geometry";
    default: return "unknown";
    }
}

void csb_hint_oracle_overlay_default_config(
    CSB_HintOracleOverlay_Config *cfg)
{
    if (!cfg) {
        return;
    }
    cfg->x = 28;
    cfg->y = 20;
    cfg->w = 264;
    cfg->h = 152;
    cfg->background = 2u;
    cfg->border = 14u;
    cfg->title = 15u;
    cfg->text = 12u;
    cfg->shadow = 1u;
}

static void stats_clear(CSB_HintOracleOverlay_Stats *stats)
{
    if (stats) {
        memset(stats, 0, sizeof(*stats));
    }
}

static void put_pixel(uint8_t *fb, int fb_w, int fb_h,
                      int x, int y, uint8_t color, size_t *counter)
{
    if (!fb || x < 0 || y < 0 || x >= fb_w || y >= fb_h) {
        return;
    }
    fb[(size_t)y * (size_t)fb_w + (size_t)x] = color;
    if (counter) {
        ++(*counter);
    }
}

static void fill_rect(uint8_t *fb, int fb_w, int fb_h,
                      int x, int y, int w, int h, uint8_t color,
                      size_t *counter)
{
    int yy;
    int xx;
    if (!fb || w <= 0 || h <= 0) {
        return;
    }
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            put_pixel(fb, fb_w, fb_h, x + xx, y + yy, color, counter);
        }
    }
}

static const uint8_t *glyph_rows(char c)
{
    static const uint8_t blank[7] = {0, 0, 0, 0, 0, 0, 0};
    static const uint8_t unknown[7] = {14, 17, 1, 2, 4, 0, 4};

    static const uint8_t A[7] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t B[7] = {30, 17, 17, 30, 17, 17, 30};
    static const uint8_t C[7] = {14, 17, 16, 16, 16, 17, 14};
    static const uint8_t D[7] = {30, 17, 17, 17, 17, 17, 30};
    static const uint8_t E[7] = {31, 16, 16, 30, 16, 16, 31};
    static const uint8_t F[7] = {31, 16, 16, 30, 16, 16, 16};
    static const uint8_t G[7] = {14, 17, 16, 23, 17, 17, 14};
    static const uint8_t H[7] = {17, 17, 17, 31, 17, 17, 17};
    static const uint8_t I[7] = {14, 4, 4, 4, 4, 4, 14};
    static const uint8_t J[7] = {7, 2, 2, 2, 18, 18, 12};
    static const uint8_t K[7] = {17, 18, 20, 24, 20, 18, 17};
    static const uint8_t L[7] = {16, 16, 16, 16, 16, 16, 31};
    static const uint8_t M[7] = {17, 27, 21, 21, 17, 17, 17};
    static const uint8_t N[7] = {17, 25, 21, 19, 17, 17, 17};
    static const uint8_t O[7] = {14, 17, 17, 17, 17, 17, 14};
    static const uint8_t P[7] = {30, 17, 17, 30, 16, 16, 16};
    static const uint8_t Q[7] = {14, 17, 17, 17, 21, 18, 13};
    static const uint8_t R[7] = {30, 17, 17, 30, 20, 18, 17};
    static const uint8_t S[7] = {15, 16, 16, 14, 1, 1, 30};
    static const uint8_t T[7] = {31, 4, 4, 4, 4, 4, 4};
    static const uint8_t U[7] = {17, 17, 17, 17, 17, 17, 14};
    static const uint8_t V[7] = {17, 17, 17, 17, 17, 10, 4};
    static const uint8_t W[7] = {17, 17, 17, 21, 21, 21, 10};
    static const uint8_t X[7] = {17, 17, 10, 4, 10, 17, 17};
    static const uint8_t Y[7] = {17, 17, 10, 4, 4, 4, 4};
    static const uint8_t Z[7] = {31, 1, 2, 4, 8, 16, 31};

    static const uint8_t D0[7] = {14, 17, 19, 21, 25, 17, 14};
    static const uint8_t D1[7] = {4, 12, 4, 4, 4, 4, 14};
    static const uint8_t D2[7] = {14, 17, 1, 2, 4, 8, 31};
    static const uint8_t D3[7] = {30, 1, 1, 14, 1, 1, 30};
    static const uint8_t D4[7] = {2, 6, 10, 18, 31, 2, 2};
    static const uint8_t D5[7] = {31, 16, 16, 30, 1, 1, 30};
    static const uint8_t D6[7] = {14, 16, 16, 30, 17, 17, 14};
    static const uint8_t D7[7] = {31, 1, 2, 4, 8, 8, 8};
    static const uint8_t D8[7] = {14, 17, 17, 14, 17, 17, 14};
    static const uint8_t D9[7] = {14, 17, 17, 15, 1, 1, 14};

    static const uint8_t period[7] = {0, 0, 0, 0, 0, 12, 12};
    static const uint8_t comma[7] = {0, 0, 0, 0, 0, 12, 8};
    static const uint8_t colon[7] = {0, 12, 12, 0, 12, 12, 0};
    static const uint8_t slash[7] = {1, 1, 2, 4, 8, 16, 16};
    static const uint8_t dash[7] = {0, 0, 0, 31, 0, 0, 0};
    static const uint8_t apostrophe[7] = {12, 4, 8, 0, 0, 0, 0};
    static const uint8_t quote[7] = {10, 10, 0, 0, 0, 0, 0};
    static const uint8_t bang[7] = {4, 4, 4, 4, 4, 0, 4};
    static const uint8_t lparen[7] = {2, 4, 8, 8, 8, 4, 2};
    static const uint8_t rparen[7] = {8, 4, 2, 2, 2, 4, 8};
    static const uint8_t plus[7] = {0, 4, 4, 31, 4, 4, 0};

    if (c >= 'a' && c <= 'z') {
        c = (char)toupper((unsigned char)c);
    }
    switch (c) {
    case ' ': return blank;
    case 'A': return A;
    case 'B': return B;
    case 'C': return C;
    case 'D': return D;
    case 'E': return E;
    case 'F': return F;
    case 'G': return G;
    case 'H': return H;
    case 'I': return I;
    case 'J': return J;
    case 'K': return K;
    case 'L': return L;
    case 'M': return M;
    case 'N': return N;
    case 'O': return O;
    case 'P': return P;
    case 'Q': return Q;
    case 'R': return R;
    case 'S': return S;
    case 'T': return T;
    case 'U': return U;
    case 'V': return V;
    case 'W': return W;
    case 'X': return X;
    case 'Y': return Y;
    case 'Z': return Z;
    case '0': return D0;
    case '1': return D1;
    case '2': return D2;
    case '3': return D3;
    case '4': return D4;
    case '5': return D5;
    case '6': return D6;
    case '7': return D7;
    case '8': return D8;
    case '9': return D9;
    case '.': return period;
    case ',': return comma;
    case ':': return colon;
    case '/': return slash;
    case '-': return dash;
    case '\'': return apostrophe;
    case '"': return quote;
    case '!': return bang;
    case '(': return lparen;
    case ')': return rparen;
    case '+': return plus;
    default: return unknown;
    }
}

static size_t draw_char(uint8_t *fb, int fb_w, int fb_h,
                        int x, int y, char c, uint8_t color)
{
    const uint8_t *rows = glyph_rows(c);
    size_t count = 0u;
    int yy;
    int xx;
    for (yy = 0; yy < 7; ++yy) {
        for (xx = 0; xx < 5; ++xx) {
            if ((rows[yy] & (uint8_t)(1u << (4 - xx))) != 0u) {
                put_pixel(fb, fb_w, fb_h, x + xx, y + yy, color, &count);
            }
        }
    }
    return count;
}

static int printable_char(unsigned char c)
{
    if (c == '\r' || c == '\n' || c == '\t') {
        return 1;
    }
    return c >= 0x20u && c <= 0x7eu;
}

static char sanitize_char(unsigned char c)
{
    if (c == '\t') {
        return ' ';
    }
    if (c == '\r') {
        return '\n';
    }
    if (!printable_char(c)) {
        return '?';
    }
    return (char)c;
}

static void draw_wrapped_text(const char *text,
                              uint8_t *fb,
                              int fb_w,
                              int fb_h,
                              int x,
                              int y,
                              int max_cols,
                              int max_lines,
                              uint8_t color,
                              CSB_HintOracleOverlay_Stats *stats)
{
    int col = 0;
    int line = 0;
    const unsigned char *p = (const unsigned char *)text;

    if (!text || max_cols <= 0 || max_lines <= 0) {
        return;
    }
    while (*p != '\0') {
        char c = sanitize_char(*p++);
        if (c == '\n') {
            if (line + 1 >= max_lines) {
                if (stats) {
                    stats->clipped = 1;
                }
                return;
            }
            ++line;
            col = 0;
            continue;
        }
        if (col >= max_cols) {
            if (line + 1 >= max_lines) {
                if (stats) {
                    stats->clipped = 1;
                }
                return;
            }
            ++line;
            col = 0;
            while (c == ' ' && *p != '\0') {
                c = sanitize_char(*p++);
            }
        }
        if (c != ' ') {
            size_t pixels = draw_char(fb, fb_w, fb_h,
                                      x + col * 6,
                                      y + line * 9,
                                      c, color);
            if (stats) {
                stats->glyph_pixels += pixels;
                ++stats->chars_drawn;
            }
        }
        ++col;
        if (line + 1 > max_lines) {
            if (stats) {
                stats->clipped = 1;
            }
            return;
        }
    }
    if (stats) {
        stats->lines_drawn = (size_t)(line + 1);
    }
}

int csb_hint_oracle_overlay_render_text(
    const char *title,
    const char *decoded_text,
    uint8_t *framebuffer,
    int framebuffer_w,
    int framebuffer_h,
    const CSB_HintOracleOverlay_Config *cfg,
    CSB_HintOracleOverlay_Stats *out_stats)
{
    CSB_HintOracleOverlay_Config local_cfg;
    int text_x;
    int title_y;
    int body_y;
    int max_cols;
    int max_lines;

    stats_clear(out_stats);
    if (!title || !decoded_text || !framebuffer ||
        framebuffer_w <= 0 || framebuffer_h <= 0) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_ARGUMENT;
    }
    if (!cfg) {
        csb_hint_oracle_overlay_default_config(&local_cfg);
        cfg = &local_cfg;
    }
    if (cfg->w < 64 || cfg->h < 48 ||
        cfg->x < 0 || cfg->y < 0 ||
        cfg->x + cfg->w > framebuffer_w ||
        cfg->y + cfg->h > framebuffer_h) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_GEOMETRY;
    }

    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x + 3, cfg->y + 3, cfg->w, cfg->h,
              cfg->shadow, NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x, cfg->y, cfg->w, cfg->h,
              cfg->background,
              out_stats ? &out_stats->background_pixels : NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x, cfg->y, cfg->w, 2,
              cfg->border,
              out_stats ? &out_stats->border_pixels : NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x, cfg->y + cfg->h - 2, cfg->w, 2,
              cfg->border,
              out_stats ? &out_stats->border_pixels : NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x, cfg->y, 2, cfg->h,
              cfg->border,
              out_stats ? &out_stats->border_pixels : NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x + cfg->w - 2, cfg->y, 2, cfg->h,
              cfg->border,
              out_stats ? &out_stats->border_pixels : NULL);
    fill_rect(framebuffer, framebuffer_w, framebuffer_h,
              cfg->x + 8, cfg->y + 18, cfg->w - 16, 1,
              cfg->border,
              out_stats ? &out_stats->border_pixels : NULL);

    text_x = cfg->x + 10;
    title_y = cfg->y + 8;
    body_y = cfg->y + 26;
    max_cols = (cfg->w - 20) / 6;
    max_lines = (cfg->h - 34) / 9;
    if (max_cols <= 0 || max_lines <= 0) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_GEOMETRY;
    }

    draw_wrapped_text(title, framebuffer, framebuffer_w, framebuffer_h,
                      text_x, title_y, max_cols, 1, cfg->title, out_stats);
    draw_wrapped_text(decoded_text, framebuffer, framebuffer_w, framebuffer_h,
                      text_x, body_y, max_cols, max_lines, cfg->text, out_stats);

    return CSB_HINT_ORACLE_OVERLAY_OK;
}

int csb_hint_oracle_overlay_render_hint(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    uint8_t *framebuffer,
    int framebuffer_w,
    int framebuffer_h,
    const CSB_HintOracleOverlay_Config *cfg,
    CSB_HintOracleOverlay_Stats *out_stats)
{
    char title[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 32u];
    char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint8_t text[CSB_HINT_ORACLE_OVERLAY_TEXT_CAP];
    size_t out_size = 0u;
    int rc;

    stats_clear(out_stats);
    if (!framebuffer || framebuffer_w <= 0 || framebuffer_h <= 0) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_ARGUMENT;
    }
    if (!cache || !cache->loaded) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_NOT_LOADED;
    }
    if (hint_index >= cache->htc.hint_count) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_HINT_OUT_OF_RANGE;
    }

    rc = csb_hint_oracle_htc_real_get_hint_name(
        cache, hint_index, hint_name, sizeof(hint_name));
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_DECODE;
    }
    rc = csb_hint_oracle_htc_real_decompress_first_page(
        cache, hint_index, text, sizeof(text) - 1u, &out_size);
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        return CSB_HINT_ORACLE_OVERLAY_ERR_DECODE;
    }
    if (out_size >= sizeof(text)) {
        out_size = sizeof(text) - 1u;
    }
    text[out_size] = '\0';

    (void)snprintf(title, sizeof(title), "CSB HINT ORACLE: %s", hint_name);
    return csb_hint_oracle_overlay_render_text(
        title, (const char *)text, framebuffer, framebuffer_w,
        framebuffer_h, cfg, out_stats);
}
