/* DM2 V1 Weather Seed Regression
 *
 * Verifies a narrow deterministic state transition:
 * - LCG weather seed advance is deterministic and consistent with API
 *   (module-level receipt functions).
 * - The runtime refuses to create the source 0x54 weather chain from only
 *   an outdoor flag and a host-provided seed.
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

static void test_weather_ticks_require_source_chain_state(void)
{
    DM2_V1_BootProfile boot = {0};
    const uint32_t seed = 0x2D2Du;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(0);

    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "indoor mode never advances weather seed");
        CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_UNKNOWN,
              "indoor mode keeps weather unavailable");
        CHECK(dm2_v1_runtime_weather_chain_started() == 0,
              "indoor mode never starts the 0x54 weather chain");
    }

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    for (int tick = 0; tick < 200; tick++) {
        dm2_v1_runtime_tick();
    }
    CHECK(dm2_v1_runtime_weather_chain_started() == 0 &&
              dm2_v1_runtime_weather_source_timer_pending() == 0 &&
              dm2_v1_runtime_get_weather_seed() == seed &&
              dm2_v1_runtime_get_weather() == DM2_WEATHER_UNKNOWN,
          "outdoor flag and host seed cannot construct a weather chain");
}

int main(void)
{
    printf("=== DM2 V1 Weather Seed Regression Test ===\n\n");

    printf("--- test_weather_seed_advances_with_reproducible_lcg ---\n");
    test_weather_seed_advances_with_reproducible_lcg();

    printf("\n--- test_weather_ticks_require_source_chain_state ---\n");
    test_weather_ticks_require_source_chain_state();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
