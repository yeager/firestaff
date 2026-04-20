#include "menu_startup_m12.h"

#include <stddef.h>
#include <string.h>

enum {
    M12_COLOR_BLACK = 0,
    M12_COLOR_BLUE = 1,
    M12_COLOR_GREEN = 2,
    M12_COLOR_CYAN = 3,
    M12_COLOR_RED = 12,
    M12_COLOR_BROWN = 6,
    M12_COLOR_DARK_GRAY = 8,
    M12_COLOR_LIGHT_GRAY = 7,
    M12_COLOR_LIGHT_BLUE = 9,
    M12_COLOR_WHITE = 15,
    M12_COLOR_YELLOW = 14
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

void M12_StartupMenu_Init(M12_StartupMenuState* state) {
    M12_StartupMenu_InitWithDataDir(state, NULL);
}

void M12_StartupMenu_InitWithDataDir(M12_StartupMenuState* state,
                                     const char* dataDir) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    M12_AssetStatus_Scan(&state->assetStatus, dataDir);
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

static void m12_draw_title(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           const char* subtitle) {
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   32,
                   12,
                   framebufferWidth - 64,
                   40,
                   M12_COLOR_YELLOW,
                   M12_COLOR_BLUE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           20,
                           "FIRESTAFF",
                           3,
                           M12_COLOR_YELLOW);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           58,
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
                   16,
                   framebufferHeight - 24,
                   framebufferWidth - 32,
                   12,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           framebufferHeight - 21,
                           text,
                           1,
                           M12_COLOR_LIGHT_GRAY);
}

static void m12_draw_main_view(const M12_StartupMenuState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    int i;
    int rowY = 84;

    m12_draw_title(framebuffer, framebufferWidth, framebufferHeight, "SELECT A DESTINATION");
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   28,
                   74,
                   framebufferWidth - 56,
                   100,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);

    for (i = 0; i < m12_entry_count(); ++i) {
        const M12_MenuEntry* entry = &state->entries[i];
        const int selected = (i == state->selectedIndex);
        unsigned char rowFill = M12_COLOR_BLACK;
        unsigned char rowBorder = M12_COLOR_DARK_GRAY;
        unsigned char textColor = M12_COLOR_WHITE;
        unsigned char stateColor = M12_COLOR_GREEN;

        if (entry->kind == M12_MENU_ENTRY_GAME && !entry->available) {
            textColor = M12_COLOR_DARK_GRAY;
            stateColor = M12_COLOR_RED;
        }
        if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
            stateColor = M12_COLOR_CYAN;
        }
        if (selected) {
            rowFill = entry->kind == M12_MENU_ENTRY_SETTINGS ? M12_COLOR_CYAN : M12_COLOR_LIGHT_BLUE;
            rowBorder = M12_COLOR_YELLOW;
            textColor = M12_COLOR_WHITE;
        }

        m12_draw_frame(framebuffer,
                       framebufferWidth,
                       framebufferHeight,
                       42,
                       rowY - 4,
                       236,
                       18,
                       rowBorder,
                       rowFill);
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      50,
                      rowY,
                      entry->title,
                      1,
                      textColor);

        if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          214,
                          rowY,
                          "OPEN",
                          1,
                          selected ? M12_COLOR_YELLOW : stateColor);
        } else if (entry->available) {
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          208,
                          rowY,
                          "READY",
                          1,
                          selected ? M12_COLOR_YELLOW : stateColor);
        } else {
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          196,
                          rowY,
                          "MISSING",
                          1,
                          selected ? M12_COLOR_YELLOW : stateColor);
        }

        rowY += 22;
    }

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   28,
                   177,
                   framebufferWidth - 56,
                   15,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  36,
                  181,
                  "DATA DIR",
                  1,
                  M12_COLOR_LIGHT_GRAY);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  92,
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
    unsigned char rowFill = selected ? M12_COLOR_LIGHT_BLUE : M12_COLOR_BLACK;
    unsigned char rowBorder = selected ? M12_COLOR_YELLOW : M12_COLOR_DARK_GRAY;
    unsigned char valueColor = selected ? M12_COLOR_YELLOW : M12_COLOR_CYAN;

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   42,
                   y - 4,
                   236,
                   18,
                   rowBorder,
                   rowFill);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  52,
                  y,
                  label,
                  1,
                  M12_COLOR_WHITE);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  170,
                  y,
                  value,
                  1,
                  valueColor);
}

static void m12_draw_settings_view(const M12_StartupMenuState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    m12_draw_title(framebuffer, framebufferWidth, framebufferHeight, "SETTINGS");
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   28,
                   74,
                   framebufferWidth - 56,
                   100,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);

    m12_draw_settings_row(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          88,
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
                          136,
                          "WINDOW MODE",
                          m12_settings_value_window_mode(state),
                          state->settingsSelectedIndex == M12_SETTINGS_ROW_WINDOW_MODE);

    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   28,
                   177,
                   framebufferWidth - 56,
                   15,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_text(framebuffer,
                  framebufferWidth,
                  framebufferHeight,
                  44,
                  181,
                  "SETTINGS ARE SESSION ONLY IN THIS SLICE",
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
        messageColor = M12_COLOR_RED;
    }

    m12_draw_title(framebuffer, framebufferWidth, framebufferHeight, "STATUS");
    m12_draw_frame(framebuffer,
                   framebufferWidth,
                   framebufferHeight,
                   40,
                   78,
                   framebufferWidth - 80,
                   82,
                   M12_COLOR_DARK_GRAY,
                   M12_COLOR_BLACK);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           92,
                           state->messageLine1,
                           1,
                           messageColor);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           114,
                           state->messageLine2,
                           1,
                           M12_COLOR_WHITE);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           136,
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
                  14,
                  M12_COLOR_DARK_GRAY);
    if (state->view == M12_MENU_VIEW_MESSAGE) {
        m12_draw_message_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else if (state->view == M12_MENU_VIEW_SETTINGS) {
        m12_draw_settings_view(state, framebuffer, framebufferWidth, framebufferHeight);
    } else {
        m12_draw_main_view(state, framebuffer, framebufferWidth, framebufferHeight);
    }
}
