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
#define M12_HIT_MAIN_RAIL_X         42
#define M12_HIT_MAIN_RAIL_W         390
#define M12_HIT_MAIN_GRID_LEFT      (M12_HIT_MAIN_RAIL_X + M12_HIT_MAIN_RAIL_W + 44)
#define M12_HIT_MAIN_GRID_TOP       40
#define M12_HIT_MAIN_GRID_BOTTOM    (M12_HIT_CANVAS_H - 130)
#define M12_HIT_MAIN_CARD_GAP       22
#define M12_HIT_MAIN_CARD_MAX_COUNT 6
#define M12_HIT_MAIN_CARD_COLS      3

/* --- Sub-view panel layout (shared by settings + game options) --- */
#define M12_HIT_PANEL_X        96
#define M12_HIT_PANEL_Y        260
#define M12_HIT_GAMEOPT_PANEL_Y 190
#define M12_HIT_PANEL_W        (M12_HIT_CANVAS_W - 2 * M12_HIT_PANEL_X)
#define M12_HIT_PANEL_H        540
#define M12_HIT_GAMEOPT_PANEL_H_V1  780
#define M12_HIT_GAMEOPT_PANEL_H_V2  780
#define M12_HIT_ROW_INDENT     36

/* --- Settings view: tab strip (CONTROLS / AUDIO / ACCESSIBILITY) ---
 * These coords mirror m12_draw_tabbed_settings_view in
 * menu_startup_m12.c.  The strip sits above the panel (which
 * starts at M12_HIT_PANEL_Y=260) and is 22 px tall, starting
 * at y=52 with a margin offset of fw/30.  Tab width is the
 * available width divided by M12_SETTINGS_TAB_COUNT (3). */
#define M12_HIT_SETTINGS_TAB_MARGIN (M12_HIT_CANVAS_W / 30)
#define M12_HIT_SETTINGS_TAB_Y      52
#define M12_HIT_SETTINGS_TAB_H      22
#define M12_HIT_SETTINGS_TAB_W      ((M12_HIT_CANVAS_W - 2 * M12_HIT_SETTINGS_TAB_MARGIN) / M12_SETTINGS_TAB_COUNT)
#define M12_HIT_ROW_HEIGHT     50

/* Settings rows visible in the modern settings panel. */
#define M12_HIT_SETTINGS_ROW_Y0     (M12_HIT_PANEL_Y + 36)
#define M12_HIT_SETTINGS_ROW_STEP   70

/* Game options rows (8 rows: version, patch, language, cheats, speed,
 * aspect, resolution, launch). Renderer draws rows 0..6 at step 52,
 * and the launch row as a dedicated button at the panel bottom. */
#define M12_HIT_GAMEOPT_ROW_Y0      (M12_HIT_GAMEOPT_PANEL_Y + 76)
#define M12_HIT_GAMEOPT_ROW_STEP    52

/* Modern renderer shows a small curated settings subset. Values are the
 * private M12_SETTINGS_ROW_* indices in menu_startup_m12.c. */
static const int m12_hit_visible_settings_rows[] = {0, 1, 3, 14, 15, 16, 30, 42, 43};
#define M12_HIT_SETTINGS_VISIBLE_ROW_COUNT \
    ((int)(sizeof(m12_hit_visible_settings_rows) / sizeof(m12_hit_visible_settings_rows[0])))
#define M12_HIT_SETTINGS_ROW_DATA_DIR 15
#define M12_HIT_SETTINGS_ROW_EXPORT 42
#define M12_HIT_SETTINGS_ROW_IMPORT 43

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

/* Redesigned extras list, matching m12_draw_extras_menu() geometry. */
#define M12_HIT_REDESIGNED_MARGIN     (M12_HIT_CANVAS_W / 20)
#define M12_HIT_EXTRAS_X              (M12_HIT_REDESIGNED_MARGIN + 20)
#define M12_HIT_EXTRAS_Y0             (M12_HIT_CANVAS_H / 5)
#define M12_HIT_EXTRAS_W              (M12_HIT_CANVAS_W / 2)
#define M12_HIT_EXTRAS_H              26
#define M12_HIT_EXTRAS_STEP           26

/* Launch button inside game options panel.  Must mirror
 * menu_startup_render_modern_m12.c:draw_game_options_view(). */
#define M12_HIT_LAUNCH_W     240
#define M12_HIT_LAUNCH_H     54
#define M12_HIT_LAUNCH_BOTTOM_PAD 24

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

static int m12_hit_main_card_rect(int index, int count, int* rx, int* ry, int* rw, int* rh) {
    int gridTop = M12_HIT_MAIN_GRID_TOP;
    int gridBottom = M12_HIT_MAIN_GRID_BOTTOM;
    int gridH = gridBottom - gridTop;
    int gap = M12_HIT_MAIN_CARD_GAP;
    int col;
    int row;
    int cardW;
    int cardH;
    if (count <= 0 || count > M12_HIT_MAIN_CARD_MAX_COUNT) return 0;
    cardW = (M12_HIT_CANVAS_W - M12_HIT_MAIN_GRID_LEFT - 48 - gap * (M12_HIT_MAIN_CARD_COLS - 1)) /
            M12_HIT_MAIN_CARD_COLS;
    cardH = (gridH - gap) / 2;
    if (index < 0 || index >= count) return 0;
    col = index % M12_HIT_MAIN_CARD_COLS;
    row = index / M12_HIT_MAIN_CARD_COLS;
    *rx = M12_HIT_MAIN_GRID_LEFT + col * (cardW + gap);
    *ry = gridTop + row * (cardH + gap);
    *rw = cardW;
    *rh = cardH;
    return 1;
}

static int m12_hit_settings_row_rect(int visibleRow, int* rx, int* ry, int* rw, int* rh) {
    if (visibleRow < 0 || visibleRow >= M12_HIT_SETTINGS_VISIBLE_ROW_COUNT) return 0;
    *rx = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT;
    *ry = M12_HIT_SETTINGS_ROW_Y0 + visibleRow * M12_HIT_SETTINGS_ROW_STEP;
    *rw = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    *rh = M12_HIT_ROW_HEIGHT;
    return 1;
}

static int m12_hit_gameopt_row_rect(int row, int* rx, int* ry, int* rw, int* rh) {
    static const int yOffsets[M12_GAME_OPT_ROW_COUNT] = {
        34, 220, 220, 220, 300, 300, 380, 380
    };
    /* Rows 0..M12_GAME_OPT_ROW_COUNT-1 are drawn in the panel. */
    if (row < 0 || row >= M12_GAME_OPT_ROW_COUNT) return 0;
    *rx = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT;
    *ry = M12_HIT_GAMEOPT_PANEL_Y + yOffsets[row];
    *rw = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    *rh = (row == M12_GAME_OPT_ROW_PRESENTATION) ? 156 : 64;
    return 1;
}

static int m12_hit_launch_rect(const M12_StartupMenuState* state,
                               int* rx, int* ry, int* rw, int* rh) {
    int panelH;
    if (!state) return 0;
    panelH = (M12_StartupMenu_GetPresentationMode(state) == M12_PRESENTATION_V1_ORIGINAL)
                 ? M12_HIT_GAMEOPT_PANEL_H_V1
                 : M12_HIT_GAMEOPT_PANEL_H_V2;
    *rw = M12_HIT_LAUNCH_W;
    *rh = M12_HIT_LAUNCH_H;
    *rx = M12_HIT_PANEL_X + (M12_HIT_PANEL_W - *rw) / 2;
    *ry = M12_HIT_GAMEOPT_PANEL_Y + panelH - *rh - M12_HIT_LAUNCH_BOTTOM_PAD;
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

    if (state->view == M12_MENU_VIEW_MAIN &&
        m12_get_nav_level() == (int)M12_NAV_EXTRAS) {
        for (i = 0; i < M12_EXTRAS_COUNT; ++i) {
            if (rect_contains(M12_HIT_EXTRAS_X,
                              M12_HIT_EXTRAS_Y0 + i * M12_HIT_EXTRAS_STEP,
                              M12_HIT_EXTRAS_W,
                              M12_HIT_EXTRAS_H,
                              x,
                              y)) {
                hit.kind = M12_HIT_EXTRAS_ROW;
                hit.index = i;
                return hit;
            }
        }
        return hit;
    }

    switch (state->view) {
        case M12_MENU_VIEW_MAIN: {
            int entryCount = M12_StartupMenu_GetEntryCount();
            int cardCount = M12_HIT_MAIN_CARD_MAX_COUNT;
            int settingsIndex = entryCount - 1;
            if (settingsIndex < 0) settingsIndex = 0;
            for (i = 0; i < cardCount; ++i) {
                if (m12_hit_main_card_rect(i, cardCount, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    hit.kind = M12_HIT_MAIN_CARD;
                    hit.index = (i < 5) ? i : settingsIndex;
                    if (hit.index >= entryCount) hit.kind = M12_HIT_NONE;
                    return hit;
                }
            }
            break;
        }
        case M12_MENU_VIEW_SETTINGS:
            /* Tab strip click: switch tabs by index.  The strip
             * sits at y=52, h=22, with three equally-sized tabs
             * across the available width.  Clicking a tab moves
             * settingsTabIndex to that tab. */
            for (i = 0; i < M12_SETTINGS_TAB_COUNT; ++i) {
                if (rect_contains(
                        M12_HIT_SETTINGS_TAB_MARGIN + i * M12_HIT_SETTINGS_TAB_W,
                        M12_HIT_SETTINGS_TAB_Y,
                        M12_HIT_SETTINGS_TAB_W - 2,
                        M12_HIT_SETTINGS_TAB_H,
                        x, y)) {
                    hit.kind = M12_HIT_SETTINGS_TAB;
                    hit.index = i;
                    return hit;
                }
            }
            for (i = 0; i < M12_HIT_SETTINGS_VISIBLE_ROW_COUNT; ++i) {
                if (m12_hit_settings_row_rect(i, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    int rowIndex = m12_hit_visible_settings_rows[i];
                    if (rowIndex == M12_HIT_SETTINGS_ROW_DATA_DIR ||
                        rowIndex == M12_HIT_SETTINGS_ROW_EXPORT ||
                        rowIndex == M12_HIT_SETTINGS_ROW_IMPORT) {
                        hit.kind = M12_HIT_SETTINGS_CYCLE;
                        hit.index = rowIndex;
                        hit.delta = 1;
                    } else if (m12_hit_is_cycle_plus(rx, rw, x)) {
                        hit.kind = M12_HIT_SETTINGS_CYCLE;
                        hit.index = rowIndex;
                        hit.delta = 1;
                    } else {
                        /* Left half just selects the row */
                        hit.kind = M12_HIT_SETTINGS_ROW;
                        hit.index = rowIndex;
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
            if (m12_hit_launch_rect(state, &rx, &ry, &rw, &rh) &&
                rect_contains(rx, ry, rw, rh, x, y)) {
                hit.kind = M12_HIT_GAMEOPT_LAUNCH;
                hit.index = M12_GAME_OPT_ROW_COUNT;
                return hit;
            }
            for (i = 0; i < M12_GAME_OPT_ROW_COUNT; ++i) {
                if (m12_hit_gameopt_row_rect(i, &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    if (i == M12_GAME_OPT_ROW_PRESENTATION) {
                        hit.kind = M12_HIT_GAMEOPT_CYCLE;
                        hit.index = i;
                        hit.delta = (x < rx + rw / 2) ? -1 : 1;
                        return hit;
                    }
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
            if (hit.index < 0 || hit.index >= M12_StartupMenu_GetEntryCount()) return 0;
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
        case M12_HIT_SETTINGS_CYCLE:
            while (state->settingsSelectedIndex != hit.index) {
                int before = state->settingsSelectedIndex;
                M12_MenuInput mv = (hit.index > state->settingsSelectedIndex)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->settingsSelectedIndex == before) break;
            }
            /* Cycle the value of the selected row (not the tab
             * strip — that's M12_HIT_SETTINGS_TAB).  v2.7.15
             * split tab cycling (LEFT/RIGHT) from value cycling
             * (VALUE_LEFT/VALUE_RIGHT) so the cycle button
             * doesn't accidentally switch tabs.  M12_HIT_SETTINGS_CYCLE
             * is the hit-test variant for DATA_DIR / EXPORT / IMPORT
             * rows that have no left-half value-cycling area (their
             * whole row is the cycle button) — same code path. */
            M12_StartupMenu_HandleInput(state,
                                        hit.delta >= 0
                                            ? M12_MENU_INPUT_VALUE_RIGHT
                                            : M12_MENU_INPUT_VALUE_LEFT);
            return 1;
        case M12_HIT_SETTINGS_TAB:
            /* Click on a settings tab strip: switch tabs by index.
             * Mirrors the keyboard LEFT/RIGHT path.  Bounded by
             * M12_SETTINGS_TAB_COUNT. */
            while (state->settingsTabIndex != hit.index) {
                int before = state->settingsTabIndex;
                M12_MenuInput mv = (hit.index > state->settingsTabIndex)
                                       ? M12_MENU_INPUT_RIGHT
                                       : M12_MENU_INPUT_LEFT;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->settingsTabIndex == before) break;
            }
            return 1;
        case M12_HIT_GAMEOPT_ROW:
        {
            int guard = 0;
            while (state->gameOptSelectedRow != hit.index &&
                   guard++ < M12_GAME_OPT_ROW_COUNT + 2) {
                int before = state->gameOptSelectedRow;
                M12_MenuInput mv = (hit.index > state->gameOptSelectedRow)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->gameOptSelectedRow == before) break;
            }
            return 1;
        }
        case M12_HIT_GAMEOPT_CYCLE:
        {
            int guard = 0;
            while (state->gameOptSelectedRow != hit.index &&
                   guard++ < M12_GAME_OPT_ROW_COUNT + 2) {
                int before = state->gameOptSelectedRow;
                M12_MenuInput mv = (hit.index > state->gameOptSelectedRow)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->gameOptSelectedRow == before) break;
            }
            if (hit.index == M12_GAME_OPT_ROW_PRESENTATION &&
                state->activatedIndex >= 0 &&
                state->activatedIndex < M12_CONFIG_GAME_COUNT) {
                if (hit.delta < 0) {
                    int guard = 0;
                    while (state->gameOptions[state->activatedIndex].presentationModeIndex !=
                               M12_PRESENTATION_V1_ORIGINAL &&
                           guard++ < M12_PRESENTATION_MODE_COUNT) {
                        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_LEFT);
                    }
                } else if (state->gameOptions[state->activatedIndex].presentationModeIndex ==
                           M12_PRESENTATION_V1_ORIGINAL) {
                    M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_RIGHT);
                }
                return 1;
            }
            M12_StartupMenu_HandleInput(state,
                                        hit.delta >= 0 ? M12_MENU_INPUT_RIGHT
                                                       : M12_MENU_INPUT_LEFT);
            return 1;
        }
        case M12_HIT_GAMEOPT_LAUNCH:
            /* Pointer activation owns the launch button directly.  Avoid
             * replaying DOWN events here because the keyboard cursor can
             * wrap/clamp differently across presentation-mode rows. */
            state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
        case M12_HIT_MESSAGE_DISMISS:
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
        case M12_HIT_EXTRAS_ROW:
            while ((int)state->extrasSelected != hit.index) {
                int before = (int)state->extrasSelected;
                M12_MenuInput mv = (hit.index > (int)state->extrasSelected)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                m12_redesigned_handle_input(state, mv == M12_MENU_INPUT_UP,
                                            mv == M12_MENU_INPUT_DOWN,
                                            0, 0, 0, 0);
                if ((int)state->extrasSelected == before) break;
            }
            m12_redesigned_handle_input(state, 0, 0, 0, 0, 1, 0);
            return 1;
    }
    return 0;
}

int M12_ModernMenu_HandlePointer(M12_StartupMenuState* state,
                                 int x, int y,
                                 int clicked,
                                 int* shouldExit) {
    int changed = 0;
    M12_MouseHit hit;
    if (!state) return 0;
    state->hoverX = x;
    state->hoverY = y;
    hit = M12_ModernMenu_HitTest(state, x, y);
    if (clicked) {
        int beforeExit = state->shouldExit;
        changed = M12_ModernMenu_ApplyHit(state, hit);
        if (shouldExit && state->shouldExit && !beforeExit) {
            *shouldExit = 1;
        }
    } else {
        /* Mouse navigation without clicking: move the same selection
         * cursor that keyboard navigation uses, so hover visibly
         * follows the pointer and a subsequent click activates exactly
         * what is highlighted. */
        switch (hit.kind) {
            case M12_HIT_MAIN_CARD:
                if (hit.index >= 0 && hit.index < M12_StartupMenu_GetEntryCount() &&
                    state->selectedIndex != hit.index) {
                    state->selectedIndex = hit.index;
                    changed = 1;
                }
                break;
            case M12_HIT_MUSEUM_CATEGORY:
                if (state->museumSelectedIndex != hit.index) {
                    state->museumSelectedIndex = hit.index;
                    changed = 1;
                }
                break;
            case M12_HIT_SETTINGS_ROW:
            case M12_HIT_SETTINGS_CYCLE:
                if (state->settingsSelectedIndex != hit.index) {
                    state->settingsSelectedIndex = hit.index;
                    changed = 1;
                }
                break;
            case M12_HIT_GAMEOPT_ROW:
            case M12_HIT_GAMEOPT_CYCLE:
            case M12_HIT_GAMEOPT_LAUNCH:
                if (state->gameOptSelectedRow != hit.index) {
                    state->gameOptSelectedRow = hit.index;
                    changed = 1;
                }
                break;
            case M12_HIT_EXTRAS_ROW:
                if ((int)state->extrasSelected != hit.index) {
                    state->extrasSelected = (M12_ExtrasItem)hit.index;
                    changed = 1;
                }
                break;
            case M12_HIT_NONE:
            case M12_HIT_MUSEUM_PAGE:
            case M12_HIT_MESSAGE_DISMISS:
            case M12_HIT_BACK:
            default:
                break;
        }
    }
    return changed;
}
