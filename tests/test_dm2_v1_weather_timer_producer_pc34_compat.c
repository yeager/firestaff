/* test_dm2_v1_weather_timer_producer_pc34_compat.c — DM2-003 follow-up:
 * weather timer producer bound to the DM2-owned source queue.
 *
 * skproject/SKULLWIN/c_weather.cpp:20-30 DM2_SET_TIMER_WEATHER queues a
 * type-0x54 c_tim (actor 0, mticks = gametick + delay).  This test
 * verifies the runtime now routes that producer through
 * dm2_v1_runtime_enqueue_source_timer / dm2_v1_proceed_timers instead of
 * keeping it purely host-side:
 *
 *  1. Outdoor first tick enqueues a pending 0x54 source timer.
 *  2. Before the 182-tick boundary no 0x54 timer is due.
 *  3. At the 182-tick boundary the dispatcher pops the 0x54 timer
 *     (acknowledged fail-closed until DM2_UPDATE_WEATHER is bound).
 *  4. The producer re-schedules the next cycle after the pop.
 *  5. Indoor sessions never enqueue a 0x54 timer.
 *  6. The host weather transition path still owns the seed transition
 *     at the boundary (behaviour unchanged).
 */

#include "dm2_v1_runtime.h"
#include "dm2_v1_weather.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

static void test_outdoor_producer_enqueues_source_timer(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ProceedTimersReceipt receipt = {0};

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(1);

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "outdoor first tick enqueues a pending 0x54 source timer");

    for (int tick = 0; tick < 180; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "0x54 timer stays pending before the 182-tick boundary");
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1,
          "dispatcher receipt is published every tick");
    CHECK(receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] == 0,
          "no 0x54 timer is due before the boundary");

    dm2_v1_runtime_tick();  /* 182nd tick: boundary */
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1,
          "dispatcher receipt is published at the boundary");
    CHECK(receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] >= 1,
          "dispatcher pops the 0x54 weather timer at the boundary");

    dm2_v1_runtime_tick();  /* producer re-schedules the next cycle */
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "producer re-schedules the next 182-tick cycle after the pop");

    dm2_v1_runtime_set_outdoor(0);
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 0,
          "switching indoor clears the pending weather timer");
}

static void test_indoor_never_enqueues_weather_timer(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ProceedTimersReceipt receipt = {0};
    int saw_weather_timer = 0;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
        if (dm2_v1_runtime_weather_source_timer_pending() != 0) {
            saw_weather_timer = 1;
        }
        if (dm2_v1_runtime_last_proceed_timers_receipt(&receipt) &&
            receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] != 0) {
            saw_weather_timer = 1;
        }
    }
    CHECK(!saw_weather_timer,
          "indoor session never enqueues or pops a 0x54 timer");
}

static void test_host_transition_path_unchanged(void)
{
    DM2_V1_BootProfile boot = {0};
    const uint32_t seed = 0x2D2Du;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    for (int tick = 0; tick < 181; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_get_weather_seed() == seed,
          "seed unchanged before the boundary");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_weather_seed() ==
          dm2_v1_weather_advance_seed(seed),
          "host transition path still advances the seed at the boundary");
}

int main(void)
{
    printf("DM2 V1 weather timer producer source-queue binding\n");
    printf("Source: skproject/SKULLWIN/c_weather.cpp:20-30 "
           "(DM2_SET_TIMER_WEATHER)\n");
    printf("        skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 "
           "(DM2_PROCEED_TIMERS)\n\n");

    test_outdoor_producer_enqueues_source_timer();
    test_indoor_never_enqueues_weather_timer();
    test_host_transition_path_unchanged();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
