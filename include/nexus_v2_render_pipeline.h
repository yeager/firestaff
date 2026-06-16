
#ifndef NEXUS_V2_RENDER_PIPELINE_H
#define NEXUS_V2_RENDER_PIPELINE_H

#include "nexus_v2_config.h"
#include "nexus_v2_upscaler.h"
#include "nexus_v2_lighting.h"
#include "nexus_v2_particles.h"
#include "nexus_v2_atmosphere.h"
#include "nexus_v2_smooth_movement.h"
#include "nexus_v1_rasterizer.h"

/* Nexus V2 render pipeline:
 *
 * V1 (320x200 indexed)
 *   ↓ EPX upscale + palette → RGBA
 * V2.1 (640x400 RGBA)
 *   ↓ bilinear smooth (optional)
 *   ↓ dynamic lighting
 *   ↓ fog + AO
 *   ↓ particles
 *   ↓ minimap overlay
 *   ↓ damage numbers
 * V2.2 (640x400 or 1280x800 RGBA)
 *   → SDL present */

typedef struct {
    Nexus_V2_Config config;
    Nexus_V2_SmoothState smooth;  /* V2 smooth movement state */
    Nexus_V2_LightingState lighting;
    Nexus_V2_ParticleSystem particles;
    Nexus_V2_Atmosphere atmosphere;
    uint32_t *output_buffer;     /* final RGBA */
    int output_w, output_h;
} Nexus_V2_RenderPipeline;

int nexus_v2_pipeline_init(Nexus_V2_RenderPipeline *pipe, Nexus_V2_Mode mode);
void nexus_v2_pipeline_render(Nexus_V2_RenderPipeline *pipe,
    const Nexus_Framebuffer *v1_fb,
    float game_x, float game_y, float game_angle, /* raw game state */
    float dt);
void nexus_v2_pipeline_tick(Nexus_V2_RenderPipeline *pipe,
    float game_x, float game_y, float game_angle);
void nexus_v2_pipeline_shutdown(Nexus_V2_RenderPipeline *pipe);

#endif

