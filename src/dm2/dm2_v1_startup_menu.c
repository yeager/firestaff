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
