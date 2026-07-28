
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

static int viewport_find_palette_index(const uint32_t palette[256],
                                       const uint8_t occupied[256],
                                       uint32_t rgba)
{
    int i;
    for (i = 0; i < 256; ++i) {
        if ((!occupied || occupied[i]) && palette[i] == rgba) return i;
    }
    return -1;
}

static void viewport_surface_palette_map(Nexus_Viewport *vp,
                                         const Nexus_DMDFTextureSurface *surface,
                                         uint8_t texel_map[256])
{
    int i;
    if (!texel_map) return;
    for (i = 0; i < 256; ++i) texel_map[i] = (uint8_t)i;
    if (!vp || !surface || !surface->valid) return;
    for (i = 0; i < 256; ++i) {
        int mapped = viewport_find_palette_index(vp->fb.palette, NULL,
                                                 surface->palette[i]);
        if (mapped >= 0) texel_map[i] = (uint8_t)mapped;
    }
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
    for (i = 0; i < 16; ++i) occupied[i] = 1;
    for (i = 0; i < plan->receipt.command_count; ++i) {
        const Nexus_DMDFTextureSurface *surface =
            viewport_plan_surface(engine, &plan->commands[i]);
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
        }
    }
    nexus_fb_set_palette(&vp->fb, palette);
    vp->material_engine = engine;
    vp->material_generation = plan->generation;
    vp->material_palette_valid = 1;
    return 1;
}

/* Stage one real static-textured Structure3 face from the active package for
 * a later Saturn-backed renderer. This intentionally has no fallback when a
 * level has no such source row, and it never turns package bytes into pixels. */
static int viewport_consume_structure3_package_geometry(
    void *context, const Nexus_V1_DgnStructure3PackageGeometryPacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid ||
        !packet->package_fnv1a64 || packet->package_fnv1a64 != packet->source_bytes_fnv1a64 || !packet->face_length || !packet->face_fnv1a64 || packet->face_offset > (uint32_t)packet->source_byte_count || packet->face_length > (uint32_t)packet->source_byte_count - packet->face_offset ||
        !packet->descriptor_fnv1a64 || !packet->image_length || packet->image_offset > (uint32_t)packet->source_byte_count || packet->image_length > (uint32_t)packet->source_byte_count - packet->image_offset || (packet->palette_offset && (!packet->palette_length || packet->palette_offset > (uint32_t)packet->source_byte_count || packet->palette_length > (uint32_t)packet->source_byte_count - packet->palette_offset)) ||
        !packet->material_target.image_payload_interval_bound ||
        packet->material_target.image_payload_candidate_byte_count == 0U ||
        (packet->material_target.descriptor_target.descriptor
                 .palette_relative_offset != 0U &&
         (!packet->material_target.palette_payload_interval_bound ||
          packet->material_target.palette_payload_candidate_byte_count == 0U))) {
        return -1;
    }
    /* Keep an exact source exemplar for inspection; traversal completion and
     * counts below cover every static package face, not just this first one. */
    if (!vp->structure3_package_geometry.valid)
        vp->structure3_package_geometry = *packet;
    return 0;
}

static void viewport_stage_structure3_package_geometry(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure3_package_geometry, 0,
           sizeof(vp->structure3_package_geometry));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure3_package_geometry(
                       engine, viewport_consume_structure3_package_geometry,
                       vp, &scene) != 1 || !scene.valid || !scene.complete ||
        scene.static_material_face_count <= 0 ||
        scene.static_material_face_count != scene.consumed_face_count ||
        !vp->structure3_package_geometry.valid) return;
    vp->last_dgn_render_receipt.structure3_package_geometry_consumed = 1;
    vp->last_dgn_render_receipt.structure3_package_geometry_bound =
        vp->structure3_package_geometry.source_geometry_bound &&
        vp->structure3_package_geometry.material_descriptor_bound;
    vp->last_dgn_render_receipt.structure3_package_geometry_no_draw =
        scene.no_draw_only && scene.blocks_real_dgn_mesh_render &&
        !scene.decoder_permitted;
    vp->last_dgn_render_receipt.structure3_package_geometry_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure3_package_geometry_scene = scene;
}

static int viewport_consume_structure3_animated_material(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialPacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid) return -1;
    if (!vp->structure3_animated_material.valid)
        vp->structure3_animated_material = *packet;
    return 0;
}

static void viewport_stage_structure3_animated_materials(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure3_animated_material, 0,
           sizeof(vp->structure3_animated_material));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure3_animated_materials(
                       engine, viewport_consume_structure3_animated_material,
                       vp, &scene) != 1 || !scene.valid || !scene.complete ||
        scene.animated_face_count <= 0 ||
        scene.animated_face_count != scene.consumed_face_count ||
        !vp->structure3_animated_material.valid) return;
    vp->last_dgn_render_receipt.structure3_animated_material_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure3_animated_material_scene = scene;
}

static int viewport_consume_structure3_animated_material_image(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialImagePacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid) return -1;
    if (!vp->structure3_animated_material_image.valid)
        vp->structure3_animated_material_image = *packet;
    return 0;
}

static void viewport_stage_structure3_animated_material_images(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure3_animated_material_image, 0,
           sizeof(vp->structure3_animated_material_image));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure3_animated_material_images(
                       engine, viewport_consume_structure3_animated_material_image,
                       vp, &scene) != 1 || !scene.valid || !scene.complete ||
        scene.consumed_image_instruction_count <= 0 ||
        scene.consumed_image_instruction_count !=
            scene.declared_image_instruction_count ||
        !vp->structure3_animated_material_image.valid) return;
    vp->last_dgn_render_receipt
        .structure3_animated_material_image_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure3_animated_material_image_scene = scene;
}

static int viewport_consume_structure3_untextured_face(
    void *context, const Nexus_V1_DgnStructure3UntexturedFacePacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid) return -1;
    if (!vp->structure3_untextured_face.valid)
        vp->structure3_untextured_face = *packet;
    return 0;
}

static void viewport_stage_structure3_untextured_faces(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure3_untextured_face, 0,
           sizeof(vp->structure3_untextured_face));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure3_untextured_faces(
                       engine, viewport_consume_structure3_untextured_face,
                       vp, &scene) != 1 || !scene.valid || !scene.complete ||
        scene.untextured_face_count <= 0 ||
        scene.untextured_face_count != scene.consumed_face_count ||
        !vp->structure3_untextured_face.valid) return;
    vp->last_dgn_render_receipt.structure3_untextured_face_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure3_untextured_face_scene = scene;
}

static void viewport_stage_structure3_complete_source_scene(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt scene;

    if (!vp) return;
    memset(&scene, 0, sizeof(scene));
    if (!engine ||
        nexus_v1_current_level_structure3_complete_source_scene_receipt(
            engine, &scene) != 1 || !scene.valid ||
        !scene.category_coverage_complete ||
        !scene.animated_payload_coverage_complete ||
        !scene.structure2_payload_coverage_complete ||
        scene.structure2_descriptor_count <= 0 ||
        scene.structure2_image_anchor_count != scene.structure2_descriptor_count ||
        scene.structure2_payload_anchors_consumed !=
            scene.structure2_payload_anchor_count ||
        scene.traversed_face_count != scene.face_count ||
        !scene.no_draw_only || scene.decoder_permitted ||
        scene.fallback_visuals_permitted ||
        !scene.blocks_real_dgn_mesh_render) return;
    vp->last_dgn_render_receipt.structure3_complete_source_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure3_complete_source_scene = scene;
}

static int viewport_consume_structure1f_source(
    void *context, const Nexus_V1_DgnStructure1FSourcePacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render || !packet->package_fnv1a64 ||
        packet->package_fnv1a64 != packet->source_bytes_fnv1a64 ||
        !packet->descriptor_length || !packet->descriptor_fnv1a64 ||
        packet->descriptor_offset > (uint32_t)packet->source_byte_count ||
        packet->descriptor_length > (uint32_t)packet->source_byte_count - packet->descriptor_offset) return -1;
    if (!vp->structure1f_source_packet.valid)
        vp->structure1f_source_packet = *packet;
    return 0;
}

static void viewport_stage_structure1f_source_scene(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure1FSourceSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure1f_source_packet, 0,
           sizeof(vp->structure1f_source_packet));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure1f_source_scene(
                       engine, viewport_consume_structure1f_source, vp,
                       &scene) != 1 || !scene.valid ||
        !scene.family_coverage_complete ||
        scene.consumed_entry_count != scene.entry_count ||
        (scene.entry_count > 0 && !vp->structure1f_source_packet.valid) ||
        !scene.no_draw_only || scene.fallback_visuals_permitted ||
        !scene.blocks_real_dgn_mesh_render) return;
    vp->last_dgn_render_receipt.structure1f_source_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure1f_source_scene = scene;
}

static void viewport_stage_structure1f2_face_adjacency_transform(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    int entry_index;

    if (!vp || !engine) return;
    memset(&vp->structure1f2_face_adjacency_transform, 0,
           sizeof(vp->structure1f2_face_adjacency_transform));
    for (entry_index = 0;
         entry_index < engine->current_level.structure1f_entry_count;
         ++entry_index) {
        Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt receipt;

        memset(&receipt, 0, sizeof(receipt));
        if (nexus_v1_engine_build_structure1f2_face_adjacency_transform_receipt(
                engine, entry_index, &receipt) != 1 || !receipt.valid ||
            !nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
                engine, &receipt)) continue;
        vp->structure1f2_face_adjacency_transform = receipt;
        vp->last_dgn_render_receipt
            .structure1f2_face_adjacency_transform_consumed = 1;
        vp->last_dgn_render_receipt.structure1f2_face_adjacency_transform =
            receipt;
        return;
    }
}

static void viewport_stage_m11_direct_lev_dungeon_no_draw(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnM11DirectLevNoDrawReceipt receipt;

    if (!vp || !engine) return;
    memset(&receipt, 0, sizeof(receipt));
    if (!engine->m11_direct_lev_dungeon_no_draw_valid ||
        !nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
            engine, engine->m11_direct_lev_dungeon_route_epoch,
            engine->m11_direct_lev_dungeon.level_index,
            engine->m11_direct_lev_dungeon.dgn_md5,
            engine->m11_direct_lev_dungeon.dgn_byte_count,
            engine->m11_direct_lev_dungeon.dgn_fnv1a64, &receipt) ||
        !receipt.no_draw_only || receipt.fallback_visuals_permitted ||
        !receipt.blocks_real_dgn_mesh_render) return;
    vp->last_dgn_render_receipt.m11_direct_lev_dungeon_no_draw_consumed = 1;
    vp->last_dgn_render_receipt.m11_direct_lev_dungeon_no_draw = receipt;
}

static int viewport_consume_structure1c_source(
    void *context, const Nexus_V1_DgnStructure1CSourcePacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) return -1;
    if (!vp->structure1c_source_packet.valid)
        vp->structure1c_source_packet = *packet;
    return 0;
}

static void viewport_stage_structure1c_source_scene(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure1CSourceSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure1c_source_packet, 0,
           sizeof(vp->structure1c_source_packet));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure1c_source_scene(
                       engine, viewport_consume_structure1c_source, vp,
                       &scene) != 1 || !scene.valid ||
        scene.consumed_record_count != scene.indexed_record_count ||
        !vp->structure1c_source_packet.valid || !scene.no_draw_only ||
        scene.fallback_visuals_permitted || !scene.blocks_real_dgn_mesh_render)
        return;
    vp->last_dgn_render_receipt.structure1c_source_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure1c_source_scene = scene;
}

static int viewport_consume_structure2_payload_anchor(
    void *context, const Nexus_V1_DgnStructure2PayloadAnchorPacket *packet)
{
    Nexus_Viewport *vp = (Nexus_Viewport *)context;

    if (!vp || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render ||
        packet->candidate_byte_count == 0U ||
        packet->next_anchor_offset <= packet->payload_anchor_offset) return -1;
    if (!vp->structure2_payload_anchor_packet.valid)
        vp->structure2_payload_anchor_packet = *packet;
    return 0;
}

static void viewport_stage_structure2_payload_anchors(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt scene;

    if (!vp) return;
    memset(&vp->structure2_payload_anchor_packet, 0,
           sizeof(vp->structure2_payload_anchor_packet));
    memset(&scene, 0, sizeof(scene));
    if (!engine || nexus_v1_current_level_visit_structure2_payload_anchors(
                       engine, viewport_consume_structure2_payload_anchor, vp,
                       &scene) != 1 || !scene.valid ||
        scene.descriptor_count <= 0 || scene.anchor_count <= 0 ||
        scene.consumed_anchor_count != scene.anchor_count ||
        scene.image_anchor_count != scene.descriptor_count ||
        !vp->structure2_payload_anchor_packet.valid || !scene.no_draw_only ||
        scene.fallback_visuals_permitted || !scene.blocks_real_dgn_mesh_render)
        return;
    vp->last_dgn_render_receipt.structure2_payload_anchor_scene_consumed = 1;
    vp->last_dgn_render_receipt.structure2_payload_anchor_scene = scene;
}

static void viewport_structure3_face_vertices(
    Nexus_RasterVertex *rv, const Nexus_V1_DgnStructure3Vector *vertices,
    int slot_count)
{
    int slot;
    for (slot = 0; slot < slot_count; ++slot) {
        rv[slot].position.x = (float)vertices[slot].x / 65536.0f;
        rv[slot].position.y = (float)vertices[slot].y / 65536.0f;
        rv[slot].position.z = (float)vertices[slot].z / 65536.0f;
        rv[slot].color = 0;
        rv[slot].texture_id = -1;
        rv[slot].uv.x = 0.0f;
        rv[slot].uv.y = 0.0f;
    }
}

static int viewport_render_structure3_mesh(
    Nexus_Viewport *vp, const Nexus_V1_Engine *engine)
{
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt scene;
    int entry_index, face_count_total = 0, textured_count = 0;
    int flat_color_count = 0, animated_static_fallback_count = 0;
    uint16_t flat_colors[254];
    int flat_color_palette_count = 0;

    if (!vp || !engine || !engine->level_loaded) return 0;
    if (nexus_v1_current_level_structure3_complete_source_scene_receipt(
            engine, &scene) != 1 || !scene.valid || !scene.decoder_permitted)
        return 0;

    for (entry_index = 0;
         entry_index < engine->current_level.structure3_directory.entry_count;
         ++entry_index) {
        int face_ordinal;
        int entry_face_count =
            engine->current_level.structure3_entry_face_counts[entry_index];
        for (face_ordinal = 0; face_ordinal < entry_face_count;
             ++face_ordinal) {
            Nexus_V1_DgnStructure3PackageGeometryPacket packet;
            Nexus_RasterVertex rv[4];
            int slot_count;

            ++face_count_total;
            if (nexus_v1_current_level_structure3_package_geometry_packet(
                    engine, (uint32_t)entry_index, (uint32_t)face_ordinal,
                    &packet) != 1 || !packet.valid)
                continue;
            slot_count = packet.vertex_slot_count;
            viewport_structure3_face_vertices(
                rv, packet.vertices, slot_count);

            if (packet.texture_surface_valid &&
                packet.texture_surface_index >= 0 &&
                packet.texture_surface_index < engine->structure2_surface_count) {
                const Nexus_DMDFTextureSurface *surface =
                    &engine->structure2_surfaces[packet.texture_surface_index];
                if (!surface->valid || !surface->pixels) continue;
                {
                    int pc;
                    for (pc = 0; pc < 256 && pc < (int)(sizeof(surface->palette) /
                         sizeof(surface->palette[0])); ++pc)
                        vp->fb.palette[pc] = surface->palette[pc];
                }
                rv[0].uv.x = 0.0f; rv[0].uv.y = 0.0f;
                rv[1].uv.x = 1.0f; rv[1].uv.y = 0.0f;
                rv[2].uv.x = 1.0f; rv[2].uv.y = 1.0f;
                if (slot_count == 4) {
                    rv[3].uv.x = 0.0f; rv[3].uv.y = 1.0f;
                    nexus_raster_quad_tex(&vp->fb, rv[0], rv[1], rv[2],
                        rv[3], &vp->cam, surface->pixels, surface->width,
                        surface->height, surface->palette);
                } else {
                    nexus_raster_triangle_tex(&vp->fb, rv[0], rv[1], rv[2],
                        &vp->cam, surface->pixels, surface->width,
                        surface->height, surface->palette);
                }
                ++textured_count;
            } else if ((packet.face.fill_selector & 0xFF00U) == 0x0800U) {
                Nexus_V1_DgnStructure3AnimatedMaterialPacket anim;
                memset(&anim, 0, sizeof(anim));
                if (nexus_v1_current_level_structure3_animated_material_packet(
                        engine, (uint32_t)entry_index,
                        (uint32_t)face_ordinal, &anim) == 1 &&
                    anim.valid && anim.first_descriptor_bound &&
                    anim.first_structure2_image_id >= 0 &&
                    anim.first_structure2_image_id <
                        engine->structure2_surface_count) {
                    const Nexus_DMDFTextureSurface *surface =
                        &engine->structure2_surfaces[
                            anim.first_structure2_image_id];
                    if (surface->valid && surface->pixels) {
                        int pc;
                        for (pc = 0; pc < 256; ++pc)
                            vp->fb.palette[pc] = surface->palette[pc];
                        rv[0].uv.x = 0.0f; rv[0].uv.y = 0.0f;
                        rv[1].uv.x = 1.0f; rv[1].uv.y = 0.0f;
                        rv[2].uv.x = 1.0f; rv[2].uv.y = 1.0f;
                        if (slot_count == 4) {
                            rv[3].uv.x = 0.0f; rv[3].uv.y = 1.0f;
                            nexus_raster_quad_tex(&vp->fb, rv[0], rv[1],
                                rv[2], rv[3], &vp->cam, surface->pixels,
                                surface->width, surface->height,
                                surface->palette);
                        } else {
                            nexus_raster_triangle_tex(&vp->fb, rv[0], rv[1],
                                rv[2], &vp->cam, surface->pixels,
                                surface->width, surface->height,
                                surface->palette);
                        }
                        ++animated_static_fallback_count;
                    }
                }
            } else if (packet.face.fill_selector & 0x8000U) {
                uint16_t color15 = packet.face.fill_selector & 0x7FFFU;
                int ci = -1, k;
                for (k = 0; k < flat_color_palette_count; ++k) {
                    if (flat_colors[k] == color15) {
                        ci = k + 1;
                        break;
                    }
                }
                if (ci < 0 && flat_color_palette_count < 254) {
                    ci = flat_color_palette_count + 1;
                    flat_colors[flat_color_palette_count] = color15;
                    vp->fb.palette[ci] =
                        nexus_v1_saturn_15bit_to_rgba(color15);
                    ++flat_color_palette_count;
                }
                if (ci < 0) continue;
                rv[0].color = (uint8_t)ci;
                rv[1].color = (uint8_t)ci;
                rv[2].color = (uint8_t)ci;
                if (slot_count == 4) {
                    rv[3].color = (uint8_t)ci;
                    nexus_raster_quad(&vp->fb, rv[0], rv[1], rv[2], rv[3],
                        &vp->cam);
                } else {
                    nexus_raster_triangle(&vp->fb, rv[0], rv[1], rv[2],
                        &vp->cam);
                }
                ++flat_color_count;
            }
        }
    }
    if (textured_count > 0 || flat_color_count > 0 ||
        animated_static_fallback_count > 0) {
        vp->last_dgn_render_receipt.structure3_mesh_rendered = 1;
        vp->last_dgn_render_receipt.structure3_mesh_face_count =
            face_count_total;
        vp->last_dgn_render_receipt.structure3_mesh_textured_face_count =
            textured_count;
        vp->last_dgn_render_receipt.structure3_mesh_flat_color_face_count =
            flat_color_count;
        vp->last_dgn_render_receipt
            .structure3_mesh_animated_static_fallback_face_count =
            animated_static_fallback_count;
        vp->last_dgn_render_receipt.written_pixels =
            viewport_count_written_pixels(&vp->fb);
        vp->last_dgn_render_receipt.frame_hash =
            viewport_dgn_frame_hash(&vp->fb);
        vp->last_dgn_render_receipt.captured_frame_ready =
            vp->last_dgn_render_receipt.written_pixels > 0 &&
            vp->last_dgn_render_receipt.frame_hash != 0u;
        vp->last_dgn_render_receipt.ready =
            vp->last_dgn_render_receipt.captured_frame_ready;
        vp->last_dgn_render_receipt.attempted = 1;
        vp->last_dgn_render_receipt.used_real_dgn_route = 1;
        return 1;
    }
    return 0;
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
        Nexus_V1_DgnStructure3RenderPacket structure3_packet;
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
        if (nexus_v1_current_level_dgn_renderer_source_receipt(
                engine, &vp->last_dgn_render_receipt.active_level_source) > 0) {
            vp->last_dgn_render_receipt.active_level_source_consumed = 1;
        }
        memset(&vp->structure3_source_packet, 0,
               sizeof(vp->structure3_source_packet));
        if (nexus_v1_current_level_structure3_render_packet(
                engine, &structure3_packet) > 0) {
            /* The viewport keeps only the authenticated geometry packet. It
             * does not inspect or rasterize its opaque Saturn state. */
            vp->structure3_source_packet = structure3_packet;
            vp->last_dgn_render_receipt.structure3_source_packet_consumed = 1;
            vp->last_dgn_render_receipt.structure3_source_geometry_bound =
                structure3_packet.source_geometry_bound;
            vp->last_dgn_render_receipt.structure3_source_no_draw =
                structure3_packet.no_draw_only &&
                structure3_packet.blocks_real_dgn_mesh_render;
        }
        vp->last_dgn_render_receipt.structure3_runtime_vdp1_command_framed =
            nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
                engine, &vp->last_dgn_render_receipt
                             .structure3_runtime_vdp1_command) == 1;
        vp->last_dgn_render_receipt
            .structure3_runtime_vdp1_texture_format_framed =
            vp->last_dgn_render_receipt.structure3_runtime_vdp1_command_framed &&
            vp->last_dgn_render_receipt.structure3_runtime_vdp1_command
                .texture_format_framed &&
            vp->last_dgn_render_receipt.structure3_runtime_vdp1_command
                .texture_span_size_matches_command;
        vp->last_dgn_render_receipt
            .structure3_runtime_vdp1_coordinate_words_framed =
            vp->last_dgn_render_receipt.structure3_runtime_vdp1_command_framed &&
            vp->last_dgn_render_receipt.structure3_runtime_vdp1_command
                .coordinate_words_framed;
        vp->last_dgn_render_receipt
            .structure3_runtime_vdp1_vram_window_bound =
            nexus_v1_current_level_structure3_vdp1_vram_window_receipt(
                engine, &vp->last_dgn_render_receipt
                             .structure3_runtime_vdp1_vram_window) == 1;
        vp->last_dgn_render_receipt
            .structure3_runtime_vdp1_command_vram_bound =
            nexus_v1_current_level_structure3_vdp1_command_vram_receipt(
                engine, &vp->last_dgn_render_receipt
                             .structure3_runtime_vdp1_command_vram) == 1;
        /* This is intentionally independent of the external capture packet.
         * A real package face may be staged, but never rasterized, before
         * Saturn pixel/palette/VDP1 evidence is available. */
        viewport_stage_structure3_package_geometry(vp, engine);
        viewport_stage_structure3_animated_materials(vp, engine);
        viewport_stage_structure3_animated_material_images(vp, engine);
        viewport_stage_structure3_untextured_faces(vp, engine);
        viewport_stage_structure3_complete_source_scene(vp, engine);
        viewport_stage_structure1f_source_scene(vp, engine);
        viewport_stage_structure1f2_face_adjacency_transform(vp, engine);
        viewport_stage_m11_direct_lev_dungeon_no_draw(vp, engine);
        viewport_stage_structure1c_source_scene(vp, engine);
        viewport_stage_structure2_payload_anchors(vp, engine);
        if (viewport_render_structure3_mesh(vp, engine))
            return;
        /* Structure1B/MNS supplies an older host material path, but it has
         * no proven mapping to this complete source-owned Structure3 scene.
         * Do not let it rasterize a different interpretation of the same
         * retail level while Saturn material/palette/VDP1 semantics are open. */
        if (vp->last_dgn_render_receipt.structure3_complete_source_scene_consumed) {
            vp->last_dgn_render_receipt.no_draw_structure3_source_scene = 1;
            vp->last_dgn_render_receipt.blocked = 1;
            return;
        }
        /* The real runtime must not convert a decoded DMDF-only bank into
         * visible DGN material. FLOORS/WALLS need independently verified BPK
         * containers and completed host routes; missing Track 1 containers
         * leave this route blocked with no legacy visual fallback. */
        if (engine->initialized &&
            !((engine->dgn_static_material_sources.canonical_pair_bound &&
               engine->dgn_static_material_sources
                   .structure1b_selector_binding_proven &&
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
            vp->last_dgn_render_receipt.no_draw_structure1f_semantics =
                blocked_plan->status ==
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
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
            uint8_t texel_map[256];
            surface = viewport_plan_surface(engine, command);
            viewport_surface_palette_map(vp, surface, texel_map);
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
    out_receipt->no_draw_structure1f_semantics =
        render->no_draw_structure1f_semantics ? 1 : 0;
    out_receipt->no_draw_structure3_source_scene =
        render->no_draw_structure3_source_scene ? 1 : 0;
    out_receipt->active_level_source_consumed =
        render->active_level_source_consumed ? 1 : 0;
    out_receipt->active_level_source = render->active_level_source;
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

    if (render->structure3_mesh_rendered) {
        out_receipt->status = NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH;
        out_receipt->host_route_consumed = 1;
        out_receipt->can_present_runtime_dgn = 1;
        out_receipt->blocks_runtime_dgn = 0;
        return 1;
    }
    /* A complete source scene is an explicit runtime barrier, independent of
     * whether the older Structure1B handoff happens to be renderable. */
    if (render->no_draw_structure3_source_scene) {
        out_receipt->status =
            NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE3_SOURCE_SCENE;
        return 0;
    }
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
    if (render->no_draw_structure1f_semantics) {
        out_receipt->status =
            NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE1F_SEMANTICS;
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
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE1F_SEMANTICS:
        return "blocked-structure1f-semantics";
    case NEXUS_V1_DGN_HOST_ROUTE_BLOCKED_STRUCTURE3_SOURCE_SCENE:
        return "blocked-structure3-source-scene";
    case NEXUS_V1_DGN_HOST_ROUTE_MISSING:
    default:
        return "missing";
    }
}
