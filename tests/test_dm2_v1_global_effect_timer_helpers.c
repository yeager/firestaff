#include "dm2_v1_global_effect_timer_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static DM2_V1_SourceTimer timer0e(uint32_t tick,
                                  uint8_t actor,
                                  int16_t value)
{
    DM2_V1_SourceTimer timer;

    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = tick;
    timer.type = DM2_V1_TIMER_TYPE_GLOBAL_EFFECT_0E;
    timer.actor = actor;
    timer.value_a = value;
    return timer;
}

static void test_process_timer_0e(void)
{
    uint8_t effects[4] = {0u, 9u, 0u, 0u};
    DM2_V1_SourceTimer timer = timer0e(12u, 1u, 37);
    DM2_V1_GlobalEffectTimerReceipt receipt;

    expect_true(dm2_v1_PROCESS_TIMER_0E(&timer, effects, 4u,
                                        &receipt) == 1,
                "PROCESS_TIMER_0E admits global effect timer");
    expect_true(effects[1] == 37u && receipt.valid && receipt.mutated &&
                    receipt.effect_before == 9u &&
                    receipt.effect_after == 37u &&
                    strcmp(receipt.symbol, "PROCESS_TIMER_0E") == 0,
                "PROCESS_TIMER_0E records effect mutation");

    timer.value_a = -4;
    expect_true(dm2_v1_PROCESS_TIMER_0E(&timer, effects, 4u,
                                        &receipt) == 1,
                "PROCESS_TIMER_0E clears non-positive effect value");
    expect_true(effects[1] == 0u && receipt.effect_after == 0u,
                "PROCESS_TIMER_0E clamps negative value to clear");

    timer.actor = 9u;
    expect_true(dm2_v1_PROCESS_TIMER_0E(&timer, effects, 4u,
                                        &receipt) == 0,
                "PROCESS_TIMER_0E blocks out-of-range effect slot");
    expect_true(receipt.blocked && !receipt.valid,
                "out-of-range timer 0E is fail-closed");
}

static void test_proceed_global_effect_timers(void)
{
    uint8_t effects[4] = {0u, 0u, 0u, 0u};
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer timer_a = timer0e(10u, 2u, 41);
    DM2_V1_SourceTimer timer_b = timer0e(25u, 1u, 12);
    DM2_V1_GlobalEffectTimerReceipt receipt;

    dm2_v1_source_timer_queue_init(&queue);
    expect_true(dm2_v1_source_timer_enqueue(&queue, &timer_b, 2u) ==
                    DM2_V1_SOURCE_TIMER_OK,
                "future timer enqueues");
    expect_true(dm2_v1_source_timer_enqueue(&queue, &timer_a, 1u) ==
                    DM2_V1_SOURCE_TIMER_OK,
                "due timer enqueues before future timer");

    expect_true(dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(&queue, 10u, effects,
                                                    4u, &receipt) == 1,
                "PROCEED_GLOBAL_EFFECT_TIMERS dispatches due 0E timer");
    expect_true(effects[2] == 41u && effects[1] == 0u &&
                    queue.count == 1u && receipt.valid &&
                    receipt.dispatched_count == 1u &&
                    receipt.remaining_timer_count == 1u &&
                    strcmp(receipt.symbol,
                           "PROCEED_GLOBAL_EFFECT_TIMERS") == 0,
                "global effect proceed leaves future timer queued");

    expect_true(dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(&queue, 24u, effects,
                                                    4u, &receipt) == 1,
                "PROCEED_GLOBAL_EFFECT_TIMERS accepts no due timers");
    expect_true(receipt.dispatched_count == 0u && queue.count == 1u,
                "no due global effect timer does not mutate queue");

    expect_true(dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(&queue, 25u, effects,
                                                    4u, &receipt) == 1,
                "PROCEED_GLOBAL_EFFECT_TIMERS dispatches future timer later");
    expect_true(effects[1] == 12u && queue.count == 0u &&
                    receipt.dispatched_count == 1u,
                "future global effect timer dispatches at due tick");
}

static void test_proceed_blocks_unknown_due_timer(void)
{
    uint8_t effects[2] = {0u, 0u};
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer timer = timer0e(5u, 0u, 1);
    DM2_V1_GlobalEffectTimerReceipt receipt;

    timer.type = 0x33u;
    dm2_v1_source_timer_queue_init(&queue);
    expect_true(dm2_v1_source_timer_enqueue(&queue, &timer, 0u) ==
                    DM2_V1_SOURCE_TIMER_OK,
                "unknown nonzero timer enqueues");
    expect_true(dm2_v1_PROCEED_GLOBAL_EFFECT_TIMERS(&queue, 5u, effects,
                                                    2u, &receipt) == 0,
                "PROCEED_GLOBAL_EFFECT_TIMERS blocks unknown due timer");
    expect_true(receipt.blocked && !receipt.valid &&
                    receipt.timer_type == 0x33u,
                "unknown due timer is fail-closed");
}

int main(void)
{
    test_process_timer_0e();
    test_proceed_global_effect_timers();
    test_proceed_blocks_unknown_due_timer();
    expect_true(strstr(dm2_v1_global_effect_timer_helpers_source_evidence(),
                       "PROCESS_TIMER_0E:2173") != 0,
                "source evidence includes PROCESS_TIMER_0E");
    expect_true(strstr(dm2_v1_global_effect_timer_helpers_source_evidence(),
                       "PROCEED_GLOBAL_EFFECT_TIMERS:2426") != 0,
                "source evidence includes global effect timers");
    if (failures) {
        return 1;
    }
    puts("DM2 global effect timer helpers: ok");
    return 0;
}
