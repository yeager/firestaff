/* Canonical G1 DB4 creature -> GDAT F9 scene-material corpus receipt.
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
    uint8_t *pixels;
} MaterialTrace;

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

static int reject_generic_resolve(void *user,
                                  DM2_V1_G1SceneTileClass tile_class,
                                  DM2_V1_G1SceneRootClass root_class,
                                  int *out_gdat_index)
{
    (void)user;
    (void)tile_class;
    (void)root_class;
    (void)out_gdat_index;
    return 0;
}

static int fetch_map_chip(void *user, int gdat_index,
                          const uint8_t **out_pixels,
                          int *out_width, int *out_height, int *out_stride)
{
    MaterialTrace *trace = user;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    int width = 0;
    int height = 0;

    if (!trace || gdat_index != trace->expected_index || trace->pixels) return -1;
    trace->pixels = dm2_v1_asset_load_image_field(
        trace->loader, DM2_GDAT_CATEGORY_CREATURES, trace->creature_type,
        DM2_GDAT_IMG_MAP_CHIP, &width, &height, &format);
    if (!trace->pixels || width <= 0 || height <= 0 ||
        format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(trace->pixels);
        trace->pixels = NULL;
        return -1;
    }
    *out_pixels = trace->pixels;
    *out_width = width;
    *out_height = height;
    *out_stride = width;
    return 0;
}

static int fetch_local_palette(void *user, int gdat_index,
                               uint8_t out_palette16[16], uint32_t *out_hash)
{
    MaterialTrace *trace = user;

    return trace && gdat_index == trace->expected_index &&
        dm2_v1_asset_load_image_local_palette(
            trace->loader, DM2_GDAT_CATEGORY_CREATURES, trace->creature_type,
            DM2_GDAT_IMG_MAP_CHIP, out_palette16, out_hash) &&
        *out_hash != 0u ? 0 : -1;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    int direct_roots = 0;
    int material_roots = 0;
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
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        fputs("FAIL: canonical G1/GDAT input was not accepted\n", stderr);
        failures = 1;
        goto done;
    }
    for (int level = 0; level < dungeon.level_count; ++level) {
        for (int x = 0; x < dungeon.level_widths[level]; ++x) {
            for (int y = 0; y < dungeon.level_heights[level]; ++y) {
                DM2_V1_G1DungeonSceneClassificationReceipt scene;
                DM2_V1_G1SceneRuntimeHandoffReceipt handoff;
                const DM2_V1_G1DirectChainNode *root;
                MaterialTrace trace;
                int creature_type;

                memset(&scene, 0, sizeof(scene));
                if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
                        &dungeon, level, x, y, &scene) ||
                    scene.root_class != DM2_V1_G1_SCENE_ROOT_CREATURE ||
                    scene.chain.node_count < 1) continue;
                root = &scene.chain.nodes[0];
                if (root->type != 4 || root->record_size < 5 ||
                    root->record_offset < 0 || root->record_offset + root->record_size >
                        dungeon.raw_size) {
                    ++failures;
                    continue;
                }
                ++direct_roots;
                creature_type = dungeon.raw_data[root->record_offset + 4];
                if (!dm2_v1_asset_load_sized(&loader, DM2_GDAT_CATEGORY_CREATURES,
                                              creature_type,
                                              DM2_GDAT_IMG_MAP_CHIP, NULL)) {
                    continue;
                }
                memset(&handoff, 0, sizeof(handoff));
                memset(&trace, 0, sizeof(trace));
                trace.loader = &loader;
                trace.creature_type = creature_type;
                trace.expected_index = dm2_v1_viewport_creature_graphic_index(
                    creature_type, 0);
                if (trace.expected_index == 0 ||
                    !dm2_v1_g1_scene_runtime_handoff(
                        &dungeon, level, x, y, reject_generic_resolve, NULL,
                        fetch_map_chip, &trace, fetch_local_palette, &trace,
                        &handoff) || !handoff.valid || handoff.blocked ||
                    handoff.creature_type != creature_type ||
                    handoff.gdat_index != trace.expected_index ||
                    handoff.material_width <= 0 || handoff.material_height <= 0 ||
                    handoff.material_stride < handoff.material_width ||
                    handoff.material_palette_hash == 0u) {
                    ++failures;
                } else {
                    ++material_roots;
                }
                dm2_v1_asset_free_pixels(trace.pixels);
            }
        }
    }
    if (direct_roots == 0 || material_roots == 0) {
        fputs("FAIL: canonical G1 has no direct DB4 creature material route\n",
              stderr);
        failures = 1;
    }

done:
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures) return 1;
    printf("PASS: canonical G1 DB4 roots=%d exact F9 material handoffs=%d\n",
           direct_roots, material_roots);
    return 0;
}
