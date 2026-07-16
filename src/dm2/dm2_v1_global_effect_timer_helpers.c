#include "dm2_v1_global_effect_timer_helpers.h"

#include <string.h>

static void dm2_global_timer_begin(
    DM2_V1_GlobalEffectTimerReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_global_effect_timer_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static uint8_t dm2_timer_value_to_effect(int16_t value)
{
    if (value <= 0) {
        return 0u;
    }
    if (value > 255) {
        return 255u;
    }
    return (uint8_t)value;
}

void dm2_v1_global_effect_timer_receipt_clear(
    DM2_V1_GlobalEffectTimerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_PROCESS_TIMER_0E(
    const DM2_V1_SourceTimer *timer,
    uint8_t *global_effects,
    size_t global_effect_count,
    DM2_V1_GlobalEffectTimerReceipt *out_receipt)
{
    uint8_t effect_after;

    dm2_global_timer_begin(out_receipt,
                           "PROCESS_TIMER_0E",
                           "SKWIN/SkWinCore.cpp:2173");
    if (!timer || !global_effects ||
        timer->type != DM2_V1_TIMER_TYPE_GLOBAL_EFFECT_0E ||
        timer->actor >= global_effect_count ||
        global_effect_count > 64u) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            if (timer) {
                out_receipt->timer_type = timer->type;
                out_receipt->effect_index = timer->actor;
            }
        }
        return 0;
    }
    effect_after = dm2_timer_value_to_effect(timer->value_a);
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->mutated = global_effects[timer->actor] != effect_after;
        out_receipt->timer_type = timer->type;
        out_receipt->effect_index = timer->actor;
        out_receipt->effect_before = global_effects[timer->actor];
        out_receipt->effect_after = effect_after;
    }
    global_effects[timer->actor] = effect_after;
    return 1;
}

int dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    uint8_t *global_effects,
    size_t global_effect_count,
    DM2_V1_GlobalEffectTimerReceipt *out_receipt)
{
    uint32_t dispatched = 0u;

    dm2_global_timer_begin(out_receipt,
                           "PROCEED_GLOBAL_EFFECT_TIMERS",
                           "SKWIN/SkWinCore.cpp:2426");
    if (!queue || !global_effects || global_effect_count > 64u) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    while (dm2_v1_source_timer_is_due(queue, game_tick)) {
        DM2_V1_SourceTimer timer;
        uint16_t source_index;
        DM2_V1_GlobalEffectTimerReceipt timer_receipt;

        if (dm2_v1_source_timer_pop_due(queue, game_tick, &timer,
                                        &source_index) !=
            DM2_V1_SOURCE_TIMER_OK) {
            if (out_receipt) {
                out_receipt->blocked = 1;
            }
            return 0;
        }
        (void)source_index;
        if (!dm2_v1_PROCESS_TIMER_0E(&timer, global_effects,
                                     global_effect_count, &timer_receipt)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->timer_type = timer.type;
                out_receipt->effect_index = timer.actor;
                out_receipt->game_tick = game_tick;
                out_receipt->dispatched_count = dispatched;
                out_receipt->remaining_timer_count = (uint32_t)queue->count;
            }
            return 0;
        }
        ++dispatched;
        if (out_receipt) {
            out_receipt->mutated |= timer_receipt.mutated;
            out_receipt->effect_index = timer_receipt.effect_index;
            out_receipt->effect_before = timer_receipt.effect_before;
            out_receipt->effect_after = timer_receipt.effect_after;
        }
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->game_tick = game_tick;
        out_receipt->timer_type = DM2_V1_TIMER_TYPE_GLOBAL_EFFECT_0E;
        out_receipt->dispatched_count = dispatched;
        out_receipt->remaining_timer_count = (uint32_t)queue->count;
    }
    return 1;
}

const char *dm2_v1_global_effect_timer_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp PROCESS_TIMER_0E:2173 "
           "PROCEED_GLOBAL_EFFECT_TIMERS:2426; bounded global-effect timer "
           "receipts over source timer queue entries.";
}
