/* Canonical PC GDAT plane plan -> M11 viewport handoff. The plan is the
 * decoded UPDATE_GFXSET output; no second GDAT callback may stand in for it.
 * Source: skproject/SKULLWIN/c_gui_vp.cpp UPDATE_GFXSET -> DRAW_DUNGEON. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const char *home = getenv("HOME");
    char default_root[1024];
    char graphics_path[1100];
    char dungeon_path[1100];

    if (!root || !root[0]) {
        if (!home || !home[0]) return 0;
        snprintf(default_root, sizeof(default_root),
                 "%s/.firestaff/data/dm2/data", home);
        root = default_root;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    return read_file(graphics_path, graphics, graphics_size) &&
        read_file(dungeon_path, dungeon, dungeon_size);
}

static int unexpected_fetches;

static int unexpected_asset_fetch(void *user, int index,
                                  const uint8_t **pixels, int *width,
                                  int *height, int *stride)
{
    (void)user;
    (void)index;
    (void)pixels;
    (void)width;
    (void)height;
    (void)stride;
    ++unexpected_fetches;
    return -1;
}

static int unexpected_palette_fetch(void *user, int index, uint8_t palette[16],
                                    uint32_t *hash)
{
    (void)user;
    (void)index;
    (void)palette;
    (void)hash;
    ++unexpected_fetches;
    return -1;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_GdatSceneM11CommandPlan plan;
    DM2_V1_GdatSceneM11CommandPlan mismatched;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int style = -1;
    int failures = 0;

    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&plan, 0, sizeof(plan));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0) {
        fputs("FAIL: canonical DM2 GDAT/G1 input was not accepted\n", stderr);
        failures = 1;
        goto done;
    }
    for (int level = 0; level < dungeon.level_count; ++level) {
        int candidate = dm2_v1_dungeon_get_map_graphics_style(&dungeon, level);
        if (candidate >= 0 && candidate <= 0xff &&
            dm2_v1_gdat_scene_m11_command_plan_build(
                &loader, (uint8_t)candidate, &plan)) {
            style = candidate;
            break;
        }
    }
    if (style < 0 || !plan.valid || plan.command_hash == 0u) {
        fputs("FAIL: no G1 MapGraphicsStyle yielded a decoded plane plan\n", stderr);
        failures = 1;
        goto done;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, style, plan.command_hash, plan.scene_colorkey,
        plan.scene_flags, 0u, plan.highest_light_level, 0u, 0u, 0u, 0u,
        0u, plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &plan);
    dm2_v1_render_floor_ceiling(&viewport);
    if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 2 ||
        viewport.last_floor_ceiling_material_consumed_mask != 3u ||
        viewport.gdat_material_palette_floor_ceiling_consumed_count == 0 ||
        viewport.fallback_floor_ceiling_drawn_count != 0 ||
        viewport.blocked_material_draw_count != 0) {
        fprintf(stderr, "FAIL: canonical plane plan did not directly reach M11 "
                "(fetch=%d planes=%d mask=%u palette=%d blocked=%d)\n",
                unexpected_fetches, viewport.asset_floor_ceiling_drawn_count,
                viewport.last_floor_ceiling_material_consumed_mask,
                viewport.gdat_material_palette_floor_ceiling_consumed_count,
                viewport.blocked_material_draw_count);
        failures = 1;
    }

    mismatched = plan;
    mismatched.graphicsset = (uint8_t)(style ^ 1);
    memset(framebuffer, 0, sizeof(framebuffer));
    unexpected_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, style, plan.command_hash, plan.scene_colorkey,
        plan.scene_flags, 0u, plan.highest_light_level, 0u, 0u, 0u, 0u,
        0u, plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &mismatched);
    dm2_v1_render_floor_ceiling(&viewport);
    if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
        viewport.fallback_floor_ceiling_drawn_count != 0 ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
        fputs("FAIL: mismatched MapGraphicsStyle plan was not a callback-free no-draw\n",
              stderr);
        failures = 1;
    }

done:
    dm2_v1_gdat_scene_m11_command_plan_free(&plan);
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures) return 1;
    puts("PASS: canonical GRAPHICSSET floor/ceiling plan reaches M11 directly");
    return 0;
}
