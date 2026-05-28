#ifndef FIRESTAFF_DM2_V2_LIGHTING_H
#define FIRESTAFF_DM2_V2_LIGHTING_H

/*
 * DM2 V2.2 Enhanced Lighting — Phase 4
 *
 * Presentation-only lighting/fog/sky-overlay for DM2 V2.
 * V1 lighting is driven by per-champion torch charges (SKULL.ASM PROCESS_TIMER_0C)
 * and per-dungeon-level ambient light.  DM2 V2.2 adds visual flourishes:
 *
 *   Indoor: fog-overlap darkness, ambient tint, sky-color ambient match.
 *   Outdoor: dynamic sky color blend (from DM2_V1_WeatherState), fog overlay
 *            when weather=Fog, per-weather ambient tint, lightning flash.
 *
 * V2 lighting is entirely presentation.  V1 game-state (torch charges, light
 * level, weather enum, time_of_day, triggers, combat, save) is unchanged.
 *
 * Reference:
 *   dm1_v2_lighting_dynamic_pc34.h — M11 V2 lighting API (DM1 V2.2)
 *   dm2_v1_weather.h              — DM2 weather/time state
 *   dm2_v1_outdoor_renderer.h     — DM2 V1 outdoor sky color
 *   dm2_v2_outdoor_enhanced.h     — outdoor V2 effects (cloud/rain/sway)
 *   docs/dm2_time.md              — time-of-day / weather / torch docs
 *
 * Source: SKULL.ASM PROCESS_TIMER_0C  — per-champion torch timers
 *         SKULL.ASM T600              — outdoor viewport
 *         SKULL.ASM T560              — indoor viewport
 *         docs/dm2_time.md §Weather   — fog/storm visibility reduction
 *         ReDMCSB PANEL.C:367-428     — DM1 palette lighting (analogy)
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "dm2_v1_weather.h"
#include <stdint.h>

/* ── Fog layer ──────────────────────────────────────────────────── */

#define DM2_V2_FOG_MAP_SIZE  32
#define DM2_V2_MAX_FOG_SOURCES 16

typedef struct {
    int x, y;           /* tile coords */
    float intensity;     /* 0.0 .. 1.0 */
    float radius;        /* tiles */
} DM2_V2_FogSource;

typedef struct {
    /* Per-tile fog density 0..255 (0=clear, 255=opaque) */
    uint8_t density[DM2_V2_FOG_MAP_SIZE][DM2_V2_FOG_MAP_SIZE];

    /* Active fog sources */
    DM2_V2_FogSource sources[DM2_V2_MAX_FOG_SOURCES];
    int source_count;

    /* Global weather-driven fog */
    float weather_fog;  /* 0..1 from weather state (fog=0.5, storm=0.3) */
} DM2_V2_FogState;

/* ── Ambient indoor light overlay ───────────────────────────────── */

typedef struct {
    /* Per-tile ambient multiplier 0..255 => 0.0x .. 1.0x tint */
    uint8_t ambient[DM2_V2_FOG_MAP_SIZE][DM2_V2_FOG_MAP_SIZE];

    /* Sky-color ambient for outdoor: lerp factor 0..1 */
    float sky_ambient_factor;

    /* Outdoor fog alpha 0..255 */
    uint8_t fog_alpha;
} DM2_V2_AmbientState;

/* ── Main lighting state ─────────────────────────────────────────── */

typedef struct {
    DM2_V2_FogState     fog;
    DM2_V2_AmbientState ambient;

    /* Time-of-day driven ambient color (per-sky-phase t: 0..1) */
    uint8_t dawn_r, dawn_g, dawn_b;   /* t < 0.25  */
    uint8_t day_r,  day_g,  day_b;   /* 0.25..0.75 */
    uint8_t dusk_r, dusk_g, dusk_b;  /* t > 0.75  */
    uint8_t night_r, night_g, night_b;

    /* Per-champion active torch flicker (indexed by champion slot) */
    float torch_flicker_phases[4];
    float torch_intensity[4];

    /* Outdoor bloom color (lightning flash) */
    uint8_t bloom_r, bloom_g, bloom_b;
    float bloom_timer;  /* seconds remaining, 0 = inactive */

    /* Phase-gate: outdoor enhanced effects enabled */
    int enhanced_outdoor;
} DM2_V2_LightingState;

/* ── Lifecycle ─────────────────────────────────────────────────── */

void dm2_v2_lighting_init(DM2_V2_LightingState *s);
void dm2_v2_lighting_reset(DM2_V2_LightingState *s);

/* ── Outdoor sky color query ───────────────────────────────────────
 * Returns DM2_V2 enhanced sky color rgba (0xAABGRRR packed or 0xRRGGBBAA).
 * Ignores NULL or out-of-range time; returns day-blue as fallback.
 * Delegates to dm2_v1_weather_sky_color for V1 sky gradient, adds V2 tint. */
uint32_t dm2_v2_sky_color_for_time(float time_fraction, int weather);
uint32_t dm2_v2_sky_color_from_weather(const DM2_V1_WeatherState *ws);

/* ── Fog management ─────────────────────────────────────────────── */

void dm2_v2_fog_init(DM2_V2_FogState *f);
void dm2_v2_fog_clear(DM2_V2_FogState *f);
int  dm2_v2_fog_add_source(DM2_V2_FogState *f, int x, int y,
                             float intensity, float radius);
void dm2_v2_fog_remove_source(DM2_V2_FogState *f, int id);
void dm2_v2_fog_set_weather(DM2_V2_FogState *f, int weather);
void dm2_v2_fog_rebuild(DM2_V2_FogState *f);
uint8_t dm2_v2_fog_get_tile(const DM2_V2_FogState *f, int x, int y);
void dm2_v2_fog_tick(DM2_V2_FogState *f, float dt);

/* ── Ambient light ──────────────────────────────────────────────── */

void dm2_v2_ambient_init(DM2_V2_AmbientState *a);
void dm2_v2_ambient_set_sky_factor(DM2_V2_AmbientState *a, float factor);
void dm2_v2_ambient_set_fog_alpha(DM2_V2_AmbientState *a, uint8_t alpha);
uint8_t dm2_v2_ambient_get_tile(const DM2_V2_AmbientState *a, int x, int y);
uint8_t dm2_v2_ambient_get_fog_alpha(const DM2_V2_AmbientState *a);

/* ── Torch flicker ─────────────────────────────────────────────── */

void dm2_v2_torch_flicker_tick(DM2_V2_LightingState *s, float dt);
float dm2_v2_torch_get_intensity(const DM2_V2_LightingState *s, int champion_slot);

/* ── Lightning bloom ────────────────────────────────────────────── */

void dm2_v2_lighting_trigger_bloom(DM2_V2_LightingState *s,
                                    uint8_t r, uint8_t g, uint8_t b,
                                    float duration_s);
void dm2_v2_lighting_tick_bloom(DM2_V2_LightingState *s, float dt);
int  dm2_v2_lighting_bloom_active(const DM2_V2_LightingState *s);
void dm2_v2_lighting_bloom_color(const DM2_V2_LightingState *s,
                                  uint8_t *r, uint8_t *g, uint8_t *b);

/* ── Source evidence ───────────────────────────────────────────── */
const char *dm2_v2_lighting_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_LIGHTING_H */
