/*
 * test_dm2_v1_update_weather_pc34_compat.c — 0x54 timer dispatch into
 * DM2_UPDATE_WEATHER(1), bounded slice.
 *
 * Verifies the skproject boundary:
 *   c_tim_proc.cpp:4179-4183  0x54 dispatch: DM2_UPDATE_WEATHER(1)
 *   c_weather.cpp:66-67       v1e147f = table1d6b76[4*v1e1472 + 0x70]
 *   c_weather.cpp:70-77       ++v1e147b; retry > 0x1f forces transition
 *                             (DM2_weather_3df7_0037(0), host-owned), no
 *                             requeue
 *   c_weather.cpp:78-86       snapshot, intensity += (u8)step *
 *                             (i8)v1d7108[(row<<5)+retry], clamp 0..0xff
 *   c_weather.cpp:87-89       requeue DM2_SET_TIMER_WEATHER(RAND16(256)+50)
 *   dm2data.cpp:889-896       table1d6b76[132] bound verbatim
 *   dm2data.cpp:1371          v1d7108[128] bound verbatim from v1d7108.dat
 *   c_random.cpp:13-31        RAND16 LCG (shared DM2_V1_DropRng binding)
 */

#include "dm2_v1_update_weather_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

/* Reference copy of the source LCG (c_random.cpp:13-31) for cross-checking
 * the requeue draw: RAND16(256) = CUTX16(state*0xbb40e62d+11 >> 8) % 256. */
static uint32_t ref_rand(uint32_t *state)
{
    *state = *state * 0xbb40e62du + 11u;
    return *state >> 8;
}

static int ref_rand16_256(uint32_t *state)
{
    return (int)((ref_rand(state) & 0xffffu) % 256u);
}

static DM2_V1_UpdateWeatherState mk_state(int16_t zone, int8_t retry,
                                          int8_t row, int8_t step,
                                          int16_t intensity)
{
    DM2_V1_UpdateWeatherState s;
    s.zone_index = zone;
    s.weather_allowed = -1;
    s.retry = retry;
    s.pattern_row = row;
    s.step = step;
    s.intensity = intensity;
    s.previous_intensity = -1;
    return s;
}

/* (a) Normal pop: retry 0 -> 1, signed pattern delta scaled by the
 * zero-extended step, requeue delay = RAND16(256) + 50 cross-checked
 * against the reference LCG. */
static void test_normal_step_and_requeue(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, 0, 2, 4, 100);
    DM2_V1_UpdateWeatherReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t ref_state = 0x12345678u;
    int expected_draw;
    int expected_delta;

    rng.random = ref_state;
    expected_draw = ref_rand16_256(&ref_state);
    /* row 2, retry 1: v1d7108[(2<<5)+1] = 0x12 = +18; step 4 -> +72. */
    expected_delta = (int)dm2_v1_update_weather_pattern[(2 << 5) + 1];
    CHECK(expected_delta == 0x12, "bound pattern byte row2/retry1 == 0x12");

    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 1,
          "normal step returns 1");
    CHECK(rc.valid == 1, "receipt valid");
    CHECK(rc.weather_allowed == 0, "zone0 weather flag == 0x00");
    CHECK(s.weather_allowed == 0, "state weather_allowed stored");
    CHECK(rc.retry == 1 && s.retry == 1, "retry incremented 0 -> 1");
    CHECK(rc.pattern_delta == 0x12, "receipt pattern delta");
    CHECK(rc.intensity_before == 100, "intensity_before snapshot");
    CHECK(s.previous_intensity == 100, "v1e146e snapshot stored");
    CHECK(rc.intensity_after == 100 + 4 * 0x12,
          "intensity += step * delta");
    CHECK(s.intensity == 100 + 4 * 0x12, "state intensity updated");
    CHECK(rc.transition_forced == 0, "no transition on normal step");
    CHECK(rc.rand_draw == expected_draw, "RAND16(256) draw matches LCG");
    CHECK(rc.reschedule_delay == expected_draw + 50,
          "requeue delay == draw + 50");
    CHECK(rng.random == ref_state, "rng advanced exactly once");
}

/* (b) retry 0x1f -> 0x20: transition forced, no intensity mutation, no
 * requeue, rng untouched. The flag read and retry store still happen
 * (source order). */
static void test_transition_forced(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(1, 0x1f, 0, 4, 100);
    DM2_V1_UpdateWeatherReceipt rc;
    DM2_V1_DropRng rng;
    rng.random = 0xdeadbeefu;

    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 1,
          "transition path returns 1");
    CHECK(rc.valid == 1, "receipt valid on transition");
    CHECK(rc.weather_allowed == 1, "zone1 weather flag == 0x01");
    CHECK(rc.retry == 0x20 && (uint8_t)s.retry == 0x20,
          "retry incremented to 0x20");
    CHECK(rc.transition_forced == 1, "transition_forced set");
    CHECK(rc.reschedule_delay == -1, "no requeue on transition");
    CHECK(rc.rand_draw == -1, "no rand draw on transition");
    CHECK(rc.intensity_after == 100 && s.intensity == 100,
          "intensity untouched on transition");
    CHECK(s.previous_intensity == -1, "no snapshot on transition");
    CHECK(rng.random == 0xdeadbeefu, "rng not advanced on transition");
}

/* (c) Clamp at both ends. Row 3 retry 31: v1d7108[(3<<5)+31] = 0xf6 =
 * -10; step 10 -> -100. Row 0 retry 1: +5; step 10 -> +50. */
static void test_clamp(void)
{
    DM2_V1_UpdateWeatherState lo = mk_state(2, 30, 3, 10, 5);
    DM2_V1_UpdateWeatherState hi = mk_state(2, 0, 0, 10, 250);
    DM2_V1_UpdateWeatherReceipt rc;

    CHECK(dm2_v1_update_weather_pattern[(3 << 5) + 31] == (int8_t)0xf6,
          "bound pattern byte row3/retry31 == 0xf6");
    CHECK(dm2_v1_update_weather_1(&lo, 0, &rc) == 1, "clamp-low runs");
    CHECK(rc.pattern_delta == -10, "negative signed delta");
    CHECK(rc.intensity_after == 0 && lo.intensity == 0,
          "intensity clamped at 0");

    CHECK(dm2_v1_update_weather_1(&hi, 0, &rc) == 1, "clamp-high runs");
    CHECK(rc.intensity_after == 0xff && hi.intensity == 0xff,
          "intensity clamped at 0xff");
}

/* (d) Out-of-bounds zone / pattern row: fail-closed, no mutation. */
static void test_bounds_fail_closed(void)
{
    DM2_V1_UpdateWeatherState s;
    DM2_V1_UpdateWeatherReceipt rc;
    DM2_V1_DropRng rng;
    rng.random = 0x42u;

    s = mk_state(32, 0, 0, 4, 100);
    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 0,
          "zone 32 rejected");
    CHECK(rc.valid == 0, "receipt invalid for bad zone");
    CHECK(s.retry == 0 && s.intensity == 100 && s.weather_allowed == -1,
          "no mutation for bad zone");
    CHECK(rng.random == 0x42u, "rng untouched for bad zone");

    s = mk_state(-1, 0, 0, 4, 100);
    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 0,
          "zone -1 rejected");

    s = mk_state(0, 0, 4, 4, 100);
    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 0,
          "pattern row 4 rejected");
    CHECK(s.retry == 0 && s.intensity == 100, "no mutation for bad row");

    s = mk_state(0, 0, -1, 4, 100);
    CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 0,
          "pattern row -1 rejected");
}

/* (e) Byte-wrap retry: pre-increment 0xff wraps to 0x00 (source byte
 * arithmetic), which is NOT > 0x1f, so the step runs with index
 * (row<<5)+0. */
static void test_retry_byte_wrap(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, (int8_t)0xff, 0, 4, 100);
    DM2_V1_UpdateWeatherReceipt rc;

    CHECK(dm2_v1_update_weather_1(&s, 0, &rc) == 1, "wrapped retry runs");
    CHECK(rc.retry == 0 && s.retry == 0, "retry wrapped to 0");
    CHECK(rc.transition_forced == 0, "no transition on wrap");
    /* v1d7108[(0<<5)+0] = 0x01; step 4 -> +4. */
    CHECK(rc.intensity_after == 104, "step used pattern index row<<5");
}

/* (f) Requeue delay stays inside the source bounds 50..305 across a
 * streak of draws. */
static void test_requeue_bounds(void)
{
    DM2_V1_DropRng rng;
    rng.random = 1u;
    for (int i = 0; i < 64; ++i) {
        DM2_V1_UpdateWeatherState s = mk_state(0, 0, 1, 1, 100);
        DM2_V1_UpdateWeatherReceipt rc;
        CHECK(dm2_v1_update_weather_1(&s, &rng, &rc) == 1,
              "streak step runs");
        CHECK(rc.reschedule_delay >= DM2_V1_UPDATE_WEATHER_REQUEUE_MIN &&
                  rc.reschedule_delay <= DM2_V1_UPDATE_WEATHER_REQUEUE_MAX,
              "requeue delay within 50..305");
    }
}

/* (g) DM2_weather_3df7_0037 arg==0 normal reseed
 * (c_weather.cpp:518-567): draw order RAND16(8000) -> RANDDIR ->
 * RAND16(3) -> RANDDIR -> RAND16(4), queue delay draw+500, state reset,
 * time-of-day from table1d70f0. */
static void test_transition_reseed(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, 7, 1, 2, 200);
    DM2_V1_WeatherTransitionReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t ref_state = 0x00C0FFEEu;
    uint32_t rs = ref_state;
    int exp_delay = (int)(ref_rand(&rs) & 0xffffu) % 8000 + 500;
    int exp_row = (int)(ref_rand(&rs) & 0x3u);
    int exp_step = (int)(ref_rand(&rs) & 0xffffu) % 3 + 1;
    int exp_wind = (int)(ref_rand(&rs) & 0x3u);
    int exp_cloud = (int)(ref_rand(&rs) & 0xffffu) % 4 + 4;
    int32_t days;

    s.day_offset = 0;
    s.storm_request = 0;
    rng.random = ref_state;

    days = dm2_v1_weather_transition(&s, 2 * 0x555 + 100, 0, &rng, &rc);
    CHECK(rc.valid == 1 && rc.reseeded == 1 && rc.storm_path == 0,
          "arg==0 reseeds on the normal path");
    CHECK(rc.light_update_requested == 1,
          "light update requested (host-owned)");
    CHECK(s.day_tick == 2 * 0x555 + 100 + 0x555,
          "day_tick = gametick + 0x555");
    CHECK(s.storm_active == 0 && s.weather_allowed == 0,
          "storm/weather-allowed cleared");
    CHECK(rc.queue_delay == exp_delay, "queue delay = RAND16(8000)+500");
    CHECK(rc.queue_delay >= 500 && rc.queue_delay < 8500,
          "queue delay within 500..8499");
    CHECK(rc.pattern_row == exp_row && s.pattern_row == (int8_t)exp_row,
          "pattern_row = RANDDIR()");
    CHECK(rc.step == exp_step && s.step == (int8_t)exp_step,
          "step = RAND16(3)+1");
    CHECK(s.wind_dir == (int8_t)exp_wind, "wind_dir = RANDDIR()");
    CHECK(rc.cloud_timer == (int16_t)exp_cloud &&
              s.cloud_timer == (int16_t)exp_cloud,
          "cloud_timer = RAND16(4)+4");
    CHECK(s.cloud_state == 1 && s.lightning_flag == 0,
          "cloud/lightning state reset");
    CHECK(s.intensity == 0 && s.previous_intensity == 0 && s.retry == 0,
          "intensity/previous/retry reset");
    /* gametick = 2*0x555+100, offset 0: t = 2 (whole hours), hour 2,
     * days 0. */
    CHECK(rc.hour == 2 && days == 0 && rc.days == 0,
          "hour/days derived from (gametick+offset)/0x555");
    CHECK(rc.day_word == dm2_v1_weather_table1d70f0[2] &&
              s.day_word == dm2_v1_weather_table1d70f0[2],
          "day_word = table1d70f0[hour]");
    CHECK(s.storm_request == 0, "storm_request cleared");
    CHECK(rc.draws == 5 && rng.random == rs,
          "exactly five LCG draws in source order");
}

/* (h) DM2_weather_3df7_0037 arg==0 storm-forced branch
 * (c_weather.cpp:537-543): v1d7188 != 0 -> delay RAND16(500), row 3,
 * step 1, rain counter cleared; draws RAND16(500) -> RANDDIR ->
 * RAND16(4). */
static void test_transition_storm_path(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, 0, 0, 3, 128);
    DM2_V1_WeatherTransitionReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t rs = 777u;
    int exp_delay = (int)(ref_rand(&rs) & 0xffffu) % 500;
    int exp_wind = (int)(ref_rand(&rs) & 0x3u);
    int exp_cloud = (int)(ref_rand(&rs) & 0xffffu) % 4 + 4;

    s.rain_counter = 42;
    s.storm_request = 1;
    rng.random = 777u;

    (void)dm2_v1_weather_transition(&s, 0, 0, &rng, &rc);
    CHECK(rc.valid == 1 && rc.storm_path == 1, "storm branch taken");
    CHECK(rc.queue_delay == exp_delay, "storm delay = RAND16(500)");
    CHECK(rc.pattern_row == 3 && rc.step == 1,
          "storm forces pattern row 3 / step 1");
    CHECK(s.rain_counter == 0, "rain counter cleared");
    CHECK(s.wind_dir == (int8_t)exp_wind, "wind drawn after the delay");
    CHECK(rc.cloud_timer == (int16_t)exp_cloud,
          "cloud timer drawn last on the storm path");
    CHECK(rc.draws == 3 && rng.random == rs,
          "exactly three LCG draws on the storm path");
}

/* (i) DM2_weather_3df7_0037 arg!=0 (c_weather.cpp:557-560): no reseed,
 * no requeue, previous cleared, step floored to 1; common tail still
 * runs (RAND16(4) draw, time-of-day). */
static void test_transition_keep_current(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, 5, 2, 0, 90);
    DM2_V1_WeatherTransitionReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t rs = 0xABCDEFu;
    int exp_cloud = (int)(ref_rand(&rs) & 0xffffu) % 4 + 4;

    s.previous_intensity = 33;
    s.storm_request = 1;
    rng.random = 0xABCDEFu;

    (void)dm2_v1_weather_transition(&s, 0x555 * 5, 1, &rng, &rc);
    CHECK(rc.valid == 1 && rc.reseeded == 0, "arg!=0 does not reseed");
    CHECK(rc.queue_delay == -1, "arg!=0 never requeues");
    CHECK(rc.light_update_requested == 0,
          "no light update on the keep-current branch");
    CHECK(s.previous_intensity == 0, "previous snapshot cleared");
    CHECK(s.step == 1, "step floored to 1 when zero");
    CHECK(s.pattern_row == 2, "pattern row kept");
    CHECK(s.intensity == 90, "intensity kept");
    CHECK(s.retry == 5, "retry kept");
    CHECK(rc.cloud_timer == (int16_t)exp_cloud,
          "common tail still draws RAND16(4)+4");
    /* gametick = 5*0x555, offset 0 -> t = 5, hour 5, days 0. */
    CHECK(rc.hour == 5 && rc.days == 0, "hour/days from common tail");
    CHECK(s.storm_request == 0, "storm_request cleared by the tail");
    CHECK(rc.draws == 1 && rng.random == rs,
          "exactly one LCG draw on the keep-current branch");
}

/* (j) Transition fail-closed on NULL rng: no mutation. */
static void test_transition_null_rng(void)
{
    DM2_V1_UpdateWeatherState s = mk_state(0, 0, 0, 2, 100);
    DM2_V1_WeatherTransitionReceipt rc;

    CHECK(dm2_v1_weather_transition(&s, 0, 0, 0, &rc) == 0,
          "NULL rng rejected");
    CHECK(rc.valid == 0, "receipt invalid for NULL rng");
    CHECK(s.intensity == 100 && s.step == 2, "no mutation for NULL rng");
}

int main(void)
{
    test_normal_step_and_requeue();
    test_transition_forced();
    test_clamp();
    test_bounds_fail_closed();
    test_retry_byte_wrap();
    test_requeue_bounds();
    test_transition_reseed();
    test_transition_storm_path();
    test_transition_keep_current();
    test_transition_null_rng();

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_update_weather_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_update_weather_pc34_compat: all checks passed\n");
    return 0;
}
