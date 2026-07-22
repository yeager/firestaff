#include "nexus_v1_click_route.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_squares.h"
#include <stdlib.h>

/* Nexus V1 mouse click-route dispatch.
 *
 * Maps UI click targets into the existing keyboard-style command queue so
 * both input paths converge on nexus_mechanics_tick().  The layer is
 * intentionally thin: it does not resolve viewport pixels or 3D projection;
 * callers that already know the logical target (inventory slot, grid square,
 * floor item) hand it off here.
 *
 * Source: DM1 COMMAND.C mouse/click dispatch; CLIKMENU.C F0366 command queue;
 *         MOVESENS.C F0267/F0268 square interaction. */

/* ── Helper: push one command if queue space exists. ── */
static int click_route_push(Nexus_MechanicsState *st, int cmd) {
    if (!st) return -1;
    return nexus_mechanics_push_command(st, cmd);
}

/* ── Helper: push the turn that makes |target_dir| the new facing. ── */
static void click_route_push_turn_to(Nexus_MechanicsState *st,
                                      int current_dir, int target_dir) {
    int diff;
    if (!st) return;
    diff = nexus_dir_diff(current_dir, target_dir);
    if (diff == 0) return;
    if (diff == 1 || diff == -3) {
        /* target is 90° to the right */
        click_route_push(st, NEXUS_CMD_TURN_RIGHT);
    } else if (diff == -1 || diff == 2) {
        /* target is 90° to the left (or directly behind: turn left twice) */
        click_route_push(st, NEXUS_CMD_TURN_LEFT);
    }
}

/* ── Inventory / equipment slot click ── */
static Nexus_ClickResult click_route_inventory(Nexus_MechanicsState *st,
                                                Nexus_V1_Engine *engine,
                                                const Nexus_ClickTarget *target) {
    int champ_idx;
    int item_id;
    const Nexus_ItemDef *def;

    if (!st || !engine || !target) return NEXUS_CLICK_RESULT_INVALID;
    if (target->champion_index < 0 ||
        target->champion_index >= engine->champions.champion_count)
        return NEXUS_CLICK_RESULT_INVALID;
    champ_idx = target->champion_index;

    if (target->kind == NEXUS_CLICK_TARGET_INVENTORY_SLOT) {
        if (target->slot < 0 || target->slot >= NEXUS_INVENTORY_SLOTS)
            return NEXUS_CLICK_RESULT_INVALID;
        item_id = engine->champions.champions[champ_idx].inventory[target->slot];
        if (item_id < 0) return NEXUS_CLICK_RESULT_NO_ACTION;
        def = nexus_itemdef_get(item_id);
        if (!def) return NEXUS_CLICK_RESULT_NO_ACTION;
        if ((def->flags & (NEXUS_ITEMF_CONSUMABLE | NEXUS_ITEMF_EQUIPPABLE)) == 0)
            return NEXUS_CLICK_RESULT_NO_ACTION; /* keys, runes, misc have no V1 click-use */
        nexus_mechanics_set_use_item_slot(st, target->slot);
        if (click_route_push(st, NEXUS_CMD_USE_ITEM) < 0)
            return NEXUS_CLICK_RESULT_BLOCKED;
        return NEXUS_CLICK_RESULT_OK;
    }

    /* Equipment slot: for V1, treat as "unequip and return to inventory".
     * This mirrors the DM1 inventory-panel click on an equipped item. */
    if (target->kind == NEXUS_CLICK_TARGET_EQUIPMENT_SLOT) {
        Nexus_V1_Champion *champ = &engine->champions.champions[champ_idx];
        int equip_slot = target->slot - 1; /* NEXUS_SLOT_* is 1-based */
        if (equip_slot < 0 || equip_slot >= NEXUS_EQUIP_SLOTS)
            return NEXUS_CLICK_RESULT_INVALID;
        if (champ->slots[equip_slot] < 0)
            return NEXUS_CLICK_RESULT_NO_ACTION;
        /* V1: clear the equipment slot and try to return the item to inventory.
         * Source: CHAMPION.C F0309 slot layout; COMMAND.C equipment clicks.
         * Champion inventory is a flat uint8_t[30] item-id array; do not use
         * the Nexus_InventorySlot helper here. */
        int i;
        item_id = champ->slots[equip_slot];
        for (i = 0; i < NEXUS_INVENTORY_SLOTS; i++) {
            if (champ->inventory[i] == 0xFFU) {
                champ->inventory[i] = (uint8_t)item_id;
                champ->slots[equip_slot] = (uint8_t)-1;
                nexus_champion_recalc_load(champ);
                return NEXUS_CLICK_RESULT_OK;
            }
        }
        /* Inventory full: equipment stays equipped (fail-closed). */
        return NEXUS_CLICK_RESULT_BLOCKED;
    }

    return NEXUS_CLICK_RESULT_INVALID;
}

/* ── World square / door click: move toward or open. ── */
static Nexus_ClickResult click_route_world_square(Nexus_MechanicsState *st,
                                                   Nexus_V1_Engine *engine,
                                                   const Nexus_ClickTarget *target) {
    int dx, dy, target_dir, current_dir;
    int sq;

    if (!st || !engine || !target) return NEXUS_CLICK_RESULT_INVALID;

    dx = target->x - st->party_x;
    dy = target->y - st->party_y;

    /* Already there: nothing to do. */
    if (dx == 0 && dy == 0) return NEXUS_CLICK_RESULT_NO_ACTION;

    sq = nexus_v1_level_get_square(&engine->current_level, target->x, target->y);

    /* Plain world-square click on a wall is a no-path.  Door clicks are
     * allowed because the step logic will attempt to open them. */
    if (target->kind == NEXUS_CLICK_TARGET_WORLD_SQUARE && sq == NEXUS_SQUARE_WALL)
        return NEXUS_CLICK_RESULT_NO_PATH;

    /* Determine the dominant cardinal direction toward the target.
     * Prefer the axis with larger offset; ties prefer y movement.
     * Source: DM1 viewport click routing resolves to one of four cardinal
     *         directions before stepping. */
    if (abs(dx) > abs(dy)) {
        target_dir = (dx > 0) ? NEXUS_DIR_EAST : NEXUS_DIR_WEST;
    } else {
        target_dir = (dy > 0) ? NEXUS_DIR_SOUTH : NEXUS_DIR_NORTH;
    }

    current_dir = st->party_dir;

    /* If already facing the target, step forward.  The UI can re-issue the
     * click after each tick to close multi-square distances. */
    if (current_dir == target_dir) {
        if (click_route_push(st, NEXUS_CMD_FORWARD) < 0)
            return NEXUS_CLICK_RESULT_BLOCKED;
        return NEXUS_CLICK_RESULT_OK;
    }

    /* Otherwise turn toward the target first.  The UI can re-issue the click
     * after the tick consumes the turn. */
    click_route_push_turn_to(st, current_dir, target_dir);
    return NEXUS_CLICK_RESULT_OK;
}

/* ── Floor item click: move onto the square, then interact to pick up. ── */
static Nexus_ClickResult click_route_floor_item(Nexus_MechanicsState *st,
                                                 Nexus_V1_Engine *engine,
                                                 const Nexus_ClickTarget *target) {
    if (!st || !engine || !target) return NEXUS_CLICK_RESULT_INVALID;

    /* If already standing on the item, interact to pick it up. */
    if (st->party_x == target->x && st->party_y == target->y) {
        if (click_route_push(st, NEXUS_CMD_INTERACT) < 0)
            return NEXUS_CLICK_RESULT_BLOCKED;
        return NEXUS_CLICK_RESULT_OK;
    }

    /* Otherwise route movement onto the item's square. */
    return click_route_world_square(st, engine, target);
}

/* ── Public constructors ── */
Nexus_ClickTarget nexus_click_target_inventory_slot(int champion_index, int slot) {
    Nexus_ClickTarget t;
    t.kind = NEXUS_CLICK_TARGET_INVENTORY_SLOT;
    t.champion_index = champion_index;
    t.slot = slot;
    t.x = t.y = 0;
    t.floor_item_idx = -1;
    return t;
}

Nexus_ClickTarget nexus_click_target_equipment_slot(int champion_index, int slot) {
    Nexus_ClickTarget t;
    t.kind = NEXUS_CLICK_TARGET_EQUIPMENT_SLOT;
    t.champion_index = champion_index;
    t.slot = slot;
    t.x = t.y = 0;
    t.floor_item_idx = -1;
    return t;
}

Nexus_ClickTarget nexus_click_target_world_square(int x, int y) {
    Nexus_ClickTarget t;
    t.kind = NEXUS_CLICK_TARGET_WORLD_SQUARE;
    t.champion_index = -1;
    t.slot = -1;
    t.x = x;
    t.y = y;
    t.floor_item_idx = -1;
    return t;
}

Nexus_ClickTarget nexus_click_target_door_square(int x, int y) {
    Nexus_ClickTarget t;
    t.kind = NEXUS_CLICK_TARGET_DOOR_SQUARE;
    t.champion_index = -1;
    t.slot = -1;
    t.x = x;
    t.y = y;
    t.floor_item_idx = -1;
    return t;
}

Nexus_ClickTarget nexus_click_target_floor_item(int x, int y, int floor_item_idx) {
    Nexus_ClickTarget t;
    t.kind = NEXUS_CLICK_TARGET_FLOOR_ITEM;
    t.champion_index = -1;
    t.slot = -1;
    t.x = x;
    t.y = y;
    t.floor_item_idx = floor_item_idx;
    return t;
}

/* ── Public dispatch ── */
Nexus_ClickResult nexus_click_route_dispatch(Nexus_MechanicsState *st,
                                             Nexus_V1_Engine *engine,
                                             const Nexus_ClickTarget *target) {
    if (!st || !engine || !target) return NEXUS_CLICK_RESULT_INVALID;

    switch (target->kind) {
    case NEXUS_CLICK_TARGET_NONE:
        return NEXUS_CLICK_RESULT_NO_ACTION;
    case NEXUS_CLICK_TARGET_INVENTORY_SLOT:
    case NEXUS_CLICK_TARGET_EQUIPMENT_SLOT:
        return click_route_inventory(st, engine, target);
    case NEXUS_CLICK_TARGET_WORLD_SQUARE:
    case NEXUS_CLICK_TARGET_DOOR_SQUARE:
        return click_route_world_square(st, engine, target);
    case NEXUS_CLICK_TARGET_FLOOR_ITEM:
        return click_route_floor_item(st, engine, target);
    }
    return NEXUS_CLICK_RESULT_INVALID;
}
