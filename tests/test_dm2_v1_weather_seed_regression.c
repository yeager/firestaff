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
    DM2_V1_WeatherTimerReceipt receipt;
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

    dm2_v1_weather_init(&state);
    dm2_v1_weather_set_seed(&state, seed);
    CHECK(dm2_v1_weather_set_timer_weather(&state, 0, 182u, &receipt) == 0,
          "DM2_SET_TIMER_WEATHER does not transition indoors");
    CHECK(receipt.valid && receipt.source_set_timer_weather &&
          !receipt.source_weather_3df7_0037 && !receipt.due &&
          receipt.seed_before == seed && receipt.seed_after == seed,
          "indoor timer receipt records no weather mutation");

    CHECK(dm2_v1_weather_set_timer_weather(&state, 1, 181u, &receipt) == 0,
          "DM2_SET_TIMER_WEATHER waits for the 182-tick boundary");
    CHECK(receipt.valid && receipt.outdoor && !receipt.due &&
          receipt.seed_before == seed && receipt.seed_after == seed,
          "not-due outdoor receipt preserves seed");

    CHECK(dm2_v1_weather_set_timer_weather(&state, 1, 182u, &receipt) == 1,
          "DM2_SET_TIMER_WEATHER dispatches weather_3df7_0037 when due");
    CHECK(receipt.valid && receipt.source_set_timer_weather &&
          receipt.source_weather_3df7_0037 && receipt.due &&
          receipt.interval_ticks == DM2_WEATHER_TIMER_INTERVAL_TICKS &&
          receipt.seed_before == seed && receipt.seed_after == s1 &&
          receipt.weather_after == (uint8_t)w1 &&
          receipt.transaction_hash != 0u,
          "due timer receipt binds source symbols and seed transition");
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
        DM2_V1_WeatherTimerReceipt receipt;
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "indoor mode never advances weather seed");
        CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_CLEAR,
              "indoor mode keeps clear weather");
        CHECK(dm2_v1_runtime_last_weather_timer_receipt(&receipt) == 1 &&
              receipt.valid && !receipt.outdoor && !receipt.due,
              "indoor runtime publishes no-transition weather receipt");
    }

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_weather_seed(seed);
    dm2_v1_runtime_set_outdoor(1);

    for (int tick = 0; tick < 181; tick++) {
        DM2_V1_WeatherTimerReceipt receipt;
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_weather_seed() == seed,
              "outdoor mode keeps seed before 182nd tick");
        CHECK(dm2_v1_runtime_last_weather_timer_receipt(&receipt) == 1 &&
              receipt.valid && receipt.outdoor && !receipt.due,
              "outdoor pre-boundary receipt is valid but not due");
    }

    dm2_v1_runtime_tick();
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
