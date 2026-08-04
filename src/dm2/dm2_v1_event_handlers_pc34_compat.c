/* DM2 V1 event handlers — skproject c_events.cpp.
 *
 * Each handler follows the callback pattern: game state is accessed
 * exclusively through the callback struct, keeping the module testable
 * without linking the full runtime. */

#include "dm2_v1_event_handlers_pc34_compat.h"
#include <stddef.h>
#include <string.h>

/* ---- DM2_CLICK_ITEM_SLOT (c_events.cpp:45) ---- */

void dm2_v1_click_item_slot(
    int16_t slot_index,
    const DM2_V1_ClickItemSlotCallbacks *cb, void *ctx,
    DM2_V1_ClickItemSlotReceipt *receipt)
{
    DM2_V1_ClickItemSlotReceipt r;
    int16_t hero_slot;
    int16_t adj_slot;
    int16_t held_item;
    int16_t slot_item;

    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    if (cb->get_event_heroidx(ctx) == -1)
        return;

    adj_slot = slot_index;
    if (adj_slot >= 8) {
        adj_slot -= 8;
        if (adj_slot < 0x1E)
            hero_slot = cb->get_v1e0976(ctx);
        else
            hero_slot = cb->get_curacthero(ctx);
        hero_slot--;
    } else {
        if (cb->get_v1e0288(ctx) != 0)
            return;
        hero_slot = adj_slot >> 1;
        if (hero_slot >= cb->get_heros_in_party(ctx))
            return;
        if (hero_slot + 1 == cb->get_v1e0976(ctx))
            return;
        if (cb->get_hero_curHP(ctx, hero_slot) == 0)
            return;
        adj_slot &= 1;
    }

    held_item = cb->get_held_item(ctx);
    if (adj_slot < 30)
        slot_item = cb->get_hero_item(ctx, hero_slot, adj_slot);
    else
        slot_item = cb->get_hand_container_item(ctx, (adj_slot - 30));

    if (slot_item == -1 && held_item == -1)
        return;

    if (held_item != -1) {
        if (!cb->is_item_fit_for_equip(ctx, held_item, adj_slot, 0))
            return;
    }

    cb->hide_mouse(ctx);

    if (held_item != -1)
        cb->remove_object_from_hand(ctx);

    if (slot_item != -1) {
        cb->remove_possession(ctx, hero_slot, adj_slot);
        cb->take_object(ctx, slot_item, 1);
    }

    if (held_item != -1)
        cb->equip_item_to_hand(ctx, hero_slot, held_item, adj_slot);

    cb->events_2e62_0cfa(ctx, 0);

    /* Refresh right panel for equipment-affecting slots */
    if (adj_slot == 0x0B || adj_slot == 0x06 || adj_slot == 0x0C ||
        (adj_slot >= 0x07 && adj_slot <= 0x09) ||
        adj_slot >= 0x1E) {
        if (cb->v1e0b6c)
            *cb->v1e0b6c = 1;
        cb->update_right_panel(ctx, 0);
    }

    if (cb->v1e0254 && *cb->v1e0254 != 0) {
        *cb->v1e0254 = 0;
        if (cb->events_443c_0434)
            cb->events_443c_0434(ctx);
    }

    cb->show_mouse(ctx);

    r.handled = 1;
    r.item_swapped = 1;
    if (receipt) *receipt = r;
}

/* ---- DM2_CLICK_VWPT (c_events.cpp:457) ---- */

void dm2_v1_click_vwpt(
    int16_t click_param,
    const DM2_V1_ClickVwptCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t held = cb->get_held_item(ctx);
    if (held != -1) {
        /* Item in hand: test wall for alcove/lever/keyhole */
        cb->player_testing_wall(ctx, click_param);
    } else {
        int16_t v = cb->get_v1e0b28(ctx);
        if (v != 0) {
            /* Push/pull rigid body */
            cb->push_pull_rigid_body(ctx, click_param);
        }
    }
}

/* ---- DM2_CLICK_MAGICAL_MAP_RUNE (c_events.cpp:395) ---- */

void dm2_v1_click_magical_map_rune(
    int16_t rune_index,
    const DM2_V1_ClickMagicalMapRuneCallbacks *cb, void *ctx)
{
    int16_t rune_cost;
    int16_t rune_mask;
    int16_t current_mask;

    if (!cb || !cb->rune_table || !cb->v1e0b62)
        return;

    rune_cost = cb->rune_table[rune_index];
    rune_mask = (int16_t)(1 << rune_index);
    current_mask = *cb->v1e0b62;

    if ((current_mask & rune_mask) == 0) {
        /* Adding rune — costs MP */
        if (cb->add_rune)
            cb->add_rune(ctx, rune_mask);
    } else {
        /* Removing rune — refunds MP */
        if (cb->remove_rune)
            cb->remove_rune(ctx, rune_mask);
    }

    *cb->v1e0b62 ^= rune_mask;
}

/* ---- DM2_CLICK_INVENTORY_EYE (c_events.cpp:1846) ---- */

void dm2_v1_click_inventory_eye(
    const DM2_V1_ClickInventoryEyeCallbacks *cb, void *ctx)
{
    int16_t hero;

    if (!cb) return;

    hero = cb->get_event_heroidx(ctx);
    if (hero == -1) return;

    cb->set_v1e0976(ctx, (int16_t)(hero + 1));
    cb->update_right_panel(ctx, 0);
}

/* ---- DM2_ACTIVATE_ACTION_HAND (c_events.cpp:2784) ---- */

void dm2_v1_activate_action_hand(
    const DM2_V1_ActivateActionHandCallbacks *cb, void *ctx)
{
    int16_t hero;

    if (!cb) return;

    hero = cb->get_event_heroidx(ctx);
    if (hero == -1) return;

    cb->set_curacthero(ctx, (int16_t)(hero + 1));
    if (cb->update_champion_display)
        cb->update_champion_display(ctx, hero);
}

/* ---- DM2_PROCEED_COMMAND_SLOT (c_events.cpp:2818) ---- */

void dm2_v1_proceed_command_slot(
    int16_t slot_index,
    const DM2_V1_ProceedCommandSlotCallbacks *cb, void *ctx,
    DM2_V1_ProceedCommandSlotReceipt *receipt)
{
    DM2_V1_ProceedCommandSlotReceipt r;
    int16_t curacthero;
    int16_t hero_idx;

    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    curacthero = cb->get_curacthero(ctx);
    if (curacthero == 0) return;

    hero_idx = curacthero - 1;

    if (slot_index != -1) {
        if (cb->engage_command) {
            int result = cb->engage_command(ctx, hero_idx, slot_index, 0);
            if (result) {
                r.command_engaged = 1;
                r.handled = 1;
            }
        }
    }

    cb->update_right_panel(ctx, 0);

    if (receipt) *receipt = r;
}

/* ---- DM2_PLAYER_TESTING_WALL (c_events.cpp:625) ---- */

void dm2_v1_player_testing_wall(
    int16_t param,
    const DM2_V1_PlayerTestingWallCallbacks *cb, void *ctx,
    DM2_V1_PlayerTestingWallReceipt *receipt)
{
    DM2_V1_PlayerTestingWallReceipt r;
    int16_t tile_class;
    int16_t x, y;
    int16_t held;

    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    tile_class = cb->get_facing_tile_class(ctx);
    if (tile_class != 0) /* not a wall */
        return;

    x = cb->get_facing_x(ctx);
    y = cb->get_facing_y(ctx);
    held = cb->get_held_item(ctx);

    /* Try alcove interaction */
    if (cb->is_wall_ornate_alcove && cb->is_wall_ornate_alcove(ctx, x, y)) {
        if (held != -1) {
            if (cb->try_insert_into_alcove &&
                cb->try_insert_into_alcove(ctx, x, y, held)) {
                r.handled = 1;
                r.action_type = 1;
            }
        } else {
            if (cb->try_take_from_alcove &&
                cb->try_take_from_alcove(ctx, x, y)) {
                r.handled = 1;
                r.action_type = 2;
            }
        }
    }

    /* Try lever */
    if (!r.handled && cb->try_toggle_lever) {
        if (cb->try_toggle_lever(ctx, x, y)) {
            r.handled = 1;
            r.action_type = 3;
        }
    }

    /* Try keyhole */
    if (!r.handled && held != -1 && cb->try_use_keyhole) {
        if (cb->try_use_keyhole(ctx, x, y, held)) {
            r.handled = 1;
            r.action_type = 4;
        }
    }

    if (r.handled && cb->queue_noise)
        cb->queue_noise(ctx, 0x84);

    if (receipt) *receipt = r;
}

/* ---- DM2_PUSH_PULL_RIGID_BODY (c_events.cpp:467) ---- */

void dm2_v1_push_pull_rigid_body(
    int16_t direction_param,
    const DM2_V1_PushPullRigidBodyCallbacks *cb, void *ctx,
    DM2_V1_PushPullRigidBodyReceipt *receipt)
{
    DM2_V1_PushPullRigidBodyReceipt r;
    int16_t push_dir;
    int16_t mode;
    int16_t target_x, target_y;

    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb || !cb->dx_table || !cb->dy_table) return;
    if (direction_param > 5) return;

    /* Compute push direction from the 6 click sub-zones:
     * 0: forward, 1: forward-right, 2: left,
     * 3: right, 4: back-left, 5: forward-left
     * Source: c_events.cpp:489-534 switch */
    switch (direction_param) {
    case 0:
        push_dir = cb->party_dir;
        mode = 0;
        break;
    case 1:
        push_dir = (cb->party_dir + 1) & 3;
        mode = 2;
        break;
    case 2:
        push_dir = (cb->party_dir + 2) & 3;
        mode = 1;
        break;
    case 3:
        push_dir = (cb->party_dir + 2) & 3;
        mode = 0;
        break;
    case 4:
        push_dir = (cb->party_dir + 2) & 3;
        mode = 2;
        break;
    case 5:
        push_dir = (cb->party_dir - 1) & 3;
        mode = 1;
        break;
    default:
        return;
    }

    /* Target tile in push direction from party */
    target_x = (int16_t)(cb->party_x + cb->dx_table[cb->party_dir]);
    target_y = (int16_t)(cb->party_y + cb->dy_table[cb->party_dir]);

    /* Check if creature is movable to push destination */
    int16_t dest_x = (int16_t)(target_x + cb->dx_table[push_dir]);
    int16_t dest_y = (int16_t)(target_y + cb->dy_table[push_dir]);

    if (cb->is_tile_blocked && cb->is_tile_blocked(ctx, dest_x, dest_y))
        return;

    /* Get the record at the target tile */
    int16_t record = -1;
    if (cb->get_tile_record)
        record = cb->get_tile_record(ctx, target_x, target_y);

    if (record == -1)
        return;

    /* Move the record */
    if (cb->move_record)
        cb->move_record(ctx, record, target_x, target_y, dest_x, dest_y);

    if (cb->queue_noise)
        cb->queue_noise(ctx, 0x82);

    if (cb->redraw_viewport)
        cb->redraw_viewport(ctx);

    r.handled = 1;
    r.pushed = 1;
    r.target_x = dest_x;
    r.target_y = dest_y;
    if (receipt) *receipt = r;
}
