#include "nexus_v1_rendering.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ── Projectile type → color mapping ───────────────────────── */

static uint8_t g_projectile_colors[NEXUS_PROJ_COUNT] = {
    12, /* FIREBALL — firebrick */
    13, /* ICEBOLT — royal blue */
    15, /* LIGHTNING — white */
    6,  /* ARROW — medium gray */
    10, /* POISON — brown (or green for acid) */
    4,  /* DEATHRAY — purple */
    10, /* ACID — brown/green */
};

/* Projectile world position → projected screen position.
 * Projectiles are billboard sprites (2D overlay on 3D scene). */
static Vec2i nexus_projectile_screen_pos(float x, float y, float z,
    const Nexus_Camera *cam)
{
    Vec3 world = {x, y, z};
    return v3_project(world, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
}

/* Render a projectile as an animated circle with directional tail */
static void nexus_render_proj_circle(Nexus_Framebuffer *fb,
    Vec2i center, int radius, uint8_t color_index)
{
    int dx, dy, dist_sq;
    int min_x = center.x - radius;
    int max_x = center.x + radius;
    int min_y = center.y - radius;
    int max_y = center.y + radius;

    if (min_x < 0) min_x = 0;
    if (max_x >= NEXUS_FB_W) max_x = NEXUS_FB_W - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= NEXUS_FB_H) max_y = NEXUS_FB_H - 1;

    for (dy = min_y; dy <= max_y; dy++) {
        for (dx = min_x; dx <= max_x; dx++) {
            dist_sq = (dx - center.x) * (dx - center.x) +
                      (dy - center.y) * (dy - center.y);
            if (dist_sq <= radius * radius) {
                int idx = dy * NEXUS_FB_W + dx;
                if (idx >= 0 && idx < NEXUS_FB_W * NEXUS_FB_H)
                    fb->color_buffer[idx] = color_index;
            }
        }
    }
}

/* Render directional tail (indicates travel direction) */
static void nexus_render_proj_tail(Nexus_Framebuffer *fb,
    Vec2i from, Vec2i to, uint8_t color_index)
{
    int dx, dy, steps, i;
    float fx, fy, inc_x, inc_y;

    dx = to.x - from.x;
    dy = to.y - from.y;
    steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    if (steps == 0) return;

    inc_x = (float)dx / (float)steps;
    inc_y = (float)dy / (float)steps;
    fx = from.x; fy = from.y;

    for (i = 0; i < steps; i++) {
        int px = (int)(fx + 0.5f);
        int py = (int)(fy + 0.5f);
        if (px >= 0 && px < NEXUS_FB_W && py >= 0 && py < NEXUS_FB_H) {
            int idx = py * NEXUS_FB_W + px;
            fb->color_buffer[idx] = color_index;
        }
        fx += inc_x; fy += inc_y;
    }
}

void nexus_render_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_Projectile *proj,
    uint8_t light_level)
{
    Vec2i screen_pos;
    int radius;
    uint8_t color;
    Vec2i tail_pos;
    (void)light_level;

    if (!fb || !cam || !proj || !proj->active) return;

    /* Compute screen position */
    screen_pos = nexus_projectile_screen_pos(proj->x, proj->y, proj->z, cam);

    /* Clip: if behind camera, don't render */
    {
        Vec4 clip_test = m4_transform(cam->view,
            (Vec4){proj->x, proj->y, proj->z, 1});
        if (clip_test.z <= 0) return;  /* behind camera */
    }

    /* Radius scales with animation frame (pulsing effect) */
    radius = 3 + (proj->anim_frame & 3);  /* 3–6 pixels */

    /* Color by projectile type */
    if (proj->type >= 0 && proj->type < NEXUS_PROJ_COUNT)
        color = g_projectile_colors[proj->type];
    else
        color = 15; /* white default */

    /* Draw main circle */
    nexus_render_proj_circle(fb, screen_pos, radius, color);

    /* Draw tail toward target (if moving) */
    tail_pos = nexus_projectile_screen_pos(
        (float)proj->target_x + 0.5f,
        0.5f,
        (float)proj->target_y + 0.5f,
        cam);
    nexus_render_proj_tail(fb, screen_pos, tail_pos, (uint8_t)(color - 4));
}

void nexus_render_projectiles(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_Projectile *projectiles, int count,
    uint8_t light_level)
{
    int i;
    if (!projectiles) return;
    for (i = 0; i < count; i++) {
        if (projectiles[i].active)
            nexus_render_projectile(fb, cam, &projectiles[i], light_level);
    }
}
