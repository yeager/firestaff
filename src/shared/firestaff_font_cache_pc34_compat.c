/*
 * firestaff_font_cache_pc34_compat.c
 *
 * Per-language TTF font cache.  Loads fonts on demand from
 * assets/fonts/NotoSans-*.ttf (or .otf for CJK) when present,
 * falls back to a system font (Arial Unicode.ttf on macOS,
 * DejaVu Sans on Linux, Arial on Windows) otherwise.
 *
 * Build with FIRESTAFF_HAVE_SDL3_TTF defined by CMake to
 * enable the SDL3_ttf-based font loading.  Without it, the
 * cache is a stub that returns NULL for all lookups, and the
 * existing bitmap-glyph path is used.
 *
 * Source-locked to the 19-language l10n set in
 * include/firestaff_l10n.h.  Fonts are looked up by language
 * code, not by user preference.
 */
#include "firestaff_font_cache_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(FIRESTAFF_HAVE_SDL3_TTF) && FIRESTAFF_HAVE_SDL3_TTF
#include <SDL3_ttf/SDL_ttf.h>
#endif

/* ── Per-language font path table ────────────────────────────────── */

typedef struct {
    FS_Language lang;
    const char* lang_code;       /* ISO 639-1 */
    const char* font_basename;   /* e.g. "NotoSans-sv.ttf" */
    const char* script;          /* "Latin", "Cyrillic", "CJK" */
} LangFontEntry;

static const LangFontEntry kLangFontTable[] = {
    {FS_LANG_EN, "en", "NotoSans-Regular.ttf",     "Latin"},
    {FS_LANG_SV, "sv", "NotoSans-sv.ttf",          "Latin"},
    {FS_LANG_DE, "de", "NotoSans-de.ttf",          "Latin"},
    {FS_LANG_FR, "fr", "NotoSans-fr.ttf",          "Latin"},
    {FS_LANG_ES, "es", "NotoSans-es.ttf",          "Latin"},
    {FS_LANG_IT, "it", "NotoSans-it.ttf",          "Latin"},
    {FS_LANG_PT, "pt", "NotoSans-pt.ttf",          "Latin"},
    {FS_LANG_NL, "nl", "NotoSans-nl.ttf",          "Latin"},
    {FS_LANG_PL, "pl", "NotoSans-pl.ttf",          "Latin"},
    {FS_LANG_CS, "cs", "NotoSans-cs.ttf",          "Latin"},
    {FS_LANG_RU, "ru", "NotoSans-ru.ttf",          "Cyrillic"},
    {FS_LANG_JA, "ja", "NotoSansCJKjp-Regular.otf", "CJK"},
    {FS_LANG_KO, "ko", "NotoSansCJKkr-Regular.otf", "CJK"},
    {FS_LANG_ZH, "zh", "NotoSansCJKsc-Regular.otf", "CJK"},
    {FS_LANG_DA, "da", "NotoSans-da.ttf",          "Latin"},
    {FS_LANG_NO, "no", "NotoSans-no.ttf",          "Latin"},
    {FS_LANG_FI, "fi", "NotoSans-fi.ttf",          "Latin"},
    {FS_LANG_HU, "hu", "NotoSans-hu.ttf",          "Latin"},
    {FS_LANG_TR, "tr", "NotoSans-tr.ttf",          "Latin"},
};

static const size_t kLangFontTableCount =
    sizeof(kLangFontTable) / sizeof(kLangFontTable[0]);

/* ── Cache state ─────────────────────────────────────────────────── */

static int g_initialized = 0;

/* Resolved font paths.  Indexed by FS_Language enum value.
 * NULL means not yet looked up; non-NULL means a usable path
 * was found. */
static const char* g_resolved_paths[FS_LANG_COUNT] = {0};

/* Per-language "is in CJK" flag.  CJK languages share a font
 * (NotoSansCJKjp-Regular covers both Hiragana/Katakana/Kanji),
 * but we cache the resolved path per-language for clarity. */
static const char* g_resolved_scripts[FS_LANG_COUNT] = {0};

/* ── Path resolution ─────────────────────────────────────────────── */

/* Try to read a file.  Returns 1 if the file exists and is
 * readable, 0 otherwise.  Cheap stat()-only check. */
static int file_exists(const char* path) {
    if (!path || !*path) return 0;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* Build a candidate path: <asset_dir>/<basename>.  Caller
 * supplies a writable buffer of size cap. */
static void build_asset_path(char* out, size_t cap,
                             const char* asset_dir,
                             const char* basename) {
    if (!asset_dir || !*asset_dir) {
        out[0] = '\0';
        return;
    }
    snprintf(out, cap, "%s/fonts/%s", asset_dir, basename);
}

/* Lookup the system fallback chain for a language.  Returns
 * a static string or NULL if no system font is available.
 *
 * Platform-specific paths:
 *   macOS:  /Library/Fonts/Arial Unicode.ttf (the only Mac
 *           font that ships with broad Unicode coverage in
 *           a redistributable manner) and /System/Library/Fonts
 *           paths for Hiragino/CJK.
 *   Linux:  /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
 *           (Debian/Ubuntu default).
 *   Windows: C:/Windows/Fonts/arial.ttf
 *
 * This function is best-effort; the cache treats NULL return
 * as "no fallback available". */
static const char* system_fallback_path(FS_Language lang) {
    static const char* kMacLatin = "/Library/Fonts/Arial Unicode.ttf";
    static const char* kMacJapanese = "/System/Library/Fonts/Hiragino Sans GB.ttc";
    static const char* kMacKorean = "/System/Library/Fonts/AppleSDGothicNeo.ttc";
    static const char* kMacCjkFallback = "/System/Library/Fonts/STHeiti Medium.ttc";
    static const char* kLinuxLatin = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    static const char* kWindowsLatin = "C:/Windows/Fonts/arial.ttf";

    if (lang == FS_LANG_JA && file_exists(kMacJapanese)) return kMacJapanese;
    if (lang == FS_LANG_KO && file_exists(kMacKorean)) return kMacKorean;
    if ((lang == FS_LANG_JA || lang == FS_LANG_KO || lang == FS_LANG_ZH)
        && file_exists(kMacCjkFallback)) return kMacCjkFallback;
    if (file_exists(kMacLatin)) return kMacLatin;
    if (file_exists(kLinuxLatin)) return kLinuxLatin;
    if (file_exists(kWindowsLatin)) return kWindowsLatin;
    return NULL;
}

/* Get the asset directory from FIRESTAFF_DATA env var, or fall
 * back to ~/.firestaff/data.  Returns a static buffer owned by
 * the cache (overwritten on each call). */
static const char* get_asset_dir(void) {
    static char buf[1024];
    const char* env = getenv("FIRESTAFF_ASSET_DIR");
    if (env && *env) {
        snprintf(buf, sizeof(buf), "%s", env);
        return buf;
    }
    env = getenv("FIRESTAFF_DATA");
    if (env && *env) {
        snprintf(buf, sizeof(buf), "%s", env);
        return buf;
    }
    const char* home = getenv("HOME");
    if (home && *home) {
        snprintf(buf, sizeof(buf), "%s/.firestaff/data", home);
        return buf;
    }
    return NULL;
}

/* Resolve a font path for a language.  Order of preference:
 *  1. <asset_dir>/fonts/<per-lang-basename>
 *  2. <asset_dir>/fonts/NotoSans-Regular.ttf (universal Latin)
 *  3. System fallback for the language's script
 *  4. NULL (no font available)
 */
static const char* resolve_font_path(FS_Language lang) {
    char asset_path[1024];
    const LangFontEntry* entry = NULL;
    size_t i;
    const char* asset_dir = get_asset_dir();

    /* Find the per-language entry. */
    for (i = 0; i < kLangFontTableCount; ++i) {
        if (kLangFontTable[i].lang == lang) {
            entry = &kLangFontTable[i];
            break;
        }
    }
    if (!entry) return NULL;

    /* 1. Per-language asset. */
    if (asset_dir) {
        build_asset_path(asset_path, sizeof(asset_path),
                         asset_dir, entry->font_basename);
        if (file_exists(asset_path)) {
            static char result[1024];
            snprintf(result, sizeof(result), "%s", asset_path);
            return result;
        }
        /* 2. Universal NotoSans-Regular fallback (Latin script). */
        if (strcmp(entry->script, "Latin") == 0) {
            build_asset_path(asset_path, sizeof(asset_path),
                             asset_dir, "NotoSans-Regular.ttf");
            if (file_exists(asset_path)) {
                static char result2[1024];
                snprintf(result2, sizeof(result2), "%s", asset_path);
                return result2;
            }
        }
        /* 2b. CJK shared fallback. */
        if (strcmp(entry->script, "CJK") == 0) {
            build_asset_path(asset_path, sizeof(asset_path),
                             asset_dir, "NotoSansCJK-Regular.otf");
            if (file_exists(asset_path)) {
                static char result3[1024];
                snprintf(result3, sizeof(result3), "%s", asset_path);
                return result3;
            }
        }
    }

    /* 3. System fallback. */
    return system_fallback_path(lang);
}

/* ── Public API ──────────────────────────────────────────────────── */

int firestaff_font_cache_init(void) {
    if (g_initialized) return 1;
    g_initialized = 1;
    memset(g_resolved_paths, 0, sizeof(g_resolved_paths));
    memset(g_resolved_scripts, 0, sizeof(g_resolved_scripts));
    return 1;
}

void firestaff_font_cache_shutdown(void) {
    g_initialized = 0;
    memset(g_resolved_paths, 0, sizeof(g_resolved_paths));
    memset(g_resolved_scripts, 0, sizeof(g_resolved_scripts));
}

const char* firestaff_font_cache_get_path(FS_Language lang) {
    if (!g_initialized) return NULL;
    if (lang < 0 || lang >= FS_LANG_COUNT) return NULL;
    if (g_resolved_paths[lang] == NULL) {
        g_resolved_paths[lang] = resolve_font_path(lang);
    }
    return g_resolved_paths[lang];
}

int firestaff_font_cache_has(FS_Language lang) {
    return firestaff_font_cache_get_path(lang) != NULL;
}

FS_Language firestaff_font_cache_get_script(FS_Language lang) {
    if (lang < 0 || lang >= FS_LANG_COUNT) return FS_LANG_EN;
    size_t i;
    for (i = 0; i < kLangFontTableCount; ++i) {
        if (kLangFontTable[i].lang == lang) {
            if (strcmp(kLangFontTable[i].script, "CJK") == 0)
                return FS_LANG_JA; /* use JA as CJK proxy */
            if (strcmp(kLangFontTable[i].script, "Cyrillic") == 0)
                return FS_LANG_RU;
            return lang; /* Latin: per-language */
        }
    }
    return FS_LANG_EN;
}
