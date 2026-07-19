/* DM2 V1 Weather Seed Regression
 *
 * Verifies a narrow deterministic state transition:
 * - LCG weather seed advance is deterministic and consistent with API
 *   (module-level receipt functions).
 * - The runtime runs the source 0x54 weather chain
 *   (skproject/SKULLWIN/c_weather.cpp DM2_weather_3df7_0037 +
 *   DM2_UPDATE_WEATHER(1)) instead of the retired synthetic 182-tick
 *   cadence; the session seed is only read at chain start.
 *
 * No game data required; test uses synthetic state only.
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

/* Reference copy of the source LCG (c_random.cpp:13-31). */
static uint32_t ref_rand(uint32_t *state)
{
    *state = *state * 0xbb40e62du + 11u;
    return *state >> 8;
}

static void test_weather_seed_advances_with_reproducible_lcg(void)
{
    DM2_V1_WeatherState state = {0};
    DM2_V1_SetTimerWeatherReceipt timer = {0};
    DM2_V1_Weather3df70037Receipt receipt = {0};
    uint32_t seed = 0x0100u;
    uint32_t s1;
    uint32_t s2;
    int w1;
    int w2;

    dm2_v1_weather_init(&state);
    dm2_v1_weather_set_seed(&state, seed);
    CHECK(state.weather_seed == seed, "seed set on weather state");

    s1 = dm2_v1_weather_advance_seed(seed);
    w1 = (int)((s1 >> 8) & 0x3u);

    CHECK(dm2_v1_weather_next_state(&state) == w1,
          "first transition returns expected weather");
    CHECK(state.weather_seed == s1, "first transition stores advanced seed");
    CHECK(state.weather == w1, "first transition weather is seed-derived");

    s2 = dm2_v1_weather_advance_seed(s1);
    w2 = (int)((s2 >> 8) & 0x3u);

    CHECK(dm2_v1_weather_next_state(&state) == w2,
          "second transition returns expected weather");
    CHECK(state.weather_seed == s2, "second transition stores advanced seed");

    dm2_v1_weather_set_seed(&state, seed);
    dm2_v1_weather_set(&state, DM2_WEATHER_CLEAR);
    CHECK(dm2_v1_weather_set_timer_weather_receipt(0, 182u, &timer),
          "DM2_SET_TIMER_WEATHER receipt is valid indoors");
    CHECK(!timer.due_now && !timer.scheduled && timer.next_tick == 0u,
          "DM2_SET_TIMER_WEATHER does not schedule indoor weather");
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, &timer, &receipt),
          "DM2_weather_3df7_0037 rejects indoor timer");
    CHECK(state.weather_seed == seed && state.weather == DM2_WEATHER_CLEAR,
          "indoor rejected transition leaves state unchanged");

    CHECK(dm2_v1_weather_set_timer_weather_receipt(1, 181u, &timer),
          "DM2_SET_TIMER_WEATHER receipt is valid before boundary");
    CHECK(!timer.due_now && timer.scheduled && timer.next_tick == 182u,
          "DM2_SET_TIMER_WEATHER schedules next 182-tick boundary");
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, &timer, &receipt),
          "DM2_weather_3df7_0037 rejects not-due timer");
    CHECK(state.weather_seed == seed && state.weather == DM2_WEATHER_CLEAR,
          "not-due rejected transition leaves state unchanged");

    CHECK(dm2_v1_weather_set_timer_weather_receipt(1, 182u, &timer),
          "DM2_SET_TIMER_WEATHER receipt is valid at boundary");
    CHECK(timer.due_now && timer.scheduled && timer.next_tick == 364u,
          "DM2_SET_TIMER_WEATHER marks due boundary and next schedule");
    CHECK(dm2_v1_weather_3df7_0037_receipt(&state, &timer, &receipt),
          "DM2_weather_3df7_0037 accepts due outdoor timer");
    CHECK(receipt.valid && receipt.transitioned,
          "DM2_weather_3df7_0037 publishes transition receipt");
    CHECK(receipt.previous_seed == seed &&
          receipt.next_seed == dm2_v1_weather_advance_seed(seed),
          "DM2_weather_3df7_0037 records seed transaction");
    CHECK(receipt.source_receipt_hash == timer.receipt_hash,
          "DM2_weather_3df7_0037 binds timer receipt hash");
}

static void test_weather_ticks_run_the_source_0x54_chain(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_UpdateWeatherState snap;
    const uint32_t seed = 0x2D2Du;
    uint32_t rs = seed;
    int delay0, row, step, intensity1;

    /* Expected chain-start transition (c_weather.cpp:518-567 draw
     * order): RAND16(8000) -> RANDDIR -> RAND16(3) -> RANDDIR ->
     * RAND16(4). */
    delay0 = (int)((ref_rand(&rs) & 0xffffu) % 8000u) + 500;
    row = (int)(ref_rand(&rs) & 0x3u);
    step = (int)((ref_rand(&rs) & 0xffffu) % 3u) + 1;
    intensity1 = step * (int)dm2_v1_update_weather_pattern[(row << 5) + 1];
    if (intensity1 < 0) intensity1 = 0;
    if (intensity1 > 0xff) intensity1 = 0xff;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(0);

    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "indoor mode never advances weather seed");
        CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_CLEAR,
              "indoor mode keeps clear weather");
        CHECK(dm2_v1_runtime_weather_chain_started() == 0,
              "indoor mode never starts the 0x54 weather chain");
    }

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    dm2_v1_runtime_tick();  /* tick 1: chain start */
    CHECK(dm2_v1_runtime_weather_chain_started() == 1,
          "outdoor first tick starts the source 0x54 chain");
    CHECK(dm2_v1_runtime_get_weather_seed() == seed,
          "session seed is only read, never advanced by the chain");
    CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_CLEAR,
          "weather enum stays a host presentation selector");

    for (int tick = 0; tick < delay0 - 1; tick++) {
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "seed unchanged before the source boundary");
    }

    dm2_v1_runtime_tick();  /* tick 1 + delay0: first pop */
    CHECK(dm2_v1_runtime_weather_chain_snapshot(&snap) == 1 &&
              snap.retry == 1 && snap.intensity == (int16_t)intensity1 &&
              snap.pattern_row == (int8_t)row && snap.step == (int8_t)step,
          "first pop steps the v1e14xx chain in source order");
    CHECK(dm2_v1_runtime_weather_source_timer_pending() == 1,
          "chain re-queues RAND16(256)+50 after the pop");
}

int main(void)
{
    printf("=== DM2 V1 Weather Seed Regression Test ===\n\n");

    printf("--- test_weather_seed_advances_with_reproducible_lcg ---\n");
    test_weather_seed_advances_with_reproducible_lcg();

    printf("\n--- test_weather_ticks_run_the_source_0x54_chain ---\n");
    test_weather_ticks_run_the_source_0x54_chain();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
