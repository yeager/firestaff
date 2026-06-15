#ifndef FIRESTAFF_M11_GAME_TEXT_TTF_RENDERER_PC34_COMPAT_H
#define FIRESTAFF_M11_GAME_TEXT_TTF_RENDERER_PC34_COMPAT_H

/*
 * M11 TTF renderer — multi-language Unicode text rendering.
 *
 * Renders UTF-8 strings via SDL3_ttf (with FreeType backend) to
 * the 320x200 DM1 viewport framebuffer.  Used as a fall-forward
 * path when the bitmap-glyph table (m11_game_text_latin_extended_
 * glyphs) cannot represent a codepoint (e.g. Cyrillic, CJK).
 *
 * The renderer is language-aware: the TTF font is selected by
 * fs_l10n_get_language().  CJK languages (ja, ko, zh) require a
 * CJK-capable font (NotoSansCJK-*.otf, Hiragino, etc.) either
 * in assets/fonts/ or as a system fallback.
 *
 * When SDL3_ttf is not available at build time, the renderer
 * is a stub: all functions return 0 and the bitmap-glyph
 * fallback is used.  See CMakeLists.txt FIRESTAFF_HAVE_SDL3_TTF.
 *
 * Source-locked to the 19-language l10n set in
 * include/firestaff_l10n.h.
 */

#ifdef __cplusplus
extern "C" {
#endif

int m11_ttf_renderer_init(void);
void m11_ttf_renderer_shutdown(void);

int m11_ttf_render_string(
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x, int y,
    const char* utf8_text,
    int fontSizePixels,
    unsigned char colorIndex);

const char* m11_ttf_renderer_active_font_path(void);
int m11_ttf_renderer_is_active(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_TEXT_TTF_RENDERER_PC34_COMPAT_H */
