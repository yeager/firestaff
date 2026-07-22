/* DM2 V1 wall-ornament material-class gate.
 * skproject/SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE (^32CB:15B8) consumes a
 * WALL_GFX image through QUERY_TEMP_PICST/DRAW_TEMP_PICST.  Firestaff treats
 * the wall ornament as a distinct dungeon material class: missing source
 * material blocks the frame instead of leaving a blank wall or procedural
 * substitute.  Exact destination rectangles are owned by the runtime plan.
 */
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <string.h>

static int withhold_material;
static int plan_valid;

static int fetch_asset(void *user, int gdat_index, const uint8_t **pixels,
                       int *width, int *height, int *stride)
{
    static const uint8_t image[16] = {
        1, 1, 2, 2, 1, 1, 2, 2,
        1, 1, 2, 2, 1, 1, 2, 2
    };
    int expected;
    (void)user;

    expected = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    if (withhold_material || gdat_index != expected) {
        return -1;
    }
    *pixels = image;
    *width = 4;
    *height = 4;
    *stride = 4;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    int expected = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    (void)user;
    if (gdat_index != expected) {
        return -1;
    }
    memset(palette, 0, 16u);
    palette[1] = 0x31u;
    palette[2] = 0x32u;
    *hash = 0x574f524eu;
    return 0;
}

int main(void)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;
    DM2_V1_WallOrnamentRenderPlan plan;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);

    /* Visible D1C wall with a WALL_GFX ornament index. */
    viewport.squares[DM2_SQ_D1C].square_type = DM2_SQUARE_WALL;
    viewport.squares[DM2_SQ_D1C].wall_ornate_gfx_index = 7u;

    /* Without a source-owned placement plan the ornament is required but
     * not drawable, so the frame must block rather than invent a destination. */
    withhold_material = 0;
    plan_valid = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0) {
        fputs("FAIL: wall ornament did not block without source placement plan\n",
              stderr);
        return 1;
    }

    /* Provide a runtime-bound placement plan.  The renderer may now consume
     * the fetched material at the source-owned destination. */
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.ornament_count = 1;
    plan.ornaments[0].view_square = DM2_SQ_D1C;
    plan.ornaments[0].gdat_index =
        dm2_v1_viewport_wall_gfx_map_chip_graphic_index(7);
    plan.ornaments[0].dst_rect = (DM2_V1_ViewportRect){ 100, 60, 16, 16 };
    plan.ornaments[0].material_hash = 0u; /* not validated in this gate */
    dm2_v1_viewport_set_gdat_wall_ornament_material_plan(&viewport, &plan);
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) != 0u ||
        viewport.asset_wall_ornament_drawn_count != 1 ||
        framebuffer[68 * DM2_VP_WIDTH + 108] != 0x32u) {
        fputs("FAIL: wall ornament did not consume source material with plan\n",
              stderr);
        return 1;
    }

    /* Withholding the asset must block even when the plan is present. */
    withhold_material = 1;
    memset(framebuffer, 0, sizeof(framebuffer));
    viewport.blocked_material_mask = 0u;
    viewport.asset_wall_ornament_drawn_count = 0;
    dm2_v1_render_wall_ornaments(&viewport);
    if ((viewport.blocked_material_mask &
         DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT) == 0u ||
        viewport.asset_wall_ornament_drawn_count != 0) {
        fputs("FAIL: missing wall ornament material did not block frame\n",
              stderr);
        return 1;
    }

    puts("PASS: wall ornaments consume only source-owned WALL_GFX material");
    return 0;
}
