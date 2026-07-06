#include "nexus_v1_startup_menu.h"
#include "nexus_v1_title.h"

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
    menu->row_count = nexus_v1_startup_row_count(menu->slot_mask);
    if (menu->selected_row < 0 || menu->selected_row >= menu->row_count) {
        menu->selected_row = 0;
    }
    return 0;
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
    if (!nexus_title_start_ready(title_frame)) {
        out_action->kind = NEXUS_V1_STARTUP_ACTION_HOLD_TITLE;
        return 1;
    }
    out_action->kind = slot_mask != 0u
                           ? NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT
                           : NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT;
    return 1;
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
