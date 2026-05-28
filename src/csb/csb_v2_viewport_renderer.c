
#include "csb_v2_viewport_renderer.h"
#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_smooth_movement.h"
#include <string.h>

static uint32_t g_csb_v2_last_render_ms;

void csb_v2_viewport_init(CSB_V2_ViewportState *s, int scale) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->scale_factor = (scale == 4) ? 4 : 2;
    s->epx_enabled = 1;
    v2_anim_clock_init(&s->clock);
    csb_v2_smooth_init();
    csb_v2_light_init();
    csb_v2_chaos_init();
    g_csb_v2_last_render_ms = 0;
}

void csb_v2_viewport_v1_tick(CSB_V2_ViewportState *s, uint32_t now_ms) {
    if (s) v2_anim_clock_v1_tick(&s->clock, now_ms);
}

void csb_v2_viewport_render_frame(CSB_V2_ViewportState *s, uint32_t now_ms) {
    float dtSeconds;
    if (!s) return;
    v2_anim_clock_render_frame(&s->clock, now_ms);
    /* Advance smooth movement animations each display frame */
    csb_v2_smooth_update_from_clock(&s->clock);
    dtSeconds = 0.0f;
    if (g_csb_v2_last_render_ms != 0 && now_ms >= g_csb_v2_last_render_ms) {
        dtSeconds = (float)(now_ms - g_csb_v2_last_render_ms) / 1000.0f;
    }
    g_csb_v2_last_render_ms = now_ms;
    csb_v2_light_tick(dtSeconds);
    csb_v2_chaos_tick(dtSeconds);
}

float csb_v2_viewport_sub_tick(const CSB_V2_ViewportState *s) {
    return s ? v2_anim_clock_sub_tick(&s->clock) : 0.0f;
}

void csb_v2_viewport_set_dungeon_light(CSB_V2_ViewportState *s, int dungeonLevel) {
    (void)s;
    csb_v2_light_set_dungeon_level(dungeonLevel);
}

void csb_v2_viewport_set_ambient_light(float ambient) {
    csb_v2_light_set_ambient(ambient);
}

int csb_v2_viewport_add_torch(float x,
                              float y,
                              uint8_t intensity,
                              uint8_t r,
                              uint8_t g,
                              uint8_t b,
                              int flicker)
{
    return csb_v2_light_add_source(x, y, 4.0f, intensity, r, g, b, flicker);
}

void csb_v2_viewport_remove_torch(int lightIndex) {
    csb_v2_light_remove_source(lightIndex);
}

void csb_v2_viewport_compute_light_map(void) {
    csb_v2_light_compute_map();
}

void csb_v2_viewport_get_tile_light(int tileX,
                                    int tileY,
                                    uint8_t *r,
                                    uint8_t *g,
                                    uint8_t *b)
{
    csb_v2_light_get_tile(tileX, tileY, r, g, b);
}

void csb_v2_viewport_trigger_dsa_light_event(CSB_V2_LightEventType type,
                                             float durationSeconds,
                                             float intensity)
{
    csb_v2_light_event_trigger(type, durationSeconds, intensity);
}

CSB_V2_LightEventType csb_v2_viewport_light_event_type(void) {
    return csb_v2_light_event_current_type();
}

void csb_v2_viewport_advance_chaos(float dtSeconds) {
    csb_v2_chaos_tick(dtSeconds);
}

int csb_v2_viewport_chaos_active_count(void) {
    return csb_v2_chaos_active_count();
}

void csb_v2_viewport_get_chaos_overlay(float *outR,
                                       float *outG,
                                       float *outB,
                                       float *outAlpha)
{
    csb_v2_chaos_render_overlay(outR, outG, outB, outAlpha);
}

const char *csb_v2_viewport_source_evidence(void) {
    return "CSB V2: shared DM1 V2.1 EPX pipeline + CSB custom backgrounds\n"
           "V2_AnimClock: V1 tick rate preserved, sub-tick interpolation\n"
           "CSBWin/Viewport.cpp: base rendering, CSBWin/Graphics.cpp: assets\n"
           "ReDMCSB PANEL.C:367-428 remains the dungeon light baseline\n";
}
