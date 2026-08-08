/* Real Track 1 MNS TEXT descriptor-to-material footprint receipt.
 * This binds only authenticated source bytes to the existing indexed bank;
 * it does not infer UVs, VDP layout, or a fallback rendering route. */
#include "nexus_v1_engine.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

typedef struct {
    int from_level;
    int from_x;
    int from_y;
    int from_dir;
    int to_level;
    int to_x;
    int to_y;
    int to_dir;
    int accepted;
    int rejected_before_present;
    int presented_frame_count;
    uint32_t from_frame_hash;
    uint32_t to_frame_hash;
} Track1PoseTransitionReceipt;

typedef struct {
    int typed_item_count;
    int first_item_id;
    int floor_descriptor_pixel_clut_candidates;
    int wall_descriptor_pixel_clut_candidates;
    int first_wall_descriptor_index;
    uint32_t first_wall_pixel_offset;
    int proven_item_material_bindings;
    int materialized_item_candidates;
} Track1Structure1FItemCandidateIndex;

/* This receipt deliberately stops at the first source boundary that is
 * actually established by Track1 data. Structure1F supplies item_id, and
 * ITEM.IBS is readable from the same authenticated package, but no parsed
 * retail field relates either an item_id or an item record to an ITEM.IBS
 * descriptor, pixel range, or CLUT. */
typedef struct {
    int item_ibs_bytes;
    uint64_t item_ibs_fnv1a64;
    int typed_item_count;
    int first_item_id;
    int alcove_item_count;
    int first_alcove_item_id;
    int item_descriptor_references;
    int item_pixel_references;
    int item_clut_references;
    int materialized_item_rasters;
} Track1Structure1FItemMaterialBlockReceipt;

static uint64_t fnv1a64_bytes(const uint8_t *data, int size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    int index;

    if (!data || size <= 0) return 0;
    for (index = 0; index < size; ++index) {
        hash ^= data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void check(int ok, const char *message)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int verify_mns(Nexus_V1_Engine *engine, const char *name)
{
    Nexus_DMDFTextureSection section;
    Nexus_DMDFMaterialBank bank;
    uint8_t *data;
    int size = 0;
    uint32_t descriptor_index;
    int mapped = 0;

    memset(&section, 0, sizeof(section));
    memset(&bank, 0, sizeof(bank));
    data = nexus_v1_read_file(engine, name, &size);
    if (!data) {
        fprintf(stderr, "FAIL: %s unavailable through canonical Nexus reader\n", name);
        return 0;
    }
    check(nexus_v1_dmdf_parse_texture_section(data, size, &section) &&
              section.valid && section.descriptor_count > 0,
          "canonical MNS exposes a bounded TEXT descriptor section");
    check(nexus_v1_dmdf_decode_text_material_bank(data, size, &bank) &&
              bank.valid,
          "canonical MNS decodes through the existing indexed material route");
    if (!section.valid || !bank.valid) {
        nexus_v1_dmdf_free_material_bank(&bank);
        free(data);
        return 0;
    }
    for (descriptor_index = 0; descriptor_index < section.descriptor_count;
         ++descriptor_index) {
        const Nexus_DMDFTextureDescriptor *descriptor =
            &section.descriptors[descriptor_index];
        const Nexus_DMDFTextureSurface *surface;
        uint64_t footprint_bytes;
        uint64_t footprint_end;

        if (!descriptor->valid ||
            descriptor->material_id >= NEXUS_DMDF_MATERIAL_COUNT) {
            ++failures;
            continue;
        }
        footprint_bytes = (uint64_t)descriptor->width * descriptor->height * 2U;
        footprint_end = (uint64_t)descriptor->pixel_offset + footprint_bytes;
        surface = &bank.surfaces[descriptor->material_id];
        check(descriptor->pixel_offset >= section.pixel_data_offset &&
                  footprint_end <= section.bytes,
              "TEXT descriptor footprint remains inside declared source bytes");
        check(surface->valid && !surface->from_bpk &&
                  surface->width == descriptor->width &&
                  surface->height == descriptor->height &&
                  surface->direct_color && surface->direct_pixels &&
                  surface->direct_pixel_count == footprint_bytes / 2U &&
                  !surface->pixels,
              "TEXT descriptor preserves exact BGR555 MNS surface");
        if (surface->valid) ++mapped;
    }
    printf("%s: descriptors=%u mapped=%d section=%u+%u\n", name,
           section.descriptor_count, mapped, section.offset, section.bytes);
    nexus_v1_dmdf_free_material_bank(&bank);
    free(data);
    return mapped == (int)section.descriptor_count;
}

static int has_mns_extension(const char *name)
{
    size_t length;
    if (!name) return 0;
    length = strlen(name);
    return length >= 4U && name[length - 4U] == '.' &&
        (name[length - 3U] == 'M' || name[length - 3U] == 'm') &&
        (name[length - 2U] == 'N' || name[length - 2U] == 'n') &&
        (name[length - 1U] == 'S' || name[length - 1U] == 's');
}

/* Inventory only: a parseable third-party MNS TEXT bank cannot become a DGN
 * route without its own canonical hash/source receipt. */
static int scan_unbound_mns_candidates(Nexus_V1_Engine *engine,
                                       int ceiling_selector,
                                       int wall_selector,
                                       int *out_ceiling_candidates,
                                       int *out_wall_candidates)
{
    int ceiling_candidates = 0;
    int wall_candidates = 0;
    int file_index;

    if (out_ceiling_candidates) *out_ceiling_candidates = 0;
    if (out_wall_candidates) *out_wall_candidates = 0;
    if (!engine || engine->source != NEXUS_SRC_ISO) return 0;
    for (file_index = 0; file_index < engine->iso.file_count; ++file_index) {
        const Nexus_ISOFile *file = &engine->iso.files[file_index];
        uint8_t *data;
        Nexus_DMDFTextureSection section;
        int descriptor_index;

        if (file->is_dir || !has_mns_extension(file->name) ||
            strcmp(file->name, "SN_FLOOR.MNS") == 0 ||
            strcmp(file->name, "SN_WALL.MNS") == 0) continue;
        data = (uint8_t *)malloc(file->size);
        if (!data || nexus_iso_read_file(&engine->iso, file, data,
                                         (int)file->size) < 0) {
            free(data);
            continue;
        }
        memset(&section, 0, sizeof(section));
        if (nexus_v1_dmdf_parse_texture_section(data, (int)file->size,
                                                &section)) {
            for (descriptor_index = 0;
                 descriptor_index < (int)section.descriptor_count;
                 ++descriptor_index) {
                if (section.descriptors[descriptor_index].material_id ==
                    (uint16_t)ceiling_selector) ++ceiling_candidates;
                if (section.descriptors[descriptor_index].material_id ==
                    (uint16_t)wall_selector) ++wall_candidates;
            }
        }
        free(data);
    }
    if (out_ceiling_candidates) *out_ceiling_candidates = ceiling_candidates;
    if (out_wall_candidates) *out_wall_candidates = wall_candidates;
    return 1;
}

static int verify_missing_selector_blocks(Nexus_V1_Engine *engine,
                                          int selector,
                                          Nexus_V1_DgnRenderCommandKind kind,
                                          Nexus_V1_DgnMaterialRejectionReason reason)
{
    Nexus_Viewport viewport;
    int level_index;

    if (!engine || selector < 0 || selector >= NEXUS_DMDF_MATERIAL_COUNT) {
        return 0;
    }
    nexus_viewport_init(&viewport);
    for (level_index = 0; level_index < 16; ++level_index) {
        int y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                    Nexus_V1_DgnRenderCommand commands[
                        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
                    Nexus_V1_DgnRenderPlanReceipt plan_receipt;
                    int command_index;
                    int found = 0;

                    if (nexus_v1_level_build_dgn_view_render_plan(
                            &engine->current_level, x, y, direction, commands,
                            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                            &plan_receipt) != 0) {
                        continue;
                    }
                    for (command_index = 0;
                         command_index < plan_receipt.command_count;
                         ++command_index) {
                        if (commands[command_index].kind == kind &&
                            commands[command_index].material_id == selector) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) continue;
                    nexus_v1_sync_dgn_runtime_pose(engine, level_index, x, y,
                                                    direction);
                    if (!nexus_v1_prepare_dgn_material_plan(engine, x, y,
                                                            direction) &&
                        engine->dgn_material_plan.receipt
                            .blocks_real_dgn_mesh_render &&
                        !engine->dgn_material_plan.receipt
                            .fallback_visuals_permitted &&
                        engine->dgn_material_plan.receipt
                            .missing_material_count > 0) {
                        if (engine->dgn_material_plan.receipt
                                .first_missing_material_id == selector &&
                            engine->dgn_material_plan.receipt
                                .first_missing_material_kind == kind &&
                            engine->dgn_material_plan.receipt
                                .material_rejection_reason == reason) {
                            Nexus_V1_DgnViewportRenderReceipt render;
                            Nexus_V1_DgnViewportHostRouteReceipt host;
                            nexus_viewport_render(&viewport, engine);
                            return nexus_viewport_last_dgn_render_receipt(
                                       &viewport, &render) == 0 &&
                                render.blocked &&
                                render.material_rejection_reason == reason &&
                                nexus_viewport_dgn_host_route_receipt(
                                    &viewport, engine, &host) == 0 &&
                                host.status ==
                                    NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_MATERIALS &&
                                host.material_rejection_reason == reason;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static int command_matches_missing_category(
    Nexus_V1_DgnRenderCommandKind command_kind,
    Nexus_V1_DgnRenderCommandKind category)
{
    if (category == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        return command_kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING;
    }
    return command_kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT ||
        command_kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT ||
        command_kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT;
}

/* Audit every selector which is actually first-missing once a bounded DGN
 * plan reaches material evaluation. Plans already blocked by an unresolved
 * Structure1F overlay or Structure2 animation are a different fail-closed
 * route and must not be relabelled as material rejections. */
static int audit_missing_selector_plans(
    Nexus_V1_Engine *engine, Nexus_V1_DgnRenderCommandKind category,
    Nexus_V1_DgnMaterialRejectionReason reason, int *out_observed)
{
    uint8_t seen[NEXUS_DMDF_MATERIAL_COUNT] = {0};
    int observed = 0;
    int failures_here = 0;
    int level_index;

    if (out_observed) *out_observed = 0;
    if (!engine) return 0;
    for (level_index = 0; level_index < 16; ++level_index) {
        int y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                    Nexus_V1_DgnRenderCommand commands[
                        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
                    Nexus_V1_DgnRenderPlanReceipt plan_receipt;
                    int command_index;
                    int selector = -1;
                    Nexus_V1_DgnRenderCommandKind first_kind =
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;

                    if (nexus_v1_level_build_dgn_view_render_plan(
                            &engine->current_level, x, y, direction, commands,
                            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                            &plan_receipt) != 0) continue;
                    if (plan_receipt.status ==
                            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS ||
                        plan_receipt.status ==
                            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE) {
                        continue;
                    }
                    for (command_index = 0;
                         command_index < plan_receipt.command_count;
                         ++command_index) {
                        const Nexus_V1_DgnRenderCommand *command =
                            &commands[command_index];
                        const Nexus_DMDFMaterialBank *bank =
                            (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                             command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
                                ? &engine->floor_materials : &engine->wall_materials;
                        if (!bank->surfaces[command->material_id].valid) {
                            selector = command->material_id;
                            first_kind = command->kind;
                            break;
                        }
                    }
                    if (selector < 0 || !command_matches_missing_category(
                            first_kind, category) || seen[selector]) continue;
                    seen[selector] = 1U;
                    ++observed;
                    nexus_v1_sync_dgn_runtime_pose(engine, level_index, x, y,
                                                    direction);
                    if (nexus_v1_prepare_dgn_material_plan(
                            engine, x, y, direction) ||
                        engine->dgn_material_plan.receipt
                            .first_missing_material_id != selector ||
                        !command_matches_missing_category(
                            engine->dgn_material_plan.receipt
                                .first_missing_material_kind, category) ||
                        engine->dgn_material_plan.receipt
                            .material_rejection_reason != reason) {
                        ++failures_here;
                    }
                }
            }
        }
    }
    if (out_observed) *out_observed = observed;
    return observed > 0 && failures_here == 0;
}

static int verify_covered_floor_selector_rasters(Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    enum { NEXUS_MNS_MIN_COVERED_POSES = 3 };
    int covered_pose_count = 0;
    unsigned int covered_level_mask = 0U;
    int level_index;

    if (!engine) return 0;
    nexus_viewport_init(&viewport);
    for (level_index = 0; level_index < 16; ++level_index) {
        int y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                    const Nexus_V1_DgnMaterialPlan *plan;
                    Nexus_V1_DgnViewportRenderReceipt render;
                    Nexus_V1_DgnViewportHostRouteReceipt host;
                    int command_index;
                    int covered_floor = 0;

                    nexus_v1_sync_dgn_runtime_pose(engine, level_index, x, y,
                                                    direction);
                    plan = nexus_v1_prepare_dgn_material_plan(engine, x, y,
                                                            direction);
                    if (!plan) continue;
                    for (command_index = 0;
                         command_index < plan->receipt.command_count;
                         ++command_index) {
                        const Nexus_V1_DgnRenderCommand *command =
                            &plan->commands[command_index];
                        if (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
                            engine->floor_materials
                                .surfaces[command->material_id].valid &&
                            !engine->floor_materials
                                .surfaces[command->material_id].from_bpk) {
                            covered_floor = 1;
                            break;
                        }
                    }
                    if (!covered_floor) continue;
                    nexus_viewport_render(&viewport, engine);
                    if (nexus_viewport_last_dgn_render_receipt(
                            &viewport, &render) == 0 && render.ready &&
                        !render.blocked && render.floor_material_surface_count > 0 &&
                        render.floor_material_surface_count == render.floor_count &&
                        render.rasterized_command_count == render.command_count &&
                        render.bpk_material_surface_count == 0 &&
                        render.palette_synced && render.captured_frame_ready &&
                        nexus_viewport_dgn_host_route_receipt(
                            &viewport, engine, &host) == 0 &&
                        host.can_present_runtime_dgn &&
                        host.static_mns_floor_command_consumed &&
                        host.static_mns_floor_descriptor_bound &&
                        host.static_mns_floor_clut_source_index >= 0 &&
                        host.static_mns_floor_clut_frame_index >= 0 &&
                        host.static_mns_floor_clut_frame_index < 256 &&
                        viewport.fb.palette[host.static_mns_floor_clut_frame_index] ==
                            host.static_mns_floor_clut_rgba) {
                        const Nexus_DMDFTextureSurface *surface =
                            &engine->floor_materials.surfaces[
                                host.static_mns_floor_material_id];
                        int host_command_found = 0;
                        for (command_index = 0;
                             command_index < plan->receipt.command_count;
                             ++command_index) {
                            const Nexus_V1_DgnRenderCommand *command =
                                &plan->commands[command_index];
                            if (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
                                command->x == host.static_mns_floor_x &&
                                command->y == host.static_mns_floor_y &&
                                command->depth == host.static_mns_floor_depth &&
                                command->material_id ==
                                    host.static_mns_floor_material_id) {
                                host_command_found = 1;
                                break;
                            }
                        }
                        if (host_command_found && surface->valid &&
                            surface->source_text_descriptor_bound &&
                            surface->source_text_section_offset ==
                                host.static_mns_floor_text_section_offset &&
                            surface->source_text_descriptor_index ==
                                host.static_mns_floor_descriptor_index &&
                            surface->source_text_pixel_offset ==
                                host.static_mns_floor_pixel_offset) {
                            printf("Covered floor receipt: level=%d pose=%d,%d,%d "
                                   "cell=%d,%d depth=%d material=%d "
                                   "TEXT=%u descriptor=%u pixel=%u CLUT=%d->%d\n",
                                   level_index, x, y, direction,
                                   host.static_mns_floor_x,
                                   host.static_mns_floor_y,
                                   host.static_mns_floor_depth,
                                   host.static_mns_floor_material_id,
                                   host.static_mns_floor_text_section_offset,
                                   host.static_mns_floor_descriptor_index,
                                   host.static_mns_floor_pixel_offset,
                                   host.static_mns_floor_clut_source_index,
                                   host.static_mns_floor_clut_frame_index);
                            ++covered_pose_count;
                            covered_level_mask |= 1U << level_index;
                            if (covered_pose_count >=
                                NEXUS_MNS_MIN_COVERED_POSES) {
                                printf("Covered floor receipt summary: poses=%d "
                                       "level-mask=%04x\n",
                                       covered_pose_count,
                                       covered_level_mask);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    printf("Covered floor receipt summary: poses=%d level-mask=%04x\n",
           covered_pose_count, covered_level_mask);
    return 0;
}

/* A covered wall can enter the typed plan, but an incomplete plan remains
 * no-draw until every selector is source-proven. This does not rasterize the
 * wall or relax the separate selector-64 rejection route. */
static int verify_covered_wall_selector_enters_plan(Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    int level_index;

    if (!engine) return 0;
    nexus_viewport_init(&viewport);
    for (level_index = 0; level_index < 16; ++level_index) {
        int y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                    const Nexus_V1_DgnMaterialPlan *plan;
                    Nexus_V1_DgnRenderPlanReceipt *receipt;
                    Nexus_V1_DgnViewportRenderReceipt render;

                    nexus_v1_sync_dgn_runtime_pose(engine, level_index, x, y,
                                                    direction);
                    plan = nexus_v1_prepare_dgn_material_plan(engine, x, y,
                                                            direction);
                    receipt = &engine->dgn_material_plan.receipt;
                    if (plan || receipt->static_mns_texturable_wall_command_count ==
                                    0) {
                        continue;
                    }
                    if (receipt->blocks_real_dgn_mesh_render &&
                        !receipt->fallback_visuals_permitted &&
                        receipt->missing_material_count > 0) {
                        nexus_viewport_render(&viewport, engine);
                        printf("Covered wall plan receipt: level=%d pose=%d,%d,%d "
                               "typed-wall-commands=%d first-missing=%d\n",
                               level_index, x, y, direction,
                               receipt->static_mns_texturable_wall_command_count,
                               receipt->first_missing_material_id);
                        return nexus_viewport_last_dgn_render_receipt(
                                   &viewport, &render) == 0 &&
                            render.blocked &&
                            !render.fallback_visuals_permitted &&
                            render.rasterized_command_count == 0 &&
                            render.written_pixels == 0 &&
                            !render.static_mns_wall_command_consumed &&
                            render.static_mns_texturable_wall_command_count ==
                                receipt->static_mns_texturable_wall_command_count;
                    }
                }
            }
        }
    }
    return 0;
}

/* Probe the existing indexed TEXT raster path with one authenticated wall
 * surface. This is not a partial-dungeon presentation route: the normal
 * viewport still blocks every incomplete material plan. */
static int verify_real_wall_text_raster(Nexus_V1_Engine *engine)
{
    Nexus_Framebuffer fb;
    Nexus_Camera camera;
    uint8_t texel_map[256];
    uint32_t palette[256];
    int selector;

    if (!engine || !engine->dgn_static_material_sources
                         .wall_mns.canonical_hash_verified ||
        !engine->wall_mns_material_route_valid) {
        return 0;
    }
    for (selector = 0; selector < NEXUS_DMDF_MATERIAL_COUNT; ++selector) {
        const Nexus_DMDFTextureSurface *surface =
            &engine->wall_materials.surfaces[selector];
        int source_index;
        int next_palette_index = 16;
        int written = 0;
        int indexed = 0;

        if (!surface->valid || !surface->source_text_descriptor_bound ||
            !surface->pixels || surface->width <= 0 || surface->height <= 0) {
            continue;
        }
        memset(texel_map, 0xff, sizeof(texel_map));
        nexus_fb_init(&fb);
        nexus_fb_clear(&fb);
        memcpy(palette, fb.palette, sizeof(palette));
        for (source_index = 0; source_index < 256; ++source_index) {
            if ((surface->palette[source_index] >> 24) == 0U) continue;
            if (next_palette_index >= 256) return 0;
            texel_map[source_index] = (uint8_t)next_palette_index;
            palette[next_palette_index++] = surface->palette[source_index];
        }
        if (next_palette_index == 16) continue;
        nexus_fb_set_palette(&fb, palette);
        nexus_camera_init(&camera, (Vec3){0.5f, 0.5f, 3.5f}, 0);
        nexus_draw_wall_tex_mapped(&fb, &camera, 0.0f, 2.0f, 0,
                                   surface->pixels, surface->width,
                                   surface->height, surface->palette,
                                   texel_map);
        for (source_index = 0; source_index < NEXUS_FB_W * NEXUS_FB_H;
             ++source_index) {
            if (fb.z_buffer[source_index] < 1e30f) {
                ++written;
                if (fb.color_buffer[source_index] >= 16U &&
                    (int)fb.color_buffer[source_index] < next_palette_index) {
                    ++indexed;
                }
            }
        }
        if (written > 0 && indexed > 0) {
            printf("Indexed wall raster receipt: selector=%d TEXT=%u "
                   "descriptor=%u pixel=%u palette-slots=%d written=%d indexed=%d\n",
                   selector, surface->source_text_section_offset,
                   surface->source_text_descriptor_index,
                   surface->source_text_pixel_offset,
                   next_palette_index - 16, written, indexed);
            return 1;
        }
    }
    return 0;
}

static int map_surface_into_shared_palette(
    const Nexus_DMDFTextureSurface *surface, uint32_t palette[256],
    uint8_t texel_map[256], int *next_palette_index)
{
    int source_index;

    if (!surface || !palette || !texel_map || !next_palette_index) return 0;
    memset(texel_map, 0xff, 256U);
    for (source_index = 0; source_index < 256; ++source_index) {
        uint32_t rgba = surface->palette[source_index];
        int palette_index;
        if ((rgba >> 24) == 0U) continue;
        for (palette_index = 16; palette_index < *next_palette_index;
             ++palette_index) {
            if (palette[palette_index] == rgba) break;
        }
        if (palette_index == *next_palette_index) {
            if (*next_palette_index >= 256) return 0;
            palette[(*next_palette_index)++] = rgba;
        }
        texel_map[source_index] = (uint8_t)palette_index;
    }
    return *next_palette_index > 16;
}

static int framebuffer_written_pixel_count(const Nexus_Framebuffer *fb)
{
    int pixel;
    int count = 0;

    if (!fb) return 0;
    for (pixel = 0; pixel < NEXUS_FB_W * NEXUS_FB_H; ++pixel) {
        if (fb->z_buffer[pixel] < 1e30f) ++count;
    }
    return count;
}

/* Bounded real-data composition evidence only. It uses the same indexed
 * raster calls as the viewport, but cannot present partial DGN geometry. */
static int verify_real_selector_zero_floor_wall_composition(
    Nexus_V1_Engine *engine)
{
    const Nexus_DMDFTextureSurface *floor;
    const Nexus_DMDFTextureSurface *wall;
    Nexus_Framebuffer fb;
    Nexus_Camera camera;
    uint32_t palette[256];
    uint8_t floor_map[256];
    uint8_t wall_map[256];
    int next_palette_index = 16;
    int z;

    if (!engine || !engine->dgn_static_material_sources.canonical_pair_bound ||
        !engine->floor_mns_material_route_valid ||
        !engine->wall_mns_material_route_valid) {
        return 0;
    }
    floor = &engine->floor_materials.surfaces[0];
    wall = &engine->wall_materials.surfaces[0];
    if (!floor->valid || !floor->source_text_descriptor_bound ||
        !floor->pixels || !wall->valid || !wall->source_text_descriptor_bound ||
        !wall->pixels) {
        return 0;
    }
    nexus_fb_init(&fb);
    memcpy(palette, fb.palette, sizeof(palette));
    if (!map_surface_into_shared_palette(floor, palette, floor_map,
                                         &next_palette_index) ||
        !map_surface_into_shared_palette(wall, palette, wall_map,
                                         &next_palette_index)) {
        return 0;
    }
    nexus_camera_init(&camera, (Vec3){0.5f, 0.5f, 3.5f}, 0);
    for (z = 1; z <= 3; ++z) {
        int wall_dir;
        for (wall_dir = 0; wall_dir < 4; ++wall_dir) {
            int floor_written;
            int wall_written;

            nexus_fb_clear(&fb);
            nexus_fb_set_palette(&fb, palette);
            nexus_draw_floor_tex_mapped(&fb, &camera, 0.0f, (float)z,
                                        floor->pixels, floor->width,
                                        floor->height, floor->palette,
                                        floor_map);
            floor_written = framebuffer_written_pixel_count(&fb);
            nexus_draw_wall_tex_mapped(&fb, &camera, 0.0f, (float)z,
                                       wall_dir, wall->pixels, wall->width,
                                       wall->height, wall->palette, wall_map);
            wall_written = framebuffer_written_pixel_count(&fb) - floor_written;
            if (floor_written > 0 && wall_written > 0) {
                printf("Indexed floor/wall composition receipt: floor=0 "
                       "wall=0 order=floor,wall palette-slots=%d z=%d dir=%d "
                       "floor-written=%d wall-written=%d total=%d\n",
                       next_palette_index - 16, z, wall_dir, floor_written,
                       wall_written, framebuffer_written_pixel_count(&fb));
                return 1;
            }
        }
    }
    return 0;
}

static int surface_has_proven_text_pixels_clut(
    const Nexus_DMDFTextureSurface *surface)
{
    int source_index;

    if (!surface || !surface->valid || !surface->source_text_descriptor_bound ||
        !surface->pixels) {
        return 0;
    }
    for (source_index = 0; source_index < 256; ++source_index) {
        if ((surface->palette[source_index] >> 24) != 0U) return 1;
    }
    return 0;
}

/* Find the next non-zero selector that real DGN commands use for both a
 * bounded floor and wall sample. The direct composition is evidence only;
 * the normal viewport is checked on the same pose and must stay no-draw. */
static int verify_next_real_selector_floor_wall_composition(
    Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    int level_index;

    if (!engine) return 0;
    nexus_viewport_init(&viewport);
    for (level_index = 0; level_index < 16; ++level_index) {
        int party_y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (party_y = 0; party_y < NEXUS_MAX_MAP_SIZE; ++party_y) {
            int party_x;
            for (party_x = 0; party_x < NEXUS_MAX_MAP_SIZE; ++party_x) {
                int party_dir;
                for (party_dir = 0; party_dir < 4; ++party_dir) {
                    Nexus_V1_DgnRenderCommand commands[
                        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
                    Nexus_V1_DgnRenderPlanReceipt plan_receipt;
                    int command_index;
                    int selector;

                    if (nexus_v1_level_build_dgn_view_render_plan(
                            &engine->current_level, party_x, party_y,
                            party_dir, commands,
                            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                            &plan_receipt) != 0) {
                        continue;
                    }
                    for (selector = 1; selector < NEXUS_DMDF_MATERIAL_COUNT;
                         ++selector) {
                        const Nexus_V1_DgnRenderCommand *floor_command = NULL;
                        const Nexus_V1_DgnRenderCommand *wall_command = NULL;
                        const Nexus_DMDFTextureSurface *floor;
                        const Nexus_DMDFTextureSurface *wall;
                        Nexus_Framebuffer fb;
                        Nexus_Camera camera;
                        uint32_t palette[256];
                        uint8_t floor_map[256];
                        uint8_t wall_map[256];
                        int next_palette_index = 16;
                        int floor_written;
                        int wall_written;
                        Nexus_V1_DgnViewportRenderReceipt render;
                        Nexus_V1_DgnViewportHostRouteReceipt host;

                        for (command_index = 0;
                             command_index < plan_receipt.command_count;
                             ++command_index) {
                            const Nexus_V1_DgnRenderCommand *command =
                                &commands[command_index];
                            if (command->material_id != selector) continue;
                            if (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
                                !floor_command) {
                                floor_command = command;
                            } else if ((command->kind ==
                                            NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT ||
                                        command->kind ==
                                            NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT ||
                                        command->kind ==
                                            NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT) &&
                                       !wall_command) {
                                wall_command = command;
                            }
                        }
                        if (!floor_command || !wall_command) continue;
                        floor = &engine->floor_materials.surfaces[selector];
                        wall = &engine->wall_materials.surfaces[selector];
                        if (!surface_has_proven_text_pixels_clut(floor) ||
                            !surface_has_proven_text_pixels_clut(wall)) {
                            continue;
                        }
                        nexus_fb_init(&fb);
                        memcpy(palette, fb.palette, sizeof(palette));
                        if (!map_surface_into_shared_palette(
                                floor, palette, floor_map,
                                &next_palette_index) ||
                            !map_surface_into_shared_palette(
                                wall, palette, wall_map,
                                &next_palette_index)) {
                            continue;
                        }
                        nexus_fb_clear(&fb);
                        nexus_fb_set_palette(&fb, palette);
                        nexus_camera_init(&camera,
                                          (Vec3){(float)party_x + 0.5f, 0.5f,
                                                 (float)party_y + 0.5f},
                                          party_dir);
                        nexus_draw_floor_tex_mapped(
                            &fb, &camera, (float)floor_command->x,
                            (float)floor_command->y, floor->pixels,
                            floor->width, floor->height, floor->palette,
                            floor_map);
                        floor_written = framebuffer_written_pixel_count(&fb);
                        nexus_draw_wall_tex_mapped(
                            &fb, &camera, (float)wall_command->x,
                            (float)wall_command->y, wall_command->wall_dir,
                            wall->pixels, wall->width, wall->height,
                            wall->palette, wall_map);
                        wall_written = framebuffer_written_pixel_count(&fb) -
                            floor_written;
                        if (floor_written <= 0 || wall_written <= 0) continue;

                        nexus_v1_sync_dgn_runtime_pose(engine, level_index,
                                                        party_x, party_y,
                                                        party_dir);
                        nexus_viewport_render(&viewport, engine);
                        if (nexus_viewport_last_dgn_render_receipt(
                                &viewport, &render) != 0 || !render.blocked ||
                            render.rasterized_command_count != 0 ||
                            render.written_pixels != 0 ||
                            render.missing_material_count <= 0 ||
                            !render.static_mns_wall_candidate_ready_for_complete_plan ||
                            render.static_mns_wall_candidate_material_id !=
                                selector ||
                            render.static_mns_wall_command_consumed ||
                            render.static_mns_composition_consumed ||
                            render.static_mns_composition_floor_written_pixels != 0 ||
                            render.static_mns_composition_wall_written_pixels != 0) {
                            continue;
                        }
                        if (nexus_viewport_dgn_host_route_receipt(
                                &viewport, engine, &host) != 0 ||
                            host.status !=
                                NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_MATERIALS ||
                            !host.static_mns_wall_candidate_ready_for_complete_plan ||
                            host.static_mns_wall_candidate_material_id !=
                                selector ||
                            host.static_mns_wall_command_consumed ||
                            host.static_mns_composition_consumed ||
                            host.recoverable_required_material_category !=
                                NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL ||
                            !host.recoverable_required_material_category_name ||
                            strcmp(host.recoverable_required_material_category_name,
                                   "wall") != 0) {
                            continue;
                        }
                        printf("Next indexed floor/wall composition receipt: "
                               "selector=%d level=%d pose=%d,%d,%d "
                               "order=floor,wall palette-slots=%d "
                               "floor-written=%d wall-written=%d total=%d "
                               "blocked-missing=%d viewport-candidate=%d "
                               "required-category=%s TEXT=%u descriptor=%u pixel=%u\n",
                               selector, level_index, party_x, party_y,
                               party_dir, next_palette_index - 16,
                               floor_written, wall_written,
                               framebuffer_written_pixel_count(&fb),
                               render.first_missing_material_id,
                               render.static_mns_wall_candidate_material_id,
                               host.recoverable_required_material_category_name,
                               render.static_mns_wall_candidate_text_section_offset,
                               render.static_mns_wall_candidate_descriptor_index,
                               render.static_mns_wall_candidate_pixel_offset);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* Survey real poses through the actual material-plan gate. A zero result is
 * valid evidence while selectors remain incomplete; this never fabricates a
 * visible dungeon frame. */
static int survey_real_complete_floor_wall_poses(Nexus_V1_Engine *engine)
{
    enum { NEXUS_MIN_COMPLETE_POSES = 3 };
    Nexus_Viewport viewport;
    uint32_t surveyed = 0U;
    uint32_t complete = 0U;
    uint32_t printed = 0U;
    uint32_t viewport_rejected = 0U;
    int level_index;

    if (!engine) return 0;
    nexus_viewport_init(&viewport);
    for (level_index = 0; level_index < 16; ++level_index) {
        int party_y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (party_y = 0; party_y < NEXUS_MAX_MAP_SIZE; ++party_y) {
            int party_x;
            for (party_x = 0; party_x < NEXUS_MAX_MAP_SIZE; ++party_x) {
                int party_dir;
                for (party_dir = 0; party_dir < 4; ++party_dir) {
                    const Nexus_V1_DgnMaterialPlan *plan;
                    Nexus_V1_DgnViewportRenderReceipt render;

                    ++surveyed;
                    nexus_v1_sync_dgn_runtime_pose(engine, level_index,
                                                    party_x, party_y,
                                                    party_dir);
                    plan = nexus_v1_prepare_dgn_material_plan(
                        engine, party_x, party_y, party_dir);
                    if (!plan || !plan->receipt.plan_ready ||
                        plan->receipt.missing_material_count != 0 ||
                        plan->receipt.blocks_real_dgn_mesh_render) {
                        continue;
                    }
                    nexus_viewport_render(&viewport, engine);
                    if (nexus_viewport_last_dgn_render_receipt(
                            &viewport, &render) != 0 || !render.ready ||
                        render.blocked || render.fallback_visuals_permitted ||
                        render.missing_material_count != 0) {
                        ++viewport_rejected;
                        continue;
                    }
                    if (!render.static_mns_composition_consumed ||
                        render.static_mns_composition_floor_written_pixels <= 0 ||
                        render.static_mns_composition_wall_written_pixels <= 0) {
                        continue;
                    }
                    ++complete;
                    if (printed < 3U) {
                        printf("Complete Track1 floor/wall pose: level=%d pose=%d,%d,%d "
                               "floor-written=%d wall-written=%d total=%d\n",
                               level_index, party_x, party_y, party_dir,
                               render.static_mns_composition_floor_written_pixels,
                               render.static_mns_composition_wall_written_pixels,
                               render.written_pixels);
                        ++printed;
                    }
                    if (complete >= NEXUS_MIN_COMPLETE_POSES) {
                        printf("Complete Track1 floor/wall pose survey: "
                               "scanned=%u complete=%u viewport-rejected=%u\n",
                               surveyed, complete, viewport_rejected);
                        return 1;
                    }
                }
            }
        }
    }
    printf("Complete Track1 floor/wall pose survey: scanned=%u complete=%u "
           "viewport-rejected=%u\n",
           surveyed, complete, viewport_rejected);
    return surveyed > 0U && complete >= NEXUS_MIN_COMPLETE_POSES;
}

/* One bounded present receipt from a real complete material plan. Re-rendering
 * the exact pose must preserve both frame and CLUT identities. */
static int verify_track1_complete_present_receipt(Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt first;
    Nexus_V1_DgnViewportHostRouteReceipt second;

    if (!engine || nexus_v1_load_level(engine, 0) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 1);
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &first) != 0 ||
        !first.can_present_runtime_dgn || first.blocks_runtime_dgn ||
        !first.captured_frame_ready || first.frame_width != NEXUS_FB_W ||
        first.frame_height != NEXUS_FB_H || first.written_pixels != 35864 ||
        first.frame_hash == 0U || first.clut_hash == 0U ||
        !first.static_mns_composition_consumed ||
        !first.static_mns_composition_shared_clut ||
        first.static_mns_composition_floor_written_pixels != 20160 ||
        first.static_mns_composition_wall_written_pixels != 15704) {
        return 0;
    }
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &second) != 0 ||
        second.frame_width != first.frame_width ||
        second.frame_height != first.frame_height ||
        second.written_pixels != first.written_pixels ||
        second.frame_hash != first.frame_hash || second.clut_hash != first.clut_hash) {
        return 0;
    }
    printf("Track1 complete present receipt: level=0 pose=0,0,1 frame=%dx%d "
           "written=%d frame-hash=%08x clut-hash=%08x\n",
           first.frame_width, first.frame_height, first.written_pixels,
           first.frame_hash, first.clut_hash);
    return 1;
}

/* This is the post-menu, champion-start host admission boundary. It does not
 * alter startup assets: callers still need the normal title/menu gate before
 * a champion execution reaches this DGN admission check. */
static int verify_track1_champion_start_runtime_admission(
    Nexus_V1_Engine *engine)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_DgnViewportHostRouteReceipt host;
    Nexus_V1_StartupRuntimeHandoffRoute route;
    Nexus_Viewport viewport;
    Track1PoseTransitionReceipt transition;
    uint32_t presented_frame_hash = 0U;
    uint32_t presented_clut_hash = 0U;
    int presented_frames = 0;

    if (!engine) return 0;
    memset(&execution, 0, sizeof(execution));
    execution.kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON;
    if (nexus_v1_load_level(engine, 0) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 1);
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &host) != 0 ||
        execution.kind != NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON ||
        host.status != NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH ||
        !host.package_consumed || !host.host_route_consumed ||
        !host.can_present_runtime_dgn || host.blocks_runtime_dgn ||
        !host.captured_frame_ready || host.frame_width != NEXUS_FB_W ||
        host.frame_height != NEXUS_FB_H || host.frame_hash != 0xafc5f63bU ||
        host.clut_hash != 0x61472ee9U || !host.palette_synced ||
        host.rasterized_command_count <= 0 || host.written_pixels != 35864) {
        return 0;
    }
    route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE;
    /* The host consumes this verified indexed frame exactly once. */
    presented_frame_hash = host.frame_hash;
    presented_clut_hash = host.clut_hash;
    ++presented_frames;
    memset(&transition, 0, sizeof(transition));
    transition.from_level = 0;
    transition.from_x = 0;
    transition.from_y = 0;
    transition.from_dir = 1;
    transition.to_level = 0;
    transition.to_x = 0;
    transition.to_y = 0;
    transition.to_dir = 2;
    transition.from_frame_hash = presented_frame_hash;

    /* A second complete pose must replace, never reuse, the first frame. */
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 2);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &host) != 0 ||
        !host.can_present_runtime_dgn || host.blocks_runtime_dgn ||
        !host.captured_frame_ready || host.frame_width != NEXUS_FB_W ||
        host.frame_height != NEXUS_FB_H || host.frame_hash == 0U ||
        host.clut_hash == 0U || host.frame_hash == presented_frame_hash) {
        return 0;
    }
    presented_frame_hash = host.frame_hash;
    presented_clut_hash = host.clut_hash;
    ++presented_frames;
    transition.accepted = 1;
    transition.presented_frame_count = presented_frames;
    transition.to_frame_hash = presented_frame_hash;
    if (!transition.accepted || transition.presented_frame_count != 2 ||
        transition.from_frame_hash == transition.to_frame_hash) {
        return 0;
    }

    if (nexus_v1_load_level(engine, 10) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 10, 22, 21, 1);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &host) != 0 ||
        !host.blocks_runtime_dgn || host.can_present_runtime_dgn ||
        host.first_missing_material_id != 193 ||
        host.material_blocker == NULL ||
        host.captured_frame_ready || host.frame_hash != 0U ||
        host.clut_hash != 0U || presented_frames != 2 ||
        presented_frame_hash == 0xafc5f63bU ||
        presented_clut_hash != 0x61472ee9U) {
        return 0;
    }
    route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
    transition.to_level = 10;
    transition.to_x = 22;
    transition.to_y = 21;
    transition.to_dir = 1;
    transition.rejected_before_present = 1;
    if (!transition.rejected_before_present ||
        transition.presented_frame_count != presented_frames) {
        return 0;
    }
    printf("Track1 champion-select->runtime admission: "
           "LEV00=ready-render-state package+host-consumed "
           "second-pose-frame=%08x presented=%d LEV10-selector-1/193=dgn-blocked "
           "transition=%d,%d,%d->%d,%d,%d final-route=%d\n",
           presented_frame_hash, presented_frames,
           transition.from_x, transition.from_y, transition.from_dir,
           transition.to_x, transition.to_y, transition.to_dir, route);
    return route == NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
}

static int verify_track1_runtime_pose_transition_dispatch(
    Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt before;
    Nexus_V1_DgnViewportHostRouteReceipt after_safe;
    Nexus_V1_DgnViewportHostRouteReceipt after_reject;
    Nexus_V1_DgnRuntimePoseTransitionReceipt transition;

    if (!engine || nexus_v1_load_level(engine, 0) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 1);
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &before) != 0 ||
        !before.can_present_runtime_dgn) {
        return 0;
    }
    if (!nexus_v1_try_dgn_runtime_pose_transition(
            engine, 0, 0, 2, &transition) || !transition.accepted ||
        transition.rejected_before_commit || transition.from_dir != 1 ||
        transition.committed_dir != 2 || transition.missing_material_count != 0) {
        return 0;
    }
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(
            &viewport, engine, &after_safe) != 0 ||
        !after_safe.can_present_runtime_dgn ||
        after_safe.frame_hash == before.frame_hash) {
        return 0;
    }
    if (nexus_v1_try_dgn_runtime_pose_transition(
            engine, 0, 0, 0, &transition) || !transition.rejected_before_commit ||
        transition.accepted || transition.missing_material_count <= 0 ||
        transition.committed_x != 0 || transition.committed_y != 0 ||
        transition.committed_dir != 2 || transition.fallback_visuals_permitted) {
        return 0;
    }
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(
            &viewport, engine, &after_reject) != 0 ||
        !after_reject.can_present_runtime_dgn ||
        after_reject.frame_hash != after_safe.frame_hash ||
        after_reject.clut_hash != after_safe.clut_hash) {
        return 0;
    }
    printf("Track1 runtime pose dispatch: accepted=0,0,1->0,0,2 "
           "rejected=0,0,0 retained-frame=%08x\n", after_reject.frame_hash);
    return 1;
}

static int find_complete_track1_pose(Nexus_V1_Engine *engine, int level,
                                     int *out_x, int *out_y, int *out_dir)
{
    int y;

    if (!engine || nexus_v1_load_level(engine, level) != 0) return 0;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        int x;
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            int dir;
            for (dir = 0; dir < 4; ++dir) {
                const Nexus_V1_DgnMaterialPlan *plan;
                nexus_v1_sync_dgn_runtime_pose(engine, level, x, y, dir);
                plan = nexus_v1_prepare_dgn_material_plan(engine, x, y, dir);
                if (plan && plan->receipt.plan_ready &&
                    !plan->receipt.blocks_real_dgn_mesh_render &&
                    plan->receipt.missing_material_count == 0) {
                    *out_x = x;
                    *out_y = y;
                    *out_dir = dir;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int verify_track1_cross_level_pose_transition_dispatch(
    Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt source;
    Nexus_V1_DgnViewportHostRouteReceipt target;
    Nexus_V1_DgnViewportHostRouteReceipt restored;
    Nexus_V1_DgnRuntimePoseTransitionReceipt transition;
    int target_level = -1;
    int target_x = 0;
    int target_y = 0;
    int target_dir = 0;
    int level;

    if (!engine) return 0;
    for (level = 1; level < 16; ++level) {
        if (find_complete_track1_pose(engine, level, &target_x, &target_y,
                                      &target_dir)) {
            target_level = level;
            break;
        }
    }
    if (target_level < 0 || nexus_v1_load_level(engine, 0) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 2);
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &source) != 0 ||
        source.status != NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH ||
        !source.package_consumed || !source.host_route_consumed ||
        !source.can_present_runtime_dgn || source.frame_width != NEXUS_FB_W ||
        source.frame_height != NEXUS_FB_H || !source.palette_synced ||
        source.clut_hash == 0U) {
        return 0;
    }
    if (!nexus_v1_try_dgn_runtime_level_pose_transition(
            engine, target_level, target_x, target_y, target_dir,
            &transition) || !transition.accepted ||
        transition.committed_level != target_level ||
        transition.committed_x != target_x || transition.committed_y != target_y ||
        transition.committed_dir != target_dir) {
        return 0;
    }
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &target) != 0 ||
        target.status != NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH ||
        !target.package_consumed || !target.host_route_consumed ||
        !target.can_present_runtime_dgn || target.frame_width != NEXUS_FB_W ||
        target.frame_height != NEXUS_FB_H || !target.palette_synced ||
        target.clut_hash == 0U || target.frame_hash == source.frame_hash) {
        return 0;
    }
    nexus_v1_tick(engine);
    nexus_viewport_render(&viewport, engine);
    if (engine->game.current_level != target_level ||
        engine->game.party_x != target_x || engine->game.party_y != target_y ||
        engine->game.party_dir != target_dir ||
        nexus_viewport_dgn_host_route_receipt(&viewport, engine, &restored) != 0 ||
        !restored.can_present_runtime_dgn ||
        restored.frame_hash != target.frame_hash ||
        restored.clut_hash != target.clut_hash) {
        return 0;
    }
    if (nexus_v1_try_dgn_runtime_level_pose_transition(
            engine, 10, 22, 21, 1, &transition) || !transition.rejected_before_commit ||
        transition.committed_level != target_level ||
        transition.committed_x != target_x || transition.committed_y != target_y ||
        transition.committed_dir != target_dir || transition.missing_material_count <= 0) {
        return 0;
    }
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &restored) != 0 ||
        restored.status != NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH ||
        !restored.package_consumed || !restored.host_route_consumed ||
        !restored.can_present_runtime_dgn || restored.frame_width != NEXUS_FB_W ||
        restored.frame_height != NEXUS_FB_H || !restored.palette_synced ||
        restored.frame_hash != target.frame_hash || restored.clut_hash != target.clut_hash) {
        return 0;
    }
    printf("Track1 complete pose transition: LEV00->LEV%02d pose=%d,%d,%d "
           "package+host-consumed frame=320x200 clut=%08x "
           "rejected-LEV10 retained-frame=%08x\n", target_level, target_x,
           target_y, target_dir, target.clut_hash, restored.frame_hash);
    return 1;
}

static int verify_track1_input_driven_pose_dispatch(Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt accepted;
    Nexus_V1_DgnViewportHostRouteReceipt rejected;

    if (!engine || !engine->mechanics || nexus_v1_load_level(engine, 1) != 0) {
        return 0;
    }
    /* A real right-turn input reaches the known complete LEV01 (0,0,1) pose. */
    nexus_v1_sync_dgn_runtime_pose(engine, 1, 0, 0, 0);
    engine->mechanics->map_index = 1;
    engine->mechanics->party_x = 0;
    engine->mechanics->party_y = 0;
    engine->mechanics->party_dir = 0;
    engine->mechanics->move_cooldown_ticks = 0;
    if (nexus_mechanics_push_command(engine->mechanics,
                                     NEXUS_CMD_TURN_RIGHT) != 0) {
        return 0;
    }
    nexus_v1_tick(engine);
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (engine->game.current_level != 1 || engine->game.party_x != 0 ||
        engine->game.party_y != 0 || engine->game.party_dir != 1 ||
        nexus_viewport_dgn_host_route_receipt(&viewport, engine, &accepted) != 0 ||
        !accepted.can_present_runtime_dgn || accepted.frame_width != NEXUS_FB_W ||
        accepted.frame_height != NEXUS_FB_H || accepted.clut_hash == 0U) {
        return 0;
    }
    if (nexus_mechanics_push_command(engine->mechanics,
                                     NEXUS_CMD_TURN_LEFT) != 0) {
        return 0;
    }
    nexus_v1_tick(engine);
    nexus_viewport_render(&viewport, engine);
    if (engine->game.current_level != 1 || engine->game.party_dir != 1 ||
        engine->mechanics->party_dir != 1 ||
        nexus_viewport_dgn_host_route_receipt(&viewport, engine, &rejected) != 0 ||
        !rejected.can_present_runtime_dgn ||
        rejected.frame_hash != accepted.frame_hash ||
        rejected.clut_hash != accepted.clut_hash) {
        return 0;
    }
    printf("Track1 input pose dispatch: turn-right accepted LEV01 frame=%08x "
           "turn-left rejected retained-clut=%08x\n", accepted.frame_hash,
           rejected.clut_hash);
    return 1;
}

static int verify_track1_item_overlay_material_gate(Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnViewportHostRouteReceipt host;
    Nexus_Viewport viewport;
    const Nexus_V1_DgnMaterialPlan *plan;
    int item_entries = 0;
    int level;

    if (!engine) return 0;
    for (level = 0; level < 16; ++level) {
        if (nexus_v1_load_level(engine, level) != 0) continue;
        if (nexus_v1_level_structure1f_spatial_receipt(
                &engine->current_level, &spatial) == 0 && spatial.valid) {
            item_entries += spatial.item_entry_count;
        }
    }
    if (nexus_v1_load_level(engine, 0) != 0) return 0;
    nexus_v1_sync_dgn_runtime_pose(engine, 0, 0, 0, 1);
    plan = nexus_v1_prepare_dgn_material_plan(engine, 0, 0, 1);
    if (!plan || !plan->receipt.plan_ready ||
        plan->receipt.structure1f_plan_item_entry_count != 0 ||
        plan->receipt.structure1f_plan_unresolved_overlay_count != 0) {
        return 0;
    }
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, engine);
    if (nexus_viewport_dgn_host_route_receipt(&viewport, engine, &host) != 0 ||
        !host.can_present_runtime_dgn || host.blocks_runtime_dgn ||
        host.rasterized_command_count != host.command_count) {
        return 0;
    }
    printf("Track1 item material gate: typed-items=%d accepted-overlay-commands=0 "
           "frame=%08x\n", item_entries, host.frame_hash);
    return 1;
}

static int verify_track1_visible_item_target_blocks_without_material(
    Nexus_V1_Engine *engine)
{
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportHostRouteReceipt host;
    int level;

    if (!engine) return 0;
    nexus_viewport_init(&viewport);
    for (level = 0; level < 16; ++level) {
        int y;
        if (nexus_v1_load_level(engine, level) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int dir;
                for (dir = 0; dir < 4; ++dir) {
                    Nexus_V1_DgnRenderCommand commands[
                        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
                    Nexus_V1_DgnRenderPlanReceipt plan;
                    Nexus_V1_DgnViewportRenderReceipt render;

                    if (nexus_v1_level_build_dgn_view_render_plan(
                            &engine->current_level, x, y, dir, commands,
                            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                            &plan) != 0 ||
                        plan.structure1f_plan_item_entry_count <= 0) {
                        continue;
                    }
                    nexus_v1_sync_dgn_runtime_pose(engine, level, x, y, dir);
                    if (nexus_v1_prepare_dgn_material_plan(engine, x, y, dir) ||
                        !engine->dgn_material_plan.receipt
                             .blocks_real_dgn_mesh_render ||
                        engine->dgn_material_plan.receipt
                                .structure1f_plan_item_entry_count <= 0 ||
                        engine->dgn_material_plan.receipt
                                .structure1f_plan_unresolved_overlay_count <= 0 ||
                        engine->dgn_material_plan.receipt
                                .fallback_visuals_permitted) {
                        return 0;
                    }
                    nexus_viewport_render(&viewport, engine);
                    if (nexus_viewport_last_dgn_render_receipt(
                            &viewport, &render) != 0 || !render.blocked ||
                        render.rasterized_command_count != 0 ||
                        render.written_pixels != 0 ||
                        render.fallback_visuals_permitted ||
                        nexus_viewport_dgn_host_route_receipt(
                            &viewport, engine, &host) != 0 ||
                        host.status !=
                            NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE1F_OBJECT_MATERIAL ||
                        !host.blocks_runtime_dgn ||
                        host.can_present_runtime_dgn ||
                        host.structure1f_visible_item_entry_count <= 0 ||
                        host.structure1f_object_material_blocker == NULL ||
                        strcmp(host.structure1f_object_material_blocker,
                               "missing-original-structure1f-item-descriptor-pixel-clut-relation") != 0 ||
                        host.structure1f_unresolved_overlay_count <= 0) {
                        return 0;
                    }
                    printf("Track1 visible item gate: level=%d pose=%d,%d,%d "
                           "items=%d unresolved=%d host=object-material "
                           "relation=descriptor-pixel-clut target=no-draw\n",
                           level, x, y, dir,
                           plan.structure1f_plan_item_entry_count,
                           plan.structure1f_plan_unresolved_overlay_count);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int verify_track1_structure1f_item_candidate_index(
    Nexus_V1_Engine *engine)
{
    Track1Structure1FItemCandidateIndex index;
    int level;
    int selector;

    if (!engine) return 0;
    memset(&index, 0, sizeof(index));
    index.first_item_id = -1;
    index.first_wall_descriptor_index = -1;
    for (level = 0; level < 16; ++level) {
        int entry;
        if (nexus_v1_load_level(engine, level) != 0) continue;
        for (entry = 0; entry < engine->current_level.structure1f_entry_count;
             ++entry) {
            const Nexus_V1_DgnStructure1FEntry *record =
                &engine->current_level.structure1f_entries[entry];
            if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
            ++index.typed_item_count;
            if (index.first_item_id < 0) index.first_item_id = record->item_id;
        }
    }
    for (selector = 0; selector < NEXUS_DMDF_MATERIAL_COUNT; ++selector) {
        const Nexus_DMDFTextureSurface *floor =
            &engine->floor_materials.surfaces[selector];
        const Nexus_DMDFTextureSurface *wall =
            &engine->wall_materials.surfaces[selector];
        if (surface_has_proven_text_pixels_clut(floor)) {
            ++index.floor_descriptor_pixel_clut_candidates;
        }
        if (surface_has_proven_text_pixels_clut(wall)) {
            ++index.wall_descriptor_pixel_clut_candidates;
            if (index.first_wall_descriptor_index < 0) {
                index.first_wall_descriptor_index =
                    (int)wall->source_text_descriptor_index;
                index.first_wall_pixel_offset = wall->source_text_pixel_offset;
            }
        }
    }
    /* Structure1F item_id is typed source data, not a documented MNS
     * descriptor reference. Do not turn matching numeric values into a
     * material route without an explicit retail relation. */
    if (index.typed_item_count <= 0 || index.first_item_id < 0 ||
        index.floor_descriptor_pixel_clut_candidates <= 0 ||
        index.wall_descriptor_pixel_clut_candidates <= 0 ||
        index.first_wall_descriptor_index < 0 ||
        index.proven_item_material_bindings != 0 ||
        index.materialized_item_candidates != 0) {
        return 0;
    }
    printf("Track1 Structure1F item candidate index: items=%d first-item=%d "
           "floor-candidates=%d wall-candidates=%d wall-descriptor=%d "
           "wall-pixel=%u proven-bindings=0 materialized=0\n",
           index.typed_item_count, index.first_item_id,
           index.floor_descriptor_pixel_clut_candidates,
           index.wall_descriptor_pixel_clut_candidates,
           index.first_wall_descriptor_index, index.first_wall_pixel_offset);
    return 1;
}

static int verify_track1_structure1f_item_material_source_block(
    Nexus_V1_Engine *engine)
{
    Track1Structure1FItemMaterialBlockReceipt receipt;
    uint8_t *item_ibs;
    int level;

    if (!engine) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.first_item_id = -1;
    receipt.first_alcove_item_id = -1;
    item_ibs = nexus_v1_read_file(engine, "ITEM.IBS", &receipt.item_ibs_bytes);
    if (!item_ibs || receipt.item_ibs_bytes <= 0) {
        free(item_ibs);
        return 0;
    }
    receipt.item_ibs_fnv1a64 = fnv1a64_bytes(item_ibs, receipt.item_ibs_bytes);
    free(item_ibs);

    for (level = 0; level < 16; ++level) {
        int entry;
        if (nexus_v1_load_level(engine, level) != 0) continue;
        for (entry = 0; entry < engine->current_level.structure1f_entry_count;
             ++entry) {
            const Nexus_V1_DgnStructure1FEntry *record =
                &engine->current_level.structure1f_entries[entry];
            if (record->family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS) {
                ++receipt.typed_item_count;
                if (receipt.first_item_id < 0) {
                    receipt.first_item_id = record->item_id;
                }
            } else if (record->family == NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
                ++receipt.alcove_item_count;
                if (receipt.first_alcove_item_id < 0) {
                    receipt.first_alcove_item_id = record->item_id;
                }
            }
        }
    }

    /* No equality, range, or byte-offset comparison is permitted here: that
     * would manufacture the missing source relation. The zero receipts are
     * the fail-closed contract until an original descriptor binding exists. */
    if (receipt.item_ibs_fnv1a64 == 0 || receipt.typed_item_count <= 0 ||
        receipt.first_item_id < 0 || receipt.item_descriptor_references != 0 ||
        receipt.item_pixel_references != 0 || receipt.item_clut_references != 0 ||
        receipt.materialized_item_rasters != 0) {
        return 0;
    }
    printf("Track1 Structure1F->ITEM.IBS source block: item-ibs=%d "
           "fnv=%016llx items=%d first-item=%d alcove-items=%d "
           "first-alcove-item=%d descriptor-refs=0 "
           "pixel-refs=0 clut-refs=0 rasters=0 target=no-draw\n",
           receipt.item_ibs_bytes,
           (unsigned long long)receipt.item_ibs_fnv1a64,
           receipt.typed_item_count, receipt.first_item_id,
           receipt.alcove_item_count, receipt.first_alcove_item_id);
    return 1;
}

static void report_category_coverage(
    const char *label,
    const Nexus_V1_DgnMaterialCategoryCoverageReceipt *coverage)
{
    if (!label || !coverage) return;
    printf("%s selector availability: commands=%u unique=%u covered-unique=%u "
           "missing=%u first-missing=%u covered=%d fallback=%d\n",
           label, coverage->command_count, coverage->unique_material_id_count,
           coverage->covered_unique_material_id_count,
           coverage->missing_material_count, coverage->first_missing_material_id,
           coverage->covered, coverage->fallback_visuals_permitted);
}

typedef struct {
    int descriptor_selector_count;
    int pixel_selector_count;
    int opaque_clut_selector_count;
    int fully_proven_selector_count;
    int first_descriptor_without_pixels;
    int first_descriptor_without_opaque_clut;
} WallMnsEvidenceReceipt;

/* This is an inventory receipt over the canonical SN_WALL.MNS bank already
 * bounded by the TEXT parser.  It does not promote a selector to rendering. */
static int inspect_wall_mns_evidence(const Nexus_V1_Engine *engine,
                                     WallMnsEvidenceReceipt *out)
{
    int selector;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->first_descriptor_without_pixels = -1;
    out->first_descriptor_without_opaque_clut = -1;
    if (!engine || !engine->dgn_static_material_sources
                         .wall_mns.canonical_hash_verified ||
        !engine->wall_mns_material_route_valid) {
        return 0;
    }

    for (selector = 0; selector < NEXUS_DMDF_MATERIAL_COUNT; ++selector) {
        const Nexus_DMDFTextureSurface *surface =
            &engine->wall_materials.surfaces[selector];
        int palette_index;
        int opaque_clut = 0;

        if (!surface->source_text_descriptor_bound) continue;
        ++out->descriptor_selector_count;
        if (!surface->pixels) {
            if (out->first_descriptor_without_pixels < 0) {
                out->first_descriptor_without_pixels = selector;
            }
            continue;
        }
        ++out->pixel_selector_count;
        for (palette_index = 0; palette_index < 256; ++palette_index) {
            if ((surface->palette[palette_index] >> 24) != 0U) {
                opaque_clut = 1;
                break;
            }
        }
        if (!opaque_clut) {
            if (out->first_descriptor_without_opaque_clut < 0) {
                out->first_descriptor_without_opaque_clut = selector;
            }
            continue;
        }
        ++out->opaque_clut_selector_count;
        ++out->fully_proven_selector_count;
    }
    return out->descriptor_selector_count > 0;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_DgnMaterialCorpusReceipt corpus;
    Nexus_V1_DgnNoDrawSelectorAuditReceipt no_draw_audit;
    int ceiling_audit_count;
    int wall_audit_count;
    int unbound_ceiling_candidates;
    int unbound_wall_candidates;
    WallMnsEvidenceReceipt wall_mns_evidence;
    const char *root = argc > 1 ? argv[1] : NULL;

    memset(&engine, 0, sizeof(engine));
    if (!root || nexus_v1_init(&engine, root) != 0) {
        puts("SKIP: Nexus Track 1 data unavailable");
        return 0;
    }
    check(engine.dgn_static_material_sources.canonical_pair_bound,
          "only the canonical SN_FLOOR/SN_WALL pair may enter this receipt");
    check(verify_mns(&engine, "SN_FLOOR.MNS"),
          "SN_FLOOR.MNS descriptor footprints bind decoded material surfaces");
    check(verify_mns(&engine, "SN_WALL.MNS"),
          "SN_WALL.MNS descriptor footprints bind decoded material surfaces");
    memset(&corpus, 0, sizeof(corpus));
    check(nexus_v1_inspect_dgn_material_corpus(&engine, &corpus) == 0 &&
              corpus.static_mns_host_route_complete &&
              corpus.floor_coverage.covered &&
              !corpus.ceiling_coverage.covered &&
              !corpus.wall_coverage.covered &&
              !corpus.bpk_host_routes_complete,
          "DGN consumes only covered MNS floor selectors and blocks remaining selectors");
    report_category_coverage("Canonical ceiling", &corpus.ceiling_coverage);
    report_category_coverage("Canonical wall", &corpus.wall_coverage);
    check(inspect_wall_mns_evidence(&engine, &wall_mns_evidence) &&
              wall_mns_evidence.pixel_selector_count <=
                  wall_mns_evidence.descriptor_selector_count &&
              wall_mns_evidence.opaque_clut_selector_count <=
                  wall_mns_evidence.pixel_selector_count &&
              wall_mns_evidence.fully_proven_selector_count <=
                  wall_mns_evidence.opaque_clut_selector_count,
          "canonical SN_WALL.MNS evidence receipt remains bounded by descriptor pixels and CLUT");
    printf("Canonical wall MNS evidence: descriptor=%d pixels=%d opaque-CLUT=%d "
           "fully-proven=%d first-DGN-missing=%u first-no-pixels=%d "
           "first-no-opaque-CLUT=%d\n",
           wall_mns_evidence.descriptor_selector_count,
           wall_mns_evidence.pixel_selector_count,
           wall_mns_evidence.opaque_clut_selector_count,
           wall_mns_evidence.fully_proven_selector_count,
           corpus.wall_coverage.first_missing_material_id,
           wall_mns_evidence.first_descriptor_without_pixels,
           wall_mns_evidence.first_descriptor_without_opaque_clut);
    check(corpus.ceiling_coverage.category ==
              NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING &&
              corpus.ceiling_coverage.command_count > 0U &&
              corpus.ceiling_coverage.missing_material_count > 0U &&
              corpus.ceiling_coverage.covered_unique_material_id_count <
                  corpus.ceiling_coverage.unique_material_id_count &&
              !corpus.ceiling_coverage.covered &&
              !corpus.ceiling_coverage.fallback_visuals_permitted,
          "canonical MNS ceiling selectors remain explicitly unavailable without fallback");
    check(corpus.wall_coverage.category == NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL &&
              corpus.wall_coverage.command_count > 0U &&
              corpus.wall_coverage.missing_material_count > 0U &&
              corpus.wall_coverage.covered_unique_material_id_count <
                  corpus.wall_coverage.unique_material_id_count &&
              !corpus.wall_coverage.covered &&
              !corpus.wall_coverage.fallback_visuals_permitted,
          "canonical MNS wall selectors remain explicitly unavailable without fallback");
    check(corpus.structure2_opaque_prs3_magic_count == 0 &&
              corpus.structure2_material_or_image_data_proven_level_count == 0,
          "Structure2 opaque payloads contain no PRS3 material bridge");
    check(scan_unbound_mns_candidates(
              &engine, corpus.ceiling_coverage.first_missing_material_id,
              corpus.wall_coverage.first_missing_material_id,
              &unbound_ceiling_candidates, &unbound_wall_candidates) &&
              engine.dgn_static_material_sources.canonical_pair_bound &&
              !engine.dgn_static_material_sources.fallback_visuals_permitted,
          "unbound MNS candidates remain inventory-only outside the canonical route");
    printf("Unbound MNS descriptor candidates: ceiling=%d wall=%d\n",
           unbound_ceiling_candidates, unbound_wall_candidates);
    memset(&no_draw_audit, 0, sizeof(no_draw_audit));
    check(nexus_v1_audit_dgn_no_draw_selectors(&engine, &no_draw_audit) == 0 &&
              no_draw_audit.canonical_static_mns_route &&
              !no_draw_audit.fallback_visuals_permitted &&
              no_draw_audit.parsed_level_count == 16 &&
              no_draw_audit.leading_missing_ceiling_selector_count == 1U &&
              no_draw_audit.leading_missing_wall_selector_count == 69U,
          "engine no-draw audit aggregates canonical ceiling and wall selector gaps");
    check(verify_missing_selector_blocks(
              &engine, corpus.ceiling_coverage.first_missing_material_id,
              NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
              NEXUS_V1_DGN_MATERIAL_REJECTION_CANONICAL_MNS_CEILING_SELECTOR),
          "first real missing ceiling selector reaches a no-fallback DGN plan gate");
    check(verify_missing_selector_blocks(
              &engine, corpus.wall_coverage.first_missing_material_id,
              NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT,
              NEXUS_V1_DGN_MATERIAL_REJECTION_CANONICAL_MNS_WALL_SELECTOR),
          "first real missing wall selector reaches a no-fallback DGN plan gate");
    check(verify_covered_wall_selector_enters_plan(&engine),
          "covered SN_WALL.MNS selectors enter typed plans while incomplete walls remain blocked");
    check(verify_real_wall_text_raster(&engine),
          "a proven SN_WALL.MNS selector produces indexed pixels through the existing TEXT rasterizer");
    check(verify_real_selector_zero_floor_wall_composition(&engine),
          "real selector-0 floor/wall composition uses one indexed CLUT with exact pixel receipts");
    check(verify_next_real_selector_floor_wall_composition(&engine),
          "next real selector composition preserves indexed accounting while its incomplete viewport stays no-draw");
    check(survey_real_complete_floor_wall_poses(&engine),
          "real Track1 pose survey records only fully material-complete floor/wall frames");
    check(verify_track1_complete_present_receipt(&engine),
          "one real complete Track1 pose produces a stable host present receipt");
    check(verify_track1_champion_start_runtime_admission(&engine),
          "Track1 champion-start host admission accepts only complete present identity and denies selector-1/193");
    check(verify_track1_runtime_pose_transition_dispatch(&engine),
          "runtime pose dispatch commits only complete next frames and retains the prior frame on material rejection");
    check(verify_track1_cross_level_pose_transition_dispatch(&engine),
          "cross-level pose dispatch commits only complete target frames and restores the prior level on material rejection");
    check(verify_track1_input_driven_pose_dispatch(&engine),
          "runtime input dispatch accepts a complete pose and retains the prior frame on incomplete input target");
    check(verify_track1_item_overlay_material_gate(&engine),
          "Structure1F items stay source-typed but cannot enter a complete frame without proven material binding");
    check(verify_track1_visible_item_target_blocks_without_material(&engine),
          "visible Structure1F item targets reject their entire frame without a proven material path");
    check(verify_track1_structure1f_item_candidate_index(&engine),
          "Structure1F item candidates retain exact MNS evidence without inventing item-to-material bindings");
    check(verify_track1_structure1f_item_material_source_block(&engine),
          "raw ITEM.IBS provenance cannot materialize Structure1F objects without a proven descriptor/pixel/CLUT relation");
    check(audit_missing_selector_plans(
              &engine, NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
              NEXUS_V1_DGN_MATERIAL_REJECTION_CANONICAL_MNS_CEILING_SELECTOR,
              &ceiling_audit_count),
          "every plan-leading missing ceiling selector retains the canonical reason");
    check(audit_missing_selector_plans(
              &engine, NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT,
              NEXUS_V1_DGN_MATERIAL_REJECTION_CANONICAL_MNS_WALL_SELECTOR,
              &wall_audit_count),
          "every plan-leading missing wall selector retains the canonical reason");
    printf("DGN no-draw selector audit: ceiling=%d wall=%d\n",
           ceiling_audit_count, wall_audit_count);
    check(verify_covered_floor_selector_rasters(&engine),
          "covered DGN floor selector reaches indexed MNS raster without BPK");
    nexus_v1_shutdown(&engine);
    return failures ? 1 : 0;
}
