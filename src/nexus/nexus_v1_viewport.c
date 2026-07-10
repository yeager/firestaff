
#include "nexus_v1_viewport.h"
#include <string.h>
#include <stdio.h>

void nexus_viewport_init(Nexus_Viewport *vp) {
    if (!vp) return;
    memset(vp, 0, sizeof(*vp));
    nexus_fb_init(&vp->fb);
}

/* Render visible dungeon squares from party position */
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

    if (engine->current_level.geometry_info.dmweb_container) {
        Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
        Nexus_V1_DgnRenderPlanReceipt receipt;
        int i;

        /* Real Nexus DGN path: draw only commands derived from Structure1B.
         * If this route blocks, do not fall through to synthetic legacy
         * visuals. */
        if (nexus_v1_level_build_dgn_view_render_plan(
                &engine->current_level,
                px,
                py,
                pdir,
                commands,
                NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                &receipt) != 0 ||
            !receipt.plan_ready) {
            return;
        }

        /* BITM pixels carry global CLUT slots. Merge the decoded DMDF CLUTs
         * before rasterizing so framebuffer indices retain source colours. */
        {
            uint32_t palette[256];
            const Nexus_DMDFMaterialBank *banks[2] = {
                &engine->floor_materials, &engine->wall_materials
            };
            int bank_index;
            memcpy(palette, vp->fb.palette, sizeof(palette));
            for (bank_index = 0; bank_index < 2; ++bank_index) {
                const Nexus_DMDFMaterialBank *bank = banks[bank_index];
                int material_index;
                for (material_index = 0; bank->valid &&
                     material_index < bank->surface_count; ++material_index) {
                    const Nexus_DMDFTextureSurface *surface =
                        &bank->surfaces[material_index];
                    int color_index;
                    for (color_index = 0; surface->valid &&
                         color_index < 256; ++color_index) {
                        if (surface->palette[color_index] != 0U)
                            palette[color_index] = surface->palette[color_index];
                    }
                }
            }
            nexus_fb_set_palette(&vp->fb, palette);
        }

        for (i = 0; i < receipt.command_count; ++i) {
            const Nexus_V1_DgnRenderCommand *command = &commands[i];
            const Nexus_DMDFMaterialBank *bank;
            const Nexus_DMDFTextureSurface *surface;
            bank = (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                    command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
                ? &engine->floor_materials : &engine->wall_materials;
            if (!bank->valid || command->material_id >= bank->surface_count) {
                /* Real DGN commands are never replaced by generic colours. */
                continue;
            }
            surface = &bank->surfaces[command->material_id];
            if (!surface->valid) continue;
            switch (command->kind) {
            case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
                nexus_draw_floor_tex(&vp->fb, &vp->cam,
                                     (float)command->x,
                                     (float)command->y,
                                     surface->pixels, surface->width,
                                     surface->height, surface->palette);
                break;
            case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
                nexus_draw_ceiling_tex(&vp->fb, &vp->cam,
                                       (float)command->x,
                                       (float)command->y,
                                       surface->pixels, surface->width,
                                       surface->height, surface->palette);
                break;
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
            case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
                nexus_draw_wall(&vp->fb, &vp->cam,
                                (float)command->x, (float)command->y,
                                command->wall_dir, 0, command->material_id,
                                surface->pixels, surface->width,
                                surface->height, surface->palette);
                break;
            default:
                break;
            }
        }
        return;
    }

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
