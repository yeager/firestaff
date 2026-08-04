/* DM2 V1 timer handler operations — skproject c_tim_proc.cpp. */

#include "dm2_v1_timer_ops_pc34_compat.h"
#include <stddef.h>

int16_t dm2_v1_process_timer_light(
    int16_t intensity,
    const DM2_V1_LightTimerCallbacks *cb, void *ctx)
{
    if (!cb || intensity == 0)
        return 0;
    int is_negative = intensity < 0 ? 1 : 0;
    int16_t abs_val = is_negative ? (int16_t)-intensity : intensity;
    int16_t decremented = (int16_t)(abs_val - 1);

    if (abs_val >= cb->light_table_size || decremented < 0)
        return 0;

    int16_t cur_light = cb->light_table[abs_val];
    int16_t prev_light = (decremented >= 0 && decremented < cb->light_table_size)
        ? cb->light_table[decremented] : 0;
    int16_t delta = (int16_t)(cur_light - prev_light);

    if (is_negative == 0) {
        /* Darkness: double the delta, subtract from light */
        delta = (int16_t)(2 * delta);
    } else {
        /* Light source: negate delta, negate decremented for re-queue */
        delta = (int16_t)-delta;
        decremented = (int16_t)-decremented;
    }

    *cb->global_light = (int16_t)(*cb->global_light + delta);

    if (is_negative ? (decremented != 0) : (decremented != 0)) {
        cb->queue_light_timer(ctx, decremented, 8);
    }

    return decremented;
}

void dm2_v1_process_timer_release_door_button(
    uint16_t record_word,
    const DM2_V1_RecordAddressCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (rec)
        rec[3] &= ~0x08;
}

void dm2_v1_process_timer_destroy_door(
    uint8_t tile_x, uint8_t tile_y,
    const DM2_V1_DestroyDoorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *tile = cb->get_tile_byte(ctx, tile_x, tile_y);
    if (!tile)
        return;
    *tile = (*tile & 0xF8) | 0x05;
    if (cb->current_map == cb->party_map && cb->redraw_flags)
        *cb->redraw_flags = 3;
}

void dm2_v1_process_timer_3d(
    uint16_t record_word, uint8_t x, uint8_t y, uint8_t timer_type,
    const DM2_V1_Timer3DCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int result = cb->move_record_to(ctx, record_word, -3, 0, x, y);
    if (result != 0 || timer_type == 0x3D)
        cb->queue_noise(ctx, x, y);
}

void dm2_v1_process_timer_0e(
    uint16_t record_db_type, uint16_t value2,
    uint8_t actor, uint16_t bonus_value,
    const DM2_V1_Timer0ECallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int32_t size = cb->get_item_size(record_db_type);
    if (size <= 0)
        return;
    uint16_t record = (uint16_t)((0 << 14) | (record_db_type << 10));
    uint8_t *rec = cb->get_record_address(ctx, record);
    if (!rec)
        return;
    void *backup = cb->alloc_memory(ctx, size);
    if (!backup)
        return;
    cb->copy_memory(backup, rec, size);
    cb->set_itemtype(ctx, record, value2);
    cb->process_item_bonus(ctx, actor, record, -1, bonus_value);
    cb->copy_memory(rec, backup, size);
    cb->dealloc_memory(ctx, backup, size);
}

int dm2_v1_continue_ornate_animator(
    uint16_t record_word, int anim_mode,
    const DM2_V1_OrnateAnimCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec)
        return 0;
    int16_t anim_len = cb->get_ornate_anim_len(ctx, rec, anim_mode);
    if (anim_len <= 0)
        return 0;
    uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
    uint16_t frame = (w2 >> 7) & 0x1FF;
    frame = (uint16_t)((frame + 1) & 0x1FF);
    w2 = (uint16_t)((w2 & 0x007F) | (frame << 7));
    rec[2] = (uint8_t)(w2 & 0xFF);
    rec[3] = (uint8_t)(w2 >> 8);
    if ((frame % (uint16_t)anim_len) == 0) {
        rec[4] &= 0xFE;
        return 0;
    }
    cb->queue_timer(ctx);
    return 1;
}

int dm2_v1_continue_tick_generator(
    uint16_t record_word, DM2_V1_TickGenTimerState *timer_state,
    const DM2_V1_TickGenCallbacks *cb, void *ctx)
{
    if (!cb || !timer_state)
        return 0;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec)
        return 0;
    uint16_t w4 = (uint16_t)(rec[4] | (rec[5] << 8));
    uint16_t action;
    uint16_t param;
    int should_invoke = 1;
    int remaining;

    if ((w4 & 0x18) == 0x18) {
        /* Alternating mode */
        uint8_t toggle = (timer_state->timer_yb >> 1) & 1;
        toggle ^= 1;
        param = toggle;
        timer_state->timer_b_bit8 = toggle;
        uint16_t act_bits = (w4 << 13) >> 15;
        remaining = (int)(act_bits | toggle);
        uint8_t yb_bit0 = (timer_state->timer_yb & 1) == 0 ? 1 : 0;
        action = yb_bit0;
    } else {
        /* Normal mode */
        action = (w4 << 13) >> 15;
        remaining = (int)action;
        if (remaining == 0)
            should_invoke = 0;
        else {
            action = (uint16_t)((w4 << 11) >> 14);
            param = 0;
        }
    }

    if (should_invoke)
        cb->invoke_actuator(ctx, rec, action, param);

    if (remaining != 0) {
        uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
        uint16_t delay_base = w2 >> 7;
        cb->requeue_timer(ctx, delay_base, timer_state->timer_yb);
        return 1;
    }
    rec[4] &= 0xFE;
    return 0;
}

int dm2_v1_activate_tick_generator_cb(
    uint8_t *actuator_record, uint16_t record_idx,
    const DM2_V1_ActivateTickGenCallbacks *cb, void *ctx)
{
    if (!cb || !actuator_record)
        return 0;
    uint16_t w2 = (uint16_t)(actuator_record[2] | (actuator_record[3] << 8));
    uint16_t subtype = w2 & 0x7F;
    uint8_t multiplier = 0;
    switch (subtype) {
    case 0x1E: multiplier = 1;   break;
    case 0x33: multiplier = 8;   break;
    case 0x34: multiplier = 16;  break;
    case 0x35: multiplier = 32;  break;
    case 0x36: multiplier = 64;  break;
    case 0x37: multiplier = 128; break;
    default: return 0;
    }
    uint16_t period = w2 >> 7;
    if (period == 0)
        return 0;
    uint32_t cycle = (uint32_t)period * (uint32_t)multiplier;
    uint32_t phase = cb->game_tick % cycle;
    uint32_t fire_tick = phase + cb->game_tick;
    cb->queue_tick_timer(ctx, record_idx, multiplier, fire_tick);
    actuator_record[4] |= 0x01;
    return 1;
}

int dm2_v1_skw_3a15_0d5c(
    const uint8_t *actuator_record, uint8_t timer_yb,
    const DM2_V1_RotateCreatureActCallbacks *cb, void *ctx)
{
    if (!cb || !actuator_record)
        return 0;
    int bit5 = (actuator_record[4] & 0x20) != 0;
    if (bit5 || timer_yb != 0) {
        if (!bit5)
            return 0;
        if (timer_yb != 1)
            return 0;
    }
    uint16_t w6 = (uint16_t)(actuator_record[6] | (actuator_record[7] << 8));
    uint16_t target_y = (w6 >> 11) & 0x1F;
    uint16_t target_x = (w6 >> 6) & 0x1F;
    uint16_t creature = cb->get_creature_at(ctx, target_x, target_y);
    if (creature == 0xFFFF)
        return 0;
    uint16_t w2 = (uint16_t)(actuator_record[2] | (actuator_record[3] << 8));
    int mode_bits = (w2 >> 7) & 0x3;
    cb->rotate_creature(ctx, creature, 1, mode_bits);
    return 1;
}
