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

void dm2_v1_process_timer_0c_cb(
    int16_t hero_index,
    const DM2_V1_Timer0CCallbacks *cb)
{
    (void)hero_index;
    if (!cb)
        return;
    *cb->hero_timeridx = -1;
    if (*cb->hero_curHP != 0)
        *cb->hero_heroflag |= 0x800;
}

void dm2_v1_process_timer_sound(
    int16_t sound_id,
    const DM2_V1_SoundTimerCallbacks *cb, void *ctx)
{
    if (!cb || !cb->play_sound)
        return;
    cb->play_sound(ctx, sound_id);
}

void dm2_v1_process_timer_update_weather(
    int16_t mode,
    const DM2_V1_WeatherTimerCallbacks *cb, void *ctx)
{
    if (!cb || !cb->update_weather)
        return;
    cb->update_weather(ctx, mode);
}

void dm2_v1_process_timer_hero_ench_flag(
    const DM2_V1_HeroEnchFlagCallbacks *cb)
{
    if (!cb || !cb->counter)
        return;
    (*cb->counter)--;
    if (*cb->counter == 0 && cb->hero_slot > 0) {
        int idx = cb->hero_slot - 1;
        if (cb->hero_curHP && cb->hero_curHP[idx] != 0 && cb->hero_heroflag)
            cb->hero_heroflag[idx] |= 0x4000;
    }
}

void dm2_v1_process_timer_ench_power(
    uint8_t actor_mask, int16_t amount,
    const DM2_V1_EnchPowerCallbacks *cb)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->heros_in_party; i++) {
        if ((actor_mask & (1 << i)) == 0)
            continue;
        if (cb->hero_curHP && cb->hero_curHP[i] != 0)
            cb->hero_ench_power[i] -= amount;
        if (cb->hero_ench_power[i] < 0)
            cb->hero_ench_power[i] = 0;
    }
}

void dm2_v1_process_timer_poison(
    uint8_t actor, int16_t amount,
    const DM2_V1_PoisonTimerCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->hero_poisoned[actor]--;
    cb->hero_poison[actor] -= amount;
    if (cb->process_poison)
        cb->process_poison(ctx, (int16_t)actor, amount);
}

void dm2_v1_process_timer_59(
    uint16_t record_word, int16_t timer_map,
    const DM2_V1_Timer59Callbacks *cb, void *ctx)
{
    if (!cb || !cb->get_record_address)
        return;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec)
        return;
    if (rec[4] & 0x04)
        return;
    if (timer_map == cb->party_map && cb->redraw_byte)
        *cb->redraw_byte |= 1;
    rec[4] &= ~0x01;
}

void dm2_v1_process_timer_move_record_rotate(
    uint16_t value_a, int16_t timer_map,
    int16_t party_x, int16_t party_y,
    const DM2_V1_MoveRecordRotateCallbacks *cb, void *ctx)
{
    if (!cb || !cb->move_record_to)
        return;
    if (timer_map != cb->party_map)
        return;
    uint16_t record = (value_a << 6) >> 11;
    (void)record;
    int16_t x = (int16_t)(value_a & 0x1f);
    cb->move_record_to(ctx, 0xFFFF, party_x, party_y, x, 0);
    int16_t dir = (int16_t)((value_a << 4) >> 14);
    if (cb->party_rotate)
        cb->party_rotate(ctx, dir);
}

void dm2_v1_process_timer_alloc_new_creature(
    uint8_t x, uint8_t y, uint8_t creature_type,
    const DM2_V1_AllocNewCreatureCallbacks *cb, void *ctx)
{
    if (!cb || !cb->alloc_new_creature || !cb->rand_dir)
        return;
    int16_t dir = cb->rand_dir(ctx);
    if (dir == 0 || !cb->calc_vector_dir)
        dir = cb->rand_dir(ctx);
    else
        dir = cb->calc_vector_dir(ctx, x, y, cb->party_x, cb->party_y);
    cb->alloc_new_creature(ctx, creature_type, 7, dir, x, y);
}

int dm2_v1_process_timer_resurrection(
    uint8_t xA, uint8_t yA, uint8_t xB, uint8_t yB, uint8_t actor,
    const DM2_V1_ResurrectionCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int16_t new_yb;

    if (yB == 0) {
        if (cb->bring_champion_to_life)
            cb->bring_champion_to_life(ctx, (int16_t)actor);
        return 1; /* final phase: no re-queue */
    } else if (yB == 1) {
        int16_t rec = cb->get_tile_record_link
            ? cb->get_tile_record_link(ctx, (int16_t)xA, (int16_t)yA) : -1;
        int found = 0;
        int guard = 0;
        while (rec >= 0 && !found && guard < 4096) {
            int16_t cls1 = cb->query_cls1_from_record
                ? cb->query_cls1_from_record(ctx, rec) : -1;
            int16_t cls2 = cb->query_cls2_from_record
                ? cb->query_cls2_from_record(ctx, rec) : -1;
            if (cls1 == 0x15 && cls2 == 0) {
                if (cb->add_item_charge)
                    cb->add_item_charge(ctx, rec, 1);
                if (rec == actor) {
                    if (cb->cut_record_from)
                        cb->cut_record_from(ctx, (uint16_t)rec, (int16_t)xA, (int16_t)yA);
                    if (cb->dealloc_record)
                        cb->dealloc_record(ctx, (uint16_t)rec);
                    found = 1;
                }
            }
            if (!cb->get_next_record_link)
                break;
            rec = cb->get_next_record_link(ctx, (uint16_t)rec);
            guard++;
        }
        new_yb = 0;
    } else { /* yB >= 2: create cloud phase */
        if (cb->create_cloud)
            cb->create_cloud(ctx, 0x28, 5, (int16_t)xB, (int16_t)yB, 0);
        new_yb = (int16_t)(yB - 1);
    }

    if (cb->queue_timer)
        cb->queue_timer(ctx);
    (void)new_yb;
    return 1;
}

int dm2_v1_process_cloud(
    uint16_t record_word, uint8_t xA, uint8_t yA,
    const DM2_V1_ProcessCloudCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t *rec = cb->get_record_address ? cb->get_record_address(ctx, record_word) : NULL;
    if (!rec)
        return 0;

    uint8_t cloud_type = rec[4];
    uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
    int16_t decay = (int16_t)(w2 & 0x7F);

    uint8_t tile = cb->get_tile_value ? cb->get_tile_value(ctx, (int16_t)xA, (int16_t)yA) : 0;
    if (((tile >> 5) & 0x7) == 4 && cb->attack_door)
        cb->attack_door(ctx, (int16_t)xA, (int16_t)yA, decay, cloud_type, 0);

    int16_t damage = cb->calc_cloud_damage ? cb->calc_cloud_damage(ctx, record_word, 0) : decay;

    if ((int16_t)xA == cb->party_x && (int16_t)yA == cb->party_y && cb->attack_party)
        cb->attack_party(ctx, damage, cloud_type, 0);

    if (cb->get_creature_at) {
        int16_t creature = cb->get_creature_at(ctx, (int16_t)xA, (int16_t)yA);
        if (creature >= 0 &&
            (!cb->is_creature_immune || !cb->is_creature_immune(ctx, creature)) &&
            cb->attack_creature) {
            cb->attack_creature(ctx, creature, (int16_t)xA, (int16_t)yA, cloud_type, decay, damage);
        }
    }

    int decayed = 0;
    if (cloud_type == 7) {
        decay -= 1;
        decayed = 1;
    } else if (cloud_type == 0x28) {
        decay -= 5;
        decayed = 1;
    } else if (cloud_type == 0x64) {
        decay -= 1;
        decayed = 1;
        if (cb->queue_noise_gen2 && cb->current_map == cb->party_map)
            cb->queue_noise_gen2(ctx, 4, 0, 0x40, 0x40, (int16_t)xA, (int16_t)yA, 0, 0, 0);
    }

    if (decayed && decay <= 0) {
        if (cb->cut_record_from)
            cb->cut_record_from(ctx, record_word, (int16_t)xA, (int16_t)yA);
        if (cb->dealloc_record)
            cb->dealloc_record(ctx, record_word);
        return 1;
    }

    if (decayed) {
        w2 = (uint16_t)((w2 & 0xFF80) | ((uint16_t)decay & 0x7F));
        rec[2] = (uint8_t)(w2 & 0xFF);
        rec[3] = (uint8_t)(w2 >> 8);
    }

    if (cb->queue_timer)
        cb->queue_timer(ctx);
    return 1;
}

int dm2_v1_step_missile(
    uint16_t record_word, uint8_t x, uint8_t y,
    const DM2_V1_StepMissileCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t *rec = cb->get_record_address ? cb->get_record_address(ctx, record_word) : NULL;
    if (!rec)
        return 0;

    uint8_t direction = (uint8_t)(rec[4] & 0x03);
    int16_t dx = cb->dx_table ? cb->dx_table[direction] : 0;
    int16_t dy = cb->dy_table ? cb->dy_table[direction] : 0;
    int16_t nx = (int16_t)((int16_t)x + dx);
    int16_t ny = (int16_t)((int16_t)y + dy);

    uint8_t tile = cb->get_tile_value ? cb->get_tile_value(ctx, nx, ny) : 0;
    int is_wall = (((tile >> 5) & 0x7) == 0);

    if (is_wall) {
        if (cb->delete_missile_record)
            cb->delete_missile_record(ctx, record_word, (int16_t)x, (int16_t)y);
        return 1;
    }

    if (nx == cb->party_x && ny == cb->party_y) {
        if (cb->attack_creature)
            cb->attack_creature(ctx, -1, nx, ny, rec[4], 0, 0);
        if (cb->delete_missile_record)
            cb->delete_missile_record(ctx, record_word, (int16_t)x, (int16_t)y);
        return 1;
    }

    int16_t creature = cb->get_creature_at ? cb->get_creature_at(ctx, nx, ny) : -1;
    if (creature >= 0) {
        int reflect = 0;
        if (cb->query_creature_ai_spec_flags)
            reflect = (cb->query_creature_ai_spec_flags(ctx, creature) & 0x2) != 0;
        if (reflect) {
            rec[4] = (uint8_t)((rec[4] & ~0x03) | ((direction + 2) & 0x03));
            if (cb->queue_timer)
                cb->queue_timer(ctx);
            return 1;
        }
        if (cb->attack_creature)
            cb->attack_creature(ctx, creature, nx, ny, rec[4], 0, 0);
        if (cb->delete_missile_record)
            cb->delete_missile_record(ctx, record_word, (int16_t)x, (int16_t)y);
        return 1;
    }

    if (cb->cut_record_from)
        cb->cut_record_from(ctx, record_word, (int16_t)x, (int16_t)y);
    if (cb->append_record_to)
        cb->append_record_to(ctx, record_word, nx, ny);
    if (cb->move_record)
        cb->move_record(ctx, cb->current_map, nx, ny, direction, record_word);
    if (cb->queue_timer)
        cb->queue_timer(ctx);
    return 1;
}

int dm2_v1_continue_ornate_noise(
    uint16_t record_word, uint8_t timer_xA, uint8_t timer_yA,
    int16_t timer_map,
    const DM2_V1_OrnateNoiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t *rec = cb->get_record_address ? cb->get_record_address(ctx, record_word) : NULL;
    if (!rec)
        return 0;

    if (rec[4] & 0x01) {
        uint8_t decoration = 0;
        uint8_t tile = cb->get_tile_value ? cb->get_tile_value(ctx, (int16_t)timer_xA, (int16_t)timer_yA) : 0;
        int is_wall = ((tile >> 5) == 0);
        if (is_wall) {
            if (cb->get_wall_decoration)
                decoration = cb->get_wall_decoration(ctx, rec);
        } else {
            if (cb->get_floor_decoration)
                decoration = cb->get_floor_decoration(ctx, rec);
        }
        int16_t anim_len = cb->get_ornate_anim_len
            ? cb->get_ornate_anim_len(ctx, rec, decoration) : 0;
        if (cb->queue_timer_with_data)
            cb->queue_timer_with_data(ctx, anim_len);
        if (cb->queue_noise_gen2 && timer_map == cb->party_map)
            cb->queue_noise_gen2(ctx, 3, decoration, 0x40, 0x40,
                                 (int16_t)timer_xA, (int16_t)timer_yA, 0, 0, 0);
    } else {
        uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
        w2 &= 0x00FF; /* clear high-byte frame counter */
        rec[2] = (uint8_t)(w2 & 0xFF);
        rec[3] = (uint8_t)(w2 >> 8);
    }
    return 1;
}
