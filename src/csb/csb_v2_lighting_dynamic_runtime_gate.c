/*
 * Product CSB V2 lighting boundary.
 *
 * ReDMCSB PANEL.C F0380--F0382 selects a palette from source-owned torch
 * charges and magical light. It does not specify RGB point lights, a
 * tile-space falloff, sinusoidal flicker, or DSA light pulses. CSBWin's
 * renderer likewise composes indexed source graphics. Preserve the documented
 * palette-light table, but do not manufacture a second lighting image until
 * an authenticated source-art transaction binds every visible pixel.
 *
 * `csb_v2_lighting_dynamic.c` remains available only to contract probes.
 */

#include "csb_v2_lighting_dynamic.h"

#include <string.h>

const uint8_t k_csb_v2_source_palette_light_amount_floor[6] = {
    99, 75, 50, 25, 1, 0
};

void csb_v2_light_init(void) {}

CSB_V2_SourcePaletteLighting csb_v2_light_build_source_palette_lighting(
    int sourcePaletteIndex, bool enhancedEffectsEnabled)
{
    CSB_V2_SourcePaletteLighting plan;
    int index = sourcePaletteIndex;
    (void)enhancedEffectsEnabled;
    memset(&plan, 0, sizeof(plan));
    if (index < 0 || index >= 6) {
        index = 5;
        plan.deterministicFallback = true;
    }
    plan.sourcePaletteIndex = (uint8_t)index;
    plan.sourceLightAmountFloor =
        k_csb_v2_source_palette_light_amount_floor[index];
    plan.darknessPercent = (uint8_t)(100u - plan.sourceLightAmountFloor);
    plan.shadowAlpha =
        (uint8_t)(((unsigned int)plan.darknessPercent * 192u + 50u) / 100u);
    /* A source palette is not permission to add host lighting. */
    plan.enhancedEffectsEnabled = false;
    return plan;
}

int csb_v2_light_add_source(float x, float y, float radius,
                            uint8_t intensity, uint8_t r, uint8_t g,
                            uint8_t b, int flicker)
{
    (void)x; (void)y; (void)radius; (void)intensity;
    (void)r; (void)g; (void)b; (void)flicker;
    return -1;
}

void csb_v2_light_remove_source(int index) { (void)index; }
void csb_v2_light_compute_map(void) {}

void csb_v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
{
    (void)x; (void)y;
    if (r) *r = 0u;
    if (g) *g = 0u;
    if (b) *b = 0u;
}

void csb_v2_light_update_flicker(float dtSeconds) { (void)dtSeconds; }
void csb_v2_light_tick(float dtSeconds) { (void)dtSeconds; }
void csb_v2_light_set_ambient(float level) { (void)level; }
float csb_v2_light_get_ambient(void) { return 0.0f; }
void csb_v2_light_set_dungeon_level(int level) { (void)level; }
int csb_v2_light_get_dungeon_level(void) { return 0; }

void csb_v2_light_event_trigger(CSB_V2_LightEventType type,
                                float durationSeconds, float intensity)
{
    (void)type; (void)durationSeconds; (void)intensity;
}

void csb_v2_light_event_tick(float dtSeconds) { (void)dtSeconds; }
int csb_v2_light_event_is_active(void) { return 0; }

CSB_V2_LightEventType csb_v2_light_event_current_type(void)
{
    return CSB_V2_LIGHT_EVENT_NORMAL;
}

const char *csb_v2_light_source_evidence(void)
{
    return "ReDMCSB PANEL.C F0380-F0382 and CSBWin Graphics.cpp own CSB "
           "light presentation; no host RGB map, flicker, or DSA pulse admitted.";
}
