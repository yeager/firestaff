/* Canonical PC G1 DB4 creature -> GDAT CREATURES/F9 handoff proof.
 * Source: skproject/SKWIN/c_map.cpp QUERY_DUNGEON_MAP_CHIP_PICT and
 * SKWIN/SkWinCore.cpp DRAW_CHIP_OF_MAGIC_MAP. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_g1_scene_runtime_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const DM2_V1_AssetLoader *loader;
    int creature_type;
    int expected_index;
    uint8_t *decoded_pixels;
    int decoded_pixels_width;
    int decoded_pixels_height;
    int fetch_calls;
    int palette_calls;
    int resolve_calls;
    int reject_fetch;
} GdatTrace;

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static uint32_t hash_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int load_canonical_files(uint8_t **graphics, size_t *graphics_size,
                                uint8_t **dungeon, size_t *dungeon_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char graphics_path[1100];
    char dungeon_path[1100];

    if (!root || !root[0]) return 0;
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    return read_file(graphics_path, graphics, graphics_size) &&
        read_file(dungeon_path, dungeon, dungeon_size);
}

static int unexpected_generic_resolve(void *user,
                                      DM2_V1_G1SceneTileClass tile_class,
                                      DM2_V1_G1SceneRootClass root_class,
                                      int *out_gdat_index)
{
    GdatTrace *trace = user;
    (void)tile_class;
    (void)root_class;
    (void)out_gdat_index;
    ++trace->resolve_calls;
    return 0;
}

static int fetch_creature_map_chip(void *user, int gdat_index,
                                   const uint8_t **out_pixels,
                                   int *out_width, int *out_height,
                                   int *out_stride)
{
    GdatTrace *trace = user;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

    ++trace->fetch_calls;
    if (trace->reject_fetch || gdat_index != trace->expected_index) return -1;
    if (!trace->decoded_pixels) {
        trace->decoded_pixels = dm2_v1_asset_load_image_field(
            trace->loader, DM2_GDAT_CATEGORY_CREATURES, trace->creature_type,
            DM2_GDAT_IMG_MAP_CHIP, &width, &height, &format);
        if (!trace->decoded_pixels || width <= 0 || height <= 0 ||
            format == DM2_IMG_FMT_UNKNOWN) {
            dm2_v1_asset_free_pixels(trace->decoded_pixels);
            trace->decoded_pixels = NULL;
            return -1;
        }
        trace->decoded_pixels_width = width;
        trace->decoded_pixels_height = height;
    } else {
        width = trace->decoded_pixels_width;
        height = trace->decoded_pixels_height;
    }
    *out_pixels = trace->decoded_pixels;
    *out_width = width;
    *out_height = height;
    *out_stride = width;
    return 0;
}

static int fetch_creature_local_palette(void *user, int gdat_index,
                                        uint8_t out_palette16[16],
                                        uint32_t *out_hash)
{
    GdatTrace *trace = user;

    ++trace->palette_calls;
    if (trace->reject_fetch || gdat_index != trace->expected_index) return -1;
    return dm2_v1_asset_load_image_local_palette(
        trace->loader, DM2_GDAT_CATEGORY_CREATURES, trace->creature_type,
        DM2_GDAT_IMG_MAP_CHIP, out_palette16, out_hash) && *out_hash != 0u
        ? 0 : -1;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1DungeonSceneClassificationReceipt scene;
    DM2_V1_G1SceneRuntimeHandoffReceipt handoff;
    DM2_V1_G1CreatureMapChipRuntimeReceipt material_receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    GdatTrace trace;
    const DM2_V1_G1DirectChainNode *root;
    const uint8_t *raw_map_chip;
    size_t raw_map_chip_size = 0u;
    int creature_level = -1;
    int creature_x = -1;
    int creature_y = -1;
    int root_object_id = -1;
    uint32_t material_identity = 0u;
    uint32_t altered_identity = 0u;
    int failures = 0;

    if (!getenv("FIRESTAFF_DM2_DATA_DIR") ||
        !getenv("FIRESTAFF_DM2_DATA_DIR")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not set");
        return 0;
    }
    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        fputs("FAIL: selected canonical DM2 data is unreadable\n", stderr);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&scene, 0, sizeof(scene));
    memset(&handoff, 0, sizeof(handoff));
    memset(&trace, 0, sizeof(trace));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        fputs("FAIL: canonical G1/GDAT input was not accepted\n", stderr);
        failures = 1;
        goto done;
    }
    /* The corpus, not a synthetic fixture, chooses the first direct DB4 root
     * whose exact CREATURES/type/F9 record is present. */
    for (int level = 0; level < dungeon.level_count && creature_level < 0;
         ++level) {
        for (int x = 0; x < dungeon.level_widths[level] && creature_level < 0;
             ++x) {
            for (int y = 0; y < dungeon.level_heights[level]; ++y) {
                DM2_V1_G1DungeonSceneClassificationReceipt candidate;
                const DM2_V1_G1DirectChainNode *candidate_root;
                int candidate_type;

                memset(&candidate, 0, sizeof(candidate));
                if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
                        &dungeon, level, x, y, &candidate) ||
                    candidate.root_class != DM2_V1_G1_SCENE_ROOT_CREATURE ||
                    candidate.chain.node_count < 1) {
                    continue;
                }
                candidate_root = &candidate.chain.nodes[0];
                if (candidate_root->type != 4 || candidate_root->record_size < 5 ||
                    candidate_root->record_offset < 0 ||
                    candidate_root->record_offset + candidate_root->record_size >
                        dungeon.raw_size) {
                    continue;
                }
                candidate_type = dungeon.raw_data[candidate_root->record_offset + 4];
                if (!dm2_v1_asset_load_sized(
                        &loader, DM2_GDAT_CATEGORY_CREATURES, candidate_type,
                        DM2_GDAT_IMG_MAP_CHIP, NULL)) {
                    continue;
                }
                scene = candidate;
                creature_level = level;
                creature_x = x;
                creature_y = y;
                break;
            }
        }
    }
    if (creature_level < 0) {
        fputs("FAIL: canonical G1 has no direct DB4 root with CREATURES/F9 art\n",
              stderr);
        failures = 1;
        goto done;
    }
    root = &scene.chain.nodes[0];
    if (root->type != 4 || root->record_size < 16 || root->record_offset < 0 ||
        root->record_offset + root->record_size > dungeon.raw_size) {
        fputs("FAIL: canonical DB4 creature root lacks b4/b15 fields\n", stderr);
        failures = 1;
        goto done;
    }
    root_object_id = dm2_v1_dungeon_get_first_thing(
        &dungeon, creature_level, creature_x, creature_y);
    if (root_object_id < 0) {
        fputs("FAIL: canonical DB4 scene root lost its source ObjectID\n", stderr);
        failures = 1;
        goto done;
    }
    trace.loader = &loader;
    trace.creature_type = dungeon.raw_data[root->record_offset + 4];
    trace.expected_index = dm2_v1_viewport_creature_graphic_index(
        trace.creature_type, 0);
    raw_map_chip = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_CREATURES, trace.creature_type,
        DM2_GDAT_IMG_MAP_CHIP, &raw_map_chip_size);
    if (!raw_map_chip || raw_map_chip_size == 0u || trace.expected_index == 0) {
        fputs("FAIL: canonical CREATURES/type/F9 source image is absent\n", stderr);
        failures = 1;
        goto done;
    }
    if (!dm2_v1_g1_scene_runtime_handoff(
            &dungeon, creature_level, creature_x, creature_y,
            unexpected_generic_resolve, &trace,
            fetch_creature_map_chip, &trace, fetch_creature_local_palette,
            &trace, &handoff) || !handoff.valid || handoff.blocked ||
        handoff.scene.root_class != DM2_V1_G1_SCENE_ROOT_CREATURE ||
        handoff.creature_type != trace.creature_type ||
        handoff.gdat_index != trace.expected_index ||
        handoff.material_width <= 0 || handoff.material_height <= 0 ||
        handoff.material_stride < handoff.material_width ||
        handoff.material_palette_hash == 0u ||
        handoff.material_pixel_hash == 0u || trace.resolve_calls != 0 ||
        trace.fetch_calls != 1 || trace.palette_calls != 1) {
        fputs("FAIL: DB4 b4 did not hand off its exact decoded CREATURES/F9 material\n",
              stderr);
        failures = 1;
        goto done;
    }
    /* The same decoded source surface must be accepted by the viewport draw,
     * not merely by the bridge.  The owner receipt is built exclusively from
     * this canonical G1 root and the handoff's real F9 metadata. */
    memset(&material_receipt, 0, sizeof(material_receipt));
    material_receipt.valid = 1;
    material_receipt.map = creature_level;
    material_receipt.source_creature_root_count = 1;
    material_receipt.material_count = 1;
    material_receipt.materials[0].x = creature_x;
    material_receipt.materials[0].y = creature_y;
    material_receipt.materials[0].object_id = (uint16_t)root_object_id;
    material_receipt.materials[0].direction =
        (uint8_t)(dungeon.raw_data[root->record_offset + 15] & 3u);
    material_receipt.materials[0].creature_type = (uint8_t)trace.creature_type;
    material_receipt.materials[0].info_slot =
        dungeon.raw_data[root->record_offset + 5];
    material_receipt.materials[0].animation_sequence =
        (uint16_t)(dungeon.raw_data[root->record_offset + 8] |
                   ((uint16_t)dungeon.raw_data[root->record_offset + 9] << 8));
    material_receipt.materials[0].animation_info =
        (uint16_t)(dungeon.raw_data[root->record_offset + 10] |
                   ((uint16_t)dungeon.raw_data[root->record_offset + 11] << 8));
    material_receipt.materials[0].raw_hash =
        hash_bytes(raw_map_chip, raw_map_chip_size);
    material_receipt.materials[0].raw_byte_count =
        (uint32_t)raw_map_chip_size;
    material_receipt.materials[0].image_width = handoff.material_width;
    material_receipt.materials[0].image_height = handoff.material_height;
    material_receipt.materials[0].image_format = DM2_IMG_FMT_UNKNOWN;
    {
        int source_width = 0;
        int source_height = 0;
        DM2_ImageFormat source_format = DM2_IMG_FMT_UNKNOWN;
        uint8_t *source_pixels = dm2_v1_asset_load_image_field(
            &loader, DM2_GDAT_CATEGORY_CREATURES, trace.creature_type,
            DM2_GDAT_IMG_MAP_CHIP, &source_width, &source_height,
            &source_format);
        dm2_v1_asset_free_pixels(source_pixels);
        material_receipt.materials[0].image_format = source_format;
    }
    material_receipt.materials[0].local_palette_hash =
        handoff.material_palette_hash;
    if (!dm2_v1_g1_creature_map_chip_material_identity(
            &material_receipt.materials[0], &material_identity) ||
        material_identity == 0u) {
        fputs("FAIL: canonical DB4 material has no source identity\n", stderr);
        failures = 1;
        goto done;
    }
    material_receipt.materials[0].local_palette_hash ^= 1u;
    if (!dm2_v1_g1_creature_map_chip_material_identity(
            &material_receipt.materials[0], &altered_identity) ||
        altered_identity == material_identity) {
        fputs("FAIL: creature identity accepted an altered local palette\n", stderr);
        failures = 1;
        goto done;
    }
    material_receipt.materials[0].local_palette_hash =
        handoff.material_palette_hash;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_creature_map_chip, &trace);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, fetch_creature_local_palette, &trace);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_g1_creature_map_chip_materials(
        &viewport, &material_receipt);
    dm2_v1_viewport_set_g1_scene_creature_material_direct(
        &viewport, 1, creature_x, creature_y, trace.creature_type,
        handoff.gdat_index, handoff.material_pixels,
        handoff.material_width, handoff.material_height,
        handoff.material_stride, handoff.material_palette16,
        handoff.material_palette_hash, handoff.material_pixel_hash);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = (uint8_t)trace.creature_type;
    viewport.creatures[0].source_kind = 2;
    viewport.creatures[0].source_info_slot =
        material_receipt.materials[0].info_slot;
    viewport.creatures[0].source_animation_sequence =
        material_receipt.materials[0].animation_sequence;
    viewport.creatures[0].source_animation_info =
        material_receipt.materials[0].animation_info;
    viewport.creatures[0].object_id = (uint16_t)root_object_id;
    viewport.creatures[0].map_x = (int16_t)creature_x;
    viewport.creatures[0].map_y = (int16_t)creature_y;
    viewport.creatures[0].direction = material_receipt.materials[0].direction;
    viewport.creatures[0].screen_x = DM2_VP_WIDTH / 2;
    viewport.creatures[0].screen_y = DM2_VP_HEIGHT / 2;
    viewport.creatures[0].health_pct = 100;
    dm2_v1_render_creatures(&viewport);
    if (viewport.asset_creature_drawn_count != 1 ||
        viewport.fallback_creature_drawn_count != 0 ||
        viewport.blocked_material_draw_count != 0 ||
        viewport.g1_scene_creature_material_consumed_count != 1 ||
        !viewport.last_creature_asset_blit_valid || trace.fetch_calls != 1 ||
        trace.palette_calls != 1) {
        fputs("FAIL: canonical DB4 owner did not consume its F9 material draw\n",
              stderr);
        failures = 1;
        goto done;
    }
    if (viewport.last_creature_render.source_info_slot !=
            material_receipt.materials[0].info_slot ||
        viewport.last_creature_render.source_animation_sequence !=
            material_receipt.materials[0].animation_sequence ||
        viewport.last_creature_render.source_animation_info !=
            material_receipt.materials[0].animation_info) {
        fputs("FAIL: DB4 animation cursor was lost before viewport render\n",
              stderr);
        failures = 1;
        goto done;
    }
    /* The direct DB4 receipt owns decoded indexed bytes as well as the
     * palette. A provider buffer mutation cannot replay under stale source
     * provenance. */
    {
        int accepted_asset_drawn = viewport.asset_creature_drawn_count;
        int accepted_scene_consumed =
            viewport.g1_scene_creature_material_consumed_count;
        int accepted_blocked = viewport.blocked_material_draw_count;
        uint8_t original_pixel = trace.decoded_pixels[0];

        trace.decoded_pixels[0] ^= 1u;
        dm2_v1_render_creatures(&viewport);
        if (viewport.asset_creature_drawn_count != accepted_asset_drawn ||
            viewport.g1_scene_creature_material_consumed_count !=
                accepted_scene_consumed ||
            viewport.blocked_material_draw_count != accepted_blocked + 1) {
            fputs("FAIL: changed DB4 decoded pixels replayed a stale handoff\n",
                  stderr);
            failures = 1;
            goto done;
        }
        trace.decoded_pixels[0] = original_pixel;
    }
    /* The direct DB4 receipt owns b15_0_1 as well as its object, tile and
     * CREATURES/type/F9 material. A changed live direction cannot replay the
     * same decoded surface. */
    {
        int accepted_asset_drawn = viewport.asset_creature_drawn_count;
        int accepted_scene_consumed =
            viewport.g1_scene_creature_material_consumed_count;
        int accepted_blocked = viewport.blocked_material_draw_count;

        viewport.creatures[0].direction =
            (uint8_t)((material_receipt.materials[0].direction + 1u) & 3u);
        if (dm2_v1_g1_creature_map_chip_matches_decoded_instance(
                &material_receipt, viewport.creatures[0].object_id,
                viewport.creatures[0].map_x, viewport.creatures[0].map_y,
                viewport.creatures[0].direction,
                viewport.creatures[0].creature_type, handoff.material_width,
                handoff.material_height, handoff.material_palette_hash)) {
            fputs("FAIL: DB4 direction mismatch passed the decoded-instance gate\n",
                  stderr);
            failures = 1;
            goto done;
        }
        dm2_v1_render_creatures(&viewport);
        if (viewport.asset_creature_drawn_count != accepted_asset_drawn) {
            fputs("FAIL: DB4 direction mismatch replayed a CREATURES/F9 material\n",
                  stderr);
            failures = 1;
            goto done;
        }
        if (viewport.g1_scene_creature_material_consumed_count !=
                accepted_scene_consumed) {
            fputs("FAIL: DB4 direction mismatch consumed its scene handoff\n",
                  stderr);
            failures = 1;
            goto done;
        }
        if (viewport.blocked_material_draw_count != accepted_blocked + 1) {
            fputs("FAIL: DB4 direction mismatch did not block the viewport blit\n",
                  stderr);
            failures = 1;
            goto done;
        }
    }
    dm2_v1_asset_free_pixels(trace.decoded_pixels);
    trace.decoded_pixels = NULL;
    trace.reject_fetch = 1;
    memset(&handoff, 0, sizeof(handoff));
    if (dm2_v1_g1_scene_runtime_handoff(
            &dungeon, creature_level, creature_x, creature_y,
            unexpected_generic_resolve, &trace,
            fetch_creature_map_chip, &trace, fetch_creature_local_palette,
            &trace, &handoff) != 0 || !handoff.blocked || handoff.valid) {
        fputs("FAIL: unavailable CREATURES/F9 material did not fail closed\n",
              stderr);
        failures = 1;
    }

done:
    dm2_v1_asset_free_pixels(trace.decoded_pixels);
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures) return 1;
    printf("PASS: canonical DB4 creature GDAT identity=%08x consumes its exact F9 material\n",
           material_identity);
    return 0;
}
