#include "firestaff_l10n.h"

#include <stdio.h>

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
        {"tr_TR", FS_LANG_TR}
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        expect_language(cases[i].locale, cases[i].language);
    }
    expect_language("C", FS_LANG_EN);
    expect_language("", FS_LANG_EN);
    expect_language(NULL, FS_LANG_EN);

    fs_l10n_set_language(FS_LANG_SV);
    if (fs_l10n_get_language() != FS_LANG_SV ||
        fs_l10n_get(FS_STR_SETTINGS) == NULL) {
        ++failures;
        printf("FAIL Swedish language table\n");
    }
    printf("# l10n %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
