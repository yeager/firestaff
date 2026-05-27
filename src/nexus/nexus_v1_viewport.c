#include "nexus_v1_viewport.h"
#include "nexus_v1_rendering.h"
#include <string.h>
#include <stdio.h>

void nexus_viewport_init(Nexus_Viewport *vp) {
    if (!vp) return;
    memset(vp, 0, sizeof(*vp));
    nexus_fb_init(&vp->fb);
}

/* Render visible dungeon squares from party position.
 * Updated to use the new textured wall/floor rendering with light levels. */
void nexus_viewport_render(Nexus_Viewport *vp, Nexus_V1_Engine *engine) {
    int px, py, pdir, d;
    int dir_dx[4] = {0, 1, 0, -1};
    int dir_dy[4] = {-1, 0, 1, 0};
    int left_dx[4] = {-1, 0, 1, 0};
    int left_dy[4] = {0, -1, 0, 1};

    if (!vp || !engine || !engine->level_loaded) return;

    /* Clear framebuffer */
    nexus_fb_clear(&vp->fb);

    /* Party position */
    px = engine->game.party_x;
    py = engine->game.party_y;
    pdir = engine->game.party_dir & 3;

    /* Setup camera at party position, looking in facing direction */
    {
        Vec3 cam_pos = {(float)px + 0.5f, 0.5f, (float)py + 0.5f};
        nexus_camera_init(&vp->cam, cam_pos, pdir);
    }

    /* Render squares in view cone: D0 (closest) to D3 (farthest) */
    for (d = 0; d < NEXUS_VIEW_DISTANCE; d++) {
        int cx = px + dir_dx[pdir] * d;
        int cy = py + dir_dy[pdir] * d;
        int lx = cx + left_dx[pdir];
        int ly = cy + left_dy[pdir];
        int rx = cx - left_dx[pdir];
        int ry = cy - left_dy[pdir];
        int sq, sq_l, sq_r;
        uint8_t light_level;

        /* Compute light level based on distance from party */
        light_level = (uint8_t)(15 - d * 3);  /* 15, 12, 9, 6 */

        /* Get square types */
        sq   = nexus_v1_level_get_square(&engine->current_level, cx, cy);
        sq_l = nexus_v1_level_get_square(&engine->current_level, lx, ly);
        sq_r = nexus_v1_level_get_square(&engine->current_level, rx, ry);

        /* Draw floor/ceiling for open squares */
        if (sq != 0) {
            nexus_render_floor_tile(&vp->fb, &vp->cam,
                (float)cx, (float)cy, -1, 0, 0, light_level);
            nexus_render_ceiling_tile(&vp->fb, &vp->cam,
                (float)cx, (float)cy, -1, 0, 0, light_level);
        }
        if (sq_l != 0) {
            nexus_render_floor_tile(&vp->fb, &vp->cam,
                (float)lx, (float)ly, -1, 0, 0, light_level);
            nexus_render_ceiling_tile(&vp->fb, &vp->cam,
                (float)lx, (float)ly, -1, 0, 0, light_level);
        }
        if (sq_r != 0) {
            nexus_render_floor_tile(&vp->fb, &vp->cam,
                (float)rx, (float)ry, -1, 0, 0, light_level);
            nexus_render_ceiling_tile(&vp->fb, &vp->cam,
                (float)rx, (float)ry, -1, 0, 0, light_level);
        }

        /* Draw walls */
        if (sq == 0) {
            /* Solid wall — draw front face (textured) */
            int wall_face = (pdir + 2) & 3;
            nexus_render_wall_quad(&vp->fb, &vp->cam,
                (float)cx, (float)cy,
                wall_face, 0, 0, 0, 255, 255,
                light_level);
        } else {
            /* Open square — draw side walls if neighbors are walls */
            if (sq_l == 0) {
                int side = (pdir + 3) & 3;
                nexus_render_wall_quad(&vp->fb, &vp->cam,
                    (float)cx, (float)cy,
                    side, 0, 0, 0, 255, 255,
                    light_level);
            }
            if (sq_r == 0) {
                int side = (pdir + 1) & 3;
                nexus_render_wall_quad(&vp->fb, &vp->cam,
                    (float)cx, (float)cy,
                    side, 0, 0, 0, 255, 255,
                    light_level);
            }
        }
    }
}

void nexus_viewport_to_rgba(const Nexus_Viewport *vp, uint32_t *rgba_out) {
    int i;
    if (!vp || !rgba_out) return;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; i++) {
        rgba_out[i] = vp->fb.palette[vp->fb.color_buffer[i]];
    }
}

/* ── Main render frame ─────────────────────────────────────────── */

/* Render one complete frame of the dungeon.
 * Orchestrates: dungeon viewport → creatures → projectiles → objects → UI. */
void nexus_v1_render_frame(Nexus_Framebuffer *fb,
    Nexus_Camera *cam,
    Nexus_V1_Engine *engine,
    const Nexus_Projectile *projectiles, int projectile_count,
    const Nexus_V1_CreatureManager *creatures,
    int frame)
{
    int ci;
    uint8_t light_level;

    if (!fb || !cam || !engine) return;

    /* Render dungeon viewport (walls, floors, ceilings) */
    /* Note: this is a simplified single-square render for Phase 4.
     * A full viewport would iterate over the view cone (NEXUS_VIEW_DISTANCE). */
    if (engine->level_loaded) {
        int px = engine->game.party_x;
        int py = engine->game.party_y;
        int pdir = engine->game.party_dir & 3;
        Vec3 cam_pos = {(float)px + 0.5f, 0.5f, (float)py + 0.5f};
        nexus_camera_init(cam, cam_pos, pdir);

        /* Render visible squares in view cone */
        {
            int dir_dx[4] = {0, 1, 0, -1};
            int dir_dy[4] = {-1, 0, 1, 0};
            int left_dx[4] = {-1, 0, 1, 0};
            int left_dy[4] = {0, -1, 0, 1};
            int d;
            for (d = 0; d < NEXUS_VIEW_DISTANCE; d++) {
                int cx = px + dir_dx[pdir] * d;
                int cy = py + dir_dy[pdir] * d;
                int lx = cx + left_dx[pdir];
                int ly = cy + left_dy[pdir];
                int rx = cx - left_dx[pdir];
                int ry = cy - left_dy[pdir];
                int sq, sq_l, sq_r;

                light_level = (uint8_t)(15 - d * 3);

                sq   = nexus_v1_level_get_square(&engine->current_level, cx, cy);
                sq_l = nexus_v1_level_get_square(&engine->current_level, lx, ly);
                sq_r = nexus_v1_level_get_square(&engine->current_level, rx, ry);

                /* Floors and ceilings */
                if (sq != 0) {
                    nexus_render_floor_tile(fb, cam, (float)cx, (float)cy, -1, 0, 0, light_level);
                    nexus_render_ceiling_tile(fb, cam, (float)cx, (float)cy, -1, 0, 0, light_level);
                }

                /* Walls */
                if (sq == 0) {
                    int front = (pdir + 2) & 3;
                    nexus_render_wall_quad(fb, cam, (float)cx, (float)cy, front, 0, 0, 0, 255, 255, light_level);
                } else {
                    if (sq_l == 0) {
                        int side = (pdir + 3) & 3;
                        nexus_render_wall_quad(fb, cam, (float)cx, (float)cy, side, 0, 0, 0, 255, 255, light_level);
                    }
                    if (sq_r == 0) {
                        int side = (pdir + 1) & 3;
                        nexus_render_wall_quad(fb, cam, (float)cx, (float)cy, side, 0, 0, 0, 255, 255, light_level);
                    }
                }
            }
        }
    }

    /* Render creatures */
    if (creatures) {
        for (ci = 0; ci < creatures->active_count; ci++) {
            const Nexus_Creature *c = &creatures->active[ci];
            if (!c->alive) continue;
            light_level = 12; /* creatures have their own light source */
            /* Find matching model in engine */
            {
                int mi;
                const Nexus_V1_Model *model = NULL;
                int model_idx = creatures->types[c->type_index].model_index;
                if (model_idx >= 0 && model_idx < engine->model_count)
                    model = &engine->models[model_idx];
                nexus_render_creature(fb, cam, model,
                    (float)c->x, (float)c->y,
                    0, c->facing, light_level);
            }
        }
    }

    /* Render projectiles */
    if (projectiles && projectile_count > 0) {
        nexus_render_projectiles(fb, cam, projectiles, projectile_count, 15);
    }

    /* Render HUD */
    nexus_render_hud(fb, &engine->game, creatures, engine);

    (void)frame; /* animation frame — used for projectile animation */
}
