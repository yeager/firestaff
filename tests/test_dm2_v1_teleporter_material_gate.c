/* SKProject DRAW_MAP_CHIP teleporter source-material gate. */
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <string.h>

static int withhold_material;

static int fetch_asset(void *user, int gdat_index, const uint8_t **pixels,
                       int *width, int *height, int *stride)
{
    static const uint8_t image[32] = {
        1, 1, 1, 1, 2, 2, 2, 2,
        1, 1, 1, 1, 2, 2, 2, 2,
        1, 1, 1, 1, 2, 2, 2, 2,
        1, 1, 1, 1, 2, 2, 2, 2
    };
    (void)user;
    if (withhold_material ||
        gdat_index != dm2_v1_viewport_teleporter_map_chip_graphic_index()) {
        return -1;
    }
    *pixels = image;
    *width = 8;
    *height = 4;
    *stride = 8;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    (void)user;
    if (gdat_index != dm2_v1_viewport_teleporter_map_chip_graphic_index()) {
        return -1;
    }
    memset(palette, 0, 16u);
    palette[1] = 0x31u;
    palette[2] = 0x32u;
    *hash = 0x54454c50u;
    return 0;
}

int main(void)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_TELEPORTER;
    viewport.tick_count = 1;
    withhold_material = 0;
    dm2_v1_render_teleporter_fields(&viewport);
    if (viewport.asset_teleporter_drawn_count != 1 ||
        viewport.gdat_local_palette_consumed_count == 0 ||
        framebuffer[78 * DM2_VP_WIDTH + 74] != 0x32u) {
        fputs("FAIL: teleporter did not consume its tick-selected local-palette GDAT frame\n", stderr);
        return 1;
    }

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_TELEPORTER;
    withhold_material = 1;
    dm2_v1_render_teleporter_fields(&viewport);
    if (viewport.asset_teleporter_drawn_count != 0 ||
        framebuffer[78 * DM2_VP_WIDTH + 74] != 0x5au ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_TELEPORTER) == 0u) {
        fputs("FAIL: missing teleporter material produced a visual fallback\n", stderr);
        return 1;
    }

    /* A live G1/GDAT scene must not use the old compact placement table.
     * DRAW_TELEPORTER_TILE owns placement through the original per-cell
     * tblGraphicsTeleporterWords/Bytes4 + RAW4 route, which is not yet fully
     * materialized here. */
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 1, 0x54454c50u,
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_TELEPORTER;
    withhold_material = 0;
    dm2_v1_render_teleporter_fields(&viewport);
    if (viewport.asset_teleporter_drawn_count != 0 ||
        framebuffer[78 * DM2_VP_WIDTH + 74] != 0x5au ||
        (viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_TELEPORTER) == 0u) {
        fputs("FAIL: source teleporter accepted compact placement fallback\n", stderr);
        return 1;
    }
    puts("PASS: teleporter fields consume only TELEPORTERS/0/F9 source material");
    return 0;
}
