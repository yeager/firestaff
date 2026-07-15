/* Canonical PC GDAT plane plan -> M11 viewport handoff. The plan is the
 * decoded UPDATE_GFXSET output; no second GDAT callback may stand in for it.
 * Source: skproject/SKULLWIN/c_gui_vp.cpp UPDATE_GFXSET -> DRAW_DUNGEON. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM2_GDAT_SOURCE_VIEWPORT_HEIGHT 136

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

static int verify_direct_handoff(int style,
                                 const DM2_V1_GdatSceneM11CommandPlan *plan)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int needs_local_palette_remap =
        plan->commands[0].format == DM2_IMG_FMT_IMG3 ||
        plan->commands[0].format == DM2_IMG_FMT_U4 ||
        plan->commands[1].format == DM2_IMG_FMT_IMG3 ||
        plan->commands[1].format == DM2_IMG_FMT_U4;

    memset(framebuffer, 0, sizeof(framebuffer));
    unexpected_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, style, plan->command_hash, plan->scene_colorkey,
        plan->scene_flags, 0u, plan->highest_light_level, 0u, 0u, 0u, 0u,
        0u, plan->ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, plan);
    dm2_v1_render_floor_ceiling(&viewport);
    if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 2 ||
        viewport.last_floor_ceiling_material_consumed_mask != 3u ||
        (needs_local_palette_remap &&
         viewport.gdat_material_palette_floor_ceiling_consumed_count == 0) ||
        viewport.fallback_floor_ceiling_drawn_count != 0 ||
        viewport.blocked_material_draw_count != 0) {
        fprintf(stderr, "FAIL: GRAPHICSSET %d plane plan did not directly reach M11 "
                "(fetch=%d planes=%d mask=%u palette=%d blocked=%d)\n",
                style, unexpected_fetches, viewport.asset_floor_ceiling_drawn_count,
                viewport.last_floor_ceiling_material_consumed_mask,
                viewport.gdat_material_palette_floor_ceiling_consumed_count,
                viewport.blocked_material_draw_count);
        return 0;
    }
    return 1;
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
    DM2_V1_GdatSceneQueryBlitRectReceipt rect_receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t seen_styles[256];
    int first_style = -1;
    int style_count = 0;
    int failures = 0;

    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&plan, 0, sizeof(plan));
    memset(&mismatched, 0, sizeof(mismatched));
    memset(&rect_receipt, 0, sizeof(rect_receipt));
    memset(seen_styles, 0, sizeof(seen_styles));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0) {
        fputs("FAIL: canonical DM2 GDAT/G1 input was not accepted\n", stderr);
        failures = 1;
        goto done;
    }
    if (!dm2_v1_gdat_scene_query_blit_rect_receipt(&loader, &rect_receipt) ||
        !rect_receipt.valid ||
        rect_receipt.floor_rect_number != DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
        rect_receipt.ceiling_rect_number !=
            DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER ||
        rect_receipt.table_hash == 0u || rect_receipt.floor_row_hash == 0u ||
        rect_receipt.ceiling_row_hash == 0u) {
        fputs("FAIL: canonical GDAT lacks source-owned QUERY_BLIT_RECT 700/701 rows\n",
              stderr);
        failures = 1;
        goto done;
    }
    for (int level = 0; level < dungeon.level_count; ++level) {
        int candidate = dm2_v1_dungeon_get_map_graphics_style(&dungeon, level);
        if (candidate < 0 || candidate > 0xff || seen_styles[candidate]) continue;
        seen_styles[candidate] = 1;
        if (!dm2_v1_gdat_scene_m11_command_plan_build(
                &loader, (uint8_t)candidate, &plan) ||
            !plan.valid || plan.graphicsset != (uint8_t)candidate ||
            plan.command_hash == 0u || plan.commands[0].raw_hash == 0u ||
            plan.commands[1].raw_hash == 0u || plan.commands[0].decoded_hash == 0u ||
            plan.commands[1].decoded_hash == 0u ||
            plan.commands[0].palette_hash == 0u ||
            plan.commands[1].palette_hash == 0u ||
            !plan.query_blit_rect.valid ||
            plan.query_blit_rect_hash == 0u ||
            plan.query_blit_rect.table_hash != rect_receipt.table_hash ||
            plan.query_blit_rect.floor_row_hash != rect_receipt.floor_row_hash ||
            plan.query_blit_rect.ceiling_row_hash != rect_receipt.ceiling_row_hash) {
            fprintf(stderr, "FAIL: G1 MapGraphicsStyle %d yielded no complete "
                    "GRAPHICSSET plane plan\n", candidate);
            failures = 1;
            dm2_v1_gdat_scene_m11_command_plan_free(&plan);
            continue;
        }
        if (plan.rects[0].rect_number != DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
            plan.rects[1].rect_number != DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER ||
            plan.rects[0].x != 0 ||
            plan.rects[0].y + plan.commands[0].height !=
                DM2_GDAT_SOURCE_VIEWPORT_HEIGHT ||
            plan.rects[1].x != 0 || plan.rects[1].y != 0 ||
            plan.rects[0].width != plan.commands[0].width ||
            plan.rects[0].height != plan.commands[0].height ||
            plan.rects[1].width != plan.commands[1].width ||
            plan.rects[1].height != plan.commands[1].height) {
            fprintf(stderr, "FAIL: G1 MapGraphicsStyle %d did not decode "
                    "QUERY_BLIT_RECT 700/701 geometry\n", candidate);
            failures = 1;
            dm2_v1_gdat_scene_m11_command_plan_free(&plan);
            continue;
        }
        if (plan.commands[0].decoded_hash !=
                dm2_v1_gdat_scene_m11_command_pixel_hash(&plan.commands[0]) ||
            plan.commands[1].decoded_hash !=
                dm2_v1_gdat_scene_m11_command_pixel_hash(&plan.commands[1]) ||
            plan.commands[0].geometry_hash == 0u ||
            plan.commands[1].geometry_hash == 0u ||
            plan.commands[0].geometry_hash !=
                dm2_v1_gdat_scene_m11_command_geometry_hash(
                    &plan.commands[0], &plan.rects[0]) ||
            plan.commands[1].geometry_hash !=
                dm2_v1_gdat_scene_m11_command_geometry_hash(
                    &plan.commands[1], &plan.rects[1])) {
            fprintf(stderr, "FAIL: G1 MapGraphicsStyle %d scene command lost "
                    "decoded pixels or QUERY_BLIT_RECT geometry\n", candidate);
            failures = 1;
            dm2_v1_gdat_scene_m11_command_plan_free(&plan);
            continue;
        }
        if (!verify_direct_handoff(candidate, &plan)) failures = 1;
        if (dm2_v1_runtime_g1_scene_map_token(level, candidate, 0) == 0u ||
            dm2_v1_runtime_g1_scene_map_token(level, candidate, 0) ==
                dm2_v1_runtime_g1_scene_map_token(level, candidate ^ 1, 0)) {
            fputs("FAIL: G1 MapGraphicsStyle is absent from the M11 scene token\n", stderr);
            failures = 1;
        }
        if (first_style < 0) {
            first_style = candidate;
            mismatched = plan;
            memset(&plan, 0, sizeof(plan));
        }
        ++style_count;
        dm2_v1_gdat_scene_m11_command_plan_free(&plan);
    }
    if (first_style < 0 || style_count == 0) {
        fputs("FAIL: no G1 MapGraphicsStyle yielded a complete plane plan\n", stderr);
        failures = 1;
        goto done;
    }

    {
        DM2_V1_GdatSceneM11CommandPlan altered_geometry = mismatched;

        ++altered_geometry.rects[0].x;
        memset(framebuffer, 0, sizeof(framebuffer));
        unexpected_fetches = 0;
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
        dm2_v1_viewport_set_asset_palette_provider(
            &viewport, unexpected_palette_fetch, NULL);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, first_style, altered_geometry.command_hash,
            altered_geometry.scene_colorkey, altered_geometry.scene_flags, 0u,
            altered_geometry.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            altered_geometry.ambient_darkness);
        dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &altered_geometry);
        dm2_v1_render_floor_ceiling(&viewport);
        if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
            viewport.fallback_floor_ceiling_drawn_count != 0 ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
            fputs("FAIL: altered QUERY_BLIT_RECT geometry was not a callback-free no-draw\n",
                  stderr);
            failures = 1;
        }
    }

    {
        uint8_t original_pixel = mismatched.commands[0].pixels[0];

        mismatched.commands[0].pixels[0] ^= 1u;
        memset(framebuffer, 0, sizeof(framebuffer));
        unexpected_fetches = 0;
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
        dm2_v1_viewport_set_asset_palette_provider(
            &viewport, unexpected_palette_fetch, NULL);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, first_style, mismatched.command_hash,
            mismatched.scene_colorkey, mismatched.scene_flags, 0u,
            mismatched.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            mismatched.ambient_darkness);
        dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &mismatched);
        dm2_v1_render_floor_ceiling(&viewport);
        mismatched.commands[0].pixels[0] = original_pixel;
        if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
            viewport.fallback_floor_ceiling_drawn_count != 0 ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
            fputs("FAIL: altered canonical floor pixels were not a callback-free no-draw\n",
                  stderr);
            failures = 1;
        }
    }

    {
        uint8_t original_palette = mismatched.commands[0].palette16[0];

        mismatched.commands[0].palette16[0] ^= 1u;
        memset(framebuffer, 0, sizeof(framebuffer));
        unexpected_fetches = 0;
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
        dm2_v1_viewport_set_asset_palette_provider(
            &viewport, unexpected_palette_fetch, NULL);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, first_style, mismatched.command_hash,
            mismatched.scene_colorkey, mismatched.scene_flags, 0u,
            mismatched.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            mismatched.ambient_darkness);
        dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &mismatched);
        dm2_v1_render_floor_ceiling(&viewport);
        mismatched.commands[0].palette16[0] = original_palette;
        if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
            viewport.fallback_floor_ceiling_drawn_count != 0 ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
            fputs("FAIL: altered canonical floor palette was not a callback-free no-draw\n",
                  stderr);
            failures = 1;
        }
    }

    mismatched.graphicsset = (uint8_t)(first_style ^ 1);
    memset(framebuffer, 0, sizeof(framebuffer));
    unexpected_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, first_style, mismatched.command_hash, mismatched.scene_colorkey,
        mismatched.scene_flags, 0u, mismatched.highest_light_level, 0u, 0u, 0u,
        0u, 0u, mismatched.ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &mismatched);
    if (viewport.gdat_scene_material_plan != NULL) {
        fputs("FAIL: mismatched MapGraphicsStyle plan remained attached\n", stderr);
        failures = 1;
    }
    dm2_v1_render_floor_ceiling(&viewport);
    if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
        viewport.fallback_floor_ceiling_drawn_count != 0 ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
        fputs("FAIL: mismatched MapGraphicsStyle plan was not a callback-free no-draw\n",
              stderr);
        failures = 1;
    }

    mismatched.graphicsset = (uint8_t)first_style;
    mismatched.query_blit_rect.floor_row_hash ^= 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    unexpected_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, first_style, mismatched.command_hash, mismatched.scene_colorkey,
        mismatched.scene_flags, 0u, mismatched.highest_light_level, 0u, 0u, 0u,
        0u, 0u, mismatched.ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &mismatched);
    dm2_v1_render_floor_ceiling(&viewport);
    if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
        viewport.fallback_floor_ceiling_drawn_count != 0 ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
        fputs("FAIL: altered QUERY_BLIT_RECT receipt was not a callback-free no-draw\n",
              stderr);
        failures = 1;
    }

done:
    dm2_v1_gdat_scene_m11_command_plan_free(&mismatched);
    dm2_v1_gdat_scene_m11_command_plan_free(&plan);
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures) return 1;
    printf("PASS: %d canonical G1 GRAPHICSSET plane plans reach M11 directly\n",
           style_count);
    return 0;
}
