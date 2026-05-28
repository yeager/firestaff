
#include "csb_v2_viewport_renderer.h"
#include "csb_v2_smooth_movement.h"
#include <string.h>

void csb_v2_viewport_init(CSB_V2_ViewportState *s, int scale) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->scale_factor = (scale == 4) ? 4 : 2;
    s->epx_enabled = 1;
    v2_anim_clock_init(&s->clock);
    csb_v2_smooth_init();
}

void csb_v2_viewport_v1_tick(CSB_V2_ViewportState *s, uint32_t now_ms) {
    if (s) v2_anim_clock_v1_tick(&s->clock, now_ms);
}

void csb_v2_viewport_render_frame(CSB_V2_ViewportState *s, uint32_t now_ms) {
    if (!s) return;
    v2_anim_clock_render_frame(&s->clock, now_ms);
    /* Advance smooth movement animations each display frame */
    csb_v2_smooth_update_from_clock(&s->clock);
}

float csb_v2_viewport_sub_tick(const CSB_V2_ViewportState *s) {
    return s ? v2_anim_clock_sub_tick(&s->clock) : 0.0f;
}

const char *csb_v2_viewport_source_evidence(void) {
    return "CSB V2: shared DM1 V2.1 EPX pipeline + CSB custom backgrounds\n"
           "V2_AnimClock: V1 tick rate preserved, sub-tick interpolation\n"
           "CSBWin/Viewport.cpp: base rendering, CSBWin/Graphics.cpp: assets\n";
}

