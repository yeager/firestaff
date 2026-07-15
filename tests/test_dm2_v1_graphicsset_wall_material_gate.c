/* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_WALL selects a GRAPHICSSET
 * material for every visible viewport cell before drawing. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int missing_after;
    int fetches;
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_wall(void *user, int gdat_index,
                      const uint8_t **out_pixels, int *out_w,
                      int *out_h, int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 2, 3, 4 };
    FetchTrace *trace = (FetchTrace *)user;
    int graphicsset = -1;
    int field = -1;

    if (!dm2_v1_viewport_wall_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 0x2a ||
        field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) {
        return -1;
    }
    ++trace->fetches;
    if (trace->missing_after > 0 && trace->fetches >= trace->missing_after) {
        return -1;
    }
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static int fetch_palette(void *user, int gdat_index,
                         uint8_t out_palette16[16], uint32_t *out_hash)
{
    int graphicsset = -1;
    int field = -1;

    (void)user;
    if (!dm2_v1_viewport_wall_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 0x2a) {
        return -1;
    }
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)i;
    *out_hash = 0x50414c31u;
    return 0;
}

static void prepare_viewport(DM2_V1_ViewportState *viewport,
                             uint8_t *framebuffer, FetchTrace *trace)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_wall, trace);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 0x2a, 0x6d324741u,
        10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
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
    trace.missing_after = 2;
    prepare_viewport(&viewport, framebuffer, &trace);
    dm2_v1_render_walls(&viewport);
    CHECK("later missing GDAT panel blocks before any wall pixels draw",
          trace.fetches >= 2 && viewport.asset_wall_drawn_count == 0 &&
              viewport.last_dungeon_wall_material_consumed_mask == 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    prepare_viewport(&viewport, framebuffer, &trace);
    dm2_v1_render_walls(&viewport);
    CHECK("complete GDAT wall set consumes every planned source panel",
          trace.fetches > 0 && viewport.asset_wall_drawn_count > 0 &&
              viewport.fallback_wall_drawn_count == 0 &&
              viewport.last_dungeon_wall_material_required_mask != 0u &&
              viewport.last_dungeon_wall_material_required_mask ==
                  viewport.last_dungeon_wall_material_consumed_mask &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u);

    printf("DM2 complete GDAT wall material gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
