#include "csb_v2_lighting_dynamic.h"

#include <math.h>
#include <string.h>

/* CSB V2 Phase 4 lighting.
 *
 * Source-lock anchors:
 * - ReDMCSB PANEL.C:367-428 selects G0304_i_DungeonViewPaletteIndex.
 * - ReDMCSB PANEL.C:370-405 reads torch charges from champion hands.
 * - ReDMCSB PANEL.C:417 adds G0407_s_Party.MagicalLightAmount.
 * - ReDMCSB DATA.C:359-360 defines G0040_ai_Graphic562_PaletteIndexToLightAmount.
 *
 * This is presentation-only. V1 torch charge, magical light, palette choice,
 * collision, sensors, and DSA execution remain owned by the source-locked core.
 */

const uint8_t k_csb_v2_source_palette_light_amount_floor[6] = {
    99, 75, 50, 25, 1, 0
};

#define CSB_V2_LIGHT_MAX_EVENTS 8

typedef struct {
    CSB_V2_LightEventType type;
    float progress;
    float durationSeconds;
    float intensity;
    int active;
} CSB_V2_LightEvent;

static CSB_V2_LightSource g_sources[CSB_V2_LIGHT_MAX_SOURCES];
static int g_sourceCount;
static CSB_V2_LightMap g_lightMap;
static float g_ambient;
static int g_dungeonLevel;
static CSB_V2_LightEvent g_events[CSB_V2_LIGHT_MAX_EVENTS];
static int g_eventCount;
static uint32_t g_lightTick;

static uint8_t clamp_u8(float value) {
    if (value <= 0.0f) {
        return 0;
    }
    if (value >= 255.0f) {
        return 255;
    }
    return (uint8_t)value;
}

static float clamp01(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static float deterministic_flicker(uint32_t tick, uint32_t seed) {
    uint32_t hash = tick * 7919u + seed * 104729u;
    float f = (float)(hash & 0xFFu) / 255.0f;
    return 0.65f + (0.35f * f);
}

static float event_tile_factor(int tileX, int tileY) {
    float factor = 1.0f;
    int i;

    for (i = 0; i < g_eventCount; ++i) {
        CSB_V2_LightEvent *event = &g_events[i];
        float p;
        float intensity;

        if (!event->active) {
            continue;
        }

        p = clamp01(event->progress);
        intensity = clamp01(event->intensity);
        switch (event->type) {
            case CSB_V2_LIGHT_EVENT_FLICKER_TORCH: {
                float f = deterministic_flicker(
                    g_lightTick,
                    (uint32_t)(tileX * 7919 + tileY * 104729));
                factor *= 1.0f - (0.30f * f * intensity * (1.0f - p));
                break;
            }
            case CSB_V2_LIGHT_EVENT_DARKNESS_BURST: {
                float darkness = sinf(p * 3.14159265f) * intensity;
                factor *= 1.0f - (0.50f * darkness);
                break;
            }
            case CSB_V2_LIGHT_EVENT_MAGICAL_PULSE: {
                float pulse = 0.5f + (0.5f * sinf(p * 6.28318530f));
                factor += 0.20f * pulse * intensity;
                break;
            }
            case CSB_V2_LIGHT_EVENT_CHAOS_SURGE: {
                float f0 = deterministic_flicker(
                    g_lightTick,
                    (uint32_t)(tileX * 7919 + tileY * 104729));
                float f1 = deterministic_flicker(
                    g_lightTick + 1u,
                    (uint32_t)(tileY * 7919 + tileX * 104729));
                factor *= 1.0f - (0.60f * ((f0 + f1) * 0.5f) * intensity);
                break;
            }
            default:
                break;
        }
    }

    return clamp01(factor);
}

void csb_v2_light_init(void) {
    memset(g_sources, 0, sizeof(g_sources));
    memset(&g_lightMap, 0, sizeof(g_lightMap));
    memset(g_events, 0, sizeof(g_events));
    g_sourceCount = 0;
    g_eventCount = 0;
    g_ambient = 0.0f;
    g_dungeonLevel = 0;
    g_lightTick = 0;
}

CSB_V2_SourcePaletteLighting csb_v2_light_build_source_palette_lighting(
    int sourcePaletteIndex,
    bool enhancedEffectsEnabled)
{
    CSB_V2_SourcePaletteLighting plan;
    int resolved = sourcePaletteIndex;

    memset(&plan, 0, sizeof(plan));
    if (resolved < 0 || resolved >= 6) {
        resolved = 5;
        plan.deterministicFallback = true;
    }

    plan.sourcePaletteIndex = (uint8_t)resolved;
    plan.sourceLightAmountFloor =
        k_csb_v2_source_palette_light_amount_floor[resolved];
    plan.darknessPercent = (uint8_t)(100 - plan.sourceLightAmountFloor);
    plan.shadowAlpha =
        (uint8_t)(((unsigned int)plan.darknessPercent * 192u + 50u) / 100u);
    plan.enhancedEffectsEnabled =
        enhancedEffectsEnabled && !plan.deterministicFallback;
    return plan;
}

int csb_v2_light_add_source(float x,
                            float y,
                            float radius,
                            uint8_t intensity,
                            uint8_t r,
                            uint8_t g,
                            uint8_t b,
                            int flicker)
{
    CSB_V2_LightSource *source;
    int index;

    if (g_sourceCount >= CSB_V2_LIGHT_MAX_SOURCES || intensity == 0 ||
        radius <= 0.0f) {
        return -1;
    }

    index = g_sourceCount++;
    source = &g_sources[index];
    source->x = x;
    source->y = y;
    source->radius = radius;
    source->baseIntensity = intensity;
    source->intensity = intensity;
    source->colorR = r;
    source->colorG = g;
    source->colorB = b;
    source->flickerPhase = (float)(index + 1) * 0.381966f;
    source->flicker = flicker ? 1 : 0;
    source->active = 1;
    return index;
}

void csb_v2_light_remove_source(int index) {
    if (index < 0 || index >= g_sourceCount) {
        return;
    }
    g_sources[index] = g_sources[g_sourceCount - 1];
    memset(&g_sources[g_sourceCount - 1], 0, sizeof(g_sources[g_sourceCount - 1]));
    --g_sourceCount;
}

void csb_v2_light_compute_map(void) {
    int x;
    int y;
    int i;
    uint8_t ambient = clamp_u8(g_ambient * 255.0f);
    uint8_t floor = k_csb_v2_source_palette_light_amount_floor[g_dungeonLevel];
    unsigned int darkness = (floor >= 99) ? 0u : ((100u - floor) * 128u) / 100u;

    ++g_lightTick;
    for (y = 0; y < CSB_V2_LIGHT_MAP_SIZE; ++y) {
        for (x = 0; x < CSB_V2_LIGHT_MAP_SIZE; ++x) {
            g_lightMap.r[y][x] = ambient;
            g_lightMap.g[y][x] = ambient;
            g_lightMap.b[y][x] = ambient;
        }
    }

    for (i = 0; i < g_sourceCount; ++i) {
        CSB_V2_LightSource *source = &g_sources[i];
        float radius2;
        float baseIntensity;
        int minX;
        int maxX;
        int minY;
        int maxY;

        if (!source->active || source->radius <= 0.0f || source->intensity == 0) {
            continue;
        }

        radius2 = source->radius * source->radius;
        baseIntensity = (float)source->intensity / 255.0f;
        minX = (int)(source->x - source->radius);
        maxX = (int)(source->x + source->radius);
        minY = (int)(source->y - source->radius);
        maxY = (int)(source->y + source->radius);
        if (minX < 0) minX = 0;
        if (minY < 0) minY = 0;
        if (maxX >= CSB_V2_LIGHT_MAP_SIZE) maxX = CSB_V2_LIGHT_MAP_SIZE - 1;
        if (maxY >= CSB_V2_LIGHT_MAP_SIZE) maxY = CSB_V2_LIGHT_MAP_SIZE - 1;

        for (y = minY; y <= maxY; ++y) {
            for (x = minX; x <= maxX; ++x) {
                float dx = (float)x - source->x;
                float dy = (float)y - source->y;
                float dist2 = dx * dx + dy * dy;
                float falloff;
                float contribution;

                if (dist2 >= radius2) {
                    continue;
                }
                falloff = 1.0f - (dist2 / radius2);
                falloff *= falloff;
                contribution = baseIntensity * falloff;
                g_lightMap.r[y][x] = clamp_u8(
                    (float)g_lightMap.r[y][x] + (float)source->colorR * contribution);
                g_lightMap.g[y][x] = clamp_u8(
                    (float)g_lightMap.g[y][x] + (float)source->colorG * contribution);
                g_lightMap.b[y][x] = clamp_u8(
                    (float)g_lightMap.b[y][x] + (float)source->colorB * contribution);
            }
        }
    }

    for (y = 0; y < CSB_V2_LIGHT_MAP_SIZE; ++y) {
        for (x = 0; x < CSB_V2_LIGHT_MAP_SIZE; ++x) {
            float factor = event_tile_factor(x, y);
            unsigned int shade = 255u - darkness;
            g_lightMap.r[y][x] = clamp_u8(
                ((float)((unsigned int)g_lightMap.r[y][x] * shade) / 255.0f) * factor);
            g_lightMap.g[y][x] = clamp_u8(
                ((float)((unsigned int)g_lightMap.g[y][x] * shade) / 255.0f) * factor);
            g_lightMap.b[y][x] = clamp_u8(
                ((float)((unsigned int)g_lightMap.b[y][x] * shade) / 255.0f) * factor);
        }
    }
}

void csb_v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (!r || !g || !b) {
        return;
    }
    if (x < 0 || x >= CSB_V2_LIGHT_MAP_SIZE ||
        y < 0 || y >= CSB_V2_LIGHT_MAP_SIZE) {
        *r = 0;
        *g = 0;
        *b = 0;
        return;
    }
    *r = g_lightMap.r[y][x];
    *g = g_lightMap.g[y][x];
    *b = g_lightMap.b[y][x];
}

void csb_v2_light_update_flicker(float dtSeconds) {
    int i;
    if (dtSeconds < 0.0f) {
        dtSeconds = 0.0f;
    }
    for (i = 0; i < g_sourceCount; ++i) {
        CSB_V2_LightSource *source = &g_sources[i];
        if (!source->active) {
            continue;
        }
        if (!source->flicker) {
            source->intensity = source->baseIntensity;
            continue;
        }
        source->flickerPhase += dtSeconds * 7.0f;
        source->intensity = clamp_u8(
            (float)source->baseIntensity *
            (0.85f + (0.15f * sinf(source->flickerPhase))));
    }
}

void csb_v2_light_tick(float dtSeconds) {
    csb_v2_light_update_flicker(dtSeconds);
    csb_v2_light_event_tick(dtSeconds);
}

void csb_v2_light_set_ambient(float level) {
    g_ambient = clamp01(level);
}

float csb_v2_light_get_ambient(void) {
    return g_ambient;
}

void csb_v2_light_set_dungeon_level(int level) {
    if (level < 0) {
        g_dungeonLevel = 0;
    } else if (level > 5) {
        g_dungeonLevel = 5;
    } else {
        g_dungeonLevel = level;
    }
}

int csb_v2_light_get_dungeon_level(void) {
    return g_dungeonLevel;
}

void csb_v2_light_event_trigger(CSB_V2_LightEventType type,
                                float durationSeconds,
                                float intensity)
{
    CSB_V2_LightEvent *event;
    int i;

    if (type <= CSB_V2_LIGHT_EVENT_NORMAL || type >= CSB_V2_LIGHT_EVENT_COUNT) {
        return;
    }
    if (g_eventCount >= CSB_V2_LIGHT_MAX_EVENTS) {
        for (i = 1; i < CSB_V2_LIGHT_MAX_EVENTS; ++i) {
            g_events[i - 1] = g_events[i];
        }
        g_eventCount = CSB_V2_LIGHT_MAX_EVENTS - 1;
    }

    event = &g_events[g_eventCount++];
    event->type = type;
    event->progress = 0.0f;
    event->durationSeconds = durationSeconds > 0.0f ? durationSeconds : 1.0f;
    event->intensity = intensity > 0.0f ? intensity : 1.0f;
    event->active = 1;
}

void csb_v2_light_event_tick(float dtSeconds) {
    int i;
    int write = 0;

    if (dtSeconds < 0.0f) {
        dtSeconds = 0.0f;
    }
    for (i = 0; i < g_eventCount; ++i) {
        CSB_V2_LightEvent *event = &g_events[i];
        event->progress += dtSeconds / event->durationSeconds;
        if (event->progress >= 1.0f) {
            event->active = 0;
            continue;
        }
        if (write != i) {
            g_events[write] = *event;
        }
        ++write;
    }
    g_eventCount = write;
}

int csb_v2_light_event_is_active(void) {
    int i;
    for (i = 0; i < g_eventCount; ++i) {
        if (g_events[i].active) {
            return 1;
        }
    }
    return 0;
}

CSB_V2_LightEventType csb_v2_light_event_current_type(void) {
    int i;
    for (i = 0; i < g_eventCount; ++i) {
        if (g_events[i].active) {
            return g_events[i].type;
        }
    }
    return CSB_V2_LIGHT_EVENT_NORMAL;
}

const char *csb_v2_light_source_evidence(void) {
    return "CSB V2 lighting: presentation-only local light map\n"
           "ReDMCSB PANEL.C:367-428 selects G0304_i_DungeonViewPaletteIndex\n"
           "ReDMCSB PANEL.C:370-405 reads torch light power from champion hands\n"
           "ReDMCSB PANEL.C:417 adds G0407_s_Party.MagicalLightAmount\n"
           "ReDMCSB DATA.C:359-360 maps palette index to light amount\n";
}
