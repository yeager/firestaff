/* Canonical PC GDAT plane plan -> M11 viewport handoff. The plan is the
 * decoded UPDATE_GFXSET output; no second GDAT callback may stand in for it.
 * Source: skproject/SKULLWIN/c_gui_vp.cpp UPDATE_GFXSET -> DRAW_DUNGEON. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM2_GDAT_SOURCE_VIEWPORT_HEIGHT 136

static int load_canonical_files(uint8_t **graphics, size_t *graphics_size,
                                uint8_t **dungeon, size_t *dungeon_size)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");

    if (!archive || !archive[0] || !graphics || !graphics_size || !dungeon ||
        !dungeon_size) return 0;
    *graphics = NULL;
    *graphics_size = 0u;
    *dungeon = NULL;
    *dungeon_size = 0u;
    if (firestaff_zip_extract_by_suffix(archive, "data/graphics.dat",
                                        graphics, graphics_size) != 0 ||
        !*graphics || !*graphics_size ||
        firestaff_zip_extract_by_suffix(archive, "data/dungeon.dat",
                                        dungeon, dungeon_size) != 0 ||
        !*dungeon || !*dungeon_size) {
        free(*graphics);
        free(*dungeon);
        *graphics = NULL;
        *graphics_size = 0u;
        *dungeon = NULL;
        *dungeon_size = 0u;
        return 0;
    }
    return 1;
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

/* The source-owned plan is passed directly to M11, so this check deliberately
 * compares the complete decoded plane rather than a representative swatch.
 * The test viewport has its initial position/tick state (all zero), matching
 * the mirror decisions below from SET_GRAPHICS_FLIP_FROM_POSITION. */
static uint8_t source_plane_palette_color(
    const DM2_V1_GdatSceneM11Command *command, uint8_t pixel)
{
    int identity = 1;

    if (pixel >= 16u) return pixel;
    for (int i = 0; i < 16; ++i) {
        if (command->palette16[i] != (uint8_t)i) {
            identity = 0;
            break;
        }
    }
    return identity ? pixel : command->palette16[pixel];
}

static int initial_plane_mirror(uint16_t scene_flags, uint8_t kind)
{
    if (kind == 1u) {
        if ((scene_flags & 8u) != 0u) {
            if ((scene_flags & 0x10u) != 0u) return 0;
            return 0; /* initial party/map parity is zero */
        }
        return 0;
    }
    if (kind == 0x20u) {
        if ((scene_flags & 2u) != 0u) {
            if ((scene_flags & 4u) != 0u) return 1; /* initial tick is zero */
            return 1; /* initial party/map parity is zero */
        }
    }
    return 0;
}

static int verify_source_plane_pixels(
    const uint8_t *framebuffer, const DM2_V1_GdatSceneM11Command *command,
    const DM2_V1_GdatSceneBlitRect *rect, int flip_mirror, const char *name)
{
    if (!framebuffer || !command || !rect || !command->pixels ||
        command->width == 0u || command->height == 0u ||
        rect->x < 0 || rect->y < 0 || rect->width == 0u || rect->height == 0u ||
        (unsigned)rect->x + rect->width > DM2_VP_WIDTH ||
        (unsigned)rect->y + rect->height > DM2_VP_HEIGHT) {
        return 0;
    }
    for (uint16_t y = 0u; y < rect->height; ++y) {
        uint16_t sy = (uint16_t)((y * command->height) / rect->height);
        for (uint16_t x = 0u; x < rect->width; ++x) {
            uint16_t sx = (uint16_t)((x * command->width) / rect->width);
            uint8_t expected;
            uint8_t actual;

            if (flip_mirror & 1) sx = (uint16_t)(command->width - 1u - sx);
            if (flip_mirror & 2) sy = (uint16_t)(command->height - 1u - sy);
            expected = source_plane_palette_color(
                command, command->pixels[sy * command->width + sx]);
            actual = framebuffer[(rect->y + y) * DM2_VP_WIDTH + rect->x + x];
            if (actual != expected) {
                fprintf(stderr, "FAIL: %s RECT_%u differs from one-shot source "
                        "plane at %u,%u (actual=%u expected=%u)\n",
                        name, rect->rect_number, x, y, actual, expected);
                return 0;
            }
        }
    }
    return 1;
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
        viewport.gdat_scene_draw_order_consumed_count != 2 ||
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
    if (!verify_source_plane_pixels(framebuffer, &plan->commands[1],
                                    &plan->rects[1],
                                    initial_plane_mirror(plan->scene_flags, 0x20u),
                                    "ceiling") ||
        !verify_source_plane_pixels(framebuffer, &plan->commands[0],
                                    &plan->rects[0],
                                    initial_plane_mirror(plan->scene_flags, 1u),
                                    "floor")) {
        return 0;
    }
    return 1;
}

static int hide_graphicsset_word_entries(DM2_V1_AssetLoader *loader,
                                         int graphicsset,
                                         const uint8_t *fields,
                                         size_t field_count,
                                         uint8_t *saved_types,
                                         size_t saved_capacity)
{
    size_t saved_count = 0u;

    if (!loader || !fields || !saved_types) return -1;
    for (uint16_t i = 0; i < loader->entry_count; ++i) {
        DM2_V1_GdatEntry *entry = &loader->entries[i];
        if (entry->cls1 != DM2_GDAT_CATEGORY_GRAPHICSSET ||
            entry->cls2 != (uint8_t)graphicsset ||
            entry->cls3 != DM2_GDAT_ENTRY_TYPE_WORD_VALUE) {
            continue;
        }
        for (size_t f = 0u; f < field_count; ++f) {
            if (entry->cls4 != fields[f]) continue;
            if (saved_count >= saved_capacity) return -1;
            saved_types[saved_count++] = entry->cls3;
            entry->cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
            break;
        }
    }
    return (int)saved_count;
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

    if (!getenv("FIRESTAFF_DM2_DOS_ARCHIVE") ||
        !getenv("FIRESTAFF_DM2_DOS_ARCHIVE")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        fputs("FAIL: selected canonical DM2 data is unreadable\n", stderr);
        return 1;
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
        static const uint8_t light_fields[] = {
            DM2_GDAT_GFXSET_AMBIANT_LIGHT,
            DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL,
            DM2_GDAT_GFXSET_AMBIANT_DARKNESS
        };
        uint8_t saved_types[3] = { 0 };
        int hidden_count = hide_graphicsset_word_entries(
            &loader, first_style, light_fields,
            sizeof(light_fields) / sizeof(light_fields[0]), saved_types,
            sizeof(saved_types) / sizeof(saved_types[0]));
        DM2_V1_GdatSceneM11CommandPlan missing_light;

        memset(&missing_light, 0, sizeof(missing_light));
        if (hidden_count < 0) {
            fputs("FAIL: could not isolate active GRAPHICSSET light words\n",
                  stderr);
            failures = 1;
        } else if (hidden_count > 0) {
            if (!dm2_v1_gdat_scene_m11_command_plan_build(
                    &loader, (uint8_t)first_style, &missing_light) ||
                !missing_light.valid ||
                missing_light.graphicsset != (uint8_t)first_style ||
                missing_light.scene_colorkey != mismatched.scene_colorkey ||
                missing_light.scene_flags != mismatched.scene_flags ||
                missing_light.ambient_light != 0u ||
                missing_light.highest_light_level != 0u ||
                missing_light.ambient_darkness != 0u ||
                missing_light.commands[0].raw_hash !=
                    mismatched.commands[0].raw_hash ||
                missing_light.commands[1].raw_hash !=
                    mismatched.commands[1].raw_hash ||
                missing_light.command_hash == mismatched.command_hash ||
                !verify_direct_handoff(first_style, &missing_light)) {
                fputs("FAIL: missing GRAPHICSSET light words blocked the "
                      "source scene material plan\n", stderr);
                failures = 1;
            }
            dm2_v1_gdat_scene_m11_command_plan_free(&missing_light);
        }
        for (uint16_t i = 0; i < loader.entry_count && hidden_count > 0; ++i) {
            DM2_V1_GdatEntry *entry = &loader.entries[i];
            if (entry->cls1 != DM2_GDAT_CATEGORY_GRAPHICSSET ||
                entry->cls2 != (uint8_t)first_style ||
                entry->cls3 != DM2_GDAT_ENTRY_TYPE_TEXT) {
                continue;
            }
            for (size_t f = 0u; f < sizeof(light_fields) / sizeof(light_fields[0]); ++f) {
                if (entry->cls4 == light_fields[f]) {
                    entry->cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
                    --hidden_count;
                    break;
                }
            }
        }
    }

    {
        DM2_V1_GdatSceneM11CommandPlan altered_order = mismatched;

        altered_order.draw_order[0] = 0u;
        altered_order.draw_order[1] = 1u;
        memset(framebuffer, 0, sizeof(framebuffer));
        unexpected_fetches = 0;
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
        dm2_v1_viewport_set_asset_palette_provider(
            &viewport, unexpected_palette_fetch, NULL);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, first_style, altered_order.command_hash,
            altered_order.scene_colorkey, altered_order.scene_flags, 0u,
            altered_order.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            altered_order.ambient_darkness);
        dm2_v1_viewport_set_gdat_scene_material_plan(&viewport, &altered_order);
        if (viewport.gdat_scene_material_plan != NULL) {
            fputs("FAIL: altered c_gui_vp plane order remained attached\n",
                  stderr);
            failures = 1;
        }
        dm2_v1_render_floor_ceiling(&viewport);
        if (unexpected_fetches != 0 || viewport.asset_floor_ceiling_drawn_count != 0 ||
            viewport.gdat_scene_draw_order_consumed_count != 0 ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u) {
            fputs("FAIL: altered c_gui_vp plane order was not callback-free no-draw\n",
                  stderr);
            failures = 1;
        }
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
    if (viewport.gdat_scene_material_plan != NULL) {
        fputs("FAIL: altered QUERY_BLIT_RECT receipt remained attached\n",
              stderr);
        failures = 1;
    }
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
