/* Canonical PC G1 MapGraphicsStyle -> GRAPHICSSET wall plan -> M11 proof.
 * Source: skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_WALL/QUERY_TEMP_PICST. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_viewport_renderer.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const DM2_V1_AssetLoader *loader;
    int graphicsset;
    int fetch_calls;
    int palette_calls;
    int reject_after;
    /* GRAPHICSSET wall fields occupy the source's 0x00..0x3f field space,
     * not the compact viewport-panel count. */
    uint8_t *pixels[64];
} WallTrace;

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

static int wall_provider_address(int gdat_index, int expected_graphicsset,
                                 int *out_field)
{
    int graphicsset = -1;
    int field = -1;

    if (!dm2_v1_viewport_wall_graphic_address(
            gdat_index, &graphicsset, &field) ||
        graphicsset != expected_graphicsset ||
        field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST || field >= 0x40) {
        return 0;
    }
    *out_field = field;
    return 1;
}

static int fetch_wall(void *user, int gdat_index, const uint8_t **out_pixels,
                      int *out_width, int *out_height, int *out_stride)
{
    WallTrace *trace = user;
    int field = -1;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint8_t *pixels;

    ++trace->fetch_calls;
    if ((trace->reject_after > 0 && trace->fetch_calls >= trace->reject_after) ||
        !wall_provider_address(gdat_index, trace->graphicsset, &field)) {
        return -1;
    }
    pixels = dm2_v1_asset_load_image_field(
        trace->loader, DM2_GDAT_CATEGORY_GRAPHICSSET, trace->graphicsset,
        field, &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 || format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(pixels);
        return -1;
    }
    if (field >= (int)(sizeof(trace->pixels) / sizeof(trace->pixels[0])) ||
        trace->pixels[field]) {
        dm2_v1_asset_free_pixels(pixels);
        return -1;
    }
    trace->pixels[field] = pixels;
    *out_pixels = pixels;
    *out_width = width;
    *out_height = height;
    *out_stride = width;
    return 0;
}

static int fetch_wall_palette(void *user, int gdat_index,
                              uint8_t out_palette16[16], uint32_t *out_hash)
{
    WallTrace *trace = user;
    int field = -1;

    ++trace->palette_calls;
    if (!wall_provider_address(gdat_index, trace->graphicsset, &field)) {
        return -1;
    }
    return dm2_v1_asset_load_image_local_palette(
        trace->loader, DM2_GDAT_CATEGORY_GRAPHICSSET, trace->graphicsset,
        field, out_palette16, out_hash) && *out_hash != 0u ? 0 : -1;
}

static void free_wall_pixels(WallTrace *trace)
{
    for (size_t i = 0; i < sizeof(trace->pixels) / sizeof(trace->pixels[0]);
         ++i) {
        dm2_v1_asset_free_pixels(trace->pixels[i]);
        trace->pixels[i] = NULL;
    }
}

static void mark_plan_walls_visible(
    DM2_V1_ViewportState *viewport,
    const DM2_V1_GdatWallM11CommandPlan *plan)
{
    if (!viewport || !plan) return;
    for (int i = 0; i < plan->command_count; ++i) {
        int square = plan->commands[i].view_square;
        if (square >= 0 && square < DM2_SQ_COUNT) {
            viewport->squares[square].flags |= DM2_SQF_HAS_WALL;
        }
    }
}

static int style_has_complete_wall_plan(const DM2_V1_AssetLoader *loader,
                                        int graphicsset)
{
    DM2_V1_GdatWallM11CommandPlan plan;
    int complete;

    memset(&plan, 0, sizeof(plan));
    complete = dm2_v1_gdat_wall_m11_command_plan_build(
        loader, (uint8_t)graphicsset, &plan) && plan.valid &&
        plan.command_count > 0 && plan.command_hash != 0u;
    dm2_v1_gdat_wall_m11_command_plan_free(&plan);
    return complete;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_GdatSceneM11CommandPlan scene_plan;
    DM2_V1_GdatWallM11CommandPlan wall_plan;
    DM2_V1_GdatWallM11CommandPlan moving_wall_plan;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    WallTrace trace;
    int graphicsset = -1;
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
    memset(&scene_plan, 0, sizeof(scene_plan));
    memset(&wall_plan, 0, sizeof(wall_plan));
    memset(&moving_wall_plan, 0, sizeof(moving_wall_plan));
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
                &loader, (uint8_t)candidate, &scene_plan) &&
            style_has_complete_wall_plan(&loader, candidate)) {
            graphicsset = candidate;
            break;
        }
        dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
    }
    if (graphicsset < 0) {
        fputs("FAIL: no canonical MapGraphicsStyle has a complete wall plan\n",
              stderr);
        failures = 1;
        goto done;
    }
    if (!dm2_v1_gdat_wall_m11_command_plan_build(
            &loader, (uint8_t)graphicsset, &wall_plan) || !wall_plan.valid ||
        wall_plan.graphicsset != (uint8_t)graphicsset ||
        wall_plan.command_count == 0 || wall_plan.command_hash == 0u) {
        fputs("FAIL: canonical GRAPHICSSET wall command plan was incomplete\n", stderr);
        failures = 1;
        goto done;
    }
    if (!dm2_v1_gdat_wall_m11_command_plan_build_for_movement(
            &loader, (uint8_t)graphicsset, 1, &moving_wall_plan) ||
        !moving_wall_plan.valid ||
        moving_wall_plan.graphicsset != (uint8_t)graphicsset ||
        moving_wall_plan.command_count != wall_plan.command_count ||
        moving_wall_plan.command_hash == 0u ||
        moving_wall_plan.command_hash == wall_plan.command_hash) {
        fputs("FAIL: canonical moving GRAPHICSSET wall command plan was incomplete\n",
              stderr);
        failures = 1;
        goto done;
    }
    {
        static const int expected_squares[] = {
            DM2_SQ_D0R, DM2_SQ_D0L,
            DM2_SQ_D1L, DM2_SQ_D1R, DM2_SQ_D1C,
            DM2_SQ_D2L, DM2_SQ_D2R, DM2_SQ_D2C,
            DM2_SQ_D3L, DM2_SQ_D3R
        };
        static const int expected_passes[] = {
            9, 11, 12, 13, 14, 15, 16, 17, 18, 19
        };
        DM2_V1_ViewportState order_viewport;
        DM2_V1_WallPanelRenderPlan order_plan;
        uint8_t order_framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

        memset(order_framebuffer, 0, sizeof(order_framebuffer));
        dm2_v1_viewport_init(&order_viewport, order_framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_gdat_scene_control(
            &order_viewport, 1, graphicsset, scene_plan.command_hash,
            scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
            scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            scene_plan.ambient_darkness);
        if (!dm2_v1_viewport_build_wall_panel_render_plan(
                &order_viewport, &order_plan) ||
            order_plan.panel_count != (int)(sizeof(expected_squares) /
                                             sizeof(expected_squares[0]))) {
            fputs("FAIL: source wall pass plan was incomplete\n", stderr);
            failures = 1;
            goto done;
        }
        for (size_t i = 0; i < sizeof(expected_squares) / sizeof(expected_squares[0]);
             ++i) {
            if (order_plan.panels[i].view_square != expected_squares[i] ||
                order_plan.panels[i].render_step != expected_passes[i] ||
                dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(
                    expected_squares[i]) != expected_passes[i]) {
                fputs("FAIL: M11 wall plan diverged from SKProject table1d7029\n",
                      stderr);
                failures = 1;
                goto done;
            }
        }
        if (dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(DM2_SQ_D3C) >= 0 ||
            dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(DM2_SQ_D0C) >= 0) {
            fputs("FAIL: a non-geometry center cell was promoted to DRAW_WALL\n",
                  stderr);
            failures = 1;
            goto done;
        }
    }
    {
        static const int side_squares[] = {
            DM2_SQ_D3L, DM2_SQ_D3R, DM2_SQ_D2L, DM2_SQ_D2R
        };
        for (size_t side = 0; side < sizeof(side_squares) / sizeof(side_squares[0]); ++side) {
            const DM2_V1_GdatWallM11Command *command = NULL;
            int field = dm2_v1_viewport_wall_field_for_square(side_squares[side]);
            int source_cell = field - 0x22;
            for (int i = 0; i < wall_plan.command_count; ++i) {
                if (wall_plan.commands[i].view_square == side_squares[side]) {
                    command = &wall_plan.commands[i];
                    break;
                }
            }
            if (!command || command->field !=
                    dm2_v1_viewport_wall_field_for_square(side_squares[side]) ||
                !command->raw_hash || !command->decoded_hash ||
                !command->palette_hash || !command->width || !command->height ||
                !command->material_source_bytes ||
                command->material_source_byte_count == 0u ||
                !command->material_receipt_hash ||
                !command->geometry_hash || field < 0x22 ||
                command->rect_number != (uint16_t)(0x2be + source_cell) ||
                !command->rect_table_hash || !command->rect_row_hash ||
                !command->metadata_hash || !command->source_width ||
                !command->source_height || !command->destination_width ||
                !command->destination_height ||
                command->source_x + command->source_width > command->width ||
                command->source_y + command->source_height > command->height) {
                fputs("FAIL: canonical side-wall command lacks RAW4 geometry\n", stderr);
                failures = 1;
                goto done;
            }
        }
    }
    for (int i = 0; i < wall_plan.command_count; ++i) {
        int resolved_graphicsset = -1;
        int resolved_field = -1;
        const DM2_V1_GdatWallM11Command *command = &wall_plan.commands[i];
        if (wall_plan.graphicsset != (uint8_t)graphicsset ||
            !dm2_v1_viewport_wall_graphic_address(
                dm2_v1_viewport_wall_graphic_index_for_graphicsset(
                    graphicsset, command->view_square),
                &resolved_graphicsset, &resolved_field) ||
            resolved_graphicsset != graphicsset ||
            resolved_field != command->field) {
            fputs("FAIL: real wall command was not bound to its live GRAPHICSSET address\n",
                  stderr);
            failures = 1;
            goto done;
        }
    }

    memset(&trace, 0, sizeof(trace));
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &wall_plan);
    mark_plan_walls_visible(&viewport, &wall_plan);
    dm2_v1_render_walls(&viewport);
    if (trace.fetch_calls != 0 || trace.palette_calls != 0 ||
        viewport.asset_wall_drawn_count != wall_plan.command_count ||
        viewport.gdat_wall_material_plan_consumed_count !=
            wall_plan.command_count ||
        viewport.fallback_wall_drawn_count != 0 ||
        viewport.last_dungeon_wall_material_required_mask == 0u ||
        viewport.last_dungeon_wall_material_required_mask !=
            viewport.last_dungeon_wall_material_consumed_mask ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL)) {
        fprintf(stderr, "FAIL: canonical GRAPHICSSET wall material did not "
                "reach M11 directly as one complete no-fallback plan "
                "(fetch=%d palette=%d draw=%d plan=%d required=%u consumed=%u "
                "fallback=%d blocked=%u)\n", trace.fetch_calls,
                trace.palette_calls, viewport.asset_wall_drawn_count,
                viewport.gdat_wall_material_plan_consumed_count,
                viewport.last_dungeon_wall_material_required_mask,
                viewport.last_dungeon_wall_material_consumed_mask,
                viewport.fallback_wall_drawn_count, viewport.blocked_material_mask);
        failures = 1;
    }
    {
        DM2_V1_GdatWallM11CommandPlan malformed_plan = wall_plan;
        int altered = 0;

        for (int i = 0; i < malformed_plan.command_count; ++i) {
            if (malformed_plan.commands[i].view_square == DM2_SQ_D2L) {
                ++malformed_plan.commands[i].destination_x;
                altered = 1;
                break;
            }
        }
        if (!altered) {
            fputs("FAIL: canonical plan omitted D2L side-wall receipt\n", stderr);
            failures = 1;
            goto done;
        }
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, graphicsset, scene_plan.command_hash,
            scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
            scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            scene_plan.ambient_darkness);
        dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &malformed_plan);
        mark_plan_walls_visible(&viewport, &malformed_plan);
        dm2_v1_render_walls(&viewport);
        if (viewport.asset_wall_drawn_count != 0 ||
            viewport.last_dungeon_wall_material_consumed_mask != 0u ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u) {
            fputs("FAIL: altered side-wall M11 destination did not fail closed\n",
                  stderr);
            failures = 1;
        }
    }
    {
        DM2_V1_GdatWallM11CommandPlan malformed_plan = wall_plan;

        malformed_plan.commands[0].material_receipt_hash = 0u;
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, graphicsset, scene_plan.command_hash,
            scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
            scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            scene_plan.ambient_darkness);
        dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &malformed_plan);
        mark_plan_walls_visible(&viewport, &malformed_plan);
        dm2_v1_render_walls(&viewport);
        if (viewport.asset_wall_drawn_count != 0 ||
            viewport.last_dungeon_wall_material_consumed_mask != 0u ||
            (viewport.blocked_material_mask &
             DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u) {
            fputs("FAIL: altered wall GFX256 receipt reached M11\n", stderr);
            failures = 1;
        }
    }
    {
        const uint16_t selected =
            (uint16_t)((1u << DM2_SQ_D3L) | (1u << DM2_SQ_D2R) |
                       (1u << DM2_SQ_D0L));

        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_party(&viewport, 3, 12, 9);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_gdat_scene_control(
            &viewport, 1, graphicsset, scene_plan.command_hash,
            scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
            scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
            scene_plan.ambient_darkness);
        dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &wall_plan);
        viewport.squares[DM2_SQ_D3L].flags |= DM2_SQF_HAS_WALL;
        viewport.squares[DM2_SQ_D2R].flags |= DM2_SQF_HAS_WALL;
        viewport.squares[DM2_SQ_D0L].flags |= DM2_SQF_HAS_WALL;
        dm2_v1_render_walls(&viewport);
        {
            DM2_V1_WallPanelRenderPlan selected_plan;
            if (!dm2_v1_viewport_build_wall_panel_render_plan(
                    &viewport, &selected_plan) ||
                selected_plan.party_direction != 3 ||
                selected_plan.selected_square_mask != selected ||
                selected_plan.panel_count != 3 ||
                viewport.asset_wall_drawn_count != 3 ||
                viewport.gdat_wall_material_plan_consumed_count != 3 ||
                viewport.last_dungeon_wall_material_required_mask != selected ||
                viewport.last_dungeon_wall_material_consumed_mask != selected ||
                (viewport.blocked_material_mask &
                 DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u) {
                fputs("FAIL: M10 wall plan did not consume the selected depth/direction cells\n",
                      stderr);
                failures = 1;
            }
        }
    }
    /* DM2_DRAW_WALL changes its signed RAW4 query while the party moves.
     * A stationary command plan must never be replayed as a substitute for
     * that live clip/offset transaction. */
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &wall_plan);
    mark_plan_walls_visible(&viewport, &wall_plan);
    dm2_v1_viewport_set_gdat_scene_movement_active(&viewport, 1);
    dm2_v1_render_walls(&viewport);
    if (viewport.asset_wall_drawn_count != 0 ||
        viewport.gdat_wall_material_plan_consumed_count != 0 ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u) {
        fputs("FAIL: moving M10 frame replayed stationary RAW4 wall geometry\n",
              stderr);
        failures = 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &moving_wall_plan);
    mark_plan_walls_visible(&viewport, &moving_wall_plan);
    dm2_v1_viewport_set_gdat_scene_movement_active(&viewport, 1);
    dm2_v1_render_walls(&viewport);
    if (viewport.asset_wall_drawn_count != moving_wall_plan.command_count ||
        viewport.gdat_wall_material_plan_consumed_count !=
            moving_wall_plan.command_count ||
        viewport.fallback_wall_drawn_count != 0 ||
        viewport.last_dungeon_wall_material_required_mask == 0u ||
        viewport.last_dungeon_wall_material_required_mask !=
            viewport.last_dungeon_wall_material_consumed_mask ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u) {
        fputs("FAIL: moving M10 frame did not consume the signed RAW4 wall plan\n",
              stderr);
        failures = 1;
    }
    /* UPDATE_GFXSET is a single G1 transaction.  A wall plan retained from
     * the old control hash must be detached before M10 can query any fallback
     * provider or draw a stale GRAPHICSSET panel. */
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_viewport_set_gdat_wall_material_plan(&viewport, &wall_plan);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash ^ 1u,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_render_walls(&viewport);
    if (viewport.gdat_wall_material_plan != NULL ||
        viewport.asset_wall_drawn_count != 0 ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u) {
        fputs("FAIL: stale G1 wall plan did not detach before M10 draw\n", stderr);
        failures = 1;
    }
    memset(&trace, 0, sizeof(trace));
    trace.loader = &loader;
    trace.graphicsset = graphicsset;
    trace.reject_after = 2;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_wall_palette,
                                                &trace);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, graphicsset, scene_plan.command_hash,
        scene_plan.scene_colorkey, scene_plan.scene_flags, 0u,
        scene_plan.highest_light_level, 0u, 0u, 0u, 0u, 0u,
        scene_plan.ambient_darkness);
    dm2_v1_render_walls(&viewport);
    if (trace.fetch_calls != 0 || trace.palette_calls != 0 ||
        viewport.asset_wall_drawn_count != 0 ||
        viewport.fallback_wall_drawn_count != 0 ||
        viewport.last_dungeon_wall_material_consumed_mask != 0u ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) ==
            0u) {
        fputs("FAIL: incomplete canonical wall material did not fail closed\n",
              stderr);
        failures = 1;
    }
    free_wall_pixels(&trace);

done:
    dm2_v1_gdat_wall_m11_command_plan_free(&moving_wall_plan);
    dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
    dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures) return 1;
    puts("PASS: canonical GRAPHICSSET wall plan reaches M11 without fallback");
    return 0;
}
