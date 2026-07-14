/* G1 terrain classes must drive the existing source-material wall pass.
 * Source: skproject/SKWIN/DME.h::tileTypeIndex -> c_gui_vp.cpp DRAW_WALL. */

#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <string.h>

static int fetch_material(void *user, int index, const uint8_t **pixels,
                          int *width, int *height, int *stride)
{
    static const uint8_t wall_pixels[4] = { 1u, 1u, 1u, 1u };
    int graphicsset;
    int field;
    (void)user;
    if (!dm2_v1_viewport_wall_graphic_address(index, &graphicsset, &field) ||
        graphicsset != 0x2a || field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) {
        return -1;
    }
    *pixels = wall_pixels;
    *width = 2;
    *height = 2;
    *stride = 2;
    return 0;
}

static int fetch_palette(void *user, int index, uint8_t palette[16],
                         uint32_t *hash)
{
    int graphicsset;
    int field;
    (void)user;
    if (!dm2_v1_viewport_wall_graphic_address(index, &graphicsset, &field) ||
        graphicsset != 0x2a || field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) {
        return -1;
    }
    memset(palette, 0, 16u);
    palette[1] = 0x4au;
    *hash = 0x47315449u ^ (uint32_t)index;
    return *hash != 0u ? 0 : -1;
}

static int fetch_door_material(void *user, int index, const uint8_t **pixels,
                               int *width, int *height, int *stride)
{
    static const uint8_t door_pixels[4] = { 1u, 1u, 1u, 1u };
    (void)user;
    if (index != dm2_v1_viewport_door_panel_graphic_index_for_record(
                      DM2_SQ_D0C, 7, 1) &&
        index != dm2_v1_viewport_door_frame_graphic_index_for_square(
                      DM2_SQ_D0C) &&
        index != dm2_v1_viewport_door_button_graphic_index_for_state(1)) {
        return -1;
    }
    *pixels = door_pixels;
    *width = 2;
    *height = 2;
    *stride = 2;
    return 0;
}

static int fetch_door_palette(void *user, int index, uint8_t palette[16],
                              uint32_t *hash)
{
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;
    int stride = 0;

    if (fetch_door_material(user, index, &pixels, &width, &height, &stride) !=
        0) {
        return -1;
    }
    memset(palette, 0, 16u);
    palette[1] = 0x4au;
    *hash = 0x4731444fu ^ (uint32_t)index;
    return *hash != 0u ? 0 : -1;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    if (dm2_v1_viewport_g1_tile_class_to_square_type(0u) != DM2_SQUARE_WALL ||
        dm2_v1_viewport_g1_tile_class_to_square_type(1u) != DM2_SQUARE_FLOOR ||
        dm2_v1_viewport_g1_tile_class_to_square_type(4u) != DM2_SQUARE_DOOR ||
        dm2_v1_viewport_g1_tile_class_to_square_type(2u) >= 0) {
        fputs("FAIL: G1 tileTypeIndex mapping widened or inverted\n", stderr);
        return 1;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_material, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 0x2a, 0x47315431u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0u);
    viewport.squares[DM2_SQ_D0C].square_type =
        (uint8_t)dm2_v1_viewport_g1_tile_class_to_square_type(0u);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_WALL;
    viewport.squares[DM2_SQ_D1R].square_type =
        (uint8_t)dm2_v1_viewport_g1_tile_class_to_square_type(0u);
    viewport.squares[DM2_SQ_D1R].flags = DM2_SQF_HAS_WALL;
    viewport.squares[DM2_SQ_D3L].square_type =
        (uint8_t)dm2_v1_viewport_g1_tile_class_to_square_type(0u);
    viewport.squares[DM2_SQ_D3L].flags = DM2_SQF_HAS_WALL;
    viewport.squares[DM2_SQ_D3R].square_type =
        (uint8_t)dm2_v1_viewport_g1_tile_class_to_square_type(0u);
    viewport.squares[DM2_SQ_D3R].flags = DM2_SQF_HAS_WALL;
    dm2_v1_render_walls(&viewport);
    if (viewport.asset_wall_drawn_count < 4 ||
        viewport.fallback_wall_drawn_count != 0 ||
        viewport.blocked_material_draw_count != 0) {
        fputs("FAIL: G1 center/side/deep wall classes did not consume source material\n", stderr);
        return 1;
    }

    /* A G1 class-4 terrain tag only becomes a door after the runtime has
     * supplied direct DB0 fields. This is the same panel/frame/button
     * transaction consumed by M11; a missing material blocks the whole door
     * rather than painting a generic substitute. */
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_door_material, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_door_palette,
                                                NULL);
    viewport.squares[DM2_SQ_D0C].square_type =
        (uint8_t)dm2_v1_viewport_g1_tile_class_to_square_type(4u);
    viewport.squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_WALL | DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_record_type = 1u;
    viewport.squares[DM2_SQ_D0C].door_gfx_index = 7u;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1u;
    viewport.squares[DM2_SQ_D0C].door_button = 1u;
    viewport.squares[DM2_SQ_D0C].door_button_state = 1u;
    viewport.squares[DM2_SQ_D0C].door_state = 4u;
    dm2_v1_render_doors(&viewport);
    if (viewport.asset_door_panel_drawn_count != 1 ||
        viewport.asset_door_frame_drawn_count != 1 ||
        viewport.asset_door_button_drawn_count != 1 ||
        viewport.fallback_door_drawn_count != 0 ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) !=
            0u) {
        fputs("FAIL: G1 DB0 door did not consume its complete source material plan\n",
              stderr);
        return 1;
    }

    puts("PASS: G1 terrain and direct DB0 door reach only source-backed GDAT material");
    return 0;
}
