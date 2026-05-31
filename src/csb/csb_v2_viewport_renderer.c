#include "csb_v2_viewport_renderer.h"
#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_smooth_movement.h"
#include <string.h>
#include <stdio.h>

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

    /* Compute elapsed_ms = wall-clock time since last render_frame call.
     * This is how a real game loop works: v1_tick fires every ~55ms
     * (V1 cadence) while render_frame fires every ~16ms (display rate).
     * After v1_tick(55000), the next render_frame(55016) has elapsed_ms=16ms
     * and animations advance by 16ms toward completion.
     *
     * In headless tests where v1_tick and render_frame are called at the
     * same timestamp, elapsed_ms = 0 and animations do not advance on that
     * call.  The next render_frame (at a later timestamp) will have
     * non-zero elapsed and animations complete normally.
     *
     * We also update clock.dt_ms for sub_tick accuracy.
     *
     * Source: dm2_v2_viewport_renderer.c dm2_v2_viewport_render_frame
     *   pattern — v2_anim_clock_render_frame + dm2_v2_smooth_tick
     *   with clock.dt_ms = wall-clock elapsed between render frames */
    const uint32_t prev_render = g_csb_v2_last_render_ms;
    const uint32_t elapsed_ms = (prev_render != 0 && now_ms > prev_render)
        ? (now_ms - prev_render) : 0;
    fprintf(stderr, "  [render] now=%u prev=%u elapsed=%u\n", now_ms, prev_render, elapsed_ms);
    g_csb_v2_last_render_ms = now_ms;
    fprintf(stderr, "  [render] AFTER ASSIGN: last_render_ms=%u\n", g_csb_v2_last_render_ms);

    /* Update V2_AnimClock dt_ms so sub_tick is accurate */
    s->clock.dt_ms = (float)elapsed_ms;

    /* Advance smooth movement animations by elapsed_ms.
     * In a real game loop this is ~16ms per frame; in headless tests
     * it may be 0 (same-timestamp test) or larger (back-to-back frames). */
    {
        V2_AnimClock fake_clock = s->clock;
        fake_clock.dt_ms = (float)elapsed_ms;
        csb_v2_smooth_update_from_clock(&fake_clock);
    }

    dtSeconds = 0.0f;
    if (prev_render != 0 && now_ms >= prev_render) {
        dtSeconds = (float)(now_ms - prev_render) / 1000.0f;
    }
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