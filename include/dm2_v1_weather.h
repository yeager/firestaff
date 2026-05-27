#ifndef FIRESTAFF_DM2_V1_WEATHER_H
#define FIRESTAFF_DM2_V1_WEATHER_H
#include <stdint.h>

/* DM2 V1 — Weather and Time-of-Day
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_tim_proc.cpp
 * docs/dm2_time.md, docs/dm2_creatures_gfx.md
 *
 * DM2 outdoor levels have:
 *   - 4 weather states (clear/rain/fog/storm)
 *   - Time-of-day cycle (0-1439 minutes from midnight)
 *   - Per-champion torch timers (PROCESS_TIMER_0C)
 *   - Rain drop sprites from graphics data
 *
 * DM1 had NO weather system and NO outdoor areas.
 */

/* ── Weather states ───────────────────────────────────────────────────
 * Source: include/dm2_v1_outdoor_renderer.h, docs/dm2_creatures_gfx.md
 * Processed each game tick via dm2_v2_outdoor_fx_tick() */

#define DM2_WEATHER_CLEAR   0
#define DM2_WEATHER_RAIN    1
#define DM2_WEATHER_FOG     2
#define DM2_WEATHER_STORM   3
#define DM2_WEATHER_COUNT   4

/* ── Time-of-day constants ────────────────────────────────────────────
 * Source: include/dm2_v1_game.h, docs/dm2_time.md
 * Minutes from midnight: 0-1439. Starting time: 720 (noon).
 * Sky color interpolates based on time. */

#define DM2_TIME_MINUTES_MAX   1440  /* minutes per day (24h * 60) */
#define DM2_TIME_START         720  /* noon in minutes */
#define DM2_TIME_DAWN_START    360  /* 6:00 AM */
#define DM2_TIME_DUSK_START   1080  /* 6:00 PM */
#define DM2_TIME_NIGHT_START  1200  /* 8:00 PM */

/* Time-of-day periods (fraction of day, 0.0-1.0) */
#define DM2_TIME_PERIOD_DAWN   0.25f
#define DM2_TIME_PERIOD_DAY    0.50f
#define DM2_TIME_PERIOD_DUSK   0.75f

/* ── Outdoor config struct (mirrors existing DM2_V1_OutdoorConfig) ────── */

typedef struct {
    int weather;          /* DM2_WEATHER_* */
    int time_of_day;      /* minutes from midnight (0-1439) */
    float time_fraction;  /* 0.0-1.0, derived from time_of_day */
    int weather_intensity;/* 0-100, affects particle density */
} DM2_V1_WeatherState;

/* ── Timer IDs ─────────────────────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 * DM2 has layered timers per-champion and per-world-state */

#define DM2_TIMER_TORCH          0  /* per-champion light countdown */
#define DM2_TIMER_RESURRECTION    1  /* death countdown before permadeath */
#define DM2_TIMER_ORNATE_ANIM    2  /* wall ornament animation */
#define DM2_TIMER_TICK_GENERATOR 3  /* primary game event ticker */
#define DM2_TIMER_CREATURE_DEATH 4  /* KILL_ON_TIMER_POSITION (b_1a 0x0F) */

/* ── Per-champion torch state ──────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_tim_proc.cpp: PROCESS_TIMER_0C */

typedef struct {
    int is_lit;
    int ticks_remaining;
    int light_radius;   /* shrinks when torch low */
} DM2_V1_TorchState;

/* ── Public API ──────────────────────────────────────────────────────── */

void dm2_v1_weather_init(DM2_V1_WeatherState *state);
void dm2_v1_weather_set(DM2_V1_WeatherState *state, int weather);
void dm2_v1_weather_advance_time(DM2_V1_WeatherState *state, int minutes);
int  dm2_v1_weather_sky_color(const DM2_V1_WeatherState *state);
int  dm2_v1_weather_particle_count(const DM2_V1_WeatherState *state);
const char *dm2_v1_weather_name(int weather);
const char *dm2_v1_weather_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_WEATHER_H */