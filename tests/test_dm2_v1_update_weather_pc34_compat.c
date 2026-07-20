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

static void test_frame_day_rollover(void);
static void test_frame_no_rollover_and_gate(void);
static void test_frame_intensity_branch(void);
static void test_frame_command_thresholds(void);
static void test_frame_slot_compaction(void);
static void test_frame_light_pending(void);
static void test_frame_bolt_and_thunder_sound(void);
static void test_frame_thunder_cloud_handoff(void);
static void test_frame_bounds_fail_closed(void);

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

    test_frame_day_rollover();
    test_frame_no_rollover_and_gate();
    test_frame_intensity_branch();
    test_frame_command_thresholds();
    test_frame_slot_compaction();
    test_frame_light_pending();
    test_frame_bolt_and_thunder_sound();
    test_frame_thunder_cloud_handoff();
    test_frame_bounds_fail_closed();

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_update_weather_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_update_weather_pc34_compat: all checks passed\n");
    return 0;
}

/* ── arg == 0 frame update (c_weather.cpp:91-506) ─────────────────── */

static DM2_V1_UpdateWeatherState mk_frame_state(int16_t zone,
                                                int16_t intensity)
{
    DM2_V1_UpdateWeatherState s;
    memset(&s, 0, sizeof(s));
    s.zone_index = zone;
    s.retry = 0;
    s.pattern_row = 1;
    s.step = 4;
    s.intensity = intensity;
    s.day_tick = 0x70000000;    /* no rollover by default */
    return s;
}

/* Scan for a seed that keeps the intensity==0 path quiet:
 * draw1 % 64 != 0 (no flash), draw2 & 1 == 0 (RANDBIT: no bolt). */
static uint32_t find_quiet_seed(void)
{
    uint32_t seed;
    for (seed = 1u; seed != 0u; ++seed) {
        uint32_t st = seed;
        uint32_t d1 = ref_rand(&st);
        uint32_t d2 = ref_rand(&st);
        if ((d1 & 0xffffu) % 64u != 0u && (d2 & 1u) == 0u)
            return seed;
    }
    return 0u;
}

static void test_frame_day_rollover(void)
{
    /* zone 1 -> table1d6b76[4*1+0x70] = 0x01 (weather allowed). */
    DM2_V1_UpdateWeatherState s = mk_frame_state(1, 0);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t seed = find_quiet_seed();

    CHECK(seed != 0u, "quiet seed found");
    rng.random = seed;
    s.day_tick = 100;
    CHECK(dm2_v1_update_weather_0(&s, 200, 0u, 1u, &rng, &rc) == 1,
          "frame slice runs");
    CHECK(rc.valid == 1, "frame receipt valid");
    CHECK(rc.weather_allowed == 1, "zone 1 weather flag read");
    CHECK(rc.day_rolled == 1, "day rollover ran");
    CHECK(rc.hour == (200 / 0x555) % 0x18, "rollover hour math");
    CHECK(rc.day_word ==
              (int16_t)dm2_v1_weather_table1d70f0[(200 / 0x555) % 0x18],
          "day_word from table1d70f0");
    CHECK(s.day_tick == 200 + 0x555, "day_tick advanced by 0x555");
    CHECK(rc.light_recalc_requests == 1,
          "rollover light recalc handed off when weather allowed");
    CHECK(rc.weather_gate == 1, "weather gate passed");
    CHECK(rc.flash_eval == 0, "quiet seed: no flash");
    CHECK(rc.draws == 2, "flash eval + RANDBIT draws on the quiet path");
}

static void test_frame_no_rollover_and_gate(void)
{
    /* zone 0 -> flag 0x00: early exit after the flash evaluation. */
    DM2_V1_UpdateWeatherState s = mk_frame_state(0, 0);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    rng.random = 0xdeadbeefu;
    s.clouds_enabled = 1;
    s.rain_enabled = 1;
    CHECK(dm2_v1_update_weather_0(&s, 42, 7u, 1u, &rng, &rc) == 1,
          "frame slice runs");
    CHECK(rc.day_rolled == 0, "no rollover before day_tick");
    CHECK(rc.weather_gate == 0, "weather gate blocks the command phase");
    CHECK(rc.cloud_cmd == 0 && rc.rain_cmd == 0 && rc.slots == 0,
          "no commands past the gate");
    CHECK(rc.light_recalc_requests == 0, "no light handoff at the gate");
    CHECK(s.cloud_state == 1, "intensity==0 sets cloud_state 1");
}

static void test_frame_intensity_branch(void)
{
    /* intensity != 0: threshold draw, cloud_state truncation, flag
     * latch, rain increment gating, gated flash evaluation. */
    DM2_V1_UpdateWeatherState s = mk_frame_state(0, 100);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    rng.random = 0x0badf00du;
    s.lightning_enabled = 1;
    s.lightning_flag = 1;           /* latched: no latch draw */
    s.rain_counter = 0x40;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, &rng, &rc) == 1,
          "frame slice runs");
    /* gametick 5: %3 != 0 and &3 != 0 -> no rain increment. */
    CHECK(s.rain_counter == 0x40, "rain holds when tick gates are closed");
    CHECK(s.cloud_state == 100, "cloud_state = CUTX8(intensity)");
    CHECK(rc.draws == 2, "r0 + gated flash draw, latch skipped");
    /* tick 4: &3 == 0 -> increment through the flag path. */
    rng.random = 0x0badf00du;
    s.rain_counter = 0x40;
    CHECK(dm2_v1_update_weather_0(&s, 4, 0u, 1u, &rng, &rc) == 1,
          "frame slice runs (tick 4)");
    CHECK(s.rain_counter == 0x41, "rain increments on tick&3 == 0");
}

static void test_frame_command_thresholds(void)
{
    /* Command byte selection: cloud 0x67/0x68/0x69 + storm, rain
     * 0x6a/0x6b/0x6c. intensity presets cloud_state via CUTX8. */
    DM2_V1_UpdateWeatherState s;
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    /* cloud 0x67: cloud_state 0x10..0x3f. */
    s = mk_frame_state(1, 0x20);
    rng.random = 0x11111111u;
    s.clouds_enabled = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD,
                                  1u, &rng, &rc) == 1, "slice runs (0x67)");
    CHECK(rc.cloud_cmd == 0x67, "cloud cmd 0x67 below 0x40");
    CHECK(rc.storm_set == 0, "no storm below 0x80");

    /* cloud 0x68: 0x40..0x7f. */
    s = mk_frame_state(1, 0x50);
    rng.random = 0x11111111u;
    s.clouds_enabled = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD,
                                  1u, &rng, &rc) == 1, "slice runs (0x68)");
    CHECK(rc.cloud_cmd == 0x68, "cloud cmd 0x68 below 0x80");

    /* cloud 0x69: >= 0x80 sets storm_active. */
    s = mk_frame_state(1, 0x90);
    rng.random = 0x11111111u;
    s.clouds_enabled = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD,
                                  1u, &rng, &rc) == 1, "slice runs (0x69)");
    CHECK(rc.cloud_cmd == 0x69, "cloud cmd 0x69 at 0x80+");
    CHECK(rc.storm_set == 1 && s.storm_active == 1, "storm set by 0x69");

    /* cloud below 0x10 writes nothing. */
    s = mk_frame_state(1, 0x08);
    rng.random = 0x11111111u;
    s.clouds_enabled = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, &rng, &rc) == 1,
          "slice runs (no cloud)");
    CHECK(rc.cloud_cmd == 0, "no cloud cmd below 0x10");

    /* rain 0x6a/0x6b/0x6c via intensity==0 presets (gametick 5 keeps the
     * counter: %3 != 0 blocks the decay). */
    s = mk_frame_state(1, 0);
    rng.random = 0x11111111u;
    s.rain_enabled = 1;
    s.rain_counter = 0x50;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN,
                                  1u, &rng, &rc) == 1, "slice runs (0x6a)");
    CHECK(rc.rain_cmd == 0x6a, "rain cmd 0x6a at 0x40..0x7f");

    s = mk_frame_state(1, 0);
    rng.random = 0x11111111u;
    s.rain_enabled = 1;
    s.rain_counter = 0x90;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN,
                                  1u, &rng, &rc) == 1, "slice runs (0x6b)");
    CHECK(rc.rain_cmd == 0x6b, "rain cmd 0x6b at 0x80..0xbf");

    s = mk_frame_state(1, 0);
    rng.random = 0x11111111u;
    s.rain_enabled = 1;
    s.rain_counter = 0x70;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN,
                                  1u, &rng, &rc) == 1, "slice runs (0x70)");
    CHECK(rc.rain_cmd == 0x6a, "rain cmd 0x6a at 0x70");

    s = mk_frame_state(1, 0);
    rng.random = 0x11111111u;
    s.rain_enabled = 1;
    s.rain_counter = (int8_t)0xd0;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN,
                                  1u, &rng, &rc) == 1, "slice runs (0x6c)");
    CHECK(rc.rain_cmd == 0x6c, "rain cmd 0x6c at 0xc0+");
}

static void test_frame_slot_compaction(void)
{
    /* A failed retrieve does not advance the slot: the next command
     * overwrites the same 10-byte slot. */
    DM2_V1_UpdateWeatherState s = mk_frame_state(1, 0x50);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    rng.random = 0x22222222u;
    s.clouds_enabled = 1;
    s.rain_enabled = 1;
    s.rain_counter = 0x50;
    /* cloud retrieve fails, rain ok -> rain lands in slot 0. */
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN,
                                  1u, &rng, &rc) == 1, "slice runs");
    CHECK(rc.cloud_cmd == 0x68, "cloud cmd evaluated");
    CHECK(rc.rain_cmd == 0x6a, "rain cmd evaluated");
    CHECK(rc.slots == 1, "one live slot after overwrite");
    CHECK(rc.live_cmds[0] == 0x6a, "rain overwrote the failed cloud slot");
}

static void test_frame_light_pending(void)
{
    DM2_V1_UpdateWeatherState s = mk_frame_state(1, 0);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    rng.random = 0x33333333u;
    s.light_pending = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, &rng, &rc) == 1,
          "slice runs");
    CHECK(s.light_pending == 0, "pending light change consumed");
    CHECK(rc.light_recalc_requests == 1, "light recalc handoff counted");
}

/* Scan for a seed that walks the intensity==0 -> flash -> bolt path:
 * draw1 % 64 == 0 (flash), draw2 is the RAND16(1) thunder check (always
 * 0 < 60), draw3 & 1 == 1 (RANDBIT bolt). */
static uint32_t find_bolt_seed(void)
{
    uint32_t seed;
    for (seed = 1u; seed != 0u; ++seed) {
        uint32_t st = seed;
        uint32_t d1 = ref_rand(&st);
        uint32_t d3;
        (void)ref_rand(&st);
        d3 = ref_rand(&st);
        if ((d1 & 0xffffu) % 64u == 0u && (d3 & 1u) != 0u)
            return seed;
    }
    return 0u;
}

static void test_frame_bolt_and_thunder_sound(void)
{
    DM2_V1_UpdateWeatherState s = mk_frame_state(1, 0);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;
    uint32_t seed = find_bolt_seed();
    uint32_t st = seed;
    int expect_bolt;
    int expect_rect;
    int expect_dir;
    int expect_vol;

    CHECK(seed != 0u, "bolt-path seed found");
    /* Expected draw sequence past the flash draw: the thunder check
     * RAND16(intensity+1) == RAND16(1) == 0 (< 60, no clouds), RANDBIT,
     * bolt index, rect rand, dir, volume (intensity==0 -> RAND16(10)+5). */
    (void)ref_rand(&st);                 /* flash draw               */
    (void)ref_rand(&st);                 /* RAND16(1) thunder check  */
    (void)ref_rand(&st);                 /* RANDBIT                  */
    expect_bolt = 100 + (int)((ref_rand(&st) & 0xffffu) % 3u);
    expect_rect = (int)((ref_rand(&st) & 0xffffu) % 100u);
    expect_dir = (int)(ref_rand(&st) & 3u);
    expect_vol = (int)((ref_rand(&st) & 0xffffu) % 10u) + 5;

    rng.random = seed;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_BOLT,
                                  1u, &rng, &rc) == 1, "slice runs");
    CHECK(rc.flash_eval == 1, "flash evaluated");
    CHECK(rc.bolt_cmd == expect_bolt, "bolt cmd = 100 + RAND16(3)");
    CHECK(rc.bolt_rect_rand == expect_rect, "bolt rect rand reported");
    CHECK(rc.bolt_dir == expect_dir, "bolt dir slot byte reported");
    CHECK(rc.thunder_sound == 1, "thunder sound handed off");
    CHECK(rc.thunder_volume == expect_vol, "thunder volume RAND16(10)+5");
    CHECK(s.thunder_latch == 1, "thunder latch set");
    CHECK(s.light_pending == 1, "final light change pending (m_4A899)");
    CHECK(rc.light_recalc_requests == 1, "final light recalc handoff");
    CHECK(rc.draws == 7, "seven LCG draws on the bolt path");
    CHECK(rc.slots == 1 && rc.live_cmds[0] == (uint8_t)expect_bolt,
          "bolt slot live");

    /* Latch set: the next flash clears it without a sound handoff. */
    rng.random = seed;
    CHECK(dm2_v1_update_weather_0(&s, 5, DM2_V1_UPDATE_WEATHER_RETRIEVE_BOLT,
                                  1u, &rng, &rc) == 1, "slice runs (latched)");
    CHECK(rc.thunder_sound == 0, "latched thunder silences the handoff");
    CHECK(s.thunder_latch == 0, "latch cleared");
}

/* Scan for a seed that walks intensity!=0 -> flash -> thunder clouds:
 * r0 (RAND), flash draw <= 7, RAND16(101) >= 60. lightning_flag starts
 * latched so no latch draw is consumed. */
static uint32_t find_thunder_seed(uint16_t *out_threshold)
{
    uint32_t seed;
    for (seed = 1u; seed != 0u; ++seed) {
        uint32_t st = seed;
        uint32_t r0 = ref_rand(&st);
        uint16_t thr = (uint16_t)(0x100u - 100u + (uint16_t)(r0 & 0xfu));
        uint32_t d2 = ref_rand(&st);
        uint32_t d3;
        if (thr == 0u || (d2 & 0xffffu) % thr > 7u)
            continue;
        d3 = ref_rand(&st);
        if ((d3 & 0xffffu) % 101u >= 60u) {
            *out_threshold = thr;
            return seed;
        }
    }
    return 0u;
}

static void test_frame_thunder_cloud_handoff(void)
{
    DM2_V1_UpdateWeatherState s = mk_frame_state(1, 100);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;
    uint16_t thr = 0;
    uint32_t seed = find_thunder_seed(&thr);

    CHECK(seed != 0u, "thunder-path seed found");
    rng.random = seed;
    s.lightning_enabled = 1;
    s.lightning_flag = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 0u, &rng, &rc) == 1,
          "slice runs");
    CHECK(rc.flash_eval == 1, "flash evaluated past threshold");
    CHECK(rc.light_flash_request == 1,
          "light flash handoff below 0xb6 intensity");
    CHECK(rc.thunder_count >= 1 && rc.thunder_count <= 8,
          "thunder count 1..8");
    CHECK(rc.cloud_placement_request == 1,
          "GDAT entry absent -> cloud placement handoff");
    CHECK(rc.invoke_message_request == 0, "no message path");
    CHECK(rc.rng_diverges == 1, "host continuation draws flagged");
    /* gdat_entry_6c != 0 selects the message/noise path instead. */
    rng.random = seed;
    s.lightning_flag = 1;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 7u, &rng, &rc) == 1,
          "slice runs (message path)");
    CHECK(rc.invoke_message_request == 1, "message path handoff");
    CHECK(rc.cloud_placement_request == 0, "no placement path");
}

static void test_frame_bounds_fail_closed(void)
{
    DM2_V1_UpdateWeatherState s = mk_frame_state(33, 0);
    DM2_V1_UpdateWeatherFrameReceipt rc;
    DM2_V1_DropRng rng;

    rng.random = 0x55555555u;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, &rng, &rc) == 0,
          "zone 33 rejected");
    CHECK(rc.valid == 0, "receipt invalid");

    s = mk_frame_state(1, 100);
    s.step = 0;
    rng.random = 0x55555555u;
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, &rng, &rc) == 0,
          "intensity with step 0 rejected (division guard)");
    CHECK(s.intensity == 100, "no mutation on rejection");

    s = mk_frame_state(1, 0);
    CHECK(dm2_v1_update_weather_0(&s, 5, 0u, 1u, 0, &rc) == 0,
          "NULL rng rejected");
    CHECK(dm2_v1_update_weather_0(0, 5, 0u, 1u, &rng, &rc) == 0,
          "NULL state rejected");
}
