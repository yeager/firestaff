/* DB4 scene-owner receipt must reach the matching viewport F9 material draw.
 * Source: skproject/SKULLWIN/c_map.cpp -> DME.h Creature::CreatureType ->
 * QUERY_DUNGEON_MAP_CHIP_PICT -> DRAW_CHIP_OF_MAGIC_MAP. */

#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int palette_mismatch;

static int fetch_asset(void *user, int index, const uint8_t **pixels,
                       int *width, int *height, int *stride)
{
    static const uint8_t image[4] = { 1, 1, 1, 1 };
    (void)user;
    if (index != dm2_v1_viewport_creature_graphic_index(7, 0)) return -1;
    *pixels = image;
    *width = 2;
    *height = 2;
    *stride = 2;
    return 0;
}

static int fetch_palette(void *user, int index, uint8_t palette[16],
                         uint32_t *hash)
{
    (void)user;
    if (index != dm2_v1_viewport_creature_graphic_index(7, 0)) return -1;
    memset(palette, 0, 16u);
    palette[1] = 0x4a;
    *hash = palette_mismatch ? 0x22222222u : 0x11111111u;
    return 0;
}

static int run_case(int mismatch)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;

    palette_mismatch = mismatch;
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 7;
    viewport.creatures[0].source_kind = 2;
    viewport.creatures[0].map_x = 12;
    viewport.creatures[0].map_y = 9;
    viewport.creatures[0].screen_x = 160;
    viewport.creatures[0].screen_y = 90;
    viewport.creatures[0].health_pct = 100;
    dm2_v1_viewport_set_g1_scene_creature_material(
        &viewport, 1, 12, 9, 7,
        dm2_v1_viewport_creature_graphic_index(7, 0),
        2, 2, 2, 0x11111111u);
    dm2_v1_render_creatures(&viewport);
    return mismatch
        ? viewport.asset_creature_drawn_count == 0 &&
              viewport.g1_scene_creature_material_consumed_count == 0 &&
              viewport.blocked_material_draw_count == 1
        : viewport.asset_creature_drawn_count == 1 &&
              viewport.g1_scene_creature_material_consumed_count == 1 &&
              viewport.blocked_material_draw_count == 0;
}

int main(void)
{
    if (!run_case(0) || !run_case(1)) {
        fputs("FAIL: G1 scene owner did not gate matching viewport material\n",
              stderr);
        return 1;
    }
    puts("PASS: G1 DB4 scene receipt owns matching viewport F9 material");
    return 0;
}
