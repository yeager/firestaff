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
 * Source: skproject SKWINSPX/src/v5/skweathr.cpp::DM2_UPDATE_WEATHER
 *
 * The original runtime derives environment state from its game clock and
 * environment globals.  Its serialized owner has not yet been recovered, so
 * a fresh Firestaff runtime must carry an explicit unknown value rather than
 * inventing a midday start. */

#define DM2_TIME_MINUTES_MAX   1440  /* minutes per day (24h * 60) */
#define DM2_TIME_UNKNOWN         (-1)
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
    uint32_t weather_seed;/* deterministic weather state */
    int weather_intensity;/* 0-100, affects particle density */
} DM2_V1_WeatherState;

/* A bounded identity for weather that has already been restored into the
 * runtime state.  This deliberately contains no guessed save offsets or
 * timer payload: save/load owns byte parsing and this module owns only the
 * validated post-restore state consumed by c_weather-style rendering. */
typedef struct {
    int valid;
    uint8_t weather;
    uint8_t intensity;
    uint16_t time_of_day;
    uint32_t weather_seed;
    uint32_t state_hash;
} DM2_V1_WeatherRestoredStateReceipt;
#define DM2_V1_WEATHER_RESTORED_STATE_RECEIPT_DEFINED 1

/* skproject/SKULLWIN/c_weather.cpp::DM2_SET_TIMER_WEATHER boundary.  It is a
 * scheduling receipt only: indoor frames and non-182 tick positions must not
 * mutate weather state or fabricate cloud/light material. */
typedef struct {
    int valid;
    int outdoor;
    uint32_t current_tick;
    uint32_t next_tick;
    int due_now;
    int scheduled;
    uint32_t receipt_hash;
} DM2_V1_SetTimerWeatherReceipt;

/* skproject/SKULLWIN/c_weather.cpp::DM2_weather_3df7_0037 boundary.  It owns
 * the live weather reseed/transition once DM2_SET_TIMER_WEATHER is due. */
typedef struct {
    int valid;
    int transitioned;
    uint8_t previous_weather;
    uint8_t next_weather;
    uint8_t next_intensity;
    uint32_t previous_seed;
    uint32_t next_seed;
    uint32_t source_receipt_hash;
    uint32_t receipt_hash;
} DM2_V1_Weather3df70037Receipt;

/* ── Timer IDs ─────────────────────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 * DM2 has layered timers per-champion and per-world-state */

#define DM2_TIMER_TORCH          0  /* per-champion light countdown */
#define DM2_TIMER_RESURRECTION    1  /* death countdown before permadeath */
#define DM2_TIMER_ORNATE_ANIM    2  /* wall ornament animation */
#define DM2_TIMER_TICK_GENERATOR 3  /* primary game event ticker */
#define DM2_TIMER_CREATURE_DEATH 4  /* KILL_ON_TIMER_POSITION (b_1a 0x0F) */
#define DM2_WEATHER_TIMER_INTERVAL_TICKS 182u

/* ── Per-champion torch state ──────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_tim_proc.cpp: PROCESS_TIMER_0C */

typedef struct {
    int is_lit;
    int ticks_remaining;
    int light_radius;   /* shrinks when torch low */
} DM2_V1_TorchState;

typedef struct {
    int valid;
    int source_set_timer_weather;
    int source_weather_3df7_0037;
    int outdoor;
    int due;
    uint32_t tick_count;
    uint32_t interval_ticks;
    uint8_t weather_before;
    uint8_t weather_after;
    uint8_t intensity_after;
    uint32_t seed_before;
    uint32_t seed_after;
    uint32_t transaction_hash;
} DM2_V1_WeatherTimerReceipt;

/* ── Public API ──────────────────────────────────────────────────────── */

void dm2_v1_weather_init(DM2_V1_WeatherState *state);
void dm2_v1_weather_set(DM2_V1_WeatherState *state, int weather);
void dm2_v1_weather_advance_time(DM2_V1_WeatherState *state, int minutes);
void dm2_v1_weather_set_seed(DM2_V1_WeatherState *state, uint32_t seed);
uint32_t dm2_v1_weather_advance_seed(uint32_t seed);
int dm2_v1_weather_next_state(DM2_V1_WeatherState *state);
int dm2_v1_weather_3df7_0037(DM2_V1_WeatherState *state,
                              DM2_V1_WeatherTimerReceipt *out_receipt);
int dm2_v1_weather_set_timer_weather(DM2_V1_WeatherState *state,
                                      int outdoor,
                                      uint32_t tick_count,
                                      DM2_V1_WeatherTimerReceipt *out_receipt);
int dm2_v1_weather_timer_receipt_from_source_receipts(
    const DM2_V1_SetTimerWeatherReceipt *timer,
    const DM2_V1_Weather3df70037Receipt *transition,
    DM2_V1_WeatherTimerReceipt *out_receipt);
int  dm2_v1_weather_sky_color(const DM2_V1_WeatherState *state);
int  dm2_v1_weather_particle_count(const DM2_V1_WeatherState *state);
const char *dm2_v1_weather_name(int weather);
const char *dm2_v1_weather_source_evidence(void);
int dm2_v1_weather_restored_state_receipt(
    const DM2_V1_WeatherState *state,
    DM2_V1_WeatherRestoredStateReceipt *out);
int dm2_v1_weather_set_timer_weather_receipt(
    int is_outdoor,
    uint32_t current_tick,
    DM2_V1_SetTimerWeatherReceipt *out);
int dm2_v1_weather_3df7_0037_receipt(
    DM2_V1_WeatherState *state,
    const DM2_V1_SetTimerWeatherReceipt *timer,
    DM2_V1_Weather3df70037Receipt *out);
const char *dm2_v1_weather_name(int weather);
const char *dm2_v1_weather_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_WEATHER_H */
