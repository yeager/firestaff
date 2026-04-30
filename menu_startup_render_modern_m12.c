/*
 * menu_startup_render_modern_m12.c
 *
 * Bounded M12 slice: modern, high-resolution, true-color startup menu.
 * See menu_startup_render_modern_m12.h for the strict scope rules.
 *
 * Style goals:
 *   - 1920x1080 HD canvas, 24-bit RGB.
 *   - Indigo -> violet vertical gradient background with subtle
 *     diagonal lighting so the output demonstrably exceeds the 16
 *     colour VGA palette (tens of thousands of distinct RGB values).
 *   - Clean panel layout with rounded corners, soft outlines, and
 *     warm gold accents.
 *   - Title treatment with a gold gradient + shadow.
 *   - Card grid for DM1 / CSB / DM2 with legible version and
 *     checksum status.
 *   - V1 / V2 / V3 presentation mode badge drawn prominently.
 *   - Shared footer with keyboard hints.
 *
 * All drawing is pure C99 - no platform code, no SDL dependency.
 * The renderer only reads the public M12_StartupMenuState.
 */

#include "menu_startup_render_modern_m12.h"

#include "asset_status_m12.h"
#include "branding_logo_readme_m12.h"
#include "card_art_m12.h"
#include "card_art_generated_m12.h"
#include "creature_art_m12.h"

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Colour helpers                                                             */
/* -------------------------------------------------------------------------- */

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} M12_RGB;

static M12_RGB rgb(unsigned char r, unsigned char g, unsigned char b) {
    M12_RGB c = {r, g, b};
    return c;
}

static unsigned char clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

static M12_RGB mix_rgb(M12_RGB a, M12_RGB b, int t, int total) {
    if (total <= 0) {
        return a;
    }
    if (t < 0) t = 0;
    if (t > total) t = total;
    M12_RGB out;
    out.r = clamp_u8(a.r + ((b.r - a.r) * t) / total);
    out.g = clamp_u8(a.g + ((b.g - a.g) * t) / total);
    out.b = clamp_u8(a.b + ((b.b - a.b) * t) / total);
    return out;
}

static M12_RGB scale_rgb(M12_RGB c, int num, int den) {
    if (den <= 0) den = 1;
    M12_RGB out;
    out.r = clamp_u8((int)c.r * num / den);
    out.g = clamp_u8((int)c.g * num / den);
    out.b = clamp_u8((int)c.b * num / den);
    return out;
}

/* Theme palette */
static M12_RGB COLOR_BG_TOP(void)      { return rgb(9,  10, 28); }
static M12_RGB COLOR_BG_BOTTOM(void)   { return rgb(38, 17, 62); }
static M12_RGB COLOR_PANEL_FILL(void)  { return rgb(22, 22, 46); }
static M12_RGB COLOR_PANEL_EDGE(void)  { return rgb(92, 82, 148); }
static M12_RGB COLOR_TEXT(void)        { return rgb(240, 236, 225); }
static M12_RGB COLOR_TEXT_DIM(void)    { return rgb(176, 170, 192); }
static M12_RGB COLOR_TEXT_FAINT(void)  { return rgb(120, 112, 148); }
static M12_RGB COLOR_ACCENT(void)      { return rgb(232, 184, 88); }
static M12_RGB COLOR_ACCENT_HI(void)   { return rgb(255, 222, 148); }
static M12_RGB COLOR_ACCENT_LO(void)   { return rgb(168, 118, 40); }
static M12_RGB COLOR_OK(void)          { return rgb(120, 200, 130); }
static M12_RGB COLOR_WARN(void)        { return rgb(220, 170, 90); }
static M12_RGB COLOR_BAD(void)         { return rgb(210, 96, 96); }
static M12_RGB COLOR_V1(void)          { return rgb(232, 184, 88); }
static M12_RGB COLOR_V2(void)          { return rgb(120, 196, 236); }
static M12_RGB COLOR_V3(void)          { return rgb(176, 132, 240); }
static M12_RGB COLOR_SHADOW(void)      { return rgb(6, 6, 14); }

/* -------------------------------------------------------------------------- */
/* Pixel plotting                                                             */
/* -------------------------------------------------------------------------- */

typedef struct {
    unsigned char* rgba;
    int w;
    int h;
} M12_ModernCanvas;

static void put_pixel(M12_ModernCanvas* c, int x, int y, M12_RGB col) {
    if (x < 0 || y < 0 || x >= c->w || y >= c->h) {
        return;
    }
    unsigned char* p = c->rgba + (y * c->w + x) * 4;
    p[0] = col.r;
    p[1] = col.g;
    p[2] = col.b;
    p[3] = 0xFF;
}

static void blend_pixel(M12_ModernCanvas* c, int x, int y, M12_RGB col, int alpha) {
    if (x < 0 || y < 0 || x >= c->w || y >= c->h) {
        return;
    }
    if (alpha <= 0) return;
    if (alpha >= 255) {
        put_pixel(c, x, y, col);
        return;
    }
    unsigned char* p = c->rgba + (y * c->w + x) * 4;
    int invA = 255 - alpha;
    p[0] = clamp_u8((p[0] * invA + col.r * alpha) / 255);
    p[1] = clamp_u8((p[1] * invA + col.g * alpha) / 255);
    p[2] = clamp_u8((p[2] * invA + col.b * alpha) / 255);
    p[3] = 0xFF;
}

static void fill_rect(M12_ModernCanvas* c, int x, int y, int w, int h, M12_RGB col) {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            put_pixel(c, xx, yy, col);
        }
    }
}

static void hline(M12_ModernCanvas* c, int x, int y, int w, M12_RGB col) {
    for (int i = 0; i < w; ++i) put_pixel(c, x + i, y, col);
}
static void vline(M12_ModernCanvas* c, int x, int y, int h, M12_RGB col) {
    for (int i = 0; i < h; ++i) put_pixel(c, x, y + i, col);
}

/* Soft rounded rectangle: fill, then pinch corners and stroke. */
static void fill_rounded_rect(M12_ModernCanvas* c, int x, int y, int w, int h, int radius, M12_RGB col) {
    if (radius < 0) radius = 0;
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    for (int yy = 0; yy < h; ++yy) {
        for (int xx = 0; xx < w; ++xx) {
            int dx = 0;
            int dy = 0;
            if (xx < radius)          dx = radius - xx;
            else if (xx >= w - radius) dx = xx - (w - radius - 1);
            if (yy < radius)          dy = radius - yy;
            else if (yy >= h - radius) dy = yy - (h - radius - 1);
            if (dx > 0 && dy > 0) {
                if (dx * dx + dy * dy > radius * radius) {
                    continue;
                }
            }
            put_pixel(c, x + xx, y + yy, col);
        }
    }
}

static void stroke_rounded_rect(M12_ModernCanvas* c, int x, int y, int w, int h, int radius, M12_RGB col) {
    /* Approximate by drawing a filled rounded rect one pixel bigger and
     * subtracting... but for simplicity: draw 4 straight edges, then
     * corner arcs. */
    if (radius < 0) radius = 0;
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    hline(c, x + radius, y, w - 2 * radius, col);
    hline(c, x + radius, y + h - 1, w - 2 * radius, col);
    vline(c, x, y + radius, h - 2 * radius, col);
    vline(c, x + w - 1, y + radius, h - 2 * radius, col);
    for (int i = 0; i <= radius; ++i) {
        for (int j = 0; j <= radius; ++j) {
            int d2 = i * i + j * j;
            if (d2 <= radius * radius && d2 >= (radius - 1) * (radius - 1)) {
                put_pixel(c, x + radius - i, y + radius - j, col);
                put_pixel(c, x + w - 1 - (radius - i), y + radius - j, col);
                put_pixel(c, x + radius - i, y + h - 1 - (radius - j), col);
                put_pixel(c, x + w - 1 - (radius - i), y + h - 1 - (radius - j), col);
            }
        }
    }
}

/* Vertical gradient fill (rect spans y0..y1). */
static void fill_vgradient(M12_ModernCanvas* c, int x, int y, int w, int h, M12_RGB top, M12_RGB bot) {
    for (int yy = 0; yy < h; ++yy) {
        M12_RGB row = mix_rgb(top, bot, yy, h - 1);
        for (int xx = 0; xx < w; ++xx) {
            put_pixel(c, x + xx, y + yy, row);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Background with subtle diagonal highlight + noise grid                     */
/* -------------------------------------------------------------------------- */

static void draw_background(M12_ModernCanvas* c) {
    M12_RGB top = COLOR_BG_TOP();
    M12_RGB bot = COLOR_BG_BOTTOM();
    for (int y = 0; y < c->h; ++y) {
        M12_RGB row = mix_rgb(top, bot, y, c->h - 1);
        for (int x = 0; x < c->w; ++x) {
            /* Diagonal highlight: soft brighten along the
             * top-left to bottom-right axis. */
            int diag = (x + y) - (c->w + c->h) / 4;
            int highlight = 0;
            if (diag > 0 && diag < (c->w + c->h) / 2) {
                highlight = (diag * 14) / ((c->w + c->h) / 2);
            }
            int r = row.r + highlight;
            int g = row.g + highlight;
            int b = row.b + highlight / 2;
            /* Stable deterministic texture noise so same state ->
             * same output. */
            int n = ((x * 131 + y * 197 + (x ^ y) * 17) & 0x1F) - 16;
            r += n / 8;
            g += n / 10;
            b += n / 6;
            put_pixel(c, x, y, rgb(clamp_u8(r), clamp_u8(g), clamp_u8(b)));
        }
    }

    /* Faint star field in the upper half (deterministic). */
    for (int i = 0; i < 180; ++i) {
        int x = (i * 3571) % c->w;
        int y = (i * 1607) % (c->h / 2);
        int brightness = 120 + (i * 37) % 110;
        M12_RGB star = rgb(clamp_u8(brightness + 30), clamp_u8(brightness + 10), clamp_u8(brightness));
        blend_pixel(c, x, y, star, 180);
        blend_pixel(c, x + 1, y, star, 80);
        blend_pixel(c, x, y + 1, star, 80);
    }
}

/* -------------------------------------------------------------------------- */
/* Font: compact 5x7 monospaced (A-Z, 0-9, punctuation). Rendered at any       */
/* integer scale with soft shadow for legibility.                             */
/* -------------------------------------------------------------------------- */

typedef struct {
    char ch;
    unsigned char rows[7]; /* each byte: bit4 ... bit0 = leftmost ... rightmost */
} ModernGlyph;

static const ModernGlyph g_glyphs[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'!', {4, 4, 4, 4, 4, 0, 4}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {',', {0, 0, 0, 0, 0, 12, 4}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 1, 2, 4, 8, 16, 16}},
    {'>', {16, 8, 4, 2, 4, 8, 16}},
    {'<', {1, 2, 4, 8, 4, 2, 1}},
    {'+', {0, 4, 4, 31, 4, 4, 0}},
    {'(', {6, 8, 16, 16, 16, 8, 6}},
    {')', {12, 2, 1, 1, 1, 2, 12}},
    {'[', {14, 8, 8, 8, 8, 8, 14}},
    {']', {14, 2, 2, 2, 2, 2, 14}},
    {'\'', {4, 4, 0, 0, 0, 0, 0}},
    {'"', {10, 10, 0, 0, 0, 0, 0}},
    {'%', {17, 18, 4, 4, 4, 9, 17}},
    {'#', {10, 10, 31, 10, 31, 10, 10}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {7, 2, 2, 2, 18, 18, 12}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 21, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 21, 21, 21, 10}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}}
};

static const ModernGlyph* find_glyph(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }
    for (size_t i = 0; i < sizeof(g_glyphs) / sizeof(g_glyphs[0]); ++i) {
        if (g_glyphs[i].ch == ch) {
            return &g_glyphs[i];
        }
    }
    return &g_glyphs[0]; /* space */
}

typedef struct {
    int scale;
    int tracking;       /* extra pixels between glyphs at unit scale (x scale) */
    int shadow;         /* 0 = off, positive = offset magnitude */
    M12_RGB color;
    M12_RGB shadowColor;
} ModernTextStyle;

static ModernTextStyle text_style_make(int scale, M12_RGB color, int shadow) {
    ModernTextStyle s;
    s.scale = scale < 1 ? 1 : scale;
    s.tracking = 1;
    s.shadow = shadow;
    s.color = color;
    s.shadowColor = COLOR_SHADOW();
    return s;
}

static int glyph_width_px(int scale, int tracking) {
    return (5 + tracking) * scale;
}

static int text_width_px(const char* s, const ModernTextStyle* st) {
    if (!s) return 0;
    int n = (int)strlen(s);
    int w = n * glyph_width_px(st->scale, st->tracking);
    w -= st->tracking * st->scale;
    if (w < 0) w = 0;
    return w;
}

static void draw_glyph(M12_ModernCanvas* c, int x, int y,
                       const ModernGlyph* g, const ModernTextStyle* st) {
    int scale = st->scale;
    for (int row = 0; row < 7; ++row) {
        unsigned char bits = g->rows[row];
        for (int col = 0; col < 5; ++col) {
            int on = (bits >> (4 - col)) & 1;
            if (!on) continue;
            int px = x + col * scale;
            int py = y + row * scale;
            if (st->shadow > 0) {
                fill_rect(c,
                          px + st->shadow, py + st->shadow,
                          scale, scale,
                          st->shadowColor);
            }
            fill_rect(c, px, py, scale, scale, st->color);
        }
    }
}

static void draw_text(M12_ModernCanvas* c, int x, int y,
                      const char* s, const ModernTextStyle* st) {
    if (!s) return;
    int step = glyph_width_px(st->scale, st->tracking);
    for (int i = 0; s[i] != '\0'; ++i) {
        const ModernGlyph* g = find_glyph(s[i]);
        draw_glyph(c, x + i * step, y, g, st);
    }
}

static void draw_text_centered(M12_ModernCanvas* c, int cx, int y,
                               const char* s, const ModernTextStyle* st) {
    int w = text_width_px(s, st);
    draw_text(c, cx - w / 2, y, s, st);
}

static void draw_text_gradient(M12_ModernCanvas* c, int x, int y, const char* s,
                               int scale, M12_RGB top, M12_RGB bot, M12_RGB shadow) {
    if (!s) return;
    ModernTextStyle sh = text_style_make(scale, shadow, 0);
    /* Shadow pass */
    for (int dy = 2; dy <= 4; ++dy) {
        ModernTextStyle ps = sh;
        ps.color = shadow;
        draw_text(c, x + dy, y + dy, s, &ps);
    }
    /* Gradient scan: draw each row of each glyph at a mixed colour. */
    int step = glyph_width_px(scale, 1);
    int rowTotal = 7 * scale - 1;
    for (int i = 0; s[i] != '\0'; ++i) {
        const ModernGlyph* g = find_glyph(s[i]);
        for (int row = 0; row < 7; ++row) {
            unsigned char bits = g->rows[row];
            for (int col = 0; col < 5; ++col) {
                int on = (bits >> (4 - col)) & 1;
                if (!on) continue;
                for (int py = 0; py < scale; ++py) {
                    int yy = row * scale + py;
                    M12_RGB c_ = mix_rgb(top, bot, yy, rowTotal);
                    fill_rect(c,
                              x + i * step + col * scale,
                              y + yy,
                              scale, 1,
                              c_);
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Layout & panels                                                            */
/* -------------------------------------------------------------------------- */

static void draw_panel(M12_ModernCanvas* c, int x, int y, int w, int h,
                       M12_RGB fill, M12_RGB edge, int radius) {
    /* Drop shadow */
    for (int i = 1; i <= 10; ++i) {
        int alpha = 40 - i * 3;
        if (alpha <= 0) break;
        for (int xx = x + i; xx < x + w + i; ++xx) {
            blend_pixel(c, xx, y + h + i, COLOR_SHADOW(), alpha);
        }
        for (int yy = y + i; yy < y + h + i; ++yy) {
            blend_pixel(c, x + w + i, yy, COLOR_SHADOW(), alpha);
        }
    }
    fill_rounded_rect(c, x, y, w, h, radius, fill);
    /* Inner highlight along the top edge */
    for (int i = 0; i < w - 2 * radius - 2; ++i) {
        blend_pixel(c, x + radius + 1 + i, y + 1,
                    rgb(clamp_u8(fill.r + 38),
                        clamp_u8(fill.g + 32),
                        clamp_u8(fill.b + 52)), 160);
    }
    stroke_rounded_rect(c, x, y, w, h, radius, edge);
}

/* -------------------------------------------------------------------------- */
/* Header: logo + title + mode badge                                          */
/* -------------------------------------------------------------------------- */

static void draw_firestaff_logo_block(M12_ModernCanvas* c, int x, int y);

static void draw_title_centered(M12_ModernCanvas* c) {
    /* Translucent band behind title */
    for (int y = 20; y < 128; ++y) {
        int alpha = 90 - (y - 20) * 48 / 108;
        for (int x = 0; x < c->w; ++x) {
            blend_pixel(c, x, y, rgb(0, 0, 0), alpha < 0 ? 0 : alpha);
        }
    }
    draw_firestaff_logo_block(c, 44, 28);

    int scale = 10;
    const char* label = "FIRESTAFF";
    ModernTextStyle probe = text_style_make(scale, COLOR_ACCENT_HI(), 0);
    int w = text_width_px(label, &probe);
    draw_text_gradient(c, (c->w - w) / 2, 30, label, scale,
                       COLOR_ACCENT_HI(), COLOR_ACCENT_LO(), COLOR_SHADOW());
    ModernTextStyle tag = text_style_make(3, COLOR_TEXT_DIM(), 2);
    draw_text_centered(c, c->w / 2, 110,
                       "THE DEFINITIVE DUNGEON MASTER FRONT DOOR", &tag);
}

static M12_RGB mode_color(int mode) {
    if (mode == M12_PRESENTATION_V2_ENHANCED_2D) return COLOR_V2();
    if (mode == M12_PRESENTATION_V3_MODERN_3D)   return COLOR_V3();
    return COLOR_V1();
}

static const char* mode_short(int mode) {
    if (mode == M12_PRESENTATION_V2_ENHANCED_2D) return "V2 ENHANCED 2D";
    if (mode == M12_PRESENTATION_V3_MODERN_3D)   return "V3 MODERN 3D";
    return "V1 ORIGINAL";
}


static const char* language_short(const M12_StartupMenuState* state) {
    int li = state ? state->settings.languageIndex : 0;
    if (li == 1) return "SV";
    if (li == 2) return "FR";
    if (li == 3) return "DE";
    return "EN";
}

static void draw_mode_badge(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    int mode = M12_StartupMenu_GetPresentationMode(state);
    M12_RGB col = mode_color(mode);
    int x = c->w - 300;
    int y = 40;
    int w = 260;
    int h = 48;
    /* Outer glow */
    for (int i = 1; i <= 6; ++i) {
        int alpha = 90 - i * 12;
        for (int xx = x - i; xx < x + w + i; ++xx) {
            blend_pixel(c, xx, y - i, col, alpha);
            blend_pixel(c, xx, y + h + i - 1, col, alpha);
        }
        for (int yy = y - i; yy < y + h + i; ++yy) {
            blend_pixel(c, x - i, yy, col, alpha);
            blend_pixel(c, x + w + i - 1, yy, col, alpha);
        }
    }
    fill_rounded_rect(c, x, y, w, h, 10, scale_rgb(col, 120, 255));
    stroke_rounded_rect(c, x, y, w, h, 10, col);
    ModernTextStyle lbl = text_style_make(2, COLOR_TEXT(), 1);
    draw_text(c, x + 16, y + 8, "MODE", &lbl);
    ModernTextStyle big = text_style_make(2, col, 2);
    const char* s = mode_short(mode);
    int tw = text_width_px(s, &big);
    draw_text(c, x + w - tw - 16, y + h - 18, s, &big);
}

/* -------------------------------------------------------------------------- */
/* Asset / version status helpers                                             */
/* -------------------------------------------------------------------------- */

static const M12_AssetVersionStatus* pick_status(const M12_StartupMenuState* state,
                                                 int gameSlot) {
    if (!state || gameSlot < 0 || gameSlot >= M12_ASSET_GAME_COUNT) {
        return NULL;
    }
    const M12_AssetVersionStatus* matched = NULL;
    for (int i = 0; i < M12_ASSET_MAX_VERSIONS_PER_GAME; ++i) {
        const M12_AssetVersionStatus* v = &state->assetStatus.versions[gameSlot][i];
        if (!v->versionId) continue;
        if (v->matched) {
            matched = v;
            break;
        }
        if (!matched) matched = v;
    }
    return matched;
}

static int slot_for_game_id(const char* id) {
    if (!id) return -1;
    if (strcmp(id, "dm1") == 0) return 0;
    if (strcmp(id, "csb") == 0) return 1;
    if (strcmp(id, "dm2") == 0) return 2;
    return -1;
}

static int game_supported(const char* id) {
    return id && strcmp(id, "dm1") == 0;
}

static M12_RGB muted_rgb(M12_RGB c) {
    int avg = ((int)c.r + (int)c.g + (int)c.b) / 3;
    return rgb(clamp_u8(avg * 7 / 10 + 34),
               clamp_u8(avg * 7 / 10 + 34),
               clamp_u8(avg * 7 / 10 + 40));
}

static void draw_readme_logo_image(M12_ModernCanvas* c, int x, int y, int w, int h) {
    if (!c || w <= 0 || h <= 0) return;
    int drawW = w;
    int drawH = drawW * M12_README_LOGO_HEIGHT / M12_README_LOGO_WIDTH;
    if (drawH > h) {
        drawH = h;
        drawW = drawH * M12_README_LOGO_WIDTH / M12_README_LOGO_HEIGHT;
    }
    int dstX = x + (w - drawW) / 2;
    int dstY = y + (h - drawH) / 2;
    for (int yy = 0; yy < drawH; ++yy) {
        int sy = yy * M12_README_LOGO_HEIGHT / drawH;
        int py = dstY + yy;
        if (py < 0 || py >= c->h) continue;
        for (int xx = 0; xx < drawW; ++xx) {
            int sx = xx * M12_README_LOGO_WIDTH / drawW;
            int px = dstX + xx;
            if (px < 0 || px >= c->w) continue;
            const unsigned char* p = g_m12ReadmeLogoRgb + ((sy * M12_README_LOGO_WIDTH + sx) * 3);
            put_pixel(c, px, py, rgb(p[0], p[1], p[2]));
        }
    }
}

static void draw_firestaff_logo_block(M12_ModernCanvas* c, int x, int y) {
    int logoW = 150;
    int logoH = 150;
    fill_rounded_rect(c, x - 12, y - 10, logoW + 24, logoH + 20, 14, rgb(14, 12, 24));
    stroke_rounded_rect(c, x - 12, y - 10, logoW + 24, logoH + 20, 14, COLOR_ACCENT());
    draw_readme_logo_image(c, x, y, logoW, logoH);
}

static void draw_generated_card_art(M12_ModernCanvas* c,
                                    const M12_GeneratedCardArt* art,
                                    int x, int y, int w, int h,
                                    int disabled) {
    if (!c || !art || !art->rgb || art->width <= 0 || art->height <= 0 || w <= 0 || h <= 0) {
        return;
    }

    int drawW = w;
    int drawH = drawW * art->height / art->width;
    if (drawH > h) {
        drawH = h;
        drawW = drawH * art->width / art->height;
    }
    int dstX = x + (w - drawW) / 2;
    int dstY = y + (h - drawH) / 2;

    for (int yy = 0; yy < drawH; ++yy) {
        int sy = yy * art->height / drawH;
        int py = dstY + yy;
        if (py < 0 || py >= c->h) continue;
        for (int xx = 0; xx < drawW; ++xx) {
            int sx = xx * art->width / drawW;
            int px = dstX + xx;
            if (px < 0 || px >= c->w) continue;
            const unsigned char* p = art->rgb + ((sy * art->width + sx) * 3);
            M12_RGB col = rgb(p[0], p[1], p[2]);
            if (disabled) col = muted_rgb(col);
            put_pixel(c, px, py, col);
        }
    }
}

static void draw_box_art_panel(M12_ModernCanvas* c,
                               const char* gameId,
                               int slotIdx,
                               int x, int y, int w, int h,
                               int disabled) {
    M12_RGB top = rgb(80, 42, 24);
    M12_RGB bot = rgb(18, 16, 24);
    M12_RGB accent = COLOR_ACCENT();
    M12_RGB ink = rgb(10, 9, 12);
    if (slotIdx == 1) {
        top = rgb(84, 28, 30);
        bot = rgb(20, 10, 16);
        accent = rgb(230, 88, 78);
    } else if (slotIdx == 2) {
        top = rgb(24, 58, 86);
        bot = rgb(8, 18, 32);
        accent = rgb(118, 190, 230);
    }
    if (disabled) {
        top = muted_rgb(top);
        bot = muted_rgb(bot);
        accent = muted_rgb(accent);
        ink = rgb(42, 42, 48);
    }

    fill_rounded_rect(c, x, y, w, h, 10, rgb(8, 8, 14));
    fill_vgradient(c, x + 4, y + 4, w - 8, h - 8, top, bot);
    stroke_rounded_rect(c, x, y, w, h, 10, disabled ? rgb(118, 118, 126) : accent);
    fill_rect(c, x + 12, y + 12, w - 24, 3, disabled ? rgb(116, 116, 116) : COLOR_ACCENT_HI());

    const M12_GeneratedCardArt* generated = M12_GeneratedCardArt_Find(gameId);
    if (generated) {
        draw_generated_card_art(c, generated, x + 8, y + 12, w - 16, h - 20, disabled);
    } else if (slotIdx == 0) {
        /* Dungeon arch + Firestaff silhouette. */
        fill_rect(c, x + w/2 - 34, y + 42, 68, h - 72, ink);
        fill_rect(c, x + w/2 - 24, y + 32, 48, 18, ink);
        for (int i = 0; i < 24; ++i) {
            fill_rect(c, x + w/2 - 24 + i, y + 32 - i/3, 48 - 2*i, 2, ink);
        }
        fill_rect(c, x + w/2 - 3, y + 30, 6, 82, accent);
        fill_rect(c, x + w/2 - 20, y + 70, 40, 7, accent);
        fill_rect(c, x + w/2 - 10, y + 22, 20, 12, COLOR_ACCENT_HI());
    } else if (slotIdx == 1) {
        /* Chaos eye. */
        fill_rect(c, x + 24, y + 46, w - 48, 56, rgb(210, 188, 150));
        fill_rect(c, x + 34, y + 56, w - 68, 36, accent);
        fill_rect(c, x + w/2 - 16, y + 60, 32, 28, ink);
        fill_rect(c, x + 20, y + 68, w - 40, 10, top);
    } else {
        /* Skullkeep towers. */
        fill_rect(c, x + 24, y + 50, w - 48, h - 78, rgb(88, 92, 104));
        fill_rect(c, x + 34, y + 34, 28, h - 62, rgb(104, 110, 124));
        fill_rect(c, x + w - 62, y + 34, 28, h - 62, rgb(104, 110, 124));
        fill_rect(c, x + w/2 - 16, y + h - 54, 32, 42, ink);
        fill_rect(c, x + w - 44, y + 22, 18, 18, accent);
    }

    if (!generated) {
        ModernTextStyle lbl = text_style_make(2, disabled ? rgb(176,176,180) : COLOR_TEXT(), 1);
        const char* text = slotIdx == 0 ? "BOX ART" : (slotIdx == 1 ? "CSB BOX ART" : "DM2 BOX ART");
        draw_text_centered(c, x + w / 2, y + h - 28, text, &lbl);
    }

    if (disabled) {
        for (int yy = y + 5; yy < y + h - 5; yy += 8) {
            for (int xx = x + 5; xx < x + w - 5; xx += 8) {
                blend_pixel(c, xx, yy, rgb(170, 170, 176), 120);
            }
        }
        fill_rounded_rect(c, x + 18, y + h / 2 - 18, w - 36, 36, 8, rgb(42, 42, 48));
        stroke_rounded_rect(c, x + 18, y + h / 2 - 18, w - 36, 36, 8, rgb(180, 180, 188));
        ModernTextStyle soon = text_style_make(2, rgb(220, 220, 224), 1);
        draw_text_centered(c, x + w / 2, y + h / 2 - 6, "COMING SOON", &soon);
    }
}

/* -------------------------------------------------------------------------- */
/* Card drawing                                                               */
/* -------------------------------------------------------------------------- */

/* Brightest VGA palette (DM/CSB PC 3.4). Duplicated here so the
 * modern renderer stays link-independent from the VGA palette table
 * used by the in-game renderer. Values match
 * G9010_auc_VgaPaletteBrightest_Compat at brightness level 0. */
static const unsigned char g_vgaBrightest[16][3] = {
    {0x00, 0x00, 0x00}, {0x4C, 0x4C, 0x4C}, {0x00, 0x80, 0x00}, {0x00, 0xCC, 0xCC},
    {0xCC, 0x33, 0x33}, {0x80, 0x00, 0x80}, {0xB0, 0x70, 0x30}, {0xCC, 0xCC, 0xCC},
    {0x70, 0x70, 0x70}, {0x33, 0x88, 0xFF}, {0x33, 0xFF, 0x66}, {0x66, 0xFF, 0xFF},
    {0xFF, 0x66, 0x66}, {0xFF, 0x66, 0xFF}, {0xFF, 0xFF, 0x66}, {0xFF, 0xFF, 0xFF}
};

/* Draw the selected creature thumbnail as a faded silhouette onto the
 * modern canvas so the background feels alive with art from the game
 * without stealing attention from the UI. Pixel value 0 is treated as
 * fully transparent; every other palette index is blended with the
 * caller-supplied tint at the supplied alpha. */
static void draw_creature_silhouette(M12_ModernCanvas* c,
                                     const M12_CreatureArtState* art,
                                     int dstX, int dstY,
                                     int dstW, int dstH,
                                     M12_RGB tint,
                                     int alphaMax) {
    if (!c || !art || !M12_CreatureArt_HasSelection(art)) return;
    if (dstW <= 0 || dstH <= 0) return;
    const M12_CreatureThumb* thumb = &art->creatures[art->selectedIndex];
    for (int y = 0; y < dstH; ++y) {
        int srcY = y * M12_CREATURE_THUMB_HEIGHT / dstH;
        int py = dstY + y;
        if (py < 0 || py >= c->h) continue;
        for (int x = 0; x < dstW; ++x) {
            int srcX = x * M12_CREATURE_THUMB_WIDTH / dstW;
            int px = dstX + x;
            if (px < 0 || px >= c->w) continue;
            unsigned char pi = thumb->pixels[srcY * M12_CREATURE_THUMB_WIDTH + srcX];
            if (pi == 0) continue;
            if (pi > 15) pi = (unsigned char)(pi & 0x0F);
            /* Combine original palette colour with the tint so it reads
             * as "atmospheric art" instead of a crisp sprite. */
            const unsigned char* base = g_vgaBrightest[pi];
            M12_RGB mixed = rgb(
                clamp_u8(((int)base[0] + tint.r) / 2),
                clamp_u8(((int)base[1] + tint.g) / 2),
                clamp_u8(((int)base[2] + tint.b) / 2));
            blend_pixel(c, px, py, mixed, alphaMax);
        }
    }
}

/* Pulse animation driven by state->frameTick. Returns a 0..255 modulation
 * factor that oscillates gently; used for selection glow breathing. */
static int pulse_modulation(unsigned int frameTick) {
    /* Triangle wave over a ~60-frame period. 0 at base, peaks at +80. */
    unsigned int phase = frameTick % 60U;
    int amp = (int)phase;
    if (amp > 30) amp = 60 - amp;
    return amp;
}

static void draw_firestaff_brand_card(M12_ModernCanvas* c,
                                     const M12_StartupMenuState* state,
                                     int x, int y, int w, int h) {
    (void)state;
    M12_RGB fill = COLOR_PANEL_FILL();
    fill_vgradient(c, x, y, w, h,
                   rgb(fill.r + 10, fill.g + 8, fill.b + 18),
                   rgb(18, 14, 24));
    stroke_rounded_rect(c, x, y, w, h, 12, COLOR_ACCENT());

    fill_vgradient(c, x + 2, y + 2, w - 4, 46,
                   rgb(86, 58, 28), rgb(42, 25, 13));
    ModernTextStyle title = text_style_make(3, COLOR_TEXT(), 2);
    draw_text(c, x + 16, y + 12, "FIRESTAFF", &title);

    /* Cover-first brand card: use the exact same logo artwork as README.md
     * (`assets/branding/firestaff-logo.png`) instead of the old tiny
     * in-renderer emblem. */
    int artX = x + 22;
    int artY = y + 76;
    int artW = w - 44;
    int artH = h - 142;
    fill_rounded_rect(c, artX, artY, artW, artH, 12, rgb(12, 10, 20));
    stroke_rounded_rect(c, artX, artY, artW, artH, 12, COLOR_ACCENT());
    draw_readme_logo_image(c, artX + 8, artY + 8, artW - 16, artH - 16);

    ModernTextStyle sub = text_style_make(2, COLOR_TEXT_DIM(), 1);
    draw_text(c, x + 32, y + h - 58, "ENTER THE FRONT DOOR", &sub);
}

static void draw_card(M12_ModernCanvas* c,
                      const M12_StartupMenuState* state,
                      int slot,
                      int x, int y, int w, int h,
                      int selected) {
    const M12_MenuEntry* entry = &state->entries[slot];
    int isSettings = entry->kind == M12_MENU_ENTRY_SETTINGS;
    int isMuseum = entry->kind == M12_MENU_ENTRY_MUSEUM;

    /* Selected cards glow (pulse-modulated for a living feel). */
    int pulseBoost = selected ? pulse_modulation(state->frameTick) : 0;
    if (selected) {
        for (int i = 1; i <= 10; ++i) {
            int alpha = 110 - i * 9 + pulseBoost;
            if (alpha > 255) alpha = 255;
            M12_RGB glow = isSettings ? rgb(160, 180, 240) : (isMuseum ? rgb(230, 190, 110) : COLOR_ACCENT());
            for (int xx = x - i; xx < x + w + i; ++xx) {
                blend_pixel(c, xx, y - i, glow, alpha);
                blend_pixel(c, xx, y + h + i - 1, glow, alpha);
            }
            for (int yy = y - i; yy < y + h + i; ++yy) {
                blend_pixel(c, x - i, yy, glow, alpha);
                blend_pixel(c, x + w + i - 1, yy, glow, alpha);
            }
        }
    }

    M12_RGB fill = COLOR_PANEL_FILL();
    M12_RGB edge = selected ? (isSettings ? rgb(160, 180, 240) : (isMuseum ? rgb(230, 190, 110) : COLOR_ACCENT()))
                            : COLOR_PANEL_EDGE();
    /* Inner gradient */
    fill_vgradient(c, x, y, w, h,
                   rgb(fill.r + 6, fill.g + 6, fill.b + 16),
                   rgb(fill.r / 2, fill.g / 2, fill.b / 2 + 10));
    stroke_rounded_rect(c, x, y, w, h, 12, edge);

    /* Header band */
    M12_RGB bandTop = isSettings ? rgb(40, 50, 110) : (isMuseum ? rgb(78, 58, 28) : rgb(70, 40, 24));
    M12_RGB bandBot = isSettings ? rgb(24, 32, 78)  : (isMuseum ? rgb(44, 32, 18) : rgb(42, 26, 16));
    fill_vgradient(c, x + 2, y + 2, w - 4, 46, bandTop, bandBot);

    ModernTextStyle title = text_style_make(3, COLOR_TEXT(), 2);
    title.tracking = 1;
    /* If title is too wide, fall back to scale 2. */
    if (text_width_px(entry->title, &title) > w - 32) {
        title = text_style_make(2, COLOR_TEXT(), 1);
    }
    if (entry->kind == M12_MENU_ENTRY_GAME && !game_supported(entry->gameId)) {
        title.color = rgb(174, 174, 182);
    }
    draw_text(c, x + 16, y + 12, entry->title, &title);

    if (isSettings) {
        ModernTextStyle p = text_style_make(2, COLOR_TEXT_DIM(), 1);
        draw_text(c, x + 16, y + 72,  "LANGUAGE", &p);
        draw_text(c, x + 16, y + 108, "PRESENTATION", &p);
        draw_text(c, x + 16, y + 144, "WINDOW",   &p);

        static const char* langs[] = {"EN", "SV", "FR", "DE"};
        static const char* grf[]   = {"V1", "V2", "V3 SOON"};
        static const char* win[]   = {"WINDOWED", "FULLSCREEN"};
        ModernTextStyle v = text_style_make(2, COLOR_ACCENT(), 1);
        int li = state->settings.languageIndex;
        int gi = state->settings.graphicsIndex;
        int wi = state->settings.windowModeIndex;
        if (li < 0) li = 0; if (li > 3) li = 3;
        if (gi < 0) gi = 0; if (gi > 2) gi = 2;
        if (wi < 0) wi = 0; if (wi > 1) wi = 1;
        draw_text(c, x + 200, y + 72,  langs[li], &v);
        draw_text(c, x + 200, y + 108, grf[gi], &v);
        draw_text(c, x + 200, y + 144, win[wi], &v);

        ModernTextStyle hint = text_style_make(1, COLOR_TEXT_FAINT(), 0);
        draw_text(c, x + 16, y + h - 32,
                  "PRESS ENTER TO CONFIGURE SETTINGS", &hint);
        return;
    }

    if (isMuseum) {
        ModernTextStyle p = text_style_make(2, COLOR_TEXT_DIM(), 1);
        ModernTextStyle v = text_style_make(2, COLOR_ACCENT(), 1);
        draw_text(c, x + 16, y + 72,  "DUNGEON MASTER", &p);
        draw_text(c, x + 16, y + 108, "CHAOS STRIKES BACK", &p);
        draw_text(c, x + 16, y + 144, "DUNGEON MASTER II", &p);
        draw_text(c, x + 16, y + 180, "CREDITS AND ARCHIVE", &p);
        draw_text(c, x + 16, y + 222, "5 SECTIONS", &v);
        ModernTextStyle hint = text_style_make(1, COLOR_TEXT_FAINT(), 0);
        draw_text(c, x + 16, y + h - 32,
                  "PRESS ENTER TO OPEN THE LORE MUSEUM", &hint);
        return;
    }

    /* Game card: status line under title, then version list. */
    int slotIdx = slot_for_game_id(entry->gameId);
    const M12_AssetVersionStatus* status = pick_status(state, slotIdx);

    M12_RGB statusColor;
    const char* statusLabel;
    if (!game_supported(entry->gameId)) {
        statusColor = rgb(168, 168, 176);
        statusLabel = "UNSUPPORTED";
    } else if (entry->available && status && status->matched) {
        statusColor = COLOR_OK();
        statusLabel = "VERIFIED";
    } else if (status) {
        statusColor = COLOR_BAD();
        statusLabel = "DATA MISSING";
    } else {
        statusColor = COLOR_WARN();
        statusLabel = "NOT SCANNED";
    }

    /* Status pill */
    {
        int pillX = x + 16;
        int pillY = y + 56;
        int pillW = 180;
        int pillH = 22;
        fill_rounded_rect(c, pillX, pillY, pillW, pillH, 6, scale_rgb(statusColor, 90, 255));
        stroke_rounded_rect(c, pillX, pillY, pillW, pillH, 6, statusColor);
        ModernTextStyle s = text_style_make(1, statusColor, 0);
        int tw = text_width_px(statusLabel, &s);
        draw_text(c, pillX + (pillW - tw) / 2, pillY + 8, statusLabel, &s);
    }

    /* Cover-first main cards: show the art large here. Version,
     * resolution, renderer, checksum and launch controls live in the
     * per-game detail menu after selection. */
    {
        int artX = x + 22;
        int artY = y + 84;
        int artW = w - 44;
        int artH = h - 150;
        if (artH < 180) artH = 180;
        draw_box_art_panel(c, entry->gameId, slotIdx, artX, artY, artW, artH, !game_supported(entry->gameId));
    }

    {
        ModernTextStyle hint = text_style_make(2, COLOR_TEXT_DIM(), 1);
        draw_text(c, x + 24, y + h - 48, "ENTER FOR GAME MENU", &hint);
    }
}

/* -------------------------------------------------------------------------- */
/* View drawing                                                               */
/* -------------------------------------------------------------------------- */

/* Top-left back button, visible on every non-main view. Coords must
 * match M12_HIT_BACK_* in menu_hit_m12.c. */
static void draw_back_button(M12_ModernCanvas* c, int highlight) {
    int x = 24;
    int y = 120;
    int w = 110;
    int h = 44;
    M12_RGB fill = highlight ? rgb(44, 38, 74) : rgb(22, 22, 46);
    M12_RGB edge = highlight ? COLOR_ACCENT_HI() : COLOR_PANEL_EDGE();
    fill_rounded_rect(c, x, y, w, h, 10, fill);
    stroke_rounded_rect(c, x, y, w, h, 10, edge);
    ModernTextStyle t = text_style_make(2, COLOR_TEXT(), 1);
    draw_text(c, x + 14, y + 14, "< BACK", &t);
}

/* Launch button, lower-right of the game-options panel. Coords must
 * match M12_HIT_LAUNCH_* in menu_hit_m12.c. */
static void draw_launch_button(M12_ModernCanvas* c,
                               int highlight,
                               unsigned int frameTick) {
    int panelX = 96;
    int panelY = 260;
    int panelW = c->w - 2 * panelX;
    int panelH = 400;
    int w = 240;
    int h = 54;
    int x = panelX + panelW - w - 36;
    int y = panelY + panelH - h - 20;
    int boost = highlight ? pulse_modulation(frameTick) : 0;
    M12_RGB fill = highlight
        ? rgb(clamp_u8(90 + boost), clamp_u8(60 + boost / 2), 24)
        : rgb(60, 42, 20);
    M12_RGB edge = COLOR_ACCENT_HI();
    for (int i = 1; i <= 6; ++i) {
        int alpha = 80 - i * 10 + boost;
        if (alpha <= 0) continue;
        for (int xx = x - i; xx < x + w + i; ++xx) {
            blend_pixel(c, xx, y - i, COLOR_ACCENT(), alpha);
            blend_pixel(c, xx, y + h + i - 1, COLOR_ACCENT(), alpha);
        }
        for (int yy = y - i; yy < y + h + i; ++yy) {
            blend_pixel(c, x - i, yy, COLOR_ACCENT(), alpha);
            blend_pixel(c, x + w + i - 1, yy, COLOR_ACCENT(), alpha);
        }
    }
    fill_rounded_rect(c, x, y, w, h, 12, fill);
    stroke_rounded_rect(c, x, y, w, h, 12, edge);
    ModernTextStyle t = text_style_make(3, COLOR_ACCENT_HI(), 2);
    int tw = text_width_px("LAUNCH >", &t);
    draw_text(c, x + (w - tw) / 2, y + 12, "LAUNCH >", &t);
}

static void draw_footer(M12_ModernCanvas* c, const char* left, const char* right) {
    int h = 48;
    int y = c->h - h - 12;
    fill_rounded_rect(c, 24, y, c->w - 48, h, 10, rgb(14, 14, 28));
    stroke_rounded_rect(c, 24, y, c->w - 48, h, 10, COLOR_PANEL_EDGE());
    ModernTextStyle t = text_style_make(2, COLOR_TEXT_DIM(), 1);
    if (left)  draw_text(c, 48,            y + 14, left, &t);
    if (right) {
        int rw = text_width_px(right, &t);
        draw_text(c, c->w - 48 - rw, y + 14, right, &t);
    }
}

static void draw_data_dir(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    const char* dir = state->assetStatus.dataDir;
    if (!dir || dir[0] == '\0') dir = "UNSET";
    /* Elide extremely long paths to keep the footer label legible at
     * scale 2. Budget: ~72 glyphs for the full line, minus 10 for the
     * "DATA DIR  " prefix, gives ~62 usable characters. */
    char dirBuf[96];
    size_t dirLen = strlen(dir);
    if (dirLen > 62) {
        size_t head = 28;
        size_t tail = 62 - head - 3;
        memcpy(dirBuf, dir, head);
        dirBuf[head + 0] = '.';
        dirBuf[head + 1] = '.';
        dirBuf[head + 2] = '.';
        memcpy(dirBuf + head + 3, dir + dirLen - tail, tail);
        dirBuf[head + 3 + tail] = '\0';
    } else {
        snprintf(dirBuf, sizeof(dirBuf), "%s", dir);
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "DATA DIR  %s", dirBuf);
    /* Uppercase for the retro-meets-modern look. */
    for (int i = 0; buf[i]; ++i) {
        if (buf[i] >= 'a' && buf[i] <= 'z') buf[i] = (char)(buf[i] - 'a' + 'A');
    }
    ModernTextStyle t = text_style_make(2, COLOR_TEXT_DIM(), 1);
    draw_text(c, 48, c->h - 86, buf, &t);
}

static void draw_main_view(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    /* Four tall front-door cards per product direction:
     * Firestaff brand, DM1, CSB, DM2. Settings remain available
     * through the settings view/input path, but are no longer a main
     * destination card in the modern front-door layout. */
    int gridTop = 170;
    int gridBottom = c->h - 130;
    int gridH = gridBottom - gridTop;
    int cardCount = 4;
    int gap = 22;
    int sideMargin = 48;
    int cardW = (c->w - 2 * sideMargin - gap * (cardCount - 1)) / cardCount;
    int cardH = gridH;

    for (int i = 0; i < cardCount; ++i) {
        int x = sideMargin + i * (cardW + gap);
        int y = gridTop;
        if (i == 0) {
            draw_firestaff_brand_card(c, state, x, y, cardW, cardH);
        } else {
            int entryIndex = i - 1;
            int selected = (state->selectedIndex == entryIndex);
            draw_card(c, state, entryIndex, x, y, cardW, cardH, selected);
        }
    }

    /* Section title */
    ModernTextStyle h = text_style_make(3, COLOR_ACCENT(), 2);
    draw_text(c, 48, 142, "SELECT A DESTINATION", &h);

    /* Faded creature silhouette in the upper-right background so the
     * screen feels like a Dungeon Master front door without stealing
     * focus from the card grid. */
    if (M12_CreatureArt_HasSelection(&state->creatureArt)) {
        int cx = c->w - 460;
        int cy = 10;
        int cw = 420;
        int ch = 136;
        draw_creature_silhouette(c, &state->creatureArt,
                                 cx, cy, cw, ch,
                                 rgb(48, 32, 72), 70);
    }
}

static void draw_setting_row(M12_ModernCanvas* c, int x, int y, int w,
                             const char* label, const char* value,
                             int selected) {
    M12_RGB fill = selected ? rgb(36, 42, 84) : rgb(20, 22, 48);
    M12_RGB edge = selected ? COLOR_ACCENT() : COLOR_PANEL_EDGE();
    fill_rounded_rect(c, x, y, w, 50, 10, fill);
    stroke_rounded_rect(c, x, y, w, 50, 10, edge);
    ModernTextStyle L = text_style_make(2, COLOR_TEXT_DIM(), 1);
    ModernTextStyle V = text_style_make(2, selected ? COLOR_ACCENT_HI() : COLOR_TEXT(), 1);
    draw_text(c, x + 20, y + 14, label, &L);
    int vw = text_width_px(value, &V);
    draw_text(c, x + w - 20 - vw, y + 14, value, &V);
}

static void draw_settings_view(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    draw_back_button(c, 0);
    ModernTextStyle h = text_style_make(4, COLOR_ACCENT(), 3);
    draw_text(c, 160, 130, "SETTINGS", &h);
    ModernTextStyle sub = text_style_make(2, COLOR_TEXT_DIM(), 1);
    draw_text(c, 160, 210, "CHANGES ARE PERSISTED IMMEDIATELY", &sub);

    int panelX = 96;
    int panelY = 260;
    int panelW = c->w - 2 * panelX;
    int panelH = 400;
    draw_panel(c, panelX, panelY, panelW, panelH,
               rgb(14, 16, 36), COLOR_PANEL_EDGE(), 18);

    static const char* langs[] = {"ENGLISH", "SVENSKA", "FRANCAIS", "DEUTSCH"};
    static const char* grf[]   = {"V1 ORIGINAL", "V2 ENHANCED 2D", "V3 MODERN/3D"};
    static const char* win[]   = {"WINDOWED", "FULLSCREEN"};
    int li = state->settings.languageIndex;
    int gi = state->settings.graphicsIndex;
    int wi = state->settings.windowModeIndex;
    if (li < 0) li = 0; if (li > 3) li = 3;
    if (gi < 0) gi = 0; if (gi > 2) gi = 2;
    if (wi < 0) wi = 0; if (wi > 1) wi = 1;

    int rowX = panelX + 36;
    int rowW = panelW - 72;
    int rowY = panelY + 36;
    draw_setting_row(c, rowX, rowY,      rowW, "LANGUAGE",       langs[li],
                     state->settingsSelectedIndex == 0);
    draw_setting_row(c, rowX, rowY + 70, rowW, "PRESENTATION MODE", grf[gi],
                     state->settingsSelectedIndex == 1);
    draw_setting_row(c, rowX, rowY + 140, rowW, "WINDOW MODE",   win[wi],
                     state->settingsSelectedIndex == 2);
}

typedef struct {
    const char* title;
    const char* subtitle;
    const char* pages[3][5];
    int pageCount;
} ModernMuseumCategory;

static const ModernMuseumCategory g_modernMuseumCategories[] = {
    {"DUNGEON MASTER", "THE ORIGINAL DUNGEON CRAWL",
     {{"1987 FTL GAMES", "CHAMPIONS ENTER THE DUNGEON", "FOUR PORTRAITS BECOME A PARTY", "THE FIRESTAFF IS THE CENTRAL RELIC", "REAL TIME PRESSURE DEFINES THE LEGEND"},
      {"KEY LORE THREADS", "LORD CHAOS SHATTERS ORDER", "THE GREY LORD IS DIVIDED", "RA RETURNS AS MASTER OF BALANCE", "THE DUNGEON IS BOTH TEST AND PRISON"},
      {"PRESERVATION NOTES", "PC AND ATARI ST LINEAGE MATTERS", "GRAPHICS DAT AND DUNGEON DAT ARE VERIFIED", "HASHED ORIGINAL DATA STAYS USER SUPPLIED", "FIRESTAFF RECORDS EVIDENCE NOT GUESSWORK"}}, 3},
    {"CHAOS STRIKES BACK", "THE CHAMPIONS RETURN",
     {{"EXPANSION AND SEQUEL DESIGN", "DUNGEON MASTER SYSTEMS BECOME DENSER", "THE CORBUM QUEST REPLACES SIMPLE DESCENT", "FOUR PATHS TEST MASTERY", "CSB REWARDS MAP MEMORY AND NERVE"},
      {"LORE SHAPE", "CHAOS STILL CASTS A LONG SHADOW", "THE PLAYER HUNTS CORBUM MATERIAL", "RETURNING CHAMPIONS FACE A HARDER MAZE", "THE WORLD FEELS OLDER AND LESS SAFE"},
      {"ARCHIVE STATUS", "CSBGRAPH DAT AND CSB DAT ARE TRACKED", "VERSION SLOTS USE HASH EVIDENCE", "LAUNCHER SHOWS READY ONLY WHEN MATCHED", "MUSEUM CONTENT STAYS STATIC AND BOUNDED"}}, 3},
    {"DUNGEON MASTER II", "THE LEGEND OUTSIDE THE FIRST DUNGEON",
     {{"THE SKULLKEEP ERA", "THE SERIES MOVES BEYOND THE ORIGINAL MAZE", "OUTDOOR AND SHOP SPACES EXPAND THE FORM", "MINIONS AND WEATHER CHANGE THE RHYTHM", "DM2 KEEPS THE PARTY SURVIVAL CORE"},
      {"LORE SHAPE", "TECHNOLOGY AND MAGIC SHARE THE STAGE", "THE WORLD IS BROADER THAN MOUNT ANAIAS", "THE PLAYER ASSEMBLES AND SURVIVES", "THE TONE IS STRANGER AND MORE MECHANICAL"},
      {"ARCHIVE STATUS", "DM2GRAPHICS DAT AND DM2DUNGEON DAT ARE TRACKED", "SUPPORTED VERSIONS CAN GROW OVER TIME", "CONTENT HERE IS A GUIDE NOT A DATA DUMP", "BINARY ASSETS REMAIN OUTSIDE THIS PASS"}}, 3},
    {"FIRESTAFF PROJECT", "ABOUT AND CREDITS",
     {{"PROJECT PURPOSE", "OPEN DUNGEON MASTER ENGINE", "DETERMINISTIC MODULAR MUSEUM GRADE", "ORIGINAL DATA IS VERIFIED NOT BUNDLED", "V1 PRESERVES BASELINE BEHAVIOUR"},
      {"CREDITS", "FTL GAMES AND SOFTWARE HEAVEN CREATED THE ORIGINALS", "DOUG BELL AND ANDY JAROS LED THE CLASSIC DESIGN", "CHRISTOPHE FONTANEL DOCUMENTED VITAL HISTORY", "FIRESTAFF BUILDS ON PRESERVATION RESEARCH"},
      {"PROJECT BOUNDARIES", "NO CLAIM OF OFFICIAL AFFILIATION", "USER SUPPLIED RETAIL DATA IS REQUIRED", "REGRESSION PROBES GUARD MENU STABILITY", "TRACKED TEXT STAYS ENGLISH IN THE REPO"}}, 3},
    {"TECHNICAL ARCHIVE", "SOURCE EVIDENCE AND VERIFICATION",
     {{"EVIDENCE MODEL", "KNOWN FILES ARE MATCHED BY HASH", "VERSION MATRICES STAY EXPLICIT", "RUNTIME PATHS REPORT MISSING DATA SAFELY", "NO SILENT FALLBACK TO UNKNOWN ORIGINALS"},
      {"STARTUP MENU", "KEYBOARD INPUT IS BOUNDED", "MOUSE HITS ROUTE THROUGH SHARED STATE", "UNKNOWN KEYS ARE NO OPS", "ESCAPE RETURNS BEFORE EXITING"},
      {"FUTURE MUSEUM WORK", "ADD MANUAL EXCERPT REFERENCES", "ADD INTERVIEW AND TIMELINE SOURCES", "ADD SMALL CURATED SCREEN PANELS", "KEEP LARGE ASSETS OUT UNTIL LICENSED"}}, 3}
};

static void draw_museum_view(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    enum { CAT_COUNT = (int)(sizeof(g_modernMuseumCategories) / sizeof(g_modernMuseumCategories[0])) };
    int cat = state->museumSelectedIndex;
    if (cat < 0) cat = 0;
    if (cat >= CAT_COUNT) cat = CAT_COUNT - 1;
    const ModernMuseumCategory* section = &g_modernMuseumCategories[cat];
    int page = state->museumPageIndex;
    if (page < 0) page = 0;
    if (page >= section->pageCount) page = section->pageCount - 1;

    draw_back_button(c, 0);
    ModernTextStyle h = text_style_make(4, COLOR_ACCENT(), 3);
    draw_text(c, 160, 130, "MUSEUM OF LORE", &h);
    ModernTextStyle sub = text_style_make(2, COLOR_TEXT_DIM(), 1);
    draw_text(c, 160, 210, "PRESERVATION NOTES, SERIES LORE, AND PROJECT CREDITS", &sub);

    int panelX = 96;
    int panelY = 260;
    int panelW = c->w - 2 * panelX;
    int panelH = 400;
    draw_panel(c, panelX, panelY, panelW, panelH,
               rgb(14, 16, 36), COLOR_PANEL_EDGE(), 18);

    int leftW = 330;
    ModernTextStyle label = text_style_make(1, COLOR_TEXT_FAINT(), 0);
    draw_text(c, panelX + 30, panelY + 28, "ARCHIVE SECTIONS", &label);
    for (int i = 0; i < CAT_COUNT; ++i) {
        int y = panelY + 54 + i * 56;
        int selected = (i == cat);
        fill_rounded_rect(c, panelX + 24, y, leftW, 42, 10,
                          selected ? rgb(48, 40, 76) : rgb(22, 22, 46));
        stroke_rounded_rect(c, panelX + 24, y, leftW, 42, 10,
                            selected ? COLOR_ACCENT() : COLOR_PANEL_EDGE());
        ModernTextStyle row = text_style_make(2, selected ? COLOR_ACCENT_HI() : COLOR_TEXT_DIM(), 1);
        draw_text(c, panelX + 42, y + 13, g_modernMuseumCategories[i].title, &row);
    }

    int contentX = panelX + leftW + 70;
    int contentW = panelW - leftW - 100;
    ModernTextStyle title = text_style_make(3, COLOR_TEXT(), 2);
    draw_text(c, contentX, panelY + 34, section->title, &title);
    ModernTextStyle subtitle = text_style_make(2, COLOR_ACCENT(), 1);
    draw_text(c, contentX, panelY + 78, section->subtitle, &subtitle);

    char pageLine[32];
    snprintf(pageLine, sizeof(pageLine), "PAGE %d/%d", page + 1, section->pageCount);
    ModernTextStyle pageStyle = text_style_make(1, COLOR_TEXT_FAINT(), 0);
    draw_text(c, contentX + contentW - 120, panelY + 40, pageLine, &pageStyle);

    for (int i = 0; i < 5; ++i) {
        ModernTextStyle line = text_style_make(i == 0 ? 2 : 2,
                                               i == 0 ? COLOR_TEXT() : COLOR_TEXT_DIM(),
                                               1);
        draw_text(c, contentX, panelY + 132 + i * 40, section->pages[page][i], &line);
    }
}

static void draw_game_options_view(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    int slot = slot_for_game_id(state->entries[state->selectedIndex].gameId);
    if (slot < 0) slot = 0;
    const M12_GameOptions* opts = &state->gameOptions[slot];
    const M12_MenuEntry* entry = &state->entries[state->selectedIndex];

    draw_back_button(c, 0);
    ModernTextStyle h = text_style_make(4, COLOR_ACCENT(), 3);
    draw_text(c, 160, 130, "GAME OPTIONS", &h);
    ModernTextStyle sub = text_style_make(2, COLOR_TEXT_DIM(), 1);
    draw_text(c, 160, 210, entry->title, &sub);

    int panelX = 96;
    int panelY = 260;
    int panelW = c->w - 2 * panelX;
    int panelH = 400;
    draw_panel(c, panelX, panelY, panelW, panelH,
               rgb(14, 16, 36), COLOR_PANEL_EDGE(), 18);

    static const char* langs[] = {"ENGLISH", "SVENSKA", "FRANCAIS", "DEUTSCH"};
    static const char* aspects[] = {"ORIGINAL", "4:3", "16:9", "16:10"};
    static const char* res[] = {"320X200", "640X400", "800X600", "1024X768", "1280X960"};
    static const char* speeds[] = {"SLOWER", "NORMAL", "FASTER"};

    const M12_AssetVersionStatus* ver = NULL;
    int vc = (int)M12_AssetStatus_GetVersionCount(entry->gameId);
    if (vc > 0) {
        int vi = opts->versionIndex;
        if (vi < 0) vi = 0;
        if (vi >= vc) vi = vc - 1;
        ver = &state->assetStatus.versions[slot][vi];
    }

    const char* verLabel = ver ? (ver->shortLabel ? ver->shortLabel
                                                   : (ver->label ? ver->label : "-"))
                                : "-";
    const char* patchLabel = opts->usePatch ? "PATCHED" : "ORIGINAL";
    const char* langLabel  = (opts->languageIndex >= 0 && opts->languageIndex < 4)
                              ? langs[opts->languageIndex] : "EN";
    const char* cheatsLabel = opts->cheatsEnabled ? "ON" : "OFF";
    int speedIdx = opts->gameSpeed;
    if (speedIdx < 0) speedIdx = 0;
    if (speedIdx > 2) speedIdx = 2;
    int aspIdx = opts->aspectRatio;
    if (aspIdx < 0) aspIdx = 0;
    if (aspIdx > 3) aspIdx = 3;
    int resIdx = opts->resolution;
    if (resIdx < 0) resIdx = 0;
    if (resIdx > 4) resIdx = 4;

    int rowX = panelX + 36;
    int rowW = panelW - 72;
    int rowY = panelY + 28;
    int step = 52;
    int sel = state->gameOptSelectedRow;

    draw_setting_row(c, rowX, rowY + 0 * step, rowW, "VERSION",     verLabel,          sel == 0);
    draw_setting_row(c, rowX, rowY + 1 * step, rowW, "PATCH",       patchLabel,        sel == 1);
    draw_setting_row(c, rowX, rowY + 2 * step, rowW, "LANGUAGE",    langLabel,         sel == 2);
    draw_setting_row(c, rowX, rowY + 3 * step, rowW, "CHEATS",      cheatsLabel,       sel == 3);
    draw_setting_row(c, rowX, rowY + 4 * step, rowW, "SPEED",       speeds[speedIdx],  sel == 4);
    draw_setting_row(c, rowX, rowY + 5 * step, rowW, "ASPECT",      aspects[aspIdx],   sel == 5);
    draw_setting_row(c, rowX, rowY + 6 * step, rowW, "RESOLUTION",  res[resIdx],       sel == 6);

    /* Mode constraint notice if V1 */
    int mode = M12_StartupMenu_GetPresentationMode(state);
    if (mode == M12_PRESENTATION_V1_ORIGINAL) {
        ModernTextStyle note = text_style_make(1, COLOR_WARN(), 0);
        draw_text(c, panelX + 36, panelY + panelH - 40,
                  "V1 ORIGINAL LOCKS ASPECT AND RESOLUTION FOR AUTHENTICITY",
                  &note);
    } else if (mode == M12_PRESENTATION_V3_MODERN_3D) {
        ModernTextStyle note = text_style_make(1, COLOR_V3(), 0);
        draw_text(c, panelX + 36, panelY + panelH - 40,
                  "V3 MODERN 3D IS A LATER MILESTONE (COMING SOON)",
                  &note);
    }

    /* Launch button: pulses when the cursor is on the synthetic launch
     * row (gameOptSelectedRow == M12_GAME_OPT_ROW_COUNT). */
    draw_launch_button(c,
                       state->gameOptSelectedRow >= M12_GAME_OPT_ROW_COUNT,
                       state->frameTick);
}

static void draw_message_view(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    draw_back_button(c, 0);
    int panelW = 840;
    int panelH = 320;
    int panelX = (c->w - panelW) / 2;
    int panelY = (c->h - panelH) / 2;
    draw_panel(c, panelX, panelY, panelW, panelH,
               rgb(16, 14, 30), COLOR_ACCENT(), 20);
    ModernTextStyle big = text_style_make(3, COLOR_ACCENT(), 2);
    const char* line1 = state->messageLine1 ? state->messageLine1 : "";
    const char* line2 = state->messageLine2 ? state->messageLine2 : "";
    const char* line3 = state->messageLine3 ? state->messageLine3 : "";
    int w1 = text_width_px(line1, &big);
    draw_text(c, panelX + (panelW - w1) / 2, panelY + 48, line1, &big);
    ModernTextStyle mid = text_style_make(2, COLOR_TEXT(), 1);
    int w2 = text_width_px(line2, &mid);
    draw_text(c, panelX + (panelW - w2) / 2, panelY + 140, line2, &mid);
    ModernTextStyle sm = text_style_make(2, COLOR_TEXT_DIM(), 1);
    int w3 = text_width_px(line3, &sm);
    draw_text(c, panelX + (panelW - w3) / 2, panelY + 220, line3, &sm);
}

/* -------------------------------------------------------------------------- */
/* Original-faithful sparse V1 path                                           */
/* -------------------------------------------------------------------------- */

static int modern_is_original_sparse_path(const M12_StartupMenuState* state) {
    (void)state;
    /* V1 original constrains the game presentation, not Firestaff's
     * front-door launcher.  The sparse renderer made default V1 startup
     * look like the old placeholder menu and hid the branded logo/card art
     * Daniel explicitly added.  Keep the sparse code below as a legacy
     * fallback implementation, but never select it from the modern renderer;
     * main_loop_m11.c already provides FIRESTAFF_LEGACY_MENU for the
     * deliberate old-menu escape hatch. */
    return 0;
}

static void draw_sparse_background(M12_ModernCanvas* c) {
    fill_rect(c, 0, 0, c->w, c->h, rgb(0, 0, 0));
    for (int y = c->h / 2 - 20; y <= c->h / 2 + 20; ++y) {
        int alpha = 16 - abs(y - c->h / 2) / 2;
        if (alpha <= 0) continue;
        for (int x = c->w / 2 - 140; x <= c->w / 2 + 140; ++x) {
            blend_pixel(c, x, y, rgb(64, 64, 64), alpha);
        }
    }
}

static void draw_sparse_center_box_modern(M12_ModernCanvas* c,
                                          int boxW,
                                          int boxH,
                                          const char* line1,
                                          const char* line2,
                                          const char* line3,
                                          M12_RGB line1Color) {
    int x = (c->w - boxW) / 2;
    int y = (c->h - boxH) / 2;
    stroke_rounded_rect(c, x, y, boxW, boxH, 4, rgb(70, 70, 70));
    ModernTextStyle l1 = text_style_make(2, line1Color, 0);
    ModernTextStyle l2 = text_style_make(2, rgb(210, 210, 210), 0);
    ModernTextStyle l3 = text_style_make(2, rgb(120, 120, 120), 0);
    if (line1 && line1[0]) draw_text_centered(c, c->w / 2, y + 18, line1, &l1);
    if (line2 && line2[0]) draw_text_centered(c, c->w / 2, y + 42, line2, &l2);
    if (line3 && line3[0]) draw_text_centered(c, c->w / 2, y + 66, line3, &l3);
}

static void draw_sparse_main_view_modern(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    int phase = (int)(state->frameTick / 16U);
    ModernTextStyle title = text_style_make(3, rgb(214, 214, 214), 0);
    ModernTextStyle sub = text_style_make(2, rgb(120, 120, 120), 0);
    ModernTextStyle rowSel = text_style_make(2, rgb(240, 240, 240), 0);
    ModernTextStyle row = text_style_make(2, rgb(126, 126, 126), 0);
    int centerX = c->w / 2;
    int baseY = c->h / 2 - 36;

    draw_text_centered(c, centerX, baseY, "DUNGEON MASTER", &title);
    if (phase >= 1) {
        draw_text_centered(c, centerX, baseY + 26, "CHAOS STRIKES BACK", &sub);
    }
    if (phase >= 2) {
        int rows = phase >= 3 ? M12_StartupMenu_GetEntryCount() : 2;
        for (int i = 0; i < rows; ++i) {
            int y = baseY + 62 + i * 22;
            ModernTextStyle* style = (i == state->selectedIndex) ? &rowSel : &row;
            if (i == state->selectedIndex) {
                draw_text(c, centerX - 160, y, ">", style);
            }
            draw_text(c, centerX - 132, y, state->entries[i].title, style);
        }
    }
}

static void draw_sparse_view_modern(M12_ModernCanvas* c, const M12_StartupMenuState* state) {
    switch (state->view) {
        case M12_MENU_VIEW_SETTINGS: {
            char line2[96];
            char line3[96];
            snprintf(line2, sizeof(line2), "LANGUAGE  %s", state->settings.languageIndex == 1 ? "SV" : state->settings.languageIndex == 2 ? "FR" : state->settings.languageIndex == 3 ? "DE" : "EN");
            snprintf(line3, sizeof(line3), "GRAPHICS  V1 ORIGINAL");
            draw_sparse_center_box_modern(c, 420, 110, "SETTINGS", line2, line3, rgb(240, 240, 240));
            break;
        }
        case M12_MENU_VIEW_GAME_OPTIONS: {
            const M12_MenuEntry* entry = &state->entries[state->selectedIndex];
            draw_sparse_center_box_modern(c,
                                          520,
                                          110,
                                          entry->title,
                                          state->gameOptions[state->selectedIndex < 3 ? state->selectedIndex : 0].usePatch ? "PATCH  PATCHED" : "PATCH  ORIGINAL",
                                          "ENTER LAUNCH   ESC BACK",
                                          rgb(240, 240, 240));
            break;
        }
        case M12_MENU_VIEW_MESSAGE: {
            M12_RGB line1Color = rgb(120, 210, 120);
            const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, state->activatedIndex);
            if (entry && !entry->available) line1Color = rgb(210, 120, 120);
            draw_sparse_center_box_modern(c, 540, 120, state->messageLine1, state->messageLine2, state->messageLine3, line1Color);
            break;
        }
        case M12_MENU_VIEW_MUSEUM: {
            int cat = state->museumSelectedIndex;
            if (cat < 0) cat = 0;
            if (cat >= 5) cat = 4;
            draw_sparse_center_box_modern(c, 560, 120,
                                          g_modernMuseumCategories[cat].title,
                                          g_modernMuseumCategories[cat].pages[0][0],
                                          "ARROWS NAVIGATE   ESC BACK",
                                          rgb(240, 240, 240));
            break;
        }
        case M12_MENU_VIEW_MAIN:
        default:
            draw_sparse_main_view_modern(c, state);
            break;
    }
}

/* -------------------------------------------------------------------------- */
/* Entrypoint                                                                 */
/* -------------------------------------------------------------------------- */

int M12_ModernMenu_NativeWidth(void)  { return M12_MODERN_MENU_NATIVE_WIDTH; }
int M12_ModernMenu_NativeHeight(void) { return M12_MODERN_MENU_NATIVE_HEIGHT; }

void M12_ModernMenu_Render(const M12_StartupMenuState* state,
                           unsigned char* rgba,
                           int width,
                           int height) {
    if (!state || !rgba || width < 16 || height < 16) {
        return;
    }
    M12_ModernCanvas c = {rgba, width, height};
    if (modern_is_original_sparse_path(state)) {
        draw_sparse_background(&c);
        draw_sparse_view_modern(&c, state);
        return;
    }
    draw_background(&c);
    draw_title_centered(&c);
    draw_mode_badge(&c, state);

    const char* footerLeft  = "UP DOWN MOVE    ENTER SELECT    ESC BACK";

    switch (state->view) {
        case M12_MENU_VIEW_SETTINGS:
            draw_settings_view(&c, state);
            footerLeft = "UP DOWN MOVE    LEFT RIGHT CYCLE    ESC BACK";
            break;
        case M12_MENU_VIEW_GAME_OPTIONS:
            draw_game_options_view(&c, state);
            footerLeft = "UP DOWN MOVE    LEFT RIGHT CYCLE    ENTER LAUNCH    ESC BACK";
            break;
        case M12_MENU_VIEW_MUSEUM:
            draw_museum_view(&c, state);
            footerLeft = "UP DOWN SECTIONS    LEFT RIGHT PAGE    ESC BACK";
            break;
        case M12_MENU_VIEW_MESSAGE:
            draw_message_view(&c, state);
            footerLeft = "ENTER OR ESC RETURNS TO MENU";
            break;
        case M12_MENU_VIEW_MAIN:
        default:
            draw_main_view(&c, state);
            break;
    }

    draw_data_dir(&c, state);

    const char* modeStr = mode_short(M12_StartupMenu_GetPresentationMode(state));
    const char* langStr = language_short(state);
    char modeHint[80];
    snprintf(modeHint, sizeof(modeHint), "PRESENTATION  %s    LANG  %s", modeStr, langStr);
    draw_footer(&c, footerLeft, modeHint);
}

int M12_ModernMenu_CountDistinctColors(const unsigned char* rgba,
                                       int width,
                                       int height,
                                       int capAfter) {
    if (!rgba || width <= 0 || height <= 0) {
        return 0;
    }
    /* 256 * 32 = 8192-bit bitmap for presence by top byte; fall back to
     * linear scan approach using a small hash set. Good enough for a
     * probe. */
    enum { HASH_SIZE = 32768 };
    unsigned int* seen = (unsigned int*)calloc(HASH_SIZE, sizeof(unsigned int));
    if (!seen) return -1;
    int distinct = 0;
    int total = width * height;
    for (int i = 0; i < total; ++i) {
        unsigned int key =
            ((unsigned int)rgba[i * 4 + 0]) |
            (((unsigned int)rgba[i * 4 + 1]) << 8) |
            (((unsigned int)rgba[i * 4 + 2]) << 16);
        /* Linear probing in the hash table, storing key+1 (0 means empty). */
        unsigned int h = ((key * 2654435761U) ^ (key >> 7)) & (HASH_SIZE - 1);
        while (seen[h] != 0 && seen[h] != key + 1) {
            h = (h + 1) & (HASH_SIZE - 1);
        }
        if (seen[h] == 0) {
            seen[h] = key + 1;
            distinct++;
            if (capAfter > 0 && distinct >= capAfter) {
                free(seen);
                return distinct;
            }
        }
    }
    free(seen);
    return distinct;
}
