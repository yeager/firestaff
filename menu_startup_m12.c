#include "menu_startup_m12.h"

#include "branding_logo_m12.h"
#include "config_m12.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum {
    M12_COLOR_BLACK = 0,
    M12_COLOR_NAVY = 1,
    M12_COLOR_GREEN = 2,
    M12_COLOR_CYAN = 3,
    M12_COLOR_MAROON = 4,
    M12_COLOR_BROWN = 6,
    M12_COLOR_LIGHT_GRAY = 7,
    M12_COLOR_DARK_GRAY = 8,
    M12_COLOR_LIGHT_BLUE = 9,
    M12_COLOR_LIGHT_GREEN = 10,
    M12_COLOR_LIGHT_CYAN = 11,
    M12_COLOR_LIGHT_RED = 12,
    M12_COLOR_MAGENTA = 13,
    M12_COLOR_YELLOW = 14,
    M12_COLOR_WHITE = 15
};

enum {
    M12_SETTINGS_ROW_LANGUAGE = 0,
    M12_SETTINGS_ROW_GRAPHICS,
    M12_SETTINGS_ROW_WINDOW_MODE,
    M12_SETTINGS_ROW_COUNT
};

static const M12_MenuEntry g_entryTemplate[] = {
    {.title = "DUNGEON MASTER", .gameId = "dm1", .kind = M12_MENU_ENTRY_GAME, .sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG, .available = 0},
    {.title = "CHAOS STRIKES BACK", .gameId = "csb", .kind = M12_MENU_ENTRY_GAME, .sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG, .available = 0},
    {.title = "DUNGEON MASTER II", .gameId = "dm2", .kind = M12_MENU_ENTRY_GAME, .sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG, .available = 0},
    {.title = "SETTINGS", .gameId = NULL, .kind = M12_MENU_ENTRY_SETTINGS, .sourceKind = M12_MENU_SOURCE_SYSTEM, .available = 1}
};

typedef struct {
    char ch;
    unsigned char rows[7];
} M12_Glyph;

typedef struct {
    int scale;
    int tracking;
    unsigned char color;
    int shadowDx;
    int shadowDy;
    unsigned char shadowColor;
} M12_TextStyle;

static const M12_Glyph g_font[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 1, 2, 4, 8, 16, 16}},
    {'>', {1, 2, 4, 8, 4, 2, 1}},
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
    {'M', {17, 27, 21, 17, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 17, 21, 27, 17}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}}
};

static const char* g_languages[] = {"EN", "SV", "FR", "DE"};
static const char* g_graphicsLevels[] = {"ORIGINAL", "UPSCALED", "AI/HD"};
static const char* g_windowModes[] = {"WINDOWED", "FULLSCREEN"};

static const M12_TextStyle g_textSmall = {1, 1, M12_COLOR_WHITE, 0, 0, M12_COLOR_BLACK};
static const M12_TextStyle g_textSmallMuted = {1, 1, M12_COLOR_LIGHT_GRAY, 0, 0, M12_COLOR_BLACK};
static const M12_TextStyle g_textSmallAccent = {1, 1, M12_COLOR_LIGHT_CYAN, 0, 0, M12_COLOR_BLACK};
static const M12_TextStyle g_textSmallShadow = {1, 1, M12_COLOR_WHITE, 1, 1, M12_COLOR_BLACK};
static const M12_TextStyle g_textMediumShadow = {2, 1, M12_COLOR_WHITE, 1, 1, M12_COLOR_BLACK};
static const M12_TextStyle g_textTitleShadow = {4, 1, M12_COLOR_YELLOW, 2, 2, M12_COLOR_MAROON};

typedef enum {
    M12_TEXT_EYEBROW = 0,
    M12_TEXT_SELECT_DESTINATION,
    M12_TEXT_SETTINGS_TITLE,
    M12_TEXT_STATUS,
    M12_TEXT_LAUNCHER_DESTINATIONS,
    M12_TEXT_DATA_DIR,
    M12_TEXT_MAIN_FOOTER,
    M12_TEXT_PERSISTED_OPTIONS,
    M12_TEXT_LANGUAGE,
    M12_TEXT_GRAPHICS_LEVEL,
    M12_TEXT_WINDOW_MODE,
    M12_TEXT_SETTINGS_SAVED,
    M12_TEXT_SETTINGS_FOOTER,
    M12_TEXT_MESSAGE_FOOTER,
    M12_TEXT_READY_TO_LAUNCH,
    M12_TEXT_ESC_RETURNS_TO_MENU,
    M12_TEXT_VALIDATOR_SCAFFOLD_ONLY,
    M12_TEXT_ADD_VERIFIED_RETAIL_HASHES,
    M12_TEXT_GAME_DATA_NOT_FOUND,
    M12_TEXT_CHECK_FIRESTAFF_DATA_DIR,
    M12_TEXT_ART_SLOT_READY,
    M12_TEXT_ART_SLOT_EMPTY,
    M12_TEXT_DROP_ART_INTO_SLOT,
    M12_TEXT_CARD_ART_ACTIVE,
    M12_TEXT_CARD_ART_SLOT,
    M12_TEXT_COUNT
} M12_TextId;

typedef struct {
    unsigned char bandColor;
    unsigned char stripeColor;
    unsigned char glowColor;
    unsigned char titleBorder;
    int overlayMode;
} M12_GraphicsTheme;

static const char* const g_localeText[4][M12_TEXT_COUNT] = {
    {
        "FRONTEND PREVIEW",
        "SELECT A DESTINATION",
        "SETTINGS",
        "STATUS",
        "LAUNCHER DESTINATIONS",
        "DATA DIR",
        "UP/DOWN MOVE   ENTER OPEN   ESC EXIT",
        "PERSISTED OPTIONS",
        "LANGUAGE",
        "GRAPHICS LEVEL",
        "WINDOW MODE",
        "CHANGES SAVE IMMEDIATELY TO CONFIG",
        "LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK",
        "ENTER OR ESC RETURNS TO MENU",
        "READY TO LAUNCH",
        "ESC RETURNS TO MENU",
        "VALIDATOR SCAFFOLD ONLY",
        "ADD VERIFIED RETAIL HASHES",
        "GAME DATA NOT FOUND",
        "CHECK FIRESTAFF DATA DIR",
        "ART SLOT READY",
        "ART SLOT EMPTY",
        "DROP ART INTO SLOT",
        "CARD ART ACTIVE",
        "CARD ART SLOT"
    },
    {
        "FRONTEND FORHANDSGLIMPSE",
        "VALJ MAL",
        "INSTALLNINGAR",
        "STATUS",
        "STARTMAL",
        "DATAKATALOG",
        "UPP/NED FLYTTA   ENTER OPPNA   ESC AVSLUTA",
        "SPARADE VAL",
        "SPRAK",
        "GRAFIKNIVA",
        "FONSTERLAGE",
        "ANDRINGAR SPARAS DIREKT I KONFIG",
        "VANSTER/HOGER VAXLA   ENTER NASTA   ESC TILLBAKA",
        "ENTER ELLER ESC TILL MENYN",
        "KLAR ATT STARTA",
        "ESC TILL MENYN",
        "ENDAST VALIDATOR-SKELETT",
        "LAGG TILL VERIFIERADE RETAILHASHAR",
        "SPELDATA SAKNAS",
        "KONTROLLERA FIRESTAFF DATAKATALOG",
        "KONSTFACK REDO",
        "KONSTFACK TOMT",
        "LAGG KONST I FACKET",
        "KORTBILD AKTIV",
        "KORTBILDSFACK"
    },
    {
        "APERCU DU LANCEUR",
        "CHOISIR UNE DESTINATION",
        "REGLAGES",
        "STATUT",
        "DESTINATIONS DU LANCEUR",
        "DOSSIER DATA",
        "HAUT/BAS DEPLACER   ENTREE OUVRIR   ESC QUITTER",
        "OPTIONS ENREGISTREES",
        "LANGUE",
        "NIVEAU GRAPHIQUE",
        "MODE FENETRE",
        "LES MODIFS SONT SAUVEES TOUT DE SUITE",
        "GAUCHE/DROITE CHANGER   ENTREE AVANCER   ESC RETOUR",
        "ENTREE OU ESC RETOUR MENU",
        "PRET AU LANCEMENT",
        "ESC RETOUR MENU",
        "VALIDATEUR SEULEMENT",
        "AJOUTER LES HACHAGES RETAIL VERIFIES",
        "DONNEES DE JEU ABSENTES",
        "VERIFIER LE DOSSIER DATA FIRESTAFF",
        "EMPLACEMENT ART PRET",
        "EMPLACEMENT ART VIDE",
        "DEPOSER L ART DANS LE SLOT",
        "ART DE CARTE ACTIF",
        "EMPLACEMENT ART"
    },
    {
        "LAUNCHER-VORSCHAU",
        "ZIEL AUSWAHLEN",
        "EINSTELLUNGEN",
        "STATUS",
        "LAUNCHER-ZIELE",
        "DATENORDNER",
        "HOCH/RUNTER BEWEGEN   ENTER OFFNEN   ESC ENDE",
        "GESPEICHERTE OPTIONEN",
        "SPRACHE",
        "GRAFIKSTUFE",
        "FENSTERMODUS",
        "ANDERUNGEN WERDEN SOFORT GESPEICHERT",
        "LINKS/RECHTS WECHSELN   ENTER WEITER   ESC ZURUCK",
        "ENTER ODER ESC ZUM MENU",
        "BEREIT ZUM START",
        "ESC ZUM MENU",
        "NUR VALIDATOR-GERUST",
        "GEPRUFTE RETAIL-HASHES HINZUFUGEN",
        "SPIELDATEN FEHLEN",
        "FIRESTAFF DATENORDNER PRUFEN",
        "ART-SLOT BEREIT",
        "ART-SLOT LEER",
        "ART IM SLOT ABLEGEN",
        "KARTENART AKTIV",
        "KARTENART-SLOT"
    }
};

static const M12_GraphicsTheme g_graphicsThemes[] = {
    {M12_COLOR_NAVY, M12_COLOR_DARK_GRAY, M12_COLOR_BROWN, M12_COLOR_YELLOW, 0},
    {M12_COLOR_LIGHT_BLUE, M12_COLOR_NAVY, M12_COLOR_LIGHT_CYAN, M12_COLOR_WHITE, 1},
    {M12_COLOR_MAGENTA, M12_COLOR_LIGHT_BLUE, M12_COLOR_YELLOW, M12_COLOR_LIGHT_CYAN, 2}
};

enum {
    M12_MODERN_MIN_WIDTH = 400,
    M12_MODERN_MIN_HEIGHT = 240
};

static int m12_entry_count(void) {
    return (int)(sizeof(g_entryTemplate) / sizeof(g_entryTemplate[0]));
}

int M12_StartupMenu_GetEntryCount(void) {
    return m12_entry_count();
}

const M12_MenuEntry* M12_StartupMenu_GetEntry(const M12_StartupMenuState* state,
                                              int index) {
    if (!state || index < 0 || index >= m12_entry_count()) {
        return NULL;
    }
    return &state->entries[index];
}

static int m12_cycle_index(int value, int delta, int count) {
    if (count <= 0) {
        return 0;
    }
    value = (value + delta) % count;
    if (value < 0) {
        value += count;
    }
    return value;
}

static int m12_clamp_index(int value, int count) {
    if (count <= 0) {
        return 0;
    }
    if (value < 0) {
        return 0;
    }
    if (value >= count) {
        return count - 1;
    }
    return value;
}

static int m12_locale_index(const M12_StartupMenuState* state) {
    return state ? m12_clamp_index(state->settings.languageIndex, 4) : 0;
}

static const char* m12_text(const M12_StartupMenuState* state, M12_TextId id) {
    int locale = m12_locale_index(state);
    if (id < 0 || id >= M12_TEXT_COUNT) {
        return "";
    }
    return g_localeText[locale][id];
}

static const M12_GraphicsTheme* m12_theme(const M12_StartupMenuState* state) {
    int index = state ? m12_clamp_index(state->settings.graphicsIndex,
                                        (int)(sizeof(g_graphicsThemes) / sizeof(g_graphicsThemes[0])))
                      : 0;
    return &g_graphicsThemes[index];
}

static void m12_sync_card_art(M12_StartupMenuState* state) {
    int i;
    if (!state) {
        return;
    }
    for (i = 0; i < m12_entry_count(); ++i) {
        M12_CardArt_Resolve(&state->cardArt[i],
                            state->entries[i].gameId,
                            M12_AssetStatus_GetDataDir(&state->assetStatus));
    }
}

static void m12_sync_entries_from_assets(M12_StartupMenuState* state) {
    int i;
    if (!state) {
        return;
    }
    for (i = 0; i < m12_entry_count(); ++i) {
        state->entries[i] = g_entryTemplate[i];
        if (state->entries[i].kind == M12_MENU_ENTRY_GAME) {
            state->entries[i].available = M12_AssetStatus_GameAvailable(&state->assetStatus,
                                                                        state->entries[i].gameId);
        }
    }
}

static void m12_save_settings(const M12_StartupMenuState* state) {
    M12_Config config;
    if (!state) {
        return;
    }
    M12_Config_SetDefaults(&config);
    config.languageIndex = state->settings.languageIndex;
    config.graphicsIndex = state->settings.graphicsIndex;
    config.windowModeIndex = state->settings.windowModeIndex;
    snprintf(config.dataDir, sizeof(config.dataDir), "%s", M12_AssetStatus_GetDataDir(&state->assetStatus));
    M12_Config_Save(&config);
}

static void m12_apply_loaded_config(M12_StartupMenuState* state, const char* dataDirOverride) {
    M12_Config config;
    if (!state) {
        return;
    }
    M12_Config_Load(&config, dataDirOverride);
    state->settings.languageIndex = m12_clamp_index(config.languageIndex,
                                                    (int)(sizeof(g_languages) / sizeof(g_languages[0])));
    state->settings.graphicsIndex = m12_clamp_index(config.graphicsIndex,
                                                    (int)(sizeof(g_graphicsLevels) / sizeof(g_graphicsLevels[0])));
    state->settings.windowModeIndex = m12_clamp_index(config.windowModeIndex,
                                                       (int)(sizeof(g_windowModes) / sizeof(g_windowModes[0])));
    M12_AssetStatus_Scan(&state->assetStatus, config.dataDir);
}

void M12_StartupMenu_Init(M12_StartupMenuState* state) {
    M12_StartupMenu_InitWithDataDir(state, NULL);
}

void M12_StartupMenu_InitWithDataDir(M12_StartupMenuState* state,
                                     const char* dataDir) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    m12_apply_loaded_config(state, dataDir);
    m12_sync_entries_from_assets(state);
    m12_sync_card_art(state);
    state->selectedIndex = 0;
    state->settingsSelectedIndex = 0;
    state->activatedIndex = -1;
    state->view = M12_MENU_VIEW_MAIN;
    state->messageLine1 = "";
    state->messageLine2 = "";
    state->messageLine3 = "";
}

static const char* m12_settings_value_language(const M12_StartupMenuState* state) {
    return g_languages[state->settings.languageIndex];
}

static const char* m12_settings_value_graphics(const M12_StartupMenuState* state) {
    return g_graphicsLevels[state->settings.graphicsIndex];
}

static const char* m12_settings_value_window_mode(const M12_StartupMenuState* state) {
    return g_windowModes[state->settings.windowModeIndex];
}

static void m12_activate_selected(M12_StartupMenuState* state) {
    const M12_MenuEntry* entry;
    if (!state) {
        return;
    }
    entry = M12_StartupMenu_GetEntry(state, state->selectedIndex);
    if (!entry) {
        return;
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        state->view = M12_MENU_VIEW_SETTINGS;
        return;
    }
    state->activatedIndex = state->selectedIndex;
    state->view = M12_MENU_VIEW_MESSAGE;
    if (entry->available) {
        state->messageLine1 = m12_text(state, M12_TEXT_READY_TO_LAUNCH);
        state->messageLine2 = entry->title;
        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
    } else if (!M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        state->messageLine1 = m12_text(state, M12_TEXT_VALIDATOR_SCAFFOLD_ONLY);
        state->messageLine2 = m12_text(state, M12_TEXT_ADD_VERIFIED_RETAIL_HASHES);
        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
    } else {
        state->messageLine1 = m12_text(state, M12_TEXT_GAME_DATA_NOT_FOUND);
        state->messageLine2 = m12_text(state, M12_TEXT_CHECK_FIRESTAFF_DATA_DIR);
        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
    }
}

static void m12_cycle_setting(M12_StartupMenuState* state, int delta) {
    if (!state) {
        return;
    }
    switch (state->settingsSelectedIndex) {
        case M12_SETTINGS_ROW_LANGUAGE:
            state->settings.languageIndex = m12_cycle_index(
                state->settings.languageIndex,
                delta,
                (int)(sizeof(g_languages) / sizeof(g_languages[0])));
            break;
        case M12_SETTINGS_ROW_GRAPHICS:
            state->settings.graphicsIndex = m12_cycle_index(
                state->settings.graphicsIndex,
                delta,
                (int)(sizeof(g_graphicsLevels) / sizeof(g_graphicsLevels[0])));
            break;
        case M12_SETTINGS_ROW_WINDOW_MODE:
            state->settings.windowModeIndex = m12_cycle_index(
                state->settings.windowModeIndex,
                delta,
                (int)(sizeof(g_windowModes) / sizeof(g_windowModes[0])));
            break;
        default:
            break;
    }
    m12_save_settings(state);
}

void M12_StartupMenu_HandleInput(M12_StartupMenuState* state,
                                 M12_MenuInput input) {
    int count;
    if (!state) {
        return;
    }
    count = m12_entry_count();
    if (count <= 0) {
        state->shouldExit = 1;
        return;
    }

    if (state->view == M12_MENU_VIEW_MESSAGE) {
        if (input == M12_MENU_INPUT_BACK || input == M12_MENU_INPUT_ACCEPT) {
            state->view = M12_MENU_VIEW_MAIN;
            state->messageLine1 = "";
            state->messageLine2 = "";
            state->messageLine3 = "";
        }
        return;
    }

    if (state->view == M12_MENU_VIEW_SETTINGS) {
        switch (input) {
            case M12_MENU_INPUT_UP:
                state->settingsSelectedIndex = m12_cycle_index(
                    state->settingsSelectedIndex,
                    -1,
                    M12_SETTINGS_ROW_COUNT);
                break;
            case M12_MENU_INPUT_DOWN:
                state->settingsSelectedIndex = m12_cycle_index(
                    state->settingsSelectedIndex,
                    1,
                    M12_SETTINGS_ROW_COUNT);
                break;
            case M12_MENU_INPUT_LEFT:
                m12_cycle_setting(state, -1);
                break;
            case M12_MENU_INPUT_RIGHT:
            case M12_MENU_INPUT_ACCEPT:
                m12_cycle_setting(state, 1);
                break;
            case M12_MENU_INPUT_BACK:
                state->view = M12_MENU_VIEW_MAIN;
                break;
            case M12_MENU_INPUT_NONE:
            default:
                break;
        }
        return;
    }

    switch (input) {
        case M12_MENU_INPUT_UP:
            state->selectedIndex = m12_cycle_index(state->selectedIndex, -1, count);
            break;
        case M12_MENU_INPUT_DOWN:
            state->selectedIndex = m12_cycle_index(state->selectedIndex, 1, count);
            break;
        case M12_MENU_INPUT_ACCEPT:
        case M12_MENU_INPUT_RIGHT:
            m12_activate_selected(state);
            break;
        case M12_MENU_INPUT_BACK:
        case M12_MENU_INPUT_LEFT:
            state->shouldExit = 1;
            break;
        case M12_MENU_INPUT_NONE:
        default:
            break;
    }
}

static void m12_put_pixel(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          unsigned char color) {
    if (!framebuffer) {
        return;
    }
    if (x < 0 || y < 0 || x >= framebufferWidth || y >= framebufferHeight) {
        return;
    }
    framebuffer[(y * framebufferWidth) + x] = color;
}

static void m12_fill_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int yy;
    int xx;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            m12_put_pixel(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          x + xx,
                          y + yy,
                          color);
        }
    }
}

static void m12_draw_frame(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char borderColor,
                           unsigned char fillColor) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, fillColor);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, 1, borderColor);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y + h - 1, w, 1, borderColor);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, 1, h, borderColor);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 1, y, 1, h, borderColor);
}

static const unsigned char* m12_find_glyph(char ch) {
    size_t i;
    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }
    for (i = 0; i < sizeof(g_font) / sizeof(g_font[0]); ++i) {
        if (g_font[i].ch == ch) {
            return g_font[i].rows;
        }
    }
    return g_font[0].rows;
}

static int m12_glyph_visible_width(const unsigned char* glyph) {
    int right = -1;
    int row;
    int col;
    if (!glyph) {
        return 0;
    }
    for (row = 0; row < 7; ++row) {
        for (col = 4; col >= 0; --col) {
            if (((glyph[row] >> (4 - col)) & 1U) != 0U) {
                if (col > right) {
                    right = col;
                }
                break;
            }
        }
    }
    return right + 1;
}

static int m12_measure_text(const char* text,
                            int scale,
                            int tracking) {
    int width = 0;
    const char* p;
    if (!text || scale <= 0) {
        return 0;
    }
    for (p = text; *p != '\0'; ++p) {
        const unsigned char* glyph = m12_find_glyph(*p);
        int glyphWidth = m12_glyph_visible_width(glyph);
        int advance = (glyphWidth > 0 ? glyphWidth + 1 : 3) * scale;
        width += advance + tracking;
    }
    if (width > 0) {
        width -= tracking;
    }
    return width;
}

static void m12_draw_text_raw(unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight,
                              int x,
                              int y,
                              const char* text,
                              int scale,
                              int tracking,
                              unsigned char color) {
    int cursorX = x;
    const char* p;
    if (!text || scale <= 0) {
        return;
    }
    for (p = text; *p != '\0'; ++p) {
        int row;
        const unsigned char* glyph = m12_find_glyph(*p);
        int glyphWidth = m12_glyph_visible_width(glyph);
        int advance = (glyphWidth > 0 ? glyphWidth + 1 : 3) * scale;
        for (row = 0; row < 7; ++row) {
            int col;
            for (col = 0; col < 5; ++col) {
                if ((glyph[row] >> (4 - col)) & 1U) {
                    m12_fill_rect(framebuffer,
                                  framebufferWidth,
                                  framebufferHeight,
                                  cursorX + (col * scale),
                                  y + (row * scale),
                                  scale,
                                  scale,
                                  color);
                }
            }
        }
        cursorX += advance + tracking;
    }
}

static void m12_draw_text(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          const char* text,
                          const M12_TextStyle* style) {
    const M12_TextStyle* resolved = style ? style : &g_textSmall;
    if (!text) {
        return;
    }
    if (resolved->shadowDx != 0 || resolved->shadowDy != 0) {
        m12_draw_text_raw(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          x + resolved->shadowDx,
                          y + resolved->shadowDy,
                          text,
                          resolved->scale,
                          resolved->tracking,
                          resolved->shadowColor);
    }
    m12_draw_text_raw(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      x,
                      y,
                      text,
                      resolved->scale,
                      resolved->tracking,
                      resolved->color);
}

static void m12_draw_centered_text(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int y,
                                   const char* text,
                                   const M12_TextStyle* style) {
    const M12_TextStyle* resolved = style ? style : &g_textSmall;
    int width;
    int x;
    if (!text) {
        return;
    }
    width = m12_measure_text(text, resolved->scale, resolved->tracking);
    x = (framebufferWidth - width) / 2;
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x,
                  y,
                  text,
                  resolved);
}

static void m12_draw_horizontal_rule(unsigned char* framebuffer,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     int x,
                                     int y,
                                     int w,
                                     unsigned char color) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, 1, color);
}

static void m12_draw_vertical_rule(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int x,
                                   int y,
                                   int h,
                                   unsigned char color) {
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, 1, h, color);
}

static void m12_draw_background(const M12_StartupMenuState* state,
                                unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight) {
    int stripeY;
    const M12_GraphicsTheme* theme = m12_theme(state);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  0,
                  0,
                  framebufferWidth,
                  framebufferHeight,
                  M12_COLOR_BLACK);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  0,
                  0,
                  framebufferWidth,
                  10,
                  theme->stripeColor);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  0,
                  10,
                  framebufferWidth,
                  16,
                  theme->bandColor);
    for (stripeY = 34; stripeY < framebufferHeight; stripeY += 18) {
        m12_fill_rect(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      0,
                      stripeY,
                      framebufferWidth,
                      1,
                      theme->stripeColor);
    }
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  8,
                  30,
                  framebufferWidth - 16,
                  framebufferHeight - 42,
                  M12_COLOR_BLACK);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   8,
                   30,
                   framebufferWidth - 16,
                   framebufferHeight - 42,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
}

static const char* m12_entry_status_text(const M12_MenuEntry* entry) {
    if (!entry) {
        return "OFFLINE";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "OPEN";
    }
    if (entry->available) {
        return "READY";
    }
    if (M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        return "MISSING";
    }
    return "SCHEMA";
}

static unsigned char m12_entry_status_fill(const M12_MenuEntry* entry,
                                           int selected) {
    if (!entry) {
        return M12_COLOR_DARK_GRAY;
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return selected ? M12_COLOR_CYAN : M12_COLOR_DARK_GRAY;
    }
    if (entry->available) {
        return selected ? M12_COLOR_YELLOW : M12_COLOR_GREEN;
    }
    if (M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        return selected ? M12_COLOR_LIGHT_RED : M12_COLOR_MAROON;
    }
    return selected ? M12_COLOR_LIGHT_CYAN : M12_COLOR_NAVY;
}

static unsigned char m12_entry_status_text_color(const M12_MenuEntry* entry,
                                                 int selected) {
    if (!entry) {
        return M12_COLOR_WHITE;
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return M12_COLOR_WHITE;
    }
    if (entry->available) {
        return M12_COLOR_BLACK;
    }
    return selected ? M12_COLOR_BLACK : M12_COLOR_WHITE;
}

static void m12_draw_status_chip(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 const char* text,
                                 unsigned char fill,
                                 unsigned char textColor);

static void m12_draw_title(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           const M12_StartupMenuState* state,
                           const char* eyebrow,
                           const char* subtitle) {
    const M12_GraphicsTheme* theme = m12_theme(state);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   16,
                   12,
                   framebufferWidth - 32,
                   48,
                   theme->titleBorder,
                   theme->bandColor);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   20,
                   16,
                   framebufferWidth - 40,
                   40,
                   theme->glowColor,
                   theme->bandColor);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  20,
                  framebufferWidth - 48,
                  4,
                  theme->glowColor);
    m12_draw_horizontal_rule(framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             24,
                             50,
                             framebufferWidth - 48,
                             M12_COLOR_DARK_GRAY);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           23,
                           "FIRESTAFF",
                           &g_textTitleShadow);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           17,
                           eyebrow,
                           &g_textSmallShadow);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           51,
                           subtitle,
                           &g_textSmallMuted);
}

static void m12_draw_footer(unsigned char* framebuffer,
                            int framebufferWidth,
                            int framebufferHeight,
                            const char* text) {
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   12,
                   framebufferHeight - 20,
                   framebufferWidth - 24,
                   12,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           framebufferHeight - 17,
                           text,
                           &g_textSmallMuted);
}

static unsigned char m12_game_card_fill(const char* gameId) {
    if (gameId && strcmp(gameId, "dm1") == 0) {
        return M12_COLOR_MAROON;
    }
    if (gameId && strcmp(gameId, "csb") == 0) {
        return M12_COLOR_NAVY;
    }
    if (gameId && strcmp(gameId, "dm2") == 0) {
        return M12_COLOR_GREEN;
    }
    return M12_COLOR_DARK_GRAY;
}

static const char* m12_game_card_line1(const M12_MenuEntry* entry) {
    if (!entry) {
        return "CONFIG";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "CONFIG";
    }
    if (entry->gameId && strcmp(entry->gameId, "dm1") == 0) {
        return "DM1";
    }
    if (entry->gameId && strcmp(entry->gameId, "csb") == 0) {
        return "CSB";
    }
    if (entry->gameId && strcmp(entry->gameId, "dm2") == 0) {
        return "DM2";
    }
    return "GAME";
}

static const char* m12_game_card_line2(const M12_MenuEntry* entry) {
    if (!entry) {
        return "PERSISTED";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "PERSISTED";
    }
    if (entry->gameId && strcmp(entry->gameId, "dm1") == 0) {
        return "LEGACY EDITION";
    }
    if (entry->gameId && strcmp(entry->gameId, "csb") == 0) {
        return "CHAOS CAMPAIGN";
    }
    if (entry->gameId && strcmp(entry->gameId, "dm2") == 0) {
        return "SKULLKEEP ERA";
    }
    return "READY";
}

static const char* m12_game_card_line3(const M12_MenuEntry* entry) {
    if (!entry) {
        return "SETTINGS";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "SETTINGS";
    }
    if (entry->available) {
        return "READY TO LAUNCH";
    }
    if (M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        return "ASSETS MISSING";
    }
    return "HASH COVERAGE PENDING";
}

static void m12_format_hash_summary(const M12_MenuEntry* entry,
                                    char* out,
                                    size_t outSize) {
    unsigned long hashCount;
    unsigned long verifiedFiles;
    unsigned long requiredFiles;
    if (!out || outSize == 0U) {
        return;
    }
    out[0] = '\0';
    if (!entry || entry->kind == M12_MENU_ENTRY_SETTINGS) {
        snprintf(out, outSize, "PROFILE AND DATA PATH");
        return;
    }
    hashCount = (unsigned long)M12_AssetStatus_GameKnownHashCount(entry->gameId);
    verifiedFiles = (unsigned long)M12_AssetStatus_GameVerifiedFileCount(entry->gameId);
    requiredFiles = (unsigned long)M12_AssetStatus_GameRequiredFileCount(entry->gameId);
    if (!M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        snprintf(out,
                 outSize,
                 "%lu OF %lu FILES HASHED",
                 verifiedFiles,
                 requiredFiles);
        return;
    }
    snprintf(out,
             outSize,
             "%lu KNOWN MD5%s",
             hashCount,
             hashCount == 1UL ? "" : "S");
}

static void m12_draw_box_motif(unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               int x,
                               int y,
                               int w,
                               int h,
                               const M12_MenuEntry* entry,
                               unsigned char accent) {
    if (!entry || entry->kind == M12_MENU_ENTRY_SETTINGS) {
        m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight, x + 10, y + 6, w - 20, h - 12, M12_COLOR_LIGHT_CYAN, M12_COLOR_BLACK);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 16, y + 12, w - 32, 4, accent);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 16, y + 22, w - 24, 4, M12_COLOR_DARK_GRAY);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 16, y + 32, w - 36, 4, M12_COLOR_DARK_GRAY);
        return;
    }
    if (entry->gameId && strcmp(entry->gameId, "dm1") == 0) {
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 14, y + 10, w - 28, h - 14, M12_COLOR_BROWN);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 20, y + 16, w - 40, h - 20, M12_COLOR_BLACK);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 24, y + 16, 5, h - 20, accent);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + w - 29, y + 16, 5, h - 20, accent);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 32, y + 24, w - 64, 5, accent);
        return;
    }
    if (entry->gameId && strcmp(entry->gameId, "csb") == 0) {
        m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight, x + 12, y + 8, w - 24, h - 16, M12_COLOR_LIGHT_CYAN, M12_COLOR_BLACK);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 22, y + 18, w - 44, 4, accent);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 22, y + 32, w - 44, 4, accent);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 30, y + 22, w - 60, 10, M12_COLOR_BLACK);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 4, y + 22, 8, 10, M12_COLOR_LIGHT_CYAN);
        return;
    }
    m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight, x + 12, y + 8, w - 24, h - 16, M12_COLOR_WHITE, M12_COLOR_BLACK);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 18, y + 14, w - 36, 6, accent);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 18, y + 26, w - 36, 6, M12_COLOR_DARK_GRAY);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 8, y + 34, 16, 8, M12_COLOR_WHITE);
    m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + (w / 2) - 3, y + 42, 6, 4, M12_COLOR_WHITE);
}

static void m12_draw_card_preview(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  const M12_MenuEntry* entry,
                                  const M12_GameCardArt* art,
                                  int x,
                                  int y,
                                  int w,
                                  int h,
                                  unsigned char fill,
                                  unsigned char accent) {
    if (entry && entry->kind != M12_MENU_ENTRY_SETTINGS && art && M12_CardArt_HasImage(art)) {
        M12_CardArt_DrawPreview(art,
                                framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x,
                                y,
                                w,
                                h,
                                fill,
                                M12_COLOR_BLACK,
                                accent,
                                M12_COLOR_YELLOW);
        return;
    }
    m12_draw_box_motif(framebuffer,
                       framebufferWidth,
                       framebufferHeight,
                       x,
                       y,
                       w,
                       h,
                       entry,
                       accent);
}

static void m12_draw_box_art(const M12_StartupMenuState* state,
                             unsigned char* framebuffer,
                             int framebufferWidth,
                             int framebufferHeight,
                             const M12_MenuEntry* entry,
                             const M12_GameCardArt* art) {
    unsigned char fill = entry && entry->kind == M12_MENU_ENTRY_SETTINGS
                             ? M12_COLOR_DARK_GRAY
                             : m12_game_card_fill(entry ? entry->gameId : NULL);
    unsigned char accent = entry && entry->available ? M12_COLOR_YELLOW : M12_COLOR_LIGHT_RED;
    unsigned char statusFill = m12_entry_status_fill(entry, 0);
    unsigned char statusText = m12_entry_status_text_color(entry, 0);
    char hashSummary[64];

    m12_format_hash_summary(entry, hashSummary, sizeof(hashSummary));

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   66,
                   94,
                   108,
                   M12_COLOR_YELLOW,
                   fill);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   22,
                   70,
                   86,
                   100,
                   M12_COLOR_BLACK,
                   fill);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  72,
                  80,
                  6,
                  accent);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  80,
                  80,
                  48,
                  M12_COLOR_BLACK);
    m12_draw_card_preview(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          entry,
                          art,
                          24,
                          80,
                          80,
                          48,
                          fill,
                          accent);
    m12_draw_horizontal_rule(framebuffer, framebufferWidth, framebufferHeight, 24, 130, 80, M12_COLOR_BLACK);
    m12_draw_horizontal_rule(framebuffer, framebufferWidth, framebufferHeight, 24, 150, 80, M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           152,
                           hashSummary,
                           &g_textSmallMuted);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           142,
                           (art && M12_CardArt_HasImage(art))
                               ? m12_text(state, M12_TEXT_CARD_ART_ACTIVE)
                               : m12_text(state, M12_TEXT_CARD_ART_SLOT),
                           &g_textSmallMuted);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           134,
                           m12_game_card_line1(entry),
                           &g_textMediumShadow);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           151,
                           m12_game_card_line2(entry),
                           &g_textSmallShadow);
    m12_draw_status_chip(framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         33,
                         82,
                         m12_entry_status_text(entry),
                         statusFill,
                         statusText);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           164,
                           m12_game_card_line3(entry),
                           entry && entry->available ? &g_textSmallAccent : &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  28,
                  104,
                  (art && M12_CardArt_HasExternalFile(art))
                      ? m12_text(state, M12_TEXT_ART_SLOT_READY)
                      : m12_text(state, M12_TEXT_CARD_ART_ACTIVE),
                  &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  28,
                  114,
                  (art && M12_CardArt_HasExternalFile(art))
                      ? M12_CardArt_GetFileName(art)
                      : M12_CardArt_GetSlotLabel(art),
                  &g_textSmallShadow);
}

static void m12_draw_status_chip(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 const char* text,
                                 unsigned char fill,
                                 unsigned char textColor) {
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   58,
                   13,
                   M12_COLOR_BLACK,
                   fill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + 2,
                  text,
                  &(M12_TextStyle){1, 1, textColor, 0, 0, M12_COLOR_BLACK});
}

static void m12_draw_main_row(unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight,
                              int y,
                              const M12_MenuEntry* entry,
                              int selected) {
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char titleColor = entry->available ? M12_COLOR_WHITE : M12_COLOR_LIGHT_GRAY;

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   122,
                   y,
                   180,
                   24,
                   border,
                   fill);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   124,
                   y + 2,
                   176,
                   20,
                   selected ? M12_COLOR_BROWN : M12_COLOR_BLACK,
                   fill);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  126,
                  y + 4,
                  6,
                  14,
                  selected ? M12_COLOR_YELLOW : (entry->available ? M12_COLOR_GREEN : M12_COLOR_LIGHT_RED));
    m12_draw_vertical_rule(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           138,
                           y + 4,
                           14,
                           M12_COLOR_DARK_GRAY);
    if (selected) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      144,
                      y + 7,
                      ">",
                      &g_textSmallAccent);
    }
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  158,
                  y + 7,
                  entry->title,
                  &(M12_TextStyle){1, 1, titleColor, 1, 1, M12_COLOR_BLACK});

    m12_draw_status_chip(framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         236,
                         y + 5,
                         m12_entry_status_text(entry),
                         m12_entry_status_fill(entry, selected),
                         m12_entry_status_text_color(entry, selected));
}

static void m12_draw_main_view(const M12_StartupMenuState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    int i;
    int rowY = 76;
    const M12_MenuEntry* selectedEntry = M12_StartupMenu_GetEntry(state, state->selectedIndex);

    m12_draw_title(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   state,
                   m12_text(state, M12_TEXT_EYEBROW),
                   m12_text(state, M12_TEXT_SELECT_DESTINATION));
    m12_draw_box_art(state,
                     framebuffer,
                     framebufferWidth,
                     framebufferHeight,
                     selectedEntry,
                     (state->selectedIndex >= 0 && state->selectedIndex < m12_entry_count())
                         ? &state->cardArt[state->selectedIndex]
                         : NULL);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   118,
                   66,
                   188,
                   108,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  126,
                  72,
                  m12_text(state, M12_TEXT_LAUNCHER_DESTINATIONS),
                  &g_textSmallAccent);
    for (i = 0; i < m12_entry_count(); ++i) {
        m12_draw_main_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          rowY,
                          &state->entries[i],
                          i == state->selectedIndex);
        rowY += 24;
    }

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   170,
                   framebufferWidth - 36,
                   18,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  175,
                  m12_text(state, M12_TEXT_DATA_DIR),
                  &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  84,
                  175,
                  M12_AssetStatus_GetDataDir(&state->assetStatus),
                  &g_textSmallShadow);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_MAIN_FOOTER));
}

static void m12_draw_settings_row(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  int y,
                                  const char* label,
                                  const char* value,
                                  int selected) {
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueFill = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueText = selected ? M12_COLOR_BLACK : M12_COLOR_WHITE;

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   122,
                   y,
                   180,
                   24,
                   border,
                   fill);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   124,
                   y + 2,
                   176,
                   20,
                   selected ? M12_COLOR_BROWN : M12_COLOR_BLACK,
                   fill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  130,
                  y + 8,
                  label,
                  &g_textSmallShadow);
    m12_draw_status_chip(framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         236,
                         y + 6,
                         value,
                         valueFill,
                         valueText);
}

static void m12_draw_settings_view(const M12_StartupMenuState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    M12_MenuEntry settingsCard = {
        .title = "SETTINGS",
        .gameId = NULL,
        .kind = M12_MENU_ENTRY_SETTINGS,
        .sourceKind = M12_MENU_SOURCE_SYSTEM,
        .available = 1
    };
    m12_draw_title(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   state,
                   m12_text(state, M12_TEXT_EYEBROW),
                   m12_text(state, M12_TEXT_SETTINGS_TITLE));
    m12_draw_box_art(state,
                     framebuffer,
                     framebufferWidth,
                     framebufferHeight,
                     &settingsCard,
                     NULL);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   118,
                   66,
                   188,
                   108,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  126,
                  72,
                  m12_text(state, M12_TEXT_PERSISTED_OPTIONS),
                  &g_textSmallAccent);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          84,
                          m12_text(state, M12_TEXT_LANGUAGE),
                          m12_settings_value_language(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_LANGUAGE);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          112,
                          m12_text(state, M12_TEXT_GRAPHICS_LEVEL),
                          m12_settings_value_graphics(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_GRAPHICS);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          140,
                          m12_text(state, M12_TEXT_WINDOW_MODE),
                          m12_settings_value_window_mode(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   170,
                   framebufferWidth - 36,
                   18,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  175,
                  m12_text(state, M12_TEXT_SETTINGS_SAVED),
                  &g_textSmallMuted);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_SETTINGS_FOOTER));
}

static void m12_draw_message_view(const M12_StartupMenuState* state,
                                  unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight) {
    unsigned char messageColor = M12_COLOR_GREEN;
    const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, state->activatedIndex);

    if (entry && !entry->available) {
        messageColor = M12_COLOR_LIGHT_RED;
    }

    m12_draw_title(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   state,
                   m12_text(state, M12_TEXT_EYEBROW),
                   m12_text(state, M12_TEXT_STATUS));
    m12_draw_box_art(state,
                     framebuffer,
                     framebufferWidth,
                     framebufferHeight,
                     entry,
                     (state->activatedIndex >= 0 && state->activatedIndex < m12_entry_count())
                         ? &state->cardArt[state->activatedIndex]
                         : NULL);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   118,
                   74,
                   188,
                   88,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           90,
                           state->messageLine1,
                           &(M12_TextStyle){1, 1, messageColor, 1, 1, M12_COLOR_BLACK});
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           112,
                           state->messageLine2,
                           &g_textSmallShadow);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           134,
                           state->messageLine3,
                           &g_textSmallMuted);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_MESSAGE_FOOTER));
}

static void m12_apply_graphics_overlay(const M12_StartupMenuState* state,
                                       unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight) {
    const M12_GraphicsTheme* theme = m12_theme(state);
    int x;
    int y;
    if (theme->overlayMode == 1) {
        for (y = 36; y < framebufferHeight - 24; y += 6) {
            for (x = 10 + ((y / 6) & 1); x < framebufferWidth - 10; x += 8) {
                m12_put_pixel(framebuffer,
                              framebufferWidth,
                              framebufferHeight,
                              x,
                              y,
                              theme->glowColor);
            }
        }
        m12_draw_frame(framebuffer,
                       framebufferWidth,
                       framebufferHeight,
                       14,
                       34,
                       framebufferWidth - 28,
                       framebufferHeight - 50,
                       theme->glowColor,
                       M12_COLOR_BLACK);
        return;
    }
    if (theme->overlayMode == 2) {
        for (x = 12; x < framebufferWidth - 12; x += 12) {
            m12_fill_rect(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          x,
                          32,
                          2,
                          framebufferHeight - 52,
                          theme->glowColor);
        }
        for (y = 40; y < framebufferHeight - 26; y += 10) {
            m12_fill_rect(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          12,
                          y,
                          framebufferWidth - 24,
                          1,
                          theme->titleBorder);
        }
        m12_draw_frame(framebuffer,
                       framebufferWidth,
                       framebufferHeight,
                       10,
                       32,
                       framebufferWidth - 20,
                       framebufferHeight - 46,
                       theme->titleBorder,
                       M12_COLOR_BLACK);
    }
}

static int m12_use_modern_layout(int framebufferWidth,
                                 int framebufferHeight) {
    return framebufferWidth >= M12_MODERN_MIN_WIDTH &&
           framebufferHeight >= M12_MODERN_MIN_HEIGHT;
}

static void m12_draw_branding_logo(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int x,
                                   int y,
                                   int scale) {
    int yy;
    int xx;
    int drawScale = scale < 1 ? 1 : scale;
    for (yy = 0; yy < M12_BRANDING_LOGO_HEIGHT; ++yy) {
        for (xx = 0; xx < M12_BRANDING_LOGO_WIDTH; ++xx) {
            size_t index = (size_t)yy * (size_t)M12_BRANDING_LOGO_WIDTH + (size_t)xx;
            unsigned char color = g_m12BrandingLogoPixels[index];
            if (!g_m12BrandingLogoMask[index]) {
                continue;
            }
            m12_fill_rect(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          x + (xx * drawScale),
                          y + (yy * drawScale),
                          drawScale,
                          drawScale,
                          color);
        }
    }
}

static void m12_draw_modern_status_pill(unsigned char* framebuffer,
                                        int framebufferWidth,
                                        int framebufferHeight,
                                        int x,
                                        int y,
                                        int w,
                                        const char* text,
                                        unsigned char fill,
                                        unsigned char border,
                                        unsigned char textColor) {
    int innerW = w < 40 ? 40 : w;
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   innerW,
                   14,
                   border,
                   fill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 6,
                  y + 3,
                  text,
                  &(M12_TextStyle){1, 1, textColor, 0, 0, M12_COLOR_BLACK});
}

static void m12_draw_modern_background(const M12_StartupMenuState* state,
                                       unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight) {
    const M12_GraphicsTheme* theme = m12_theme(state);
    int y;
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  0,
                  0,
                  framebufferWidth,
                  framebufferHeight,
                  M12_COLOR_BLACK);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  0,
                  0,
                  framebufferWidth,
                  framebufferHeight / 3,
                  theme->bandColor);
    for (y = 0; y < framebufferHeight; y += 12) {
        m12_fill_rect(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      0,
                      y,
                      framebufferWidth,
                      1,
                      (y / 12) & 1 ? theme->stripeColor : theme->glowColor);
    }
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  8,
                  8,
                  framebufferWidth - 16,
                  framebufferHeight - 16,
                  M12_COLOR_BLACK);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   8,
                   8,
                   framebufferWidth - 16,
                   framebufferHeight - 16,
                   theme->titleBorder,
                   M12_COLOR_BLACK);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  18,
                  18,
                  framebufferWidth - 36,
                  2,
                  theme->glowColor);
}

static void m12_draw_modern_hero(const M12_StartupMenuState* state,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int w,
                                 int h,
                                 const char* subtitle) {
    const M12_GraphicsTheme* theme = m12_theme(state);
    int logoScale = framebufferWidth >= 640 ? 2 : 1;
    int logoW = M12_BRANDING_LOGO_WIDTH * logoScale;
    int logoH = M12_BRANDING_LOGO_HEIGHT * logoScale;
    int logoX = x + 16;
    int logoY = y + (h - logoH) / 2;
    int textX = logoX + logoW + 18;
    int pillY = y + h - 22;
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   h,
                   theme->titleBorder,
                   theme->bandColor);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x + 4,
                   y + 4,
                   w - 8,
                   h - 8,
                   theme->glowColor,
                   theme->bandColor);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + 8,
                  w - 16,
                  6,
                  theme->glowColor);
    m12_draw_branding_logo(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           logoX,
                           logoY,
                           logoScale);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  textX,
                  y + 18,
                  m12_text(state, M12_TEXT_EYEBROW),
                  &g_textSmallShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  textX,
                  y + 36,
                  subtitle,
                  &g_textMediumShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  textX,
                  y + 56,
                  m12_text(state, M12_TEXT_READY_TO_LAUNCH),
                  &g_textSmallAccent);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                textX,
                                pillY,
                                58,
                                m12_settings_value_language(state),
                                M12_COLOR_DARK_GRAY,
                                theme->glowColor,
                                M12_COLOR_WHITE);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                textX + 64,
                                pillY,
                                88,
                                m12_settings_value_graphics(state),
                                M12_COLOR_DARK_GRAY,
                                theme->glowColor,
                                M12_COLOR_WHITE);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                textX + 158,
                                pillY,
                                92,
                                m12_settings_value_window_mode(state),
                                M12_COLOR_DARK_GRAY,
                                theme->glowColor,
                                M12_COLOR_WHITE);
}

static const char* m12_entry_detail_line(const M12_MenuEntry* entry) {
    if (!entry) {
        return "OFFLINE";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "PERSISTED OPTIONS AND DISPLAY MODE";
    }
    if (entry->available) {
        return "VERIFIED DATA READY";
    }
    if (M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
        return "HASHED, BUT FILES ARE MISSING";
    }
    return "KNOWN SLOT, HASH COVERAGE STILL GROWING";
}

static void m12_draw_modern_main_row(unsigned char* framebuffer,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     int x,
                                     int y,
                                     int w,
                                     const M12_MenuEntry* entry,
                                     int selected) {
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char accent = selected ? M12_COLOR_YELLOW
                                    : (entry && entry->available ? M12_COLOR_GREEN : M12_COLOR_LIGHT_RED);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   36,
                   border,
                   fill);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + 8,
                  6,
                  20,
                  accent);
    if (selected) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      x + 20,
                      y + 6,
                      ">",
                      &g_textSmallAccent);
    }
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 34,
                  y + 7,
                  entry ? entry->title : "UNKNOWN",
                  &g_textSmallShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 34,
                  y + 20,
                  m12_entry_detail_line(entry),
                  &g_textSmallMuted);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x + w - 74,
                                y + 11,
                                64,
                                m12_entry_status_text(entry),
                                m12_entry_status_fill(entry, selected),
                                M12_COLOR_BLACK,
                                m12_entry_status_text_color(entry, selected));
}

static void m12_draw_modern_sidebar(const M12_StartupMenuState* unusedState,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    const M12_MenuEntry* entry,
                                    const M12_GameCardArt* art,
                                    const char* title,
                                    const char* footerText) {
    unsigned char fill = entry && entry->kind == M12_MENU_ENTRY_SETTINGS
                             ? M12_COLOR_DARK_GRAY
                             : m12_game_card_fill(entry ? entry->gameId : NULL);
    unsigned char accent = entry && entry->available ? M12_COLOR_YELLOW : M12_COLOR_LIGHT_RED;
    char hashSummary[64];
    (void)unusedState;
    m12_format_hash_summary(entry, hashSummary, sizeof(hashSummary));
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   h,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + 10,
                  title,
                  &g_textSmallAccent);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + 22,
                  w - 20,
                  2,
                  accent);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + 30,
                  w - 20,
                  h / 2 - 10,
                  fill);
    m12_draw_card_preview(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          entry,
                          art,
                          x + 12,
                          y + 32,
                          w - 24,
                          h / 2 - 14,
                          fill,
                          accent);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + h / 2 + 18,
                  entry ? m12_game_card_line2(entry) : "NO PROFILE",
                  &g_textSmallShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + h / 2 + 31,
                  entry ? m12_game_card_line3(entry) : "",
                  entry && entry->available ? &g_textSmallAccent : &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + h / 2 + 47,
                  hashSummary,
                  &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + h - 24,
                  footerText,
                  &g_textSmallMuted);
}

static void m12_draw_modern_settings_row(unsigned char* framebuffer,
                                         int framebufferWidth,
                                         int framebufferHeight,
                                         int x,
                                         int y,
                                         int w,
                                         const char* label,
                                         const char* value,
                                         int selected) {
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueFill = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueText = selected ? M12_COLOR_BLACK : M12_COLOR_WHITE;
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   28,
                   border,
                   fill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + 10,
                  label,
                  &g_textSmallShadow);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x + w - 92,
                                y + 7,
                                82,
                                value,
                                valueFill,
                                border,
                                valueText);
}

static void m12_draw_main_view_modern(const M12_StartupMenuState* state,
                                      unsigned char* framebuffer,
                                      int framebufferWidth,
                                      int framebufferHeight) {
    const M12_MenuEntry* selectedEntry = M12_StartupMenu_GetEntry(state, state->selectedIndex);
    const M12_GameCardArt* selectedArt = (state->selectedIndex >= 0 && state->selectedIndex < m12_entry_count())
                                             ? &state->cardArt[state->selectedIndex]
                                             : NULL;
    int margin = framebufferWidth / 30;
    int heroH = framebufferHeight / 3;
    int contentY;
    int leftW;
    int rightX;
    int rowY;
    int i;
    if (margin < 12) {
        margin = 12;
    }
    if (heroH < 82) {
        heroH = 82;
    }
    contentY = margin + heroH + 10;
    leftW = (framebufferWidth * 56) / 100;
    rightX = margin + leftW + 10;

    m12_draw_modern_background(state, framebuffer, framebufferWidth, framebufferHeight);
    m12_draw_modern_hero(state,
                         framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         margin,
                         margin,
                         framebufferWidth - (margin * 2),
                         heroH,
                         m12_text(state, M12_TEXT_SELECT_DESTINATION));
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  margin,
                  contentY,
                  m12_text(state, M12_TEXT_LAUNCHER_DESTINATIONS),
                  &g_textSmallAccent);
    rowY = contentY + 12;
    for (i = 0; i < m12_entry_count(); ++i) {
        m12_draw_modern_main_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 margin,
                                 rowY,
                                 leftW - 4,
                                 &state->entries[i],
                                 i == state->selectedIndex);
        rowY += 42;
    }
    m12_draw_modern_sidebar(state,
                            framebuffer,
                            framebufferWidth,
                            framebufferHeight,
                            rightX,
                            contentY,
                            framebufferWidth - margin - rightX,
                            framebufferHeight - contentY - 28,
                            selectedEntry,
                            selectedArt,
                            m12_text(state, M12_TEXT_STATUS),
                            M12_AssetStatus_GetDataDir(&state->assetStatus));
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_MAIN_FOOTER));
}

static void m12_draw_settings_view_modern(const M12_StartupMenuState* state,
                                          unsigned char* framebuffer,
                                          int framebufferWidth,
                                          int framebufferHeight) {
    M12_MenuEntry settingsCard = {
        .title = "SETTINGS",
        .gameId = NULL,
        .kind = M12_MENU_ENTRY_SETTINGS,
        .sourceKind = M12_MENU_SOURCE_SYSTEM,
        .available = 1
    };
    int margin = framebufferWidth / 30;
    int heroH = framebufferHeight / 3;
    int contentY;
    int leftW;
    int panelX;
    if (margin < 12) {
        margin = 12;
    }
    if (heroH < 82) {
        heroH = 82;
    }
    contentY = margin + heroH + 10;
    leftW = (framebufferWidth * 38) / 100;
    panelX = margin + leftW + 12;
    m12_draw_modern_background(state, framebuffer, framebufferWidth, framebufferHeight);
    m12_draw_modern_hero(state,
                         framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         margin,
                         margin,
                         framebufferWidth - (margin * 2),
                         heroH,
                         m12_text(state, M12_TEXT_SETTINGS_TITLE));
    m12_draw_modern_sidebar(state,
                            framebuffer,
                            framebufferWidth,
                            framebufferHeight,
                            margin,
                            contentY,
                            leftW,
                            framebufferHeight - contentY - 28,
                            &settingsCard,
                            NULL,
                            m12_text(state, M12_TEXT_PERSISTED_OPTIONS),
                            m12_text(state, M12_TEXT_SETTINGS_SAVED));
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   panelX,
                   contentY,
                   framebufferWidth - margin - panelX,
                   framebufferHeight - contentY - 28,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  panelX + 10,
                  contentY + 10,
                  m12_text(state, M12_TEXT_PERSISTED_OPTIONS),
                  &g_textSmallAccent);
    m12_draw_modern_settings_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 panelX + 10,
                                 contentY + 28,
                                 framebufferWidth - margin - panelX - 20,
                                 m12_text(state, M12_TEXT_LANGUAGE),
                                 m12_settings_value_language(state),
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_LANGUAGE);
    m12_draw_modern_settings_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 panelX + 10,
                                 contentY + 60,
                                 framebufferWidth - margin - panelX - 20,
                                 m12_text(state, M12_TEXT_GRAPHICS_LEVEL),
                                 m12_settings_value_graphics(state),
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_GRAPHICS);
    m12_draw_modern_settings_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 panelX + 10,
                                 contentY + 92,
                                 framebufferWidth - margin - panelX - 20,
                                 m12_text(state, M12_TEXT_WINDOW_MODE),
                                 m12_settings_value_window_mode(state),
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  panelX + 10,
                  contentY + 126,
                  m12_text(state, M12_TEXT_SETTINGS_SAVED),
                  &g_textSmallMuted);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_SETTINGS_FOOTER));
}

static void m12_draw_message_view_modern(const M12_StartupMenuState* state,
                                         unsigned char* framebuffer,
                                         int framebufferWidth,
                                         int framebufferHeight) {
    const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, state->activatedIndex);
    const M12_GameCardArt* art = (state->activatedIndex >= 0 && state->activatedIndex < m12_entry_count())
                                     ? &state->cardArt[state->activatedIndex]
                                     : NULL;
    int margin = framebufferWidth / 30;
    int heroH = framebufferHeight / 3;
    int contentY;
    int boxX;
    int boxW;
    unsigned char messageColor = entry && !entry->available ? M12_COLOR_LIGHT_RED : M12_COLOR_LIGHT_GREEN;
    if (margin < 12) {
        margin = 12;
    }
    if (heroH < 82) {
        heroH = 82;
    }
    contentY = margin + heroH + 14;
    boxX = margin + 18;
    boxW = framebufferWidth - ((margin + 18) * 2);
    m12_draw_modern_background(state, framebuffer, framebufferWidth, framebufferHeight);
    m12_draw_modern_hero(state,
                         framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         margin,
                         margin,
                         framebufferWidth - (margin * 2),
                         heroH,
                         m12_text(state, M12_TEXT_STATUS));
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   boxX,
                   contentY,
                   boxW,
                   framebufferHeight - contentY - 28,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_card_preview(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          entry,
                          art,
                          boxX + 12,
                          contentY + 12,
                          132,
                          82,
                          entry && entry->kind == M12_MENU_ENTRY_SETTINGS
                              ? M12_COLOR_DARK_GRAY
                              : m12_game_card_fill(entry ? entry->gameId : NULL),
                          entry && entry->available ? M12_COLOR_YELLOW : M12_COLOR_LIGHT_RED);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  boxX + 160,
                  contentY + 18,
                  state->messageLine1,
                  &(M12_TextStyle){2, 1, messageColor, 1, 1, M12_COLOR_BLACK});
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  boxX + 160,
                  contentY + 52,
                  state->messageLine2,
                  &g_textSmallShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  boxX + 160,
                  contentY + 74,
                  state->messageLine3,
                  &g_textSmallMuted);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_text(state, M12_TEXT_MESSAGE_FOOTER));
}

void M12_StartupMenu_Draw(const M12_StartupMenuState* state,
                          unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight) {
    if (!state || !framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }
    memset(framebuffer,
           M12_COLOR_BLACK,
           (size_t)framebufferWidth * (size_t)framebufferHeight);
    if (m12_use_modern_layout(framebufferWidth, framebufferHeight)) {
        if (state->view == M12_MENU_VIEW_MESSAGE) {
            m12_draw_message_view_modern(state, framebuffer, framebufferWidth, framebufferHeight);
        } else if (state->view == M12_MENU_VIEW_SETTINGS) {
            m12_draw_settings_view_modern(state, framebuffer, framebufferWidth, framebufferHeight);
        } else {
            m12_draw_main_view_modern(state, framebuffer, framebufferWidth, framebufferHeight);
        }
        m12_apply_graphics_overlay(state, framebuffer, framebufferWidth, framebufferHeight);
        return;
    }
    m12_draw_background(state, framebuffer, framebufferWidth, framebufferHeight);
    if (state->view == M12_MENU_VIEW_MESSAGE) {
        m12_draw_message_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else if (state->view == M12_MENU_VIEW_SETTINGS) {
        m12_draw_settings_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else {
        m12_draw_main_view(state, framebuffer, framebufferWidth, framebufferHeight);
    }
    m12_apply_graphics_overlay(state, framebuffer, framebufferWidth, framebufferHeight);
}

int M12_StartupMenu_GetRenderPaletteLevel(const M12_StartupMenuState* state) {
    int graphicsIndex = state ? m12_clamp_index(state->settings.graphicsIndex,
                                                (int)(sizeof(g_graphicsLevels) / sizeof(g_graphicsLevels[0])))
                              : 0;
    switch (graphicsIndex) {
        case 0:
            return 0;
        case 1:
            return 1;
        default:
            return 2;
    }
}
