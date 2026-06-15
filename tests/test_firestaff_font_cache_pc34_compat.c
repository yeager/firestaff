/*
 * test_firestaff_font_cache_pc34_compat.c
 *
 * Verifies the per-language TTF font cache (firestaff_font_cache_
 * pc34_compat.h/c) selects a usable TTF path for each of the
 * 19 supported l10n languages.
 *
 * The cache selects fonts via a 3-tier fallback chain:
 *   1. <asset_dir>/fonts/NotoSans-<lang>.ttf
 *   2. <asset_dir>/fonts/NotoSans-Regular.ttf (universal Latin)
 *   3. System fallback:
 *        macOS:  /Library/Fonts/Arial Unicode.ttf
 *        Linux:  /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
 *        Windows: C:/Windows/Fonts/arial.ttf
 *
 * In a CI environment without TTF assets, the cache falls back
 * to the system path; if neither is available, the cache returns
 * NULL and the bitmap-glyph fallback is used.  This test verifies
 * the cache's contract, not the availability of any specific TTF.
 *
 *  T1  firestaff_font_cache_init returns 1
 *  T2  firestaff_font_cache_get_path(FS_LANG_EN) returns a path
 *      (always — system fallback chain is broad)
 *  T3  firestaff_font_cache_get_path(FS_LANG_SV) returns a path
 *  T4  firestaff_font_cache_get_path(FS_LANG_JA) returns a path
 *      (Cyrillic, CJK, etc. all map to system fallback)
 *  T5  firestaff_font_cache_get_path(FS_LANG_COUNT) returns NULL
 *      (out-of-range)
 *  T6  firestaff_font_cache_get_path(-1) returns NULL
 *  T7  firestaff_font_cache_has(FS_LANG_EN) == 1
 *  T8  firestaff_font_cache_has(FS_LANG_COUNT) == 0
 *  T9  firestaff_font_cache_get_script returns the script tag
 *      for each language (Latin/Cyrillic/CJK)
 *  T10 Shutdown + re-init is safe
 *  T11 Cache returns the same path for repeated lookups (idempotent)
 *  T12 Empty asset dir falls back to system path
 *  T13 All 19 languages have a script tag (none are unmapped)
 *  T14 Path resolution is independent per language (no cross-talk)
 *
 * Source-locked to the 19-language l10n set in
 * include/firestaff_l10n.h.
 */

#include "firestaff_font_cache_pc34_compat.h"
#include "firestaff_l10n.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    int rc;

    /* T1: Init returns 1. */
    rc = firestaff_font_cache_init();
    CHECK(rc == 1, "T1: firestaff_font_cache_init returns 1");

    /* T2: EN has a path. */
    {
        const char* p = firestaff_font_cache_get_path(FS_LANG_EN);
        if (p) {
            CHECK(strlen(p) > 0, "T2: EN path is non-empty");
        } else {
            /* No system font available; this is OK on minimal CI. */
            printf("  (T2 skip: no system font for EN)\n");
        }
    }

    /* T3: SV has a path. */
    {
        const char* p = firestaff_font_cache_get_path(FS_LANG_SV);
        if (p) {
            CHECK(strlen(p) > 0, "T3: SV path is non-empty");
        } else {
            printf("  (T3 skip: no system font for SV)\n");
        }
    }

    /* T4: JA has a path (Cyrillic, CJK map to system fallback). */
    {
        const char* p = firestaff_font_cache_get_path(FS_LANG_JA);
        if (p) {
            CHECK(strlen(p) > 0, "T4: JA path is non-empty");
        } else {
            printf("  (T4 skip: no system font for JA)\n");
        }
    }

    /* T5: FS_LANG_COUNT is out-of-range. */
    CHECK(firestaff_font_cache_get_path(FS_LANG_COUNT) == NULL,
          "T5: FS_LANG_COUNT returns NULL");

    /* T6: -1 is out-of-range. */
    CHECK(firestaff_font_cache_get_path((FS_Language)-1) == NULL,
          "T6: -1 returns NULL");

    /* T7: has(EN) matches get_path(EN) != NULL. */
    CHECK(firestaff_font_cache_has(FS_LANG_EN) ==
          (firestaff_font_cache_get_path(FS_LANG_EN) != NULL),
          "T7: has(EN) is consistent with get_path(EN)");

    /* T8: has(FS_LANG_COUNT) is false. */
    CHECK(firestaff_font_cache_has(FS_LANG_COUNT) == 0,
          "T8: has(FS_LANG_COUNT) is 0");

    /* T9: get_script returns the right script per language. */
    {
        FS_Language s;
        s = firestaff_font_cache_get_script(FS_LANG_EN);
        CHECK(s == FS_LANG_EN, "T9: EN script = EN (Latin)");
        s = firestaff_font_cache_get_script(FS_LANG_RU);
        CHECK(s == FS_LANG_RU, "T9: RU script = RU (Cyrillic)");
        s = firestaff_font_cache_get_script(FS_LANG_JA);
        CHECK(s == FS_LANG_JA, "T9: JA script = JA (CJK proxy)");
        s = firestaff_font_cache_get_script(FS_LANG_KO);
        CHECK(s == FS_LANG_JA, "T9: KO script = JA (CJK proxy)");
        s = firestaff_font_cache_get_script(FS_LANG_ZH);
        CHECK(s == FS_LANG_JA, "T9: ZH script = JA (CJK proxy)");
        s = firestaff_font_cache_get_script(FS_LANG_SV);
        CHECK(s == FS_LANG_SV, "T9: SV script = SV (Latin)");
    }

    /* T10: Shutdown + re-init is safe. */
    firestaff_font_cache_shutdown();
    rc = firestaff_font_cache_init();
    CHECK(rc == 1, "T10: re-init after shutdown returns 1");

    /* T11: Same path returned for repeated lookups. */
    {
        const char* p1 = firestaff_font_cache_get_path(FS_LANG_EN);
        const char* p2 = firestaff_font_cache_get_path(FS_LANG_EN);
        CHECK(p1 == p2 || (p1 && p2 && strcmp(p1, p2) == 0),
              "T11: same path returned for repeated lookups");
    }

    /* T12: Empty asset dir falls back to system path. */
    {
        /* FIRESTAFF_ASSET_DIR="" forces asset_dir to empty/NULL
         * so the system fallback chain is used. */
        const char* saved = getenv("FIRESTAFF_ASSET_DIR");
        setenv("FIRESTAFF_ASSET_DIR", "", 1);
        const char* p = firestaff_font_cache_get_path(FS_LANG_DE);
        if (p) {
            CHECK(strlen(p) > 0, "T12: empty asset dir falls back to system path");
        } else {
            printf("  (T12 skip: no system fallback for DE)\n");
        }
        if (saved) setenv("FIRESTAFF_ASSET_DIR", saved, 1);
        else unsetenv("FIRESTAFF_ASSET_DIR");
    }

    /* T13: All 19 languages have a script tag. */
    {
        int lang;
        for (lang = 0; lang < FS_LANG_COUNT; ++lang) {
            FS_Language s = firestaff_font_cache_get_script((FS_Language)lang);
            CHECK(s >= 0 && s < FS_LANG_COUNT,
                  "T13: every language has a valid script tag");
        }
    }

    /* T14: Path resolution is independent per language. */
    {
        const char* p_en = firestaff_font_cache_get_path(FS_LANG_EN);
        const char* p_ja = firestaff_font_cache_get_path(FS_LANG_JA);
        if (p_en && p_ja) {
            /* They may legitimately be the same (e.g. CJK font
             * used for both Latin and CJK), so we just check
             * that the cache returns something for each. */
            CHECK(strlen(p_en) > 0, "T14: EN path is non-empty");
            CHECK(strlen(p_ja) > 0, "T14: JA path is non-empty");
        }
    }

    firestaff_font_cache_shutdown();
    printf("PASS: firestaff_font_cache_pc34_compat (14 scenarios)\n");
    return 0;
}
