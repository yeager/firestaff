#include "dm1_v2_lighting_dynamic_pc34.h"

/* ReDMCSB DATA.C:360 / PANEL.C:419-423. PC34 selects one of these six
 * palettes from the source-owned F0337 light total; it has no RGB light map. */
static const uint8_t k_palette_light_amount[6] = { 99, 75, 50, 25, 1, 0 };

M11_V2_SourcePaletteLighting v2_light_build_source_palette_lighting(
    int source_palette_index,
    bool enhanced_effects_enabled)
{
    M11_V2_SourcePaletteLighting plan;
    int index = source_palette_index;
    plan.deterministic_fallback = false;
    if (index < 0 || index >= 6) {
        index = 5;
        plan.deterministic_fallback = true;
    }
    plan.source_palette_index = (uint8_t)index;
    plan.source_light_amount_floor = k_palette_light_amount[index];
    plan.darkness_percent = (uint8_t)(100 - plan.source_light_amount_floor);
    plan.shadow_alpha = (uint8_t)(
        ((unsigned int)plan.darkness_percent * 192u + 50u) / 100u);
    (void)enhanced_effects_enabled;
    /* V2 local lighting is not a PC34 presentation route. */
    plan.enhanced_effects_enabled = false;
    return plan;
}

/* Generic RGB sources, propagation and flicker are intentionally inert. */
void v2_light_init(void) {}
void v2_light_clear_sources(void) {}
int v2_light_source_count(void) { return 0; }
int v2_light_add_source(float x, float y, float radius, uint8_t intensity,
                        uint8_t r, uint8_t g, uint8_t b) {
    (void)x;
    (void)y;
    (void)radius;
    (void)intensity;
    (void)r;
    (void)g;
    (void)b;
    return -1;
}
void v2_light_remove_source(int idx) { (void)idx; }
void v2_light_compute_map(void) {}
void v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    (void)x;
    (void)y;
    if (r) *r = 0;
    if (g) *g = 0;
    if (b) *b = 0;
}
void v2_light_update_flicker(float dt) { (void)dt; }
void v2_light_set_deterministic_seed(uint32_t seed) { (void)seed; }
void v2_light_tick(float dt) { (void)dt; }

int v22_light_add(int x, int y, float intensity, float radius,
                  uint32_t color, int flicker) {
    (void)x;
    (void)y;
    (void)intensity;
    (void)radius;
    (void)color;
    (void)flicker;
    return -1;
}
void v22_light_remove(int id) { (void)id; }
void v22_light_rebuild_map(void) {}
float v22_light_get(int x, int y) {
    (void)x;
    (void)y;
    return 0.0f;
}
void v22_light_set_ambient(float a) { (void)a; }
void v22_light_clear(void) {}
int v22_light_source_count(void) { return 0; }
void v22_light_tick(float dt) { (void)dt; }
