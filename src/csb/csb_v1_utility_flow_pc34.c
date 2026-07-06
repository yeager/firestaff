/* pass603: CSB V1 Phase 6 — Utility Setup Flow
 *
 * CSB utility disk setup flow. Different from DM1's utility flow:
 *   - No champion creation hall (CSB uses champion import from DM1 saves)
 *   - Always starts from utility disk (CEDT mode, not direct start)
 *   - Party is imported from DM1 saves, not created in-game
 *   - Resurrect/reincarnate available from champion panel
 *
 * Source references:
 *   ReDMCSB ENTRANCE.C — setup/selector flow adapted for CSB
 *   ReDMCSB CEDTINC7.C — utility disk prompt strings
 *   ReDMCSB CEDTDATA.C — G3921 PLEASE_INSERT_UTILITY_DISK
 *   ReDMCSB CEDTDATA.C — G3755 THAT_S_THE_CSB_UTILITY_DISK
 *   ReDMCSB CEDTDATA.C — G3764 THAT_S_NOT_THE_UTILITY_DISK
 *   CSBWin/CSBCode.cpp — StartChaos setup (11414 lines)
 */

#include "csb_v1_utility_flow_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "csb_v1_utility_import_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Internal constants ──────────────────────────────────────────────── */

/* Maximum disk check attempts (prevents infinite loops) */
#define CSB_V1_UTIL_MAX_ATTEMPTS   5
#define CSB_V1_UTIL_MENU_ACTION_COUNT CSB_V1_UTIL_MENU_ROW_COUNT
#define CSB_V1_UTIL_MENU_X 38
#define CSB_V1_UTIL_MENU_Y 104
#define CSB_V1_UTIL_MENU_W 244
#define CSB_V1_UTIL_MENU_ROW_H 12
#define CSB_V1_UTIL_MENU_H (CSB_V1_UTIL_MENU_ROW_H * CSB_V1_UTIL_MENU_ROW_COUNT)
#define CSB_V1_UTIL_PANEL_X 38
#define CSB_V1_UTIL_PANEL_Y 80
#define CSB_V1_UTIL_PANEL_W 244
#define CSB_V1_UTIL_PROMPT_Y 92
#define CSB_V1_UTIL_PREVIEW_X 48
#define CSB_V1_UTIL_PREVIEW_Y 154
#define CSB_V1_UTIL_PREVIEW_ROW_H 10
#define CSB_V1_UTIL_PREVIEW_MAX_ROWS 4

static const CSB_V1_UtilFlowAction s_csb_v1_util_menu_actions[
    CSB_V1_UTIL_MENU_ACTION_COUNT] = {
    CSB_V1_UTIL_ACTION_IMPORT,
    CSB_V1_UTIL_ACTION_LOAD,
    CSB_V1_UTIL_ACTION_NEW,
    CSB_V1_UTIL_ACTION_VIEW
};

/* Utility disk prompt strings (from ReDMCSB CEDTDATA.C).
 * These are reserved for the UI layer: the UI calls csb_v1_util_flow_get_prompt(ctx)
 * to retrieve the current prompt string for the flow state.
 * Marked as intentionally unused until the UI layer is wired up. */
/*
 * Utility disk prompt strings (from ReDMCSB CEDTDATA.C):
 *   G3921: PLEASE PUT THE CHAOS STRIKES BACK UTILITY DISK IN ~
 *   G3755: THAT'S THE CHAOS STRIKES BACK UTILITY DISK!
 *   G3764: THAT'S NOT THE UTILITY DISK!
 *   G3922: IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE
 *   G3923: LOAD SAVED GAME
 *   G3924: START NEW GAME
 *   G3925: VIEW CHAMPION DETAILS
 * These strings are reserved for the UI layer (not yet wired).
 */

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_utility_flow_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C: setup/selector flow adapted for CSB\n"
        "ReDMCSB CEDTINC7.C: utility disk prompt flow\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3755 THAT_S_THE_CSB_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3764 THAT_S_NOT_THE_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3922/G3923/G3924/G3925 menu strings\n"
        "CSBWin/CSBCode.cpp: StartChaos champion init (11414 lines)\n"
        "CSBWin/SaveGame.cpp: DM1 import path (2953 lines)\n"
        "MEDIA529_F20E_F20J: CSB utility disk boot path\n"
        "MEDIA332_F20E_F21E_A31E_F31E: CSB utility vs game disk\n";
}

/* ── Initialize ─────────────────────────────────────────────────────── */
void csb_v1_util_flow_init(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = CSB_V1_UTIL_FLOW_INIT;
    ctx->action = CSB_V1_UTIL_ACTION_EXIT;
    ctx->disk_result = CSB_V1_UTIL_DISK_MISSING;
    ctx->attempts = 0;
    ctx->max_attempts = CSB_V1_UTIL_MAX_ATTEMPTS;
    ctx->utility_disk_verified = 0;
    ctx->import_confirmed = 0;
    ctx->selected_action_index = 0;
    ctx->last_error = 0;
    memset(ctx->utility_disk_path, 0, sizeof(ctx->utility_disk_path));
    memset(ctx->dm1_save_path, 0, sizeof(ctx->dm1_save_path));
    memset(ctx->csb_save_path, 0, sizeof(ctx->csb_save_path));
}

/* ── State name for UI ───────────────────────────────────────────────── */
const char *csb_v1_util_flow_state_name(CSB_V1_UtilFlowState state)
{
    switch (state) {
    case CSB_V1_UTIL_FLOW_INIT:           return "INIT";
    case CSB_V1_UTIL_FLOW_INSERT_DISK:    return "INSERT_DISK";
    case CSB_V1_UTIL_FLOW_VERIFY_DISK:    return "VERIFY_DISK";
    case CSB_V1_UTIL_FLOW_DISK_OK:        return "DISK_OK";
    case CSB_V1_UTIL_FLOW_SELECT_ACTION:  return "SELECT_ACTION";
    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS: return "IMPORT_CHAMPIONS";
    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT: return "CONFIRM_IMPORT";
    case CSB_V1_UTIL_FLOW_LOAD_GAME:      return "LOAD_GAME";
    case CSB_V1_UTIL_FLOW_NEW_GAME:       return "NEW_GAME";
    case CSB_V1_UTIL_FLOW_ERROR:          return "ERROR";
    case CSB_V1_UTIL_FLOW_DONE:           return "DONE";
    default:                              return "UNKNOWN";
    }
}

const char *csb_v1_util_flow_prompt(const CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return "";
    switch (ctx->state) {
    case CSB_V1_UTIL_FLOW_INIT:
    case CSB_V1_UTIL_FLOW_INSERT_DISK:
        return "PLEASE PUT THE CHAOS STRIKES BACK UTILITY DISK IN";
    case CSB_V1_UTIL_FLOW_VERIFY_DISK:
        return "CHECKING CHAOS STRIKES BACK UTILITY DISK";
    case CSB_V1_UTIL_FLOW_DISK_OK:
        return "THAT'S THE CHAOS STRIKES BACK UTILITY DISK!";
    case CSB_V1_UTIL_FLOW_SELECT_ACTION:
        return "IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE\n"
               "LOAD SAVED GAME\n"
               "START NEW GAME\n"
               "VIEW CHAMPION DETAILS";
    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS:
        return "IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE";
    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT:
        return "IMPORT THESE CHAMPIONS TO CHAOS STRIKES BACK?";
    case CSB_V1_UTIL_FLOW_LOAD_GAME:
        return "LOAD SAVED GAME";
    case CSB_V1_UTIL_FLOW_NEW_GAME:
        return "START NEW GAME";
    case CSB_V1_UTIL_FLOW_ERROR:
        return csb_v1_util_flow_last_error((CSB_V1_UtilFlowContext *)ctx);
    case CSB_V1_UTIL_FLOW_DONE:
        return "CHAOS STRIKES BACK READY";
    default:
        return "";
    }
}

/* ── Last error string ──────────────────────────────────────────────── */
const char *csb_v1_util_flow_last_error(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return "NULL context";
    switch (ctx->last_error) {
    case 0:   return "No error";
    case -1:  return "Disk missing or unreadable";
    case -2:  return "Wrong disk type (not CSB Utility Disk)";
    case -3:  return "Import failed — invalid DM1 save";
    case -4:  return "Load failed — save not found";
    case -5:  return "Save corrupted";
    case -6:  return "Maximum disk check attempts reached";
    case -7:  return "Invalid party state";
    case -8:  return "No champions imported/loaded";
    default:  return "Unknown error";
    }
}

/* ── Set action ─────────────────────────────────────────────────────── */
void csb_v1_util_flow_set_action(CSB_V1_UtilFlowContext *ctx,
                                   CSB_V1_UtilFlowAction action)
{
    int i;
    if (!ctx) return;
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION &&
        ctx->state != CSB_V1_UTIL_FLOW_INIT) {
        /* Can only set action from SELECT_ACTION state */
        return;
    }
    ctx->action = action;
    for (i = 0; i < CSB_V1_UTIL_MENU_ACTION_COUNT; ++i) {
        if (s_csb_v1_util_menu_actions[i] == action) {
            ctx->selected_action_index = i;
            break;
        }
    }
}

int csb_v1_util_flow_move_action_cursor(CSB_V1_UtilFlowContext *ctx,
                                        int delta)
{
    int next;

    if (!ctx) return -1;
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION &&
        ctx->state != CSB_V1_UTIL_FLOW_INIT) {
        return -1;
    }
    next = ctx->selected_action_index + delta;
    while (next < 0) next += CSB_V1_UTIL_MENU_ACTION_COUNT;
    next %= CSB_V1_UTIL_MENU_ACTION_COUNT;
    ctx->selected_action_index = next;
    ctx->action = s_csb_v1_util_menu_actions[next];
    return ctx->selected_action_index;
}

CSB_V1_UtilFlowAction csb_v1_util_flow_selected_action(
    const CSB_V1_UtilFlowContext *ctx)
{
    int index;

    if (!ctx) return CSB_V1_UTIL_ACTION_EXIT;
    index = ctx->selected_action_index;
    if (index < 0 || index >= CSB_V1_UTIL_MENU_ACTION_COUNT) {
        return CSB_V1_UTIL_ACTION_EXIT;
    }
    return s_csb_v1_util_menu_actions[index];
}

int csb_v1_util_flow_accept_selected_action(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return -1;
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION &&
        ctx->state != CSB_V1_UTIL_FLOW_INIT) {
        return -1;
    }
    ctx->action = csb_v1_util_flow_selected_action(ctx);
    return 0;
}

int csb_v1_util_flow_cancel_to_menu(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return -1;
    switch (ctx->state) {
    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS:
    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT:
    case CSB_V1_UTIL_FLOW_LOAD_GAME:
    case CSB_V1_UTIL_FLOW_NEW_GAME:
    case CSB_V1_UTIL_FLOW_ERROR:
        ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
        ctx->action = CSB_V1_UTIL_ACTION_EXIT;
        ctx->import_confirmed = 0;
        ctx->last_error = 0;
        return 0;
    default:
        return -1;
    }
}

int csb_v1_util_flow_retry_error(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx || ctx->state != CSB_V1_UTIL_FLOW_ERROR) {
        return -1;
    }

    ctx->last_error = 0;
    ctx->import_confirmed = 0;
    ctx->action = CSB_V1_UTIL_ACTION_EXIT;
    if (ctx->disk_result == CSB_V1_UTIL_DISK_OK ||
        ctx->utility_disk_verified) {
        ctx->disk_result = CSB_V1_UTIL_DISK_OK;
        ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
    } else {
        ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
    }
    return 0;
}

const char *csb_v1_util_flow_action_label(CSB_V1_UtilFlowAction action)
{
    switch (action) {
    case CSB_V1_UTIL_ACTION_IMPORT:
        return "IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE";
    case CSB_V1_UTIL_ACTION_LOAD:
        return "LOAD SAVED GAME";
    case CSB_V1_UTIL_ACTION_NEW:
        return "START NEW GAME";
    case CSB_V1_UTIL_ACTION_VIEW:
        return "VIEW CHAMPION DETAILS";
    case CSB_V1_UTIL_ACTION_EXIT:
        return "EXIT";
    default:
        return "";
    }
}

int csb_v1_util_flow_entrance_command_for_action(
    CSB_V1_UtilFlowAction action)
{
    /* ReDMCSB ENTRANCE.C utility menu resolves LOAD/NEW back into the
     * entrance command path; keep these source ids in CSB utility code so
     * M11 only executes the already resolved startup command. */
    switch (action) {
    case CSB_V1_UTIL_ACTION_LOAD:
        return CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34;
    case CSB_V1_UTIL_ACTION_NEW:
        return CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    case CSB_V1_UTIL_ACTION_IMPORT:
    case CSB_V1_UTIL_ACTION_VIEW:
    case CSB_V1_UTIL_ACTION_EXIT:
    default:
        return 0;
    }
}

int csb_v1_util_flow_handle_input(CSB_V1_UtilFlowContext *ctx,
                                  CSB_V1_UtilInput input,
                                  int preview_active,
                                  CSB_V1_UtilInputResult *out_result)
{
    if (out_result) {
        memset(out_result, 0, sizeof(*out_result));
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_NONE;
        out_result->action = CSB_V1_UTIL_ACTION_EXIT;
        out_result->selected_action_index =
            ctx ? ctx->selected_action_index : 0;
        out_result->preview_active = preview_active ? 1 : 0;
    }
    if (!ctx || !out_result) {
        return 0;
    }
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION &&
        ctx->state != CSB_V1_UTIL_FLOW_INIT) {
        return 0;
    }

    if (input == CSB_V1_UTIL_INPUT_BACK && preview_active) {
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_CLOSE_PREVIEW;
        out_result->selected_action_index = ctx->selected_action_index;
        out_result->preview_active = 0;
        return 1;
    }
    if (input == CSB_V1_UTIL_INPUT_UP ||
        input == CSB_V1_UTIL_INPUT_DOWN) {
        int delta = input == CSB_V1_UTIL_INPUT_UP ? -1 : 1;
        int selected = csb_v1_util_flow_move_action_cursor(ctx, delta);
        if (selected < 0) {
            return 0;
        }
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_CURSOR_MOVED;
        out_result->action = csb_v1_util_flow_selected_action(ctx);
        out_result->selected_action_index = selected;
        out_result->preview_active = 0;
        return 1;
    }
    if (input == CSB_V1_UTIL_INPUT_ACCEPT ||
        input == CSB_V1_UTIL_INPUT_ACTION) {
        if (csb_v1_util_flow_accept_selected_action(ctx) != 0) {
            return 0;
        }
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_ACTIVATE;
        out_result->action = ctx->action;
        out_result->selected_action_index = ctx->selected_action_index;
        out_result->preview_active = preview_active ? 1 : 0;
        return 1;
    }
    return 0;
}

int csb_v1_util_flow_menu_layout(const CSB_V1_UtilFlowContext *ctx,
                                 CSB_V1_UtilMenuLayout *out_layout)
{
    int i;

    if (!ctx || !out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    out_layout->prompt = csb_v1_util_flow_prompt(ctx);
    out_layout->x = CSB_V1_UTIL_MENU_X;
    out_layout->y = CSB_V1_UTIL_MENU_Y;
    out_layout->w = CSB_V1_UTIL_MENU_W;
    out_layout->h = CSB_V1_UTIL_MENU_H;
    out_layout->row_count = CSB_V1_UTIL_MENU_ACTION_COUNT;
    out_layout->selected_action_index = ctx->selected_action_index;

    for (i = 0; i < CSB_V1_UTIL_MENU_ACTION_COUNT; ++i) {
        CSB_V1_UtilMenuRow *row = &out_layout->rows[i];
        row->action = s_csb_v1_util_menu_actions[i];
        row->label = csb_v1_util_flow_action_label(row->action);
        row->x = CSB_V1_UTIL_MENU_X;
        row->y = CSB_V1_UTIL_MENU_Y + i * CSB_V1_UTIL_MENU_ROW_H;
        row->w = CSB_V1_UTIL_MENU_W;
        row->h = CSB_V1_UTIL_MENU_ROW_H;
        row->selected = (i == ctx->selected_action_index) ? 1 : 0;
    }

    return 1;
}

int csb_v1_util_flow_menu_render_rows(
    const CSB_V1_UtilFlowContext *ctx,
    CSB_V1_UtilRenderRow *rows,
    int max_rows)
{
    CSB_V1_UtilMenuLayout layout;
    int i;
    int count = 0;

    if (!ctx || !rows || max_rows <= 0 ||
        !csb_v1_util_flow_menu_layout(ctx, &layout)) {
        return 0;
    }
    memset(rows, 0, (size_t)max_rows * sizeof(rows[0]));
    for (i = 0; i < layout.row_count && count < max_rows; ++i) {
        const CSB_V1_UtilMenuRow *menu_row = &layout.rows[i];
        CSB_V1_UtilRenderRow *out = &rows[count];
        out->action = menu_row->action;
        out->selected = menu_row->selected;
        out->x = menu_row->x;
        out->y = menu_row->y;
        out->w = menu_row->w;
        out->h = menu_row->h;
        out->highlight_x = menu_row->x - 2;
        out->highlight_y = menu_row->y;
        out->highlight_w = menu_row->w;
        out->highlight_h = menu_row->h;
        out->text_x = menu_row->x;
        out->text_y = menu_row->y + 2;
        snprintf(out->label,
                 sizeof(out->label),
                 "%c %s",
                 menu_row->selected ? '>' : ' ',
                 menu_row->label ? menu_row->label : "");
        ++count;
    }
    return count;
}

int csb_v1_util_flow_panel_layout(const CSB_V1_UtilFlowContext *ctx,
                                  int preview_active,
                                  CSB_V1_UtilPanelLayout *out_layout)
{
    CSB_V1_UtilMenuLayout menu;
    int bottom;

    if (!ctx || !out_layout ||
        !csb_v1_util_flow_menu_layout(ctx, &menu)) {
        return 0;
    }
    memset(out_layout, 0, sizeof(*out_layout));
    out_layout->x = CSB_V1_UTIL_PANEL_X;
    out_layout->y = CSB_V1_UTIL_PANEL_Y;
    out_layout->w = CSB_V1_UTIL_PANEL_W;
    out_layout->import_status_x = CSB_V1_UTIL_PANEL_X;
    out_layout->import_status_y = CSB_V1_UTIL_PANEL_Y;
    out_layout->prompt_x = CSB_V1_UTIL_PANEL_X;
    out_layout->prompt_y = CSB_V1_UTIL_PROMPT_Y;
    out_layout->preview_x = CSB_V1_UTIL_PREVIEW_X;
    out_layout->preview_y = CSB_V1_UTIL_PREVIEW_Y;
    out_layout->preview_row_h = CSB_V1_UTIL_PREVIEW_ROW_H;
    out_layout->preview_max_rows = CSB_V1_UTIL_PREVIEW_MAX_ROWS;

    bottom = menu.y + menu.h;
    if (preview_active) {
        int preview_bottom =
            CSB_V1_UTIL_PREVIEW_Y +
            CSB_V1_UTIL_PREVIEW_ROW_H * CSB_V1_UTIL_PREVIEW_MAX_ROWS;
        if (preview_bottom > bottom) {
            bottom = preview_bottom;
        }
    }
    out_layout->h = bottom - out_layout->y;
    return 1;
}

CSB_V1_UtilFlowAction csb_v1_util_flow_action_at_point(
    const CSB_V1_UtilFlowContext *ctx,
    int x,
    int y)
{
    CSB_V1_UtilMenuLayout layout;
    int i;

    if (!csb_v1_util_flow_menu_layout(ctx, &layout)) {
        return CSB_V1_UTIL_ACTION_EXIT;
    }
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION) {
        return CSB_V1_UTIL_ACTION_EXIT;
    }
    for (i = 0; i < layout.row_count; ++i) {
        const CSB_V1_UtilMenuRow *row = &layout.rows[i];
        if (x >= row->x && x < row->x + row->w &&
            y >= row->y && y < row->y + row->h) {
            return row->action;
        }
    }
    return CSB_V1_UTIL_ACTION_EXIT;
}

int csb_v1_util_flow_handle_point(CSB_V1_UtilFlowContext *ctx,
                                  int x,
                                  int y,
                                  int preview_active,
                                  CSB_V1_UtilInputResult *out_result)
{
    CSB_V1_UtilMenuLayout layout;
    int i;

    if (out_result) {
        memset(out_result, 0, sizeof(*out_result));
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_NONE;
        out_result->action = CSB_V1_UTIL_ACTION_EXIT;
        out_result->selected_action_index =
            ctx ? ctx->selected_action_index : 0;
        out_result->preview_active = preview_active ? 1 : 0;
    }
    if (!ctx || !out_result ||
        ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION ||
        !csb_v1_util_flow_menu_layout(ctx, &layout)) {
        return 0;
    }
    for (i = 0; i < layout.row_count; ++i) {
        const CSB_V1_UtilMenuRow *row = &layout.rows[i];
        if (x >= row->x && x < row->x + row->w &&
            y >= row->y && y < row->y + row->h) {
            ctx->selected_action_index = i;
            ctx->action = row->action;
            out_result->kind = CSB_V1_UTIL_INPUT_RESULT_ACTIVATE;
            out_result->action = row->action;
            out_result->selected_action_index = i;
            out_result->preview_active = 0;
            return 1;
        }
    }
    if (csb_v1_util_flow_panel_contains_point(ctx, preview_active, x, y)) {
        out_result->kind = CSB_V1_UTIL_INPUT_RESULT_NONE;
        out_result->action = CSB_V1_UTIL_ACTION_EXIT;
        out_result->selected_action_index = ctx->selected_action_index;
        out_result->preview_active = preview_active ? 1 : 0;
        return 1;
    }
    return 0;
}

int csb_v1_util_flow_panel_contains_point(const CSB_V1_UtilFlowContext *ctx,
                                          int preview_active,
                                          int x,
                                          int y)
{
    CSB_V1_UtilPanelLayout panel;

    if (!csb_v1_util_flow_panel_layout(ctx, preview_active, &panel)) {
        return 0;
    }
    return x >= panel.x && x < panel.x + panel.w &&
           y >= panel.y && y < panel.y + panel.h;
}

/* ── Set paths ──────────────────────────────────────────────────────── */
void csb_v1_util_flow_set_dm1_path(CSB_V1_UtilFlowContext *ctx,
                                    const char *path)
{
    if (!ctx || !path) return;
    strncpy(ctx->dm1_save_path, path, sizeof(ctx->dm1_save_path) - 1);
    ctx->dm1_save_path[sizeof(ctx->dm1_save_path) - 1] = '\0';
}

void csb_v1_util_flow_set_csb_path(CSB_V1_UtilFlowContext *ctx,
                                     const char *path)
{
    if (!ctx || !path) return;
    strncpy(ctx->csb_save_path, path, sizeof(ctx->csb_save_path) - 1);
    ctx->csb_save_path[sizeof(ctx->csb_save_path) - 1] = '\0';
}

void csb_v1_util_flow_set_utility_disk_path(CSB_V1_UtilFlowContext *ctx,
                                            const char *path)
{
    if (!ctx || !path) return;
    strncpy(ctx->utility_disk_path,
            path,
            sizeof(ctx->utility_disk_path) - 1);
    ctx->utility_disk_path[sizeof(ctx->utility_disk_path) - 1] = '\0';
}

void csb_v1_util_flow_mark_utility_disk_verified(CSB_V1_UtilFlowContext *ctx,
                                                 int verified)
{
    if (!ctx) return;
    ctx->utility_disk_verified = verified ? 1 : 0;
    if (ctx->utility_disk_verified) {
        ctx->disk_result = CSB_V1_UTIL_DISK_OK;
    }
}

/* ── Confirm import ─────────────────────────────────────────────────── */
void csb_v1_util_flow_confirm_import(CSB_V1_UtilFlowContext *ctx,
                                       int confirmed)
{
    if (!ctx) return;
    ctx->import_confirmed = confirmed ? 1 : 0;
}

/* ── Ready check ─────────────────────────────────────────────────────── */
int csb_v1_util_flow_is_ready(const CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return 0;
    /* Ready if: state is DONE and action is not EXIT,
     * OR state is NEW_GAME with valid party (champion_count > 0) */
    return (ctx->state == CSB_V1_UTIL_FLOW_DONE &&
            ctx->action != CSB_V1_UTIL_ACTION_EXIT &&
            ctx->action != CSB_V1_UTIL_ACTION_IMPORT) ||
           (ctx->state == CSB_V1_UTIL_FLOW_NEW_GAME);
}

/* ── Get utility disk type (from save_load) ──────────────────────────── */
static CSB_V1_UtilDiskResult check_disk_type(const char *drive_path)
{
    int r = csb_v1_util_check_disk(drive_path);
    if (r == 0) return CSB_V1_UTIL_DISK_OK;      /* correct disk */
    if (r == 1) return CSB_V1_UTIL_DISK_WRONG;   /* wrong disk */
    return CSB_V1_UTIL_DISK_MISSING;             /* no disk / error */
}

/* ── Import champions from DM1 save ─────────────────────────────────── */
/* Import flow (ReDMCSB SAVEGAME.C F0100-F0120 state machine):
 *   1. Validate DM1 save path
 *   2. Check utility disk (CSB utility disk must be in drive)
 *   3. Read DM1 save file into buffer
 *   4. Run import state machine
 *   5. Return imported champion count
 *
 * This is called from the IMPORT_CHAMPIONS state. */
static int do_import(CSB_V1_UtilFlowContext *ctx, CSB_V1_PartyState *party)
{
    CSB_V1_ImportResult import_result;
    int count;

    if (!ctx || !party) return -1;

    /* Path must be set */
    if (ctx->dm1_save_path[0] == '\0') {
        ctx->last_error = -3;
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }

    /* Import from DM1 save file */
    memset(&import_result, 0, sizeof(import_result));
    count = csb_v1_import_from_dm1_save_file(party,
                                              ctx->dm1_save_path,
                                              &import_result);
    if (count < 0) {
        count = csb_v1_character_import_dm1_save(party, ctx->dm1_save_path);
        if (count <= 0) {
            ctx->last_error = -3;
            ctx->state = CSB_V1_UTIL_FLOW_ERROR;
            return -1;
        }
    }

    if (count == 0) {
        ctx->last_error = -3;
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }

    /* Import successful */
    ctx->last_error = 0;
    return count;
}

/* ── Main flow step ──────────────────────────────────────────────────── */
/* csb_v1_util_flow_step:
 *   Runs one step of the CSB utility flow state machine.
 *   Call repeatedly until it returns 1 (done) or negative (error).
 *
 *   ReDMCSB ENTRANCE.C flow adapted for CSB:
 *     INIT → INSERT_DISK → VERIFY_DISK → DISK_OK →
 *     SELECT_ACTION → (IMPORT_CHAMPIONS | LOAD_GAME | NEW_GAME) → DONE
 *
 *   Firestaff startup may mark the utility disk as already verified by the
 *   asset scanner, or may pass an explicit utility disk/file path to check.
 */
int csb_v1_util_flow_step(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return -1;

    switch (ctx->state) {

    case CSB_V1_UTIL_FLOW_INIT:
        /* Initialize utility flow.
         * ReDMCSB CEDTINC7.C: "PLEASE PUT THE CSB UTILITY DISK IN ~"
         * First step: prompt for disk insertion. */
        csb_v1_character_init_default(&ctx->imported_party);
        ctx->imported_champion_count = 0;
        ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        ctx->attempts = 0;
        ctx->last_error = 0;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_INSERT_DISK:
        /* Show "INSERT DISK" prompt.
         * ReDMCSB G3921: "PLEASE PUT THE CHAOS STRIKES BACK UTILITY DISK IN ~"
         * On real hardware, would wait for user to insert disk.
         * On desktop platforms, this immediately transitions to VERIFY_DISK. */
        ctx->state = CSB_V1_UTIL_FLOW_VERIFY_DISK;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_VERIFY_DISK:
        /* Check if the correct utility disk is in the drive.
         * ReDMCSB F0428: RequireGameDiskInDrive
         * F0452: GetDiskTypeInDrive_CPSB returns CSB_V1_DISK_TYPE_UTILITY_DISK.
         *
         * Modern startup usually proves the required utility/media asset
         * before this state machine runs.  In that case the UI marks the
         * disk as verified and we keep the source-visible DISK_OK state
         * transition without probing a nonexistent floppy device. */
        if (ctx->utility_disk_verified) {
            ctx->disk_result = CSB_V1_UTIL_DISK_OK;
        } else if (ctx->utility_disk_path[0] != '\0') {
            ctx->disk_result = check_disk_type(ctx->utility_disk_path);
        } else {
            ctx->disk_result = CSB_V1_UTIL_DISK_MISSING;
        }
        ctx->attempts++;

        if (ctx->disk_result == CSB_V1_UTIL_DISK_OK) {
            /* Correct disk */
            ctx->state = CSB_V1_UTIL_FLOW_DISK_OK;
        } else if (ctx->attempts >= ctx->max_attempts) {
            /* Too many attempts */
            ctx->last_error = -6;
            ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        } else if (ctx->disk_result == CSB_V1_UTIL_DISK_WRONG) {
            /* Wrong disk — ReDMCSB G3764: "THAT'S NOT THE UTILITY DISK!" */
            ctx->last_error = -2;
            /* Continue trying (show wrong disk message) */
            ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        } else {
            /* No disk — retry */
            ctx->last_error = -1;
            ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_DISK_OK:
        /* Disk verified — ReDMCSB G3755: "THAT'S THE CSB UTILITY DISK!"
         * Show success message, then transition to action selection. */
        ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_SELECT_ACTION:
        /* Main menu: choose action.
         * ReDMCSB ENTRANCE.C selector:
         *   G3922: IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE
         *   G3923: LOAD SAVED GAME
         *   G3924: START NEW GAME
         *   G3925: VIEW CHAMPION DETAILS
         *
         * In Firestaff: the UI layer calls csb_v1_util_flow_set_action()
         * to select. If action is already set, process it. */
        if (ctx->action == CSB_V1_UTIL_ACTION_EXIT) {
            ctx->state = CSB_V1_UTIL_FLOW_DONE;
            return 1;
        }

        switch (ctx->action) {
        case CSB_V1_UTIL_ACTION_IMPORT:
            ctx->state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
            break;
        case CSB_V1_UTIL_ACTION_LOAD:
            ctx->state = CSB_V1_UTIL_FLOW_LOAD_GAME;
            break;
        case CSB_V1_UTIL_ACTION_NEW:
            /* New game requires champions first (import or load) */
            if (ctx->imported_party.ChampionCount == 0) {
                ctx->last_error = -8;  /* no champions */
                ctx->state = CSB_V1_UTIL_FLOW_ERROR;
            } else {
                ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
            }
            break;
        case CSB_V1_UTIL_ACTION_VIEW:
            /* View — no state change, just continue */
            break;
        case CSB_V1_UTIL_ACTION_EXIT:
            ctx->state = CSB_V1_UTIL_FLOW_DONE;
            return 1;
        default:
            /* No action selected yet — stay in this state */
            break;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS:
        /* Import champions from DM1 save.
         * ReDMCSB SAVEGAME.C F0100-F0120 state machine.
         * If path is set, import immediately.
         * If path not set, return error (UI should set path first). */
        if (ctx->dm1_save_path[0] == '\0') {
            /* No path set — prompt for file selection.
             * In Firestaff, the UI would show a file picker here.
             * We return 0 (continue) and let the UI set the path. */
            return 0;
        }

        {
            CSB_V1_PartyState party;
            int count;
            csb_v1_character_init_default(&party);
            count = do_import(ctx, &party);
            if (count < 0) {
                /* Error already set in ctx->last_error and ctx->state */
                return 0;
            }
            ctx->imported_party = party;
            ctx->imported_champion_count = count;
            /* Import successful — show confirmation preview */
            ctx->state = CSB_V1_UTIL_FLOW_CONFIRM_IMPORT;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT:
        /* Show import preview (party champion names/stats).
         * ReDMCSB CEDTDATA.C: preview party before writing.
         *
         * In Firestaff: the UI shows the imported champions and asks
         * "IMPORT X CHAMPIONS TO PARTY SLOT?". User confirms via
         * csb_v1_util_flow_confirm_import().
         *
         * If import_confirmed is set, commit the import and go to NEW_GAME.
         * If import not confirmed, go back to SELECT_ACTION. */
        if (ctx->import_confirmed) {
            /* Commit: party already updated in IMPORT_CHAMPIONS state.
             * Transition to NEW_GAME to start playing. */
            ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
        } else {
            /* Not confirmed — back to action selection */
            ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
            ctx->action = CSB_V1_UTIL_ACTION_EXIT;  /* reset action */
        }
        ctx->import_confirmed = 0;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_LOAD_GAME:
        /* Load saved game from CSB save file.
         * ReDMCSB LOADSAVE.C F0435: STARTEND_LoadGame.
         * Uses the runtime loader so Firestaff-native, CSBWin, and bounded
         * CSBGAME roster paths all share the same byte gate.
         *
         * In Firestaff: the UI sets csb_save_path, then this state
         * loads the game state from the save file. */
        if (ctx->csb_save_path[0] == '\0') {
            /* No path set — prompt for file selection.
             * Stay in this state and let UI set path. */
            return 0;
        }

        {
            CSB_V1_RuntimeProfile loaded;
            CSB_V1_PartyState party;
            int count;

            csb_v1_runtime_init(&loaded, NULL);
            memset(&party, 0, sizeof(party));
            if (csb_v1_runtime_load_game_from_path(&loaded,
                                                   ctx->csb_save_path) !=
                CSB_V1_LOAD_OK) {
                ctx->last_error = -5;
                ctx->state = CSB_V1_UTIL_FLOW_ERROR;
                csb_v1_runtime_cleanup(&loaded);
                return 0;
            }
            count = csb_v1_runtime_get_party_state(&loaded, &party);
            if (count <= 0 || party.ChampionCount <= 0) {
                ctx->last_error = -8;
                ctx->state = CSB_V1_UTIL_FLOW_ERROR;
                csb_v1_runtime_cleanup(&loaded);
                return 0;
            }
            ctx->imported_party = party;
            ctx->imported_champion_count = count;
            ctx->last_error = 0;
            ctx->action = CSB_V1_UTIL_ACTION_LOAD;
            csb_v1_runtime_cleanup(&loaded);
            ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_NEW_GAME:
        /* New game state — party is ready.
         * The game engine would be launched here with the party.
         * ReDMCSB ENTRANCE.C: after setup, launches GAME with party.
         *
         * In Firestaff: this signals that the game is ready to start.
         * The UI transitions to the game view. */
        /* Store party metadata in ctx->reserved for get_party().
         * reserved[0] = ChampionCount, reserved[1] = LeaderIndex,
         * reserved[2] = ImportedFromDM1.
         * The full party body is kept in ctx->imported_party so M11 can
         * hand the exact utility-import result to the CSB runtime. */
        ctx->reserved[0] = ctx->imported_party.ChampionCount;
        ctx->reserved[1] = ctx->imported_party.LeaderIndex;
        ctx->reserved[2] = ctx->imported_party.ImportedFromDM1;
        ctx->state = CSB_V1_UTIL_FLOW_DONE;
        return 1;  /* done */

    case CSB_V1_UTIL_FLOW_ERROR:
        /* Error state — something went wrong.
         * In Firestaff: the UI shows the error message and
         * offers to retry or exit. */
        return -1;  /* error */

    case CSB_V1_UTIL_FLOW_DONE:
        /* Flow completed successfully */
        return 1;

    default:
        ctx->last_error = -7;  /* invalid state */
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }
}

/* ── Get imported party (for external access) ────────────────────────── */
/* csb_v1_util_flow_get_party:
 *   Returns the imported/loaded party state.
 *   Call this after flow is done (returns 1) to get the party for the game.
 *
 *   Implementation: step() keeps the full imported party body in
 *   ctx->imported_party before transitioning to DONE.  The reserved[]
 *   metadata remains as a compatibility fallback for older callers:
 *     reserved[0] = ChampionCount
 *     reserved[1] = LeaderIndex
 *     reserved[2] = ImportedFromDM1
 *
 *   Returns: champion count (>= 0) on success, -1 on error.
 *   On success, out_party receives the imported champion records plus
 *   ChampionCount / LeaderIndex / ImportedFromDM1.
 */
int csb_v1_util_flow_get_party(CSB_V1_UtilFlowContext *ctx,
                                CSB_V1_PartyState *out_party)
{
    if (!ctx) return -1;
    if (!out_party) return -1;

    memset(out_party, 0, sizeof(*out_party));

    if (ctx->state != CSB_V1_UTIL_FLOW_DONE &&
        ctx->state != CSB_V1_UTIL_FLOW_NEW_GAME) {
        return -1;  /* flow not complete */
    }

    *out_party = ctx->imported_party;
    if (out_party->ChampionCount == 0 && ctx->reserved[0] > 0) {
        out_party->ChampionCount = ctx->reserved[0];
        out_party->LeaderIndex = ctx->reserved[1];
        out_party->ImportedFromDM1 = ctx->reserved[2];
    }

    return out_party->ChampionCount;
}
