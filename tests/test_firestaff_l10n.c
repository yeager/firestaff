#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "firestaff_l10n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect_language(const char* locale, FS_Language expected) {
    FS_Language actual = fs_l10n_language_from_locale(locale);
    if (actual != expected) {
        ++failures;
        printf("FAIL locale=%s got=%d want=%d\n", locale ? locale : "(null)",
               (int)actual, (int)expected);
    }
}

int main(void) {
    static const struct {
        const char* locale;
        FS_Language language;
    } cases[] = {
        {"en_US.UTF-8", FS_LANG_EN}, {"sv_SE.UTF-8", FS_LANG_SV},
        {"de_DE", FS_LANG_DE}, {"fr_FR", FS_LANG_FR},
        {"es_ES", FS_LANG_ES}, {"it_IT", FS_LANG_IT},
        {"pt-BR", FS_LANG_PT}, {"nl_NL", FS_LANG_NL},
        {"pl_PL", FS_LANG_PL}, {"cs_CZ", FS_LANG_CS},
        {"ru_RU.UTF-8", FS_LANG_RU}, {"ja_JP", FS_LANG_JA},
        {"ko_KR", FS_LANG_KO}, {"zh_CN", FS_LANG_ZH},
        {"da_DK", FS_LANG_DA}, {"nb_NO", FS_LANG_NO},
        {"fi_FI", FS_LANG_FI}, {"hu_HU", FS_LANG_HU},
        {"tr_TR", FS_LANG_TR}, {"id_ID.UTF-8", FS_LANG_ID}
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        expect_language(cases[i].locale, cases[i].language);
    }
    expect_language("C", FS_LANG_EN);
    expect_language("", FS_LANG_EN);
    expect_language(NULL, FS_LANG_EN);

#if !defined(_WIN32)
    /* A generic C locale is common for GUI/Steam launchers.  It must not
     * mask the first supported LANGUAGE preference, and unsupported entries
     * in that list must fall through to the next real user choice. */
    setenv("LC_ALL", "C.UTF-8", 1);
    unsetenv("LC_MESSAGES");
    setenv("LANGUAGE", "xx:sv_SE:en", 1);
    setenv("LANG", "de_DE.UTF-8", 1);
    if (fs_l10n_detect_system_language() != FS_LANG_SV) {
        ++failures;
        printf("FAIL LANGUAGE list should select Swedish after generic LC_ALL\n");
    }
    setenv("LC_ALL", "fr_FR.UTF-8", 1);
    if (fs_l10n_detect_system_language() != FS_LANG_FR) {
        ++failures;
        printf("FAIL explicit LC_ALL should take precedence\n");
    }
    unsetenv("LC_ALL");
    unsetenv("LC_MESSAGES");
    unsetenv("LANGUAGE");
    unsetenv("LANG");
#endif

    fs_l10n_set_language(FS_LANG_SV);
    if (fs_l10n_get_language() != FS_LANG_SV ||
        fs_l10n_get(FS_STR_SETTINGS) == NULL ||
        fs_l10n_get(FS_STR_AUTO) == NULL ||
        strcmp(fs_l10n_get(FS_STR_AUTO), "Auto") != 0) {
        ++failures;
        printf("FAIL Swedish language table\n");
    }

    /* Every language row must keep the same field order as FS_StringId.
     * This catches missing entries that otherwise shift AUTO/ON/OFF. */
    for (i = 0; i < FS_LANG_COUNT; ++i) {
        fs_l10n_set_language((FS_Language)i);
        if (fs_l10n_get(FS_STR_AUTO)[0] == '\0' ||
            fs_l10n_get(FS_STR_ON)[0] == '\0' ||
            fs_l10n_get(FS_STR_OFF)[0] == '\0') {
            ++failures;
            printf("FAIL language table %d has empty AUTO/ON/OFF\n", (int)i);
        }
    }
    printf("# l10n %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
