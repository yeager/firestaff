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
    dm2_v1_render_walls(&viewport);
    if (viewport.asset_wall_drawn_count < 2 ||
        viewport.fallback_wall_drawn_count != 0 ||
        viewport.blocked_material_draw_count != 0) {
        fputs("FAIL: G1 center/side wall classes did not consume source material\n", stderr);
        return 1;
    }

    puts("PASS: G1 center/side-ray terrain reaches only source-backed wall material");
    return 0;
}
