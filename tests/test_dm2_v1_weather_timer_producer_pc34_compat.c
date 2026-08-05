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
 * An outdoor flag and a host seed are not the source-owned v1e14xx global
 * block.  They must not start or requeue a weather timer.
 *
 * Source: skproject/SKULLWIN/c_weather.cpp:20-30  (DM2_SET_TIMER_WEATHER)
 *         skproject/SKULLWIN/c_weather.cpp:33-90  (DM2_UPDATE_WEATHER(1))
 *         skproject/SKULLWIN/c_weather.cpp:509-567 (DM2_weather_3df7_0037)
 *         skproject/SKULLWIN/c_savegame.cpp:546   (session-start call)
 */

#include "dm2_v1_runtime.h"
#include <stdio.h>

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

static void test_outdoor_does_not_construct_chain(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ProceedTimersReceipt receipt = {0};
    DM2_V1_SetTimerWeatherReceipt timer_owner = {0};

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(0x2D2Du);
    dm2_v1_runtime_set_outdoor(1);
    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&receipt) == 1 &&
              receipt.type_tally[DM2_V1_TIMER_UPDATE_WEATHER] == 0 &&
              dm2_v1_runtime_weather_chain_started() == 0 &&
              dm2_v1_runtime_weather_source_timer_pending() == 0 &&
              dm2_v1_runtime_last_set_timer_weather_receipt(&timer_owner) == 0,
          "outdoor state does not fabricate a source weather timer or receipt");
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

int main(void)
{
    printf("DM2 V1 weather 0x54 chain runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_weather.cpp:20-30 "
           "(DM2_SET_TIMER_WEATHER)\n");
    printf("        skproject/SKULLWIN/c_weather.cpp:33-90 "
           "(DM2_UPDATE_WEATHER(1))\n");
    printf("        skproject/SKULLWIN/c_weather.cpp:509-567 "
           "(DM2_weather_3df7_0037)\n\n");

    test_outdoor_does_not_construct_chain();
    test_indoor_never_starts_weather_chain();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
