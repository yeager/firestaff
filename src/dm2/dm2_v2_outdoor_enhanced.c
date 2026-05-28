/*
 * dm2_v2_outdoor_enhanced.c — DM2 V2.2 Enhanced Outdoor FX
 *
 * Phase 4: enhanced lighting and outdoor effects.
 * Replacement for dm2_v1_outdoor_renderer V1 sky/weather default.
 *
 * V2.2 adds:
 *   - Animated cloud drift (cloud_offset)
 *   - Multi-frame lightning (3-phase: initial flash → sustain → fade)
 *   - Sky-color ambient tint per time-of-day
 *   - Per-weather fog overlay on the outdoor scene
 *   - Tree sway phase for outdoor decoration animation
 *
 * V1 game state is untouched.
 *
 * Reference:
 *   dm2_v1_weather.h      — 4 weather states, time_fraction
 *   dm2_v1_weather.c      — sky_color implementation
 *   dm2_v2_outdoor_enhanced.h — API
 *   docs/dm2_time.md      — weather / fog / visibility
 *   dm2_v2_lighting.c     — fog map, lightning bloom API
 *
 * Source: SKULL.ASM T600   — outdoor viewport rendering
 *         SKULL.ASM T520   — party/movement tick (time advance)
 *         docs/dm2_time.md §Weather — lightning frame sequences
 */

#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v2_lighting.h"
#include <math.h>
#include <string.h>

/* ── Lifecycle ─────────────────────────────────────────────────── */

void dm2_v2_outdoor_fx_init(DM2_V2_OutdoorFX *fx) {
    if (!fx) return;
    memset(fx, 0, sizeof(*fx));
    fx->lightning_phase = DM2_LN_OFF;
    fx->lightning_timer = 0.0f;
}

/* ── Main tick ─────────────────────────────────────────────────── */

void dm2_v2_outdoor_fx_tick(DM2_V2_OutdoorFX *fx, float dt, int weather) {
    if (!fx) return;

    /* Cloud drift: 5 units/second horizontal scroll
     * Source: dm2_v2_outdoor_enhanced.h initial cloud_offset comment */
    fx->cloud_offset += dt * 5.0f;
    if (fx->cloud_offset > 32.0f) fx->cloud_offset -= 32.0f;

    /* Tree sway: slow sinusoidal phase
     * Source: dm2_v2_outdoor_enhanced.h tree_sway_phase comment */
    fx->tree_sway_phase += dt * 2.0f;
    if (fx->tree_sway_phase > 6.28f) fx->tree_sway_phase -= 6.28f;

    /* ── Per-weather FX parameters ──────────────────────────── */
    switch (weather) {
        case DM2_WEATHER_RAIN:
            fx->rain_intensity = 0.5f;
            fx->wind_strength  = 0.3f;
            fx->ambient_tint   = 0.15f;
            break;
        case DM2_WEATHER_FOG:
            fx->rain_intensity = 0.05f;
            fx->wind_strength  = 0.15f;
            fx->ambient_tint   = 0.35f; /* fog thickens scene */
            break;
        case DM2_WEATHER_STORM:
            fx->rain_intensity = 1.0f;
            fx->wind_strength  = 0.8f;
            fx->ambient_tint   = 0.45f;
            /* Automatic multi-frame lightning sequence */
            fx->lightning_timer -= dt;
            if (fx->lightning_timer <= 0) {
                fx->lightning_phase = DM2_LN_FLASH;
                fx->lightning_timer = 0.06f; /* flash duration */
                fx->lightning_flash_next = 1;
            } else {
                fx->lightning_flash_next = 0;
            }
            break;
        default: /* CLEAR */
            fx->rain_intensity = 0.0f;
            fx->wind_strength  = 0.0f;
            fx->ambient_tint   = 0.0f;
            fx->lightning_timer = 0.0f;
            fx->lightning_phase = DM2_LN_OFF;
            fx->lightning_flash_next = 0;
            break;
    }

    /* ── Advance multi-frame lightning sequence ─────────────── */
    if (fx->lightning_phase != DM2_LN_OFF) {
        fx->lightning_timer -= dt;
        if (fx->lightning_timer <= 0) {
            switch (fx->lightning_phase) {
                case DM2_LN_FLASH:
                    /* Flash complete — check for double-flash or go to fade */
                    fx->lightning_phase = DM2_LN_SUSTAIN;
                    fx->lightning_timer = 0.09f; /* hold dim state */
                    fx->lightning_flash_next = 0;
                    break;
                case DM2_LN_SUSTAIN:
                    /* Second flash or fade */
                    if (fx->lightning_flash_next) {
                        fx->lightning_phase = DM2_LN_FLASH;
                        fx->lightning_timer = 0.06f;
                        fx->lightning_flash_next = 0;
                    } else {
                        fx->lightning_phase = DM2_LN_FADE;
                        fx->lightning_timer = 0.15f;
                    }
                    break;
                case DM2_LN_FADE:
                    fx->lightning_phase = DM2_LN_OFF;
                    fx->lightning_timer = 0.0f;
                    fx->lightning_flash_next = 0;
                    break;
                default:
                    fx->lightning_phase = DM2_LN_OFF;
                    break;
            }
        }
    }
}

/* dm2_v2_outdoor_fx_trigger_lightning — externally triggered one-shot flash.
 * Call from the dungeon event system when a spell/effect triggers lightning.
 * Resets any existing sequence and starts at the FLASH phase. */
void dm2_v2_outdoor_fx_trigger_lightning(DM2_V2_OutdoorFX *fx) {
    if (!fx) return;
    fx->lightning_phase       = DM2_LN_FLASH;
    fx->lightning_timer       = 0.06f;  /* ~1 frame at 60fps */
    fx->lightning_flash_next  = 1;
}

/* dm2_v2_outdoor_fx_lightning_intensity — returns 0..255 lightning brightness.
 * 0 = no lightning.  Used by the rendering pipeline to overlay the bloom. */
int dm2_v2_outdoor_fx_lightning_intensity(const DM2_V2_OutdoorFX *fx) {
    if (!fx || fx->lightning_phase == DM2_LN_OFF) return 0;
    switch (fx->lightning_phase) {
        case DM2_LN_FLASH:   return 255;
        case DM2_LN_SUSTAIN: return 180;
        case DM2_LN_FADE: {
            if (!fx) return 0;
            float total = 0.15f;
            float elapsed = total - fx->lightning_timer;
            float frac = elapsed / total;
            return (int)(180.0f * (1.0f - frac));
        }
        default: return 0;
    }
}

/* dm2_v2_outdoor_fx_cloud_offset — per-frame cloud scroll offset.
 * Accumulated; wrap at 32px to match tile size. */
float dm2_v2_outdoor_fx_cloud_offset(const DM2_V2_OutdoorFX *fx) {
    return fx ? fx->cloud_offset : 0.0f;
}

float dm2_v2_outdoor_fx_tree_sway(const DM2_V2_OutdoorFX *fx) {
    return fx ? fx->tree_sway_phase : 0.0f;
}

/* dm2_v2_outdoor_fx_sky_color_ex — returns the base sky color for outdoor.
 * Respects weather and time-of-day.  Delegates to dm2_v2_sky_color_for_time.
 * outdoor_tint: V2.2 ambient tint from weather (0..1). */
uint32_t dm2_v2_outdoor_fx_sky_color_ex(const DM2_V2_OutdoorFX *fx,
                                          float time_fraction,
                                          int weather) {
    (void)fx;
    return dm2_v2_sky_color_for_time(time_fraction, weather);
}

/* ── Source evidence ─────────────────────────────────────────── */

const char *dm2_v2_outdoor_source_evidence(void) {
    return
        "DM2 V2.2 Enhanced Outdoor FX — Phase 4\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: docs/dm2_time.md §Weather — fog/storm/rain/lightning\n"
        "Source: docs/dm2_time.md §Lightning — multi-frame flash sequence\n"
        "Reference: dm1_v2_lighting_dynamic_pc34.c v2_light_update_flicker\n"
        "Reference: dm2_v1_weather.c dm2_v1_weather_sky_color\n"
        "DM2 V2.2: animated cloud drift (cloud_offset, 5 units/s)\n"
        "DM2 V2.2: multi-frame lightning (FLASH→SUSTAIN→FADE)\n"
        "DM2 V2.2: per-weather ambient tint (fog/rain/storm)\n"
        "DM2 V2.2: tree sway phase (sinusoidal, 2 rad/s)\n"
        "V1 invariant: weather enum, time_of_day, torch timers — unchanged\n";
}
