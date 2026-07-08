#include "nexus_v1_startup_menu.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_title_sequence.h"

#include <stdio.h>
#include <string.h>

static int nexus_v1_startup_row_count(unsigned int slot_mask)
{
    int row_count = 1; /* NEW GAME */
    int slot;

    for (slot = 0; slot < NEXUS_SAVE_MAX_SLOTS; ++slot) {
        if (slot_mask & (1u << slot)) {
            ++row_count;
        }
    }
    return row_count;
}

static unsigned int nexus_v1_startup_slot_mask_clamp(unsigned int slot_mask)
{
    unsigned int mask = 0u;
    int slot;

    for (slot = 0; slot < NEXUS_SAVE_MAX_SLOTS; ++slot) {
        mask |= (1u << slot);
    }
    return slot_mask & mask;
}

static void nexus_v1_startup_action_clear(Nexus_V1_StartupAction *action)
{
    if (!action) {
        return;
    }
    memset(action, 0, sizeof(*action));
    action->kind = NEXUS_V1_STARTUP_ACTION_NONE;
    action->row = -1;
    action->slot = -1;
}

static void nexus_v1_startup_menu_from_snapshot(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupMenu *menu)
{
    if (!snapshot || !menu) {
        return;
    }
    nexus_v1_startup_menu_init(menu,
                               snapshot->save_dir[0]
                                   ? snapshot->save_dir
                                   : NULL);
    menu->selected_row = snapshot->selected_row;
    (void)nexus_v1_startup_menu_refresh(menu, snapshot->slot_mask);
}

static void nexus_v1_startup_menu_to_snapshot(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupMenuSnapshot *snapshot)
{
    if (!menu || !snapshot) {
        return;
    }
    snprintf(snapshot->save_dir,
             sizeof(snapshot->save_dir),
             "%s",
             menu->save_dir);
    snapshot->slot_mask = menu->slot_mask;
    snapshot->row_count = menu->row_count;
    snapshot->selected_row = menu->selected_row;
}

static void nexus_v1_startup_save_execution_clear(
    Nexus_V1_StartupSaveExecution *execution)
{
    if (!execution) {
        return;
    }
    memset(execution, 0, sizeof(*execution));
    execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE;
}

static void nexus_v1_startup_title_execution_clear(
    Nexus_V1_StartupTitleExecution *execution)
{
    if (!execution) {
        return;
    }
    memset(execution, 0, sizeof(*execution));
    execution->kind = NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE;
}

static void nexus_v1_startup_champion_execution_clear(
    Nexus_V1_StartupChampionExecution *execution)
{
    if (!execution) {
        return;
    }
    memset(execution, 0, sizeof(*execution));
    execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE;
    execution->cursor = -1;
}

static void nexus_v1_startup_mode_update_clear(
    Nexus_V1_StartupModeUpdate *update)
{
    if (!update) {
        return;
    }
    memset(update, 0, sizeof(*update));
    update->save_selected_row = -1;
    update->champion_cursor = -1;
}

static void nexus_v1_startup_apply_receipt_clear(
    Nexus_V1_StartupApplyReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = NEXUS_V1_STARTUP_APPLY_RESULT_IGNORE;
}

void nexus_v1_startup_host_receipt_clear(
    Nexus_V1_StartupHostReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = NEXUS_V1_STARTUP_HOST_INPUT_IGNORED;
}

int nexus_v1_startup_boot_status_host_receipt(
    Nexus_V1_StartupBootStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    nexus_v1_startup_host_receipt_clear(out_receipt);
    out_receipt->status_scope = "BOOT";
    switch (status) {
    case NEXUS_V1_STARTUP_BOOT_STATUS_TITLE:
        out_receipt->status = "NEXUS TITLE";
        out_receipt->input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        break;
    case NEXUS_V1_STARTUP_BOOT_STATUS_TITLE_FALLBACK:
        out_receipt->status = "NEXUS TITLE";
        out_receipt->input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        break;
    case NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR:
        out_receipt->status = "NEXUS DATA ERROR";
        break;
    case NEXUS_V1_STARTUP_BOOT_STATUS_LEVEL_ERROR:
        out_receipt->status = "NEXUS LEVEL ERROR";
        break;
    default:
        out_receipt->status = "NEXUS STARTUP FAILED";
        break;
    }
    return 1;
}

int nexus_v1_startup_resume_status_host_receipt(
    Nexus_V1_StartupResumeStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    nexus_v1_startup_host_receipt_clear(out_receipt);
    out_receipt->status_scope = "BOOT";
    switch (status) {
    case NEXUS_V1_STARTUP_RESUME_STATUS_FAILED:
        out_receipt->status = "NEXUS RESUME FAILED";
        break;
    case NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_INVALID:
        out_receipt->status = "NEXUS RESUME LEVEL INVALID";
        break;
    case NEXUS_V1_STARTUP_RESUME_STATUS_DIR_INVALID:
        out_receipt->status = "NEXUS RESUME DIR INVALID";
        break;
    case NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_ERROR:
        out_receipt->status = "NEXUS RESUME LEVEL ERROR";
        break;
    case NEXUS_V1_STARTUP_RESUME_STATUS_ENGINE_LOST:
        out_receipt->status = "NEXUS RESUME ENGINE LOST";
        break;
    case NEXUS_V1_STARTUP_RESUME_STATUS_RESUMED:
        out_receipt->status = "NEXUS RESUMED";
        out_receipt->input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        break;
    default:
        out_receipt->status = "NEXUS RESUME FAILED";
        break;
    }
    return 1;
}

void nexus_v1_startup_host_action_receipt_clear(
    Nexus_V1_StartupHostActionReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_menu_state_receipt_init(
        &receipt->save_state_receipt);
    nexus_v1_startup_champion_state_receipt_init(
        &receipt->champion_state_receipt);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_startup_idle_receipt_clear(
    Nexus_V1_StartupIdleReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_startup_launch_receipt_clear(
    Nexus_V1_StartupLaunchReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_menu_state_receipt_init(
        &receipt->save_state_receipt);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

int nexus_v1_startup_launch_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupLaunchReceipt *out_receipt)
{
    if (out_receipt) {
        nexus_v1_startup_launch_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        if (out_receipt) {
            out_receipt->host_receipt.status_scope = "BOOT";
            out_receipt->host_receipt.status = "NEXUS STARTUP FAILED";
        }
        return 0;
    }
    if (nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_host_facts(
            &out_receipt->save_state_receipt,
            facts)) {
        out_receipt->save_state_receipt_valid = 1;
    }
    out_receipt->host_receipt.mode_update.set_title_active = 1;
    out_receipt->host_receipt.mode_update.title_active = 1;
    out_receipt->host_receipt.mode_update.set_title_frame = 1;
    out_receipt->host_receipt.mode_update.title_frame = 0;
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "BOOT";
    out_receipt->host_receipt.status = "NEXUS TITLE";
    return 1;
}

int nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupIdleReceipt *out_receipt)
{
    if (out_receipt) {
        nexus_v1_startup_idle_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        return 0;
    }
    if (facts->title_active) {
        out_receipt->host_receipt.mode_update.set_title_frame = 1;
        out_receipt->host_receipt.mode_update.title_frame =
            facts->title_frame + 1;
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = "STARTUP";
        out_receipt->host_receipt.status = "NEXUS TITLE";
        return 1;
    }
    if (facts->save_select_active) {
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_IGNORED;
        return 1;
    }
    if (facts->champion_select_active) {
        out_receipt->host_receipt.mode_update.set_champion_frame = 1;
        out_receipt->host_receipt.mode_update.champion_frame =
            facts->champion_frame + 1;
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = "STARTUP";
        out_receipt->host_receipt.status = "NEXUS CHAMPIONS";
        return 1;
    }
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_IGNORED;
    return 1;
}

int nexus_v1_startup_host_receipt_from_apply_receipt(
    const Nexus_V1_StartupApplyReceipt *apply_receipt,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    if (!apply_receipt || !out_receipt) {
        return 0;
    }
    nexus_v1_startup_host_receipt_clear(out_receipt);
    out_receipt->mode_update = apply_receipt->mode_update;
    out_receipt->status_scope = apply_receipt->status_scope;
    out_receipt->status = apply_receipt->status;
    switch (apply_receipt->result) {
        case NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW:
            out_receipt->input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
            break;
        case NEXUS_V1_STARTUP_APPLY_RESULT_RETURN_TO_LAUNCHER:
            out_receipt->input_result =
                NEXUS_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER;
            break;
        case NEXUS_V1_STARTUP_APPLY_RESULT_IGNORE:
        default:
            out_receipt->input_result =
                NEXUS_V1_STARTUP_HOST_INPUT_IGNORED;
            break;
    }
    return 1;
}

void nexus_v1_startup_menu_state_receipt_init(
    Nexus_V1_StartupMenuStateReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

void nexus_v1_startup_champion_state_receipt_init(
    Nexus_V1_StartupChampionStateReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

static void nexus_v1_startup_draw_clear(
    Nexus_V1_StartupDrawCommand *command)
{
    if (!command) {
        return;
    }
    memset(command, 0, sizeof(*command));
    command->portrait_index = -1;
    command->text_style = NEXUS_V1_STARTUP_TEXT_SMALL;
}

static int nexus_v1_startup_push_draw(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count,
    const Nexus_V1_StartupDrawCommand *src)
{
    if (!commands || !count || !src || *count < 0 ||
        *count >= max_commands) {
        return 0;
    }
    commands[*count] = *src;
    ++(*count);
    return 1;
}

static int nexus_v1_startup_push_background(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count)
{
    Nexus_V1_StartupDrawCommand command;
    nexus_v1_startup_draw_clear(&command);
    command.kind = NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND;
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

static int nexus_v1_startup_push_rect(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count,
    Nexus_V1_StartupDrawKind kind,
    const Nexus_V1_StartupRect *rect,
    int color)
{
    Nexus_V1_StartupDrawCommand command;
    if (!rect) {
        return 0;
    }
    nexus_v1_startup_draw_clear(&command);
    command.kind = kind;
    command.rect = *rect;
    command.text_color = color;
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

static int nexus_v1_startup_push_text(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count,
    int x,
    int y,
    const char *text,
    Nexus_V1_StartupTextStyle style,
    int text_color,
    int shadow_color)
{
    Nexus_V1_StartupDrawCommand command;
    if (!text || !text[0]) {
        return 1;
    }
    nexus_v1_startup_draw_clear(&command);
    command.kind = NEXUS_V1_STARTUP_DRAW_TEXT;
    command.x = x;
    command.y = y;
    command.text_style = style;
    command.text_color = text_color;
    command.shadow_color = shadow_color;
    snprintf(command.label, sizeof(command.label), "%s", text);
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

static int nexus_v1_startup_push_portrait(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count,
    const Nexus_V1_StartupChampionRenderRow *row)
{
    Nexus_V1_StartupDrawCommand command;
    if (!row || row->portrait_index < 0) {
        return 1;
    }
    nexus_v1_startup_draw_clear(&command);
    command.kind = NEXUS_V1_STARTUP_DRAW_PORTRAIT;
    command.rect.x = row->portrait_x;
    command.rect.y = row->portrait_y;
    command.rect.w = row->portrait_w;
    command.rect.h = row->portrait_h;
    command.portrait_index = row->portrait_index;
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

static int nexus_v1_startup_push_boot_title_frame(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count,
    int title_frame)
{
    Nexus_V1_StartupDrawCommand command;
    nexus_v1_startup_draw_clear(&command);
    command.kind = NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME;
    command.title_frame = title_frame < 0 ? 0 : title_frame;
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

static int nexus_v1_startup_push_warning_background(
    Nexus_V1_StartupDrawCommand *commands,
    int max_commands,
    int *count)
{
    Nexus_V1_StartupDrawCommand command;
    nexus_v1_startup_draw_clear(&command);
    command.kind = NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND;
    return nexus_v1_startup_push_draw(commands, max_commands, count, &command);
}

Nexus_V1_StartupInput nexus_v1_startup_input_from_firestaff_menu_code(
    int menu_input)
{
    enum {
        FIRESTAFF_MENU_INPUT_NONE = 0,
        FIRESTAFF_MENU_INPUT_UP = 1,
        FIRESTAFF_MENU_INPUT_DOWN = 2,
        FIRESTAFF_MENU_INPUT_ACCEPT = 9,
        FIRESTAFF_MENU_INPUT_BACK = 10,
        FIRESTAFF_MENU_INPUT_ACTION = 11
    };

    switch (menu_input) {
        case FIRESTAFF_MENU_INPUT_UP:
            return NEXUS_V1_STARTUP_INPUT_UP;
        case FIRESTAFF_MENU_INPUT_DOWN:
            return NEXUS_V1_STARTUP_INPUT_DOWN;
        case FIRESTAFF_MENU_INPUT_ACCEPT:
            return NEXUS_V1_STARTUP_INPUT_ACCEPT;
        case FIRESTAFF_MENU_INPUT_ACTION:
            return NEXUS_V1_STARTUP_INPUT_ACTION;
        case FIRESTAFF_MENU_INPUT_BACK:
            return NEXUS_V1_STARTUP_INPUT_BACK;
        case FIRESTAFF_MENU_INPUT_NONE:
        default:
            return NEXUS_V1_STARTUP_INPUT_NONE;
    }
}

void nexus_v1_startup_menu_init(Nexus_V1_StartupMenu *menu,
                                const char *save_dir)
{
    if (!menu) {
        return;
    }
    memset(menu, 0, sizeof(*menu));
    if (save_dir && save_dir[0]) {
        snprintf(menu->save_dir, sizeof(menu->save_dir), "%s", save_dir);
    } else {
        nexus_v1_save_default_dir(menu->save_dir, sizeof(menu->save_dir));
    }
    menu->row_count = 1;
    menu->selected_row = -1;
}

int nexus_v1_startup_menu_refresh(Nexus_V1_StartupMenu *menu,
                                  unsigned int slot_mask)
{
    if (!menu) {
        return 0;
    }
    menu->slot_mask = nexus_v1_startup_slot_mask_clamp(slot_mask);
    menu->row_count = nexus_v1_startup_row_count(menu->slot_mask);
    if (menu->row_count < 1) {
        menu->row_count = 1;
    }
    if (menu->selected_row < 0) {
        menu->selected_row = menu->row_count - 1;
    }
    if (menu->selected_row >= menu->row_count) {
        menu->selected_row = menu->row_count - 1;
    }
    return 1;
}

int nexus_v1_startup_menu_scan(Nexus_V1_StartupMenu *menu)
{
    Nexus_V1_SaveManager mgr;
    int slot;

    if (!menu) {
        return -1;
    }
    nexus_v1_save_init(&mgr,
                       menu->save_dir[0] ? menu->save_dir : NULL);
    if (nexus_v1_save_scan(&mgr) != 0) {
        return -1;
    }
    menu->slot_mask = 0u;
    memset(menu->slots, 0, sizeof(menu->slots));
    for (slot = 0; slot < NEXUS_SAVE_MAX_SLOTS; ++slot) {
        const Nexus_V1_SaveSlot *save_slot =
            nexus_v1_save_get_slot(&mgr, (uint8_t)slot);
        if (!save_slot || !save_slot->occupied) {
            continue;
        }
        menu->slot_mask |= (1u << slot);
        menu->slots[slot] = *save_slot;
    }
    (void)nexus_v1_startup_menu_refresh(menu, menu->slot_mask);
    return 0;
}

int nexus_v1_startup_menu_scan_or_new_game(Nexus_V1_StartupMenu *menu)
{
    if (!menu) {
        return 0;
    }
    if (nexus_v1_startup_menu_scan(menu) == 0) {
        return 1;
    }
    menu->slot_mask = 0u;
    memset(menu->slots, 0, sizeof(menu->slots));
    return nexus_v1_startup_menu_refresh(menu, 0u);
}

int nexus_v1_startup_menu_row_at(const Nexus_V1_StartupMenu *menu,
                                 int row,
                                 Nexus_V1_StartupRowKind *out_kind,
                                 int *out_slot)
{
    int cursor = 0;
    int slot;

    if (out_kind) {
        *out_kind = NEXUS_V1_STARTUP_ROW_NONE;
    }
    if (out_slot) {
        *out_slot = -1;
    }
    if (!menu || row < 0) {
        return 0;
    }
    for (slot = 0; slot < NEXUS_SAVE_MAX_SLOTS; ++slot) {
        if ((menu->slot_mask & (1u << slot)) == 0u) {
            continue;
        }
        if (row == cursor) {
            if (out_kind) {
                *out_kind = NEXUS_V1_STARTUP_ROW_SLOT;
            }
            if (out_slot) {
                *out_slot = slot;
            }
            return 1;
        }
        ++cursor;
    }
    if (row == cursor) {
        if (out_kind) {
            *out_kind = NEXUS_V1_STARTUP_ROW_NEW_GAME;
        }
        return 1;
    }
    return 0;
}

int nexus_v1_startup_menu_selected_path(const Nexus_V1_StartupMenu *menu,
                                        char *out_path,
                                        size_t out_path_size)
{
    Nexus_V1_StartupRowKind kind = NEXUS_V1_STARTUP_ROW_NONE;
    int slot = -1;

    if (!menu || !out_path || out_path_size == 0u) {
        return 0;
    }
    out_path[0] = '\0';
    if (!nexus_v1_startup_menu_row_at(menu,
                                      menu->selected_row,
                                      &kind,
                                      &slot) ||
        kind != NEXUS_V1_STARTUP_ROW_SLOT ||
        slot < 0 || slot >= NEXUS_SAVE_MAX_SLOTS) {
        return 0;
    }
    snprintf(out_path, out_path_size, "%s/nexus_save_%02d.dat",
             menu->save_dir[0] ? menu->save_dir : ".",
             slot);
    return 1;
}

int nexus_v1_startup_menu_move_selected(Nexus_V1_StartupMenu *menu,
                                        int delta)
{
    int next;

    if (!menu || menu->row_count <= 0) {
        return 0;
    }
    next = menu->selected_row + delta;
    if (next < 0) {
        next = 0;
    }
    if (next >= menu->row_count) {
        next = menu->row_count - 1;
    }
    menu->selected_row = next;
    return 1;
}

int nexus_v1_startup_menu_activate_selected(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupRowKind row_kind = NEXUS_V1_STARTUP_ROW_NONE;
    int slot = -1;

    nexus_v1_startup_action_clear(out_action);
    if (!menu || !out_action ||
        !nexus_v1_startup_menu_row_at(menu,
                                      menu->selected_row,
                                      &row_kind,
                                      &slot)) {
        return 0;
    }
    out_action->row = menu->selected_row;
    out_action->slot = slot;
    if (row_kind == NEXUS_V1_STARTUP_ROW_SLOT) {
        if (!nexus_v1_startup_menu_selected_path(menu,
                                                 out_action->path,
                                                 sizeof(out_action->path))) {
            nexus_v1_startup_action_clear(out_action);
            return 0;
        }
        out_action->kind = NEXUS_V1_STARTUP_ACTION_LOAD_SLOT;
        return 1;
    }
    if (row_kind == NEXUS_V1_STARTUP_ROW_NEW_GAME) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_NEW_GAME;
        return 1;
    }
    return 0;
}

int nexus_v1_startup_title_handle_input(int title_frame,
                                        unsigned int slot_mask,
                                        Nexus_V1_StartupInput input,
                                        Nexus_V1_StartupAction *out_action)
{
    nexus_v1_startup_action_clear(out_action);
    if (!out_action) {
        return 0;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_BACK) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_NONE) {
        return 0;
    }
    if (input != NEXUS_V1_STARTUP_INPUT_ACCEPT &&
        input != NEXUS_V1_STARTUP_INPUT_ACTION) {
        return 0;
    }
    {
        Nexus_V1_TitleFrame frame_state;
        if (!nexus_v1_title_frame(title_frame, NEXUS_FB_H, &frame_state) ||
            frame_state.phase != NEXUS_V1_TITLE_PHASE_START_READY) {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_HOLD_TITLE;
            return 1;
        }
    }
    out_action->kind = slot_mask != 0u
                           ? NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT
                           : NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT;
    return 1;
}

int nexus_v1_startup_boot_handle_input(int boot_frame,
                                       unsigned int slot_mask,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action)
{
    nexus_v1_startup_action_clear(out_action);
    if (!out_action) {
        return 0;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_BACK) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_NONE) {
        return 0;
    }
    if (input != NEXUS_V1_STARTUP_INPUT_ACCEPT &&
        input != NEXUS_V1_STARTUP_INPUT_ACTION) {
        return 0;
    }
    {
        Nexus_V1_BootFrame frame_state;
        if (!nexus_v1_boot_frame(boot_frame, NEXUS_FB_H, &frame_state) ||
            !frame_state.start_ready) {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_HOLD_TITLE;
            return 1;
        }
    }
    out_action->kind = slot_mask != 0u
                           ? NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT
                           : NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT;
    return 1;
}

int nexus_v1_startup_title_handle_hit(int title_frame,
                                      unsigned int slot_mask,
                                      Nexus_V1_StartupAction *out_action)
{
    return nexus_v1_startup_title_handle_input(
        title_frame,
        slot_mask,
        NEXUS_V1_STARTUP_INPUT_ACCEPT,
        out_action);
}

int nexus_v1_startup_menu_handle_input(Nexus_V1_StartupMenu *menu,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action)
{
    nexus_v1_startup_action_clear(out_action);
    if (!menu || !out_action) {
        return 0;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_UP) {
        (void)nexus_v1_startup_menu_move_selected(menu, -1);
        out_action->kind = NEXUS_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_DOWN) {
        (void)nexus_v1_startup_menu_move_selected(menu, 1);
        out_action->kind = NEXUS_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_ACCEPT ||
        input == NEXUS_V1_STARTUP_INPUT_ACTION) {
        return nexus_v1_startup_menu_activate_selected(menu, out_action);
    }
    if (input == NEXUS_V1_STARTUP_INPUT_BACK) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE;
        return 1;
    }
    return 0;
}

int nexus_v1_startup_menu_handle_hit(Nexus_V1_StartupMenu *menu,
                                     const Nexus_V1_StartupHit *hit,
                                     Nexus_V1_StartupAction *out_action)
{
    nexus_v1_startup_action_clear(out_action);
    if (!menu || !hit || !out_action) {
        return 0;
    }
    if (hit->kind == NEXUS_V1_STARTUP_HIT_SAVE_PANEL) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (hit->kind != NEXUS_V1_STARTUP_HIT_SAVE_ROW ||
        hit->row < 0 || hit->row >= menu->row_count) {
        return 0;
    }
    menu->selected_row = hit->row;
    return nexus_v1_startup_menu_activate_selected(menu, out_action);
}

int nexus_v1_startup_menu_snapshot_refresh(
    Nexus_V1_StartupMenuSnapshot *snapshot)
{
    Nexus_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    nexus_v1_startup_menu_from_snapshot(snapshot, &menu);
    nexus_v1_startup_menu_to_snapshot(&menu, snapshot);
    return 1;
}

int nexus_v1_startup_menu_snapshot_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row)
{
    Nexus_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    nexus_v1_startup_menu_init(&menu, save_dir);
    menu.selected_row = selected_row;
    if (!nexus_v1_startup_menu_refresh(&menu, slot_mask)) {
        return 0;
    }
    nexus_v1_startup_menu_to_snapshot(&menu, snapshot);
    return 1;
}

int nexus_v1_startup_menu_snapshot_scan_or_new_game_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    int selected_row)
{
    Nexus_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    nexus_v1_startup_menu_init(&menu, save_dir);
    menu.selected_row = selected_row;
    if (!nexus_v1_startup_menu_scan_or_new_game(&menu)) {
        return 0;
    }
    nexus_v1_startup_menu_to_snapshot(&menu, snapshot);
    return 1;
}

int nexus_v1_startup_menu_state_receipt_from_snapshot(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupMenuStateReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    nexus_v1_startup_menu_state_receipt_init(out_receipt);
    if (!snapshot) {
        return 0;
    }
    snprintf(out_receipt->save_dir,
             sizeof(out_receipt->save_dir),
             "%s",
             snapshot->save_dir);
    out_receipt->slot_mask = nexus_v1_startup_slot_mask_clamp(
        snapshot->slot_mask);
    out_receipt->row_count = snapshot->row_count < 1 ? 1 : snapshot->row_count;
    out_receipt->selected_row = snapshot->selected_row;
    if (out_receipt->selected_row < 0) {
        out_receipt->selected_row = 0;
    }
    if (out_receipt->selected_row >= out_receipt->row_count) {
        out_receipt->selected_row = out_receipt->row_count - 1;
    }
    return 1;
}

int nexus_v1_startup_menu_state_receipt_from_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row)
{
    Nexus_V1_StartupMenuSnapshot snapshot;

    if (!nexus_v1_startup_menu_snapshot_from_facts(&snapshot,
                                                  save_dir,
                                                  slot_mask,
                                                  selected_row)) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                             out_receipt);
}

int nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    int selected_row)
{
    Nexus_V1_StartupMenuSnapshot snapshot;

    if (!nexus_v1_startup_menu_snapshot_scan_or_new_game_from_facts(
            &snapshot,
            save_dir,
            selected_row)) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                             out_receipt);
}

int nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_host_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts)
{
    if (!facts) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_facts(
        out_receipt,
        facts->save_dir,
        facts->save_selected_row);
}

int nexus_v1_startup_menu_snapshot_row_at(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    int row,
    Nexus_V1_StartupRowKind *out_kind,
    int *out_slot)
{
    Nexus_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    nexus_v1_startup_menu_from_snapshot(snapshot, &menu);
    return nexus_v1_startup_menu_row_at(&menu, row, out_kind, out_slot);
}

int nexus_v1_startup_menu_snapshot_handle_input(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupInput input,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupMenu menu;
    int handled;

    if (!snapshot) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    nexus_v1_startup_menu_from_snapshot(snapshot, &menu);
    handled = nexus_v1_startup_menu_handle_input(&menu, input, out_action);
    nexus_v1_startup_menu_to_snapshot(&menu, snapshot);
    return handled;
}

int nexus_v1_startup_menu_handle_firestaff_input_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    Nexus_V1_StartupAction *out_action)
{
    if (!nexus_v1_startup_menu_snapshot_from_facts(snapshot,
                                                  save_dir,
                                                  slot_mask,
                                                  selected_row)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_menu_snapshot_handle_input(
        snapshot,
        nexus_v1_startup_input_from_firestaff_menu_code(menu_input),
        out_action);
}

int nexus_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupMenuSnapshot snapshot;

    if (!nexus_v1_startup_menu_handle_firestaff_input_from_facts(
            &snapshot,
            save_dir,
            slot_mask,
            selected_row,
            menu_input,
            out_action)) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                             out_receipt);
}

int nexus_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupAction *out_action)
{
    if (!facts) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
        out_receipt,
        facts->save_dir,
        facts->slot_mask,
        facts->save_selected_row,
        menu_input,
        out_action);
}

int nexus_v1_startup_menu_snapshot_handle_hit(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const Nexus_V1_StartupHit *hit,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupMenu menu;
    int handled;

    if (!snapshot) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    nexus_v1_startup_menu_from_snapshot(snapshot, &menu);
    handled = nexus_v1_startup_menu_handle_hit(&menu, hit, out_action);
    nexus_v1_startup_menu_to_snapshot(&menu, snapshot);
    return handled;
}

int nexus_v1_startup_menu_handle_pointer_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int row_count,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupHit hit;

    if (!nexus_v1_startup_save_hit(row_count, x, y, &hit)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    if (!nexus_v1_startup_menu_snapshot_from_facts(snapshot,
                                                  save_dir,
                                                  slot_mask,
                                                  selected_row)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_menu_snapshot_handle_hit(snapshot,
                                                    &hit,
                                                    out_action);
}

int nexus_v1_startup_menu_handle_pointer_from_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int row_count,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupMenuSnapshot snapshot;

    if (!nexus_v1_startup_menu_handle_pointer_from_facts(&snapshot,
                                                        save_dir,
                                                        slot_mask,
                                                        selected_row,
                                                        row_count,
                                                        x,
                                                        y,
                                                        out_action)) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                             out_receipt);
}

int nexus_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    if (!facts) {
        nexus_v1_startup_menu_state_receipt_init(out_receipt);
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_menu_handle_pointer_from_facts_with_receipt(
        out_receipt,
        facts->save_dir,
        facts->slot_mask,
        facts->save_selected_row,
        facts->save_row_count,
        x,
        y,
        out_action);
}

int nexus_v1_startup_execute_save_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupSaveExecution *out_execution)
{
    if (!out_execution) {
        return 0;
    }
    nexus_v1_startup_save_execution_clear(out_execution);
    if (!action) {
        return 0;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_NONE) {
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_STATUS_REDRAW;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS SAVE SELECT";
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
        action->path[0] != '\0') {
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT;
        out_execution->status_scope = "BOOT";
        out_execution->status = "NEXUS RESUMED";
        out_execution->failure_status = "NEXUS LOAD FAILED";
        snprintf(out_execution->path,
                 sizeof(out_execution->path),
                 "%s",
                 action->path);
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_NEW_GAME) {
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_CHAMPIONS;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS CHAMPIONS";
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE) {
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_TITLE;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS TITLE";
        return 1;
    }
    return 0;
}

int nexus_v1_startup_execute_title_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution)
{
    if (!out_execution) {
        return 0;
    }
    nexus_v1_startup_title_execution_clear(out_execution);
    if (!action) {
        return 0;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER) {
        out_execution->kind =
            NEXUS_V1_STARTUP_TITLE_EXEC_RETURN_TO_LAUNCHER;
        out_execution->status_scope = "RETURN";
        out_execution->status = "BACK TO LAUNCHER";
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE) {
        out_execution->kind = NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS TITLE";
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT) {
        out_execution->kind =
            NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS LOAD GAME";
        return 1;
    }
    if (action->kind == NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT) {
        out_execution->kind =
            NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_CHAMPIONS;
        out_execution->status_scope = "STARTUP";
        out_execution->status = "NEXUS CHAMPIONS";
        return 1;
    }
    return 0;
}

int nexus_v1_startup_title_execution_mode_update(
    const Nexus_V1_StartupTitleExecution *execution,
    Nexus_V1_StartupModeUpdate *out_update)
{
    if (!execution || !out_update ||
        execution->kind == NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE) {
        return 0;
    }
    nexus_v1_startup_mode_update_clear(out_update);
    switch (execution->kind) {
        case NEXUS_V1_STARTUP_TITLE_EXEC_RETURN_TO_LAUNCHER:
        case NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE:
            return 1;
        case NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT:
            out_update->set_title_active = 1;
            out_update->title_active = 0;
            out_update->set_title_frame = 1;
            out_update->title_frame = 0;
            out_update->set_save_select_active = 1;
            out_update->save_select_active = 1;
            out_update->set_save_selected_row = 1;
            out_update->save_selected_row = 0;
            return 1;
        case NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_CHAMPIONS:
            out_update->set_title_active = 1;
            out_update->title_active = 0;
            out_update->set_title_frame = 1;
            out_update->title_frame = 0;
            out_update->set_champion_select_active = 1;
            out_update->champion_select_active = 1;
            out_update->set_champion_cursor = 1;
            out_update->champion_cursor = 0;
            out_update->set_champion_frame = 1;
            out_update->champion_frame = 0;
            return 1;
        default:
            break;
    }
    return 0;
}

int nexus_v1_startup_execute_champion_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupChampionExecution *out_execution)
{
    if (!out_execution) {
        return 0;
    }
    nexus_v1_startup_champion_execution_clear(out_execution);
    if (!action) {
        return 0;
    }
    out_execution->status_scope = "STARTUP";
    switch (action->kind) {
        case NEXUS_V1_STARTUP_ACTION_NONE:
            out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW;
            out_execution->status = "NEXUS CHAMPIONS";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR:
            out_execution->kind =
                NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR;
            out_execution->cursor = action->row;
            out_execution->status = "NEXUS CHAMPION CURSOR";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED:
            out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW;
            out_execution->status = "NEXUS CHAMPION ADDED";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_CHAMPION_SKIPPED:
            out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW;
            out_execution->status = "NEXUS CHAMPION SKIPPED";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED:
            out_execution->kind =
                NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR;
            out_execution->cursor = action->row;
            out_execution->status = "NEXUS CHAMPION REMOVED";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION:
            out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW;
            out_execution->status = "NEXUS NEEDS CHAMPION";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_START_DUNGEON:
            out_execution->kind =
                NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON;
            out_execution->status_scope = "BOOT";
            out_execution->status = "NEXUS READY";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT:
            out_execution->kind =
                NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_SAVE_SELECT;
            out_execution->select_last_save_row = 1;
            out_execution->status = "NEXUS LOAD GAME";
            return 1;
        case NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE:
            out_execution->kind =
                NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_TITLE;
            out_execution->status = "NEXUS TITLE";
            return 1;
        default:
            break;
    }
    return 0;
}

int nexus_v1_startup_save_execution_mode_update(
    const Nexus_V1_StartupSaveExecution *execution,
    Nexus_V1_StartupModeUpdate *out_update)
{
    if (!execution || !out_update ||
        execution->kind == NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE) {
        return 0;
    }
    nexus_v1_startup_mode_update_clear(out_update);
    switch (execution->kind) {
        case NEXUS_V1_STARTUP_SAVE_EXEC_STATUS_REDRAW:
            return 1;
        case NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT:
            out_update->set_save_select_active = 1;
            out_update->save_select_active = 0;
            return 1;
        case NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_CHAMPIONS:
            out_update->set_save_select_active = 1;
            out_update->save_select_active = 0;
            out_update->set_champion_select_active = 1;
            out_update->champion_select_active = 1;
            out_update->set_champion_cursor = 1;
            out_update->champion_cursor = 0;
            out_update->set_champion_frame = 1;
            out_update->champion_frame = 0;
            return 1;
        case NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_TITLE:
            out_update->set_save_select_active = 1;
            out_update->save_select_active = 0;
            out_update->set_title_active = 1;
            out_update->title_active = 1;
            out_update->set_title_frame = 1;
            out_update->title_frame = 0;
            return 1;
        default:
            break;
    }
    return 0;
}

int nexus_v1_startup_apply_receipt_from_save_execution(
    const Nexus_V1_StartupSaveExecution *execution,
    int load_success,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupModeUpdate update;

    if (!execution || !out_receipt ||
        execution->kind == NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE) {
        return 0;
    }
    nexus_v1_startup_apply_receipt_clear(out_receipt);
    if (execution->kind == NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT &&
        !load_success) {
        out_receipt->result = NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = execution->failure_status
                                  ? execution->failure_status
                                  : "NEXUS LOAD FAILED";
        return 1;
    }
    if (!nexus_v1_startup_save_execution_mode_update(execution, &update)) {
        return 0;
    }
    out_receipt->mode_update = update;
    out_receipt->result = NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW;
    out_receipt->status_scope = execution->status_scope
                                    ? execution->status_scope
                                    : "STARTUP";
    out_receipt->status = execution->status ? execution->status
                                            : "NEXUS SAVE SELECT";
    if (execution->kind == NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT) {
        out_receipt->status_scope = execution->status_scope
                                        ? execution->status_scope
                                        : "BOOT";
        out_receipt->status = execution->status ? execution->status
                                                : "NEXUS RESUMED";
    }
    return 1;
}

int nexus_v1_startup_execute_save_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupSaveExecution local_execution;
    Nexus_V1_StartupSaveExecution *execution;
    int load_success = 1;

    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
    }
    if (out_receipt) {
        nexus_v1_startup_apply_receipt_clear(out_receipt);
    }
    if (!action || !out_receipt) {
        return 0;
    }
    execution = out_execution ? out_execution : &local_execution;
    if (!nexus_v1_startup_execute_save_action(action, execution)) {
        return 0;
    }
    if (execution->kind == NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT) {
        load_success = load_save
            ? load_save(load_userdata, execution->path)
            : 0;
    }
    return nexus_v1_startup_apply_receipt_from_save_execution(
        execution,
        load_success,
        out_receipt);
}

int nexus_v1_startup_execute_save_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt)
{
    Nexus_V1_StartupApplyReceipt apply_receipt;

    nexus_v1_startup_apply_receipt_clear(&apply_receipt);
    if (out_host_receipt) {
        nexus_v1_startup_host_receipt_clear(out_host_receipt);
    }
    if (!nexus_v1_startup_execute_save_action_with_receipt(action,
                                                           load_save,
                                                           load_userdata,
                                                           out_execution,
                                                           &apply_receipt)) {
        return 0;
    }
    if (out_host_receipt &&
        !nexus_v1_startup_host_receipt_from_apply_receipt(&apply_receipt,
                                                          out_host_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_startup_execute_save_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupMenuStateReceipt save_receipt;

    if (out_execution) {
        nexus_v1_startup_save_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
            &save_receipt,
            facts,
            menu_input,
            &action)) {
        return 0;
    }
    if (!nexus_v1_startup_execute_save_action_with_host_receipt(
            &action,
            load_save,
            load_userdata,
            out_execution,
            &out_receipt->host_receipt)) {
        return 0;
    }
    out_receipt->save_state_receipt = save_receipt;
    out_receipt->save_state_receipt_valid = 1;
    return 1;
}

int nexus_v1_startup_execute_save_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupMenuStateReceipt save_receipt;

    if (out_execution) {
        nexus_v1_startup_save_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
            &save_receipt,
            facts,
            x,
            y,
            &action)) {
        return 0;
    }
    if (!nexus_v1_startup_execute_save_action_with_host_receipt(
            &action,
            load_save,
            load_userdata,
            out_execution,
            &out_receipt->host_receipt)) {
        return 0;
    }
    out_receipt->save_state_receipt = save_receipt;
    out_receipt->save_state_receipt_valid = 1;
    return 1;
}

int nexus_v1_startup_apply_receipt_from_title_execution(
    const Nexus_V1_StartupTitleExecution *execution,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupModeUpdate update;

    if (!execution || !out_receipt ||
        execution->kind == NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE) {
        return 0;
    }
    if (!nexus_v1_startup_title_execution_mode_update(execution, &update)) {
        return 0;
    }
    nexus_v1_startup_apply_receipt_clear(out_receipt);
    out_receipt->mode_update = update;
    out_receipt->status_scope = execution->status_scope
                                    ? execution->status_scope
                                    : "STARTUP";
    out_receipt->status = execution->status ? execution->status
                                            : "NEXUS TITLE";
    out_receipt->result =
        execution->kind == NEXUS_V1_STARTUP_TITLE_EXEC_RETURN_TO_LAUNCHER
            ? NEXUS_V1_STARTUP_APPLY_RESULT_RETURN_TO_LAUNCHER
            : NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW;
    return 1;
}

int nexus_v1_startup_execute_title_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupTitleExecution local_execution;
    Nexus_V1_StartupTitleExecution *execution;

    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
    }
    if (out_receipt) {
        nexus_v1_startup_apply_receipt_clear(out_receipt);
    }
    if (!action || !out_receipt) {
        return 0;
    }
    execution = out_execution ? out_execution : &local_execution;
    if (!nexus_v1_startup_execute_title_action(action, execution)) {
        return 0;
    }
    return nexus_v1_startup_apply_receipt_from_title_execution(
        execution,
        out_receipt);
}

int nexus_v1_startup_execute_title_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt)
{
    Nexus_V1_StartupApplyReceipt apply_receipt;

    nexus_v1_startup_apply_receipt_clear(&apply_receipt);
    if (out_host_receipt) {
        nexus_v1_startup_host_receipt_clear(out_host_receipt);
    }
    if (!nexus_v1_startup_execute_title_action_with_receipt(action,
                                                            out_execution,
                                                            &apply_receipt)) {
        return 0;
    }
    if (out_host_receipt &&
        !nexus_v1_startup_host_receipt_from_apply_receipt(&apply_receipt,
                                                          out_host_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;

    if (out_execution) {
        nexus_v1_startup_title_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt || !facts->title_active) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_boot_handle_input(
            facts->title_frame,
            facts->slot_mask,
            nexus_v1_startup_input_from_firestaff_menu_code(menu_input),
            &action)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_action_with_host_receipt(
        &action,
        out_execution,
        &out_receipt->host_receipt);
}

int nexus_v1_startup_execute_title_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;

    if (out_execution) {
        nexus_v1_startup_title_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt || !facts->title_active) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_title_handle_hit(facts->title_frame,
                                           facts->slot_mask,
                                           &action)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_action_with_host_receipt(
        &action,
        out_execution,
        &out_receipt->host_receipt);
}

int nexus_v1_startup_champion_execution_mode_update(
    const Nexus_V1_StartupChampionExecution *execution,
    int save_row_count,
    Nexus_V1_StartupModeUpdate *out_update)
{
    if (!execution || !out_update ||
        execution->kind == NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE) {
        return 0;
    }
    nexus_v1_startup_mode_update_clear(out_update);
    switch (execution->kind) {
        case NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW:
            return 1;
        case NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR:
            out_update->set_champion_cursor = 1;
            out_update->champion_cursor = execution->cursor;
            return 1;
        case NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON:
            out_update->set_champion_select_active = 1;
            out_update->champion_select_active = 0;
            out_update->set_champion_frame = 1;
            out_update->champion_frame = 0;
            return 1;
        case NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_SAVE_SELECT:
            out_update->set_champion_select_active = 1;
            out_update->champion_select_active = 0;
            out_update->set_champion_frame = 1;
            out_update->champion_frame = 0;
            out_update->set_save_select_active = 1;
            out_update->save_select_active = 1;
            if (execution->select_last_save_row && save_row_count > 0) {
                out_update->set_save_selected_row = 1;
                out_update->save_selected_row = save_row_count - 1;
            }
            return 1;
        case NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_TITLE:
            out_update->set_champion_select_active = 1;
            out_update->champion_select_active = 0;
            out_update->set_champion_frame = 1;
            out_update->champion_frame = 0;
            out_update->set_title_active = 1;
            out_update->title_active = 1;
            out_update->set_title_frame = 1;
            out_update->title_frame = 0;
            return 1;
        default:
            break;
    }
    return 0;
}

int nexus_v1_startup_apply_receipt_from_champion_execution(
    const Nexus_V1_StartupChampionExecution *execution,
    int save_row_count,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupModeUpdate update;

    if (!execution || !out_receipt ||
        execution->kind == NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE) {
        return 0;
    }
    if (!nexus_v1_startup_champion_execution_mode_update(
            execution,
            save_row_count,
            &update)) {
        return 0;
    }
    nexus_v1_startup_apply_receipt_clear(out_receipt);
    out_receipt->mode_update = update;
    out_receipt->result = NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW;
    out_receipt->status_scope = execution->status_scope
                                    ? execution->status_scope
                                    : "STARTUP";
    out_receipt->status = execution->status ? execution->status
                                            : "NEXUS CHAMPIONS";
    return 1;
}

int nexus_v1_startup_execute_champion_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    int save_row_count,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution local_execution;
    Nexus_V1_StartupChampionExecution *execution;

    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
    }
    if (out_receipt) {
        nexus_v1_startup_apply_receipt_clear(out_receipt);
    }
    if (!action || !out_receipt) {
        return 0;
    }
    execution = out_execution ? out_execution : &local_execution;
    if (!nexus_v1_startup_execute_champion_action(action, execution)) {
        return 0;
    }
    return nexus_v1_startup_apply_receipt_from_champion_execution(
        execution,
        save_row_count,
        out_receipt);
}

int nexus_v1_startup_execute_champion_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    int save_row_count,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt)
{
    Nexus_V1_StartupApplyReceipt apply_receipt;

    nexus_v1_startup_apply_receipt_clear(&apply_receipt);
    if (out_host_receipt) {
        nexus_v1_startup_host_receipt_clear(out_host_receipt);
    }
    if (!nexus_v1_startup_execute_champion_action_with_receipt(
            action,
            save_row_count,
            out_execution,
            &apply_receipt)) {
        return 0;
    }
    if (out_host_receipt &&
        !nexus_v1_startup_host_receipt_from_apply_receipt(&apply_receipt,
                                                          out_host_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_startup_execute_champion_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupChampionStateReceipt champion_receipt;

    if (out_execution) {
        nexus_v1_startup_champion_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_champion_handle_firestaff_input_from_host_facts_with_receipt(
            &champion_receipt,
            facts,
            menu_input,
            &action)) {
        return 0;
    }
    if (!nexus_v1_startup_execute_champion_action_with_host_receipt(
            &action,
            facts->save_row_count,
            out_execution,
            &out_receipt->host_receipt)) {
        return 0;
    }
    out_receipt->champion_state_receipt = champion_receipt;
    out_receipt->champion_state_receipt_valid = 1;
    return 1;
}

int nexus_v1_startup_execute_champion_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupChampionStateReceipt champion_receipt;

    if (out_execution) {
        nexus_v1_startup_champion_execution_clear(out_execution);
    }
    if (out_receipt) {
        nexus_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        nexus_v1_startup_action_clear(&action);
        return 0;
    }
    if (!nexus_v1_startup_champion_handle_pointer_from_host_facts_with_receipt(
            &champion_receipt,
            facts,
            x,
            y,
            &action)) {
        return 0;
    }
    if (!nexus_v1_startup_execute_champion_action_with_host_receipt(
            &action,
            facts->save_row_count,
            out_execution,
            &out_receipt->host_receipt)) {
        return 0;
    }
    out_receipt->champion_state_receipt = champion_receipt;
    out_receipt->champion_state_receipt_valid = 1;
    return 1;
}

int nexus_v1_startup_menu_build_save_render_rows(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupSaveRenderRow *rows,
    int max_rows)
{
    int row;
    int count = 0;

    if (!menu || !rows || max_rows <= 0) {
        return 0;
    }
    memset(rows, 0, (size_t)max_rows * sizeof(rows[0]));
    for (row = 0; row < menu->row_count && count < max_rows; ++row) {
        Nexus_V1_StartupRowKind kind = NEXUS_V1_STARTUP_ROW_NONE;
        int slot = -1;
        Nexus_V1_StartupSaveRenderRow *out = &rows[count];

        if (!nexus_v1_startup_menu_row_at(menu, row, &kind, &slot) ||
            !nexus_v1_startup_save_row_rect(row, &out->rect)) {
            continue;
        }
        out->kind = kind;
        out->row = row;
        out->slot = slot;
        out->selected = (row == menu->selected_row) ? 1 : 0;
        out->highlight_rect.x = out->rect.x - 2;
        out->highlight_rect.y = out->rect.y;
        out->highlight_rect.w = 160;
        out->highlight_rect.h = 11;
        out->text_x = NEXUS_V1_STARTUP_SAVE_ROW_TEXT_X;
        out->text_y = out->rect.y + 1;
        if (kind == NEXUS_V1_STARTUP_ROW_SLOT) {
            snprintf(out->label,
                     sizeof(out->label),
                     "%c LOAD SLOT %02d",
                     out->selected ? '>' : ' ',
                     slot);
        } else if (kind == NEXUS_V1_STARTUP_ROW_NEW_GAME) {
            snprintf(out->label,
                     sizeof(out->label),
                     "%c NEW GAME",
                     out->selected ? '>' : ' ');
        }
        ++count;
    }
    return count;
}

int nexus_v1_startup_menu_snapshot_build_save_render_rows(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupSaveRenderRow *rows,
    int max_rows)
{
    Nexus_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    nexus_v1_startup_menu_from_snapshot(snapshot, &menu);
    return nexus_v1_startup_menu_build_save_render_rows(&menu,
                                                        rows,
                                                        max_rows);
}

static void nexus_v1_startup_chrome_clear(
    Nexus_V1_StartupChromeRender *chrome)
{
    if (!chrome) {
        return;
    }
    memset(chrome, 0, sizeof(*chrome));
    chrome->title_x = NEXUS_V1_STARTUP_TITLE_X;
    chrome->title_y = NEXUS_V1_STARTUP_TITLE_Y;
    chrome->subtitle_x = NEXUS_V1_STARTUP_TITLE_X;
    chrome->subtitle_y = NEXUS_V1_STARTUP_SUBTITLE_Y;
    chrome->footer_x = NEXUS_V1_STARTUP_FOOTER_X;
    chrome->footer_y = NEXUS_V1_STARTUP_FOOTER_Y;
}

int nexus_v1_startup_menu_build_save_chrome_render(
    Nexus_V1_StartupChromeRender *out_chrome)
{
    if (!out_chrome) {
        return 0;
    }
    nexus_v1_startup_chrome_clear(out_chrome);
    snprintf(out_chrome->title,
             sizeof(out_chrome->title),
             "DUNGEON MASTER NEXUS");
    snprintf(out_chrome->subtitle,
             sizeof(out_chrome->subtitle),
             "LOAD GAME");
    snprintf(out_chrome->footer,
             sizeof(out_chrome->footer),
             "ACCEPT LOADS  ACTION STARTS");
    return 1;
}

int nexus_v1_startup_menu_build_champion_chrome_render(
    Nexus_V1_StartupChromeRender *out_chrome)
{
    if (!out_chrome) {
        return 0;
    }
    nexus_v1_startup_chrome_clear(out_chrome);
    snprintf(out_chrome->title,
             sizeof(out_chrome->title),
             "DUNGEON MASTER NEXUS");
    snprintf(out_chrome->subtitle,
             sizeof(out_chrome->subtitle),
             "SELECT CHAMPIONS");
    return 1;
}

int nexus_v1_startup_menu_build_champion_render_rows(
    const Nexus_V1_ChampionPool *pool,
    int cursor,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer)
{
    return nexus_v1_startup_menu_build_champion_render_rows_for_frame(
        pool, cursor, 0, rows, max_rows, out_footer);
}

int nexus_v1_startup_menu_build_champion_render_rows_for_frame(
    const Nexus_V1_ChampionPool *pool,
    int cursor,
    int frame,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer)
{
    int row;
    int count = 0;
    int first_row = 0;
    int blink_on;

    if (!pool || !rows || max_rows <= 0) {
        return 0;
    }
    if (frame < 0) {
        frame = 0;
    }
    blink_on = ((frame / 12) & 1) == 0;
    memset(rows, 0, (size_t)max_rows * sizeof(rows[0]));
    if (out_footer) {
        Nexus_V1_StartupRect footer_rect;
        memset(out_footer, 0, sizeof(*out_footer));
        if (nexus_v1_startup_champion_footer_rect(&footer_rect)) {
            out_footer->rect = footer_rect;
        }
        out_footer->text_x = NEXUS_V1_STARTUP_FOOTER_X;
        out_footer->text_y = NEXUS_V1_STARTUP_FOOTER_Y;
        snprintf(out_footer->label,
                 sizeof(out_footer->label),
                 "PARTY %d/%d  ACCEPT ADD  ACTION START",
                 pool->party_count,
                 NEXUS_MAX_PARTY);
    }
    first_row = nexus_v1_startup_champion_visible_first_row(
        pool->champion_count, cursor, max_rows);
    for (row = first_row; row < pool->champion_count && count < max_rows; ++row) {
        Nexus_V1_StartupChampionRenderRow *out = &rows[count];
        int in_party = 0;
        int party_index;

        if (!nexus_v1_startup_champion_row_rect(count, &out->rect)) {
            continue;
        }
        for (party_index = 0; party_index < pool->party_count; ++party_index) {
            if (pool->party[party_index] == row) {
                in_party = 1;
                break;
            }
        }
        out->row = row;
        out->selected = (row == cursor) ? 1 : 0;
        out->in_party = in_party;
        out->portrait_index = pool->champions[row].portrait_index;
        out->highlight_rect = out->rect;
        out->highlight_rect.x -= 2;
        out->highlight_rect.y -= 1;
        out->highlight_rect.w = 266;
        out->highlight_rect.h += 2;
        out->portrait_x = NEXUS_V1_STARTUP_CHAMPION_PORTRAIT_X;
        out->portrait_y = out->rect.y + 1;
        out->portrait_w = 10;
        out->portrait_h = 10;
        out->highlight_visible = out->selected && blink_on ? 1 : 0;
        out->text_color = out->selected && blink_on ? 11 : 15;
        out->shadow_color = 0;
        out->portrait_border_color = out->selected && blink_on ? 11 :
            (out->in_party ? 7 : 12);
        out->party_marker_color = out->in_party ? 7 : 12;
        out->text_x = NEXUS_V1_STARTUP_CHAMPION_ROW_TEXT_X;
        out->text_y = out->rect.y + 1;
        snprintf(out->label,
                 sizeof(out->label),
                 "%c %s %s HP %d MP %d",
                 out->selected ? '>' : ' ',
                 out->in_party ? "*" : " ",
                 pool->champions[row].name_ascii,
                 pool->champions[row].max_health,
                 pool->champions[row].max_mana);
        ++count;
    }
    return count;
}

int nexus_v1_startup_champion_snapshot_refresh(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot)
{
    if (!snapshot) {
        return 0;
    }
    if (!pool || pool->champion_count <= 0) {
        snapshot->cursor = 0;
        if (snapshot->frame < 0) {
            snapshot->frame = 0;
        }
        return 1;
    }
    if (snapshot->cursor < 0 || snapshot->cursor >= pool->champion_count) {
        snapshot->cursor = 0;
    }
    if (snapshot->frame < 0) {
        snapshot->frame = 0;
    }
    return 1;
}

int nexus_v1_startup_champion_snapshot_from_facts(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame)
{
    if (!snapshot) {
        return 0;
    }
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->slot_mask = nexus_v1_startup_slot_mask_clamp(slot_mask);
    snapshot->cursor = cursor;
    snapshot->frame = frame;
    return nexus_v1_startup_champion_snapshot_refresh(pool, snapshot);
}

int nexus_v1_startup_champion_state_receipt_from_snapshot(
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupChampionStateReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    nexus_v1_startup_champion_state_receipt_init(out_receipt);
    if (!snapshot) {
        return 0;
    }
    out_receipt->slot_mask = nexus_v1_startup_slot_mask_clamp(
        snapshot->slot_mask);
    out_receipt->cursor = snapshot->cursor < 0 ? 0 : snapshot->cursor;
    out_receipt->frame = snapshot->frame < 0 ? 0 : snapshot->frame;
    return 1;
}

int nexus_v1_startup_champion_state_receipt_from_facts(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame)
{
    Nexus_V1_StartupChampionSnapshot snapshot;

    if (!nexus_v1_startup_champion_snapshot_from_facts(pool,
                                                       &snapshot,
                                                       slot_mask,
                                                       cursor,
                                                       frame)) {
        nexus_v1_startup_champion_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_champion_state_receipt_from_snapshot(
        &snapshot,
        out_receipt);
}

int nexus_v1_startup_champion_snapshot_handle_input(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupInput input,
    Nexus_V1_StartupAction *out_action)
{
    int cursor;
    int handled;

    if (!snapshot) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    (void)nexus_v1_startup_champion_snapshot_refresh(pool, snapshot);
    cursor = snapshot->cursor;
    handled = nexus_v1_startup_champion_handle_input(pool,
                                                     &cursor,
                                                     snapshot->slot_mask,
                                                     input,
                                                     out_action);
    snapshot->cursor = cursor;
    (void)nexus_v1_startup_champion_snapshot_refresh(pool, snapshot);
    return handled;
}

int nexus_v1_startup_champion_handle_firestaff_input_from_facts(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action)
{
    if (!nexus_v1_startup_champion_snapshot_from_facts(pool,
                                                       snapshot,
                                                       slot_mask,
                                                       cursor,
                                                       frame)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_champion_snapshot_handle_input(
        pool,
        snapshot,
        nexus_v1_startup_input_from_firestaff_menu_code(firestaff_input),
        out_action);
}

int nexus_v1_startup_champion_handle_firestaff_input_from_facts_with_receipt(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupChampionSnapshot snapshot;

    if (!nexus_v1_startup_champion_handle_firestaff_input_from_facts(
            pool,
            &snapshot,
            slot_mask,
            cursor,
            frame,
            firestaff_input,
            out_action)) {
        nexus_v1_startup_champion_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_champion_state_receipt_from_snapshot(
        &snapshot,
        out_receipt);
}

int nexus_v1_startup_champion_handle_firestaff_input_from_host_facts_with_receipt(
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action)
{
    if (!facts) {
        nexus_v1_startup_champion_state_receipt_init(out_receipt);
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_champion_handle_firestaff_input_from_facts_with_receipt(
        facts->champion_pool,
        out_receipt,
        facts->slot_mask,
        facts->champion_cursor,
        facts->champion_frame,
        firestaff_input,
        out_action);
}

int nexus_v1_startup_champion_snapshot_handle_hit(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    const Nexus_V1_StartupHit *hit,
    Nexus_V1_StartupAction *out_action)
{
    int cursor;
    int handled;

    if (!snapshot) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    (void)nexus_v1_startup_champion_snapshot_refresh(pool, snapshot);
    cursor = snapshot->cursor;
    handled = nexus_v1_startup_champion_handle_hit(pool,
                                                   &cursor,
                                                   snapshot->slot_mask,
                                                   hit,
                                                   out_action);
    snapshot->cursor = cursor;
    (void)nexus_v1_startup_champion_snapshot_refresh(pool, snapshot);
    return handled;
}

int nexus_v1_startup_champion_handle_pointer_from_facts(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupHit hit;

    if (!nexus_v1_startup_champion_snapshot_from_facts(pool,
                                                       snapshot,
                                                       slot_mask,
                                                       cursor,
                                                       frame)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    if (!nexus_v1_startup_champion_hit_at_cursor(
            pool ? pool->champion_count : 0,
            snapshot->cursor,
            x,
            y,
            &hit)) {
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_champion_snapshot_handle_hit(pool,
                                                        snapshot,
                                                        &hit,
                                                        out_action);
}

int nexus_v1_startup_champion_handle_pointer_from_facts_with_receipt(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    Nexus_V1_StartupChampionSnapshot snapshot;

    if (!nexus_v1_startup_champion_handle_pointer_from_facts(pool,
                                                             &snapshot,
                                                             slot_mask,
                                                             cursor,
                                                             frame,
                                                             x,
                                                             y,
                                                             out_action)) {
        nexus_v1_startup_champion_state_receipt_init(out_receipt);
        return 0;
    }
    return nexus_v1_startup_champion_state_receipt_from_snapshot(
        &snapshot,
        out_receipt);
}

int nexus_v1_startup_champion_handle_pointer_from_host_facts_with_receipt(
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action)
{
    if (!facts) {
        nexus_v1_startup_champion_state_receipt_init(out_receipt);
        nexus_v1_startup_action_clear(out_action);
        return 0;
    }
    return nexus_v1_startup_champion_handle_pointer_from_facts_with_receipt(
        facts->champion_pool,
        out_receipt,
        facts->slot_mask,
        facts->champion_cursor,
        facts->champion_frame,
        x,
        y,
        out_action);
}

int nexus_v1_startup_champion_snapshot_build_render_rows(
    const Nexus_V1_ChampionPool *pool,
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_startup_menu_build_champion_render_rows_for_frame(
        pool,
        snapshot->cursor,
        snapshot->frame,
        rows,
        max_rows,
        out_footer);
}

int nexus_v1_startup_presentation_build_save(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupChromeRender chrome;
    Nexus_V1_StartupSaveRenderRow rows[16];
    int row_count;
    int row;
    int count = 0;

    if (!snapshot || !out_commands || max_commands <= 0) {
        return 0;
    }
    memset(out_commands,
           0,
           (size_t)max_commands * sizeof(out_commands[0]));
    if (!nexus_v1_startup_push_background(out_commands,
                                          max_commands,
                                          &count)) {
        return count;
    }
    if (nexus_v1_startup_menu_build_save_chrome_render(&chrome)) {
        if (!nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        chrome.title_x,
                                        chrome.title_y,
                                        chrome.title,
                                        NEXUS_V1_STARTUP_TEXT_TITLE,
                                        15,
                                        0) ||
            !nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        chrome.subtitle_x,
                                        chrome.subtitle_y,
                                        chrome.subtitle,
                                        NEXUS_V1_STARTUP_TEXT_SHADOW,
                                        15,
                                        0)) {
            return count;
        }
    }
    row_count = nexus_v1_startup_menu_snapshot_build_save_render_rows(
        snapshot,
        rows,
        (int)(sizeof(rows) / sizeof(rows[0])));
    for (row = 0; row < row_count; ++row) {
        const Nexus_V1_StartupSaveRenderRow *render_row = &rows[row];
        if (render_row->selected &&
            !nexus_v1_startup_push_rect(out_commands,
                                        max_commands,
                                        &count,
                                        NEXUS_V1_STARTUP_DRAW_FILL_RECT,
                                        &render_row->highlight_rect,
                                        8)) {
            return count;
        }
        if (!nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        render_row->text_x,
                                        render_row->text_y,
                                        render_row->label,
                                        render_row->selected
                                            ? NEXUS_V1_STARTUP_TEXT_SHADOW
                                            : NEXUS_V1_STARTUP_TEXT_SMALL,
                                        15,
                                        0)) {
            return count;
        }
    }
    if (chrome.footer[0]) {
        (void)nexus_v1_startup_push_text(out_commands,
                                         max_commands,
                                         &count,
                                         chrome.footer_x,
                                         chrome.footer_y,
                                         chrome.footer,
                                         NEXUS_V1_STARTUP_TEXT_SMALL,
                                         15,
                                         0);
    }
    return count;
}

int nexus_v1_startup_presentation_build_save_from_facts(
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupMenuSnapshot snapshot;

    if (!nexus_v1_startup_menu_snapshot_from_facts(&snapshot,
                                                   save_dir,
                                                   slot_mask,
                                                   selected_row)) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_startup_presentation_build_save(&snapshot,
                                                    out_commands,
                                                    max_commands);
}

int nexus_v1_startup_presentation_build_save_from_host_facts(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!facts) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_startup_presentation_build_save_from_facts(
        facts->save_dir,
        facts->slot_mask,
        facts->save_selected_row,
        out_commands,
        max_commands);
}

int nexus_v1_startup_presentation_build_title(
    int title_frame,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_BootFrame boot_frame;
    int count = 0;

    if (!out_commands || max_commands <= 0) {
        return 0;
    }
    memset(out_commands,
           0,
           (size_t)max_commands * sizeof(out_commands[0]));
    if (!nexus_v1_boot_frame(title_frame, 200, &boot_frame)) {
        return count;
    }
    if (boot_frame.warning_visible) {
        (void)nexus_v1_startup_push_warning_background(out_commands,
                                                       max_commands,
                                                       &count);
        return count;
    }
    (void)nexus_v1_startup_push_boot_title_frame(out_commands,
                                                max_commands,
                                                &count,
                                                boot_frame.title_frame);
    return count;
}

int nexus_v1_startup_presentation_build_champion(
    const Nexus_V1_ChampionPool *pool,
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupChromeRender chrome;
    Nexus_V1_StartupChampionRenderRow rows[12];
    Nexus_V1_StartupChampionFooterRender footer;
    int row_count;
    int row;
    int count = 0;

    if (!pool || !snapshot || !out_commands || max_commands <= 0) {
        return 0;
    }
    memset(out_commands,
           0,
           (size_t)max_commands * sizeof(out_commands[0]));
    if (!nexus_v1_startup_push_background(out_commands,
                                          max_commands,
                                          &count)) {
        return count;
    }
    if (nexus_v1_startup_menu_build_champion_chrome_render(&chrome)) {
        if (!nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        chrome.title_x,
                                        chrome.title_y,
                                        chrome.title,
                                        NEXUS_V1_STARTUP_TEXT_TITLE,
                                        15,
                                        0) ||
            !nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        chrome.subtitle_x,
                                        chrome.subtitle_y,
                                        chrome.subtitle,
                                        NEXUS_V1_STARTUP_TEXT_SHADOW,
                                        15,
                                        0)) {
            return count;
        }
    }
    row_count = nexus_v1_startup_champion_snapshot_build_render_rows(
        pool,
        snapshot,
        rows,
        (int)(sizeof(rows) / sizeof(rows[0])),
        &footer);
    for (row = 0; row < row_count; ++row) {
        const Nexus_V1_StartupChampionRenderRow *render_row = &rows[row];
        if (render_row->highlight_visible &&
            !nexus_v1_startup_push_rect(out_commands,
                                        max_commands,
                                        &count,
                                        NEXUS_V1_STARTUP_DRAW_OUTLINE_RECT,
                                        &render_row->highlight_rect,
                                        render_row->text_color)) {
            return count;
        }
        if (!nexus_v1_startup_push_portrait(out_commands,
                                            max_commands,
                                            &count,
                                            render_row)) {
            return count;
        }
        if (render_row->portrait_border_color > 0) {
            Nexus_V1_StartupRect border;
            border.x = render_row->portrait_x - 1;
            border.y = render_row->portrait_y - 1;
            border.w = render_row->portrait_w + 2;
            border.h = render_row->portrait_h + 2;
            if (!nexus_v1_startup_push_rect(out_commands,
                                            max_commands,
                                            &count,
                                            NEXUS_V1_STARTUP_DRAW_OUTLINE_RECT,
                                            &border,
                                            render_row->portrait_border_color)) {
                return count;
            }
        }
        if (!nexus_v1_startup_push_text(out_commands,
                                        max_commands,
                                        &count,
                                        render_row->text_x,
                                        render_row->text_y,
                                        render_row->label,
                                        NEXUS_V1_STARTUP_TEXT_SMALL,
                                        render_row->text_color,
                                        render_row->shadow_color)) {
            return count;
        }
    }
    (void)nexus_v1_startup_push_text(out_commands,
                                     max_commands,
                                     &count,
                                     footer.text_x,
                                     footer.text_y,
                                     footer.label,
                                     NEXUS_V1_STARTUP_TEXT_SMALL,
                                     15,
                                     0);
    return count;
}

int nexus_v1_startup_presentation_build_champion_from_facts(
    const Nexus_V1_ChampionPool *pool,
    unsigned int slot_mask,
    int cursor,
    int frame,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupChampionSnapshot snapshot;

    if (!nexus_v1_startup_champion_snapshot_from_facts(pool,
                                                       &snapshot,
                                                       slot_mask,
                                                       cursor,
                                                       frame)) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_startup_presentation_build_champion(pool,
                                                        &snapshot,
                                                        out_commands,
                                                        max_commands);
}

int nexus_v1_startup_presentation_build_champion_from_host_facts(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!facts) {
        if (out_commands && max_commands > 0) {
            memset(out_commands,
                   0,
                   (size_t)max_commands * sizeof(out_commands[0]));
        }
        return 0;
    }
    return nexus_v1_startup_presentation_build_champion_from_facts(
        facts->champion_pool,
        facts->slot_mask,
        facts->champion_cursor,
        facts->champion_frame,
        out_commands,
        max_commands);
}

int nexus_v1_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor)
{
    int i;
    if (!commands || command_count <= 0 || !executor) {
        return 0;
    }
    for (i = 0; i < command_count; ++i) {
        const Nexus_V1_StartupDrawCommand *command = &commands[i];
        switch (command->kind) {
            case NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND:
                if (executor->draw_title_background) {
                    executor->draw_title_background(executor->userdata,
                                                    command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_FILL_RECT:
                if (executor->fill_rect) {
                    executor->fill_rect(executor->userdata, command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_OUTLINE_RECT:
                if (executor->outline_rect) {
                    executor->outline_rect(executor->userdata, command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_TEXT:
                if (executor->draw_text) {
                    executor->draw_text(executor->userdata, command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_PORTRAIT:
                if (executor->draw_portrait) {
                    executor->draw_portrait(executor->userdata, command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME:
                if (executor->draw_boot_title_frame) {
                    executor->draw_boot_title_frame(executor->userdata,
                                                    command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND:
                if (executor->draw_warning_background) {
                    executor->draw_warning_background(executor->userdata,
                                                      command);
                }
                break;
            case NEXUS_V1_STARTUP_DRAW_NONE:
            default:
                break;
        }
    }
    return 1;
}

int nexus_v1_startup_champion_handle_input(Nexus_V1_ChampionPool *pool,
                                           int *cursor,
                                           unsigned int slot_mask,
                                           Nexus_V1_StartupInput input,
                                           Nexus_V1_StartupAction *out_action)
{
    int current;
    int next_cursor;

    nexus_v1_startup_action_clear(out_action);
    if (!pool || !cursor || !out_action) {
        return 0;
    }
    current = *cursor;
    if (pool->champion_count <= 0) {
        current = 0;
    } else if (current < 0 || current >= pool->champion_count) {
        current = 0;
    }

    if (input == NEXUS_V1_STARTUP_INPUT_BACK) {
        int removed = nexus_v1_champion_unrecruit_last(pool);
        if (removed >= 0) {
            *cursor = removed;
            out_action->kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED;
            out_action->row = removed;
        } else if (slot_mask != 0u) {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT;
        } else {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE;
        }
        return 1;
    }
    if (pool->champion_count <= 0) {
        return 0;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_UP) {
        current = nexus_v1_champion_next_selectable(
            pool,
            current + pool->champion_count - 1,
            -1);
        *cursor = current;
        out_action->kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR;
        out_action->row = current;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_DOWN) {
        current = nexus_v1_champion_next_selectable(pool, current + 1, 1);
        *cursor = current;
        out_action->kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR;
        out_action->row = current;
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_ACCEPT) {
        next_cursor = current;
        if (nexus_v1_champion_recruit_and_advance(pool,
                                                  current,
                                                  &next_cursor) >= 0) {
            *cursor = next_cursor;
            out_action->kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED;
            out_action->row = current;
        } else {
            *cursor = current;
            out_action->kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_SKIPPED;
            out_action->row = current;
        }
        return 1;
    }
    if (input == NEXUS_V1_STARTUP_INPUT_ACTION) {
        if (pool->party_count <= 0) {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION;
            out_action->row = current;
        } else {
            out_action->kind = NEXUS_V1_STARTUP_ACTION_START_DUNGEON;
        }
        return 1;
    }
    return 0;
}

int nexus_v1_startup_champion_handle_hit(Nexus_V1_ChampionPool *pool,
                                         int *cursor,
                                         unsigned int slot_mask,
                                         const Nexus_V1_StartupHit *hit,
                                         Nexus_V1_StartupAction *out_action)
{
    nexus_v1_startup_action_clear(out_action);
    if (!pool || !cursor || !hit || !out_action) {
        return 0;
    }
    if (hit->kind == NEXUS_V1_STARTUP_HIT_CHAMPION_PANEL) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (hit->kind == NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER) {
        return nexus_v1_startup_champion_handle_input(
            pool,
            cursor,
            slot_mask,
            NEXUS_V1_STARTUP_INPUT_ACTION,
            out_action);
    }
    if (hit->kind != NEXUS_V1_STARTUP_HIT_CHAMPION_ROW ||
        hit->row < 0 || hit->row >= pool->champion_count) {
        return 0;
    }
    *cursor = hit->row;
    return nexus_v1_startup_champion_handle_input(
        pool,
        cursor,
        slot_mask,
        NEXUS_V1_STARTUP_INPUT_ACCEPT,
        out_action);
}

int nexus_v1_startup_receipt_phase(int title_active,
                                   int save_select_active,
                                   int champion_select_active,
                                   int title_frame,
                                   char *out_phase,
                                   int out_phase_size,
                                   int *out_startup_active,
                                   int *out_startup_frame)
{
    const char *phase = "nexus-runtime";
    int active = 0;

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    if (title_active) {
        phase = "nexus-title";
        active = 1;
    } else if (save_select_active) {
        phase = "nexus-save-select";
        active = 1;
    } else if (champion_select_active) {
        phase = "nexus-champion-select";
        active = 1;
    }
    snprintf(out_phase, (size_t)out_phase_size, "%s", phase);
    if (out_startup_active) {
        *out_startup_active = active;
    }
    if (out_startup_frame) {
        *out_startup_frame = title_frame;
    }
    return 1;
}

int nexus_v1_startup_presentation_receipt(int title_active,
                                          int save_select_active,
                                          int champion_select_active,
                                          int title_frame,
                                          char *out_phase,
                                          int out_phase_size,
                                          int *out_startup_active,
                                          int *out_startup_frame,
                                          char *out_animation,
                                          int out_animation_size,
                                          int *out_animation_active,
                                          int *out_title_frame,
                                          int *out_title_frame_max,
                                          int *out_title_ready)
{
    const int active_title = title_active ? 1 : 0;
    const int ready_frames = nexus_v1_boot_start_ready_frames();
    const char *animation = "nexus-runtime";

    if (active_title) {
        animation = "nexus-title";
    } else if (champion_select_active) {
        animation = "nexus-champion-select";
    }

    if (!nexus_v1_startup_receipt_phase(title_active,
                                        save_select_active,
                                        champion_select_active,
                                        title_frame,
                                        out_phase,
                                        out_phase_size,
                                        out_startup_active,
                                        out_startup_frame)) {
        return 0;
    }
    if (!out_animation || out_animation_size <= 0) {
        return 0;
    }
    snprintf(out_animation, (size_t)out_animation_size, "%s", animation);
    if (out_animation_active) {
        *out_animation_active = active_title;
    }
    if (out_title_frame) {
        *out_title_frame = active_title ? title_frame : -1;
    }
    if (out_title_frame_max) {
        *out_title_frame_max = ready_frames;
    }
    if (out_title_ready) {
        *out_title_ready = !active_title || title_frame >= ready_frames;
    }
    return 1;
}
