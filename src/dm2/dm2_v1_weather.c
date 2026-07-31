/* dm2_v1_weather.c — DM2 V1 Weather and Time-of-Day
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_tim_proc.cpp
 * docs/dm2_time.md, docs/dm2_creatures_gfx.md
 *
 * DM2 outdoor: 4 weather states, 1440-minute day cycle, per-champion torch.
 * DM1: no weather, no outdoor areas, single global torch.
 */

#include "dm2_v1_weather.h"
#include <math.h>
#include <string.h>

/* ReDMCSB/Baseline deterministic RNG for seeded transitions:
 * BASE.C F1695 / F1765:
 *   state = state * 0xBB40E62D + 11
 *   next_weather_state = (state >> 8) & 0x3
 */
#define DM2_WEATHER_LCG_MULTIPLIER 0xBB40E62Du
#define DM2_WEATHER_LCG_INCREMENT 11u

/* ── Weather names ────────────────────────────────────────────────────────
 * Source: docs/dm2_creatures_gfx.md, existing dm2_v1_outdoor_renderer.c */

static const char *const g_weather_names[DM2_WEATHER_COUNT] = {
    [DM2_WEATHER_CLEAR] = "Clear",
    [DM2_WEATHER_RAIN]  = "Rain",
    [DM2_WEATHER_FOG]   = "Fog",
    [DM2_WEATHER_STORM] = "Storm",
};

static uint32_t dm2_weather_state_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

void dm2_v1_weather_init(DM2_V1_WeatherState *state) {
    if (!state) return;
    state->weather = DM2_WEATHER_CLEAR;
    /* skweathr.cpp::DM2_UPDATE_WEATHER obtains the environment clock from
     * timdat.gametick plus source globals.  Do not substitute a noon clock
     * while that save/runtime owner is unavailable. */
    state->time_of_day = DM2_TIME_UNKNOWN;
    state->time_fraction = 0.0f;
    state->weather_intensity = 0;
    state->weather_seed = 0x0100u;
}

void dm2_v1_weather_set(DM2_V1_WeatherState *state, int weather) {
    if (!state) return;
    if (weather < 0) weather = 0;
    if (weather >= DM2_WEATHER_COUNT) weather = DM2_WEATHER_COUNT - 1;
    state->weather = weather;
    /* Intensity mapping: clear=0, rain=40, fog=30, storm=80 */
    static const uint8_t intensity[DM2_WEATHER_COUNT] = { 0, 40, 30, 80 };
    state->weather_intensity = intensity[weather];
}

void dm2_v1_weather_set_seed(DM2_V1_WeatherState *state, uint32_t seed) {
    if (!state) return;
    state->weather_seed = seed;
}

uint32_t dm2_v1_weather_advance_seed(uint32_t seed) {
    return seed * DM2_WEATHER_LCG_MULTIPLIER + DM2_WEATHER_LCG_INCREMENT;
}

int dm2_v1_weather_next_state(DM2_V1_WeatherState *state) {
    uint32_t next_seed;
    int next_weather;
    if (!state) return DM2_WEATHER_CLEAR;
    next_seed = dm2_v1_weather_advance_seed(state->weather_seed);
    state->weather_seed = next_seed;
    next_weather = (int)((next_seed >> 8) & 0x3u);
    dm2_v1_weather_set(state, next_weather);
    return state->weather;
}

static void dm2_v1_weather_mix_timer_receipt(
    DM2_V1_WeatherTimerReceipt *receipt)
{
    uint32_t hash = 2166136261u;

    if (!receipt) return;
#define DM2_WEATHER_TIMER_HASH(v) \
    do { \
        hash ^= (uint32_t)(v); \
        hash *= 16777619u; \
    } while (0)
    DM2_WEATHER_TIMER_HASH(receipt->source_set_timer_weather);
    DM2_WEATHER_TIMER_HASH(receipt->source_weather_3df7_0037);
    DM2_WEATHER_TIMER_HASH(receipt->outdoor);
    DM2_WEATHER_TIMER_HASH(receipt->due);
    DM2_WEATHER_TIMER_HASH(receipt->tick_count);
    DM2_WEATHER_TIMER_HASH(receipt->interval_ticks);
    DM2_WEATHER_TIMER_HASH(receipt->weather_before);
    DM2_WEATHER_TIMER_HASH(receipt->weather_after);
    DM2_WEATHER_TIMER_HASH(receipt->intensity_after);
    DM2_WEATHER_TIMER_HASH(receipt->seed_before);
    DM2_WEATHER_TIMER_HASH(receipt->seed_after);
#undef DM2_WEATHER_TIMER_HASH
    receipt->transaction_hash = hash == 0u ? 1u : hash;
    receipt->valid = 1;
}

int dm2_v1_weather_3df7_0037(DM2_V1_WeatherState *state,
                              DM2_V1_WeatherTimerReceipt *out_receipt)
{
    DM2_V1_WeatherTimerReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || state->weather < DM2_WEATHER_CLEAR ||
        state->weather >= DM2_WEATHER_COUNT) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.source_weather_3df7_0037 = 1;
    receipt.due = 1;
    receipt.interval_ticks = DM2_WEATHER_TIMER_INTERVAL_TICKS;
    receipt.weather_before = (uint8_t)state->weather;
    receipt.seed_before = state->weather_seed;

    (void)dm2_v1_weather_next_state(state);

    receipt.weather_after = (uint8_t)state->weather;
    receipt.intensity_after = (uint8_t)state->weather_intensity;
    receipt.seed_after = state->weather_seed;
    dm2_v1_weather_mix_timer_receipt(&receipt);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_weather_set_timer_weather(DM2_V1_WeatherState *state,
                                      int outdoor,
                                      uint32_t tick_count,
                                      DM2_V1_WeatherTimerReceipt *out_receipt)
{
    DM2_V1_WeatherTimerReceipt receipt;
    int due;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || state->weather < DM2_WEATHER_CLEAR ||
        state->weather >= DM2_WEATHER_COUNT) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.source_set_timer_weather = 1;
    receipt.outdoor = outdoor ? 1 : 0;
    receipt.tick_count = tick_count;
    receipt.interval_ticks = DM2_WEATHER_TIMER_INTERVAL_TICKS;
    receipt.weather_before = (uint8_t)state->weather;
    receipt.weather_after = (uint8_t)state->weather;
    receipt.intensity_after = (uint8_t)state->weather_intensity;
    receipt.seed_before = state->weather_seed;
    receipt.seed_after = state->weather_seed;

    due = receipt.outdoor &&
        tick_count != 0u &&
        tick_count % DM2_WEATHER_TIMER_INTERVAL_TICKS == 0u;
    receipt.due = due ? 1 : 0;
    if (due) {
        DM2_V1_WeatherTimerReceipt transition;
        if (!dm2_v1_weather_3df7_0037(state, &transition)) return 0;
        receipt.source_weather_3df7_0037 =
            transition.source_weather_3df7_0037;
        receipt.weather_after = transition.weather_after;
        receipt.intensity_after = transition.intensity_after;
        receipt.seed_after = transition.seed_after;
    }
    dm2_v1_weather_mix_timer_receipt(&receipt);
    if (out_receipt) *out_receipt = receipt;
    return due ? 1 : 0;
}

int dm2_v1_weather_timer_receipt_from_source_receipts(
    const DM2_V1_SetTimerWeatherReceipt *timer,
    const DM2_V1_Weather3df70037Receipt *transition,
    DM2_V1_WeatherTimerReceipt *out_receipt)
{
    DM2_V1_WeatherTimerReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!timer || !transition || !timer->valid || !timer->outdoor ||
        !timer->due_now || !timer->scheduled || timer->receipt_hash == 0u ||
        !transition->valid || !transition->transitioned ||
        transition->source_receipt_hash != timer->receipt_hash) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.source_set_timer_weather = 1;
    receipt.source_weather_3df7_0037 = 1;
    receipt.outdoor = 1;
    receipt.due = 1;
    receipt.tick_count = timer->current_tick;
    receipt.interval_ticks = DM2_WEATHER_TIMER_INTERVAL_TICKS;
    receipt.weather_before = transition->previous_weather;
    receipt.weather_after = transition->next_weather;
    receipt.intensity_after = transition->next_intensity;
    receipt.seed_before = transition->previous_seed;
    receipt.seed_after = transition->next_seed;
    dm2_v1_weather_mix_timer_receipt(&receipt);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

void dm2_v1_weather_advance_time(DM2_V1_WeatherState *state, int minutes) {
    if (!state) return;
    if (minutes < 0) return;
    if (state->time_of_day < 0 ||
        state->time_of_day >= DM2_TIME_MINUTES_MAX) {
        return;
    }
    state->time_of_day = (state->time_of_day + minutes) % DM2_TIME_MINUTES_MAX;
    state->time_fraction = (float)state->time_of_day / (float)DM2_TIME_MINUTES_MAX;
}

/* This legacy helper is kept only for callers that have a recovered
 * source-owned time. It must never manufacture a sky colour for unknown
 * environment state. Actual outdoor pixels remain GDAT-gated elsewhere. */
int dm2_v1_weather_sky_color(const DM2_V1_WeatherState *state) {
    if (!state || state->time_of_day < 0 ||
        state->time_of_day >= DM2_TIME_MINUTES_MAX) return -1;
    float t = state->time_fraction;
    /* Weather override: fog/storm always gray */
    if (state->weather >= DM2_WEATHER_FOG) {
        return 0xFF666666;
    }
    /* Rain: desaturated blue-gray */
    if (state->weather == DM2_WEATHER_RAIN) {
        if (t < 0.25f) return 0xFF887788;
        if (t < 0.75f) return 0xFF667788;
        return 0xFF333344;
    }
    /* Clear sky: full color gradient */
    if (t < 0.25f) {  /* dawn: red-orange gradient */
        uint8_t r = (uint8_t)(60 + t * 4.0f * 140);
        uint8_t b = (uint8_t)(180 + t * 4.0f * 20);
        return (0xFF000000u) | ((uint32_t)r << 16) | ((uint32_t)(r / 2) << 8) | b;
    }
    if (t < 0.75f) {  /* day: normal blue */
        return 0xFF4488CC;
    }
    /* dusk: orange-red */
    float nt = (t - 0.75f) * 4.0f;
    uint8_t g = (uint8_t)(80 - nt * 70);
    return (0xFF000000u) | ((uint32_t)(220 - nt * 200) << 16) | ((uint32_t)g << 8) | 200;
}

int dm2_v1_weather_particle_count(const DM2_V1_WeatherState *state) {
    if (!state) return 0;
    if (state->weather == DM2_WEATHER_CLEAR) return 0;
    /* Particle count proportional to weather intensity and rain density.
     * Rain: blitline_48 16→8-bit overlay sprites from GDAT.
     * Storm: more particles than rain. Fog: no particles (just overlay). */
    switch (state->weather) {
        case DM2_WEATHER_RAIN:  return state->weather_intensity * 2;
        case DM2_WEATHER_FOG:   return 0;  /* fog is overlay, no particles */
        case DM2_WEATHER_STORM: return state->weather_intensity * 3;
        default: return 0;
    }
}

int dm2_v1_weather_restored_state_receipt(
    const DM2_V1_WeatherState *state,
    DM2_V1_WeatherRestoredStateReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* Save restoration has already completed before this boundary.  Keep the
     * source-neutral runtime fields as identity only; c_weather.cpp owns
     * separate live cloud/rain counters and no original save offset for them
     * is proven here. */
    if (!state || state->weather < DM2_WEATHER_CLEAR ||
        state->weather >= DM2_WEATHER_COUNT || state->weather_intensity < 0 ||
        state->weather_intensity > UINT8_MAX || state->time_of_day < 0 ||
        state->time_of_day >= DM2_TIME_MINUTES_MAX) {
        return 0;
    }
    out->weather = (uint8_t)state->weather;
    out->intensity = (uint8_t)state->weather_intensity;
    out->time_of_day = (uint16_t)state->time_of_day;
    out->weather_seed = state->weather_seed;
    hash = dm2_weather_state_hash_step(hash, out->weather);
    hash = dm2_weather_state_hash_step(hash, out->intensity);
    hash = dm2_weather_state_hash_step(hash, out->time_of_day);
    hash = dm2_weather_state_hash_step(hash, out->weather_seed);
    if (hash == 0u) return 0;
    out->state_hash = hash;
    out->valid = 1;
    return 1;
}

int dm2_v1_weather_set_timer_weather_receipt(
    int is_outdoor,
    uint32_t current_tick,
    DM2_V1_SetTimerWeatherReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->outdoor = is_outdoor ? 1 : 0;
    out->current_tick = current_tick;
    out->due_now = out->outdoor && current_tick != 0u &&
        current_tick % 182u == 0u;
    out->scheduled = out->outdoor ? 1 : 0;
    out->next_tick = out->outdoor
        ? current_tick + (out->due_now ? 182u : 182u - current_tick % 182u)
        : 0u;
    hash = dm2_weather_state_hash_step(hash, 0x53545745u);
    hash = dm2_weather_state_hash_step(hash, (uint32_t)out->outdoor);
    hash = dm2_weather_state_hash_step(hash, current_tick);
    hash = dm2_weather_state_hash_step(hash, out->next_tick);
    hash = dm2_weather_state_hash_step(hash, (uint32_t)out->due_now);
    out->receipt_hash = hash;
    out->valid = hash != 0u;
    return out->valid;
}

int dm2_v1_weather_3df7_0037_receipt(
    DM2_V1_WeatherState *state,
    const DM2_V1_SetTimerWeatherReceipt *timer,
    DM2_V1_Weather3df70037Receipt *out)
{
    uint32_t hash = 2166136261u;
    uint32_t next_seed;
    int next_weather;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!state || !timer || !timer->valid || !timer->outdoor ||
        !timer->due_now || timer->receipt_hash == 0u ||
        state->weather < DM2_WEATHER_CLEAR ||
        state->weather >= DM2_WEATHER_COUNT) {
        return 0;
    }
    out->previous_weather = (uint8_t)state->weather;
    out->previous_seed = state->weather_seed;
    next_seed = dm2_v1_weather_advance_seed(state->weather_seed);
    next_weather = (int)((next_seed >> 8) & 0x3u);
    dm2_v1_weather_set_seed(state, next_seed);
    dm2_v1_weather_set(state, next_weather);
    out->next_weather = (uint8_t)state->weather;
    out->next_intensity = (uint8_t)state->weather_intensity;
    out->next_seed = state->weather_seed;
    out->source_receipt_hash = timer->receipt_hash;
    out->transitioned = 1;
    hash = dm2_weather_state_hash_step(hash, 0x57334446u);
    hash = dm2_weather_state_hash_step(hash, timer->receipt_hash);
    hash = dm2_weather_state_hash_step(hash, out->previous_weather);
    hash = dm2_weather_state_hash_step(hash, out->previous_seed);
    hash = dm2_weather_state_hash_step(hash, out->next_weather);
    hash = dm2_weather_state_hash_step(hash, out->next_intensity);
    hash = dm2_weather_state_hash_step(hash, out->next_seed);
    out->receipt_hash = hash;
    out->valid = hash != 0u;
    return out->valid;
}

const char *dm2_v1_weather_name(int weather) {
    if (weather < 0 || weather >= DM2_WEATHER_COUNT) return "?";
    return g_weather_names[weather];
}

const char *dm2_v1_weather_source_evidence(void) {
    return
        "DM2 V1 Weather and Time-of-Day — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKULLWIN/c_weather.cpp DM2_SET_TIMER_WEATHER\n"
        "Source: skproject/SKULLWIN/c_weather.cpp DM2_weather_3df7_0037\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp (PROCESS_TIMER_0C, CONTINUE_TICK_GENERATOR)\n"
        "Source: skproject/SKULLWIN/c_timer.cpp (timer system state)\n"
        "Source: docs/dm2_time.md (time-of-day 0-1439 min, per-champion torch)\n"
        "Source: docs/dm2_creatures_gfx.md (rain drop sprites from GDAT blitline_48)\n"
        "Source: include/dm2_v1_outdoor_renderer.h (4 weather states, time_fraction)\n"
        "Source: include/dm2_v1_game.h (time_of_day=720 noon start, 1440 min/day)\n"
        "DM1 comparison: NO weather system, NO outdoor, single torch (global)\n"
        "DM2 comparison: 4 weather states, outdoor areas, per-champion torch, event queue\n";
}
