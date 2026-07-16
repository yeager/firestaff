/* Source: skproject SKWIN/SkWinCore.cpp DRAW_WALL 47466-47474.
 * Every visible wall queries GRAPHICSSET with the live MapGraphicsStyle;
 * source-required rendering must not replace it with Firestaff's set 1. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int expected_graphicsset;
    int wall_fetches;
    int wrong_graphicsset_fetches;
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_wall(void *user,
                      int gdat_index,
                      const uint8_t **out_pixels,
                      int *out_w,
                      int *out_h,
                      int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 2, 3, 4 };
    FetchTrace *trace = (FetchTrace *)user;
    int graphicsset = -1;
    int field = -1;

    if (!dm2_v1_viewport_wall_graphic_address(
            gdat_index, &graphicsset, &field)) {
        return -1;
    }
    ++trace->wall_fetches;
    if (graphicsset != trace->expected_graphicsset || field < 0x22) {
        ++trace->wrong_graphicsset_fetches;
    }
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

/* skproject binds GRAPHICSSET, dtPalette16, light, colorkey and the selected
 * wall fields before DM2_DRAW_WALL.  Keep the data-free test on that same
 * transaction boundary: a scene hash alone must not authorize pixels. */
static int bind_source_wall_transaction(DM2_V1_ViewportState *viewport)
{
    static const uint8_t palette16[16] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    };
    const uint32_t map_token = 0x4d415000u;
    const uint32_t scene_hash = 0x6d324741u;

    dm2_v1_viewport_set_scene_map_load_token(viewport, map_token);
    dm2_v1_viewport_set_gdat_interface_palette(
        viewport, 1, 0x50414c31u, palette16);
    return dm2_v1_viewport_bind_static_graphicsset_scene_record(
               viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_light_control(
            viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_ambient_light_control(
            viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
            viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_flags_control(
            viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_colorkey_control(
            viewport, map_token, scene_hash) &&
        dm2_v1_viewport_bind_static_scene_all_wall_materials(
            viewport, map_token, scene_hash);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_WallPanelRenderPlan plan;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    CHECK("source wall plan rejects missing live G1 graphics-set receipt",
          dm2_v1_viewport_build_wall_panel_render_plan(&viewport, &plan) == 0 &&
              plan.panel_count == 0);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.expected_graphicsset = 0x2a;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_render_walls(&viewport);
    CHECK("source wall draw blocks before callback lookup without a G1 plan",
          trace.fetches == 0 && viewport.asset_wall_drawn_count == 0 &&
              viewport.last_dungeon_wall_material_consumed_mask == 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.expected_graphicsset = 0x2a;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 0x2a, 0x6d324741u,
        10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CHECK("complete source wall transaction binds",
          bind_source_wall_transaction(&viewport) == 1);
    dm2_v1_render_walls(&viewport);
    CHECK("callback-complete GDAT cannot replace a source-owned wall plan",
          trace.fetches == 0 && viewport.asset_wall_drawn_count == 0 &&
              viewport.fallback_wall_drawn_count == 0 &&
              viewport.last_dungeon_wall_material_required_mask == 0u &&
              viewport.last_dungeon_wall_material_consumed_mask == 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u);

    printf("DM2 GRAPHICSSET wall material gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
