#include "dm2_v1_startup_menu.h"

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
