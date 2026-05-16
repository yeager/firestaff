#include "firestaff_l10n.h"

static FS_Language g_lang = FS_LANG_EN;

static const char *g_strings[FS_LANG_COUNT][FS_STR_COUNT] = {
    /* English */
    [FS_LANG_EN] = {
        "PLAY", "SETTINGS", "EXTRAS", "QUIT",
        "SELECT GAME", "New Game (V1 Original)", "New Game (V2.1 Upscaled)",
        "New Game (V2.2 Enhanced)", "Continue Saved Game", "Coming soon",
        "Back", "Display", "Video", "Audio", "Controls", "Accessibility",
        "Museum of Lore", "Bestiary", "Spell Reference", "Map Viewer",
        "Item Encyclopedia", "Changelog", "Screenshot Gallery",
        "Language", "On", "Off"
    },
    /* Swedish */
    [FS_LANG_SV] = {
        "SPELA", "INSTÄLLNINGAR", "EXTRA", "AVSLUTA",
        "VÄLJ SPEL", "Nytt spel (V1 Original)", "Nytt spel (V2.1 Uppskalat)",
        "Nytt spel (V2.2 Förbättrat)", "Fortsätt sparat spel", "Kommer snart",
        "Tillbaka", "Bildskärm", "Video", "Ljud", "Kontroller", "Tillgänglighet",
        "Kunskapsmuseet", "Bestiary", "Trollformler", "Kartvisare",
        "Föremålsencyklopedi", "Ändringslogg", "Skärmbildsgalleri",
        "Språk", "På", "Av"
    },
    /* German */
    [FS_LANG_DE] = {
        "SPIELEN", "EINSTELLUNGEN", "EXTRAS", "BEENDEN",
        "SPIEL WÄHLEN", "Neues Spiel (V1 Original)", "Neues Spiel (V2.1 Hochskaliert)",
        "Neues Spiel (V2.2 Erweitert)", "Gespeichertes Spiel fortsetzen", "Demnächst",
        "Zurück", "Anzeige", "Video", "Audio", "Steuerung", "Barrierefreiheit",
        "Wissensmuseum", "Bestiarium", "Zaubersprüche", "Kartenansicht",
        "Gegenstandslexikon", "Änderungsprotokoll", "Screenshot-Galerie",
        "Sprache", "An", "Aus"
    },
    /* French */
    [FS_LANG_FR] = {
        "JOUER", "PARAMÈTRES", "EXTRAS", "QUITTER",
        "CHOISIR UN JEU", "Nouvelle partie (V1 Original)", "Nouvelle partie (V2.1 Amélioré)",
        "Nouvelle partie (V2.2 Enrichi)", "Continuer la partie", "Bientôt disponible",
        "Retour", "Affichage", "Vidéo", "Audio", "Contrôles", "Accessibilité",
        "Musée du Savoir", "Bestiaire", "Référence des Sorts", "Visionneuse de Carte",
        "Encyclopédie des Objets", "Journal des Modifications", "Galerie de Captures",
        "Langue", "Activé", "Désactivé"
    },
};

static const char *g_lang_names[FS_LANG_COUNT] = {
    "English", "Svenska", "Deutsch", "Français"
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
        if (val[0] == s && val[1] == v) return FS_LANG_SV;
        if (val[0] == d && val[1] == e) return FS_LANG_DE;
        if (val[0] == f && val[1] == r) return FS_LANG_FR;
        if (val[0] == e && val[1] == n) return FS_LANG_EN;
    }
    return FS_LANG_EN; /* default */
}

/* Map UI language → asset dungeon.dat language.
 * Swedish doesnt have a DM1 dungeon.dat, falls back to English. IntNetAdmin/ ReDMCSB_WIP20210206/ _media_tmp/ anpr-viewer/ artifacts/ autismapps/ bildordbok/ bildstod-service/ bildstod-writer/ bildstod/ bokstavsresan/ cert-watch/ cldr-viewer/ codex/ commonvoice-status/ crowdin_translation_work/ csbv1-gap/ cve-monitor/ dagboken/ data/ ddtp-bzr/ ddtp-translate/ debian-repo/ desktop-editor/ docs/ elementary-l10n/ emojikitchen/ energimataren/ fedora-l10n/ firestaff-debug/ firestaff-dm1-analysis/ firestaff-land-ready-20260508-2/ firestaff-local/ firestaff-merge-main-20260508/ firestaff-n2-pass84/ firestaff-n2-pool-prompts/ firestaff-passC-edit/ firestaff-safety-20260508-075301/ firestaff-screens/ firestaff-show/ firestaff-v2-gap-manifest/ firestaff-v2-regression-smoke/ firestaff/ firestaff_expected/ firestaff_work/ firewall-manager/ font-preview/ fontforge-pr/ fontforge-repo/ freecad-fork/ github-l10n/ gnome-l10n/ ha-l10n/ hall-/ homebrew-tap/ hunspell-sv/ ilskehanteraren/ inbox-media/ incoming/ kladvaljaren/ klocklararen/ kodi-repo/ kodi-subtitle-translator/ l10n-ci/ l10n-conv/ l10n-glossary/ l10n-lint-repo/ l10n-lint/ l10n-overview-deploy/ l10n-preview/ langpack-inspector/ libretranslate-gui/ libvisio-ng/ libvisio-rs/ linguaedit-website/ linguaedit/ locale-tester/ log-viewer/ mail-attachments/ makebread/ matlagaren/ media-edit/ media/ memory/ mempalace-curated/ mempalace-upstream/ mempalace/ meningsbyggaren/ mousemodev1/ mousev1/ mqtt-dashboard/ mqtt-inspector/ obd2-viewer/ openbve-review/ openclaw-dashboard-plus/ openclaw-studio/ openclaw-sv/ out/ ovningstavlan/ parity-evidence/ pass435-doors/ pass435-explore1/ pass435-forward/ pass435-hall-door/ pass435-junction/ pass435-kp5/ pass435audit/ pauskollen/ pcap-viewer/ pengakollen/ piper-plus/ piper-voices-sv/ plans/ po-diff/ po-translate/ portfolio/ posh-kids/ pptx_env/ process-explorer/ projects/ python-docs-sv/ qt-l10n-dashboard/ raknestod/ references/ regex-tester/ release-work/ releases/ repos/ review-backup-2026-03-14/ routev1/ rpm-repo/ samtalsstod/ screenshots/ scripts/ scummvm-gtk/ signing/ sleepcoach/ smart-home/ snap-l10n/ sonata-swedish/ state/ stegvisaren/ svlang/ swedish-tm/ sysinfo-gtk/ temp_files/ tm-manager/ tmp-axel/ tmp-dl10n/ tmp-dm1/ tmp-firestaff/ tmp-formatjs-pr6200/ tmp-gmail/ tmp-img/ tmp-mempalace-guard/ tmp-mempalace-hardening/ tmp-mempalace-init-yes/ tmp-n2-hall/ tmp-redmcsb-source/ tmp-route-inspect/ tmp/ tmp_firestaff/ tmp_firestaff_controls/ tmp_firestaff_mouse/ tmp_firestaff_route/ tmp_firestaff_spell/ tmp_hall_images/ tools/ tp-lint/ tp-status/ trafficcam/ translations/ tts-tester/ tts-training/ tvtracker/ tx-sync-workspace/ ubuntu-l10n/ ubuntu-repo/ valjaren/ vim-sv/ vim/ visio-viewer-extension/ vsdview-work/ vsdview/ wifi-analyzer/ worktrees/ zanalytics/ zigbee-manager/ zscaler-api-client.wiki/ zscaler-guardian/ zstack/ zstart/
