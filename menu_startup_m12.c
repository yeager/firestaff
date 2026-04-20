#include "menu_startup_m12.h"

#include <stddef.h>
#include <string.h>

enum {
    M12_COLOR_BLACK = 0,
    M12_COLOR_BLUE = 1,
    M12_COLOR_DARK_GRAY = 8,
    M12_COLOR_LIGHT_GRAY = 7,
    M12_COLOR_WHITE = 15,
    M12_COLOR_YELLOW = 14,
    M12_COLOR_RED = 12
};

static const M12_MenuGameEntry g_menuEntries[] = {
    {"DUNGEON MASTER", 1},
    {"CHAOS STRIKES BACK", 0},
    {"DUNGEON MASTER II", 0}
};

typedef struct {
    char ch;
    unsigned char rows[7];
} M12_Glyph;

static const M12_Glyph g_font[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'>', {1, 2, 4, 8, 4, 2, 1}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
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

static int m12_entry_count(void) {
    return (int)(sizeof(g_menuEntries) / sizeof(g_menuEntries[0]));
}

int M12_StartupMenu_GetEntryCount(void) {
    return m12_entry_count();
}

const M12_MenuGameEntry* M12_StartupMenu_GetEntry(int index) {
    if (index < 0 || index >= m12_entry_count()) {
        return NULL;
    }
    return &g_menuEntries[index];
}

void M12_StartupMenu_Init(M12_StartupMenuState* state) {
    if (!state) {
        return;
    }
    state->selectedIndex = 0;
    state->shouldExit = 0;
    state->activatedIndex = -1;
    state->view = M12_MENU_VIEW_MAIN;
    state->messageLine1 = "";
    state->messageLine2 = "";
}

static void m12_activate_selected(M12_StartupMenuState* state) {
    const M12_MenuGameEntry* entry;
    if (!state) {
        return;
    }
    entry = M12_StartupMenu_GetEntry(state->selectedIndex);
    if (!entry) {
        return;
    }
    state->activatedIndex = state->selectedIndex;
    state->view = M12_MENU_VIEW_MESSAGE;
    if (entry->available) {
        state->messageLine1 = "STARTING DUNGEON MASTER...";
        state->messageLine2 = "PRESS ESC TO RETURN";
    } else {
        state->messageLine1 = "GAME DATA NOT FOUND";
        state->messageLine2 = "PRESS ESC TO RETURN";
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
        if (input == M12_MENU_INPUT_BACK) {
            state->view = M12_MENU_VIEW_MAIN;
            state->messageLine1 = "";
            state->messageLine2 = "";
        }
        return;
    }
    switch (input) {
        case M12_MENU_INPUT_UP:
            state->selectedIndex =
                (state->selectedIndex + count - 1) % count;
            break;
        case M12_MENU_INPUT_DOWN:
            state->selectedIndex = (state->selectedIndex + 1) % count;
            break;
        case M12_MENU_INPUT_ACCEPT:
            m12_activate_selected(state);
            break;
        case M12_MENU_INPUT_BACK:
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

static void m12_draw_main_view(const M12_StartupMenuState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    int i;
    int rowY = 64;
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           18,
                           "FIRESTAFF",
                           3,
                           M12_COLOR_YELLOW);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           44,
                           "SELECT A GAME",
                           1,
                           M12_COLOR_WHITE);

    for (i = 0; i < m12_entry_count(); ++i) {
        const M12_MenuGameEntry* entry = &g_menuEntries[i];
        const int selected = (i == state->selectedIndex);
        const int textX = 64;
        unsigned char textColor = entry->available ? M12_COLOR_WHITE
                                                   : M12_COLOR_DARK_GRAY;
        if (selected) {
            m12_fill_rect(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          40,
                          rowY - 6,
                          framebufferWidth - 80,
                          22,
                          entry->available ? M12_COLOR_BLUE
                                           : M12_COLOR_LIGHT_GRAY);
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          48,
                          rowY,
                          ">",
                          1,
                          M12_COLOR_YELLOW);
            textColor = entry->available ? M12_COLOR_WHITE : M12_COLOR_BLACK;
        }
        m12_draw_text(framebuffer,
                      framebufferWidth,
                      framebufferHeight,
                      textX,
                      rowY,
                      entry->title,
                      1,
                      textColor);
        if (!entry->available) {
            m12_draw_text(framebuffer,
                          framebufferWidth,
                          framebufferHeight,
                          224,
                          rowY,
                          "MISSING",
                          1,
                          selected ? M12_COLOR_RED : M12_COLOR_DARK_GRAY);
        }
        rowY += 28;
    }

    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           166,
                           "ENTER TO ACTIVATE",
                           1,
                           M12_COLOR_LIGHT_GRAY);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           180,
                           "ESC TO EXIT",
                           1,
                           M12_COLOR_LIGHT_GRAY);
}

static void m12_draw_message_view(const M12_StartupMenuState* state,
                                  unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight) {
    unsigned char messageColor = M12_COLOR_WHITE;
    const M12_MenuGameEntry* entry =
        M12_StartupMenu_GetEntry(state->activatedIndex);
    if (entry && !entry->available) {
        messageColor = M12_COLOR_RED;
    }
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           54,
                           "FIRESTAFF",
                           2,
                           M12_COLOR_YELLOW);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           98,
                           state->messageLine1,
                           1,
                           messageColor);
    m12_draw_centered_text(framebuffer,
                           framebufferWidth,
                           framebufferHeight,
                           126,
                           state->messageLine2,
                           1,
                           M12_COLOR_LIGHT_GRAY);
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
    if (state->view == M12_MENU_VIEW_MESSAGE) {
        m12_draw_message_view(state,
                              framebuffer,
                              framebufferWidth,
                              framebufferHeight);
    } else {
        m12_draw_main_view(state,
                           framebuffer,
                           framebufferWidth,
                           framebufferHeight);
    }
}
