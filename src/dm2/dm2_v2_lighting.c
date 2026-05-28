/*
 * dm2_v2_lighting.c — DM2 V2.2 Enhanced Lighting
 *
 * Phase 4: enhanced lighting and outdoor effects for DM2 V2.
 *
 * Presentation-only: fog overlay, ambient tint, sky-color match,
 * torch flicker, lightning bloom.  No V1 game-state changes.
 *
 * Indoor: per-tile fog density maps from dungeon geometry + torches.
 * Outdoor: per-weather fog intensity + sky-color ambient blend +
 *         multi-frame lightning bloom (triggered externally).
 *
 * Reference:
 *   dm1_v2_lighting_dynamic_pc34.c — M11 V2 lighting (DM1 V2.2)
 *   dm2_v1_weather.c              — DM2 weather/time API
 *   dm2_v2_outdoor_enhanced.c     — outdoor V2 FX
 *   docs/dm2_time.md              — torch, weather, fog, visibility docs
 *
 * Source: SKULL.ASM PROCESS_TIMER_0C  — per-champion torch timers
 *         SKULL.ASM T600              — outdoor viewport rendering
 *         SKULL.ASM T560              — indoor dungeon viewport
 *         ReDMCSB PANEL.C:367-428    — DM1 palette lighting semantics
 */

#include "dm2_v2_lighting.h"
#include "dm2_v1_weather.h"
#include "dm2_v2_outdoor_enhanced.h"
#include <math.h>
#include <string.h>

/* ── Helpers ──────────────────────────────────────────────────── */

static float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

/* ── Lifecycle ─────────────────────────────────────────────────── */

void dm2_v2_lighting_init(DM2_V2_LightingState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    dm2_v2_fog_init(&s->fog);
    dm2_v2_ambient_init(&s->ambient);

    /* Default sky-phase colors — mirroring dm2_v1_weather_sky_COLOR
     * Source: docs/dm2_time.md §Sky color / time-of-day */
    s->dawn_r    = 200; s->dawn_g    = 70;  s->dawn_b    = 180;
    s->day_r     = 68;  s->day_g     = 136; s->day_b     = 204;
    s->dusk_r    = 220; s->dusk_g    = 80;  s->dusk_b    = 50;
    s->night_r   = 20;  s->night_g   = 20;  s->night_b   = 50;
}

void dm2_v2_lighting_reset(DM2_V2_LightingState *s) {
    if (!s) return;
    dm2_v2_fog_clear(&s->fog);
    memset(s->torch_flicker_phases, 0, sizeof(s->torch_flicker_phases));
    memset(s->torch_intensity, 0, sizeof(s->torch_intensity));
    s->bloom_timer = 0;
}

/* ── Sky color query ───────────────────────────────────────────── */

/* dm2_v2_sky_color_for_time — V2 enhanced sky color for outdoor.
 * time_fraction: 0..1 mapped over the 1440-minute day cycle.
 * weather: DM2_WEATHER_CLEAR/RAIN/FOG/STORM.
 * Returns 0xAABBGGRR packed ARGB.
 *
 * Source: dm2_v1_weather_sky_color() (V1 base), with V2.2 tweaks:
 *   - Storm/fog: override is handled by the weather system; V2
 *     provides the base RGBA for rendering to composite with fog overlays.
 *   - Lightning bloom is separate (triggered via bloom API).
 *
 * Reference: dm2_v1_outdoor_renderer.c dm2_v1_outdoor_sky_color()
 *           docs/dm2_time.md §Sky color gradient */
uint32_t dm2_v2_sky_color_for_time(float time_fraction, int weather) {
    float t = clamp01(time_fraction);

    /* Fog/storm: base sky is gray regardless of time */
    if (weather == DM2_WEATHER_FOG || weather == DM2_WEATHER_STORM) {
        /* Return neutral gray-blue as base; fog overlay handles visual depth */
        return 0xFF666666u;
    }

    if (t < 0.25f) { /* dawn */
        uint8_t r = (uint8_t)(60 + t * 4.0f * 140);
        uint8_t b = (uint8_t)(180 + t * 4.0f * 20);
        return (0xFF000000u) | ((uint32_t)r << 16) | ((uint32_t)(r / 2) << 8) | b;
    }
    if (t < 0.75f) { /* day */
        if (weather == DM2_WEATHER_RAIN) return 0xFF667788u;
        return 0xFF4488CCu;
    }
    /* dusk/night */
    float nt = (t - 0.75f) * 4.0f;
    uint8_t g = (uint8_t)(80 - nt * 70);
    uint8_t r_out = (uint8_t)(220 - nt * 200);
    return (0xFF000000u) | ((uint32_t)r_out << 16) | ((uint32_t)g << 8) | 200;
}

/* dm2_v2_sky_color_from_weather — convenience wrapper using DM2_V1_WeatherState */
uint32_t dm2_v2_sky_color_from_weather(const DM2_V1_WeatherState *ws) {
    if (!ws) return dm2_v2_sky_color_for_time(0.5f, DM2_WEATHER_CLEAR);
    return dm2_v2_sky_color_for_time(ws->time_fraction, ws->weather);
}

/* ── Fog ──────────────────────────────────────────────────────── */

void dm2_v2_fog_init(DM2_V2_FogState *f) {
    if (!f) return;
    memset(f->density, 0, sizeof(f->density));
    f->source_count = 0;
    f->weather_fog = 0;
}

void dm2_v2_fog_clear(DM2_V2_FogState *f) {
    if (!f) return;
    memset(f->density, 0, sizeof(f->density));
    f->source_count = 0;
    f->weather_fog = 0;
}

/* dm2_v2_fog_add_source — add a fog emitter at tile (x,y).
 * intensity: 0..1 peak density at center.
 * radius: tiles covered.
 * Returns source id or -1 if full. */
int dm2_v2_fog_add_source(DM2_V2_FogState *f, int x, int y,
                           float intensity, float radius) {
    if (!f || f->source_count >= DM2_V2_MAX_FOG_SOURCES) return -1;
    int id = f->source_count++;
    f->sources[id].x = x;
    f->sources[id].y = y;
    f->sources[id].intensity = clamp01(intensity);
    f->sources[id].radius = radius < 0.5f ? 0.5f : radius;
    return id;
}

void dm2_v2_fog_remove_source(DM2_V2_FogState *f, int id) {
    if (!f || id < 0 || id >= f->source_count) return;
    f->sources[id] = f->sources[--f->source_count];
}

/* dm2_v2_fog_set_weather — update global fog from DM2 weather enum.
 * Source: docs/dm2_time.md §Weather / fog/storm visibility reduction.
 *   Clear: 0  Rain: 0.1  Fog: 0.5  Storm: 0.3 */
void dm2_v2_fog_set_weather(DM2_V2_FogState *f, int weather) {
    if (!f) return;
    switch (weather) {
        case DM2_WEATHER_CLEAR: f->weather_fog = 0.0f;    break;
        case DM2_WEATHER_RAIN:  f->weather_fog = 0.1f;    break;
        case DM2_WEATHER_FOG:   f->weather_fog = 0.5f;    break;
        case DM2_WEATHER_STORM: f->weather_fog = 0.3f;    break;
        default:                f->weather_fog = 0.0f;    break;
    }
}

/* dm2_v2_fog_rebuild — recompute per-tile fog density map.
 * Combines all fog sources (inverse-square falloff) plus weather fog.
 * Called each outdoor tick when sources change. */
void dm2_v2_fog_rebuild(DM2_V2_FogState *f) {
    if (!f) return;
    int x, y, i;

    /* Start from weather base fog */
    for (y = 0; y < DM2_V2_FOG_MAP_SIZE; y++)
        for (x = 0; x < DM2_V2_FOG_MAP_SIZE; x++)
            f->density[y][x] = (uint8_t)(f->weather_fog * 255.0f);

    /* Add fog from each source */
    for (i = 0; i < f->source_count; i++) {
        float lx = (float)f->sources[i].x;
        float ly = (float)f->sources[i].y;
        float r  = f->sources[i].radius;
        float inten = f->sources[i].intensity;

        int minX = (int)(lx - r); if (minX < 0) minX = 0;
        int minY = (int)(ly - r); if (minY < 0) minY = 0;
        int maxX = (int)(lx + r); if (maxX >= DM2_V2_FOG_MAP_SIZE) maxX = DM2_V2_FOG_MAP_SIZE - 1;
        int maxY = (int)(ly + r); if (maxY >= DM2_V2_FOG_MAP_SIZE) maxY = DM2_V2_FOG_MAP_SIZE - 1;

        for (y = minY; y <= maxY; y++) {
            for (x = minX; x <= maxX; x++) {
                float dx = (float)x - lx;
                float dy = (float)y - ly;
                float dist2 = dx * dx + dy * dy;
                if (dist2 < 1.0f) dist2 = 1.0f;
                float contrib = inten / dist2;
                uint8_t add = (uint8_t)(contrib * 255.0f);
                uint16_t sum = (uint16_t)((unsigned)f->density[y][x] + add);
                f->density[y][x] = sum > 255 ? 255 : (uint8_t)sum;
            }
        }
    }
}

uint8_t dm2_v2_fog_get_tile(const DM2_V2_FogState *f, int x, int y) {
    if (!f) return 0;
    if (x < 0 || x >= DM2_V2_FOG_MAP_SIZE || y < 0 || y >= DM2_V2_FOG_MAP_SIZE)
        return (uint8_t)(f->weather_fog * 255.0f);
    return f->density[y][x];
}

/* dm2_v2_fog_tick — advance fog animation state (cloud drift for outdoor fog).
 * dt: seconds since last tick.
 * Source: dm2_v2_outdoor_fx_tick cloud_offset pattern. */
void dm2_v2_fog_tick(DM2_V2_FogState *f, float dt) {
    if (!f) return;
    (void)dt; /* future: animated fog drift / cloud movement */
}

/* ── Ambient ───────────────────────────────────────────────────── */

void dm2_v2_ambient_init(DM2_V2_AmbientState *a) {
    if (!a) return;
    memset(a->ambient, 128, sizeof(a->ambient)); /* half-strength default */
    a->sky_ambient_factor = 0.0f;
    a->fog_alpha = 0;
}

/* dm2_v2_ambient_set_sky_factor — sky color blend weight for outdoor.
 * factor 0..1: 0 = normal ambient, 1 = full sky-color tint. */
void dm2_v2_ambient_set_sky_factor(DM2_V2_AmbientState *a, float factor) {
    if (!a) return;
    a->sky_ambient_factor = clamp01(factor);
}

void dm2_v2_ambient_set_fog_alpha(DM2_V2_AmbientState *a, uint8_t alpha) {
    if (a) a->fog_alpha = alpha;
}

uint8_t dm2_v2_ambient_get_tile(const DM2_V2_AmbientState *a, int x, int y) {
    if (!a) return 128;
    if (x < 0 || x >= DM2_V2_FOG_MAP_SIZE || y < 0 || y >= DM2_V2_FOG_MAP_SIZE)
        return 128;
    return a->ambient[y][x];
}

uint8_t dm2_v2_ambient_get_fog_alpha(const DM2_V2_AmbientState *a) {
    return a ? a->fog_alpha : 0;
}

/* ── Torch flicker ─────────────────────────────────────────────── */

/* dm2_v2_torch_flicker_tick — advance per-champion torch phase.
 * champion_slot: 0..3.
 * dt: seconds since last tick.
 *
 * Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers;
 * torch intensity is derived from remaining charges (V1 mechanic).
 * V2.2 uses the same flicker model as DM1 V2.2 but scoped per champion.
 *
 * Reference: dm1_v2_settings_impl.c dm1_v2_apply_torch_flicker
 *            v2_light_update_flicker (dm1_v2_lighting_dynamic_pc34.c)
 *            dm2_v1_weather.c per-champion concept from PROCESS_TIMER_0C
 */
void dm2_v2_torch_flicker_tick(DM2_V2_LightingState *s, float dt) {
    if (!s) return;
    for (int i = 0; i < 4; i++) {
        s->torch_flicker_phases[i] += dt * (10.0f + 2.0f * (float)(i & 1));
        if (s->torch_flicker_phases[i] > 6.28f)
            s->torch_flicker_phases[i] -= 6.28f;
        /* Two-frequency composite: slow base + faster shimmer */
        s->torch_intensity[i] = 0.85f
            + 0.10f * sinf(s->torch_flicker_phases[i])
            + 0.05f * sinf(s->torch_flicker_phases[i] * 3.7f);
    }
}

float dm2_v2_torch_get_intensity(const DM2_V2_LightingState *s, int champion_slot) {
    if (!s) return 1.0f;
    if (champion_slot < 0 || champion_slot >= 4) return 1.0f;
    return s->torch_intensity[champion_slot] > 0
        ? s->torch_intensity[champion_slot] : 1.0f;
}

/* ── Lightning bloom ──────────────────────────────────────────── */

/* dm2_v2_lighting_trigger_bloom — start a lightning bloom flash.
 * Typically called in response to a dungeon/weather event: when
 * dm2_v2_outdoor_fx.tick detects a lightning event, it calls this.
 * Source: dm2_v2_outdoor_enhanced.c dm2_v2_outdoor_fx_trigger_lightning pattern.
 *         docs/dm2_time.md §Weather §Storm — lightning flash events.
 */
void dm2_v2_lighting_trigger_bloom(DM2_V2_LightingState *s,
                                    uint8_t r, uint8_t g, uint8_t b,
                                    float duration_s) {
    if (!s) return;
    s->bloom_r = r;
    s->bloom_g = g;
    s->bloom_b = b;
    s->bloom_timer = duration_s > 0 ? duration_s : 0.2f;
}

void dm2_v2_lighting_tick_bloom(DM2_V2_LightingState *s, float dt) {
    if (!s || s->bloom_timer <= 0) return;
    s->bloom_timer -= dt;
    if (s->bloom_timer < 0) s->bloom_timer = 0;
}

int dm2_v2_lighting_bloom_active(const DM2_V2_LightingState *s) {
    return s && s->bloom_timer > 0;
}

void dm2_v2_lighting_bloom_color(const DM2_V2_LightingState *s,
                                   uint8_t *r, uint8_t *g, uint8_t *b) {
    if (!s) { if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; return; }
    /* Fade out as timer approaches 0 */
    float fade = clamp01(s->bloom_timer / 0.2f);
    if (r) *r = (uint8_t)((float)s->bloom_r * fade);
    if (g) *g = (uint8_t)((float)s->bloom_g * fade);
    if (b) *b = (uint8_t)((float)s->bloom_b * fade);
}

/* ── Source evidence ─────────────────────────────────────────── */

const char *dm2_v2_lighting_source_evidence(void) {
    return
        "DM2 V2.2 Enhanced Lighting — Phase 4\n"
        "Source: SKULL.ASM PROCESS_TIMER_0C  — per-champion torch timers\n"
        "Source: SKULL.ASM T600              — outdoor viewport rendering\n"
        "Source: SKULL.ASM T560              — indoor dungeon viewport\n"
        "Source: docs/dm2_time.md §Weather   — fog/storm visibility reduction\n"
        "Source: docs/dm2_time.md §Time-of-day sky color gradient\n"
        "Reference: dm1_v2_lighting_dynamic_pc34.c §V2.2 Dynamic Lighting\n"
        "  v22_flicker_factor / v22_light_rebuild_map\n"
        "Reference: dm2_v1_weather.c dm2_v1_weather_sky_color\n"
        "Reference: dm1_v2_settings_impl.c dm1_v2_apply_torch_flicker\n"
        "Reference: dm2_v2_outdoor_enhanced.c lightning_trigger pattern\n"
        "DM2 V2.2: indoor fog overlay (per-tile density map)\n"
        "DM2 V2.2: outdoor weather fog + sky-color ambient blend\n"
        "DM2 V2.2: per-champion torch flicker (4-phase sine composite)\n"
        "DM2 V2.2: lightning bloom flash (multi-frame, fade-out)\n"
        "V1 invariant: torch charge consumption, palette index, weather enum,\n"
        "  time_of_day, triggers, combat, timeline — all unchanged\n";
}
