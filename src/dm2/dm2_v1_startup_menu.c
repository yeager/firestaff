#include "dm2_v1_startup_menu.h"
#include "dm2_v1_save_load.h"

#include <stdio.h>
#include <string.h>

static void dm2_v1_startup_action_clear(DM2_V1_StartupAction *action)
{
    if (!action) {
        return;
    }
    memset(action, 0, sizeof(*action));
    action->kind = DM2_V1_STARTUP_ACTION_NONE;
    action->row = -1;
    action->slot = -1;
}

static void dm2_v1_startup_action_plan_clear(
    DM2_V1_StartupActionPlan *plan)
{
    if (!plan) {
        return;
    }
    memset(plan, 0, sizeof(*plan));
    plan->kind = DM2_V1_STARTUP_PLAN_IGNORE;
    plan->slot = -1;
}

static void dm2_v1_startup_execution_clear(
    DM2_V1_StartupExecution *execution)
{
    if (!execution) {
        return;
    }
    memset(execution, 0, sizeof(*execution));
    execution->kind = DM2_V1_STARTUP_EXEC_IGNORE;
}

static void dm2_v1_startup_mode_update_clear(
    DM2_V1_StartupModeUpdate *update)
{
    if (!update) {
        return;
    }
    memset(update, 0, sizeof(*update));
}

static void dm2_v1_startup_input_outcome_clear(
    DM2_V1_StartupInputOutcome *outcome)
{
    if (!outcome) {
        return;
    }
    memset(outcome, 0, sizeof(*outcome));
    outcome->result = DM2_V1_STARTUP_INPUT_RESULT_IGNORED;
}

static void dm2_v1_startup_apply_receipt_clear(
    DM2_V1_StartupApplyReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->outcome.result = DM2_V1_STARTUP_INPUT_RESULT_IGNORED;
}

void dm2_v1_startup_menu_state_receipt_init(
    DM2_V1_StartupMenuStateReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

DM2_V1_StartupInput dm2_v1_startup_input_from_firestaff_menu_code(
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
            return DM2_V1_STARTUP_INPUT_UP;
        case FIRESTAFF_MENU_INPUT_DOWN:
            return DM2_V1_STARTUP_INPUT_DOWN;
        case FIRESTAFF_MENU_INPUT_ACCEPT:
            return DM2_V1_STARTUP_INPUT_ACCEPT;
        case FIRESTAFF_MENU_INPUT_ACTION:
            return DM2_V1_STARTUP_INPUT_ACTION;
        case FIRESTAFF_MENU_INPUT_BACK:
            return DM2_V1_STARTUP_INPUT_BACK;
        case FIRESTAFF_MENU_INPUT_NONE:
        default:
            return DM2_V1_STARTUP_INPUT_NONE;
    }
}

int dm2_v1_startup_save_path_to_root_slot(const char *save_path,
                                          char *out_root,
                                          int out_root_cap,
                                          uint8_t *out_slot,
                                          int *out_last_session)
{
    const char *base;
    const char *slash;
    int slot;

    if (!save_path || !save_path[0] || !out_root || out_root_cap <= 0 ||
        !out_slot || !out_last_session) {
        return 0;
    }
    *out_last_session = 0;
    slash = strrchr(save_path, '/');
#ifdef _WIN32
    {
        const char *backslash = strrchr(save_path, '\\');
        if (!slash || (backslash && backslash > slash)) {
            slash = backslash;
        }
    }
#endif
    base = slash ? slash + 1 : save_path;
    if (strcmp(base, "SKSave.dat") == 0 ||
        strcmp(base, "SKSave.bak") == 0) {
        slot = 0;
        *out_last_session = 1;
    } else {
        if (strncmp(base, "SKSave", 6) != 0 ||
            base[6] < '0' || base[6] > '9' ||
            base[7] < '0' || base[7] > '9' ||
            strcmp(base + 8, ".dat") != 0) {
            return 0;
        }
        slot = (base[6] - '0') * 10 + (base[7] - '0');
        if (slot < 0 || slot >= 10) {
            return 0;
        }
    }
    if (slash) {
        size_t len = (size_t)(slash - save_path);
        if (len == 0u || len >= (size_t)out_root_cap) {
            return 0;
        }
        memcpy(out_root, save_path, len);
        out_root[len] = '\0';
    } else {
        if (out_root_cap < 2) {
            return 0;
        }
        out_root[0] = '.';
        out_root[1] = '\0';
    }
    *out_slot = (uint8_t)slot;
    return 1;
}

DM2_V1_StartupSavePathResult
dm2_v1_startup_load_session_from_save_path(const char *save_path,
                                           char *out_root,
                                           int out_root_cap,
                                           DM2_V1_SessionState *out_session,
                                           uint8_t *out_slot,
                                           int *out_last_session)
{
    char parsed_root[512];
    uint8_t slot = 0u;
    int last_session = 0;
    int load_result;

    if (!out_session ||
        !dm2_v1_startup_save_path_to_root_slot(save_path,
                                               parsed_root,
                                               (int)sizeof(parsed_root),
                                               &slot,
                                               &last_session)) {
        return DM2_V1_STARTUP_SAVE_PATH_INVALID;
    }
    if (out_root && out_root_cap > 0) {
        snprintf(out_root, (size_t)out_root_cap, "%s", parsed_root);
    }
    if (out_slot) {
        *out_slot = slot;
    }
    if (out_last_session) {
        *out_last_session = last_session;
    }
    memset(out_session, 0, sizeof(*out_session));
    load_result = last_session
        ? dm2_v1_session_load_last_session(parsed_root, out_session)
        : dm2_v1_session_load_slot(parsed_root, slot, out_session);
    return load_result == 0
        ? DM2_V1_STARTUP_SAVE_PATH_LOADED
        : DM2_V1_STARTUP_SAVE_PATH_LOAD_FAILED;
}

void dm2_v1_startup_menu_init(DM2_V1_StartupMenu *menu,
                              const char *save_root)
{
    if (!menu) {
        return;
    }
    memset(menu, 0, sizeof(*menu));
    if (save_root && save_root[0]) {
        snprintf(menu->save_root, sizeof(menu->save_root), "%s", save_root);
    }
    menu->row_count = 1;
}

int dm2_v1_startup_menu_count_rows(int resume_available,
                                   unsigned int slot_mask)
{
    int count = resume_available ? 1 : 0;
    int slot;

    for (slot = 0; slot < 10; ++slot) {
        if (slot_mask & (1u << slot)) {
            ++count;
        }
    }
    return count + 1;
}

int dm2_v1_startup_menu_scan_saves(DM2_V1_StartupMenu *menu)
{
    unsigned int slot_mask = 0u;
    int resume_available;
    int slot;

    if (!menu) {
        return 0;
    }
    resume_available =
        dm2_v1_save_has_valid_last_session(menu->save_root) ? 1 : 0;
    for (slot = 0; slot < 10; ++slot) {
        if (dm2_v1_save_has_valid_slot(menu->save_root, (uint8_t)slot)) {
            slot_mask |= (1u << slot);
        }
    }
    return dm2_v1_startup_menu_refresh(
        menu,
        resume_available,
        slot_mask);
}

int dm2_v1_startup_menu_refresh(DM2_V1_StartupMenu *menu,
                                int resume_available,
                                unsigned int slot_mask)
{
    if (!menu) {
        return 0;
    }
    menu->resume_available = resume_available ? 1 : 0;
    menu->slot_mask = slot_mask & 0x03ffu;
    menu->row_count = dm2_v1_startup_menu_count_rows(
        menu->resume_available,
        menu->slot_mask);
    if (menu->row_count < 1) {
        menu->row_count = 1;
    }
    if (menu->selected_row < 0) {
        menu->selected_row = 0;
    }
    if (menu->selected_row >= menu->row_count) {
        menu->selected_row = menu->row_count - 1;
    }
    return 1;
}

void dm2_v1_startup_menu_snapshot_init(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root)
{
    DM2_V1_StartupMenu menu;

    if (!snapshot) {
        return;
    }
    dm2_v1_startup_menu_init(&menu, save_root);
    (void)dm2_v1_startup_menu_snapshot_from_menu(snapshot, &menu);
}

int dm2_v1_startup_menu_snapshot_scan_saves(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root)
{
    DM2_V1_StartupMenu menu;

    if (!snapshot) {
        return 0;
    }
    dm2_v1_startup_menu_init(
        &menu,
        save_root && save_root[0] ? save_root : snapshot->save_root);
    menu.selected_row = snapshot->selected_row;
    if (!dm2_v1_startup_menu_scan_saves(&menu)) {
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_from_menu(snapshot, &menu);
}

int dm2_v1_startup_menu_snapshot_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row)
{
    DM2_V1_StartupMenu menu;
    const char *root;

    if (!snapshot) {
        return 0;
    }
    root = save_root && save_root[0] ? save_root : fallback_save_root;
    dm2_v1_startup_menu_init(&menu, root);
    menu.selected_row = selected_row;
    if (!dm2_v1_startup_menu_refresh(&menu,
                                     resume_available,
                                     slot_mask)) {
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_from_menu(snapshot, &menu);
}

int dm2_v1_startup_menu_snapshot_scan_saves_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    const char *scan_save_root)
{
    if (!dm2_v1_startup_menu_snapshot_from_facts(snapshot,
                                                save_root,
                                                fallback_save_root,
                                                resume_available,
                                                slot_mask,
                                                selected_row)) {
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_scan_saves(snapshot,
                                                  scan_save_root);
}

int dm2_v1_startup_menu_state_receipt_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupMenuStateReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_menu_state_receipt_init(out_receipt);
    if (!snapshot) {
        return 0;
    }
    snprintf(out_receipt->save_root,
             sizeof(out_receipt->save_root),
             "%s",
             snapshot->save_root);
    out_receipt->resume_available = snapshot->resume_available ? 1 : 0;
    out_receipt->slot_mask = snapshot->slot_mask & 0x03ffu;
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

int dm2_v1_startup_menu_state_receipt_from_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row)
{
    DM2_V1_StartupMenuSnapshot snapshot;

    if (!dm2_v1_startup_menu_snapshot_from_facts(&snapshot,
                                                save_root,
                                                fallback_save_root,
                                                resume_available,
                                                slot_mask,
                                                selected_row)) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return dm2_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                           out_receipt);
}

int dm2_v1_startup_menu_state_receipt_scan_saves_from_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    const char *scan_save_root)
{
    DM2_V1_StartupMenuSnapshot snapshot;

    if (!dm2_v1_startup_menu_snapshot_scan_saves_from_facts(
            &snapshot,
            save_root,
            fallback_save_root,
            resume_available,
            slot_mask,
            selected_row,
            scan_save_root)) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return dm2_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                           out_receipt);
}

int dm2_v1_startup_menu_state_receipt_scan_saves_from_host_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts)
{
    if (!facts) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return dm2_v1_startup_menu_state_receipt_scan_saves_from_facts(
        out_receipt,
        facts->save_root,
        facts->fallback_save_root,
        facts->resume_available,
        facts->slot_mask,
        facts->selected_row,
        facts->scan_save_root);
}

int dm2_v1_startup_menu_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupMenu *out_menu)
{
    if (!snapshot || !out_menu) {
        return 0;
    }
    dm2_v1_startup_menu_init(
        out_menu,
        snapshot->save_root[0] ? snapshot->save_root : NULL);
    out_menu->selected_row = snapshot->selected_row;
    return dm2_v1_startup_menu_refresh(out_menu,
                                       snapshot->resume_available,
                                       snapshot->slot_mask);
}

int dm2_v1_startup_menu_snapshot_from_menu(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const DM2_V1_StartupMenu *menu)
{
    if (!snapshot || !menu) {
        return 0;
    }
    memset(snapshot, 0, sizeof(*snapshot));
    snprintf(snapshot->save_root,
             sizeof(snapshot->save_root),
             "%s",
             menu->save_root);
    snapshot->resume_available = menu->resume_available;
    snapshot->slot_mask = menu->slot_mask;
    snapshot->row_count = menu->row_count;
    snapshot->selected_row = menu->selected_row;
    return 1;
}

int dm2_v1_startup_menu_snapshot_row_at(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    int row,
    DM2_V1_StartupRowKind *out_kind,
    int *out_slot)
{
    DM2_V1_StartupMenu menu;

    if (!dm2_v1_startup_menu_from_snapshot(snapshot, &menu)) {
        if (out_kind) {
            *out_kind = DM2_V1_STARTUP_ROW_NONE;
        }
        if (out_slot) {
            *out_slot = -1;
        }
        return 0;
    }
    return dm2_v1_startup_menu_row_at(&menu, row, out_kind, out_slot);
}

int dm2_v1_startup_menu_snapshot_handle_input(
    DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupInput input,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupMenu menu;
    int handled;

    if (!snapshot) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    if (!dm2_v1_startup_menu_from_snapshot(snapshot, &menu)) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    handled = dm2_v1_startup_menu_handle_input(&menu, input, out_action);
    (void)dm2_v1_startup_menu_snapshot_from_menu(snapshot, &menu);
    return handled;
}

int dm2_v1_startup_menu_snapshot_handle_firestaff_input(
    DM2_V1_StartupMenuSnapshot *snapshot,
    int menu_input,
    DM2_V1_StartupAction *out_action)
{
    return dm2_v1_startup_menu_snapshot_handle_input(
        snapshot,
        dm2_v1_startup_input_from_firestaff_menu_code(menu_input),
        out_action);
}

int dm2_v1_startup_menu_handle_firestaff_input_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    DM2_V1_StartupAction *out_action)
{
    if (!dm2_v1_startup_menu_snapshot_from_facts(snapshot,
                                                save_root,
                                                fallback_save_root,
                                                resume_available,
                                                slot_mask,
                                                selected_row)) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_handle_firestaff_input(
        snapshot,
        menu_input,
        out_action);
}

int dm2_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupMenuSnapshot snapshot;

    if (!dm2_v1_startup_menu_handle_firestaff_input_from_facts(
            &snapshot,
            save_root,
            fallback_save_root,
            resume_available,
            slot_mask,
            selected_row,
            menu_input,
            out_action)) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return dm2_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                           out_receipt);
}

int dm2_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts,
    int menu_input,
    DM2_V1_StartupAction *out_action)
{
    if (!facts) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    return dm2_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
        out_receipt,
        facts->save_root,
        facts->fallback_save_root,
        facts->resume_available,
        facts->slot_mask,
        facts->selected_row,
        menu_input,
        out_action);
}

int dm2_v1_startup_menu_snapshot_handle_hit(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const DM2_V1_StartupHit *hit,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupMenu menu;
    int handled;

    if (!snapshot) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    if (!dm2_v1_startup_menu_from_snapshot(snapshot, &menu)) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    handled = dm2_v1_startup_menu_handle_hit(&menu, hit, out_action);
    (void)dm2_v1_startup_menu_snapshot_from_menu(snapshot, &menu);
    return handled;
}

int dm2_v1_startup_menu_snapshot_handle_pointer(
    DM2_V1_StartupMenuSnapshot *snapshot,
    int row_count,
    int x,
    int y,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupHit hit;

    if (!dm2_v1_startup_hit(row_count, x, y, &hit)) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_handle_hit(
        snapshot,
        &hit,
        out_action);
}

int dm2_v1_startup_menu_handle_pointer_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    DM2_V1_StartupAction *out_action)
{
    if (!dm2_v1_startup_menu_snapshot_from_facts(snapshot,
                                                save_root,
                                                fallback_save_root,
                                                resume_available,
                                                slot_mask,
                                                selected_row)) {
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    return dm2_v1_startup_menu_snapshot_handle_pointer(snapshot,
                                                       snapshot->row_count,
                                                       x,
                                                       y,
                                                       out_action);
}

int dm2_v1_startup_menu_handle_pointer_from_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupMenuSnapshot snapshot;

    if (!dm2_v1_startup_menu_handle_pointer_from_facts(&snapshot,
                                                       save_root,
                                                       fallback_save_root,
                                                       resume_available,
                                                       slot_mask,
                                                       selected_row,
                                                       x,
                                                       y,
                                                       out_action)) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        return 0;
    }
    return dm2_v1_startup_menu_state_receipt_from_snapshot(&snapshot,
                                                           out_receipt);
}

int dm2_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts,
    int x,
    int y,
    DM2_V1_StartupAction *out_action)
{
    if (!facts) {
        dm2_v1_startup_menu_state_receipt_init(out_receipt);
        dm2_v1_startup_action_clear(out_action);
        return 0;
    }
    return dm2_v1_startup_menu_handle_pointer_from_facts_with_receipt(
        out_receipt,
        facts->save_root,
        facts->fallback_save_root,
        facts->resume_available,
        facts->slot_mask,
        facts->selected_row,
        x,
        y,
        out_action);
}

int dm2_v1_startup_menu_row_at(const DM2_V1_StartupMenu *menu,
                               int row,
                               DM2_V1_StartupRowKind *out_kind,
                               int *out_slot)
{
    int cursor = 0;
    int slot;

    if (out_kind) {
        *out_kind = DM2_V1_STARTUP_ROW_NONE;
    }
    if (out_slot) {
        *out_slot = -1;
    }
    if (!menu || row < 0) {
        return 0;
    }
    if (menu->resume_available) {
        if (row == cursor) {
            if (out_kind) {
                *out_kind = DM2_V1_STARTUP_ROW_CONTINUE;
            }
            return 1;
        }
        ++cursor;
    }
    for (slot = 0; slot < 10; ++slot) {
        if ((menu->slot_mask & (1u << slot)) == 0u) {
            continue;
        }
        if (row == cursor) {
            if (out_kind) {
                *out_kind = DM2_V1_STARTUP_ROW_SLOT;
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
            *out_kind = DM2_V1_STARTUP_ROW_NEW_GAME;
        }
        return 1;
    }
    return 0;
}

int dm2_v1_startup_menu_move_selected(DM2_V1_StartupMenu *menu,
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

int dm2_v1_startup_menu_activate_selected(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupAction *out_action)
{
    DM2_V1_StartupRowKind row_kind = DM2_V1_STARTUP_ROW_NONE;
    int slot = -1;

    dm2_v1_startup_action_clear(out_action);
    if (!menu || !out_action ||
        !dm2_v1_startup_menu_row_at(menu,
                                    menu->selected_row,
                                    &row_kind,
                                    &slot)) {
        return 0;
    }
    out_action->row = menu->selected_row;
    out_action->slot = slot;
    if (row_kind == DM2_V1_STARTUP_ROW_CONTINUE) {
        out_action->kind = DM2_V1_STARTUP_ACTION_CONTINUE;
        return 1;
    }
    if (row_kind == DM2_V1_STARTUP_ROW_SLOT) {
        out_action->kind = DM2_V1_STARTUP_ACTION_LOAD_SLOT;
        return 1;
    }
    if (row_kind == DM2_V1_STARTUP_ROW_NEW_GAME) {
        out_action->kind = DM2_V1_STARTUP_ACTION_NEW_GAME;
        return 1;
    }
    return 0;
}

int dm2_v1_startup_menu_handle_input(DM2_V1_StartupMenu *menu,
                                     DM2_V1_StartupInput input,
                                     DM2_V1_StartupAction *out_action)
{
    dm2_v1_startup_action_clear(out_action);
    if (!menu || !out_action) {
        return 0;
    }
    if (input == DM2_V1_STARTUP_INPUT_UP) {
        (void)dm2_v1_startup_menu_move_selected(menu, -1);
        out_action->kind = DM2_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (input == DM2_V1_STARTUP_INPUT_DOWN) {
        (void)dm2_v1_startup_menu_move_selected(menu, 1);
        out_action->kind = DM2_V1_STARTUP_ACTION_NONE;
        return 1;
    }
    if (input == DM2_V1_STARTUP_INPUT_ACCEPT ||
        input == DM2_V1_STARTUP_INPUT_ACTION) {
        return dm2_v1_startup_menu_activate_selected(menu, out_action);
    }
    if (input == DM2_V1_STARTUP_INPUT_BACK) {
        out_action->kind = DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER;
        out_action->row = menu->selected_row;
        out_action->slot = -1;
        return 1;
    }
    return 0;
}

int dm2_v1_startup_menu_handle_hit(DM2_V1_StartupMenu *menu,
                                   const DM2_V1_StartupHit *hit,
                                   DM2_V1_StartupAction *out_action)
{
    dm2_v1_startup_action_clear(out_action);
    if (!menu || !hit || !out_action) {
        return 0;
    }
    if (hit->kind == DM2_V1_STARTUP_HIT_PANEL) {
        out_action->kind = DM2_V1_STARTUP_ACTION_NONE;
        out_action->row = menu->selected_row;
        return 1;
    }
    if (hit->kind != DM2_V1_STARTUP_HIT_ROW ||
        hit->row < 0 || hit->row >= menu->row_count) {
        return 0;
    }
    menu->selected_row = hit->row;
    return dm2_v1_startup_menu_activate_selected(menu, out_action);
}

int dm2_v1_startup_plan_for_action(
    const DM2_V1_StartupAction *action,
    DM2_V1_StartupActionPlan *out_plan)
{
    dm2_v1_startup_action_plan_clear(out_plan);
    if (!action || !out_plan) {
        return 0;
    }
    if (action->kind == DM2_V1_STARTUP_ACTION_NONE) {
        out_plan->kind = DM2_V1_STARTUP_PLAN_IGNORE;
        out_plan->success_status = "DM2 START SELECT";
        return 1;
    }
    if (action->kind == DM2_V1_STARTUP_ACTION_CONTINUE) {
        out_plan->kind = DM2_V1_STARTUP_PLAN_CONTINUE;
        out_plan->rescan_saves_on_failure = 1;
        out_plan->success_status = "DM2 CONTINUED";
        out_plan->failure_status = "DM2 CONTINUE FAILED";
        return 1;
    }
    if (action->kind == DM2_V1_STARTUP_ACTION_LOAD_SLOT &&
        action->slot >= 0) {
        out_plan->kind = DM2_V1_STARTUP_PLAN_LOAD_SLOT;
        out_plan->slot = action->slot;
        out_plan->rescan_saves_on_failure = 1;
        out_plan->success_status = "DM2 SLOT LOADED";
        out_plan->failure_status = "DM2 SLOT LOAD FAILED";
        return 1;
    }
    if (action->kind == DM2_V1_STARTUP_ACTION_NEW_GAME) {
        out_plan->kind = DM2_V1_STARTUP_PLAN_NEW_GAME;
        out_plan->success_status = "DM2 NEW GAME";
        return 1;
    }
    if (action->kind == DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER) {
        out_plan->kind = DM2_V1_STARTUP_PLAN_RETURN_TO_LAUNCHER;
        out_plan->success_status = "BACK TO LAUNCHER";
        return 1;
    }
    return 0;
}

int dm2_v1_startup_execute_plan(
    const DM2_V1_StartupActionPlan *plan,
    const char *save_root,
    DM2_V1_StartupExecution *out_execution)
{
    if (!out_execution) {
        return 0;
    }
    dm2_v1_startup_execution_clear(out_execution);
    if (!plan) {
        return 0;
    }
    if (plan->kind == DM2_V1_STARTUP_PLAN_IGNORE) {
        out_execution->kind = DM2_V1_STARTUP_EXEC_STATUS_REDRAW;
        out_execution->status = plan->success_status;
        return 1;
    }
    if (plan->kind == DM2_V1_STARTUP_PLAN_CONTINUE) {
        if (dm2_v1_session_load_last_session(
                save_root,
                &out_execution->session) != 0) {
            out_execution->kind = DM2_V1_STARTUP_EXEC_STATUS_REDRAW;
            out_execution->status = plan->failure_status;
            out_execution->rescan_saves = plan->rescan_saves_on_failure;
            return 1;
        }
        out_execution->kind = DM2_V1_STARTUP_EXEC_SESSION_READY;
        out_execution->status = plan->success_status;
        return 1;
    }
    if (plan->kind == DM2_V1_STARTUP_PLAN_LOAD_SLOT &&
        plan->slot >= 0) {
        if (dm2_v1_session_load_slot(
                save_root,
                (uint8_t)plan->slot,
                &out_execution->session) != 0) {
            out_execution->kind = DM2_V1_STARTUP_EXEC_STATUS_REDRAW;
            out_execution->status = plan->failure_status;
            out_execution->rescan_saves = plan->rescan_saves_on_failure;
            return 1;
        }
        out_execution->kind = DM2_V1_STARTUP_EXEC_SESSION_READY;
        out_execution->status = plan->success_status;
        return 1;
    }
    if (plan->kind == DM2_V1_STARTUP_PLAN_NEW_GAME) {
        dm2_v1_session_new(&out_execution->session);
        out_execution->kind = DM2_V1_STARTUP_EXEC_SESSION_READY;
        out_execution->status = plan->success_status;
        return 1;
    }
    if (plan->kind == DM2_V1_STARTUP_PLAN_RETURN_TO_LAUNCHER) {
        out_execution->kind = DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER;
        out_execution->status = plan->success_status;
        return 1;
    }
    return 0;
}

int dm2_v1_startup_execute_action(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupExecution *out_execution)
{
    DM2_V1_StartupActionPlan plan;

    if (!out_execution) {
        return 0;
    }
    dm2_v1_startup_execution_clear(out_execution);
    if (!dm2_v1_startup_plan_for_action(action, &plan)) {
        return 0;
    }
    return dm2_v1_startup_execute_plan(&plan, save_root, out_execution);
}

int dm2_v1_startup_execution_mode_update(
    const DM2_V1_StartupExecution *execution,
    DM2_V1_StartupModeUpdate *out_update)
{
    if (!execution || !out_update ||
        execution->kind == DM2_V1_STARTUP_EXEC_IGNORE) {
        return 0;
    }
    dm2_v1_startup_mode_update_clear(out_update);
    if (execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY) {
        out_update->set_startup_menu_active = 1;
        out_update->startup_menu_active = 0;
    }
    return 1;
}

int dm2_v1_startup_execution_input_outcome(
    const DM2_V1_StartupExecution *execution,
    int session_applied,
    DM2_V1_StartupInputOutcome *out_outcome)
{
    if (!out_outcome) {
        return 0;
    }
    dm2_v1_startup_input_outcome_clear(out_outcome);
    if (!execution || execution->kind == DM2_V1_STARTUP_EXEC_IGNORE) {
        return 0;
    }
    if (execution->kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW) {
        out_outcome->result = DM2_V1_STARTUP_INPUT_RESULT_REDRAW;
        out_outcome->rescan_saves = execution->rescan_saves;
        out_outcome->status_scope = "STARTUP";
        out_outcome->status = execution->status
            ? execution->status
            : "DM2 START SELECT";
        return 1;
    }
    if (execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY) {
        out_outcome->result = DM2_V1_STARTUP_INPUT_RESULT_REDRAW;
        out_outcome->status_scope = "STARTUP";
        out_outcome->status = session_applied
            ? (execution->status ? execution->status : "DM2 STARTED")
            : "DM2 LOAD FAILED";
        return 1;
    }
    if (execution->kind == DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER) {
        out_outcome->result =
            DM2_V1_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER;
        out_outcome->status_scope = "RETURN";
        out_outcome->status = execution->status
            ? execution->status
            : "BACK TO LAUNCHER";
        return 1;
    }
    return 0;
}

int dm2_v1_startup_apply_receipt_from_execution(
    const DM2_V1_StartupExecution *execution,
    int session_applied,
    DM2_V1_StartupApplyReceipt *out_receipt)
{
    DM2_V1_StartupModeUpdate mode_update;
    DM2_V1_StartupInputOutcome outcome;

    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_apply_receipt_clear(out_receipt);
    if (!execution || execution->kind == DM2_V1_STARTUP_EXEC_IGNORE) {
        return 0;
    }
    if (!dm2_v1_startup_execution_mode_update(execution, &mode_update) ||
        !dm2_v1_startup_execution_input_outcome(
            execution,
            session_applied,
            &outcome)) {
        return 0;
    }
    out_receipt->session_should_apply =
        execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY ? 1 : 0;
    out_receipt->session_applied = session_applied ? 1 : 0;
    if (out_receipt->session_should_apply && !out_receipt->session_applied) {
        memset(&mode_update, 0, sizeof(mode_update));
    }
    out_receipt->mode_update = mode_update;
    out_receipt->outcome = outcome;
    return 1;
}

int dm2_v1_startup_execute_action_with_receipt(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupApplyReceipt *out_receipt)
{
    DM2_V1_StartupExecution local_execution;
    DM2_V1_StartupExecution *execution;
    int session_applied = 0;

    if (out_execution) {
        dm2_v1_startup_execution_clear(out_execution);
    }
    if (out_receipt) {
        dm2_v1_startup_apply_receipt_clear(out_receipt);
    }
    if (!action || !out_receipt) {
        return 0;
    }

    execution = out_execution ? out_execution : &local_execution;
    if (!dm2_v1_startup_execute_action(action, save_root, execution)) {
        return 0;
    }
    if (execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY &&
        apply_session) {
        session_applied = apply_session(apply_userdata,
                                        &execution->session);
    }
    return dm2_v1_startup_apply_receipt_from_execution(execution,
                                                       session_applied,
                                                       out_receipt);
}

int dm2_v1_startup_execute_action_with_host_receipt(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostReceipt *out_host_receipt)
{
    DM2_V1_StartupApplyReceipt apply_receipt;

    dm2_v1_startup_apply_receipt_clear(&apply_receipt);
    if (out_host_receipt) {
        dm2_v1_startup_host_receipt_clear(out_host_receipt);
    }
    if (!dm2_v1_startup_execute_action_with_receipt(action,
                                                    save_root,
                                                    apply_session,
                                                    apply_userdata,
                                                    out_execution,
                                                    &apply_receipt)) {
        return 0;
    }
    if (out_host_receipt &&
        !dm2_v1_startup_host_receipt_from_apply_receipt(&apply_receipt,
                                                        out_host_receipt)) {
        return 0;
    }
    return 1;
}

void dm2_v1_startup_host_action_receipt_clear(
    DM2_V1_StartupHostActionReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    dm2_v1_startup_host_receipt_clear(&receipt->host_receipt);
    dm2_v1_startup_menu_state_receipt_init(&receipt->menu_state_receipt);
}

static void dm2_v1_startup_save_menu_handoff_set(
    DM2_V1_StartupHostActionReceipt *receipt,
    const DM2_V1_StartupAction *action,
    const DM2_V1_StartupActionPlan *plan,
    const DM2_V1_StartupExecution *execution,
    const char *save_root)
{
    DM2_V1_StartupSaveMenuHandoffReceipt *handoff;

    if (!receipt || !plan || !execution) {
        return;
    }
    handoff = &receipt->save_menu_handoff;
    memset(handoff, 0, sizeof(*handoff));
    handoff->valid = 1;
    handoff->action_kind = action
        ? action->kind
        : DM2_V1_STARTUP_ACTION_NONE;
    handoff->plan_kind = plan->kind;
    handoff->execution_kind = execution->kind;
    handoff->source_row = action ? action->row : -1;
    handoff->source_slot = action ? action->slot : -1;
    snprintf(handoff->save_root,
             sizeof(handoff->save_root),
             "%s",
             save_root ? save_root : "");
    handoff->rescan_saves = execution->rescan_saves ? 1 : 0;
    handoff->session_ready =
        execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY ? 1 : 0;
    handoff->return_to_launcher =
        execution->kind == DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER ? 1 : 0;
    /* SKULL.ASM T200 owns DM2 save namespace resolution; keep the resolved
     * menu save root and action plan beside the host receipt so M11 need not
     * rebuild load/continue/new-game routing from strings. */
}

static void dm2_v1_startup_input_route_set(
    DM2_V1_StartupHostActionReceipt *receipt,
    int source_kind,
    int firestaff_menu_input,
    DM2_V1_StartupInput startup_input,
    int pointer_x,
    int pointer_y,
    const DM2_V1_StartupHostFacts *facts,
    const DM2_V1_StartupMenuStateReceipt *menu_receipt,
    const DM2_V1_StartupAction *action)
{
    DM2_V1_StartupInputRouteReceipt *route;

    if (!receipt || !facts || !menu_receipt || !action) {
        return;
    }
    route = &receipt->input_route;
    memset(route, 0, sizeof(*route));
    route->valid = 1;
    route->source_kind = source_kind;
    route->firestaff_menu_input = firestaff_menu_input;
    route->startup_input = startup_input;
    route->pointer_x = pointer_x;
    route->pointer_y = pointer_y;
    route->selected_row_before = facts->selected_row;
    route->selected_row_after = menu_receipt->selected_row;
    route->action_kind = action->kind;
    route->action_row = action->row;
    route->action_slot = action->slot;
    /* SKULL.ASM T000 owns the startup menu loop. Keep the input translation
     * and resulting DM2 startup action together so host HUD/menu routing does
     * not reinterpret Firestaff key or pointer codes. */
}

static void dm2_v1_startup_host_menu_route_set(
    DM2_V1_StartupHostActionReceipt *receipt,
    const DM2_V1_StartupExecution *execution,
    const DM2_V1_StartupMenuStateReceipt *menu_receipt,
    const DM2_V1_StartupHostFacts *facts)
{
    DM2_V1_StartupHostMenuRouteReceipt *route;

    if (!receipt || !execution || !facts) {
        return;
    }
    route = &receipt->host_menu_route;
    memset(route, 0, sizeof(*route));
    route->valid = 1;
    route->consume_input = 1;
    route->redraw_startup_menu =
        receipt->host_receipt.input_result == DM2_V1_STARTUP_HOST_INPUT_REDRAW
            ? 1 : 0;
    route->close_startup_menu =
        receipt->host_receipt.mode_update.set_startup_menu_active &&
        receipt->host_receipt.mode_update.startup_menu_active == 0;
    route->return_to_launcher =
        receipt->host_receipt.input_result ==
            DM2_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER ? 1 : 0;
    route->apply_session =
        execution->kind == DM2_V1_STARTUP_EXEC_SESSION_READY ? 1 : 0;
    route->rescan_saves = receipt->host_receipt.rescan_saves ? 1 : 0;
    route->selected_row_after =
        menu_receipt ? menu_receipt->selected_row : facts->selected_row;
    route->runtime_menu_ready =
        !route->return_to_launcher && !route->close_startup_menu;
    route->runtime_action_ready =
        execution->kind != DM2_V1_STARTUP_EXEC_IGNORE ? 1 : 0;
    route->first_hud_frame_ready =
        route->close_startup_menu &&
        route->apply_session &&
        receipt->host_receipt.input_result ==
            DM2_V1_STARTUP_HOST_INPUT_REDRAW;
    route->status_scope = receipt->host_receipt.status_scope;
    route->status = receipt->host_receipt.status;
    /* DM2-owned M11/M12 route: callers can now use these flags directly
     * instead of re-reading selected-row, status text, execution kind, or
     * the first runtime HUD-frame boundary. */
}

void dm2_v1_startup_idle_receipt_clear(
    DM2_V1_StartupIdleReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    dm2_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void dm2_v1_startup_launch_receipt_clear(
    DM2_V1_StartupLaunchReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    dm2_v1_startup_host_receipt_clear(&receipt->host_receipt);
    dm2_v1_startup_menu_state_receipt_init(&receipt->menu_state_receipt);
}

int dm2_v1_startup_runtime_handoff_receipt_from_state(
    DM2_V1_StartupRuntimeHandoffReceipt *out_receipt,
    int startup_menu_active,
    int hud_runtime_ready)
{
    int active;

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    active = startup_menu_active ? 1 : 0;
    out_receipt->valid = 1;
    out_receipt->startup_menu_active = active;
    out_receipt->animation_active = active;
    snprintf(out_receipt->animation,
             sizeof(out_receipt->animation),
             "%s",
             active ? "dm2-startup-menu" : "dm2-runtime");
    out_receipt->title_frame = 0;
    out_receipt->title_frame_max = 0;
    out_receipt->title_ready = active ? 0 : 1;
    /* skproject/SKWIN SkWinCore startup keeps title/menu presentation and
     * HUD/game runtime as one boot handoff; Firestaff records the same
     * boundary here so M11 does not infer HUD/runtime init from text status. */
    out_receipt->initialize_v2_runtime = 1;
    out_receipt->initialize_hud_runtime = 1;
    out_receipt->initialize_touch_runtime = 1;
    out_receipt->hud_runtime_ready = hud_runtime_ready ? 1 : 0;
    out_receipt->runtime_menu_ready = active;
    out_receipt->runtime_action_ready = active ? 0 : 1;
    out_receipt->first_hud_frame_ready = active ? 0 : 1;
    return 1;
}

void dm2_v1_startup_direct_resume_receipt_clear(
    DM2_V1_StartupDirectResumeReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    dm2_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

int dm2_v1_startup_launch_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    DM2_V1_StartupLaunchReceipt *out_receipt)
{
    if (out_receipt) {
        dm2_v1_startup_launch_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        if (out_receipt) {
            out_receipt->host_receipt.status_scope = "BOOT";
            out_receipt->host_receipt.status = "DM2 START MENU FAILED";
        }
        return 0;
    }
    dm2_v1_session_new(&out_receipt->session);
    out_receipt->session_valid = 1;
    if (dm2_v1_startup_menu_state_receipt_scan_saves_from_host_facts(
            &out_receipt->menu_state_receipt,
            facts)) {
        out_receipt->menu_state_receipt_valid = 1;
    }
    out_receipt->host_receipt.mode_update.set_startup_menu_active = 1;
    out_receipt->host_receipt.mode_update.startup_menu_active = 1;
    out_receipt->host_receipt.input_result =
        DM2_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "BOOT";
    out_receipt->host_receipt.status = "DM2 START MENU";
    out_receipt->host_receipt.inspect_scope = "READY";
    out_receipt->host_receipt.inspect_detail =
        "DM2 V1 ASSETS VERIFIED; V2 RUNTIMES LIVE";
    out_receipt->host_receipt.log_line = "T0: DM2 START MENU";
    (void)dm2_v1_startup_runtime_handoff_receipt_from_state(
        &out_receipt->runtime_handoff,
        out_receipt->host_receipt.mode_update.startup_menu_active,
        1);
    return 1;
}

int dm2_v1_startup_advance_idle_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    int mouth_redraw,
    DM2_V1_StartupIdleReceipt *out_receipt)
{
    if (out_receipt) {
        dm2_v1_startup_idle_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt || !facts->startup_menu_active) {
        return 0;
    }
    out_receipt->host_receipt.input_result =
        mouth_redraw ? DM2_V1_STARTUP_HOST_INPUT_REDRAW
                     : DM2_V1_STARTUP_HOST_INPUT_IGNORED;
    out_receipt->host_receipt.status_scope = "STARTUP";
    out_receipt->host_receipt.status = "DM2 STARTUP MENU";
    return 1;
}

int dm2_v1_startup_execute_action_from_host_facts_with_receipt(
    const DM2_V1_StartupAction *action,
    const DM2_V1_StartupHostFacts *facts,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    const char *save_root;
    DM2_V1_StartupActionPlan plan;
    DM2_V1_StartupExecution local_execution;
    DM2_V1_StartupExecution *execution;

    if (out_receipt) {
        dm2_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        return 0;
    }
    dm2_v1_startup_action_plan_clear(&plan);
    if (!dm2_v1_startup_plan_for_action(action, &plan)) {
        return 0;
    }

    save_root = facts->fallback_save_root && facts->fallback_save_root[0]
                    ? facts->fallback_save_root
                    : facts->save_root;
    execution = out_execution ? out_execution : &local_execution;
    if (!dm2_v1_startup_execute_action_with_host_receipt(
            action,
            save_root,
            apply_session,
            apply_userdata,
            execution,
            &out_receipt->host_receipt)) {
        return 0;
    }
    if (out_receipt->host_receipt.rescan_saves &&
        dm2_v1_startup_menu_state_receipt_scan_saves_from_host_facts(
            &out_receipt->menu_state_receipt,
            facts)) {
        out_receipt->menu_state_receipt_valid = 1;
    }
    dm2_v1_startup_save_menu_handoff_set(out_receipt,
                                         action,
                                         &plan,
                                         execution,
                                         save_root);
    out_receipt->save_menu_handoff.menu_state_receipt_valid =
        out_receipt->menu_state_receipt_valid;
    out_receipt->save_menu_handoff.selected_row_after =
        out_receipt->menu_state_receipt_valid
            ? out_receipt->menu_state_receipt.selected_row
            : facts->selected_row;
    dm2_v1_startup_host_menu_route_set(
        out_receipt,
        execution,
        out_receipt->menu_state_receipt_valid
            ? &out_receipt->menu_state_receipt
            : NULL,
        facts);
    return 1;
}

int dm2_v1_startup_execute_firestaff_input_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    int menu_input,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    DM2_V1_StartupAction action;
    DM2_V1_StartupMenuStateReceipt menu_receipt;

    if (out_execution) {
        dm2_v1_startup_execution_clear(out_execution);
    }
    if (out_receipt) {
        dm2_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        dm2_v1_startup_action_clear(&action);
        return 0;
    }
    if (!dm2_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
            &menu_receipt,
            facts,
            menu_input,
            &action)) {
        return 0;
    }
    if (!dm2_v1_startup_execute_action_from_host_facts_with_receipt(
            &action,
            facts,
            apply_session,
            apply_userdata,
            out_execution,
            out_receipt)) {
        return 0;
    }
    if (!out_receipt->menu_state_receipt_valid) {
        out_receipt->menu_state_receipt = menu_receipt;
        out_receipt->menu_state_receipt_valid = 1;
    }
    if (out_receipt->host_menu_route.valid) {
        out_receipt->host_menu_route.selected_row_after =
            out_receipt->menu_state_receipt.selected_row;
    }
    dm2_v1_startup_input_route_set(
        out_receipt,
        1,
        menu_input,
        dm2_v1_startup_input_from_firestaff_menu_code(menu_input),
        -1,
        -1,
        facts,
        &menu_receipt,
        &action);
    return 1;
}

int dm2_v1_startup_execute_pointer_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    int x,
    int y,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt)
{
    DM2_V1_StartupAction action;
    DM2_V1_StartupMenuStateReceipt menu_receipt;

    if (out_execution) {
        dm2_v1_startup_execution_clear(out_execution);
    }
    if (out_receipt) {
        dm2_v1_startup_host_action_receipt_clear(out_receipt);
    }
    if (!facts || !out_receipt) {
        dm2_v1_startup_action_clear(&action);
        return 0;
    }
    if (!dm2_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
            &menu_receipt,
            facts,
            x,
            y,
            &action)) {
        return 0;
    }
    if (!dm2_v1_startup_execute_action_from_host_facts_with_receipt(
            &action,
            facts,
            apply_session,
            apply_userdata,
            out_execution,
            out_receipt)) {
        return 0;
    }
    if (!out_receipt->menu_state_receipt_valid) {
        out_receipt->menu_state_receipt = menu_receipt;
        out_receipt->menu_state_receipt_valid = 1;
    }
    if (out_receipt->host_menu_route.valid) {
        out_receipt->host_menu_route.selected_row_after =
            out_receipt->menu_state_receipt.selected_row;
    }
    dm2_v1_startup_input_route_set(out_receipt,
                                   2,
                                   0,
                                   DM2_V1_STARTUP_INPUT_NONE,
                                   x,
                                   y,
                                   facts,
                                   &menu_receipt,
                                   &action);
    return 1;
}

void dm2_v1_startup_host_receipt_clear(
    DM2_V1_StartupHostReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = DM2_V1_STARTUP_HOST_INPUT_IGNORED;
}

int dm2_v1_startup_host_receipt_from_apply_receipt(
    const DM2_V1_StartupApplyReceipt *apply_receipt,
    DM2_V1_StartupHostReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_host_receipt_clear(out_receipt);
    if (!apply_receipt) {
        return 0;
    }

    out_receipt->mode_update = apply_receipt->mode_update;
    out_receipt->rescan_saves = apply_receipt->outcome.rescan_saves;
    out_receipt->status_scope = apply_receipt->outcome.status_scope;
    out_receipt->status = apply_receipt->outcome.status;
    switch (apply_receipt->outcome.result) {
    case DM2_V1_STARTUP_INPUT_RESULT_REDRAW:
        out_receipt->input_result = DM2_V1_STARTUP_HOST_INPUT_REDRAW;
        return 1;
    case DM2_V1_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER:
        out_receipt->input_result =
            DM2_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER;
        return 1;
    case DM2_V1_STARTUP_INPUT_RESULT_IGNORED:
    default:
        out_receipt->input_result = DM2_V1_STARTUP_HOST_INPUT_IGNORED;
        return 1;
    }
}

int dm2_v1_startup_resume_status_host_receipt(
    DM2_V1_StartupResumeStatus status,
    DM2_V1_StartupHostReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_host_receipt_clear(out_receipt);
    out_receipt->status_scope = "BOOT";
    switch (status) {
    case DM2_V1_STARTUP_RESUME_STATUS_PATH_INVALID:
        out_receipt->status = "DM2 RESUME PATH INVALID";
        break;
    case DM2_V1_STARTUP_RESUME_STATUS_FAILED:
        out_receipt->status = "DM2 RESUME FAILED";
        break;
    case DM2_V1_STARTUP_RESUME_STATUS_RESUMED:
        out_receipt->status = "DM2 RESUMED";
        out_receipt->inspect_scope = "READY";
        out_receipt->inspect_detail =
            "DM2 V1 ASSETS VERIFIED; V2 RUNTIMES LIVE";
        out_receipt->log_line = "T0: DM2 RESUMED";
        out_receipt->input_result = DM2_V1_STARTUP_HOST_INPUT_REDRAW;
        break;
    default:
        out_receipt->status = "DM2 RESUME FAILED";
        break;
    }
    return 1;
}

int dm2_v1_startup_execute_save_path(
    const char *save_path,
    char *out_save_root,
    int out_save_root_cap,
    DM2_V1_StartupExecution *out_execution)
{
    DM2_V1_StartupSavePathResult result;

    if (!out_execution) {
        return 0;
    }
    dm2_v1_startup_execution_clear(out_execution);
    result = dm2_v1_startup_load_session_from_save_path(
        save_path,
        out_save_root,
        out_save_root_cap,
        &out_execution->session,
        NULL,
        NULL);
    if (result == DM2_V1_STARTUP_SAVE_PATH_INVALID) {
        out_execution->kind = DM2_V1_STARTUP_EXEC_STATUS_REDRAW;
        out_execution->status = "DM2 RESUME PATH INVALID";
        return 1;
    }
    if (result != DM2_V1_STARTUP_SAVE_PATH_LOADED) {
        out_execution->kind = DM2_V1_STARTUP_EXEC_STATUS_REDRAW;
        out_execution->status = "DM2 RESUME FAILED";
        return 1;
    }
    out_execution->kind = DM2_V1_STARTUP_EXEC_SESSION_READY;
    out_execution->status = "DM2 RESUMED";
    return 1;
}

int dm2_v1_startup_execute_save_path_with_host_receipt(
    const char *save_path,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupDirectResumeReceipt *out_receipt)
{
    char save_root[512];
    DM2_V1_StartupExecution local_execution;
    DM2_V1_StartupExecution *execution;
    DM2_V1_StartupResumeStatus status;
    int session_applied = 0;

    if (!out_receipt) {
        return 0;
    }
    dm2_v1_startup_direct_resume_receipt_clear(out_receipt);
    execution = out_execution ? out_execution : &local_execution;
    memset(save_root, 0, sizeof(save_root));
    if (!dm2_v1_startup_execute_save_path(save_path,
                                          save_root,
                                          (int)sizeof(save_root),
                                          execution)) {
        (void)dm2_v1_startup_resume_status_host_receipt(
            DM2_V1_STARTUP_RESUME_STATUS_FAILED,
            &out_receipt->host_receipt);
        return 0;
    }
    if (save_root[0] != '\0') {
        snprintf(out_receipt->save_root,
                 sizeof(out_receipt->save_root),
                 "%s",
                 save_root);
        out_receipt->save_root_valid = 1;
    }
    if (execution->kind != DM2_V1_STARTUP_EXEC_SESSION_READY) {
        status = execution->kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
                         execution->status &&
                         strcmp(execution->status,
                                "DM2 RESUME PATH INVALID") == 0
                     ? DM2_V1_STARTUP_RESUME_STATUS_PATH_INVALID
                     : DM2_V1_STARTUP_RESUME_STATUS_FAILED;
        (void)dm2_v1_startup_resume_status_host_receipt(
            status,
            &out_receipt->host_receipt);
        return 1;
    }
    out_receipt->session_ready = 1;
    if (apply_session) {
        session_applied = apply_session(apply_userdata, &execution->session);
    }
    if (!session_applied) {
        (void)dm2_v1_startup_resume_status_host_receipt(
            DM2_V1_STARTUP_RESUME_STATUS_FAILED,
            &out_receipt->host_receipt);
        return 1;
    }
    out_receipt->session_applied = 1;
    (void)dm2_v1_startup_resume_status_host_receipt(
        DM2_V1_STARTUP_RESUME_STATUS_RESUMED,
        &out_receipt->host_receipt);
    return 1;
}

int dm2_v1_startup_menu_build_render_rows(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupRenderRow *rows,
    int max_rows)
{
    int row;
    int count = 0;

    if (!menu || !rows || max_rows <= 0) {
        return 0;
    }
    memset(rows, 0, (size_t)max_rows * sizeof(rows[0]));
    for (row = 0; row < menu->row_count && count < max_rows; ++row) {
        DM2_V1_StartupRowKind kind = DM2_V1_STARTUP_ROW_NONE;
        int slot = -1;
        DM2_V1_StartupRenderRow *out = &rows[count];

        if (!dm2_v1_startup_menu_row_at(menu, row, &kind, &slot) ||
            !dm2_v1_startup_row_rect(row, &out->rect)) {
            continue;
        }
        (void)dm2_v1_startup_row_highlight_rect(row, &out->highlight_rect);
        out->kind = kind;
        out->row = row;
        out->slot = slot;
        out->selected = (row == menu->selected_row) ? 1 : 0;
        out->text_x = DM2_V1_STARTUP_ROW_TEXT_X;
        out->text_y = out->rect.y + 2;
        if (kind == DM2_V1_STARTUP_ROW_CONTINUE) {
            snprintf(out->label, sizeof(out->label), "CONTINUE");
        } else if (kind == DM2_V1_STARTUP_ROW_SLOT) {
            snprintf(out->label, sizeof(out->label), "LOAD SLOT %02d", slot);
        } else if (kind == DM2_V1_STARTUP_ROW_NEW_GAME) {
            snprintf(out->label, sizeof(out->label), "NEW GAME");
        }
        ++count;
    }
    return count;
}

int dm2_v1_startup_receipt_phase(int startup_menu_active,
                                 char *out_phase,
                                 int out_phase_size,
                                 int *out_startup_active)
{
    const char *phase = startup_menu_active
        ? "dm2-startup-menu"
        : "dm2-runtime";

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    snprintf(out_phase, (size_t)out_phase_size, "%s", phase);
    if (out_startup_active) {
        *out_startup_active = startup_menu_active ? 1 : 0;
    }
    return 1;
}
