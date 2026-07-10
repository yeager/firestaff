
#include "nexus_v1_viewport.h"
#include <string.h>
#include <stdio.h>

void nexus_viewport_init(Nexus_Viewport *vp) {
    if (!vp) return;
    memset(vp, 0, sizeof(*vp));
    nexus_fb_init(&vp->fb);
    memcpy(vp->base_palette, vp->fb.palette, sizeof(vp->base_palette));
}

static int viewport_count_written_pixels(const Nexus_Framebuffer *fb)
{
    int i;
    int count = 0;
    if (!fb) return 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->z_buffer[i] < 1e30f) ++count;
    }
    return count;
}

static const Nexus_DMDFTextureSurface *viewport_plan_surface(
    const Nexus_V1_Engine *engine, const Nexus_V1_DgnRenderCommand *command)
{
    const Nexus_DMDFMaterialBank *bank =
        (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
         command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
            ? &engine->floor_materials : &engine->wall_materials;
    return &bank->surfaces[command->material_id];
}

static uint8_t *viewport_plan_palette_map(
    Nexus_Viewport *vp, const Nexus_V1_DgnRenderCommand *command)
{
    return (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
            command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
        ? vp->floor_material_palette_map[command->material_id]
        : vp->wall_material_palette_map[command->material_id];
}

static int viewport_find_palette_index(const uint32_t palette[256],
                                       const uint8_t occupied[256],
                                       uint32_t rgba)
{
    int i;
    for (i = 0; i < 256; ++i) {
        if (occupied[i] && palette[i] == rgba) return i;
    }
    return -1;
}

static int viewport_sync_dgn_material_palette(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine,
    const Nexus_V1_DgnMaterialPlan *plan)
{
    int i;
    uint32_t palette[256];
    uint8_t occupied[256] = {0};
    int next_slot = 16;
    if (vp->material_palette_valid && vp->material_engine == engine &&
        vp->material_generation == plan->generation) return 1;
    memcpy(palette, vp->base_palette, sizeof(palette));
    memset(vp->floor_material_palette_map, 0xff,
           sizeof(vp->floor_material_palette_map));
    memset(vp->wall_material_palette_map, 0xff,
           sizeof(vp->wall_material_palette_map));
    for (i = 0; i < 16; ++i) occupied[i] = 1;
    for (i = 0; i < plan->receipt.command_count; ++i) {
        const Nexus_DMDFTextureSurface *surface =
            viewport_plan_surface(engine, &plan->commands[i]);
        uint8_t *texel_map = viewport_plan_palette_map(vp, &plan->commands[i]);
        int color_index;
        for (color_index = 0; color_index < 256; ++color_index) {
            uint32_t rgba = surface->palette[color_index];
            int mapped_index;
            if ((rgba >> 24) == 0U) continue;
            mapped_index = viewport_find_palette_index(palette, occupied, rgba);
            if (mapped_index < 0) {
                while (next_slot < 256 && occupied[next_slot]) ++next_slot;
                if (next_slot == 256) {
                    vp->material_palette_valid = 0;
                    return 0;
                }
                mapped_index = next_slot++;
                palette[mapped_index] = rgba;
                occupied[mapped_index] = 1;
            }
            texel_map[color_index] = (uint8_t)mapped_index;
        }
    }
    nexus_fb_set_palette(&vp->fb, palette);
    vp->material_engine = engine;
    vp->material_generation = plan->generation;
    vp->material_palette_valid = 1;
    return 1;
}

/* Render visible dungeon squares from party position */
void nexus_viewport_render(Nexus_Viewport *vp, Nexus_V1_Engine *engine) {
    int px, py, pdir, d;
    int dir_dx[4] = {0, 1, 0, -1};
    int dir_dy[4] = {-1, 0, 1, 0};
    int left_dx[4] = {-1, 0, 1, 0};
    int left_dy[4] = {0, -1, 0, 1};

    if (!vp || !engine || !engine->level_loaded) return;
    memset(&vp->last_dgn_render_receipt, 0,
           sizeof(vp->last_dgn_render_receipt));

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

    if (engine->current_level.geometry_info.dmweb_container) {
        const Nexus_V1_DgnMaterialPlan *plan;
        int i;

        /* Real Nexus DGN path: draw only commands derived from Structure1B.
         * If this route blocks, do not fall through to synthetic legacy
         * visuals. */
        vp->last_dgn_render_receipt.attempted = 1;
        vp->last_dgn_render_receipt.used_real_dgn_route = 1;
        vp->last_dgn_render_receipt.fallback_visuals_permitted = 0;
        vp->last_dgn_render_receipt.party_x = px;
        vp->last_dgn_render_receipt.party_y = py;
        vp->last_dgn_render_receipt.party_dir = pdir;
        plan = nexus_v1_prepare_dgn_material_plan(engine, px, py, pdir);
        if (!plan) {
            vp->last_dgn_render_receipt.blocked = 1;
            return;
        }
        vp->last_dgn_render_receipt.command_count =
            plan->receipt.command_count;
        vp->last_dgn_render_receipt.floor_count = plan->receipt.floor_count;
        vp->last_dgn_render_receipt.wall_count = plan->receipt.wall_count;
        vp->last_dgn_render_receipt.ceiling_count =
            plan->receipt.command_count - plan->receipt.floor_count -
            plan->receipt.wall_count;
        if (!viewport_sync_dgn_material_palette(vp, engine, plan)) {
            vp->last_dgn_render_receipt.blocked = 1;
            return;
        }
        vp->last_dgn_render_receipt.palette_synced = 1;

        for (i = 0; i < plan->receipt.command_count; ++i) {
            const Nexus_V1_DgnRenderCommand *command = &plan->commands[i];
            const Nexus_DMDFTextureSurface *surface;
            uint8_t *texel_map;
            surface = viewport_plan_surface(engine, command);
            texel_map = viewport_plan_palette_map(vp, command);
            if (surface && surface->valid) {
                vp->last_dgn_render_receipt.material_surface_count++;
            }
            switch (command->kind) {
            case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
                nexus_draw_floor_tex_mapped_heights(&vp->fb, &vp->cam,
                                     (float)command->x,
                                     (float)command->y,
                                     command->floor_height,
                                     command->floor_rotation,
                                     surface->pixels, surface->width,
                                     surface->height, surface->palette, texel_map);
                vp->last_dgn_render_receipt.rasterized_command_count++;
                break;
            case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
                nexus_draw_ceiling_tex_mapped_heights(&vp->fb, &vp->cam,
                                       (float)command->x,
                                       (float)command->y,
                                       command->ceiling_height,
                                       command->floor_rotation,
                                       surface->pixels, surface->width,
                                       surface->height, surface->palette, texel_map);
                vp->last_dgn_render_receipt.rasterized_command_count++;
                break;
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
                nexus_draw_wall_tex_mapped(&vp->fb, &vp->cam,
                                           (float)command->x, (float)command->y,
                                           command->wall_dir, surface->pixels,
                                           surface->width, surface->height,
                                           surface->palette, texel_map);
                vp->last_dgn_render_receipt.rasterized_command_count++;
                break;
            default:
                break;
            }
        }
        vp->last_dgn_render_receipt.written_pixels =
            viewport_count_written_pixels(&vp->fb);
        vp->last_dgn_render_receipt.ready =
            vp->last_dgn_render_receipt.command_count > 0 &&
            vp->last_dgn_render_receipt.command_count ==
                vp->last_dgn_render_receipt.material_surface_count &&
            vp->last_dgn_render_receipt.command_count ==
                vp->last_dgn_render_receipt.rasterized_command_count &&
            vp->last_dgn_render_receipt.written_pixels > 0;
        return;
    }

    vp->last_dgn_render_receipt.fallback_visuals_permitted = 1;

    /* Render squares in view cone: D0 (closest) to D3 (farthest) */
    for (d = 0; d < NEXUS_VIEW_DISTANCE; d++) {
        int cx = px + dir_dx[pdir] * d;
        int cy = py + dir_dy[pdir] * d;
        int lx, ly, rx, ry;
        int sq, sq_l, sq_r;

        /* Center, left, right columns */
        lx = cx + left_dx[pdir];
        ly = cy + left_dy[pdir];
        rx = cx - left_dx[pdir];
        ry = cy - left_dy[pdir];

        /* Get square types */
        sq   = nexus_v1_level_get_square(&engine->current_level, cx, cy);
        sq_l = nexus_v1_level_get_square(&engine->current_level, lx, ly);
        sq_r = nexus_v1_level_get_square(&engine->current_level, rx, ry);

        /* Draw floor/ceiling for open squares */
        if (sq != 0) {
            nexus_draw_floor(&vp->fb, &vp->cam, (float)cx, (float)cy, 8, 9);
        }
        if (sq_l != 0) {
            nexus_draw_floor(&vp->fb, &vp->cam, (float)lx, (float)ly, 8, 9);
        }
        if (sq_r != 0) {
            nexus_draw_floor(&vp->fb, &vp->cam, (float)rx, (float)ry, 8, 9);
        }

        /* Draw walls where square is wall (type 0) or at boundaries */
        if (sq == 0) {
            /* Solid wall — draw front face */
            int wall_face = (pdir + 2) & 3; /* facing toward party */
            nexus_draw_wall_simple(&vp->fb, &vp->cam, (float)cx, (float)cy, wall_face, 5 + (d % 3));
        } else {
            /* Open square — draw side walls if neighbors are walls */
            if (sq_l == 0) {
                int side = (pdir + 3) & 3; /* left wall */
                nexus_draw_wall_simple(&vp->fb, &vp->cam, (float)cx, (float)cy, side, 6);
            }
            if (sq_r == 0) {
                int side = (pdir + 1) & 3; /* right wall */
                nexus_draw_wall_simple(&vp->fb, &vp->cam, (float)cx, (float)cy, side, 6);
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

int nexus_viewport_last_dgn_render_receipt(
    const Nexus_Viewport *vp,
    Nexus_V1_DgnViewportRenderReceipt *out_receipt)
{
    if (!vp || !out_receipt) return -1;
    *out_receipt = vp->last_dgn_render_receipt;
    return 0;
}
