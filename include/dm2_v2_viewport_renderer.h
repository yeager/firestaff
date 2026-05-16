
#ifndef FIRESTAFF_DM2_V2_VIEWPORT_RENDERER_H
#define FIRESTAFF_DM2_V2_VIEWPORT_RENDERER_H
#include <stdint.h>
#include "dm1_v2_anim_timing.h"

/* DM2 V2 Viewport — upscaled indoor + outdoor rendering.
 * Indoor: shares DM1 V2.1 EPX pipeline.
 * Outdoor: sky gradient, tree parallax, building perspective — all
 * rendered at V1 resolution then upscaled.
 * V2.2: smooth camera transitions between indoor/outdoor. */

typedef struct {
    int is_outdoor;
    int scale_factor;
    int epx_enabled;
    float sky_scroll_offset;
    float parallax_offset;
    V2_AnimClock clock;
    V2_Anim indoor_outdoor_blend; /* smooth transition */
} DM2_V2_ViewportState;

void dm2_v2_viewport_init(DM2_V2_ViewportState *s, int scale);
void dm2_v2_viewport_set_outdoor(DM2_V2_ViewportState *s, int outdoor);
void dm2_v2_viewport_v1_tick(DM2_V2_ViewportState *s, uint32_t now_ms);
void dm2_v2_viewport_render_frame(DM2_V2_ViewportState *s, uint32_t now_ms);
const char *dm2_v2_viewport_source_evidence(void);
#endif

