#ifndef FIRESTAFF_FONT_CACHE_PC34_COMPAT_H
#define FIRESTAFF_FONT_CACHE_PC34_COMPAT_H

/*
 * Firestaff font cache — multi-language TTF font loader.
 *
 * Each FS_Language has a per-language TTF font file (or a
 * fallback chain).  The cache loads fonts on first use and
 * keeps them resident until shutdown.
 *
 * Font selection per language:
 *   - FS_LANG_EN/SV/DE/FR/ES/IT/PT/NL/PL/CS/DA/NO/FI/HU/TR
 *     Latin Extended: assets/fonts/NotoSans-<lang>.ttf if
 *     present, else system Arial Unicode, else DejaVu Sans
 *   - FS_LANG_RU  Cyrillic: assets/fonts/NotoSans-Regular.ttf
 *     if present, else system fallback
 *   - FS_LANG_JA  Japanese: assets/fonts/NotoSansCJKjp-Regular.otf
 *     if present, else system Hiragino, else fallback
 *   - FS_LANG_KO  Korean: assets/fonts/NotoSansCJKkr-Regular.otf
 *   - FS_LANG_ZH  Chinese: assets/fonts/NotoSansCJKsc-Regular.otf
 *
 * When FIRESTAFF_HAVE_SDL3_TTF is 0, the cache is a stub: all
 * lookup functions return NULL and the bitmap-glyph fallback
 * path is used.  This allows the build to succeed even without
 * SDL3_ttf installed (Linux CI runners without TTF deps).
 *
 * Source-locked to the 19-language l10n set in
 * include/firestaff_l10n.h.
 */

#include "firestaff_l10n.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the font cache.  Returns 1 on success, 0 on failure
 * (e.g. SDL3_ttf not available).  After a failed init, all
 * subsequent lookup calls return NULL. */
int firestaff_font_cache_init(void);

/* Shut down the font cache and free all loaded fonts. */
void firestaff_font_cache_shutdown(void);

/* Returns the absolute path of the TTF file for the given
 * language, or NULL if no font is available.  The returned
 * string is owned by the cache; do NOT free it.  When SDL3_ttf
 * is unavailable, returns NULL. */
const char* firestaff_font_cache_get_path(FS_Language lang);

/* Returns 1 if the cache has a loaded font for this language,
 * 0 otherwise.  After init, this returns 1 for any language
 * that has a system or asset TTF available. */
int firestaff_font_cache_has(FS_Language lang);

/* Returns the FS_Language's primary Unicode block.  Used by
 * callers to decide whether to render via TTF or bitmap
 * fallback.  CJK languages return FS_LANG_JA/KO/ZH; Latin
 * languages return the language itself. */
FS_Language firestaff_font_cache_get_script(FS_Language lang);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FONT_CACHE_PC34_COMPAT_H */
