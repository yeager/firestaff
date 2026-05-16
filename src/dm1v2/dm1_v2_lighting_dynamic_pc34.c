#include "dm1_v2_lighting_dynamic_pc34.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/*
 * Source-lock: ReDMCSB PANEL.C:F0337_INVENTORY_SetDungeonViewPalette_...
 * computes the canonical dungeon light from torch charges plus magical light,
 * then selects G0304_i_DungeonViewPaletteIndex.  DM1 V2 keeps that gameplay
 * lighting semantic in the V1-compatible core; this module is a presentation-only
 * additive light map/fog overlay for the modern viewport.
 */

static M11_V2_LightSource g_sources[M11_V2_LIGHT_MAX_SOURCES];
static int g_source_count = 0;
static M11_V2_LightMap g_light_map;

void v2_light_init(void) {
    memset(g_light_map.r, 0, sizeof(g_light_map.r));
    memset(g_light_map.g, 0, sizeof(g_light_map.g));
    memset(g_light_map.b, 0, sizeof(g_light_map.b));
    g_source_count = 0;
}

int v2_light_add_source(float x, float y, float radius, uint8_t intensity,
                         uint8_t r, uint8_t g, uint8_t b) {
    int idx;
    if (g_source_count >= M11_V2_LIGHT_MAX_SOURCES) return -1;
    idx = g_source_count++;
    g_sources[idx].x = x;
    g_sources[idx].y = y;
    g_sources[idx].radius = radius;
    g_sources[idx].intensity = intensity;
    g_sources[idx].color_r = r;
    g_sources[idx].color_g = g;
    g_sources[idx].color_b = b;
    g_sources[idx].flicker_phase = (float)(rand() % 1000) / 1000.0f;
    return idx;
}

void v2_light_remove_source(int idx) {
    if (idx < 0 || idx >= g_source_count) return;
    g_sources[idx] = g_sources[g_source_count - 1];
    g_source_count--;
}

void v2_light_compute_map(void) {
    int x, y, i;
    for (y = 0; y < M11_V2_LIGHT_MAP_SIZE; y++) {
        for (x = 0; x < M11_V2_LIGHT_MAP_SIZE; x++) {
            float total_r = 0.0f, total_g = 0.0f, total_b = 0.0f;
            for (i = 0; i < g_source_count; i++) {
                float dx = (float)x - g_sources[i].x;
                float dy = (float)y - g_sources[i].y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < g_sources[i].radius) {
                    float falloff = 1.0f - (dist / g_sources[i].radius);
                    float intensity_f;
                    falloff = falloff * falloff;
                    intensity_f = (float)g_sources[i].intensity / 255.0f;
                    total_r += (float)g_sources[i].color_r * intensity_f * falloff;
                    total_g += (float)g_sources[i].color_g * intensity_f * falloff;
                    total_b += (float)g_sources[i].color_b * intensity_f * falloff;
                }
            }
            g_light_map.r[y][x] = (uint8_t)fminf(255.0f, total_r);
            g_light_map.g[y][x] = (uint8_t)fminf(255.0f, total_g);
            g_light_map.b[y][x] = (uint8_t)fminf(255.0f, total_b);
        }
    }
}

void v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (x < 0 || x >= M11_V2_LIGHT_MAP_SIZE || y < 0 || y >= M11_V2_LIGHT_MAP_SIZE) {
        *r = *g = *b = 0;
        return;
    }
    *r = g_light_map.r[y][x];
    *g = g_light_map.g[y][x];
    *b = g_light_map.b[y][x];
}

void v2_light_update_flicker(float dt) {
    int i;
    for (i = 0; i < g_source_count; i++) {
        g_sources[i].flicker_phase += dt * 2.0f;
        g_sources[i].intensity = (uint8_t)((float)g_sources[i].intensity *
            (0.8f + 0.2f * sinf(g_sources[i].flicker_phase)));
    }
}

/* V2.2 Dynamic Lighting — per-tile light levels + torch flicker
 *
 * V1 uses a flat light level per dungeon level.
 * V2.2 adds per-tile light propagation from:
 *   - Torches (held or wall-mounted)
 *   - Spells (light spell, fireball glow)
 *   - Creatures (some glow)
 *   - Ambient (per-level base)
 *
 * Light propagates 4 tiles with inverse-square falloff.
 * Source: ReDMCSB CHAMPION.C F0315_GetScentOrdinal uses similar radius.
 */

#define V22_LIGHT_MAP_SIZE 32
#define V22_MAX_LIGHT_SOURCES 32

typedef struct {
    int x, y;
    float intensity; /* 0..1 */
    float radius;    /* tiles */
    uint32_t color;  /* ARGB */
    int flicker;     /* 0=steady, 1=torch flicker */
} V22_LightSource;

static V22_LightSource g_lights[V22_MAX_LIGHT_SOURCES];
static int g_light_count = 0;
static float g_v22_light_map[V22_LIGHT_MAP_SIZE][V22_LIGHT_MAP_SIZE];
static float g_ambient = 0.1f;
static uint32_t g_light_tick = 0;

int v22_light_add(int x, int y, float intensity, float radius,
    uint32_t color, int flicker)
{
    if (g_light_count >= V22_MAX_LIGHT_SOURCES) return -1;
    g_lights[g_light_count].x = x;
    g_lights[g_light_count].y = y;
    g_lights[g_light_count].intensity = intensity;
    g_lights[g_light_count].radius = radius;
    g_lights[g_light_count].color = color;
    g_lights[g_light_count].flicker = flicker;
    return g_light_count++;
}

void v22_light_remove(int id) {
    if (id >= 0 && id < g_light_count) {
        g_lights[id] = g_lights[--g_light_count];
    }
}

static float v22_flicker_factor(int id) {
    if (!g_lights[id].flicker) return 1.0f;
    /* Simple hash-based flicker per source */
    uint32_t h = g_light_tick * 7919 + (uint32_t)id * 104729;
    float f = (float)(h & 0xFF) / 255.0f;
    return 0.7f + 0.3f * f; /* flicker between 70-100% */
}

void v22_light_rebuild_map(void) {
    int x, y, i;
    g_light_tick++;
    /* Start with ambient */
    for (y = 0; y < V22_LIGHT_MAP_SIZE; y++)
        for (x = 0; x < V22_LIGHT_MAP_SIZE; x++)
            g_v22_light_map[y][x] = g_ambient;

    /* Add each light source with inverse-square falloff */
    for (i = 0; i < g_light_count; i++) {
        float lx = (float)g_lights[i].x;
        float ly = (float)g_lights[i].y;
        float r = g_lights[i].radius;
        float inten = g_lights[i].intensity * v22_flicker_factor(i);
        int minX = (int)(lx - r); if (minX < 0) minX = 0;
        int minY = (int)(ly - r); if (minY < 0) minY = 0;
        int maxX = (int)(lx + r); if (maxX >= V22_LIGHT_MAP_SIZE) maxX = V22_LIGHT_MAP_SIZE - 1;
        int maxY = (int)(ly + r); if (maxY >= V22_LIGHT_MAP_SIZE) maxY = V22_LIGHT_MAP_SIZE - 1;

        for (y = minY; y <= maxY; y++) {
            for (x = minX; x <= maxX; x++) {
                float dx = (float)x - lx;
                float dy = (float)y - ly;
                float dist2 = dx * dx + dy * dy;
                if (dist2 < 1.0f) dist2 = 1.0f;
                float contrib = inten / dist2;
                g_v22_light_map[y][x] += contrib;
                if (g_v22_light_map[y][x] > 1.0f) g_v22_light_map[y][x] = 1.0f;
            }
        }
    }
}

float v22_light_get(int x, int y) {
    if (x < 0 || x >= V22_LIGHT_MAP_SIZE || y < 0 || y >= V22_LIGHT_MAP_SIZE)
        return g_ambient;
    return g_v22_light_map[y][x];
}

void v22_light_set_ambient(float a) { g_ambient = a < 0 ? 0 : (a > 1 ? 1 : a); }
void v22_light_clear(void) { g_light_count = 0; }

