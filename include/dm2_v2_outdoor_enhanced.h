#ifndef FIRESTAFF_DM2_V2_OUTDOOR_ENHANCED_H
#define FIRESTAFF_DM2_V2_OUTDOOR_ENHANCED_H

/*
 * DM2 V2.2 Enhanced Outdoor — Phase 4: Lighting and Outdoor Effects
 *
 * V1 outdoor: static sky gradient + weather flag.
 * V2.2 outdoor: animated weather (rain particles, cloud movement, lightning
 *   multi-frame, tree sway), dynamic per-weather ambient tint, sky-color
 *   blend from time-of-day.
 *
 * All effects are presentation-only.  V1 game state (weather enum,
 * time_of_day, triggers, torch charges) is untouched.
 *
 * Source: SKULL.ASM T600 — outdoor viewport rendering
 *         docs/dm2_time.md §Weather / §Lightning
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Lightning multi-frame state machine ─────────────────────── */

typedef enum {
    DM2_LN_OFF     = 0,
    DM2_LN_FLASH   = 1,   /* initial flash */
    DM2_LN_SUSTAIN = 2,   /* second/third flash or hold */
    DM2_LN_FADE    = 3,   /* gradual fade-out */
} DM2_LightningPhase;

/* ── Main outdoor FX state ────────────────────────────────────── */

typedef struct {
    /* Animation state */
    float cloud_offset;       /* horizontal cloud drift, wraps 32px */
    float tree_sway_phase;   /* sinusoidal sway phase, wraps 2π */
    float ambient_tint;       /* per-weather ambient overlay 0..1 */

    /* Lightning multi-frame sequence */
    DM2_LightningPhase lightning_phase;
    float              lightning_timer;  /* frame countdown */
    int                lightning_flash_next; /* double-flash trigger */
    float              lightning_flash_interval; /* random interval base */

    /* Weather-driven parameters */
    float rain_intensity;    /* rain particle density 0..1 */
    float wind_strength;    /* wind speed 0..1 */
} DM2_V2_OutdoorFX;

/* ── Lifecycle ───────────────────────────────────────────────── */

void dm2_v2_outdoor_fx_init(DM2_V2_OutdoorFX *fx);

/* dm2_v2_outdoor_fx_tick — advance outdoor animation state.
 * dt: seconds since last tick.
 * weather: DM2_WEATHER_CLEAR/RAIN/FOG/STORM from dm2_v1_weather.h.
 * Updates cloud drift, tree sway, weather FX, and lightning sequence. */
void dm2_v2_outdoor_fx_tick(DM2_V2_OutdoorFX *fx, float dt, int weather);

/* dm2_v2_outdoor_fx_trigger_lightning — one-shot lightning flash.
 * Starts multi-frame FLASH→SUSTAIN→FADE sequence. */
void dm2_v2_outdoor_fx_trigger_lightning(DM2_V2_OutdoorFX *fx);

/* dm2_v2_outdoor_fx_lightning_intensity — 0 (no lightning) .. 255 (flash peak). */
int dm2_v2_outdoor_fx_lightning_intensity(const DM2_V2_OutdoorFX *fx);

/* dm2_v2_outdoor_fx_cloud_offset — current cloud scroll offset. */
float dm2_v2_outdoor_fx_cloud_offset(const DM2_V2_OutdoorFX *fx);

/* dm2_v2_outdoor_fx_tree_sway — current tree sway phase (radians). */
float dm2_v2_outdoor_fx_tree_sway(const DM2_V2_OutdoorFX *fx);

/* dm2_v2_outdoor_fx_sky_color_ex — base sky color for outdoor rendering.
 * Accounts for time_fraction and weather; matches V1 sky color API. */
uint32_t dm2_v2_outdoor_fx_sky_color_ex(const DM2_V2_OutdoorFX *fx,
                                         float time_fraction, int weather);

/* ── Source evidence ─────────────────────────────────────────── */
const char *dm2_v2_outdoor_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
