#ifndef FIRESTAFF_L10N_H
#define FIRESTAFF_L10N_H

/* Firestaff Localization — multi-language string table */

typedef enum {
    FS_LANG_EN = 0,
    FS_LANG_SV,    /* Swedish */
    FS_LANG_DE,    /* German */
    FS_LANG_FR,    /* French */
    FS_LANG_ES,    /* Spanish */
    FS_LANG_IT,    /* Italian */
    FS_LANG_PT,    /* Portuguese */
    FS_LANG_NL,    /* Dutch */
    FS_LANG_PL,    /* Polish */
    FS_LANG_CS,    /* Czech */
    FS_LANG_RU,    /* Russian */
    FS_LANG_JA,    /* Japanese */
    FS_LANG_KO,    /* Korean */
    FS_LANG_ZH,    /* Chinese (Simplified) */
    FS_LANG_DA,    /* Danish */
    FS_LANG_NO,    /* Norwegian */
    FS_LANG_FI,    /* Finnish */
    FS_LANG_HU,    /* Hungarian */
    FS_LANG_TR,    /* Turkish */
    FS_LANG_COUNT  /* 20 languages */
} FS_Language;

typedef enum {
    FS_STR_PLAY = 0,
    FS_STR_SETTINGS,
    FS_STR_EXTRAS,
    FS_STR_QUIT,
    FS_STR_SELECT_GAME,
    FS_STR_NEW_GAME_V1,
    FS_STR_NEW_GAME_V21,
    FS_STR_NEW_GAME_V22,
    FS_STR_CONTINUE,
    FS_STR_COMING_SOON,
    FS_STR_BACK,
    FS_STR_DISPLAY,
    FS_STR_VIDEO,
    FS_STR_AUDIO,
    FS_STR_CONTROLS,
    FS_STR_ACCESSIBILITY,
    FS_STR_MUSEUM,
    FS_STR_BESTIARY,
    FS_STR_SPELLS,
    FS_STR_MAP_VIEWER,
    FS_STR_ITEMS,
    FS_STR_CHANGELOG,
    FS_STR_SCREENSHOTS,
    FS_STR_MANUAL,
    FS_STR_LANGUAGE,
    FS_STR_ON,
    FS_STR_OFF,
    FS_STR_COUNT
} FS_StringId;

void fs_l10n_set_language(FS_Language lang);
FS_Language fs_l10n_get_language(void);
const char *fs_l10n_get(FS_StringId id);
const char *fs_l10n_language_name(FS_Language lang);


FS_Language fs_l10n_detect_system_language(void);
int fs_l10n_to_asset_language(FS_Language ui_lang);
void fs_l10n_init_from_system(void);

#endif

