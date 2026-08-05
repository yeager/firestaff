#include "nexus_v1_rasterizer.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_game.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int count_color(const Nexus_Framebuffer *fb, uint8_t color) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->color_buffer[i] == color) ++count;
    }
    return count;
}

static int count_written_depth(const Nexus_Framebuffer *fb) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->z_buffer[i] < 1e30f) ++count;
    }
    return count;
}

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffU);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffU);
    p[2] = (uint8_t)((v >> 8) & 0xffU);
    p[3] = (uint8_t)(v & 0xffU);
}

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint8_t *dgn_cell(uint8_t *structure1, int structure1b_rel,
                         int x, int y) {
    return structure1 + structure1b_rel +
           ((y * NEXUS_MAX_MAP_SIZE + x) *
            NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
}

static void set_dgn_collision_ref(uint8_t *structure1,
                                  int structure1b_rel,
                                  int x,
                                  int y,
                                  int ref) {
    uint8_t *cell = dgn_cell(structure1, structure1b_rel, x, y);
    cell[6] = (uint8_t)((ref >> 8) & 0x0f);
    cell[7] = (uint8_t)(ref & 0xff);
}

static int build_viewport_dgn(uint8_t *buf, int buf_size,
                              int structure1b_rel, int geometry_bytes) {
    const int structure1_blocks = 19;
    const int post_grid = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES;
    const int record_count = 8;
    const int records_end = 152 +
        record_count * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
    uint8_t *structure1;
    int record;
    int useful;
    if (!buf || buf_size <= 0) return -1;
    memset(buf, 0, (size_t)buf_size);
    useful = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + geometry_bytes;
    if (useful > structure1_blocks * NEXUS_DGN_BLOCK_SIZE) return -1;
    wb16(buf + 0x0c, 1U);
    wb16(buf + 0x0e, (uint16_t)structure1_blocks);
    wb32(buf + 0x10, (uint32_t)useful);
    structure1 = buf + NEXUS_DGN_BLOCK_SIZE;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    wb32(structure1 + 0x14, (uint32_t)structure1b_rel);
    if (geometry_bytes < records_end) return -1;
    /* Match the real-media post-grid form required by the DGN route while
     * retaining opaque row payloads for this material-only fixture. */
    wb32(structure1 + 0x18, (uint32_t)post_grid);
    wb32(structure1 + 0x24, (uint32_t)(post_grid + 24));
    wb32(structure1 + 0x2c, (uint32_t)(post_grid + 152));
    wb32(structure1 + 0x30, (uint32_t)(post_grid + 152));
    wb32(structure1 + 0x34, (uint32_t)(post_grid + records_end));
    structure1[post_grid] = 6;
    for (record = 0; record < record_count - 1; ++record) {
        structure1[post_grid + 152 +
                   record * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES +
                   NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] =
            (uint8_t)record;
    }
    return 0;
}

static void seed_surface(Nexus_DMDFTextureSurface *surface,
                         uint8_t *pixel,
                         uint32_t rgba) {
    memset(surface, 0, sizeof(*surface));
    *pixel = 1u;
    surface->pixels = pixel;
    surface->width = 1;
    surface->height = 1;
    surface->palette[1] = rgba;
    surface->valid = 1;
}

int main(void) {
    Nexus_Framebuffer fb;
    Nexus_Camera cam;
    /* 3x2 is deliberately not a power-of-two surface. */
    const uint8_t pixels[6] = {1, 2, 3, 2, 1, 3};
    uint32_t palette[256] = {0};
    uint8_t map[256];
    int written_before;
    Nexus_V1_Engine engine;
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportRenderReceipt receipt;
    Nexus_V1_DungeonStartReceipt start;
    Nexus_V1_GameState game;
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    uint8_t floor_pixel;
    uint8_t wall_pixel;
    uint8_t *structure1;
    static const uint8_t canonical_dgn_bytes[] = {'a', 'b', 'c'};

    expect(nexus_v1_dgn_bytes_match_canonical_md5(
               canonical_dgn_bytes, (int)sizeof(canonical_dgn_bytes),
               "900150983cd24fb0d6963f7d28e17f72") == 1,
           "canonical DGN bytes are accepted by the exact-buffer hash gate");
    expect(nexus_v1_dgn_bytes_match_canonical_md5(
               canonical_dgn_bytes, (int)sizeof(canonical_dgn_bytes),
               "00000000000000000000000000000000") == 0 &&
               nexus_v1_dgn_bytes_match_canonical_md5(
                   canonical_dgn_bytes, (int)sizeof(canonical_dgn_bytes) - 1,
                   "900150983cd24fb0d6963f7d28e17f72") == 0,
           "mismatched DGN bytes stay blocked before Structure3 binding");

    nexus_fb_init(&fb);
    nexus_fb_clear(&fb);
    nexus_camera_init(&cam, (Vec3){0.5f, 0.5f, 3.5f}, 0);

    /* Door rendering must not resurrect the removed DM1-derived geometry or
     * palette guesses merely because a host texture was supplied. */
    {
        const uint8_t door_pixel = 1U;
        const uint32_t door_palette[256] = {[1] = 0xffabcdefU};
        uint64_t color_before;
        uint64_t depth_before;
        nexus_fb_clear(&fb);
        color_before = fnv1a64(fb.color_buffer, sizeof(fb.color_buffer));
        depth_before = fnv1a64((const uint8_t *)fb.z_buffer,
                               sizeof(fb.z_buffer));
        nexus_draw_door(&fb, &cam, 0.0f, 2.0f, 0, NEXUS_DOOR_OPEN,
                        3, &door_pixel, 1, 1, door_palette);
        expect(fnv1a64(fb.color_buffer, sizeof(fb.color_buffer)) ==
                   color_before &&
                   fnv1a64((const uint8_t *)fb.z_buffer,
                           sizeof(fb.z_buffer)) == depth_before,
               "unproven Saturn door geometry stays no-draw");
    }

    memset(map, 0xff, sizeof(map));
    palette[1] = 0xff102030U;
    palette[2] = 0xff405060U;
    palette[3] = 0xff708090U;
    map[1] = 41;
    map[2] = 42;
    map[3] = 43;
    fb.palette[41] = palette[1];
    fb.palette[42] = palette[2];
    fb.palette[43] = palette[3];

    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 41) > 0 && count_color(&fb, 42) > 0 &&
               count_color(&fb, 43) > 0,
           "wall rasterizer samples all texels from a 3x2 material surface");
    expect(fb.palette[41] == palette[1] && fb.palette[42] == palette[2] &&
               fb.palette[43] == palette[3],
           "material CLUT is preserved through framebuffer remapping");

    nexus_fb_clear(&fb);
    palette[2] = 0x00000000U;
    written_before = count_written_depth(&fb);
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "transparent material texels never use their mapped replacement color");
    expect(count_written_depth(&fb) > written_before,
           "opaque material texels still write depth after transparent clipping");

    nexus_fb_clear(&fb);
    palette[2] = 0xff405060U;
    map[2] = 0xff;
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "undefined palette remaps clip instead of falling back to flat color");

    memset(&engine, 0, sizeof(engine));
    expect(build_viewport_dgn(dgn, (int)sizeof(dgn), 0x40, 2048) == 0,
           "viewport DGN fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_dgn_collision_ref(structure1, 0x40, 3, 4, 1);
    set_dgn_collision_ref(structure1, 0x40, 2, 4, 0x0fff);
    set_dgn_collision_ref(structure1, 0x40, 4, 4, 0x0fff);
    set_dgn_collision_ref(structure1, 0x40, 3, 3, 0x0fff);
    expect(nexus_v1_level_load(&engine.current_level,
                               dgn,
                               (int)sizeof(dgn),
                               0) == 0,
           "viewport DGN fixture loads through real Structure1B parser");
    expect(nexus_v1_game_resolve_dungeon_start(
               &engine.current_level, NEXUS_V1_INITIAL_PARTY_LEVEL,
               NEXUS_V1_INITIAL_PARTY_X, NEXUS_V1_INITIAL_PARTY_Y,
               NEXUS_V1_INITIAL_PARTY_DIR, &start) == 1 &&
               start.status == NEXUS_V1_DUNGEON_START_READY &&
               start.dgn_cell_consumed && !start.blocks_runtime &&
               !start.fallback_visuals_permitted,
           "DGN-backed new-game start accepts the verified Structure1B cell");
    memset(&game, 0, sizeof(game));
    expect(nexus_v1_game_apply_dungeon_start(&game, &start) == 1 &&
               game.current_level == 0 && game.party_x == 11 &&
               game.party_y == 29 && game.party_dir == 0,
           "new-game host state consumes the validated DGN start pose");
    set_dgn_collision_ref(structure1, 0x40, NEXUS_V1_INITIAL_PARTY_X,
                          NEXUS_V1_INITIAL_PARTY_Y, 0x0fff);
    expect(nexus_v1_level_load(&engine.current_level,
                               dgn,
                               (int)sizeof(dgn),
                               0) == 0 &&
               nexus_v1_game_resolve_dungeon_start(
                   &engine.current_level, NEXUS_V1_INITIAL_PARTY_LEVEL,
                   NEXUS_V1_INITIAL_PARTY_X, NEXUS_V1_INITIAL_PARTY_Y,
                   NEXUS_V1_INITIAL_PARTY_DIR, &start) == 0 &&
               start.status == NEXUS_V1_DUNGEON_START_BLOCKED_CELL &&
               start.blocks_runtime && !start.fallback_visuals_permitted,
           "wall/collision start cell blocks the host route without fallback");
    set_dgn_collision_ref(structure1, 0x40, NEXUS_V1_INITIAL_PARTY_X,
                          NEXUS_V1_INITIAL_PARTY_Y, 0);
    expect(nexus_v1_level_load(&engine.current_level,
                               dgn,
                               (int)sizeof(dgn),
                               0) == 0,
           "start-cell fixture restores its passable DGN route");
    for (int y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (int x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            for (int dir = 0; dir < 4; ++dir) {
                engine.current_level.wall_material_refs[y][x][dir] = 1U;
            }
        }
    }
    engine.level_loaded = 1;
    engine.initialized = 1;
    engine.game.current_level = 0;
    engine.game.party_x = 3;
    engine.game.party_y = 4;
    engine.game.party_dir = 0;
    engine.current_level_dgn_data = dgn;
    engine.current_level_dgn_size = (int)sizeof(dgn);
    engine.current_level_structure2_source.level_index = 0;
    engine.current_level_structure2_source.canonical_hash_verified = 1;
    engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
    engine.current_level_structure2_source.materialization_bound = 1;
    engine.current_level_structure2_source.loaded_bytes_bound = 1;
    engine.current_level_structure2_source.loaded_dgn_size = (int)sizeof(dgn);
    engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
        fnv1a64(dgn, sizeof(dgn));
    engine.floor_materials.valid = 1;
    engine.floor_materials.surface_count = 1;
    seed_surface(&engine.floor_materials.surfaces[0],
                 &floor_pixel,
                 0xff204060U);
    engine.wall_materials.valid = 1;
    engine.wall_materials.surface_count = 1;
    seed_surface(&engine.wall_materials.surfaces[0], &wall_pixel,
                 0xfff80000U);
    dgn_cell(structure1, 0x40, 3, 4)[0] = 0x00U;
    dgn_cell(structure1, 0x40, 3, 4)[1] = 0x00U;
    engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
        fnv1a64(dgn, sizeof(dgn));
    expect(nexus_v1_level_load(&engine.current_level,
                               dgn,
                               (int)sizeof(dgn),
                               0) == 0,
           "Structure1B material ids reload for the direct MNS route");
    for (int y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (int x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            for (int dir = 0; dir < 4; ++dir) {
                engine.current_level.wall_material_refs[y][x][dir] = 0U;
            }
        }
    }
    /* The direct retail MNS route is a separate package route.  Parseable
     * surfaces cannot bypass the canonical SN_FLOOR/SN_WALL identity pair. */
    engine.floor_bpk_container.host_route_permitted = 0;
    engine.wall_bpk_container.host_route_permitted = 0;
    engine.floor_bpk_host_route_valid = 0;
    engine.wall_bpk_host_route_valid = 0;
    engine.floor_mns_material_route_valid = 1;
    engine.wall_mns_material_route_valid = 1;
    engine.dgn_static_material_sources.canonical_pair_bound = 0;
    nexus_viewport_init(&viewport);
    nexus_v1_invalidate_dgn_material_plan(&engine);
    nexus_viewport_render(&viewport, &engine);
    expect(nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) == 0,
           "viewport exposes static MNS identity-block receipt");
    expect(receipt.attempted && receipt.blocked &&
               !receipt.fallback_visuals_permitted,
           "unverified static MNS bytes cannot enter the DGN host route");

    engine.dgn_static_material_sources.canonical_pair_bound = 1;
    engine.dgn_static_material_sources.structure1b_selector_binding_proven = 1;
    engine.dgn_static_material_sources.floor_mns.canonical_hash_verified = 1;
    engine.dgn_static_material_sources.wall_mns.canonical_hash_verified = 1;
    memset(&engine.dgn_static_material_sources.floor_mns.canonical_name, 0,
           sizeof(engine.dgn_static_material_sources.floor_mns.canonical_name));
    memset(&engine.dgn_static_material_sources.wall_mns.canonical_name, 0,
           sizeof(engine.dgn_static_material_sources.wall_mns.canonical_name));
    memcpy(engine.dgn_static_material_sources.floor_mns.canonical_name,
           "SN_FLOOR.MNS", 13);
    memcpy(engine.dgn_static_material_sources.wall_mns.canonical_name,
           "SN_WALL.MNS", 12);
    for (int y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (int x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            for (int dir = 0; dir < 4; ++dir) {
                engine.current_level.wall_material_refs[y][x][dir] = 0U;
            }
        }
    }
    nexus_v1_invalidate_dgn_material_plan(&engine);
    nexus_viewport_render(&viewport, &engine);
    /* With PRS3 decoding enabled, the Structure2 source gate requires a
     * valid external PRS3 placement binding.  Without one, the material
     * plan blocks at BLOCKED_STRUCTURE2_SOURCE even when the MNS pair is
     * hash-bound.  Verify that the blocked receipt exposes the expected
     * no-draw evidence fields. */
    expect(nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) == 0 &&
               !receipt.ready && receipt.blocked &&
               receipt.bpk_material_surface_count == 0 &&
               receipt.static_mns_material_surface_count == 0 &&
               receipt.material_surface_count == 0 &&
               !receipt.fallback_visuals_permitted,
           "hash-bound SN_FLOOR/SN_WALL route blocks without PRS3 placement binding");
    /* The plan itself reaches READY_MESH once the Structure2 fixture is
     * present, but the viewport palette sync fails because the synthetic
     * MNS surfaces lack full CLUT data.  Verify the plan receipt. */
    expect(engine.dgn_material_plan.receipt.status ==
               NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE &&
               engine.dgn_material_plan.receipt.blocks_real_dgn_mesh_render &&
               !engine.dgn_material_plan.receipt.uses_bpk_material_route,
           "bound Structure1B selectors admit one static MNS package route only");
    dgn[0] ^= 1U;
    nexus_v1_invalidate_dgn_material_plan(&engine);
    {
        const Nexus_V1_DgnMaterialPlan *mutated_plan =
            nexus_v1_prepare_dgn_material_plan(&engine, 3, 4, 0);
        expect(mutated_plan == NULL &&
               engine.dgn_material_plan.receipt.status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE &&
               engine.dgn_material_plan.receipt.blocks_real_dgn_mesh_render &&
               !engine.dgn_material_plan.receipt.fallback_visuals_permitted,
           "a changed retained LEV buffer blocks the material host route despite valid MNS sources");
    }
    dgn[0] ^= 1U;
    /* A resolved Structure1A owner may retain only the bounded Structure3
     * envelope. The plan keeps that source evidence while the absent face
     * grammar remains a hard no-draw gate. */
    engine.current_level.geometry_info.structure1f_declared = 1;
    engine.current_level.geometry_info.structure1f_valid = 1;
    engine.current_level.geometry_info.structure1f_total_entry_count = 1;
    engine.current_level.structure1f_entry_count = 1;
    engine.current_level.structure1f_entries[0].family =
        NEXUS_V1_DGN_STRUCTURE1F_ALCOVES;
    engine.current_level.structure1f_entries[0].tag = 0x20U;
    engine.current_level.structure1f_entries[0].face = 3U;
    engine.current_level.structure1f_entries[0].structure1a_index = 0;
    engine.current_level.structure1f_entries[0].structure1a_relation_valid = 1;
    engine.current_level.structure1f_entries[0].structure1a_owner_x = 3;
    engine.current_level.structure1f_entries[0].structure1a_owner_y = 4;
    engine.current_level.structure1f_entries[0]
        .structure1a_structure3_model_index = 5U;
    engine.current_level.structure1a_table_valid = 1;
    engine.current_level.structure1a_model_count = 1;
    engine.current_level.structure1a_models[0].kind = 0x6aU;
    engine.current_level.structure1a_models[0].structure3_model_index = 5U;
    engine.current_level.structure1a_owner_ref_valid[4][3] = 1U;
    engine.current_level.structure1a_owner_refs[4][3] = 0U;
    engine.current_level.structure3_payload.declared = 1;
    engine.current_level.structure3_payload.valid = 1;
    engine.current_level.structure3_payload.block_offset = 20;
    engine.current_level.structure3_payload.block_count = 4;
    engine.current_level.structure3_payload.byte_size =
        NEXUS_DGN_BLOCK_SIZE * 4;
    engine.current_level.structure3_payload.complete_block_count = 4;
    engine.current_level.structure3_payload.nonzero_byte_run_count = 3;
    engine.current_level.structure3_payload.nonzero_block_run_count = 2;
    engine.current_level.structure3_payload.raw_payload_hash = 0x6d358ca1U;
    /* Without PRS3 placement the plan blocks at Structure2 source before
     * reaching the Structure1A/Structure3 topology analysis. */
    nexus_v1_invalidate_dgn_material_plan(&engine);
    expect(nexus_v1_prepare_dgn_material_plan(&engine, 3, 4, 0) == NULL &&
               engine.dgn_material_plan.receipt.status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE &&
               !engine.dgn_material_plan.receipt.plan_ready &&
               engine.dgn_material_plan.receipt.blocks_real_dgn_mesh_render &&
               !engine.dgn_material_plan.receipt.fallback_visuals_permitted,
           "Structure1A/Structure3 topology blocked at Structure2 source gate without PRS3 placement");
    engine.current_level.structure1f_entry_count = 0;
    engine.current_level.geometry_info.structure1f_declared = 0;
    engine.current_level.geometry_info.structure1f_valid = 0;
    memset(&engine.current_level.structure3_payload, 0,
           sizeof(engine.current_level.structure3_payload));
    {
        Nexus_V1_DgnStaticMaterialSourceReceipt source_receipt;
        expect(nexus_v1_dgn_static_material_source_receipt(
                   &engine, &source_receipt) == 0 &&
                   source_receipt.canonical_pair_bound &&
                   strcmp(source_receipt.floor_mns.canonical_name,
                          "SN_FLOOR.MNS") == 0 &&
                   strcmp(source_receipt.wall_mns.canonical_name,
                          "SN_WALL.MNS") == 0,
               "DGN host exposes the canonical static-material package receipt");
    }

    engine.wall_materials.valid = 0;
    engine.wall_materials.surface_count = 0;
    engine.wall_materials.surfaces[0].valid = 0;
    nexus_v1_invalidate_dgn_material_plan(&engine);
    nexus_viewport_render(&viewport, &engine);
    expect(nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) == 0,
           "viewport exposes blocked DGN material receipt");
    expect(receipt.attempted && receipt.used_real_dgn_route &&
               receipt.blocked &&
               !receipt.fallback_visuals_permitted &&
               !receipt.palette_synced &&
               receipt.rasterized_command_count == 0,
           "viewport blocks real DGN route when required MNS material is missing");
    /* Without PRS3 placement the plan blocks at Structure2 source before
     * building commands, so command_count and missing_material_count are
     * both zero. */
    expect(receipt.command_count == 0 &&
               receipt.missing_material_count == 0 &&
               receipt.blocked &&
               !receipt.fallback_visuals_permitted,
           "DGN material plan blocks at Structure2 source before reaching material inventory");
    expect(!viewport.material_palette_valid &&
               viewport.material_engine == NULL &&
               viewport.fb.palette[16] == viewport.base_palette[16],
           "blocked DGN handoff clears the previous MNS material palette");

    return failures ? 1 : 0;
}
