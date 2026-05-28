#include "firestaff_l10n.h"

static FS_Language g_lang = FS_LANG_EN;

static const char *g_strings[FS_LANG_COUNT][FS_STR_COUNT] = {
    /* English */
    [FS_LANG_EN] = {
        "PLAY", "SETTINGS", "EXTRAS", "QUIT",
        "SELECT GAME", "New Game (V1 Original)", "New Game (V2.1 Upscaled)",
        "New Game (V2.2 Enhanced)", "Continue Saved Game", "Coming soon",
        "Back", "Display", "Video", "Audio", "Controls", "Accessibility",
        "Museum of Lore", "Manual / Docs", "Bestiary", "Spell Reference", "Map Viewer",
        "Item Encyclopedia", "Changelog", "Screenshot Gallery",
        "Language", "On", "Off"
    },
    /* Swedish */
    [FS_LANG_SV] = {
        "SPELA", "INSTÄLLNINGAR", "EXTRA", "AVSLUTA",
        "VÄLJ SPEL", "Nytt spel (V1 Original)", "Nytt spel (V2.1 Uppskalat)",
        "Nytt spel (V2.2 Förbättrat)", "Fortsätt sparat spel", "Kommer snart",
        "Tillbaka", "BILDSKÄRM", "Video", "Ljud", "Kontroller", "Tillgänglighet",
        "Kunskapsmuseet", "Handbok / Dokument", "Bestiary", "Trollformler", "Kartvisare",
        "FÖREMÅLSENCYKLOPEDI", "ÄNDRINGSLOGG", "SKÄRMBILDSGALLERI",
        "Språk", "På", "Av"
    },
    /* German */
    [FS_LANG_DE] = {
        "SPIELEN", "EINSTELLUNGEN", "EXTRAS", "BEENDEN",
        "SPIEL WÄHLEN", "Neues Spiel (V1 Original)", "Neues Spiel (V2.1 Hochskaliert)",
        "Neues Spiel (V2.2 Erweitert)", "Gespeichertes Spiel fortsetzen", "Demnächst",
        "Zurück", "Anzeige", "Video", "Audio", "Steuerung", "Barrierefreiheit",
        "Wissensmuseum", "Handbuch / Dokumentation", "Bestiarium", "Zaubersprüche", "Kartenansicht",
        "Gegenstandslexikon", "Änderungsprotokoll", "Screenshot-Galerie",
        "Sprache", "An", "Aus"
    },
    /* French */
    [FS_LANG_FR] = {
        "JOUER", "PARAMÈTRES", "EXTRAS", "QUITTER",
        "CHOISIR UN JEU", "Nouvelle partie (V1 Original)", "Nouvelle partie (V2.1 Amélioré)",
        "Nouvelle partie (V2.2 Enrichi)", "Continuer la partie", "Bientôt disponible",
        "Retour", "Affichage", "Vidéo", "Audio", "Contrôles", "Accessibilité",
        "Musée du Savoir", "Manuel / Documentation", "Bestiaire", "Référence des Sorts", "Visionneuse de Carte",
        "Encyclopédie des Objets", "Journal des Modifications", "Galerie de Captures",
        "Langue", "Activé", "Désactivé"
    },
};

static const char *g_lang_names[FS_LANG_COUNT] = {
    "English", "Svenska", "Deutsch", "Français",
    "Español", "Italiano", "Português", "Nederlands",
    "Polski", "Čeština", "Русский", "日本語",
    "한국어", "简体中文", "Dansk", "Norsk",
    "Suomi", "Magyar", "Türkçe"
};

void fs_l10n_set_language(FS_Language lang) {
    if (lang >= 0 && lang < FS_LANG_COUNT) g_lang = lang;
}

FS_Language fs_l10n_get_language(void) { return g_lang; }

const char *fs_l10n_get(FS_StringId id) {
    if (id < 0 || id >= FS_STR_COUNT) return "???";
    const char *s = g_strings[g_lang][id];
    return s ? s : g_strings[FS_LANG_EN][id];
}

const char *fs_l10n_language_name(FS_Language lang) {
    if (lang < 0 || lang >= FS_LANG_COUNT) return "???";
    return g_lang_names[lang];
}

/* ══════════════════════════════════════════════════════════════════════
 * System language detection + asset language binding
 * ══════════════════════════════════════════════════════════════════════ */

#include <stdlib.h>

FS_Language fs_l10n_detect_system_language(void) {
    /* Check LANG, LC_ALL, LC_MESSAGES environment variables */
    const char *env_vars[] = {"LC_ALL", "LC_MESSAGES", "LANG", NULL};
    int i;
    for (i = 0; env_vars[i]; i++) {
        const char *val = getenv(env_vars[i]);
        if (!val || !val[0]) continue;
        /* Match language prefix: sv, de, fr */
        if (val[0] == 's' && val[1] == 'v') return FS_LANG_SV;
        if (val[0] == 'd' && val[1] == 'e') return FS_LANG_DE;
        if (val[0] == 'f' && val[1] == 'r') return FS_LANG_FR;
        if (val[0] == 'e' && val[1] == 'n') return FS_LANG_EN;
    }
    return FS_LANG_EN; /* default */
}

/* Map UI language → asset dungeon.dat language.
 * Swedish has no DM1 dungeon.dat, falls back to English. */
