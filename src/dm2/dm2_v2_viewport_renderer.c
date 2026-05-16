
#include "dm2_v2_viewport_renderer.h"
#include <string.h>

void dm2_v2_viewport_init(DM2_V2_ViewportState *s, int scale) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->scale_factor = (scale == 4) ? 4 : 2;
    s->epx_enabled = 1;
    v2_anim_clock_init(&s->clock);
}

void dm2_v2_viewport_set_outdoor(DM2_V2_ViewportState *s, int outdoor) {
    if (!s) return;
    if (outdoor != s->is_outdoor) {
        /* V2.2: smooth blend between indoor/outdoor rendering */
        v2_anim_start(&s->indoor_outdoor_blend,
            (float)s->is_outdoor, (float)outdoor,
            V1_TICK_MS * 3, V2_EASE_IN_OUT_QUAD);
        s->is_outdoor = outdoor;
    }
}

void dm2_v2_viewport_v1_tick(DM2_V2_ViewportState *s, uint32_t now_ms) {
    if (!s) return;
    v2_anim_clock_v1_tick(&s->clock, now_ms);
    /* Outdoor parallax: scroll sky slowly */
    if (s->is_outdoor) s->sky_scroll_offset += 0.1f;
}

void dm2_v2_viewport_render_frame(DM2_V2_ViewportState *s, uint32_t now_ms) {
    if (!s) return;
    v2_anim_clock_render_frame(&s->clock, now_ms);
    v2_anim_update(&s->indoor_outdoor_blend, s->clock.dt_ms);
}

const char *dm2_v2_viewport_source_evidence(void) {
    return "DM2 V2.1: EPX upscale for indoor, sky gradient for outdoor\n"
           "DM2 V2.2: smooth indoor/outdoor blend transition\n"
           "V2_AnimClock: V1 tick rate preserved\n";
}

