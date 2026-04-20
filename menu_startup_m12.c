#include "menu_startup_m12.h"

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
    {"DUNGEON MASTER", "dm1", M12_MENU_ENTRY_GAME, 0},
    {"CHAOS STRIKES BACK", "csb", M12_MENU_ENTRY_GAME, 0},
    {"DUNGEON MASTER II", "dm2", M12_MENU_ENTRY_GAME, 0},
    {"SETTINGS", NULL, M12_MENU_ENTRY_SETTINGS, 1}
};

typedef struct {
    char ch;
    unsigned char rows[7];
} M12_Glyph;

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
        state->messageLine1 = "READY TO LAUNCH";
        state->messageLine2 = entry->title;
        state->messageLine3 = "ESC RETURNS TO MENU";
    } else {
        state->messageLine1 = "GAME DATA NOT FOUND";
        state->messageLine2 = "CHECK FIRESTAFF DATA DIR";
        state->messageLine3 = "ESC RETURNS TO MENU";
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

static void m12_draw_text(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          const char* text,
                          int scale,
                          unsigned char color) {
    int cursorX = x;
    const char* p;
    if (!text || scale <= 0) {
        return;
    }
    for (p = text; *p != '\0'; ++p) {
        int row;
        const unsigned char* glyph = m12_find_glyph(*p);
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
        cursorX += 6 * scale;
    }
}

static void m12_draw_centered_text(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   int y,
                                   const char* text,
                                   int scale,
                                   unsigned char color) {
    int width;
    int x;
    if (!text) {
        return;
    }
    width = (int)strlen(text) * 6 * scale;
    x = (framebufferWidth - width) / 2;
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x,
                  y,
                  text,
                  scale,
                  color);
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

static void m12_draw_title(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           const char* eyebrow,
                           const char* subtitle) {
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   16,
                   12,
                   framebufferWidth - 32,
                   42,
                   M12_COLOR_YELLOW,
                   M12_COLOR_NAVY);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  20,
                  16,
                  framebufferWidth - 40,
                  5,
                  M12_COLOR_BROWN);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           24,
                           "FIRESTAFF",
                           3,
                           M12_COLOR_YELLOW);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           17,
                           eyebrow,
                           1,
                           M12_COLOR_WHITE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           48,
                           subtitle,
                           1,
                           M12_COLOR_LIGHT_GRAY);
}

static void m12_draw_footer(unsigned char* framebuffer,
                            int framebufferWidth,
                            int framebufferHeight,
                            const char* text) {
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   12,
                   framebufferHeight - 18,
                   framebufferWidth - 24,
                   10,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           framebufferHeight - 16,
                           text,
                           1,
                           M12_COLOR_LIGHT_GRAY);
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
        return "THE CLASSIC";
    }
    if (entry->gameId && strcmp(entry->gameId, "csb") == 0) {
        return "HARD MODE";
    }
    if (entry->gameId && strcmp(entry->gameId, "dm2") == 0) {
        return "COMING LATER";
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
    return entry->available ? "MD5 VERIFIED" : "ASSETS MISSING";
}

static void m12_draw_box_art(unsigned char* framebuffer,
                             int framebufferWidth,
                             int framebufferHeight,
                             const M12_MenuEntry* entry) {
    unsigned char fill = entry && entry->kind == M12_MENU_ENTRY_SETTINGS
                             ? M12_COLOR_DARK_GRAY
                             : m12_game_card_fill(entry ? entry->gameId : NULL);
    unsigned char accent = entry && entry->available ? M12_COLOR_YELLOW : M12_COLOR_LIGHT_RED;

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   66,
                   92,
                   108,
                   M12_COLOR_YELLOW,
                   fill);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  72,
                  80,
                  8,
                  accent);
    m12_draw_horizontal_rule(framebuffer, framebufferWidth, framebufferHeight, 24, 102, 80, M12_COLOR_BLACK);
    m12_draw_horizontal_rule(framebuffer, framebufferWidth, framebufferHeight, 24, 126, 80, M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           88,
                           m12_game_card_line1(entry),
                           2,
                           M12_COLOR_WHITE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           116,
                           m12_game_card_line2(entry),
                           1,
                           M12_COLOR_WHITE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           138,
                           m12_game_card_line3(entry),
                           1,
                           entry && entry->available ? M12_COLOR_YELLOW : M12_COLOR_WHITE);
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
                   12,
                   M12_COLOR_BLACK,
                   fill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  x + 8,
                  y + 2,
                  text,
                  1,
                  textColor);
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
                   22,
                   border,
                   fill);
    m12_fill_rect(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  126,
                  y + 3,
                  4,
                  16,
                  selected ? M12_COLOR_YELLOW : (entry->available ? M12_COLOR_GREEN : M12_COLOR_LIGHT_RED));
    if (selected) {
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      136,
                      y + 7,
                      ">",
                      1,
                      M12_COLOR_YELLOW);
    }
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  148,
                  y + 7,
                  entry->title,
                  1,
                  titleColor);

    if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
        m12_draw_status_chip(framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             236,
                             y + 5,
                             "OPEN",
                             selected ? M12_COLOR_CYAN : M12_COLOR_DARK_GRAY,
                             M12_COLOR_WHITE);
    } else if (entry->available) {
        m12_draw_status_chip(framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             232,
                             y + 5,
                             "READY",
                             selected ? M12_COLOR_YELLOW : M12_COLOR_GREEN,
                             M12_COLOR_BLACK);
    } else {
        m12_draw_status_chip(framebuffer,
                             framebufferWidth,
                             framebufferHeight,
                             224,
                             y + 5,
                             "MISSING",
                             selected ? M12_COLOR_LIGHT_RED : M12_COLOR_MAROON,
                             M12_COLOR_WHITE);
    }
}

static void m12_draw_main_view(const M12_StartupMenuState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    int i;
    int rowY = 70;
    const M12_MenuEntry* selectedEntry = M12_StartupMenu_GetEntry(state, state->selectedIndex);

    m12_draw_title(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   "PRODUCT PREVIEW",
                   "SELECT A DESTINATION");
    m12_draw_box_art(framebuffer, framebufferWidth, framebufferHeight, selectedEntry);
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
                  "PLAYABLE BUILDS",
                  1,
                  M12_COLOR_LIGHT_CYAN);
    for (i = 0; i < m12_entry_count(); ++i) {
        m12_draw_main_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          rowY,
                          &state->entries[i],
                          i == state->selectedIndex);
        rowY += 25;
    }

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   177,
                   framebufferWidth - 36,
                   14,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  181,
                  "DATA DIR",
                  1,
                  M12_COLOR_LIGHT_GRAY);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  84,
                  181,
                  M12_AssetStatus_GetDataDir(&state->assetStatus),
                  1,
                  M12_COLOR_WHITE);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    "UP/DOWN MOVE   ENTER OPEN   ESC EXIT");
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
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  132,
                  y + 8,
                  label,
                  1,
                  M12_COLOR_WHITE);
    m12_draw_status_chip(framebuffer,
                         framebufferWidth,
                         framebufferHeight,
                         230,
                         y + 6,
                         value,
                         valueFill,
                         valueText);
}

static void m12_draw_settings_view(const M12_StartupMenuState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    M12_MenuEntry settingsCard = {"SETTINGS", NULL, M12_MENU_ENTRY_SETTINGS, 1};
    m12_draw_title(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   "PRODUCT PREVIEW",
                   "SETTINGS");
    m12_draw_box_art(framebuffer, framebufferWidth, framebufferHeight, &settingsCard);
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
                  "PERSISTED OPTIONS",
                  1,
                  M12_COLOR_LIGHT_CYAN);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          84,
                          "LANGUAGE",
                          m12_settings_value_language(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_LANGUAGE);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          112,
                          "GRAPHICS LEVEL",
                          m12_settings_value_graphics(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_GRAPHICS);
    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          140,
                          "WINDOW MODE",
                          m12_settings_value_window_mode(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE);
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   18,
                   177,
                   framebufferWidth - 36,
                   14,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  24,
                  181,
                  "CHANGES SAVE IMMEDIATELY TO CONFIG",
                  1,
                  M12_COLOR_LIGHT_GRAY);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    "LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK");
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
                   "PRODUCT PREVIEW",
                   "STATUS");
    m12_draw_box_art(framebuffer, framebufferWidth, framebufferHeight, entry);
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
                           1,
                           messageColor);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           112,
                           state->messageLine2,
                           1,
                           M12_COLOR_WHITE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           134,
                           state->messageLine3,
                           1,
                           M12_COLOR_LIGHT_GRAY);
    m12_draw_footer(framebuffer,
                    framebufferWidth,
                    framebufferHeight,
                    "ENTER OR ESC RETURNS TO MENU");
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
                  8,
                  M12_COLOR_DARK_GRAY);
    if (state->view == M12_MENU_VIEW_MESSAGE) {
        m12_draw_message_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else if (state->view == M12_MENU_VIEW_SETTINGS) {
        m12_draw_settings_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else {
        m12_draw_main_view(state, framebuffer, framebufferWidth, framebufferHeight);
    }
}
