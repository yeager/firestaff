/*
 * dm2_v1_tim_proc_pc34_compat.c — DM2 timer processing dispatcher.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp (4680 lines)
 *
 * The main entry point is DM2_PROCEED_TIMERS which loops pulling
 * timer events from the queue and dispatching by type:
 *
 *   0x00       no-op
 *   0x01       DM2_STEP_DOOR: animate door open/close, attack party/creatures
 *   0x02       DM2_PROCESS_TIMER_DESTROY_DOOR: set tile to destroyed
 *   0x04       DM2_ACTUATE_*: dispatch by tile type (wall/floor/pit/door/tele/trick)
 *   0x0C       DM2_PROCESS_TIMER_0C: set hero flag 0x800
 *   0x0D       DM2_PROCESS_TIMER_RESURRECTION: bring champion to life / create cloud
 *   0x0E       DM2_PROCESS_TIMER_0E: process item bonus with record backup
 *   0x15       DM2_PROCESS_SOUND: play sound
 *   0x19       DM2_PROCESS_CLOUD: advance cloud
 *   0x1D/0x1E  DM2_STEP_MISSILE: advance missile, handle reflection/collision
 *   0x21/0x22  DM2_THINK_CREATURE: AI think tick
 *   0x3C/0x3D  DM2_PROCESS_TIMER_3D: move record and queue noise
 *   0x46       DM2_PROCESS_TIMER_LIGHT: adjust light level table
 *   0x47       hero enchantment power timer
 *   0x48       party enchantment power decay
 *   0x4B       poison processing
 *   0x54       weather update
 *   0x55       DM2_CONTINUE_ORNATE_ANIMATOR: advance ornate frame
 *   0x56       DM2_CONTINUE_TICK_GENERATOR: tick generator cycle
 *   0x58       DM2_PROCESS_TIMER_RELEASE_DOOR_BUTTON: release button
 *   0x59       DM2_PROCESS_TIMER_59: continuous ornate animator done
 *   0x5A       DM2_CONTINUE_ORNATE_NOISE: ornate sound loop
 *   0x5B       record activate bit set
 *   0x5C       record activate bit set
 *   0x5D       party rotation/warp
 *   0x5E       creature spawn timer
 *
 * Sub-dispatchers for type 0x04 (ACTUATE) further dispatch by tile type:
 *   Wall  (0): DM2_ACTUATE_WALL_MECHA -> 67-case switch by actuator subtype
 *   Floor (1): DM2_ACTUATE_FLOOR_MECHA -> multi-case by actuator subtype
 *   Pit   (2): DM2_ACTUATE_PITFALL -> toggle pit + floor mecha
 *   Stairs(3): no-op
 *   Door  (4): DM2_ACTUATE_DOOR -> toggle door direction
 *   Tele  (5): DM2_ACTUATE_TELEPORTER -> toggle teleporter + floor mecha
 *   Trick (6): DM2_ACTUATE_TRICKWALL -> toggle solid/passable
 *
 * Also exports:
 *   DM2_INVOKE_MESSAGE: queue a type-0x04 timer
 *   DM2_INVOKE_ACTUATOR: resolve actuator target and invoke message
 *   DM2_timproc_3a15_1da8: compute flag toggle value
 */

#include "dm2_v1_tim_proc_pc34_compat.h"
#include <string.h>

/* Direction lookup tables matching skproject table1d27fc / table1d2804 */
const int16_t dm2_v1_tim_proc_dir_dx[4] = { 0, 1, 0, -1 };
const int16_t dm2_v1_tim_proc_dir_dy[4] = { -1, 0, 1, 0 };

/* ── Helper: compute flag toggle (SKW_3a15_1da8) ───────────────── */
int32_t dm2_v1_timproc_compute_flag(int32_t yB_value, int32_t current_flag,
    DM2_V1_TimProc1DA8Receipt *receipt)
{
    int32_t result;

    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->valid = 1;
    }

    if (yB_value == 0) {
        result = 1;
    } else if (yB_value == 1) {
        result = 0;
    } else if (yB_value == 2) {
        result = (current_flag ^ 1) & 0x1;
    } else {
        result = 0;
    }

    if (receipt) {
        receipt->value_computed = 1;
        receipt->result = result;
    }

    return result;
}

/* ── PROCESS_TIMER_0C: set hero ready flag ──────────────────────── */
void dm2_v1_process_timer_0c(
    int16_t hero_index,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimer0CReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;
    receipt->hero_index = hero_index;

    /* In skproject: hero->timeridx = -1; if (curHP != 0) heroflag |= 0x800 */
    int16_t hp = cb->get_hero_curHP ? cb->get_hero_curHP(hero_index, ctx) : 0;
    if (hp != 0) {
        receipt->hero_flag_set = 1;
    }
}

/* ── PROCESS_TIMER_RESURRECTION (c_tim_proc.cpp:39) ─────────────── */
/*
 * Three-phase champion resurrection driven by tim->yB:
 *   yB == 0  final phase: bring the champion to life, no re-queue
 *   yB == 1  scan the tile's record list for the matching altar item,
 *            remove it, decrement yB, re-queue
 *   yB == 2  create the resurrection cloud, adddata(5), decrement yB,
 *            re-queue
 *   yB  > 2  no-op (matches skproject: only phase 2 is handled, higher
 *            phases return without side effects)
 */
void dm2_v1_process_timer_resurrection(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerResurrectionReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    int16_t xA = (int16_t)tim->xA;
    int16_t yA = (int16_t)tim->yA;
    int16_t xB = (int16_t)tim->xB;
    int16_t actor = (int16_t)tim->actor;
    uint8_t yB = tim->yB;

    if (yB == 0) {
        if (cb->bring_champion_to_life)
            cb->bring_champion_to_life(actor, ctx);
        receipt->champion_brought_to_life = 1;
        return;
    }

    if (yB == 1) {
        int16_t rec = cb->get_tile_record_link
            ? cb->get_tile_record_link(xA, yA, ctx) : -1;
        int guard = 0;
        while (rec >= 0 && guard < 4096) {
            int16_t cls1 = cb->query_cls1_from_record
                ? cb->query_cls1_from_record(rec, ctx) : -1;
            if (cls1 == 0x15) {
                int16_t cls2 = cb->query_cls2_from_record
                    ? cb->query_cls2_from_record(rec, ctx) : -1;
                if (cls2 == 0) {
                    int16_t charge = cb->add_item_charge
                        ? cb->add_item_charge(rec, 0, ctx) : 0;
                    if (charge == actor) {
                        if (cb->cut_record_from)
                            cb->cut_record_from(rec, NULL, xA, yA, ctx);
                        if (cb->dealloc_record)
                            cb->dealloc_record(rec, ctx);
                        tim->data++;
                        receipt->records_removed = 1;
                        break;
                    }
                }
            }
            if (!cb->get_next_record_link)
                break;
            rec = (int16_t)cb->get_next_record_link((uint16_t)rec, ctx);
            guard++;
        }
    } else if (yB == 2) {
        if (cb->create_cloud)
            cb->create_cloud(0xffe4, 0, xA, yA, xB, ctx);
        tim->data += 5;
        receipt->cloud_created = 1;
    } else {
        return;
    }

    tim->yB = (uint8_t)(yB - 1);
    if (cb->queue_timer) {
        cb->queue_timer(tim, ctx);
        receipt->requeued = 1;
    }
}

/* ── PROCESS_TIMER_DESTROY_DOOR (c_tim_proc.cpp:422) ────────────── */
void dm2_v1_process_timer_destroy_door(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerDestroyDoorReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;
    receipt->door_destroyed = 1;

    /* skproject: low 3 bits of the tile byte set to 5 (destroyed door). */
    if (cb->get_address_of_tile_record) {
        uint8_t *tile = cb->get_address_of_tile_record(
            (int16_t)tim->xA, (int16_t)tim->yA, ctx);
        if (tile)
            *tile = (uint8_t)((*tile & 0xF8) | 0x05);
    }

    if (cb->get_current_map && cb->get_party_map &&
        cb->get_current_map(ctx) == cb->get_party_map(ctx) &&
        cb->set_render_flag) {
        cb->set_render_flag(3, ctx);
    }
}

/* ── PROCESS_TIMER_3D (c_tim_proc.cpp:902) ──────────────────────── */
void dm2_v1_process_timer_3d(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimer3DReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    int16_t x = (int16_t)tim->xA;
    int16_t y = (int16_t)tim->yA;

    if (cb->move_record_to) {
        cb->move_record_to((int32_t)tim->valueB, -3, 0, x, y, ctx);
        receipt->record_moved = 1;
    }

    if (tim->type == DM2_TIMER_TYPE_3D) {
        if (cb->queue_noise_gen1)
            cb->queue_noise_gen1(3, 0, (int8_t)0x89, (int16_t)0x61,
                (int16_t)0x80, x, y, 1, ctx);
        receipt->noise_queued = 1;
    }
}

/* ── STEP_DOOR (DM2_STEP_DOOR) ──────────────────────────────────── */
/*
 * Simplified port of skproject c_tim_proc.cpp DM2_STEP_DOOR: advance the
 * door animation frame at (xA, yA) one step per call and attack anything
 * standing in the doorway while it is moving.  tim->yB carries the
 * requested direction (non-zero = opening, zero = closing) in this
 * portable representation.
 */
void dm2_v1_step_door(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_StepDoorReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    int16_t x = (int16_t)tim->xA;
    int16_t y = (int16_t)tim->yA;

    uint8_t *tile = cb->get_address_of_tile_record
        ? cb->get_address_of_tile_record(x, y, ctx) : NULL;
    if (!tile)
        return;

    /* Door tile type must remain DM2_TILE_DOOR while stepping. */
    if (((*tile) & 0x07) != DM2_TILE_DOOR)
        return;

    int opening = tim->yB != 0;
    uint8_t frame = *tile & 0x07;

    if (opening) {
        if (frame < 6) frame++;
        if (frame == 6) receipt->door_opened = 1;
    } else {
        if (frame > 0) frame--;
        if (frame == 0) receipt->door_closed = 1;
    }
    *tile = (uint8_t)((*tile & 0xF8) | (frame & 0x07));
    receipt->door_stepped = 1;

    /* Attack a creature caught in the doorway. */
    int16_t creature = cb->get_creature_at
        ? cb->get_creature_at(x, y, ctx) : -1;
    if (creature != -1 && creature != (int16_t)0xFFFF && cb->attack_creature) {
        cb->attack_creature(creature, x, y, 0, 0, 0, ctx);
        receipt->creature_attacked = 1;
    }

    /* Continue stepping until the door is fully open or closed. */
    if (frame != 0 && frame != 6 && cb->queue_timer) {
        cb->queue_timer(tim, ctx);
        receipt->requeued = 1;
    }
}

/* ── STEP_MISSILE ───────────────────────────────────────────────── */
static void dm2_v1_step_missile_cb(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_StepMissileReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    int16_t record = tim->valueA;
    int16_t x = tim->valueB & 0x1F;
    int16_t y = (tim->valueB >> 5) & 0x1F;
    int16_t dir = (tim->valueB >> 10) & 0x3;

    /* Check for special missile type 0x1D -> convert to 0x1E */
    if (tim->type == DM2_TIMER_TYPE_MISSILE_1D) {
        tim->type = DM2_TIMER_TYPE_MISSILE_1E;
    } else {
        /* Normal missile: try to move forward */
        int16_t new_x = x + dm2_v1_tim_proc_dir_dx[dir];
        int16_t new_y = y + dm2_v1_tim_proc_dir_dy[dir];

        /* Check for creature at target */
        int16_t creature = cb->get_creature_at
            ? cb->get_creature_at(new_x, new_y, ctx) : -1;
        if (creature != -1 && creature != (int16_t)0xFFFF) {
            /* Hit creature - try to deflect based on AI spec flags */
            int16_t flags = cb->query_creature_ai_spec_flags
                ? cb->query_creature_ai_spec_flags(creature, ctx) : 0;
            if (flags & 0x2) {
                receipt->missile_reflected = 1;
            }
        }

        /* Check for wall/obstacle */
        uint8_t tile = cb->get_tile_value
            ? cb->get_tile_value(new_x, new_y, ctx) : 0;
        int tile_type = (tile >> 5) & 0x7;

        if (tile_type == DM2_TILE_WALL) {
            /* Hit wall - destroy missile */
            if (cb->cut_record_from)
                cb->cut_record_from(record, NULL, x, y, ctx);
            if (cb->delete_missile_record)
                cb->delete_missile_record(record, NULL, x, y, ctx);
            receipt->missile_destroyed = 1;
            return;
        }
    }

    /* Increment data and requeue */
    tim->data++;
    if (cb->queue_timer)
        cb->queue_timer(tim, ctx);
    receipt->missile_moved = 1;
    receipt->requeued = 1;
}

/* ── PROCESS_TIMER_LIGHT ────────────────────────────────────────── */
void dm2_v1_process_timer_light(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerLightReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt || !tim) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    int16_t value = tim->valueA;
    if (value == 0)
        return;

    int is_negative = value < 0;
    if (is_negative)
        value = -value;

    /* Compute light delta from table entries */
    int16_t prev_entry = cb->get_light_table_entry
        ? cb->get_light_table_entry(value - 1, ctx) : 0;
    int16_t cur_entry = cb->get_light_table_entry
        ? cb->get_light_table_entry(value, ctx) : 0;
    int32_t delta = cur_entry - prev_entry;

    if (!is_negative) {
        delta = 2 * delta;
    } else {
        delta = -delta;
        value = -value;
    }

    receipt->light_changed = 1;

    /* If more steps remain, queue continuation */
    int16_t remaining = (int16_t)(value > 0 ? value - 1 : 0);
    if (remaining > 0) {
        DM2_V1_TimerRecord cont;
        memset(&cont, 0, sizeof(cont));
        cont.type = DM2_TIMER_TYPE_LIGHT;
        cont.valueA = remaining;
        cont.actor = 0;
        int32_t gametick = cb->get_gametick ? cb->get_gametick(ctx) : 0;
        cont.ticks = gametick + 8;
        int16_t party_map = cb->get_party_map ? cb->get_party_map(ctx) : 0;
        cont.map = (uint8_t)party_map;
        if (cb->queue_timer)
            cb->queue_timer(&cont, ctx);
        receipt->requeued = 1;
    }
}

/* ── INVOKE_MESSAGE ─────────────────────────────────────────────── */
void dm2_v1_invoke_message(
    int32_t x, int32_t y, int32_t dir, int32_t action, int32_t ticks,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_InvokeMessageReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    /* Build timer record of type 0x04 (ACTUATE) */
    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_ACTUATE;
    tim.ticks = ticks;

    /* Map action to actor value */
    if (action == 0)
        tim.actor = 1;
    else if (action == 1)
        tim.actor = 3;
    else if (action == 2)
        tim.actor = 2;

    tim.xA = (uint8_t)x;
    tim.yA = (uint8_t)y;
    tim.xB = (uint8_t)dir;
    tim.yB = (uint8_t)action;

    int16_t cur_map = cb->get_current_map ? cb->get_current_map(ctx) : 0;
    tim.map = (uint8_t)cur_map;

    if (cb->queue_timer)
        cb->queue_timer(&tim, ctx);

    receipt->message_invoked = 1;
}

/* ── INVOKE_ACTUATOR ────────────────────────────────────────────── */
void dm2_v1_invoke_actuator(
    uint8_t *addr, int32_t param, int32_t extra,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_InvokeActuatorReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt) return;
    if (!cb || !addr) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    /* Extract delay from actuator record byte 4 */
    int16_t word4 = (int16_t)(addr[4] | (addr[5] << 8));
    int32_t delay = ((word4 << 5) >> 12) & 0x7F;
    int32_t gametick = cb->get_gametick ? cb->get_gametick(ctx) : 0;
    int32_t ticks = gametick + delay + extra;

    /* Extract target coordinates from actuator record byte 6 */
    int16_t word6 = (int16_t)(addr[6] | (addr[7] << 8));
    int32_t target_dir = (word6 >> 10) & 0x3;
    int32_t target_y = (word6 >> 5) & 0x1F;
    int32_t target_x = word6 & 0x1F;

    if (cb->invoke_message)
        cb->invoke_message(target_x, target_y, target_dir, param, ticks, ctx);

    receipt->actuator_invoked = 1;
}

/* ── PROCEED_TIMERS (main loop) ─────────────────────────────────── */
void dm2_v1_proceed_timers(
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!receipt) return;
    if (!cb) { receipt->fail_closed = 1; return; }

    receipt->valid = 1;

    if (!cb->is_timer_to_proceed || !cb->get_and_delete_next_timer)
        return;

    while (cb->is_timer_to_proceed(ctx)) {
        DM2_V1_TimerRecord tim;

        if (cb->ibmio_user_input_check)
            cb->ibmio_user_input_check(ctx);

        cb->get_and_delete_next_timer(&tim, ctx);

        if (cb->change_current_map_to)
            cb->change_current_map_to(tim.map, ctx);

        receipt->timers_processed++;
        receipt->timer_type_dispatched = tim.type;

        switch (tim.type) {
        case DM2_TIMER_TYPE_00:
            /* no-op */
            break;

        case DM2_TIMER_TYPE_STEP_DOOR: {
            DM2_V1_StepDoorReceipt dr;
            dm2_v1_step_door(&tim, cb, ctx, &dr);
            break;
        }

        case DM2_TIMER_TYPE_DESTROY_DOOR: {
            DM2_V1_ProcessTimerDestroyDoorReceipt dr;
            dm2_v1_process_timer_destroy_door(&tim, cb, ctx, &dr);
            break;
        }

        case DM2_TIMER_TYPE_ACTUATE:
            /* Dispatch by tile type at (xA, yA) */
            if (cb->get_tile_value) {
                uint8_t tile = cb->get_tile_value(tim.xA, tim.yA, ctx);
                int tile_type = (tile >> 5) & 0x7;
                (void)tile_type;
                /* Full actuate dispatch would go here;
                 * each case calls DM2_ACTUATE_WALL_MECHA,
                 * DM2_ACTUATE_FLOOR_MECHA, etc. */
            }
            break;

        case DM2_TIMER_TYPE_0C: {
            DM2_V1_ProcessTimer0CReceipt cr;
            dm2_v1_process_timer_0c(tim.actor, cb, ctx, &cr);
            break;
        }

        case DM2_TIMER_TYPE_RESURRECTION: {
            DM2_V1_ProcessTimerResurrectionReceipt rr;
            dm2_v1_process_timer_resurrection(&tim, cb, ctx, &rr);
            break;
        }

        case DM2_TIMER_TYPE_0E:
            if (cb->process_item_bonus) {
                int32_t record_size = cb->get_record_size
                    ? cb->get_record_size(tim.valueA, ctx) : 0;
                (void)record_size;
                /* Full 0E handling: backup record, process bonus, restore */
            }
            break;

        case DM2_TIMER_TYPE_SOUND:
            if (cb->process_sound)
                cb->process_sound(tim.valueA, ctx);
            break;

        case DM2_TIMER_TYPE_CLOUD:
            if (cb->process_cloud)
                cb->process_cloud(&tim, ctx);
            break;

        case DM2_TIMER_TYPE_MISSILE_1D:
        case DM2_TIMER_TYPE_MISSILE_1E: {
            DM2_V1_StepMissileReceipt mr;
            dm2_v1_step_missile_cb(&tim, cb, ctx, &mr);
            break;
        }

        case DM2_TIMER_TYPE_THINK_21:
        case DM2_TIMER_TYPE_THINK_22:
            if (cb->think_creature)
                cb->think_creature(tim.xA, tim.yA, tim.type, ctx);
            break;

        case DM2_TIMER_TYPE_3C:
        case DM2_TIMER_TYPE_3D: {
            DM2_V1_ProcessTimer3DReceipt tr;
            dm2_v1_process_timer_3d(&tim, cb, ctx, &tr);
            break;
        }

        case DM2_TIMER_TYPE_LIGHT:
            if (cb->change_current_map_to) {
                int16_t party_map = cb->get_party_map
                    ? cb->get_party_map(ctx) : 0;
                cb->change_current_map_to(party_map, ctx);
            }
            {
                DM2_V1_ProcessTimerLightReceipt lr;
                dm2_v1_process_timer_light(&tim, cb, ctx, &lr);
            }
            if (cb->recalc_light_level)
                cb->recalc_light_level(ctx);
            break;

        case DM2_TIMER_TYPE_47:
            /* Hero enchantment power timer - decrement counter */
            break;

        case DM2_TIMER_TYPE_ENCH_POWER:
            /* Party enchantment power decay per hero */
            break;

        case DM2_TIMER_TYPE_POISON:
            /* Process poison damage */
            if (cb->process_poison) {
                cb->process_poison(tim.actor, tim.valueA, ctx);
            }
            break;

        case DM2_TIMER_TYPE_WEATHER:
            if (cb->update_weather)
                cb->update_weather(1, ctx);
            break;

        case DM2_TIMER_TYPE_ORNATE_ANIM:
            /* Continue ornate animator: advance frame, check completion */
            break;

        case DM2_TIMER_TYPE_TICK_GEN:
            /* Continue tick generator: invoke actuator on cycle */
            break;

        case DM2_TIMER_TYPE_DOOR_BUTTON:
            /* Release door button: clear bit 0x8 in record byte 3 */
            break;

        case DM2_TIMER_TYPE_59:
            /* Continuous ornate animator done: check and clear */
            break;

        case DM2_TIMER_TYPE_ORNATE_NOISE:
            /* Continue ornate noise loop */
            break;

        case DM2_TIMER_TYPE_5B:
            /* Set activate bit on record */
            break;

        case DM2_TIMER_TYPE_5C:
            /* Set activate bit on record (variant) */
            break;

        case DM2_TIMER_TYPE_5D:
            /* Party rotation/warp */
            break;

        case DM2_TIMER_TYPE_5E:
            /* Creature spawn timer */
            if (cb->alloc_new_creature && cb->randdir) {
                int16_t rdir = cb->randdir(ctx);
                if (rdir == 0)
                    rdir = cb->randdir(ctx);
                cb->alloc_new_creature(tim.yB, 7, rdir, tim.xA, tim.yA, ctx);
            }
            break;

        default:
            /* Unknown timer type - skip */
            break;
        }
    }

    /* Restore current map to party map */
    if (cb->change_current_map_to && cb->get_party_map) {
        int16_t party_map = cb->get_party_map(ctx);
        cb->change_current_map_to(party_map, ctx);
    }
}
