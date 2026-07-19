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

int main(void)
{
    test_normal_step_and_requeue();
    test_transition_forced();
    test_clamp();
    test_bounds_fail_closed();
    test_retry_byte_wrap();
    test_requeue_bounds();

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_update_weather_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_update_weather_pc34_compat: all checks passed\n");
    return 0;
}
