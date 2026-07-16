/* DM2 V1 Weather Seed Regression
 *
 * Verifies a narrow deterministic state transition:
 * - LCG weather seed advance is deterministic and consistent with API.
 * - Outdoor weather transition happens at the 182-tick boundary.
 * - No transition occurs when not in outdoor mode.
 *
 * No game data required; test uses synthetic state only.
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

static void test_weather_ticks_only_change_outdoor_weather_at_boundary(void)
{
    DM2_V1_BootProfile boot = {0};
    const uint32_t seed = 0x2D2Du;
    const uint32_t expected_seed_after_182 = dm2_v1_weather_advance_seed(seed);
    const int expected_weather_after_182 = (int)((expected_seed_after_182 >> 8) & 0x3u);

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(0);

    for (int tick = 0; tick < 182; tick++) {
        DM2_V1_SetTimerWeatherReceipt timer = {0};
        DM2_V1_Weather3df70037Receipt weather = {0};
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "indoor mode never advances weather seed");
        CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_CLEAR,
              "indoor mode keeps clear weather");
        CHECK(dm2_v1_runtime_last_set_timer_weather_receipt(&timer),
              "runtime publishes DM2_SET_TIMER_WEATHER receipt indoors");
        CHECK(!timer.due_now && !timer.scheduled,
              "runtime indoor timer receipt never schedules weather");
        CHECK(!dm2_v1_runtime_last_weather_3df7_0037_receipt(&weather),
              "runtime indoor path has no weather transition receipt");
    }

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    for (int tick = 0; tick < 181; tick++) {
        DM2_V1_SetTimerWeatherReceipt timer = {0};
        DM2_V1_Weather3df70037Receipt weather = {0};
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "outdoor mode keeps seed before 182nd tick");
        CHECK(dm2_v1_runtime_last_set_timer_weather_receipt(&timer),
              "runtime publishes outdoor timer receipt before boundary");
        CHECK(!timer.due_now && timer.scheduled,
              "runtime outdoor timer receipt is scheduled but not due");
        CHECK(!dm2_v1_runtime_last_weather_3df7_0037_receipt(&weather),
              "runtime outdoor not-due path has no transition receipt");
    }

    dm2_v1_runtime_tick();
    {
        DM2_V1_SetTimerWeatherReceipt timer = {0};
        DM2_V1_Weather3df70037Receipt weather = {0};
        CHECK(dm2_v1_runtime_last_set_timer_weather_receipt(&timer),
              "runtime publishes outdoor due timer receipt");
        CHECK(timer.due_now && timer.scheduled && timer.current_tick == 182u,
              "runtime due timer receipt binds tick 182");
        CHECK(dm2_v1_runtime_last_weather_3df7_0037_receipt(&weather),
              "runtime publishes DM2_weather_3df7_0037 receipt at boundary");
        CHECK(weather.source_receipt_hash == timer.receipt_hash,
              "runtime weather transition binds timer receipt");
        CHECK(weather.previous_seed == seed &&
              weather.next_seed == expected_seed_after_182,
              "runtime weather transition records seed transaction");
    }
    CHECK(dm2_v1_runtime_get_weather_seed() == expected_seed_after_182,
          "outdoor mode advances weather seed at tick 182");
    CHECK(dm2_v1_runtime_get_weather() == expected_weather_after_182,
          "outdoor mode weather at tick 182 is seeded result");
    {
        DM2_V1_WeatherTimerReceipt receipt;
        CHECK(dm2_v1_runtime_last_weather_timer_receipt(&receipt) == 1 &&
              receipt.valid && receipt.outdoor && receipt.due &&
              receipt.source_set_timer_weather &&
              receipt.source_weather_3df7_0037 &&
              receipt.tick_count == 182u &&
              receipt.seed_before == seed &&
              receipt.seed_after == expected_seed_after_182 &&
              receipt.weather_after == (uint8_t)expected_weather_after_182,
              "runtime receipt owns the due weather transition");
    }
}

int main(void)
{
    printf("=== DM2 V1 Weather Seed Regression Test ===\n\n");

    printf("--- test_weather_seed_advances_with_reproducible_lcg ---\n");
    test_weather_seed_advances_with_reproducible_lcg();

    printf("\n--- test_weather_ticks_only_change_outdoor_weather_at_boundary ---\n");
    test_weather_ticks_only_change_outdoor_weather_at_boundary();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
