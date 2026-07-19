/* test_dm2_v1_weather_timer_producer_pc34_compat.c — DM2-003 follow-up:
 * the source 0x54 weather chain wired into the runtime.
 *
 * skproject has no fixed weather interval: DM2_weather_3df7_0037
 * (c_weather.cpp:509-567) queues the next type-0x54 c_tim with
 * RAND16(8000)+500 (or RAND16(500) when storm-forced), and each
 * DM2_UPDATE_WEATHER(1) pop re-queues RAND16(256)+50
 * (c_weather.cpp:87-89).  This test verifies the runtime now runs that
 * self-perpetuating chain instead of the retired synthetic 182-tick
 * cadence:
 *
 *  1. Outdoor first tick starts the chain: the session transition
 *     (arg=0) queues a 0x54 timer with the source delay.
 *  2. No 0x54 pop happens before the source delay boundary.
 *  3. At the boundary the bound 0x54 handler steps the v1e14xx chain
 *     state (retry/intensity from v1d7108) and re-queues.
 *  4. The chain keeps running across pops (self-perpetuating).
 *  5. Indoor sessions never start the chain; leaving outdoor stops it.
 *
 * Source: skproject/SKULLWIN/c_weather.cpp:20-30  (DM2_SET_TIMER_WEATHER)
 *         skproject/SKULLWIN/c_weather.cpp:33-90  (DM2_UPDATE_WEATHER(1))
 *         skproject/SKULLWIN/c_weather.cpp:509-567 (DM2_weather_3df7_0037)
 *         skproject/SKULLWIN/c_savegame.cpp:546   (session-start call)
 */

#include "dm2_v1_runtime.h"
#include "dm2_v1_update_weather_pc34_compat.h"
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

/* Reference copy of the source LCG (c_random.cpp:13-31) for deriving the
 * expected chain delays/state. */
static uint32_t ref_rand(uint32_t *state)
{
    *state = *state * 0xbb40e62du + 11u;
    return *state >> 8;
}

static int ref_rand16(uint32_t *state, uint32_t n)
{
    return (int)((ref_rand(state) & 0xffffu) % n);
}

static void test_outdoor_chain_starts_and_requeues(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ProceedTimersReceipt receipt = {0};
    DM2_V1_UpdateWeatherState snap;
    const uint32_t seed = 0x2D2Du;
    uint32_t rs = seed;
    int delay0, row, step, delay1;
    int intensity1, intensity2;

    /* Expected chain: transition draws RAND16(8000) -> RANDDIR ->
     * RAND16(3) -> RANDDIR -> RAND16(4); the first pop then draws
     * RAND16(256) for the requeue. */
    delay0 = ref_rand16(&rs, 8000) + 500;
    row = (int)(ref_rand(&rs) & 0x3u);
    step = ref_rand16(&rs, 3) + 1;
    (void)ref_rand(&rs);            /* wind_dir RANDDIR */
    (void)ref_rand16(&rs, 4);       /* cloud_timer RAND16(4)+4 */
    delay1 = ref_rand16(&rs, 256) + 50;

    intensity1 = step * (int)dm2_v1_update_weather_pattern[(row << 5) + 1];
    if (intensity1 < 0) intensity1 = 0;
    if (intensity1 > 0xff) intensity1 = 0xff;
    intensity2 = intensity1 +
        step * (int)dm2_v1_update_weather_pattern[(row << 5) + 2];
    if (intensity2 < 0) intensity2 = 0;
    if (intensity2 > 0xff) intensity2 = 0xff;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    dm2_v1_runtime_tick();  /* tick 1: chain start */
    CHECK(dm2_v1_runtime_weather_chain_started() == 1,
          "outdoor first tick starts the 0x54 weather chain");
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "chain start leaves a pending 0x54 source timer");
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&snap) == 1 &&
              snap.pattern_row == (int8_t)row && snap.step == (int8_t)step &&
              snap.retry == 0 && snap.intensity == 0,
          "chain state reseeded by the session transition");

    for (int tick = 0; tick < delay0 - 1; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1 &&
              receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] == 0,
          "no 0x54 pop before the source delay boundary");

    dm2_v1_runtime_tick();  /* tick 1 + delay0: first pop */
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1 &&
              receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] >= 1,
          "dispatcher pops the 0x54 timer at the source boundary");
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&snap) == 1 &&
              snap.retry == 1 && snap.intensity == (int16_t)intensity1,
          "handler steps retry/intensity from v1d7108 at the pop");
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "handler re-queues the next 0x54 timer (self-perpetuating)");

    for (int tick = 0; tick < delay1 - 1; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1 &&
              receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] == 0,
          "no 0x54 pop before the RAND16(256)+50 requeue boundary");

    dm2_v1_runtime_tick();  /* second pop */
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1 &&
              receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] >= 1,
          "dispatcher pops the re-queued 0x54 timer");
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&snap) == 1 &&
              snap.retry == 2 && snap.intensity == (int16_t)intensity2,
          "second pop steps the chain again in source order");

    dm2_v1_runtime_set_outdoor(0);
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_weather_chain_started() == 0 &&
              dm2_v1_runtime_weather_source_timer_pending() == 0,
          "switching indoor stops the weather chain");
}

static void test_indoor_never_starts_weather_chain(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ProceedTimersReceipt receipt = {0};
    int saw_weather_timer = 0;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
        if (dm2_v1_runtime_weather_chain_started() != 0 ||
            dm2_v1_runtime_weather_source_timer_pending() != 0) {
            saw_weather_timer = 1;
        }
        if (dm2_v1_runtime_last_proceed_timers_receipt(&receipt) &&
            receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] != 0) {
            saw_weather_timer = 1;
        }
    }
    CHECK(!saw_weather_timer,
          "indoor session never starts or pops the 0x54 chain");
}

static void test_chain_restart_on_outdoor_reentry(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_UpdateWeatherState first;
    DM2_V1_UpdateWeatherState second;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(0x1234u);
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&first) == 1,
          "chain runs after first outdoor entry");

    dm2_v1_runtime_set_outdoor(0);
    dm2_v1_runtime_tick();
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&second) == 1,
          "chain restarts after outdoor re-entry");
    CHECK(first.pattern_row == second.pattern_row &&
              first.step == second.step &&
              first.cloud_timer == second.cloud_timer,
          "restart re-seeds deterministically from the session seed");
}

int main(void)
{
    printf("DM2 V1 weather 0x54 chain runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_weather.cpp:20-30 "
           "(DM2_SET_TIMER_WEATHER)\n");
    printf("        skproject/SKULLWIN/c_weather.cpp:33-90 "
           "(DM2_UPDATE_WEATHER(1))\n");
    printf("        skproject/SKULLWIN/c_weather.cpp:509-567 "
           "(DM2_weather_3df7_0037)\n\n");

    test_outdoor_chain_starts_and_requeues();
    test_indoor_never_starts_weather_chain();
    test_chain_restart_on_outdoor_reentry();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
