
#ifndef FIRESTAFF_CSB_V2_VIEWPORT_RENDERER_H
#define FIRESTAFF_CSB_V2_VIEWPORT_RENDERER_H
#include <stdint.h>
#include "dm1_v2_anim_timing.h"

/* CSB V2 Viewport — upscaled CSB rendering with V2.1/V2.2 features.
 * Shares DM1 V2 upscale pipeline (EPX) but adds:
 *   - Custom background upscaling per DSA room
 *   - Prison door intro animation (smooth)
 *   - Extended creature sprite sheets */

typedef struct {
    int scale_factor;
    int epx_enabled;
    int custom_bg_active;
    int prison_door_progress; /* 0-100 */
    V2_AnimClock clock;
} CSB_V2_ViewportState;

void csb_v2_viewport_init(CSB_V2_ViewportState *s, int scale);
void csb_v2_viewport_v1_tick(CSB_V2_ViewportState *s, uint32_t now_ms);
void csb_v2_viewport_render_frame(CSB_V2_ViewportState *s, uint32_t now_ms);
float csb_v2_viewport_sub_tick(const CSB_V2_ViewportState *s);
const char *csb_v2_viewport_source_evidence(void);
#endif

