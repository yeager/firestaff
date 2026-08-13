/*
 * dm2_v1_proceed_timers_pc34_compat.c — DM2 V1 source-ordered timer
 * dispatcher (DM2-003).
 *
 * Mirrors skproject/SKULLWIN/c_tim_proc.cpp DM2_PROCEED_TIMERS
 * (lines 3980-4230): while DM2_IS_TIMER_TO_PROCEED, pop the heap head via
 * DM2_GET_AND_DELETE_NEXT_TIMER, DM2_CHANGE_CURRENT_MAP_TO the timer map,
 * and dispatch on the type matrix.  Unknown types fall through to the
 * source's `continue`; known types without a bound DM2-owned handler are
 * acknowledged in source order and counted fail-closed — the host never
 * substitutes behaviour for them.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 (DM2_PROCEED_TIMERS)
 *         skproject/SKULLWIN/c_timer.cpp:31-47   (DM2_cmp_timers order)
 *         skproject/SKULLWIN/c_timer.cpp:261-278 (GET_AND_DELETE_NEXT_TIMER)
 *         skproject/SKULLWIN/c_timer.h:8-103     (c_tim 12-byte record)
 *         skproject/SKULLWIN/c_events.cpp,
 *         skproject/SKULLWIN/c_eventqueue.cpp    (event-queue boundary)
 */

#include "dm2_v1_proceed_timers_pc34_compat.h"

#include <string.h>

/* Source timer-type matrix membership, c_tim_proc.cpp:3980-4230.  The
 * source's compare chain admits exactly these type bytes; everything else
 * hits `continue`. */
int dm2_v1_timer_type_is_known(uint8_t type)
{
    switch (type) {
        case DM2_V1_TIMER_STEP_DOOR:
        case DM2_V1_TIMER_DESTROY_DOOR:
        case DM2_V1_TIMER_ACTUATE_TILE:
        case DM2_V1_TIMER_PROCESS_0C:
        case DM2_V1_TIMER_RESURRECTION:
        case DM2_V1_TIMER_PROCESS_0E:
        case DM2_V1_TIMER_PROCESS_SOUND:
        case DM2_V1_TIMER_PROCESS_CLOUD:
        case DM2_V1_TIMER_STEP_MISSILE_ALT:
        case DM2_V1_TIMER_STEP_MISSILE:
        case DM2_V1_TIMER_THINK_CREATURE_A:
        case DM2_V1_TIMER_THINK_CREATURE_B:
        case DM2_V1_TIMER_PROCESS_3C:
        case DM2_V1_TIMER_PROCESS_3D:
        case DM2_V1_TIMER_LIGHT:
        case DM2_V1_TIMER_HERO_ENCH_FLAG:
        case DM2_V1_TIMER_ENCH_POWER:
        case DM2_V1_TIMER_POISON:
        case DM2_V1_TIMER_UPDATE_WEATHER:
        case DM2_V1_TIMER_ORNATE_ANIMATOR:
        case DM2_V1_TIMER_TICK_GENERATOR:
        case DM2_V1_TIMER_RELEASE_DOOR_BUTTON:
        case DM2_V1_TIMER_PROCESS_59:
        case DM2_V1_TIMER_ORNATE_NOISE:
        case DM2_V1_TIMER_5B_RECORD_CLEAR:
        case DM2_V1_TIMER_5C_RECORD_SET:
        case DM2_V1_TIMER_MOVE_RECORD_ROTATE:
        case DM2_V1_TIMER_ALLOC_NEW_CREATURE:
            return 1;
        default:
            return 0;
    }
}

static int dm2_v1_timer_map_of(const DM2_V1_SourceTimer *timer)
{
    /* c_timer.h:64 — getmap() is the high byte of l_00. */
    return (int)((timer->ticks_and_map >> 24) & 0xffu);
}

static void dm2_v1_dispatch_actuate_tile(const DM2_V1_TimerDispatcher *dispatcher,
                                         const DM2_V1_SourceTimer *timer,
                                         uint16_t source_index,
                                         DM2_V1_ProceedTimersReceipt *receipt)
{
    int tile_class = -1;

    /* c_tim_proc.cpp:4214-4230 — the 0x04 timer reads the square class
     * (mapdat.map[x][y] >> 5) and subdispatches 0..6; class 3 is a source
     * fall-through no-op.  Class > 6 throws in the source; fail-closed
     * here instead. */
    if (dispatcher->tile_class_at != NULL) {
        tile_class = dispatcher->tile_class_at(dispatcher->context,
                                               dm2_v1_timer_map_of(timer),
                                               (int)(int8_t)(timer->value_a & 0xff),
                                               (int)(int8_t)((timer->value_a >> 8) & 0xff));
    }
    if (tile_class < 0 || tile_class >= DM2_V1_TIMER_ACTUATOR_TILE_CLASSES) {
        receipt->fail_closed_count++;
        return;
    }
    receipt->actuator_tile_tally[tile_class]++;
    if (tile_class == 3) {
        /* Source case 3: break — acknowledged no-op. */
        receipt->dispatched_count++;
        return;
    }
    if (dispatcher->actuator_tile[tile_class] == NULL) {
        receipt->fail_closed_count++;
        return;
    }
    if (dispatcher->actuator_tile[tile_class](dispatcher->context, timer,
                                              source_index, receipt)) {
        receipt->dispatched_count++;
    } else {
        receipt->handler_rejected_count++;
    }
}

int dm2_v1_proceed_timers(DM2_V1_SourceTimerQueue *queue,
                          uint32_t game_tick,
                          const DM2_V1_TimerDispatcher *dispatcher,
                          DM2_V1_ProceedTimersReceipt *out_receipt)
{
    static const DM2_V1_TimerDispatcher empty_dispatcher;

    if (queue == NULL || out_receipt == NULL) {
        return 0;
    }
    if (dispatcher == NULL) {
        dispatcher = &empty_dispatcher;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->valid = 1;
    out_receipt->game_tick = game_tick;
    out_receipt->last_map = -1;

    /* DM2_PROCEED_TIMERS loop, c_tim_proc.cpp:3991-4007: proceed while a
     * due timer exists; dequeue before changing map or dispatching. */
    while (dm2_v1_source_timer_is_due(queue, game_tick)) {
        DM2_V1_SourceTimer timer;
        uint16_t source_index = 0;
        uint8_t type;

        if (dm2_v1_source_timer_pop_due(queue, game_tick, &timer,
                                        &source_index) !=
            DM2_V1_SOURCE_TIMER_OK) {
            break;
        }
        out_receipt->due_count++;
        out_receipt->map_switches++;
        out_receipt->last_map = dm2_v1_timer_map_of(&timer);

        type = timer.type;
        if (type < DM2_V1_TIMER_TYPE_MATRIX_SIZE) {
            out_receipt->type_tally[type]++;
        }
        if (!dm2_v1_timer_type_is_known(type)) {
            /* Source: unmatched RG4UW falls through to `continue`. */
            out_receipt->skipped_unknown_type++;
            continue;
        }
        if (type == DM2_V1_TIMER_ACTUATE_TILE) {
            dm2_v1_dispatch_actuate_tile(dispatcher, &timer, source_index,
                                         out_receipt);
            continue;
        }
        if (type >= DM2_V1_TIMER_TYPE_MATRIX_SIZE ||
            dispatcher->handlers[type] == NULL) {
            out_receipt->fail_closed_count++;
            continue;
        }
        if (dispatcher->handlers[type](dispatcher->context, &timer,
                                       source_index, out_receipt)) {
            out_receipt->dispatched_count++;
        } else {
            out_receipt->handler_rejected_count++;
        }
    }
    return 1;
}

const char *dm2_v1_proceed_timers_source_evidence(void)
{
    return
        "DM2 V1 source-ordered timer dispatcher — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 (DM2_PROCEED_TIMERS)\n"
        "Source: skproject/SKULLWIN/c_timer.cpp:31-47 (DM2_cmp_timers order)\n"
        "Source: skproject/SKULLWIN/c_timer.cpp:261-278 (GET_AND_DELETE_NEXT_TIMER)\n"
        "Source: skproject/SKULLWIN/c_timer.h:8-103 (c_tim 12-byte record)\n"
        "Source: skproject/SKULLWIN/c_events.cpp (timer-producing events)\n"
        "Source: skproject/SKULLWIN/c_eventqueue.cpp (event queue boundary)\n";
}
