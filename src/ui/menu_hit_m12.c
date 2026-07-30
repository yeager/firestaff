/*
 * menu_hit_m12.c — bounded M12 mouse hit-testing for the modern
 * high-resolution startup menu. The layout constants here must be
 * kept in sync with menu_startup_render_modern_m12.c (the renderer).
 * See menu_hit_m12.h for scope constraints.
 */

#include "menu_hit_m12.h"
#include "menu_row_metrics_m12.h"

#include <string.h>

/* Canvas size (must match M12_MODERN_MENU_NATIVE_{WIDTH,HEIGHT}). */
#define M12_HIT_CANVAS_W 1920
#define M12_HIT_CANVAS_H 1080

/* --- Main view layout (mirrors draw_main_view) --- */
#define M12_HIT_MAIN_RAIL_X         42
#define M12_HIT_MAIN_RAIL_W         390
#define M12_HIT_MAIN_RAIL_Y         40
#define M12_HIT_MAIN_RAIL_H         (M12_HIT_CANVAS_H - 132)
#define M12_HIT_MAIN_MUSEUM_X       (M12_HIT_MAIN_RAIL_X + 24)
#define M12_HIT_MAIN_MUSEUM_Y       (M12_HIT_MAIN_RAIL_Y + M12_HIT_MAIN_RAIL_H - 106)
#define M12_HIT_MAIN_MUSEUM_W       (M12_HIT_MAIN_RAIL_W - 48)
#define M12_HIT_MAIN_MUSEUM_H       58
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
#define M12_HIT_PANEL_H        800
#define M12_HIT_GAMEOPT_PANEL_H_V1  780
#define M12_HIT_GAMEOPT_PANEL_H_V2  780
#define M12_HIT_ROW_INDENT     36

/* --- Settings view: tab strip (GAME / GRAPHICS / CONTROLS / AUDIO / ACCESSIBILITY / ONLINE) ---
 * These coords mirror m12_draw_tabbed_settings_view in
 * menu_startup_m12.c.  The strip sits above the panel (which
 * starts at M12_HIT_PANEL_Y=260) and is 22 px tall, starting
 * at y=52 with a margin offset of fw/30.  Tab width is the
 * available width divided by M12_SETTINGS_TAB_COUNT. */
#define M12_HIT_SETTINGS_TAB_MARGIN (M12_HIT_CANVAS_W / 30)
#define M12_HIT_SETTINGS_TAB_Y      52
#define M12_HIT_SETTINGS_TAB_H      M12_MENU_ROW_MODERN_TAB_HEIGHT
#define M12_HIT_SETTINGS_TAB_W      ((M12_HIT_CANVAS_W - 2 * M12_HIT_SETTINGS_TAB_MARGIN) / M12_SETTINGS_TAB_COUNT)
#define M12_HIT_ROW_HEIGHT     50

/* Settings rows visible in the modern settings panel. */
#define M12_HIT_SETTINGS_ROW_Y0     (M12_HIT_PANEL_Y + 36)
#define M12_HIT_SETTINGS_ROW_STEP   70
#define M12_HIT_SETTINGS_TWO_COLUMN_THRESHOLD 8
#define M12_HIT_SETTINGS_COLUMN_GAP 24

/* Game options rows (8 rows: version, patch, language, cheats, speed,
 * aspect, resolution, launch). Renderer draws rows 0..6 at step 52,
 * and the launch row as a dedicated button at the panel bottom. */
#define M12_HIT_GAMEOPT_ROW_Y0      (M12_HIT_GAMEOPT_PANEL_Y + 76)
#define M12_HIT_GAMEOPT_ROW_STEP    52

#define M12_HIT_LANGUAGE_POPUP_X      (M12_HIT_PANEL_X + M12_HIT_ROW_INDENT + M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT - 632)
#define M12_HIT_LANGUAGE_POPUP_Y      (M12_HIT_SETTINGS_ROW_Y0 + 56)
#define M12_HIT_LANGUAGE_POPUP_W      632
#define M12_HIT_LANGUAGE_POPUP_PAD    18
#define M12_HIT_LANGUAGE_POPUP_COLS   2
#define M12_HIT_LANGUAGE_POPUP_ITEM_W ((M12_HIT_LANGUAGE_POPUP_W - 2 * M12_HIT_LANGUAGE_POPUP_PAD - 14) / 2)
#define M12_HIT_LANGUAGE_POPUP_ITEM_H 42
#define M12_HIT_LANGUAGE_POPUP_ITEM_GAP 8

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

static int m12_hit_settings_row_rect(int visibleRow,
                                     int visibleRowCount,
                                     int* rx,
                                     int* ry,
                                     int* rw,
                                     int* rh) {
    const int useTwoColumns = visibleRowCount > M12_HIT_SETTINGS_TWO_COLUMN_THRESHOLD;
    const int contentW = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    const int columnW = useTwoColumns
        ? (contentW - M12_HIT_SETTINGS_COLUMN_GAP) / 2
        : contentW;
    const int rowsPerColumn = useTwoColumns
        ? (visibleRowCount + 1) / 2
        : visibleRowCount;
    const int column = useTwoColumns ? visibleRow / rowsPerColumn : 0;
    const int rowInColumn = useTwoColumns ? visibleRow % rowsPerColumn : visibleRow;

    if (visibleRow < 0 || visibleRow >= visibleRowCount || visibleRowCount <= 0) return 0;
    *rx = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT +
        column * (columnW + M12_HIT_SETTINGS_COLUMN_GAP);
    *ry = M12_HIT_SETTINGS_ROW_Y0 + rowInColumn * M12_HIT_SETTINGS_ROW_STEP;
    *rw = columnW;
    *rh = M12_HIT_ROW_HEIGHT;
    return 1;
}

static int m12_hit_gameopt_tile(int x, int y, M12_MouseHit* out) {
    /* draw_game_options_view uses a 4-column grid.  Keep each clickable
     * tile separate: the old full-width row test made PATCH and LANGUAGE
     * clicks operate VERSION, and SPEED-HOTKEYS clicks operate SPEED. */
    const int row_x = M12_HIT_PANEL_X + M12_HIT_ROW_INDENT;
    const int row_w = M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT;
    const int tile_gap = 16;
    const int tile_w = (row_w - 3 * tile_gap) / 4;
    const int tile_h = 64;
    const int grid_y = M12_HIT_GAMEOPT_PANEL_Y + 220;
    int column;
    int grid_row;

    if (!out || x < row_x || x >= row_x + row_w ||
        y < grid_y || y >= grid_y + 3 * (tile_h + tile_gap) + tile_h) {
        return 0;
    }
    column = (x - row_x) / (tile_w + tile_gap);
    if (column < 0 || column >= 4 ||
        x >= row_x + column * (tile_w + tile_gap) + tile_w) {
        return 0;
    }
    grid_row = (y - grid_y) / (tile_h + tile_gap);
    if (grid_row < 0 || grid_row >= 4 ||
        y >= grid_y + grid_row * (tile_h + tile_gap) + tile_h) {
        return 0;
    }

    out->kind = M12_HIT_GAMEOPT_CYCLE;
    out->delta = x >= row_x + column * (tile_w + tile_gap) +
                         (tile_w * M12_HIT_CYCLE_SPLIT_NUM) /
                             M12_HIT_CYCLE_SPLIT_DEN ? 1 : -1;
    if (grid_row == 0) {
        if (column == 0) out->index = M12_GAME_OPT_ROW_VERSION;
        else if (column == 2) out->index = M12_GAME_OPT_ROW_PATCH;
        else if (column == 3) out->index = M12_GAME_OPT_ROW_LANGUAGE;
        else return 0; /* DATA is a read-only asset status. */
    } else if (grid_row == 1) {
        if (column == 0) out->index = M12_GAME_OPT_ROW_CHEATS;
        else if (column == 1 || column == 2) out->index = M12_GAME_OPT_ROW_SPEED;
        else return 0; /* Quick Resume belongs to global settings. */
    } else if (grid_row == 2) {
        if (column == 0) out->index = M12_GAME_OPT_ROW_ASPECT;
        else if (column == 1) out->index = M12_GAME_OPT_ROW_RESOLUTION;
        else return 0; /* Renderer/window are global settings. */
    } else {
        return 0;
    }
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

/* A settings row has a descriptive left half and an explicit value control
 * on the right.  The old hit map changed a value even when the label itself
 * was clicked, while displaying no affordance for that behaviour. */
static int m12_hit_settings_cycle_delta(int rx, int rw, int x, int* outDelta) {
    int valueStart = rx + (rw * M12_HIT_CYCLE_SPLIT_NUM) / M12_HIT_CYCLE_SPLIT_DEN;
    int valueMid = valueStart + (rx + rw - valueStart) / 2;
    if (x < valueStart) return 0;
    if (outDelta) *outDelta = x < valueMid ? -1 : 1;
    return 1;
}

static int m12_hit_language_popup_item(int x, int y) {
    int count = M12_StartupMenu_GetLanguageCount();
    int i;
    for (i = 0; i < count; ++i) {
        int col = i % M12_HIT_LANGUAGE_POPUP_COLS;
        int row = i / M12_HIT_LANGUAGE_POPUP_COLS;
        int ix = M12_HIT_LANGUAGE_POPUP_X + M12_HIT_LANGUAGE_POPUP_PAD +
                 col * (M12_HIT_LANGUAGE_POPUP_ITEM_W + 14);
        int iy = M12_HIT_LANGUAGE_POPUP_Y + M12_HIT_LANGUAGE_POPUP_PAD +
                 row * (M12_HIT_LANGUAGE_POPUP_ITEM_H + M12_HIT_LANGUAGE_POPUP_ITEM_GAP);
        if (rect_contains(ix, iy, M12_HIT_LANGUAGE_POPUP_ITEM_W,
                          M12_HIT_LANGUAGE_POPUP_ITEM_H, x, y)) {
            return i;
        }
    }
    return -1;
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
            /* Five game covers and Settings occupy the 3x2 grid.  The
             * Museum is deliberately presented in the Firestaff rail, so
             * give it an equally direct pointer path instead of leaving it
             * keyboard-only. */
            if (entryCount > 5 &&
                rect_contains(M12_HIT_MAIN_MUSEUM_X,
                              M12_HIT_MAIN_MUSEUM_Y,
                              M12_HIT_MAIN_MUSEUM_W,
                              M12_HIT_MAIN_MUSEUM_H,
                              x, y)) {
                hit.kind = M12_HIT_MAIN_CARD;
                hit.index = 5;
                return hit;
            }
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
            if (state->languagePopupOpen) {
                int li = m12_hit_language_popup_item(x, y);
                if (li >= 0) {
                    hit.kind = M12_HIT_LANGUAGE_POPUP_ITEM;
                    hit.index = li;
                    return hit;
                }
            }
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
            {
                int visibleRowCount = 0;
                const int* visibleRows = M12_StartupMenu_GetSettingsRowsForTab(
                    state ? state->settingsTabIndex : M12_SETTINGS_TAB_GAME,
                    &visibleRowCount);
                for (i = 0; i < visibleRowCount; ++i) {
                if (m12_hit_settings_row_rect(i, visibleRowCount,
                                              &rx, &ry, &rw, &rh) &&
                    rect_contains(rx, ry, rw, rh, x, y)) {
                    int rowIndex = visibleRows[i];
                    if (rowIndex == M12_STARTUP_SETTINGS_ROW_LANGUAGE) {
                        hit.kind = M12_HIT_SETTINGS_ROW;
                        hit.index = rowIndex;
                    } else if (rowIndex == M12_STARTUP_SETTINGS_ROW_DATA_DIR ||
                        rowIndex == M12_STARTUP_SETTINGS_ROW_EXPORT ||
                        rowIndex == M12_STARTUP_SETTINGS_ROW_IMPORT) {
                        hit.kind = M12_HIT_SETTINGS_CYCLE;
                        hit.index = rowIndex;
                        hit.delta = 1;
                    } else if (m12_hit_settings_cycle_delta(rx, rw, x, &hit.delta)) {
                        hit.kind = M12_HIT_SETTINGS_CYCLE;
                        hit.index = rowIndex;
                    } else {
                        /* Left half just selects the row */
                        hit.kind = M12_HIT_SETTINGS_ROW;
                        hit.index = rowIndex;
                    }
                    return hit;
                }
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
            /* The two large mode cards are the presentation selector. */
            if (rect_contains(M12_HIT_PANEL_X + M12_HIT_ROW_INDENT,
                              M12_HIT_GAMEOPT_PANEL_Y + 34,
                              M12_HIT_PANEL_W - 2 * M12_HIT_ROW_INDENT,
                              156, x, y)) {
                hit.kind = M12_HIT_GAMEOPT_CYCLE;
                hit.index = M12_GAME_OPT_ROW_PRESENTATION;
                hit.delta = x < M12_HIT_CANVAS_W / 2 ? -1 : 1;
                return hit;
            }
            if (m12_hit_gameopt_tile(x, y, &hit)) {
                return hit;
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
            state->languagePopupOpen = 0;
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
        case M12_HIT_SETTINGS_CYCLE: {
            /* settingsSelectedIndex holds a row id, and the visible row
             * order per tab is not numerically sorted (the two-column
             * settings layout groups row ids non-monotonically), so the
             * step direction must follow the row's position in the
             * visible list.  A numeric comparison can oscillate forever
             * between two rows whose id order and visible order disagree. */
            int rowCount = 0;
            const int* visibleRows = M12_StartupMenu_GetSettingsRowsForTab(
                state->settingsTabIndex, &rowCount);
            int guard = rowCount + 1;
            while (state->settingsSelectedIndex != hit.index &&
                   guard-- > 0) {
                int before = state->settingsSelectedIndex;
                int curPos = -1;
                int tgtPos = -1;
                int i;
                for (i = 0; i < rowCount; ++i) {
                    if (visibleRows[i] == state->settingsSelectedIndex) curPos = i;
                    if (visibleRows[i] == hit.index) tgtPos = i;
                }
                M12_MenuInput mv = (tgtPos >= curPos)
                                       ? M12_MENU_INPUT_DOWN
                                       : M12_MENU_INPUT_UP;
                M12_StartupMenu_HandleInput(state, mv);
                if (state->settingsSelectedIndex == before) break;
            }
            }
            if (hit.index == M12_STARTUP_SETTINGS_ROW_LANGUAGE) {
                state->languagePopupOpen = !state->languagePopupOpen;
                state->languagePopupSelectedIndex = state->settings.languageIndex;
                return 1;
            }
            state->languagePopupOpen = 0;
            if (hit.kind == M12_HIT_SETTINGS_ROW) {
                /* Selecting a label is intentionally non-destructive. */
                return 1;
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
        case M12_HIT_LANGUAGE_POPUP_ITEM:
        {
            int count = M12_StartupMenu_GetLanguageCount();
            if (hit.index < 0 || hit.index >= count) return 0;
            state->settingsSelectedIndex = M12_STARTUP_SETTINGS_ROW_LANGUAGE;
            /* Match keyboard popup semantics: set the highlighted flag then
             * commit it through ACCEPT, which synchronizes PO/l10n state and
             * marks the choice explicit. Cycling while the popup is open
            * only moves its cursor and leaves the selected language intact. */
            state->languagePopupSelectedIndex = hit.index;
            M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
            return 1;
        }
        case M12_HIT_SETTINGS_TAB:
            state->languagePopupOpen = 0;
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
