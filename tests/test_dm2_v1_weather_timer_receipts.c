#include "dm2_v1_weather.h"

#include <stdio.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    DM2_V1_WeatherState state;
    DM2_V1_SetTimerWeatherReceipt timer;
    DM2_V1_Weather3df70037Receipt weather;
    const uint32_t seed = 0x2d2du;
    const uint32_t next_seed = dm2_v1_weather_advance_seed(seed);
    const int next_weather = (int)((next_seed >> 8) & 0x3u);

    dm2_v1_weather_init(&state);
    dm2_v1_weather_set_seed(&state, seed);
    /* A timer receipt is only meaningful for an already source-restored
     * weather chain; startup itself intentionally has no selector. */
    dm2_v1_weather_set(&state, DM2_WEATHER_CLEAR);

    CHECK(dm2_v1_weather_set_timer_weather_receipt(0, 182u, &timer));
    CHECK(timer.valid);
    CHECK(!timer.outdoor);
    CHECK(!timer.scheduled);
    CHECK(!timer.due_now);
    CHECK(timer.next_tick == 0u);
    CHECK(timer.receipt_hash != 0u);
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, &timer, &weather));
    CHECK(state.weather_seed == seed);
    CHECK(state.weather == DM2_WEATHER_CLEAR);

    CHECK(dm2_v1_weather_set_timer_weather_receipt(1, 181u, &timer));
    CHECK(timer.valid);
    CHECK(timer.outdoor);
    CHECK(timer.scheduled);
    CHECK(!timer.due_now);
    CHECK(timer.next_tick == 182u);
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, &timer, &weather));
    CHECK(state.weather_seed == seed);
    CHECK(state.weather == DM2_WEATHER_CLEAR);

    CHECK(dm2_v1_weather_set_timer_weather_receipt(1, 182u, &timer));
    CHECK(timer.valid);
    CHECK(timer.outdoor);
    CHECK(timer.scheduled);
    CHECK(timer.due_now);
    CHECK(timer.current_tick == 182u);
    CHECK(timer.next_tick == 364u);
    CHECK(dm2_v1_weather_3df7_0037_receipt(&state, &timer, &weather));
    CHECK(weather.valid);
    CHECK(weather.transitioned);
    CHECK(weather.previous_weather == DM2_WEATHER_CLEAR);
    CHECK(weather.previous_seed == seed);
    CHECK(weather.next_seed == next_seed);
    CHECK(weather.next_weather == (uint8_t)next_weather);
    CHECK(weather.next_intensity == (uint8_t)state.weather_intensity);
    CHECK(weather.source_receipt_hash == timer.receipt_hash);
    CHECK(weather.receipt_hash != 0u);
    CHECK(state.weather_seed == next_seed);
    CHECK(state.weather == next_weather);

    CHECK(!dm2_v1_weather_set_timer_weather_receipt(1, 182u, NULL));
    CHECK(!dm2_v1_weather_3df7_0037_receipt(NULL, &timer, &weather));
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, NULL, &weather));
    CHECK(!dm2_v1_weather_3df7_0037_receipt(&state, &timer, NULL));

    if (failures) {
        fprintf(stderr, "DM2 weather timer receipts: %d failure(s)\n", failures);
        return 1;
    }
    puts("DM2 weather timer receipts passed");
    return 0;
}
