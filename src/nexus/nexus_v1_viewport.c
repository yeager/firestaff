
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

static uint32_t viewport_dgn_frame_hash(const Nexus_Framebuffer *fb)
{
    int i;
    uint32_t hash = 2166136261u;
    if (!fb) return 0u;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        uint32_t z_mark = fb->z_buffer[i] < 1e30f ? 1u : 0u;
        hash ^= (uint32_t)fb->color_buffer[i];
        hash *= 16777619u;
        hash ^= z_mark;
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static const Nexus_DMDFTextureSurface *viewport_plan_surface(
    const Nexus_V1_Engine *engine, const Nexus_V1_DgnRenderCommand *command)
{
    if (command->animated_texture_declared &&
        command->animated_texture_structure2_image_valid &&
        engine->animated_floor_material_route_valid) {
        const Nexus_DMDFTextureSurface *animated =
            &engine->animated_floor_materials.surfaces[
                command->animated_texture_structure2_image_id];
        if (animated->valid) return animated;
    }
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
    int px, py, pdir;

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
        /* The real runtime must not convert a decoded DMDF-only bank into
         * visible DGN material. FLOORS/WALLS need independently verified BPK
         * containers and completed host routes; missing Track 1 containers
         * leave this route blocked with no legacy visual fallback. */
        if (engine->initialized &&
            !((engine->dgn_static_material_sources.canonical_pair_bound &&
               engine->floor_mns_material_route_valid &&
               engine->wall_mns_material_route_valid) ||
              (engine->floor_bpk_container.host_route_permitted &&
               engine->wall_bpk_container.host_route_permitted &&
               engine->floor_bpk_host_route_valid &&
               engine->wall_bpk_host_route_valid &&
               engine->floor_bpk_host_route.host_consumed_surfaces &&
               engine->wall_bpk_host_route.host_consumed_surfaces))) {
            vp->last_dgn_render_receipt.blocked = 1;
            return;
        }
        plan = nexus_v1_prepare_dgn_material_plan(engine, px, py, pdir);
        if (!plan) {
            const Nexus_V1_DgnRenderPlanReceipt *blocked_plan =
                &engine->dgn_material_plan.receipt;
            vp->last_dgn_render_receipt.command_count =
                blocked_plan->command_count;
            vp->last_dgn_render_receipt.floor_count =
                blocked_plan->floor_count;
            vp->last_dgn_render_receipt.wall_count =
                blocked_plan->wall_count;
            vp->last_dgn_render_receipt.ceiling_count =
                blocked_plan->ceiling_count;
            vp->last_dgn_render_receipt.missing_material_count =
                blocked_plan->missing_material_count;
            vp->last_dgn_render_receipt.first_missing_material_id =
                blocked_plan->first_missing_material_id;
            vp->last_dgn_render_receipt.first_missing_material_kind =
                blocked_plan->first_missing_material_kind;
            vp->last_dgn_render_receipt.no_draw_structure2_source =
                blocked_plan->status ==
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE &&
                (engine->current_level.structure2_payload.valid ||
                 engine->current_level_structure2_source
                     .structure2_payload_envelope_valid);
            vp->last_dgn_render_receipt.blocked = 1;
            return;
        }
        vp->last_dgn_render_receipt.command_count =
            plan->receipt.command_count;
        vp->last_dgn_render_receipt.floor_count = plan->receipt.floor_count;
        vp->last_dgn_render_receipt.wall_count = plan->receipt.wall_count;
        vp->last_dgn_render_receipt.ceiling_count =
            plan->receipt.ceiling_count;
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
                if (surface->from_bpk) {
                    vp->last_dgn_render_receipt.bpk_material_surface_count++;
                }
                switch (command->kind) {
                case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
                    vp->last_dgn_render_receipt.floor_material_surface_count++;
                    if (surface->from_bpk) {
                        vp->last_dgn_render_receipt
                            .bpk_floor_material_surface_count++;
                    }
                    break;
                case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
                    vp->last_dgn_render_receipt.ceiling_material_surface_count++;
                    if (surface->from_bpk) {
                        vp->last_dgn_render_receipt
                            .bpk_ceiling_material_surface_count++;
                    }
                    break;
                case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
                case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
                case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
                    vp->last_dgn_render_receipt.wall_material_surface_count++;
                    if (surface->from_bpk) {
                        vp->last_dgn_render_receipt
                            .bpk_wall_material_surface_count++;
                    }
                    break;
                default:
                    break;
                }
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
        vp->last_dgn_render_receipt.frame_hash =
            viewport_dgn_frame_hash(&vp->fb);
        vp->last_dgn_render_receipt.captured_frame_ready =
            vp->last_dgn_render_receipt.written_pixels > 0 &&
            vp->last_dgn_render_receipt.frame_hash != 0u;
        vp->last_dgn_render_receipt.ready =
            vp->last_dgn_render_receipt.command_count > 0 &&
            vp->last_dgn_render_receipt.command_count ==
                vp->last_dgn_render_receipt.material_surface_count &&
            vp->last_dgn_render_receipt.floor_count ==
                vp->last_dgn_render_receipt.floor_material_surface_count &&
            vp->last_dgn_render_receipt.ceiling_count ==
                vp->last_dgn_render_receipt.ceiling_material_surface_count &&
            vp->last_dgn_render_receipt.wall_count ==
                vp->last_dgn_render_receipt.wall_material_surface_count &&
            vp->last_dgn_render_receipt.command_count ==
                vp->last_dgn_render_receipt.rasterized_command_count &&
            vp->last_dgn_render_receipt.captured_frame_ready;
        return;
    }

    /* A non-DMWeb level is not a Nexus runtime scene. Do not draw the old
     * procedural grid when authoritative Saturn DGN parsing failed. */
    vp->last_dgn_render_receipt.blocked = 1;
    vp->last_dgn_render_receipt.fallback_visuals_permitted = 0;
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

int nexus_viewport_dgn_host_route_receipt(
    const Nexus_Viewport *vp,
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnViewportHostRouteReceipt *out_receipt)
{
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    const Nexus_V1_DgnViewportRenderReceipt *render;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_MISSING;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->blocks_runtime_dgn = 1;
    if (!vp || !engine) return -1;

    memset(&handoff, 0, sizeof(handoff));
    if (nexus_v1_current_level_dgn_renderer_handoff_receipt(
            engine, &handoff) != 0) {
        return -1;
    }
    render = &vp->last_dgn_render_receipt;
    out_receipt->package_consumed = 1;
    out_receipt->handoff_status = handoff.status;
    out_receipt->fallback_visuals_permitted =
        handoff.fallback_visuals_permitted || render->fallback_visuals_permitted;
    out_receipt->no_draw_structure2_source =
        render->no_draw_structure2_source ? 1 : 0;
    out_receipt->level = engine->game.current_level;
    out_receipt->party_x = render->party_x;
    out_receipt->party_y = render->party_y;
    out_receipt->party_dir = render->party_dir;
    out_receipt->command_count = render->command_count;
    out_receipt->floor_count = render->floor_count;
    out_receipt->ceiling_count = render->ceiling_count;
    out_receipt->wall_count = render->wall_count;
    out_receipt->material_surface_count = render->material_surface_count;
    out_receipt->bpk_material_surface_count =
        render->bpk_material_surface_count;
    out_receipt->bpk_floor_material_surface_count =
        render->bpk_floor_material_surface_count;
    out_receipt->bpk_ceiling_material_surface_count =
        render->bpk_ceiling_material_surface_count;
    out_receipt->bpk_wall_material_surface_count =
        render->bpk_wall_material_surface_count;
    out_receipt->rasterized_command_count = render->rasterized_command_count;
    out_receipt->written_pixels = render->written_pixels;
    out_receipt->palette_synced = render->palette_synced;
    out_receipt->captured_frame_ready = render->captured_frame_ready;
    out_receipt->frame_hash = render->frame_hash;
    out_receipt->missing_material_count = render->missing_material_count;
    out_receipt->first_missing_material_id = render->first_missing_material_id;
    out_receipt->first_missing_material_kind =
        render->first_missing_material_kind;
    out_receipt->post_grid_0x30_ref_unique_count =
        handoff.post_grid_0x30_ref_unique_count;
    out_receipt->max_post_grid_0x30_ref =
        handoff.max_post_grid_0x30_ref;

    if (!handoff.can_render_dgn_mesh ||
        handoff.blocks_real_dgn_mesh_render ||
        handoff.fallback_visuals_permitted) {
        out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_HANDOFF;
        return 0;
    }
    if (render->no_draw_structure2_source) {
        out_receipt->status =
            NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE2_SOURCE;
        return 0;
    }
    if (!render->attempted || !render->used_real_dgn_route) {
        out_receipt->status =
            NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_VIEWPORT_NOT_RENDERED;
        return 0;
    }
    out_receipt->host_route_consumed = 1;
    if (render->missing_material_count > 0) {
        out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_MATERIALS;
        return 0;
    }
    if (!render->ready || render->blocked ||
        render->rasterized_command_count != render->command_count ||
        render->written_pixels <= 0 || !render->captured_frame_ready ||
        render->frame_hash == 0u || !render->palette_synced) {
        out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_RASTER;
        return 0;
    }
    out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH;
    out_receipt->can_present_runtime_dgn = 1;
    out_receipt->blocks_runtime_dgn = 0;
    return 0;
}

const char *nexus_viewport_dgn_host_route_status_name(
    Nexus_V1_DgnViewportHostRouteStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH:
        return "ready-rendered-mesh";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_HANDOFF:
        return "blocked-handoff";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_VIEWPORT_NOT_RENDERED:
        return "blocked-viewport-not-rendered";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_MATERIALS:
        return "blocked-materials";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_RASTER:
        return "blocked-raster";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE2_SOURCE:
        return "blocked-structure2-source";
    case NEXUS_V1_DGN_HOST_ROUTE_MISSING:
    default:
        return "missing";
    }
}
