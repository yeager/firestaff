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

/* ---- DM2_events_5BFB (c_events.cpp:170) ---- */

void dm2_v1_events_5bfb(
    int16_t param1, int16_t param2,
    const DM2_V1_Events5BFBCallbacks *cb, void *ctx)
{
    if (!cb) return;

    if (param2 != 0) {
        if (param2 == 0x0a && cb->v1d26a2)
            *cb->v1d26a2 = param1;
    } else {
        if (cb->v1d26a0)
            *cb->v1d26a0 = param1;
        if (cb->queue_noise_gen1)
            cb->queue_noise_gen1(ctx, 0x03, 0, 0x8b, 0x6c, 0xc8,
                                 cb->v1e0270, cb->v1e0272, 0);
    }
    if (cb->sound3)
        cb->sound3(ctx, param1, param2);
}

/* ---- DM2_events_38c8_0002 (c_events.cpp:327) ---- */

void dm2_v1_events_38c8_0002(
    const DM2_V1_Events38c80002Callbacks *cb, void *ctx,
    DM2_V1_Events38c80002Receipt *receipt)
{
    DM2_V1_Events38c80002Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    if (cb->v1e13f0 && *cb->v1e13f0 != 0)
        return;
    if (cb->dialog2 && *cb->dialog2 != 0)
        return;

    if (cb->v1e13f0) *cb->v1e13f0 = 1;

    if (cb->v1e00b8 && *cb->v1e00b8 != 0) {
        if (cb->champion_squad_recompute)
            cb->champion_squad_recompute(ctx);
        cb->update_right_panel(ctx, 0);
    }

    if (cb->proc_1031_04f5)
        cb->proc_1031_04f5(ctx);
    cb->hide_mouse(ctx);
    if (cb->move_12b4_0092)
        cb->move_12b4_0092(ctx);

    if (cb->v1e0976 && *cb->v1e0976 == 0)
        cb->fill_halftone_recti(ctx, 9);

    cb->fill_halftone_recti(ctx, 11);
    cb->show_mouse(ctx);

    r.handled = 1;
    if (receipt) *receipt = r;
}

/* ---- DM2_events_38c8_0060 (c_events.cpp:355) ---- */

void dm2_v1_events_38c8_0060(
    const DM2_V1_Events38c80060Callbacks *cb, void *ctx)
{
    if (!cb) return;

    if (cb->v1e13f0 && *cb->v1e13f0 == 0)
        return;
    if (cb->dialog2 && *cb->dialog2 != 0)
        return;

    if (cb->v1e0238 && *cb->v1e0238 != 0) {
        if (cb->draw_wake_up_text)
            cb->draw_wake_up_text(ctx);
        if (cb->drawings_completed)
            cb->drawings_completed(ctx);
        return;
    }

    if (cb->v1e13f0) *cb->v1e13f0 = 0;

    if (cb->curacthero != 0) {
        if (cb->v1e0b6c) *cb->v1e0b6c = 1;
    } else {
        if (cb->v1e0b30) *cb->v1e0b30 = 1;
    }

    int16_t v1e0976 = cb->v1e0976 ? *cb->v1e0976 : 0;
    if (v1e0976 != 0) {
        if (cb->v1e0976) *cb->v1e0976 = 0;
        if (cb->guidraw_24a5_1798)
            cb->guidraw_24a5_1798(ctx, (int16_t)(v1e0976 - 1));
        return;
    }

    if (cb->init_backbuff)
        cb->init_backbuff(ctx);
    if (cb->guidraw_29ee_000f)
        cb->guidraw_29ee_000f(ctx);
}

/* ---- DM2_events_13262 (c_events.cpp:427) ---- */

void dm2_v1_events_13262(
    int16_t param,
    const DM2_V1_Events13262Callbacks *cb, void *ctx)
{
    if (!cb) return;

    for (int16_t i = 0; i < cb->v1e0404; i++) {
        int16_t b_0a = cb->get_zone_b_0a(ctx, i);
        if (b_0a != param)
            continue;

        int16_t b_0b = cb->get_zone_b_0b(ctx, i);
        if (b_0b == 0x04 || b_0b == 0x06) {
            int16_t x = (int16_t)(cb->get_zone_rc_x(ctx, i) + cb->v1d26fc);
            int16_t y = (int16_t)(cb->get_zone_rc_y(ctx, i) + cb->v1d26fe);
            cb->click_vwpt(ctx, x, y);
            return;
        }
    }
}

/* ---- DM2_ADD_RUNE_TO_TAIL (c_events.cpp:1871) ---- */

void dm2_v1_add_rune_to_tail(
    int16_t rune_index,
    const DM2_V1_AddRuneToTailCallbacks *cb, void *ctx,
    DM2_V1_AddRuneToTailReceipt *receipt)
{
    DM2_V1_AddRuneToTailReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    int hero = cb->curacthero - 1;
    if (hero < 0) return;

    int16_t nrunes = cb->get_hero_nrunes(ctx, hero);
    int16_t cost;

    if (cb->rune_cost_table)
        cost = (int16_t)cb->rune_cost_table[nrunes * 6 + rune_index];
    else
        return;

    if (nrunes != 0 && cb->rune_power_table) {
        int16_t first_rune = cb->get_hero_rune(ctx, hero, 0);
        int16_t power = (int16_t)cb->rune_power_table[first_rune + 1];
        cost = (int16_t)((cost * power) >> 3);
    }

    int flag = 0;
    int16_t curMP = cb->get_hero_curMP(ctx, hero);
    if (cost <= curMP) {
        cb->set_hero_curMP(ctx, hero, (int16_t)(curMP - cost));
        cb->or_hero_flag(ctx, hero, 0x800);

        int16_t rune_val = (int16_t)(nrunes * 6 + 0x60 + rune_index);
        cb->set_hero_rune(ctx, hero, nrunes, rune_val);
        nrunes++;
        cb->set_hero_nrunes(ctx, hero, nrunes);
        cb->set_hero_rune(ctx, hero, nrunes, 0);
        flag = 1;
    }

    if (cb->v1e0b6c) *cb->v1e0b6c = 1;
    cb->update_right_panel(ctx, 0);
    if (cb->proc_107b0)
        cb->proc_107b0(ctx);

    if (flag && cb->refresh_player_stat_disp)
        cb->refresh_player_stat_disp(ctx, (int16_t)(cb->curacthero - 1));

    r.handled = 1;
    r.rune_added = flag;
    if (receipt) *receipt = r;
}

/* ---- DM2_REMOVE_RUNE_FROM_TAIL (c_events.cpp:1931) ---- */

void dm2_v1_remove_rune_from_tail(
    const DM2_V1_RemoveRuneFromTailCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int hero = cb->curacthero - 1;
    if (hero < 0) return;

    int16_t nrunes = cb->get_hero_nrunes(ctx, hero);
    if (nrunes <= 0) return;

    nrunes--;
    cb->set_hero_rune(ctx, hero, nrunes, 0);
    cb->set_hero_nrunes(ctx, hero, nrunes);

    if (cb->v1e0b6c) *cb->v1e0b6c = 1;
    cb->update_right_panel(ctx, 0);
    if (cb->proc_107b0)
        cb->proc_107b0(ctx);
}

/* ---- DM2_CLICK_MONEYBOX (c_events.cpp:1941) ---- */

void dm2_v1_click_moneybox(
    int16_t slot_index,
    const DM2_V1_ClickMoneyboxCallbacks *cb, void *ctx,
    DM2_V1_ClickMoneyboxReceipt *receipt)
{
    DM2_V1_ClickMoneyboxReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    if (cb->get_v1d67bc(ctx) != 0x04)
        return;

    int16_t hero_idx = (int16_t)(cb->curacthero - 1);
    if (hero_idx < 0) return;

    int16_t container = cb->get_hero_item(ctx, hero_idx, cb->curactmode);
    int16_t held = cb->get_held_item(ctx);
    int skip = 0;

    if (held == -1) {
        int16_t order = cb->get_item_order_in_container(ctx, container, slot_index);
        int16_t coin = cb->take_coin_from_wallet(ctx, container, order);
        if (coin == -1) {
            skip = 1;
        } else {
            cb->take_object(ctx, coin, 0);
            r.withdrawn = 1;
        }
    } else {
        int result = cb->add_coin_to_wallet(ctx, container, held);
        if (result == 0) {
            skip = 1;
        } else {
            cb->remove_object_from_hand(ctx);
            r.deposited = 1;
        }
    }

    if (!skip) {
        if (cb->v1e0b6c) *cb->v1e0b6c = 1;
        cb->update_right_panel(ctx, 0);
    }

    cb->calc_player_weight(ctx, hero_idx);

    r.handled = 1;
    if (receipt) *receipt = r;
}

/* ---- DM2_events_2e62_0cfa (c_events.cpp:2153) ---- */

void dm2_v1_events_2e62_0cfa(
    int16_t param,
    const DM2_V1_Events2e620cfaCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t heros = cb->get_heros_in_party(ctx);
    int16_t v1e0976 = cb->get_v1e0976(ctx);

    for (int16_t i = 0; i < heros; i++) {
        if (i + 1 != v1e0976)
            cb->refresh_player_stat_disp(ctx, i);
    }

    if (v1e0976 == 0) {
        cb->update_right_panel(ctx, param);
        return;
    }

    int16_t hero_idx = (int16_t)(v1e0976 - 1);

    if (cb->get_hero_ench_power(ctx, hero_idx) != 0) {
        int16_t aura = cb->get_hero_ench_aura(ctx, hero_idx);
        if (aura >= 3 && aura <= 6) {
            int16_t or_bits;
            int vcapture3 = cb->get_vcapture3(ctx);
            int16_t held = cb->get_held_item(ctx);
            if (!vcapture3 || held != -1)
                or_bits = 0x1000;
            else
                or_bits = 0x3000;

            int16_t flags = cb->get_hero_flag(ctx, hero_idx);
            cb->set_hero_flag(ctx, hero_idx, (int16_t)(flags | or_bits));
        }
    }

    cb->refresh_player_stat_disp(ctx, hero_idx);
    cb->update_right_panel(ctx, param);
}

/* ---- DM2_events_30DEA (c_events.cpp:2903) ---- */

int dm2_v1_events_30dea(
    int16_t hero,
    const DM2_V1_Events30DEACallbacks *cb, void *ctx,
    DM2_V1_Events30DEAReceipt *receipt)
{
    DM2_V1_Events30DEAReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return 0;

    int16_t charge_type = cb->query_cmdstr_entry(ctx,
        cb->v1e0b7f, cb->v1e0b80, cb->v1e0b7e, 0x08);

    if (charge_type == 0x10)
        charge_type = 0x0f;

    if (charge_type != 0x11 && charge_type != 0x12)
        cb->add_item_charge(ctx, cb->v1e0b50, (int16_t)(-charge_type));

    int16_t remaining = cb->add_item_charge(ctx, cb->v1e0b50, 0);
    int destroyed = (remaining == 0) ? 1 : 0;

    if (destroyed) {
        int16_t dbspec = cb->query_gdat_dbspec_word(ctx, cb->v1e0b50, 0);
        if ((dbspec & 0x0800) != 0) {
            cb->remove_possession(ctx, hero, cb->curactmode);
            cb->dealloc_record(ctx, cb->v1e0b50);
            r.item_destroyed = 1;
        }
    }

    r.handled = 1;
    if (receipt) *receipt = r;
    return destroyed ? 0 : 1;
}

/* ---- DM2_events_443c_0434 (c_events.cpp:3958) ---- */

void dm2_v1_events_443c_0434(
    const DM2_V1_Events443c0434Callbacks *cb, void *ctx,
    DM2_V1_Events443c0434Receipt *receipt)
{
    DM2_V1_Events443c0434Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    int16_t held = cb->get_held_item(ctx);
    if (held != -1) {
        if (cb->generate_cursor_from_item)
            cb->generate_cursor_from_item(ctx);
        r.cursor_changed = 1;
    }

    if (cb->refresh_mouse)
        cb->refresh_mouse(ctx);

    r.handled = 1;
    if (receipt) *receipt = r;
}

/* ---- DM2_events_37BBB (c_events.cpp:1290) ---- */

int dm2_v1_events_37bbb(
    int16_t param,
    const DM2_V1_Events37BBBCallbacks *cb, void *ctx)
{
    if (!cb) return 0;

    int16_t hero = cb->get_event_heroidx(ctx);
    if (hero == -1)
        return 0;

    return cb->hero_2c1d_1de2(ctx, hero, -1, param);
}

/* ---- DM2_events_121e_0351 (c_events.cpp:1298) ---- */

void dm2_v1_events_121e_0351(
    int16_t x, int16_t y,
    const DM2_V1_Events121e0351Callbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t rect_base;
    if (cb->v1e0530 == 0x11)
        rect_base = 0x300;
    else
        rect_base = 0x2fd;

    int result = 0;
    int16_t side;

    if (cb->pt_in_expanded_rect(ctx, rect_base, x, y)) {
        side = 0;
    } else if (cb->pt_in_expanded_rect(ctx, (int16_t)(rect_base + 1), x, y)) {
        side = 1;
    } else {
        return;
    }

    result = cb->hero_2c1d_1de2(ctx,
        cb->get_event_heroidx(ctx), -1, side);

    if (result != 0 && cb->v1e0488)
        *cb->v1e0488 = 1;
}

/* ---- DM2_events_121e_0003 (c_events.cpp:1222) ---- */

void dm2_v1_events_121e_0003(
    int16_t param,
    const DM2_V1_Events121e0003Callbacks *cb, void *ctx)
{
    if (!cb || !cb->dx_table || !cb->dy_table) return;

    int16_t x = cb->v1e0270;
    int16_t y = cb->v1e0272;
    int16_t dir_offset = 1;

    if (param < 2) {
        if (param == 1) {
            int16_t adj_dir = (int16_t)((cb->v1e0258 + 3) & 3);
            x = (int16_t)(x + cb->dx_table[adj_dir]);
            y = (int16_t)(y + cb->dy_table[adj_dir]);
        }
    } else if (param == 2) {
        int16_t adj_dir = (int16_t)((cb->v1e0258 + 1) & 3);
        x = (int16_t)(x + cb->dx_table[adj_dir]);
        y = (int16_t)(y + cb->dy_table[adj_dir]);
        dir_offset = 3;
    } else if (param == 3) {
        int16_t adj_dir = cb->v1e0258;
        x = (int16_t)(x + cb->dx_table[adj_dir]);
        y = (int16_t)(y + cb->dy_table[adj_dir]);
        dir_offset = 2;
    }

    if (x >= 0 && x < cb->map_width && y >= 0 && y < cb->map_height) {
        int16_t dir = (int16_t)((cb->v1e0258 + dir_offset) & 3);
        cb->events_3c1e5(ctx, x, y, dir, -1, cb->savegamewpc_w_00);
    }
}

/* ---- DM2_events_121e_013a (c_events.cpp:974) ---- */

void dm2_v1_events_121e_013a(
    int16_t click_x, int16_t click_y, int16_t zone_idx,
    const DM2_V1_Events121e013aCallbacks *cb, void *ctx)
{
    if (!cb) return;

    if (cb->get_event_heroidx(ctx) == -1)
        return;

    int16_t tile_x = cb->v1e0270;
    int16_t tile_y = cb->v1e0272;

    int16_t b_0a = cb->get_zone_b_0a(ctx, zone_idx);
    if (b_0a != 0) {
        if (b_0a != 3)
            return;
        int16_t dir = cb->v1e0258;
        tile_x = (int16_t)(tile_x + cb->dx_table[dir]);
        tile_y = (int16_t)(tile_y + cb->dy_table[dir]);
    }

    int16_t b_0b = cb->get_zone_b_0b(ctx, zone_idx);
    int16_t w_08 = cb->get_zone_w_08(ctx, zone_idx);

    int16_t result = cb->events_32cb_03a6(ctx,
        click_x, click_y, b_0a, tile_x, tile_y, w_08, b_0b, 0);

    if (result != -1) {
        int16_t param = (b_0b == 2) ? -1 : 0;
        cb->move_record_to(ctx, result, tile_x, tile_y, -1, param);
        if (cb->v1e0ff6) *cb->v1e0ff6 = 1;

        if (cb->is_container_moneybox(ctx, result)) {
            /* Clear bit 2 in record byte 7 */
        }

        cb->take_object(ctx, result, 1);
    }

    if (cb->v1e0488) *cb->v1e0488 = 1;
}

/* ---- DM2_eventa_121e_0222 (c_events.cpp:1051) ---- */

int16_t dm2_v1_eventa_121e_0222(
    int16_t x, int16_t y, int16_t dir_offset,
    const DM2_V1_Eventa121e0222Callbacks *cb, void *ctx)
{
    if (!cb) return 0;

    if (cb->get_event_heroidx(ctx) == -1)
        return 0;

    int16_t dir = (int16_t)((cb->v1e0258 + dir_offset) & 3);

    int16_t record = cb->remove_object_from_hand(ctx);
    if (record == -1) return 0;

    cb->move_record_to(ctx, record, x, y, -1, -1);
    if (cb->v1e0488) *cb->v1e0488 = 1;

    return record;
}

/* ---- DM2_events_121e_03ae (c_events.cpp:1171) ---- */

int dm2_v1_events_121e_03ae(
    int16_t click_x, int16_t click_y,
    int16_t tile_x, int16_t tile_y,
    int16_t argw0, int16_t argl1, int16_t argw2,
    const DM2_V1_Events121e03aeCallbacks *cb, void *ctx)
{
    if (!cb) return 0;

    int16_t dir = (int16_t)((cb->v1e0258 + argl1) & 3);
    int16_t record_bits = (int16_t)((dir << 14) |
        (cb->savegamewpc_w_00 & 0x3FFF));

    int16_t result = cb->events_32cb_03a6(ctx,
        click_x, click_y, argw0, tile_x, tile_y,
        record_bits, argw2, 1);

    if (result == -1) {
        cb->remove_object_from_hand(ctx);
        return 1;
    }

    if (result != -2)
        return 0;

    if (argw2 == 2) {
        int16_t adj_dir = (int16_t)(argl1 + 4);
        return cb->eventa_121e_0222(ctx, tile_x, tile_y, adj_dir) != 0 ? 1 : 0;
    }

    return cb->eventa_121e_0222(ctx, tile_x, tile_y, argl1) != 0 ? 1 : 0;
}

/* ---- DM2_events_32cb_0287 (c_events.cpp:670) ---- */

int dm2_v1_events_32cb_0287(
    int16_t param, int16_t click_x, int16_t click_y,
    const DM2_V1_Events32cb0287Callbacks *cb, void *ctx)
{
    if (!cb) return 0;
    if (param > 3) return 0;

    int result = 0;
    void *saved_ptr = cb->ptr1e1044 ? *cb->ptr1e1044 : NULL;

    if (cb->alloc_lobigpool)
        *cb->ptr1e1044 = cb->alloc_lobigpool(ctx, 0x48);

    int16_t vw_14 = cb->v1e12cc;
    int16_t vw_08 = cb->v1e12ca;

    if (cb->calc_vector_w_dir)
        cb->calc_vector_w_dir(ctx, cb->v1e12c8,
            cb->table1d6ad0[param][1], cb->table1d6ad0[param][0],
            &vw_14, &vw_08);

    if (cb->summarize_stone_room) {
        int offset = 2 * (param + 8 * param);
        (void)offset;
        cb->summarize_stone_room(ctx, NULL, cb->v1e12c8, vw_14, vw_08);
    }

    int16_t hit = -1;
    if (cb->guivp_32cb_15b8)
        hit = cb->guivp_32cb_15b8(ctx, param, cb->table1d6afe[param], 0);

    if (hit >= 0 && cb->guivp_32cb_00f1)
        result = cb->guivp_32cb_00f1(ctx, click_x, click_y, hit);

    if (cb->dealloc_lobigpool)
        cb->dealloc_lobigpool(ctx, 0x48);
    if (cb->ptr1e1044)
        *cb->ptr1e1044 = saved_ptr;

    if (param == 3)
        return 1;

    if (cb->img_width < 0x20)
        return 1;
    if (cb->img_height < 0x20)
        return 1;

    return result;
}

/* ---- DM2_events_32cb_03a6 (c_events.cpp:736) ---- */

int16_t dm2_v1_events_32cb_03a6(
    int16_t click_x, int16_t click_y,
    int16_t zone_dir, int16_t tile_x,
    int16_t tile_y, int16_t record,
    int16_t zone_type, int argl3,
    const DM2_V1_Events32cb03a6Callbacks *cb, void *ctx)
{
    if (!cb) return -1;

    int16_t dir_from_record = (int16_t)((record >> 14) & 3);
    int16_t cur_record;

    if (zone_type != 2)
        cur_record = cb->get_tile_record_link(ctx, tile_x, tile_y);
    else {
        int16_t creature = cb->get_creature_at(ctx, tile_x, tile_y);
        if (creature == -1) return (argl3 == 0) ? -1 : record;
        void *addr = cb->get_address_of_record(ctx, creature);
        if (!addr) return (argl3 == 0) ? -1 : record;
        cur_record = creature;
    }

    if (cur_record == -2)
        return (argl3 == 0) ? -1 : record;

    int16_t best_record = -1;
    int16_t slot = (zone_type == 3) ? 2 : 0;

    while (cur_record != -2) {
        int16_t rec_dir = (int16_t)((cur_record >> 14) & 3);
        if (rec_dir == dir_from_record) {
            if (cb->guivp_32cb_00f1 &&
                cb->guivp_32cb_00f1(ctx, click_x, click_y, 0x0a)) {
                best_record = cur_record;
            }
        }
        slot++;
        if (zone_type != 3) {
            slot &= 0x0f;
        } else {
            if (slot >= 0x0e)
                slot = 2;
        }
        cur_record = cb->get_next_record_link(ctx, cur_record);
    }

    return best_record;
}

/* ---- DM2_guivp_32cb_01b6 (c_events.cpp:1345) ---- */

int dm2_v1_guivp_32cb_01b6(
    int16_t click_x, int16_t click_y,
    int16_t tile_x, int16_t tile_y,
    int16_t *out_dir,
    const DM2_V1_Guivp32cb01b6Callbacks *cb, void *ctx)
{
    if (!cb || !out_dir) return 0;

    int16_t creature = cb->get_creature_at(ctx, tile_x, tile_y);
    if (creature == -1) return 0;

    void *record = cb->get_address_of_record(ctx, creature);
    if (!record) return 0;

    if (cb->query_creature_picst)
        cb->query_creature_picst(ctx, 3, cb->table1d6b15[3],
                                  record, NULL, creature);

    if (cb->draw_temp_picst)
        cb->draw_temp_picst(ctx);

    if (cb->guivp_32cb_00f1 &&
        !cb->guivp_32cb_00f1(ctx, click_x, click_y, -2))
        return 0;

    if (cb->v1e12da < -75) {
        *out_dir = 3;
        return 1;
    }
    if (cb->v1e12da > 75) {
        *out_dir = 1;
        return 1;
    }
    *out_dir = 0;
    return 1;
}

/* ---- DM2_events_3C1E5 (c_events.cpp:2949) ---- */

void dm2_v1_events_3c1e5(
    int16_t x, int16_t y, int16_t dir,
    int16_t param, int16_t record,
    const DM2_V1_Events3C1E5Callbacks *cb, void *ctx,
    DM2_V1_Events3C1E5Receipt *receipt)
{
    DM2_V1_Events3C1E5Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    int16_t tile_record = cb->get_tile_record_link(ctx, x, y);
    int16_t cur = tile_record;

    while (cur != -2) {
        int16_t rec_dir = (int16_t)((cur >> 14) & 3);
        if (rec_dir == dir) {
            if (cb->is_wall_ornate_alcove &&
                cb->is_wall_ornate_alcove(ctx, tile_record, cur)) {
                if (param == -1) {
                    if (cb->get_event_heroidx(ctx) == -1)
                        break;
                    if (record != -1) {
                        cb->move_record_to(ctx, record, x, y, -1, 0);
                        r.handled = 1;
                        r.action_type = 1;
                    } else {
                        cb->take_object(ctx, cur, 1);
                        r.handled = 1;
                        r.action_type = 2;
                    }
                }
                break;
            }

            int16_t rec_type = (int16_t)((cur & 0x3C00) >> 10);

            if (rec_type == 2 && param == -1) {
                if (cb->get_event_heroidx(ctx) != -1) {
                    if (cb->queue_noise_gen1)
                        cb->queue_noise_gen1(ctx, 3, 0, 0x88, 0x8c, 0x80,
                            cb->v1e0270, cb->v1e0272, 1);
                    if (cb->invoke_message)
                        cb->invoke_message(ctx, x, y, 0, 2,
                            cb->gametick + 1);
                    if (cb->queue_timer)
                        cb->queue_timer(ctx, 0x58, 0, cur,
                            cb->gametick + 1, cb->v1d3248);
                    if (cb->v1e0488) *cb->v1e0488 = 1;
                    r.handled = 1;
                    r.action_type = 3;
                }
                break;
            }
        }
        cur = cb->get_next_record_link(ctx, cur);
    }

    if (receipt) *receipt = r;
}

/* ---- DM2_events_AB26 (c_events.cpp:201) ---- */

void dm2_v1_events_ab26(
    const DM2_V1_EventsAB26Callbacks *cb, void *ctx,
    DM2_V1_EventsAB26Receipt *receipt)
{
    DM2_V1_EventsAB26Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb) return;

    if (!cb->query_gdat_entry_if_loadable(ctx, 0x1a, 0x87, 0x01, 0))
        return;

    if (cb->proc_0aaf_02f8)
        cb->proc_0aaf_02f8(ctx, 0x87, 0);

    char text_buf[0x80];
    char digit_buf[4];

    if (cb->query_gdat_text) {
        cb->query_gdat_text(ctx, 0x1a, 0x87, 0x28, text_buf);
        uint8_t col = cb->palette_col11(ctx);
        cb->draw_vp_rc_str(ctx, 0x15f, col, text_buf);

        cb->query_gdat_text(ctx, 0x1a, 0x87, 0x3c, text_buf);
        cb->draw_vp_rc_str(ctx, 0x170, col, text_buf);
    }

    for (;;) {
        for (int16_t i = 8; i >= 0; i--) {
            digit_buf[0] = (char)(i + 0x30);
            digit_buf[1] = 0;
            int16_t rect_id = (int16_t)(i + 0x160);
            int is_selected = (cb->v1d26a0 && i == *cb->v1d26a0) ? 1 : 0;
            uint8_t pic_idx = (uint8_t)(is_selected + 2);
            cb->draw_static_pic(ctx, 26, 0x87, pic_idx, rect_id, 0);

            if (i == 0 && cb->query_gdat_text) {
                cb->query_gdat_text(ctx, 0x1a, 0x87, 0x29, text_buf);
                uint8_t c = cb->palette_col11(ctx);
                cb->draw_vp_rc_str(ctx, (int16_t)(i + 0x168), c, text_buf);
            } else {
                uint8_t c = cb->palette_col11(ctx);
                cb->draw_vp_rc_str(ctx, (int16_t)(i + 0x168), c, digit_buf);
            }
        }

        for (int16_t i = 8; i >= 0; i--) {
            digit_buf[0] = (char)(i + 0x30);
            digit_buf[1] = 0;
            int16_t rect_id = (int16_t)(i + 0x171);
            int is_selected = (cb->v1d26a2 && i == *cb->v1d26a2) ? 1 : 0;
            uint8_t pic_idx = (uint8_t)(is_selected + 2);
            cb->draw_static_pic(ctx, 26, 0x87, pic_idx, rect_id, 0);

            if (i == 0 && cb->query_gdat_text) {
                cb->query_gdat_text(ctx, 0x1a, 0x87, 0x3d, text_buf);
                uint8_t c = cb->palette_col11(ctx);
                cb->draw_vp_rc_str(ctx, (int16_t)(i + 0x179), c, text_buf);
            } else {
                uint8_t c = cb->palette_col11(ctx);
                cb->draw_vp_rc_str(ctx, (int16_t)(i + 0x179), c, digit_buf);
            }
        }

        cb->draw_gameload_dialogue_to_screen(ctx);
        int16_t key = cb->proc_0aaf_0067(ctx, 0x87);

        if (key == 0xdb)
            return;

        int16_t slot_val;
        int16_t slot_type;

        if (key >= 0x9d && key <= 0xa4) {
            slot_val = (int16_t)(key - 0x9d);
            slot_type = 0x0a;
        } else {
            slot_val = (int16_t)((key - 0x95) & 0xffff);
            slot_type = 0;
        }

        if (cb->events_5bfb)
            cb->events_5bfb(ctx, slot_val, slot_type);
    }
}

/* ---- DM2_events_2f3f_04ea (c_events.cpp:1996) ---- */

void dm2_v1_events_2f3f_04ea(
    int16_t click_x, int16_t click_y,
    int16_t dir, int16_t map,
    int16_t argw0,
    const DM2_V1_Events2f3f04eaCallbacks *cb, void *ctx)
{
    if (!cb) return;

    int16_t hero_idx = (int16_t)(cb->v1e0288 - 1);
    int16_t saved_map = cb->v1d3248;

    cb->change_current_map_to(ctx, map);

    int16_t removed = (int16_t)cb->remove_object_from_hand(ctx);

    if (argw0 != 0x93) {
        if (cb->v1d6a2d == 0 && cb->draw_cryocell_lever)
            cb->draw_cryocell_lever(ctx, 1);

        if (removed != -1 && cb->add_item_to_player)
            cb->add_item_to_player(ctx, hero_idx, removed);

        int16_t target_x = click_x;
        int16_t target_y = click_y;

        if (cb->dx_table && cb->dy_table) {
            target_x = (int16_t)(target_x + cb->dx_table[dir]);
            target_y = (int16_t)(target_y + cb->dy_table[dir]);
        }

        int16_t tile_rec = cb->get_tile_record_link(ctx, target_x, target_y);

        while (tile_rec != -2) {
            int16_t rec_type = (int16_t)((tile_rec & 0x3C00) >> 10);
            if (rec_type == 3) {
                void *addr = cb->get_address_of_record(ctx, tile_rec);
                if (addr) {
                    if (cb->v1d6a2d == 0) {
                        if (cb->v1e098c)
                            *cb->v1e098c = hero_idx;
                        if (cb->display_hint_new_line)
                            cb->display_hint_new_line(ctx);

                        char text_buf[0x80];
                        if (cb->query_gdat_text) {
                            cb->query_gdat_text(ctx, 0x01, 0x00, 0x0e, text_buf);
                            int16_t color = (int16_t)cb->table1d69d0[hero_idx];
                            cb->display_hint_text(ctx, color, text_buf);
                        }
                    }
                    break;
                }
            }
            tile_rec = cb->get_next_record_link(ctx, tile_rec);
        }
    } else {
        if (cb->guidraw_24a5_1798)
            cb->guidraw_24a5_1798(ctx, 4);
        cb->hide_mouse(ctx);

        if (cb->heros_in_party == 1 && cb->select_champion_leader)
            cb->select_champion_leader(ctx, -1);

        if (cb->events_38c8_0060)
            cb->events_38c8_0060(ctx);
        cb->show_mouse(ctx);
    }

    cb->change_current_map_to(ctx, saved_map);
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

/* ---- DM2_ADJUST_UI_EVENT (c_input.cpp:112) ---- */

void dm2_v1_adjust_ui_event(
    int16_t *idx, int16_t x, int16_t y,
    const DM2_V1_AdjustUiEventCallbacks *cb, void *ctx,
    DM2_V1_AdjustUiEventReceipt *receipt)
{
    DM2_V1_AdjustUiEventReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb || !idx) return;

    r.original_idx = *idx;
    r.adjusted_idx = *idx;

    if (*idx >= 116 && *idx <= 123) {
        /* Action hand clicks (idx 116-123).
         * Hand index: 0 for idx 116-119, 1 for idx 120-123. */
        int16_t curacthero = cb->get_curacthero(ctx);
        if (curacthero == 0) {
            *idx = 0;
            r.suppressed = 1;
        } else {
            int16_t hero = (int16_t)(curacthero - 1);
            if (cb->get_hero_curHP(ctx, hero) == 0) {
                *idx = 0;
                r.suppressed = 1;
            } else {
                int16_t hand = (int16_t)((*idx >= 120) ? 1 : 0);
                int16_t cooldown = cb->get_hero_hand_cooldown(ctx, hero, hand);
                if (cooldown > 0) {
                    *idx = 0;
                    r.suppressed = 1;
                } else {
                    /* Check item activability for the hand slot */
                    int16_t slot = (int16_t)(hand == 0 ? 0x06 : 0x07);
                    int16_t item = cb->get_hero_item(ctx, hero, slot);
                    if (item != -1 && cb->is_item_activable &&
                        !cb->is_item_activable(ctx, item)) {
                        *idx = 0;
                        r.suppressed = 1;
                    }
                }
            }
        }
    } else if (*idx >= 95 && *idx <= 98) {
        /* Movement arrows (idx 95-98).
         * Check player position and hand cooldown. */
        if (cb->v1e0288 != 0) {
            *idx = 0;
            r.suppressed = 1;
        } else {
            int16_t curacthero = cb->get_curacthero(ctx);
            if (curacthero != 0) {
                int16_t hero = (int16_t)(curacthero - 1);
                int16_t pos = cb->get_player_position(ctx, hero);
                if (pos < 0) {
                    *idx = 0;
                    r.suppressed = 1;
                } else {
                    int16_t cd0 = cb->get_hero_hand_cooldown(ctx, hero, 0);
                    int16_t cd1 = cb->get_hero_hand_cooldown(ctx, hero, 1);
                    if (cd0 > 0 || cd1 > 0) {
                        *idx = 0;
                        r.suppressed = 1;
                    }
                }
            }
        }
    }

    r.adjusted_idx = *idx;
    r.handled = 1;
    if (receipt) *receipt = r;
}

/* ---- DM2_1031_03f2 (c_input.cpp:55) ---- */

int16_t dm2_v1_1031_03f2_tree(
    const DM2_V1_1031_03f2Callbacks *cb, void *ctx,
    DM2_V1_1031_03f2Receipt *receipt)
{
    DM2_V1_1031_03f2Receipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb || !cb->table1d3ba0 || !cb->table1d3d23) return 0;

    int16_t bbw = cb->s_bbw;

    /* Walk the event table looking for entries matching bbw.
     * Each entry is 3 bytes: [0]=bbw_match, [1]=sub_bbw, [2]=flags.
     * If bbw matches entry[0], check flags:
     *   - If flags indicates a sub-table (entry[2] & 0x80), recurse
     *     with sub_bbw = entry[1].
     *   - Otherwise return the event index from table1d3d23. */
    for (int16_t i = 0; i < cb->table1d3ba0_count; i++) {
        const uint8_t *entry = &cb->table1d3ba0[i * 3];
        if ((int16_t)entry[0] != bbw)
            continue;

        if (entry[2] & 0x80) {
            /* Recursive: look up sub-table */
            int16_t sub_result = 0;
            if (cb->recurse) {
                sub_result = cb->recurse(ctx, (int16_t)entry[1]);
                r.recursed = 1;
            }
            if (sub_result != 0) {
                r.handled = 1;
                r.event_index = sub_result;
                if (receipt) *receipt = r;
                return sub_result;
            }
        } else {
            /* Direct lookup from table1d3d23 */
            int16_t evt_idx = cb->table1d3d23[(int)entry[1]];
            if (evt_idx == 0 && cb->v1d39bc)
                evt_idx = cb->v1d39bc[(int)entry[1]];
            r.handled = 1;
            r.event_index = evt_idx;
            if (receipt) *receipt = r;
            return evt_idx;
        }
    }

    r.handled = 1;
    r.event_index = 0;
    if (receipt) *receipt = r;
    return 0;
}

/* ---- DM2_0b36_129a (c_input.cpp:522) ---- */

void dm2_v1_0b36_129a(
    const char *str, int16_t color,
    const DM2_V1_0b36_129aCallbacks *cb, void *ctx,
    DM2_V1_0b36_129aReceipt *receipt)
{
    DM2_V1_0b36_129aReceipt r;
    memset(&r, 0, sizeof(r));
    if (receipt) *receipt = r;
    if (!cb || !str) return;

    /* Query string metrics to center within button rect */
    int16_t str_w = 0;
    int16_t str_h = 0;
    if (cb->query_str_width)
        str_w = cb->query_str_width(ctx, str);
    if (cb->query_str_height)
        str_h = cb->query_str_height(ctx, str);

    /* Center horizontally, vertically within button group area */
    int16_t draw_x = (int16_t)(cb->btn_x + (cb->btn_w - str_w) / 2);
    int16_t draw_y = (int16_t)(cb->btn_y + (cb->btn_h - str_h) / 2);

    if (cb->draw_string)
        cb->draw_string(ctx, draw_x, draw_y, str, color);

    r.handled = 1;
    r.draw_x = draw_x;
    r.draw_y = draw_y;
    if (receipt) *receipt = r;
}
