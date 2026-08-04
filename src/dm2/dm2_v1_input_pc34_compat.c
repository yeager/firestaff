/*
 * dm2_v1_input_pc34_compat.c — DM2 input dispatcher and event interpreter.
 *
 * Source: skproject/SKWINSPX/src/v5/c_input.cpp
 *
 * Handles UI event dispatch (227-case switch), bytecode event execution,
 * input polling, and the outer event loop.
 */

#include "dm2_v1_input_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* TODO: fill from skproject c_input.cpp table1d3efd */
const int8_t dm2_v1_table1d3efd[236] = {0};

void dm2_v1_handle_ui_event(
    const DM2_V1_HandleUiEventCallbacks *cb,
    void *ctx,
    const DM2_V1_HandleUiEventRequest *req,
    DM2_V1_HandleUiEventReceipt *receipt)
{
    if (!req || !receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->dispatched_idx = req->event_idx;

    if (!cb) {
        receipt->handled = 0;
        return;
    }

    switch (req->event_idx) {
    case 227:
        if (cb->release_mouse) cb->release_mouse(ctx);
        receipt->dispatch_category = 27;
        break;
    case 225:
        if (cb->clear_1031) cb->clear_1031(ctx);
        receipt->dispatch_category = 25;
        break;
    case 1: case 2:
        if (cb->perform_turn) cb->perform_turn(ctx, req->event_idx);
        receipt->dispatch_category = 1;
        break;
    case 3: case 4: case 5: case 6:
        if (cb->perform_move) cb->perform_move(ctx, req->event_idx);
        receipt->dispatch_category = 2;
        break;
    case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27:
    case 28: case 29: case 30: case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43:
    case 44: case 45: case 46: case 47: case 48: case 49: case 50: case 51:
    case 52: case 53: case 54: case 55: case 56: case 57: case 58: case 59:
    case 60: case 61: case 62: case 63: case 64: case 65:
        if (cb->click_item_slot) cb->click_item_slot(ctx, req->event_idx - 20);
        receipt->dispatch_category = 3;
        break;
    case 234: case 235: case 236: case 237:
        if (cb->put_item_to_player)
            cb->put_item_to_player(ctx, req->event_idx - 234);
        receipt->dispatch_category = 4;
        break;
    case 125: case 126: case 127: case 128: case 129:
        if (cb->change_player_pos)
            cb->change_player_pos(ctx, req->event_idx - 125);
        receipt->dispatch_category = 5;
        break;
    case 228: case 229: case 230: case 231: case 232: case 233:
        if (cb->click_moneybox)
            cb->click_moneybox(ctx, req->event_idx - 228);
        receipt->dispatch_category = 6;
        break;
    case 93:
        if (cb->rotate_direction) cb->rotate_direction(ctx, 0);
        receipt->dispatch_category = 7;
        break;
    case 94:
        if (cb->rotate_direction) cb->rotate_direction(ctx, 1);
        receipt->dispatch_category = 7;
        break;
    case 16: case 17: case 18: case 19:
        if (cb->select_leader) cb->select_leader(ctx, req->event_idx - 16);
        receipt->dispatch_category = 8;
        break;
    case 7: case 8: case 9: case 10: case 11:
        if (cb->panel_button) cb->panel_button(ctx, req->event_idx - 7);
        receipt->dispatch_category = 9;
        break;
    case 82:
        if (cb->panel_button) cb->panel_button(ctx, 82);
        receipt->dispatch_category = 9;
        break;
    case 108:
        if (cb->try_cast_spell) cb->try_cast_spell(ctx);
        receipt->dispatch_category = 10;
        break;
    case 107:
        if (cb->remove_rune) cb->remove_rune(ctx);
        receipt->dispatch_category = 11;
        break;
    case 101: case 102: case 103: case 104: case 105: case 106:
        if (cb->add_rune) cb->add_rune(ctx, req->event_idx - 101);
        receipt->dispatch_category = 12;
        break;
    case 112:
        if (cb->proceed_command_slot) cb->proceed_command_slot(ctx, -1);
        receipt->dispatch_category = 13;
        break;
    case 113: case 114: case 115:
        if (cb->proceed_command_slot)
            cb->proceed_command_slot(ctx, req->event_idx - 113);
        receipt->dispatch_category = 13;
        break;
    case 116: case 117: case 118: case 119:
    case 120: case 121: case 122: case 123:
        if (cb->activate_action_hand)
            cb->activate_action_hand(ctx, req->event_idx - 116);
        receipt->dispatch_category = 14;
        break;
    case 95: case 96: case 97: case 98:
        if (cb->set_spelling_champion)
            cb->set_spelling_champion(ctx, req->event_idx - 95);
        receipt->dispatch_category = 15;
        break;
    case 70:
        if (cb->consume_object) cb->consume_object(ctx);
        receipt->dispatch_category = 16;
        break;
    case 71:
        if (cb->click_eye) cb->click_eye(ctx);
        receipt->dispatch_category = 17;
        break;
    case 72:
        if (cb->click_hero_stat) cb->click_hero_stat(ctx);
        receipt->dispatch_category = 18;
        break;
    case 80:
        if (cb->click_viewport)
            cb->click_viewport(ctx, req->x, req->y);
        receipt->dispatch_category = 19;
        break;
    case 240: case 241: case 242:
        if (cb->event_13262)
            cb->event_13262(ctx, req->event_idx - 240);
        receipt->dispatch_category = 20;
        break;
    case 85:
        if (cb->click_map_at) cb->click_map_at(ctx);
        receipt->dispatch_category = 21;
        break;
    case 86: case 87: case 88: case 89:
        if (cb->click_map_rune)
            cb->click_map_rune(ctx, req->event_idx - 86);
        receipt->dispatch_category = 22;
        break;
    case 140:
        if (cb->game_save_menu) cb->game_save_menu(ctx);
        receipt->dispatch_category = 23;
        break;
    case 142: case 143: case 144: case 145:
        if (cb->sleep_wake) cb->sleep_wake(ctx, req->event_idx - 142);
        receipt->dispatch_category = 24;
        break;
    case 218:
        if (cb->show_credits) cb->show_credits(ctx);
        receipt->dispatch_category = 26;
        break;
    case 224:
        if (cb->prepare_exit) cb->prepare_exit(ctx);
        receipt->dispatch_category = 28;
        break;
    case 215: case 216: case 217:
        if (cb->set_savegame_flag)
            cb->set_savegame_flag(ctx, req->event_idx - 215, 1);
        receipt->dispatch_category = 29;
        break;
    default:
        receipt->handled = 0;
        return;
    }

    receipt->handled = 1;
}

void dm2_v1_exec_event(
    const DM2_V1_ExecEventCallbacks *cb,
    void *ctx,
    const int8_t *bytecode)
{
    const uint8_t *p;
    uint8_t raw, opcode;
    int cont;

    if (!bytecode || !cb) return;

    p = (const uint8_t *)bytecode;

    for (;;) {
        raw = *p++;
        if (raw & 0x80) break;

        cont = (raw & 0x40) != 0;
        opcode = raw & 0x3F;
        if (opcode > 16) opcode = 16;

        switch (opcode) {
        case 0: {
            uint8_t val = *p++;
            if (cb->set_delay) cb->set_delay(ctx, val);
            break;
        }
        case 1:
            if (cb->set_event_unk02) cb->set_event_unk02(ctx);
            break;
        case 2: {
            int16_t val = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            if (cb->set_v1e0478) cb->set_v1e0478(ctx, val);
            break;
        }
        case 3: {
            int16_t val = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            if (cb->set_v1e048c) cb->set_v1e048c(ctx, val);
            break;
        }
        case 4: {
            uint8_t val = *p++;
            if (cb->push_event_state) cb->push_event_state(ctx, val);
            break;
        }
        case 5: {
            int16_t val = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            if (cb->handle_ui_event) cb->handle_ui_event(ctx, val, 0, 0);
            break;
        }
        case 6: {
            uint8_t a = *p++;
            uint8_t b = *p++;
            uint8_t c = *p++;
            int16_t x = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            int16_t y = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            if (cb->draw_gdat_image)
                cb->draw_gdat_image(ctx, a, b, c, x, y);
            break;
        }
        case 7: {
            uint8_t val = *p++;
            if (cb->draw_squad_icon) cb->draw_squad_icon(ctx, val);
            break;
        }
        case 8: {
            int16_t param = (int16_t)(p[0] | (p[1] << 8));
            p += 2;
            if (cb->resolve_hero) cb->resolve_hero(ctx, param);
            break;
        }
        case 9: {
            uint8_t val = *p++;
            if (cb->draw_rune) cb->draw_rune(ctx, val);
            break;
        }
        case 10:
            if (cb->draw_spell) cb->draw_spell(ctx);
            break;
        case 11: {
            uint8_t val = *p++;
            if (cb->draw_command_slot) cb->draw_command_slot(ctx, val);
            break;
        }
        case 12: {
            uint8_t val = *p++;
            if (cb->refresh_stat) cb->refresh_stat(ctx, val);
            break;
        }
        case 13:
            if (cb->draw_guide) cb->draw_guide(ctx);
            break;
        case 14: {
            uint8_t val = *p++;
            if (cb->gameload_dialogue) cb->gameload_dialogue(ctx, val);
            break;
        }
        case 15:
        case 16:
            /* no-op, consume no extra bytes */
            break;
        }

        if (!cont) break;
    }
}

void dm2_v1_input_check(
    const DM2_V1_InputCheckCallbacks *cb,
    void *ctx,
    DM2_V1_InputCheckReceipt *receipt)
{
    if (!cb || !receipt) return;
    memset(receipt, 0, sizeof(*receipt));

    /* Poll keyboard */
    if (cb->has_key && cb->get_key) {
        while (cb->has_key(ctx)) {
            cb->get_key(ctx);
            receipt->keys_processed++;
        }
    }

    /* Poll UI events */
    if (cb->has_event && cb->get_event) {
        while (cb->has_event(ctx)) {
            int16_t ex = 0, ey = 0;
            int16_t event_idx = cb->get_event(ctx, &ex, &ey);
            receipt->events_processed++;

            const int8_t *route = NULL;
            if (cb->event_table)
                route = cb->event_table(ctx, event_idx);

            if (route) {
                if (cb->exec_event)
                    cb->exec_event(ctx, route);
                if (cb->transmit_ui_event)
                    cb->transmit_ui_event(ctx, event_idx, ex, ey);
            } else {
                if (cb->set_ui_event)
                    cb->set_ui_event(ctx, event_idx, ex, ey);
            }
        }
    }
}

void dm2_v1_event_loop(
    const DM2_V1_InputCheckCallbacks *cb,
    void *ctx)
{
    DM2_V1_InputCheckReceipt receipt;
    dm2_v1_input_check(cb, ctx, &receipt);
}

int16_t dm2_v1_1031_03f2(
    int16_t key,
    const int16_t *tree,
    int16_t tree_size)
{
    if (!tree || tree_size <= 0) return -1;

    for (int16_t i = 0; i < tree_size; i++) {
        if (tree[i] == key) return i;
    }

    return -1;
}

