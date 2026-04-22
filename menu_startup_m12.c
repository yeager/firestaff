#include "menu_startup_m12.h"

#include "branding_logo_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static int m12_cycle_index(int value, int delta, int count);
static int m12_clamp_index(int value, int count);
static int m12_game_slot_from_id(const char* gameId);
static int m12_game_version_count(const M12_StartupMenuState* state, int gameIndex);
static void m12_normalize_game_version_index(M12_StartupMenuState* state, int gameIndex);
static const M12_AssetVersionStatus* m12_selected_version_status(const M12_StartupMenuState* state,
                                                                 int gameIndex);
static const char* m12_selected_version_label(const M12_StartupMenuState* state,
                                              int gameIndex,
                                              int shortLabel);
static void m12_init_game_options(M12_GameOptions* opts);
static void m12_cycle_game_opt_with_mode(M12_GameOptions* opts, int row, int delta, int presentationMode);
static void m12_enforce_mode_constraints(M12_GameOptions* opts, int presentationMode);

static const char* g_aspectRatios[] = {"ORIGINAL", "4:3", "16:9", "16:10"};
static const char* g_resolutions[] = {"320x200", "640x400", "800x600", "1024x768", "1280x960"};
static const char* g_patchModes[] = {"ORIGINAL", "PATCHED"};
static const char* g_languages[] = {"EN", "SV", "FR", "DE"};
static const char* g_languageNames[] = {"ENGLISH", "SVENSKA", "FRANCAIS", "DEUTSCH"};
static const char* g_cheatsToggle[] = {"OFF", "ON"};
static const char* g_speedLabels[] = {"SLOWER", "NORMAL", "FASTER"};

int M12_GameOptions_SpeedHotkeysEnabled(const M12_GameOptions* opts) {
    if (!opts) {
        return 0;
    }
    return opts->cheatsEnabled ? 1 : 0;
}

int M12_GameOptions_RowLockedByMode(int row, int presentationMode) {
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL) {
        /* V1 locks aspect ratio and resolution to original values */
        if (row == M12_GAME_OPT_ROW_ASPECT || row == M12_GAME_OPT_ROW_RESOLUTION) {
            return 1;
        }
    }
    return 0;
}

static void m12_init_game_options(M12_GameOptions* opts) {
    if (!opts) {
        return;
    }
    opts->versionIndex = 0;
    opts->usePatch = 0;
    opts->languageIndex = 0;
    opts->cheatsEnabled = 0;
    opts->gameSpeed = 1; /* index into g_speedLabels: NORMAL */
    opts->aspectRatio = 0;
    opts->resolution = 0;
}

static void m12_cycle_game_opt_with_mode(M12_GameOptions* opts, int row, int delta, int presentationMode) {
    if (!opts) {
        return;
    }
    /* Reject cycling on mode-locked rows */
    if (presentationMode >= 0 && M12_GameOptions_RowLockedByMode(row, presentationMode)) {
        return;
    }
    switch (row) {
        case M12_GAME_OPT_ROW_VERSION:
            break;
        case M12_GAME_OPT_ROW_PATCH:
            opts->usePatch = m12_cycle_index(opts->usePatch, delta, 2);
            break;
        case M12_GAME_OPT_ROW_LANGUAGE:
            opts->languageIndex = m12_cycle_index(opts->languageIndex,
                                                  delta,
                                                  (int)(sizeof(g_languages) / sizeof(g_languages[0])));
            break;
        case M12_GAME_OPT_ROW_CHEATS:
            opts->cheatsEnabled = m12_cycle_index(opts->cheatsEnabled, delta, 2);
            if (!opts->cheatsEnabled) {
                opts->gameSpeed = 1; /* reset to NORMAL when cheats disabled */
            }
            break;
        case M12_GAME_OPT_ROW_SPEED:
            if (opts->cheatsEnabled) {
                opts->gameSpeed = m12_cycle_index(opts->gameSpeed, delta, 3);
            }
            break;
        case M12_GAME_OPT_ROW_ASPECT:
            opts->aspectRatio = m12_cycle_index(opts->aspectRatio, delta, M12_ASPECT_COUNT);
            break;
        case M12_GAME_OPT_ROW_RESOLUTION:
            opts->resolution = m12_cycle_index(opts->resolution, delta, M12_RES_COUNT);
            break;
        default:
            break;
    }
}

static void m12_enforce_mode_constraints(M12_GameOptions* opts, int presentationMode) {
    if (!opts) {
        return;
    }
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL) {
        opts->aspectRatio = M12_ASPECT_ORIGINAL;
        opts->resolution = M12_RES_320x200;
    }
}

static void m12_clamp_game_options(M12_GameOptions* opts) {
    if (!opts) {
        return;
    }
    if (opts->versionIndex < 0) {
        opts->versionIndex = 0;
    }
    opts->usePatch = m12_clamp_index(opts->usePatch, 2);
    opts->languageIndex = m12_clamp_index(opts->languageIndex,
                                          (int)(sizeof(g_languages) / sizeof(g_languages[0])));
    opts->cheatsEnabled = m12_clamp_index(opts->cheatsEnabled, 2);
    if (opts->cheatsEnabled) {
        opts->gameSpeed = m12_clamp_index(opts->gameSpeed, 3);
    } else {
        opts->gameSpeed = 1;
    }
    opts->aspectRatio = m12_clamp_index(opts->aspectRatio, M12_ASPECT_COUNT);
    opts->resolution = m12_clamp_index(opts->resolution, M12_RES_COUNT);
}

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

static const char* g_presentationModes[] = {
    "V1 ORIGINAL",
    "V2 ENHANCED 2D",
    "V3 MODERN/3D"
};
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
    M12_TEXT_PRESENTATION_MODE,
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

enum {
    M12_RUNTIME_CATALOG_MAX_ENTRIES = 128,
    M12_RUNTIME_CATALOG_MSGID_CAPACITY = 128,
    M12_RUNTIME_CATALOG_MSGSTR_CAPACITY = 256
};

typedef struct {
    char msgid[M12_RUNTIME_CATALOG_MSGID_CAPACITY];
    char msgstr[M12_RUNTIME_CATALOG_MSGSTR_CAPACITY];
} M12_RuntimeCatalogEntry;

typedef struct {
    int attempted;
    int loaded;
    int entryCount;
    M12_RuntimeCatalogEntry entries[M12_RUNTIME_CATALOG_MAX_ENTRIES];
} M12_RuntimeCatalog;

static const char* const g_localeTextEnglish[M12_TEXT_COUNT] = {
    "FRONTEND PREVIEW",
    "SELECT A DESTINATION",
    "SETTINGS",
    "STATUS",
    "LAUNCHER DESTINATIONS",
    "DATA DIR",
    "UP/DOWN MOVE   ENTER OPEN   ESC EXIT",
    "PERSISTED OPTIONS",
    "LANGUAGE",
    "PRESENTATION MODE",
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
};

static M12_RuntimeCatalog g_runtimeCatalogs[4];

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

static void m12_copy_string(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!value) {
        value = "";
    }
    snprintf(out, outSize, "%s", value);
}

static int m12_starts_with(const char* text, const char* prefix) {
    size_t i;
    if (!text || !prefix) {
        return 0;
    }
    for (i = 0U; prefix[i] != '\0'; ++i) {
        if (text[i] != prefix[i]) {
            return 0;
        }
    }
    return 1;
}

static int m12_parse_po_quoted(char* out, size_t outSize, const char* text) {
    size_t src = 0U;
    size_t dst = 0U;
    if (!out || outSize == 0U || !text) {
        return 0;
    }
    while (text[src] != '\0' && isspace((unsigned char)text[src])) {
        ++src;
    }
    if (text[src] != '"') {
        return 0;
    }
    ++src;
    while (text[src] != '\0') {
        unsigned char ch = (unsigned char)text[src++];
        if (ch == '"') {
            out[dst] = '\0';
            return 1;
        }
        if (ch == '\\' && text[src] != '\0') {
            unsigned char esc = (unsigned char)text[src++];
            switch (esc) {
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;
                case '\\': ch = '\\'; break;
                case '"': ch = '"'; break;
                default: ch = esc; break;
            }
        }
        if (dst + 1U >= outSize) {
            break;
        }
        out[dst++] = (char)ch;
    }
    out[dst] = '\0';
    return 1;
}

static void m12_append_string(char* out, size_t outSize, const char* value) {
    size_t len;
    if (!out || outSize == 0U || !value) {
        return;
    }
    len = strlen(out);
    if (len >= outSize - 1U) {
        return;
    }
    snprintf(out + len, outSize - len, "%s", value);
}

static char m12_fold_utf8_char(const unsigned char* src, int* consumed) {
    if (!src || !consumed) {
        return '\0';
    }
    *consumed = 1;
    if (src[0] < 0x80U) {
        unsigned char ch = src[0];
        if (ch >= 'a' && ch <= 'z') {
            ch = (unsigned char)(ch - 'a' + 'A');
        }
        if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
            ch == ' ' || ch == '-' || ch == '.' || ch == ':' || ch == '/' || ch == '>') {
            return (char)ch;
        }
        if (ch == '\'' || ch == '`') {
            return ' ';
        }
        return ' ';
    }
    if (src[0] == 0xC3U) {
        *consumed = 2;
        switch (src[1]) {
            case 0x80U: case 0x82U: case 0x84U: case 0x85U:
            case 0x87U:
            case 0xA0U: case 0xA2U: case 0xA4U: case 0xA5U:
                return (src[1] == 0x87U) ? 'C' : 'A';
            case 0x89U: case 0x88U: case 0x8AU: case 0x8BU:
            case 0xA9U: case 0xA8U: case 0xAAU: case 0xABU:
                return 'E';
            case 0x8EU: case 0x8FU: case 0xAEU: case 0xAFU:
                return 'I';
            case 0x94U: case 0x96U: case 0xB4U: case 0xB6U:
                return 'O';
            case 0x99U: case 0x9BU: case 0x9CU: case 0xB9U: case 0xBBU: case 0xBCU:
                return 'U';
            default:
                return ' ';
        }
    }
    return ' ';
}

static void m12_sanitize_display_text(char* out, size_t outSize, const char* value) {
    size_t dst = 0U;
    size_t src = 0U;
    if (!out || outSize == 0U) {
        return;
    }
    out[0] = '\0';
    if (!value) {
        return;
    }
    while (value[src] != '\0' && dst + 1U < outSize) {
        int consumed = 1;
        char folded = m12_fold_utf8_char((const unsigned char*)&value[src], &consumed);
        if (folded != '\0') {
            out[dst++] = folded;
        }
        src += (size_t)(consumed > 0 ? consumed : 1);
    }
    out[dst] = '\0';
}

static int m12_store_catalog_entry(M12_RuntimeCatalog* catalog,
                                   const char* msgid,
                                   const char* msgstr) {
    char sanitized[M12_RUNTIME_CATALOG_MSGSTR_CAPACITY];
    if (!catalog || !msgid || !msgstr || msgid[0] == '\0' || msgstr[0] == '\0' ||
        catalog->entryCount >= M12_RUNTIME_CATALOG_MAX_ENTRIES) {
        return 0;
    }
    m12_sanitize_display_text(sanitized, sizeof(sanitized), msgstr);
    if (sanitized[0] == '\0') {
        return 0;
    }
    m12_copy_string(catalog->entries[catalog->entryCount].msgid,
                    sizeof(catalog->entries[catalog->entryCount].msgid),
                    msgid);
    m12_copy_string(catalog->entries[catalog->entryCount].msgstr,
                    sizeof(catalog->entries[catalog->entryCount].msgstr),
                    sanitized);
    catalog->entryCount += 1;
    return 1;
}

static int m12_resolve_catalog_path(int localeIndex, char* out, size_t outSize) {
    char path[FSP_PATH_MAX];
    if (!out || outSize == 0U || localeIndex < 0 || localeIndex >= 4) {
        return 0;
    }
    if (snprintf(path, sizeof(path), "po/startup-menu.%s.po", g_languages[localeIndex]) <= 0) {
        return 0;
    }
    path[sizeof(path) - 1U] = '\0';
    for (size_t i = 0U; path[i] != '\0'; ++i) {
        path[i] = (char)tolower((unsigned char)path[i]);
    }
    if (FSP_FileExists(path)) {
        m12_copy_string(out, outSize, path);
        return 1;
    }
    return 0;
}

static void m12_load_runtime_catalog(int localeIndex) {
    M12_RuntimeCatalog* catalog;
    char path[FSP_PATH_MAX];
    FILE* fp;
    char line[1024];
    char msgid[M12_RUNTIME_CATALOG_MSGID_CAPACITY] = "";
    char msgstr[M12_RUNTIME_CATALOG_MSGSTR_CAPACITY] = "";
    int inMsgid = 0;
    int inMsgstr = 0;
    if (localeIndex < 0 || localeIndex >= 4) {
        return;
    }
    catalog = &g_runtimeCatalogs[localeIndex];
    if (catalog->attempted) {
        return;
    }
    catalog->attempted = 1;
    catalog->loaded = 0;
    catalog->entryCount = 0;
    if (localeIndex == 0 || !m12_resolve_catalog_path(localeIndex, path, sizeof(path))) {
        return;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char parsed[M12_RUNTIME_CATALOG_MSGSTR_CAPACITY];
        char* trimmed = line;
        while (*trimmed != '\0' && isspace((unsigned char)*trimmed)) {
            ++trimmed;
        }
        if (m12_starts_with(trimmed, "msgid ")) {
            if (msgid[0] != '\0' && msgstr[0] != '\0') {
                m12_store_catalog_entry(catalog, msgid, msgstr);
            }
            msgid[0] = '\0';
            msgstr[0] = '\0';
            inMsgid = m12_parse_po_quoted(parsed, sizeof(parsed), trimmed + 5);
            inMsgstr = 0;
            if (inMsgid) {
                m12_append_string(msgid, sizeof(msgid), parsed);
            }
        } else if (m12_starts_with(trimmed, "msgstr ")) {
            msgstr[0] = '\0';
            inMsgstr = m12_parse_po_quoted(parsed, sizeof(parsed), trimmed + 6);
            inMsgid = 0;
            if (inMsgstr) {
                m12_append_string(msgstr, sizeof(msgstr), parsed);
            }
        } else if (trimmed[0] == '"') {
            if (m12_parse_po_quoted(parsed, sizeof(parsed), trimmed)) {
                if (inMsgid) {
                    m12_append_string(msgid, sizeof(msgid), parsed);
                } else if (inMsgstr) {
                    m12_append_string(msgstr, sizeof(msgstr), parsed);
                }
            }
        }
    }
    if (msgid[0] != '\0' && msgstr[0] != '\0') {
        m12_store_catalog_entry(catalog, msgid, msgstr);
    }
    fclose(fp);
    catalog->loaded = catalog->entryCount > 0 ? 1 : 0;
}

static const char* m12_translate_for_locale(int localeIndex, const char* english) {
    int i;
    M12_RuntimeCatalog* catalog;
    if (!english || english[0] == '\0') {
        return "";
    }
    if (localeIndex <= 0 || localeIndex >= 4) {
        return english;
    }
    m12_load_runtime_catalog(localeIndex);
    catalog = &g_runtimeCatalogs[localeIndex];
    if (!catalog->loaded) {
        return english;
    }
    for (i = 0; i < catalog->entryCount; ++i) {
        if (strcmp(catalog->entries[i].msgid, english) == 0) {
            return catalog->entries[i].msgstr;
        }
    }
    return english;
}

static int m12_locale_index(const M12_StartupMenuState* state) {
    return state ? m12_clamp_index(state->settings.languageIndex, 4) : 0;
}

static const char* m12_text(const M12_StartupMenuState* state, M12_TextId id) {
    int locale = m12_locale_index(state);
    if (id < 0 || id >= M12_TEXT_COUNT) {
        return "";
    }
    return m12_translate_for_locale(locale, g_localeTextEnglish[id]);
}

static const char* m12_tr(const M12_StartupMenuState* state, const char* english) {
    return m12_translate_for_locale(m12_locale_index(state), english);
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

static void m12_save_config(const M12_StartupMenuState* state) {
    M12_Config config;
    int gi;
    if (!state) {
        return;
    }
    M12_Config_SetDefaults(&config);
    config.languageIndex = state->settings.languageIndex;
    config.languageExplicit = state->languageExplicit ? 1 : 0;
    config.graphicsIndex = state->settings.graphicsIndex;
    config.windowModeIndex = state->settings.windowModeIndex;
    for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
        M12_GameOptions opts = state->gameOptions[gi];
        m12_clamp_game_options(&opts);
        config.gameVersionIndex[gi] = opts.versionIndex;
        config.gameUsePatch[gi] = opts.usePatch;
        config.gameLanguageIndex[gi] = opts.languageIndex;
        config.gameCheatsEnabled[gi] = opts.cheatsEnabled;
        config.gameSpeed[gi] = opts.gameSpeed;
        config.gameAspectRatio[gi] = opts.aspectRatio;
        config.gameResolution[gi] = opts.resolution;
    }
    snprintf(config.dataDir, sizeof(config.dataDir), "%s", M12_AssetStatus_GetDataDir(&state->assetStatus));
    M12_Config_Save(&config);
}

static void m12_apply_loaded_config(M12_StartupMenuState* state, const char* dataDirOverride) {
    M12_Config config;
    int gi;
    if (!state) {
        return;
    }
    M12_Config_Load(&config, dataDirOverride);
    state->settings.languageIndex = m12_clamp_index(config.languageIndex,
                                                    (int)(sizeof(g_languages) / sizeof(g_languages[0])));
    state->languageExplicit = config.languageExplicit ? 1 : 0;
    state->settings.graphicsIndex = m12_clamp_index(config.graphicsIndex,
                                                    (int)(sizeof(g_presentationModes) /
                                                          sizeof(g_presentationModes[0])));
    state->settings.windowModeIndex = m12_clamp_index(config.windowModeIndex,
                                                       (int)(sizeof(g_windowModes) / sizeof(g_windowModes[0])));
    for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
        state->gameOptions[gi].versionIndex = config.gameVersionIndex[gi];
        state->gameOptions[gi].usePatch = config.gameUsePatch[gi];
        state->gameOptions[gi].languageIndex = config.gameLanguageIndex[gi];
        state->gameOptions[gi].cheatsEnabled = config.gameCheatsEnabled[gi];
        state->gameOptions[gi].gameSpeed = config.gameSpeed[gi];
        state->gameOptions[gi].aspectRatio = config.gameAspectRatio[gi];
        state->gameOptions[gi].resolution = config.gameResolution[gi];
        m12_clamp_game_options(&state->gameOptions[gi]);
    }
    M12_AssetStatus_Scan(&state->assetStatus, config.dataDir);
    for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
        m12_normalize_game_version_index(state, gi);
    }
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
    {
        int gi;
        for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
            m12_init_game_options(&state->gameOptions[gi]);
        }
    }
    m12_apply_loaded_config(state, dataDir);
    m12_sync_entries_from_assets(state);
    m12_sync_card_art(state);
    M12_CreatureArt_Init(&state->creatureArt,
                         M12_AssetStatus_GetDataDir(&state->assetStatus),
                         (unsigned int)time(NULL));
    state->selectedIndex = 0;
    state->settingsSelectedIndex = 0;
    state->gameOptSelectedRow = 0;
    state->activatedIndex = -1;
    state->view = M12_MENU_VIEW_MAIN;
    state->messageLine1 = "";
    state->messageLine2 = "";
    state->messageLine3 = "";
    state->frameTick = 0;
    state->hoverX = -1;
    state->hoverY = -1;
}

static const char* m12_settings_value_language(const M12_StartupMenuState* state) {
    return g_languages[state->settings.languageIndex];
}

static const char* m12_game_value_language_name(const M12_StartupMenuState* state,
                                                const M12_GameOptions* opts) {
    int index = opts ? m12_clamp_index(opts->languageIndex,
                                       (int)(sizeof(g_languageNames) / sizeof(g_languageNames[0])))
                     : 0;
    return m12_tr(state, g_languageNames[index]);
}

static const char* m12_settings_value_graphics(const M12_StartupMenuState* state) {
    return m12_tr(state, g_presentationModes[state->settings.graphicsIndex]);
}

static const char* m12_settings_value_window_mode(const M12_StartupMenuState* state) {
    return m12_tr(state, g_windowModes[state->settings.windowModeIndex]);
}

static int m12_game_slot_from_id(const char* gameId) {
    if (!gameId) {
        return -1;
    }
    if (strcmp(gameId, "dm1") == 0) {
        return 0;
    }
    if (strcmp(gameId, "csb") == 0) {
        return 1;
    }
    if (strcmp(gameId, "dm2") == 0) {
        return 2;
    }
    return -1;
}

static int m12_game_version_count(const M12_StartupMenuState* state, int gameIndex) {
    static const char* const gameIds[M12_CONFIG_GAME_COUNT] = {"dm1", "csb", "dm2"};
    if (!state || gameIndex < 0 || gameIndex >= M12_CONFIG_GAME_COUNT) {
        return 1;
    }
    {
        size_t count = M12_AssetStatus_GetVersionCount(gameIds[gameIndex]);
        return count > 0U ? (int)count : 1;
    }
}

static void m12_normalize_game_version_index(M12_StartupMenuState* state, int gameIndex) {
    int count;
    if (!state || gameIndex < 0 || gameIndex >= M12_CONFIG_GAME_COUNT) {
        return;
    }
    count = m12_game_version_count(state, gameIndex);
    state->gameOptions[gameIndex].versionIndex = m12_clamp_index(state->gameOptions[gameIndex].versionIndex,
                                                                 count);
}

static const M12_AssetVersionStatus* m12_selected_version_status(const M12_StartupMenuState* state,
                                                                 int gameIndex) {
    static const char* const gameIds[M12_CONFIG_GAME_COUNT] = {"dm1", "csb", "dm2"};
    if (!state || gameIndex < 0 || gameIndex >= M12_CONFIG_GAME_COUNT) {
        return NULL;
    }
    return M12_AssetStatus_GetVersion(&state->assetStatus,
                                      gameIds[gameIndex],
                                      (size_t)state->gameOptions[gameIndex].versionIndex);
}

static const char* m12_selected_version_label(const M12_StartupMenuState* state,
                                              int gameIndex,
                                              int shortLabel) {
    const M12_AssetVersionStatus* version = m12_selected_version_status(state, gameIndex);
    if (!version) {
        return "UNKNOWN";
    }
    return shortLabel ? version->shortLabel : version->label;
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
    if (entry->available) {
        int gi = m12_clamp_index(state->selectedIndex, M12_CONFIG_GAME_COUNT);
        int pmode = m12_clamp_index(state->settings.graphicsIndex, M12_PRESENTATION_MODE_COUNT);
        m12_normalize_game_version_index(state, gi);
        m12_enforce_mode_constraints(&state->gameOptions[gi], pmode);
        state->view = M12_MENU_VIEW_GAME_OPTIONS;
        state->gameOptSelectedRow = 0;
        return;
    }
    state->view = M12_MENU_VIEW_MESSAGE;
    if (!M12_AssetStatus_GameHasCompleteHashSet(entry->gameId)) {
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
            state->languageExplicit = 1;
            break;
        case M12_SETTINGS_ROW_GRAPHICS:
                state->settings.graphicsIndex = m12_cycle_index(
                    state->settings.graphicsIndex,
                    delta,
                    (int)(sizeof(g_presentationModes) /
                          sizeof(g_presentationModes[0])));
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
    m12_save_config(state);
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

    if (state->view == M12_MENU_VIEW_GAME_OPTIONS) {
        int gi = m12_clamp_index(state->activatedIndex, M12_CONFIG_GAME_COUNT);
        int pmode = m12_clamp_index(state->settings.graphicsIndex, M12_PRESENTATION_MODE_COUNT);
        int versionCount = m12_game_version_count(state, gi);
        switch (input) {
            case M12_MENU_INPUT_UP:
                state->gameOptSelectedRow = m12_cycle_index(
                    state->gameOptSelectedRow,
                    -1,
                    M12_GAME_OPT_ROW_COUNT + 1);
                break;
            case M12_MENU_INPUT_DOWN:
                state->gameOptSelectedRow = m12_cycle_index(
                    state->gameOptSelectedRow,
                    1,
                    M12_GAME_OPT_ROW_COUNT + 1);
                break;
            case M12_MENU_INPUT_LEFT:
                if (state->gameOptSelectedRow < M12_GAME_OPT_ROW_COUNT) {
                    if (state->gameOptSelectedRow == M12_GAME_OPT_ROW_VERSION) {
                        state->gameOptions[gi].versionIndex = m12_cycle_index(state->gameOptions[gi].versionIndex,
                                                                              -1,
                                                                              versionCount);
                    } else {
                        m12_cycle_game_opt_with_mode(&state->gameOptions[gi],
                                           state->gameOptSelectedRow, -1, pmode);
                    }
                    m12_save_config(state);
                }
                break;
            case M12_MENU_INPUT_RIGHT:
                if (state->gameOptSelectedRow < M12_GAME_OPT_ROW_COUNT) {
                    if (state->gameOptSelectedRow == M12_GAME_OPT_ROW_VERSION) {
                        state->gameOptions[gi].versionIndex = m12_cycle_index(state->gameOptions[gi].versionIndex,
                                                                              1,
                                                                              versionCount);
                    } else {
                        m12_cycle_game_opt_with_mode(&state->gameOptions[gi],
                                           state->gameOptSelectedRow, 1, pmode);
                    }
                    m12_save_config(state);
                }
                break;
            case M12_MENU_INPUT_ACCEPT:
                if (state->gameOptSelectedRow >= M12_GAME_OPT_ROW_COUNT) {
                    /* Launch row — V3 blocks launch with coming-soon message */
                    if (pmode == M12_PRESENTATION_V3_MODERN_3D) {
                        state->view = M12_MENU_VIEW_MESSAGE;
                        state->messageLine1 = m12_tr(state, "V3 MODERN/3D");
                        state->messageLine2 = m12_tr(state, "COMING SOON");
                        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
                    } else if (!m12_selected_version_status(state, gi) ||
                               !m12_selected_version_status(state, gi)->matched) {
                        state->view = M12_MENU_VIEW_MESSAGE;
                        state->messageLine1 = m12_tr(state, "SELECTED VERSION NOT FOUND");
                        state->messageLine2 = m12_selected_version_label(state, gi, 0);
                        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
                    } else {
                        state->view = M12_MENU_VIEW_MESSAGE;
                        state->messageLine1 = m12_text(state, M12_TEXT_READY_TO_LAUNCH);
                        state->messageLine2 = state->entries[state->activatedIndex].title;
                        state->messageLine3 = m12_text(state, M12_TEXT_ESC_RETURNS_TO_MENU);
                    }
                } else {
                    if (state->gameOptSelectedRow == M12_GAME_OPT_ROW_VERSION) {
                        state->gameOptions[gi].versionIndex = m12_cycle_index(state->gameOptions[gi].versionIndex,
                                                                              1,
                                                                              versionCount);
                    } else {
                        m12_cycle_game_opt_with_mode(&state->gameOptions[gi],
                                           state->gameOptSelectedRow, 1, pmode);
                    }
                    m12_save_config(state);
                }
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

static void m12_draw_language_flag(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int x,
                                   int y,
                                   int languageIndex) {
    int idx = m12_clamp_index(languageIndex,
                              (int)(sizeof(g_languages) / sizeof(g_languages[0])));
    m12_draw_frame(framebuffer, framebufferWidth, framebufferHeight,
                   x, y, 18, 12, M12_COLOR_WHITE, M12_COLOR_BLACK);
    switch (idx) {
        case 1: /* Sweden */
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 1, 16, 10, M12_COLOR_LIGHT_BLUE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 6, y + 1, 2, 10, M12_COLOR_YELLOW);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 4, 16, 2, M12_COLOR_YELLOW);
            break;
        case 2: /* France */
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 1, 5, 10, M12_COLOR_LIGHT_BLUE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 6, y + 1, 6, 10, M12_COLOR_WHITE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 12, y + 1, 5, 10, M12_COLOR_LIGHT_RED);
            break;
        case 3: /* Germany */
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 1, 16, 3, M12_COLOR_BLACK);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 4, 16, 3, M12_COLOR_LIGHT_RED);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 7, 16, 4, M12_COLOR_YELLOW);
            break;
        case 0:
        default: /* UK-ish English marker, bounded and recognisable */
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 1, 16, 10, M12_COLOR_LIGHT_BLUE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 5, 16, 2, M12_COLOR_WHITE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 8, y + 1, 2, 10, M12_COLOR_WHITE);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 1, y + 5, 16, 1, M12_COLOR_LIGHT_RED);
            m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 8, y + 1, 1, 10, M12_COLOR_LIGHT_RED);
            break;
    }
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

static const char* m12_game_card_line2(const M12_StartupMenuState* state,
                                       const M12_MenuEntry* entry,
                                       int gameIndex) {
    if (!entry || !state) {
        return "PERSISTED";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "PERSISTED";
    }
    return m12_selected_version_label(state, gameIndex, 1);
}

static const char* m12_game_card_line3(const M12_StartupMenuState* state,
                                       const M12_MenuEntry* entry,
                                       int gameIndex) {
    const M12_AssetVersionStatus* version;
    if (!entry || !state) {
        return "SETTINGS";
    }
    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        return "SETTINGS";
    }
    version = m12_selected_version_status(state, gameIndex);
    if (version && version->matched) {
        return "READY TO LAUNCH";
    }
    return "MISSING OR MISMATCHED";
}

static void m12_draw_status_icon(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int matched) {
    unsigned char color = matched ? M12_COLOR_LIGHT_GREEN : M12_COLOR_LIGHT_RED;
    if (matched) {
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 1, y + 4, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 7, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 7, y + 2, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 10, y - 1, 3, 3, color);
    } else {
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 1, y + 1, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 4, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 7, y + 7, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 7, y + 1, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 4, 3, 3, color);
        m12_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x + 1, y + 7, 3, 3, color);
    }
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
             "%lu VERSION%s / %lu FILE%s",
             hashCount,
             hashCount == 1UL ? "" : "S",
             requiredFiles,
             requiredFiles == 1UL ? "" : "S");
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
    int gameIndex = m12_game_slot_from_id(entry ? entry->gameId : NULL);
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
                           m12_game_card_line2(state, entry, gameIndex),
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
                           m12_game_card_line3(state, entry, gameIndex),
                           entry && gameIndex >= 0 && m12_selected_version_status(state, gameIndex) &&
                               m12_selected_version_status(state, gameIndex)->matched
                               ? &g_textSmallAccent
                               : &g_textSmallMuted);
    if (gameIndex >= 0) {
        const M12_AssetVersionStatus* version = m12_selected_version_status(state, gameIndex);
        if (version) {
            m12_draw_status_icon(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 88,
                                 146,
                                 version->matched);
        }
    }
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
                                  int selected,
                                  int languageIndex,
                                  int showFlag) {
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
    if (showFlag) {
        m12_draw_language_flag(framebuffer,
                               framebufferWidth,
                               framebufferHeight,
                               212,
                               y + 6,
                               languageIndex);
    }
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
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_LANGUAGE,
                          state->settings.languageIndex,
                          1);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          112,
                          m12_text(state, M12_TEXT_PRESENTATION_MODE),
                          m12_settings_value_graphics(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_GRAPHICS,
                          0,
                          0);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          140,
                          m12_text(state, M12_TEXT_WINDOW_MODE),
                          m12_settings_value_window_mode(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE,
                          0,
                          0);
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

static void m12_draw_modern_sidebar(const M12_StartupMenuState* state,
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
    int gameIndex = m12_game_slot_from_id(entry ? entry->gameId : NULL);
    char hashSummary[64];
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
                  entry ? m12_game_card_line2(state, entry, gameIndex) : "NO PROFILE",
                  &g_textSmallShadow);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  y + h / 2 + 31,
                  entry ? m12_game_card_line3(state, entry, gameIndex) : "",
                  entry && gameIndex >= 0 && m12_selected_version_status(state, gameIndex) &&
                      m12_selected_version_status(state, gameIndex)->matched
                      ? &g_textSmallAccent
                      : &g_textSmallMuted);
    if (gameIndex >= 0) {
        const M12_AssetVersionStatus* version = m12_selected_version_status(state, gameIndex);
        if (version) {
            m12_draw_status_icon(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 x + w - 24,
                                 y + h / 2 + 20,
                                 version->matched);
        }
    }
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
                                         int selected,
                                         int languageIndex,
                                         int showFlag) {
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
    if (showFlag) {
        m12_draw_language_flag(framebuffer,
                               framebufferWidth,
                               framebufferHeight,
                               x + w - 116,
                               y + 8,
                               languageIndex);
    }
}

static void m12_draw_game_card(const M12_StartupMenuState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               int x,
                               int y,
                               int w,
                               int h,
                               int entryIndex,
                               int selected) {
    const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, entryIndex);
    const M12_GameCardArt* art = (entryIndex >= 0 && entryIndex < m12_entry_count())
                                     ? &state->cardArt[entryIndex]
                                     : NULL;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char accent = entry && entry->available ? M12_COLOR_GREEN : M12_COLOR_LIGHT_RED;
    unsigned char gameFill = entry ? m12_game_card_fill(entry->gameId) : M12_COLOR_DARK_GRAY;
    int artH = h * 55 / 100;
    if (artH < 30) {
        artH = 30;
    }
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   h,
                   border,
                   fill);
    if (selected) {
        m12_fill_rect(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      x + 4,
                      y + 4,
                      w - 8,
                      3,
                      M12_COLOR_YELLOW);
    }
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + 12,
                  w - 16,
                  artH,
                  gameFill);
    m12_draw_card_preview(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          entry,
                          art,
                          x + 10,
                          y + 14,
                          w - 20,
                          artH - 4,
                          gameFill,
                          accent);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + artH + 18,
                  entry ? entry->title : "UNKNOWN",
                  selected ? &g_textMediumShadow : &g_textSmallShadow);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x + 8,
                                y + artH + 36,
                                64,
                                m12_entry_status_text(entry),
                                m12_entry_status_fill(entry, selected),
                                M12_COLOR_BLACK,
                                m12_entry_status_text_color(entry, selected));
    if (entry && entry->kind == M12_MENU_ENTRY_GAME && entryIndex >= 0 && entryIndex < M12_CONFIG_GAME_COUNT) {
        m12_draw_language_flag(framebuffer,
                               framebufferWidth,
                               framebufferHeight,
                               x + w - 28,
                               y + 10,
                               state->gameOptions[entryIndex].languageIndex);
    }
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + h - 14,
                  m12_entry_detail_line(entry),
                  &g_textSmallMuted);
}

static void m12_draw_info_sidebar(const M12_StartupMenuState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int x,
                                   int y,
                                   int w,
                                   int h,
                                   int settingsSelected) {
    const M12_GraphicsTheme* theme = m12_theme(state);
    int logoScale = framebufferWidth >= 640 ? 2 : 1;
    int logoW = M12_BRANDING_LOGO_WIDTH * logoScale;
    int logoH = M12_BRANDING_LOGO_HEIGHT * logoScale;
    int logoX = x + (w - logoW) / 2;
    int logoY = y + 12;
    int textY = logoY + logoH + 8;
    int btnY;
    unsigned char settingsBorder = settingsSelected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char settingsFill = settingsSelected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x,
                   y,
                   w,
                   h,
                   theme->titleBorder,
                   M12_COLOR_BLACK);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 4,
                  y + 4,
                  w - 8,
                  4,
                  theme->glowColor);
    m12_draw_branding_logo(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           logoX,
                           logoY,
                           logoScale);
    {
        const char* eyebrow = m12_text(state, M12_TEXT_EYEBROW);
        const char* subtitle = m12_text(state, M12_TEXT_SELECT_DESTINATION);
        int ew = m12_measure_text(eyebrow, g_textSmallAccent.scale, g_textSmallAccent.tracking);
        int sw = m12_measure_text(subtitle, g_textSmallMuted.scale, g_textSmallMuted.tracking);
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      x + (w - ew) / 2,
                      textY,
                      eyebrow,
                      &g_textSmallAccent);
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      x + (w - sw) / 2,
                      textY + 12,
                      subtitle,
                      &g_textSmallMuted);
    }
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 10,
                  textY + 26,
                  w - 20,
                  1,
                  M12_COLOR_DARK_GRAY);
    btnY = textY + 34;
    /* Museum of Lore */
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x + 8,
                   btnY,
                   w - 16,
                   20,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    {
        int mw = m12_measure_text("MUSEUM OF LORE", g_textSmallMuted.scale, g_textSmallMuted.tracking);
        m12_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x + 8 + (w - 16 - mw) / 2, btnY + 6,
                      "MUSEUM OF LORE", &g_textSmallMuted);
    }
    btnY += 24;
    /* Credits */
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x + 8,
                   btnY,
                   w - 16,
                   20,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    {
        int cw2 = m12_measure_text("CREDITS", g_textSmallMuted.scale, g_textSmallMuted.tracking);
        m12_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x + 8 + (w - 16 - cw2) / 2, btnY + 6,
                      "CREDITS", &g_textSmallMuted);
    }
    btnY += 24;
    /* Settings */
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   x + 8,
                   btnY,
                   w - 16,
                   24,
                   settingsBorder,
                   settingsFill);
    {
        const M12_TextStyle* sStyle = settingsSelected ? &g_textSmallShadow : &g_textSmallMuted;
        int sw2 = m12_measure_text("SETTINGS", sStyle->scale, sStyle->tracking);
        m12_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x + 8 + (w - 16 - sw2) / 2, btnY + 8,
                      "SETTINGS", sStyle);
    }
    /* Creature art showcase above data dir */
    if (M12_CreatureArt_HasSelection(&state->creatureArt)) {
        int creatureY = btnY + 30;
        int creatureH = y + h - 48 - creatureY;
        if (creatureH > 20) {
            M12_CreatureArt_Draw(&state->creatureArt,
                                framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x + 8,
                                creatureY,
                                w - 16,
                                creatureH);
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          x + 8,
                          creatureY + creatureH + 2,
                          M12_CreatureArt_GetSelectedName(&state->creatureArt),
                          &g_textSmallMuted);
        }
    }
    /* Data dir at bottom */
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 6,
                  y + h - 24,
                  m12_text(state, M12_TEXT_DATA_DIR),
                  &g_textSmallMuted);
    m12_draw_language_flag(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           x + 6,
                           y + h - 40,
                           state->settings.languageIndex);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 28,
                  y + h - 37,
                  "LAUNCHER",
                  &g_textSmallMuted);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 6,
                  y + h - 12,
                  M12_AssetStatus_GetDataDir(&state->assetStatus),
                  &g_textSmallMuted);
}

static void m12_draw_main_view_modern(const M12_StartupMenuState* state,
                                      unsigned char* framebuffer,
                                      int framebufferWidth,
                                      int framebufferHeight) {
    int margin = framebufferWidth / 30;
    int sidebarW;
    int cardsX;
    int cardsW;
    int cardW;
    int cardGap;
    int cardH;
    int i;
    int settingsSelected;
    if (margin < 12) {
        margin = 12;
    }
    sidebarW = (framebufferWidth * 24) / 100;
    if (sidebarW < 100) {
        sidebarW = 100;
    }
    cardsX = margin + sidebarW + 10;
    cardsW = framebufferWidth - margin - cardsX;
    cardGap = 8;
    cardW = (cardsW - (cardGap * 2)) / 3;
    cardH = framebufferHeight - (margin * 2) - 18;
    if (cardH < 100) {
        cardH = 100;
    }
    settingsSelected = (state->selectedIndex == 3);

    m12_draw_modern_background(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Left sidebar: logo, about, museum, credits, settings */
    m12_draw_info_sidebar(state,
                          framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          margin,
                          margin,
                          sidebarW,
                          cardH,
                          settingsSelected);

    /* Three game cards: DM1, CSB, DM2 */
    for (i = 0; i < 3; ++i) {
        m12_draw_game_card(state,
                           framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           cardsX + i * (cardW + cardGap),
                           margin,
                           cardW,
                           cardH,
                           i,
                           i == state->selectedIndex);
    }

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
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_LANGUAGE,
                                 state->settings.languageIndex,
                                 1);
    m12_draw_modern_settings_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 panelX + 10,
                                 contentY + 60,
                                 framebufferWidth - margin - panelX - 20,
                                 m12_text(state, M12_TEXT_PRESENTATION_MODE),
                                 m12_settings_value_graphics(state),
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_GRAPHICS,
                                 0,
                                 0);
    m12_draw_modern_settings_row(framebuffer,
                                 framebufferWidth,
                                 framebufferHeight,
                                 panelX + 10,
                                 contentY + 92,
                                 framebufferWidth - margin - panelX - 20,
                                 m12_text(state, M12_TEXT_WINDOW_MODE),
                                 m12_settings_value_window_mode(state),
                                 state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE,
                                 0,
                                 0);
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

static void m12_draw_game_opt_row(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  int x,
                                  int y,
                                  int w,
                                  const char* label,
                                  const char* value,
                                  int selected,
                                  int dimmed,
                                  int languageIndex,
                                  int showFlag) {
    unsigned char fill = selected ? M12_COLOR_NAVY : M12_COLOR_BLACK;
    unsigned char border = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueFill = dimmed ? M12_COLOR_DARK_GRAY
                                     : (selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY);
    unsigned char valueTextC = dimmed ? M12_COLOR_DARK_GRAY
                                      : (selected ? M12_COLOR_BLACK : M12_COLOR_WHITE);
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
                  dimmed ? &g_textSmallMuted : &g_textSmallShadow);
    m12_draw_modern_status_pill(framebuffer,
                                framebufferWidth,
                                framebufferHeight,
                                x + w - 92,
                                y + 7,
                                82,
                                value,
                                valueFill,
                                border,
                                valueTextC);
    if (showFlag) {
        m12_draw_language_flag(framebuffer,
                               framebufferWidth,
                               framebufferHeight,
                               x + w - 116,
                               y + 8,
                               languageIndex);
    }
}

static void m12_draw_game_options_view_modern(const M12_StartupMenuState* state,
                                              unsigned char* framebuffer,
                                              int framebufferWidth,
                                              int framebufferHeight) {
    const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, state->activatedIndex);
    const M12_GameCardArt* art = (state->activatedIndex >= 0 && state->activatedIndex < m12_entry_count())
                                     ? &state->cardArt[state->activatedIndex]
                                     : NULL;
    int gi = m12_clamp_index(state->activatedIndex, 3);
    const M12_GameOptions* opts = &state->gameOptions[gi];
    int margin = framebufferWidth / 30;
    int heroH = framebufferHeight / 4;
    int contentY;
    int leftW;
    int panelX;
    int panelW;
    int rowY;
    int speedDimmed;
    int pmode;
    int aspectLocked;
    int resLocked;
    const M12_AssetVersionStatus* version;
    unsigned char gameFill;
    if (margin < 12) {
        margin = 12;
    }
    if (heroH < 64) {
        heroH = 64;
    }
    contentY = margin + heroH + 10;
    leftW = (framebufferWidth * 35) / 100;
    panelX = margin + leftW + 12;
    panelW = framebufferWidth - margin - panelX;
    gameFill = entry ? m12_game_card_fill(entry->gameId) : M12_COLOR_DARK_GRAY;
    speedDimmed = !opts->cheatsEnabled;
    pmode = m12_clamp_index(state->settings.graphicsIndex, M12_PRESENTATION_MODE_COUNT);
    aspectLocked = M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_ASPECT, pmode);
    resLocked = M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION, pmode);
    version = m12_selected_version_status(state, gi);

    m12_draw_modern_background(state, framebuffer, framebufferWidth, framebufferHeight);
    m12_draw_modern_hero(state,
                         framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         margin,
                         margin,
                         framebufferWidth - (margin * 2),
                         heroH,
                         entry ? entry->title : "GAME OPTIONS");

    /* Left: card art preview */
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   margin,
                   contentY,
                   leftW,
                   framebufferHeight - contentY - 28,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_card_preview(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          entry,
                          art,
                          margin + 10,
                          contentY + 10,
                          leftW - 20,
                          (framebufferHeight - contentY - 28) / 2,
                          gameFill,
                          M12_COLOR_YELLOW);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  margin + 10,
                  contentY + (framebufferHeight - contentY - 28) / 2 + 16,
                  entry ? entry->title : "UNKNOWN",
                  &g_textSmallShadow);
    if (entry && version && version->matched) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      margin + 10,
                      contentY + (framebufferHeight - contentY - 28) / 2 + 30,
                      "VERIFIED DATA READY",
                      &g_textSmallAccent);
    }

    /* Right: options panel */
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   panelX,
                   contentY,
                   panelW,
                   framebufferHeight - contentY - 28,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  panelX + 10,
                  contentY + 10,
                  m12_tr(state, "GAME OPTIONS"),
                  &g_textSmallAccent);
    /* Mode badge */
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  panelX + 10,
                  contentY + 20,
                  m12_tr(state, g_presentationModes[pmode]),
                  pmode == M12_PRESENTATION_V3_MODERN_3D ? &g_textSmallMuted : &g_textSmallShadow);
    rowY = contentY + 38;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "VERSION"),
                          version ? m12_tr(state, version->shortLabel) : "UNKNOWN",
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_VERSION,
                          0,
                          0,
                          0);
    if (version) {
        m12_draw_status_icon(framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             panelX + panelW - 132,
                             rowY + 8,
                             version->matched);
    }
    rowY += 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "PATCH"),
                          m12_tr(state, g_patchModes[opts->usePatch]),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_PATCH,
                          0,
                          0,
                          0);
    rowY += 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "LANGUAGE"),
                          m12_game_value_language_name(state, opts),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_LANGUAGE,
                          0,
                          opts->languageIndex,
                          1);
    rowY += 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "CHEATS"),
                          m12_tr(state, g_cheatsToggle[opts->cheatsEnabled]),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_CHEATS,
                          0,
                          0,
                          0);
    rowY += 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "SPEED"),
                          m12_tr(state, g_speedLabels[m12_clamp_index(opts->gameSpeed, 3)]),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_SPEED,
                          speedDimmed,
                          0,
                          0);
    if (speedDimmed) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      panelX + 10,
                      rowY + 28,
                      m12_tr(state, "ENABLE CHEATS TO UNLOCK SPEED / HOTKEYS"),
                      &g_textSmallMuted);
    }
    rowY += speedDimmed ? 42 : 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "ASPECT RATIO"),
                          m12_tr(state, g_aspectRatios[m12_clamp_index(opts->aspectRatio, M12_ASPECT_COUNT)]),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_ASPECT,
                          aspectLocked,
                          0,
                          0);
    if (aspectLocked) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      panelX + 10,
                      rowY + 28,
                      m12_tr(state, "LOCKED BY V1 ORIGINAL MODE"),
                      &g_textSmallMuted);
    }
    rowY += aspectLocked ? 42 : 32;
    m12_draw_game_opt_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10,
                          rowY,
                          panelW - 20,
                          m12_tr(state, "RESOLUTION"),
                          m12_tr(state, g_resolutions[m12_clamp_index(opts->resolution, M12_RES_COUNT)]),
                          state->gameOptSelectedRow == M12_GAME_OPT_ROW_RESOLUTION,
                          resLocked,
                          0,
                          0);
    if (resLocked) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      panelX + 10,
                      rowY + 28,
                      m12_tr(state, "LOCKED BY V1 ORIGINAL MODE"),
                      &g_textSmallMuted);
    }
    rowY += resLocked ? 42 : 36;
    {
        int launchSelected = state->gameOptSelectedRow >= M12_GAME_OPT_ROW_COUNT;
        unsigned char launchFill = launchSelected ? M12_COLOR_GREEN : M12_COLOR_DARK_GRAY;
        unsigned char launchBorder = launchSelected ? M12_COLOR_YELLOW : M12_COLOR_GREEN;
        m12_draw_frame(framebuffer,
                       framebufferWidth,
                       framebufferHeight,
                       panelX + 10,
                       rowY,
                       panelW - 20,
                       28,
                       launchBorder,
                       launchFill);
        {
            const char* launchLabel = (pmode == M12_PRESENTATION_V3_MODERN_3D)
                                          ? m12_tr(state, "> COMING SOON")
                                          : m12_tr(state, "> LAUNCH");
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          panelX + 10 + (panelW - 20) / 2 - 24,
                          rowY + 10,
                          launchLabel,
                          launchSelected ? &g_textSmallShadow : &g_textSmallMuted);
        }
    }
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    m12_tr(state, "ESC: BACK  ARROWS: NAVIGATE  ENTER: SELECT"));
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
        } else if (state->view == M12_MENU_VIEW_GAME_OPTIONS) {
            m12_draw_game_options_view_modern(state, framebuffer, framebufferWidth, framebufferHeight);
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
    } else if (state->view == M12_MENU_VIEW_GAME_OPTIONS) {
        m12_draw_game_options_view_modern(state, framebuffer, framebufferWidth, framebufferHeight);
    } else {
        m12_draw_main_view(state, framebuffer, framebufferWidth, framebufferHeight);
    }
    m12_apply_graphics_overlay(state, framebuffer, framebufferWidth, framebufferHeight);
}

int M12_StartupMenu_GetRenderPaletteLevel(const M12_StartupMenuState* state) {
    int graphicsIndex = state ? m12_clamp_index(state->settings.graphicsIndex,
                                                (int)(sizeof(g_presentationModes) /
                                                      sizeof(g_presentationModes[0])))
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

int M12_StartupMenu_GetPresentationMode(const M12_StartupMenuState* state) {
    if (!state) {
        return M12_PRESENTATION_V1_ORIGINAL;
    }
    return m12_clamp_index(state->settings.graphicsIndex, M12_PRESENTATION_MODE_COUNT);
}

const char* M12_StartupMenu_GetPresentationModeLabel(const M12_StartupMenuState* state) {
    int mode = M12_StartupMenu_GetPresentationMode(state);
    return g_presentationModes[mode];
}

M12_LaunchIntent M12_StartupMenu_GetLaunchIntent(const M12_StartupMenuState* state) {
    M12_LaunchIntent intent;
    const M12_AssetVersionStatus* version;
    int gi;
    int pmode;
    memset(&intent, 0, sizeof(intent));
    intent.valid = 0;
    if (!state || state->activatedIndex < 0 || state->activatedIndex >= m12_entry_count()) {
        return intent;
    }
    pmode = m12_clamp_index(state->settings.graphicsIndex, M12_PRESENTATION_MODE_COUNT);
    /* V3 is not launchable yet */
    if (pmode == M12_PRESENTATION_V3_MODERN_3D) {
        return intent;
    }
    gi = m12_clamp_index(state->activatedIndex, M12_CONFIG_GAME_COUNT);
    version = m12_selected_version_status(state, gi);
    intent.gameId = state->entries[state->activatedIndex].gameId;
    intent.versionId = version ? version->versionId : NULL;
    intent.presentationMode = pmode;
    intent.options = state->gameOptions[gi];
    /* Enforce constraints on the returned options */
    m12_enforce_mode_constraints(&intent.options, pmode);
    intent.valid = state->entries[state->activatedIndex].available && version && version->matched ? 1 : 0;
    return intent;
}
