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

/* ── Weather names ────────────────────────────────────────────────────────
 * Source: docs/dm2_creatures_gfx.md, existing dm2_v1_outdoor_renderer.c */

static const char *const g_weather_names[DM2_WEATHER_COUNT] = {
    [DM2_WEATHER_CLEAR] = "Clear",
    [DM2_WEATHER_RAIN]  = "Rain",
    [DM2_WEATHER_FOG]   = "Fog",
    [DM2_WEATHER_STORM] = "Storm",
};

void dm2_v1_weather_init(DM2_V1_WeatherState *state) {
    if (!state) return;
    state->weather = DM2_WEATHER_CLEAR;
    state->time_of_day = DM2_TIME_START;  /* noon */
    state->time_fraction = 0.5f;
    state->weather_intensity = 0;
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

void dm2_v1_weather_advance_time(DM2_V1_WeatherState *state, int minutes) {
    if (!state) return;
    if (minutes < 0) return;
    state->time_of_day = (state->time_of_day + minutes) % DM2_TIME_MINUTES_MAX;
    state->time_fraction = (float)state->time_of_day / (float)DM2_TIME_MINUTES_MAX;
}

/* dm2_v1_weather_sky_color — derive sky color from weather + time-of-day
 * Source: docs/dm2_time.md (sky gradient derivation), existing dm2_v1_outdoor_renderer.c
 * Matches: dm2_v1_outdoor_sky_color() — kept for API parity.
 * DM2 outdoor sky: dawn gradient → day blue → dusk red/orange → night dark.
 * Weather overrides: fog/storm → gray. Rain → desaturated. */
int dm2_v1_weather_sky_color(const DM2_V1_WeatherState *state) {
    if (!state) return 0xFF4488CC;
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

const char *dm2_v1_weather_name(int weather) {
    if (weather < 0 || weather >= DM2_WEATHER_COUNT) return "?";
    return g_weather_names[weather];
}

const char *dm2_v1_weather_source_evidence(void) {
    return
        "DM2 V1 Weather and Time-of-Day — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp (PROCESS_TIMER_0C, CONTINUE_TICK_GENERATOR)\n"
        "Source: skproject/SKULLWIN/c_timer.cpp (timer system state)\n"
        "Source: docs/dm2_time.md (time-of-day 0-1439 min, per-champion torch)\n"
        "Source: docs/dm2_creatures_gfx.md (rain drop sprites from GDAT blitline_48)\n"
        "Source: include/dm2_v1_outdoor_renderer.h (4 weather states, time_fraction)\n"
        "Source: include/dm2_v1_game.h (time_of_day=720 noon start, 1440 min/day)\n"
        "DM1 comparison: NO weather system, NO outdoor, single torch (global)\n"
        "DM2 comparison: 4 weather states, outdoor areas, per-champion torch, event queue\n";
}