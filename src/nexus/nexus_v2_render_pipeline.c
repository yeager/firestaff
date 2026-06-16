#include "nexus_v2_render_pipeline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int nexus_v2_pipeline_init(Nexus_V2_RenderPipeline *pipe, Nexus_V2_Mode mode) {
    if (!pipe) return -1;
    memset(pipe, 0, sizeof(*pipe));
    nexus_v2_config_init(&pipe->config, mode);
    nexus_v2_lighting_init(&pipe->lighting);
    nexus_v2_particles_init(&pipe->particles);
    nexus_v2_atmosphere_init(&pipe->atmosphere, 0);
    nexus_v2_smooth_init(&pipe->smooth);

    pipe->output_w = pipe->config.render_width;
    pipe->output_h = pipe->config.render_height;
    pipe->output_buffer = (uint32_t *)calloc(pipe->output_w * pipe->output_h, sizeof(uint32_t));
    if (!pipe->output_buffer) return -1;

    printf("Nexus V2 pipeline: %s (%dx%d) smooth=%s\n",
        mode == NEXUS_V2_UPSCALED ? "V2.1 Upscaled" :
        mode == NEXUS_V2_ENHANCED ? "V2.2 Enhanced" : "V1 Original",
        pipe->output_w, pipe->output_h,
        pipe->config.smooth_movement ? "ON" : "OFF");
    return 0;
}

/*
 * nexus_v2_pipeline_tick — call once per V1 tick (55ms).
 * Records game state and triggers smooth transitions when position changes.
 */
void nexus_v2_pipeline_tick(Nexus_V2_RenderPipeline *pipe,
    float game_x, float game_y, float game_angle) {
    if (!pipe) return;
    if (!pipe->config.smooth_movement) return;

    /* Record pre-tick position (used to derive from position for animation) */
    float prev_x = pipe->smooth.prev_x;
    float prev_y = pipe->smooth.prev_y;

    /* Trigger smooth walk animation if position changed */
    if (game_x != prev_x || game_y != prev_y) {
        nexus_v2_smooth_start_walk(&pipe->smooth, prev_x, prev_y, game_x, game_y);
    }

    /* Trigger smooth turn animation if angle changed.
     * Turn detection: compare angle modulo 4 (0=N,1=E,2=S,3=W) */
    {
        int prev_dir = (int)prev_x; /* reuse prev_x as temp */
        int game_dir = (int)game_x;
        (void)prev_dir; (void)game_dir;
    }
    /* Note: angle wrapping handled inside nexus_v2_smooth_start_turn */
    if (game_angle != pipe->smooth.prev_angle) {
        /* Only fire turn if it's a pure rotation (no significant move) */
        float dx = fabsf(game_x - pipe->smooth.prev_x);
        float dy = fabsf(game_y - pipe->smooth.prev_y);
        if (dx < 0.1f && dy < 0.1f) {
            nexus_v2_smooth_start_turn(&pipe->smooth,
                pipe->smooth.prev_angle, game_angle);
        }
    }

    /* Record current position as pre-tick for next tick */
    nexus_v2_smooth_tick(&pipe->smooth, game_x, game_y, game_angle);
}

void nexus_v2_pipeline_render(Nexus_V2_RenderPipeline *pipe,
    const Nexus_Framebuffer *v1_fb,
    float game_x, float game_y, float game_angle,
    float dt)
{
    if (!pipe || !v1_fb || !pipe->output_buffer) return;

    /* Derive interpolated camera position from smooth state */
    float cam_x, cam_y;
    float cam_z = 0.5f; /* fixed camera height */
    float cam_dir_f = game_angle;

    if (pipe->config.smooth_movement && pipe->config.mode != NEXUS_V2_OFF) {
        nexus_v2_smooth_get_position(&pipe->smooth, &cam_x, &cam_y);
        cam_dir_f = nexus_v2_smooth_get_angle(&pipe->smooth);
        /* If no active animation, fall back to game state */
        if (!nexus_v2_smooth_is_active(&pipe->smooth)) {
            cam_x = game_x;
            cam_y = game_y;
        }
        /* Advance smooth animations */
        nexus_v2_smooth_update(&pipe->smooth, dt);
    } else {
        cam_x = game_x;
        cam_y = game_y;
    }

    /* Camera direction as integer (0-3) */
    int cam_dir = (int)(cam_dir_f / 90.0f) & 3;

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
                &pipe->lighting, cam_x, cam_z, cam_dir);
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