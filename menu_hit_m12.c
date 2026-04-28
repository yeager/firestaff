/*
 * menu_hit_m12.c — bounded M12 mouse hit-testing for the modern
 * high-resolution startup menu. The layout constants here must be
 * kept in sync with menu_startup_render_modern_m12.c (the renderer).
 * See menu_hit_m12.h for scope constraints.
 */

#include "menu_hit_m12.h"

#include <string.h>

/* Canvas size (must match M12_MODERN_MENU_NATIVE_{WIDTH,HEIGHT}). */
#define M12_HIT_CANVAS_W 1920
#define M12_HIT_CANVAS_H 1080

/* --- Main view layout (mirrors draw_main_view) --- */
#define M12_HIT_MAIN_GRID_TOP       170
#define M12_HIT_MAIN_GRID_BOTTOM    (M12_HIT_CANVAS_H - 130)
#define M12_HIT_MAIN_SIDE_MARGIN    48
#define M12_HIT_MAIN_CARD_GAP       22
#define M12_HIT_MAIN_CARD_COUNT     4

/* --- Sub-view panel layout (shared by settings + game options) --- */
#define M12_HIT_PANEL_X        96
#define M12_HIT_PANEL_Y        260
#define M12_HIT_PANEL_W        (M12_HIT_CANVAS_W - 2 * M12_HIT_PANEL_X)
#define M12_HIT_PANEL_H        400
#define M12_HIT_ROW_INDENT     36
#define M12_HIT_ROW_HEIGHT     50

/* Settings rows (3 rows, 70px step, starting panelY+36) */
#define M12_HIT_SETTINGS_ROW_Y0     (M12_HIT_PANEL_Y + 36)
#define M12_HIT_SETTINGS_ROW_STEP   70

/* Game options rows (8 rows: version, patch, language, cheats, speed,
 * aspect, resolution, launch). Renderer draws rows 0..6 at step 52,
 * and the launch row as a dedicated button at the panel bottom. */
#define M12_HIT_GAMEOPT_ROW_Y0      (M12_HIT_PANEL_Y + 28)
#define M12_HIT_GAMEOPT_ROW_STEP    52

/* Settings view has exactly three rows: language, graphics, window mode.
 * Mirrors the private M12_SETTINGS_ROW_COUNT in menu_startup_m12.c. */
#define M12_HIT_SETTINGS_ROW_COUNT  3

/* Museum view mirrors the modern renderer: section rows in the left
 * panel, broad content area on the right for page cycling. */
#define M12_HIT_MUSEUM_CATEGORY_COUNT 5
#define M12_HIT_MUSEUM_CAT_X          (M12_HIT_PANEL_X + 24)
#define M12_HIT_MUSEUM_CAT_Y0         (M12_HIT_PANEL_Y + 54)
#define M12_HIT_MUSEUM_CAT_W          330
#define M12_HIT_MUSEUM_CAT_H          42
#define M12_HIT_MUSEUM_CAT_STEP       56
#define M12_HIT_MUSEUM_CONTENT_X      (M12_HIT_PANEL_X + M12_HIT_MUSEUM_CAT_W + 70)
#define M12_HIT_MUSEUM_CONTENT_Y      (M12_HIT_PANEL_Y + 24)
#define M12_HIT_MUSEUM_CONTENT_W      (M12_HIT_PANEL_W - M12_HIT_MUSEUM_CAT_W - 100)
#define M12_HIT_MUSEUM_CONTENT_H      (M12_HIT_PANEL_H - 48)

/* Launch button inside game options panel. */
#define M12_HIT_LAUNCH_W     240
#define M12_HIT_LAUNCH_H     54
#define M12_HIT_LAUNCH_X     (M12_HIT_PANEL_X + M12_HIT_PANEL_W - M12_HIT_LAUNCH_W - 36)
#define M12_HIT_LAUNCH_Y     (M12_HIT_PANEL_Y + M12_HIT_PANEL_H - M12_HIT_LAUNCH_H - 20)

/* Back button (visible in all non-main views, top-left). */
#define M12_HIT_BACK_X     24
#define M12_HIT_BACK_Y     120
#define M12_HIT_BACK_W     110
#define M12_HIT_BACK_H     44

/* Cycle-arrow strips inside a settings / game-options row.
 * The left half of the row (from row start) is LEFT cycle, the right
 * half is RIGHT cycle. Clicking the label area (centre) just selects
 * the row (delta = 0). For simplicity we split 50/50 on the value
 * side (the right half) of the row: left portion of the right half
 * -> -1 cycle, right portion -> +1 cycle. */
#define M12_HIT_CYCLE_SPLIT_NUM   55   /* right half starts at 55% across */
#define M12_HIT_CYCLE_SPLIT_DEN   100

static int rect_contains(int rx, int ry, int rw, int rh, int x, int y) {
    return x >= rx && y >= ry && x < rx + rw && y < ry + rh;
}

static int m12_hit_main_card_rect(int index, int* rx, int* ry, int* rw, int* rh) {
    int gridTop = M12_HIT_MAIN_GRID_TOP;
    int gridBottom = M12_HIT_MAIN_GRID_BOTTOM;
    int gridH = gridBottom - gridTop;
    int side = M12_HIT_MAIN_SIDE_MARGIN;
    int gap = M12_HIT_MAIN_CARD_GAP;
    int count = M12_HIT_MAIN_CARD_COUNT;
    int cardW = (M12_HIT_CANVAS_W - 2 * side - gap * (count - 1)) / count;
    if (index < 0 || index >= count) return 0;
    *rx = side + index * (cardW + gap);
    *ry = gridTop;
    *rw = cardW;
    *rh = gridH;
    return 1;
}

static int m12_hit_settings_row_rect(int row, int* rx, int* ry, int* rw, int* rh) {
    if (row < 0 || row >= M12_HIT_SETTINGS_ROW_COUNT) return 0;
    *rx = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT;
    *ry = M12_HIT_SETTINGS_ROW_Y0 + row * M12_HIT_SETTINGS_ROW_STEP;
    *rw = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    *rh = M12_HIT_ROW_HEIGHT;
    return 1;
}

static int m12_hit_gameopt_row_rect(int row, int* rx, int* ry, int* rw, int* rh) {
    /* Rows 0..M12_GAME_OPT_ROW_COUNT-1 are drawn in the panel. */
    if (row < 0 || row >= M12_GAME_OPT_ROW_COUNT) return 0;
    *rx = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT;
    *ry = M12_HIT_GAMEOPT_ROW_Y0 + row * M12_HIT_GAMEOPT_ROW_STEP;
    *rw = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    *rh = M12_HIT_ROW_HEIGHT;
    return 1;
}

static int m12_hit_is_cycle_plus(int rx, int rw, int x) {
    int split = rx + (rw * M12_HIT_CYCLE_SPLIT_NUM) / M12_HIT_CYCLE_SPLIT_DEN;
    return x >= split;
}

M12_MouseHit M12_ModernMenu_HitTest(const M12_StartupMenuState* state,
                                    int x, int y) {
    M12_MouseHit hit;
    int rx, ry, rw, rh;
    int i;
    hit.kind = M12_HIT_NONE;
    hit.index = 0;
    hit.delta = 0;
    if (!state) return hit;
    if (x < 0 || y < 0 || x >= M12_HIT_CANVAS_W || y >= M12_HIT_CANVAS_H) {
        return hit;
    }

    if (state->view != M12_MENU_VIEW_MAIN) {
        if (rect_contains(M12_HIT_BACK_X, M12_HIT_BACK_Y,
                          M12_HIT_BACK_W, M12_HIT_BACK_H, x, y)) {
            hit.kind = M12_HIT_BACK;
            return hit;
        }
    }

    switch (state->view) {
        case M12_MENU_VIEW_MAIN:
            /* Modern front-door view has one brand card followed by
             * three game cards.  The brand card is decorative; visible
             * game card slots 1..3 map to menu entry indices 0..2. */
            for (i = 1; i < M12_HIT_MAIN_CARD_COUNT; ++i) {
                if (m12_hit_main_card_rect(i, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    hit.kind = M12_HIT_MAIN_CARD;
                    hit.index = i - 1;
                    return hit;
                }
            }
            break;
        case M12_MENU_VIEW_SETTINGS:
            for (i = 0; i < M12_HIT_SETTINGS_ROW_COUNT; ++i) {
                if (m12_hit_settings_row_rect(i, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    if (m12_hit_is_cycle_plus(rx, rw, x)) {
                        hit.kind = M12_HIT_SETTINGS_CYCLE;
                        hit.index = i;
                        hit.delta = 1;
                    } else {
                        /* Left half just selects the row */
                        hit.kind = M12_HIT_SETTINGS_ROW;
                        hit.index = i;
                    }
                    return hit;
                }
            }
            break;
        case M12_MENU_VIEW_MUSEUM:
            for (i = 0; i < M12_HIT_MUSEUM_CATEGORY_COUNT; ++i) {
                if (rect_contains(M12_HIT_MUSEUM_CAT_X,
                                  M12_HIT_MUSEUM_CAT_Y0 + i * M12_HIT_MUSEUM_CAT_STEP,
                                  M12_HIT_MUSEUM_CAT_W,
                                  M12_HIT_MUSEUM_CAT_H,
                                  x,
                                  y)) {
                    hit.kind = M12_HIT_MUSEUM_CATEGORY;
                    hit.index = i;
                    return hit;
                }
            }
            if (rect_contains(M12_HIT_MUSEUM_CONTENT_X,
                              M12_HIT_MUSEUM_CONTENT_Y,
                              M12_HIT_MUSEUM_CONTENT_W,
                              M12_HIT_MUSEUM_CONTENT_H,
                              x,
                              y)) {
                hit.kind = M12_HIT_MUSEUM_PAGE;
                hit.delta = m12_hit_is_cycle_plus(M12_HIT_MUSEUM_CONTENT_X,
                                                  M12_HIT_MUSEUM_CONTENT_W,
                                                  x) ? 1 : -1;
                return hit;
            }
            break;
        case M12_MENU_VIEW_GAME_OPTIONS:
            /* Launch button */
            if (rect_contains(M12_HIT_LAUNCH_X, M12_HIT_LAUNCH_Y,
                              M12_HIT_LAUNCH_W, M12_HIT_LAUNCH_H, x, y)) {
                hit.kind = M12_HIT_GAMEOPT_LAUNCH;
                hit.index = M12_GAME_OPT_ROW_COUNT;
                return hit;
            }
            for (i = 0; i < M12_GAME_OPT_ROW_COUNT; ++i) {
                if (m12_hit_gameopt_row_rect(i, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    if (m12_hit_is_cycle_plus(rx, rw, x)) {
                        hit.kind = M12_HIT_GAMEOPT_CYCLE;
                        hit.index = i;
                        hit.delta = 1;
                    } else {
                        hit.kind = M12_HIT_GAMEOPT_ROW;
                        hit.index = i;
                    }
                    return hit;
                }
            }
            break;
        case M12_MENU_VIEW_MESSAGE:
            /* Anywhere dismisses */
            hit.kind = M12_HIT_MESSAGE_DISMISS;
            return hit;
        default:
            break;
    }
    return hit;
}

int M12_ModernMenu_ApplyHit(M12_StartupMenuState* state,
                            M12_MouseHit hit) {
    if (!state) return 0;
    switch (hit.kind) {
        case M12_HIT_NONE:
            return 0;
        case M12_HIT_BACK:
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_BACK);
            return 1;
        case M12_HIT_MAIN_CARD: {
            int i;
            /* Move selection to the clicked card via UP/DOWN to keep a
             * single source of truth for cursor movement, then accept. */
            if (hit.index < 0 || hit.index >= M12_HIT_MAIN_CARD_COUNT) return 0;
            while (state->selectedIndex != hit.index) {
                int delta = (hit.index > state->selectedIndex) ? 1 : -1;
                M12_MenuInput mv = (delta > 0) ? M12_MENU_INPUT_DOWN
                                              : M12_MENU_INPUT_UP;
                int before = state->selectedIndex;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->selectedIndex == before) break;
                (void)i;
            }
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
        }
        case M12_HIT_MUSEUM_CATEGORY:
            while (state->museumSelectedIndex != hit.index) {
                int before = state->museumSelectedIndex;
                M12_MenuInput mv = (hit.index > state->museumSelectedIndex)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->museumSelectedIndex == before) break;
            }
            return 1;
        case M12_HIT_MUSEUM_PAGE:
            M12_StartupMenu_HandleInput(state,
                                        hit.delta >= 0 ? M12_MENU_INPUT_RIGHT
                                                       : M12_MENU_INPUT_LEFT);
            return 1;
        case M12_HIT_SETTINGS_ROW:
            while (state->settingsSelectedIndex != hit.index) {
                int before = state->settingsSelectedIndex;
                M12_MenuInput mv = (hit.index > state->settingsSelectedIndex)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->settingsSelectedIndex == before) break;
            }
            return 1;
        case M12_HIT_SETTINGS_CYCLE:
            while (state->settingsSelectedIndex != hit.index) {
                int before = state->settingsSelectedIndex;
                M12_MenuInput mv = (hit.index > state->settingsSelectedIndex)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->settingsSelectedIndex == before) break;
            }
            M12_StartupMenu_HandleInput(state,
                                        hit.delta >= 0 ? M12_MENU_INPUT_RIGHT
                                                       : M12_MENU_INPUT_LEFT);
            return 1;
        case M12_HIT_GAMEOPT_ROW:
            while (state->gameOptSelectedRow != hit.index) {
                int before = state->gameOptSelectedRow;
                M12_MenuInput mv = (hit.index > state->gameOptSelectedRow)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->gameOptSelectedRow == before) break;
            }
            return 1;
        case M12_HIT_GAMEOPT_CYCLE:
            while (state->gameOptSelectedRow != hit.index) {
                int before = state->gameOptSelectedRow;
                M12_MenuInput mv = (hit.index > state->gameOptSelectedRow)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->gameOptSelectedRow == before) break;
            }
            M12_StartupMenu_HandleInput(state,
                                        hit.delta >= 0 ? M12_MENU_INPUT_RIGHT
                                                       : M12_MENU_INPUT_LEFT);
            return 1;
        case M12_HIT_GAMEOPT_LAUNCH:
            /* Jump the cursor to the launch row, then accept. */
            while (state->gameOptSelectedRow < M12_GAME_OPT_ROW_COUNT) {
                int before = state->gameOptSelectedRow;
                M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_DOWN);
                if (state->gameOptSelectedRow == before) break;
            }
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
        case M12_HIT_MESSAGE_DISMISS:
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
    }
    return 0;
}

int M12_ModernMenu_HandlePointer(M12_StartupMenuState* state,
                                 int x, int y,
                                 int clicked,
                                 int* shouldExit) {
    int changed = 0;
    if (!state) return 0;
    state->hoverX = x;
    state->hoverY = y;
    if (clicked) {
        int beforeExit = state->shouldExit;
        M12_MouseHit hit = M12_ModernMenu_HitTest(state, x, y);
        changed = M12_ModernMenu_ApplyHit(state, hit);
        if (shouldExit && state->shouldExit && !beforeExit) {
            *shouldExit = 1;
        }
    }
    return changed;
}
