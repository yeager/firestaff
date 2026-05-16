
#include "nexus_v2_render_pipeline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int nexus_v2_pipeline_init(Nexus_V2_RenderPipeline *pipe, Nexus_V2_Mode mode) {
    if (!pipe) return -1;
    memset(pipe, 0, sizeof(*pipe));
    nexus_v2_config_init(&pipe->config, mode);
    nexus_v2_lighting_init(&pipe->lighting);
    nexus_v2_particles_init(&pipe->particles);
    nexus_v2_atmosphere_init(&pipe->atmosphere, 0);

    pipe->output_w = pipe->config.render_width;
    pipe->output_h = pipe->config.render_height;
    pipe->output_buffer = (uint32_t *)calloc(pipe->output_w * pipe->output_h, sizeof(uint32_t));
    if (!pipe->output_buffer) return -1;

    printf("Nexus V2 pipeline: %s (%dx%d)\n",
        mode == NEXUS_V2_UPSCALED ? "V2.1 Upscaled" :
        mode == NEXUS_V2_ENHANCED ? "V2.2 Enhanced" : "V1 Original",
        pipe->output_w, pipe->output_h);
    return 0;
}

void nexus_v2_pipeline_render(Nexus_V2_RenderPipeline *pipe,
    const Nexus_Framebuffer *v1_fb,
    float cam_x, float cam_y, float cam_z, int cam_dir,
    float dt)
{
    if (!pipe || !v1_fb || !pipe->output_buffer) return;

    if (pipe->config.mode == NEXUS_V2_OFF) {
        /* V1: direct palette conversion, no scaling */
        int i;
        for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H && i < pipe->output_w * pipe->output_h; i++)
            pipe->output_buffer[i] = v1_fb->palette[v1_fb->color_buffer[i]];
        return;
    }

    /* Step 1: EPX upscale V1 → V2 resolution */
    nexus_v2_epx_upscale(v1_fb->color_buffer, NEXUS_FB_W, NEXUS_FB_H,
        pipe->output_buffer, pipe->output_w, pipe->output_h,
        v1_fb->palette);

    /* Step 2: Bilinear smooth (V2.1+) */
    if (pipe->config.bilinear_filter)
        nexus_v2_bilinear_smooth(pipe->output_buffer, pipe->output_w, pipe->output_h);

    if (pipe->config.mode == NEXUS_V2_ENHANCED) {
        /* Step 3: Dynamic lighting */
        if (pipe->config.dynamic_lighting) {
            nexus_v2_lighting_tick(&pipe->lighting, dt);
            nexus_v2_apply_lighting(pipe->output_buffer, pipe->output_w, pipe->output_h,
                &pipe->lighting, cam_x, cam_y, cam_z);
        }

        /* Step 4: Fog + tinting */
        if (pipe->config.fog)
            nexus_v2_apply_fog(pipe->output_buffer, pipe->output_w, pipe->output_h,
                &pipe->atmosphere);

        /* Step 5: Ambient occlusion */
        if (pipe->config.ambient_occlusion)
            nexus_v2_apply_ao(pipe->output_buffer, pipe->output_w, pipe->output_h,
                pipe->atmosphere.ao_strength);

        /* Step 6: Particles */
        if (pipe->config.particles) {
            nexus_v2_particles_tick(&pipe->particles, dt);
            nexus_v2_particles_render(&pipe->particles,
                pipe->output_buffer, pipe->output_w, pipe->output_h,
                v1_fb->palette, cam_x, cam_z, cam_dir);
        }
    }
}

void nexus_v2_pipeline_shutdown(Nexus_V2_RenderPipeline *pipe) {
    if (!pipe) return;
    free(pipe->output_buffer);
    pipe->output_buffer = NULL;
    printf("Nexus V2 pipeline shutdown\n");
}

