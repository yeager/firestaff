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

static int fetch_wall_local_palette(void *user,
                                    int gdat_index,
                                    uint8_t out_palette16[16],
                                    uint32_t *out_hash)
{
    (void)user;
    (void)gdat_index;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0xa0 + i);
    if (out_hash) *out_hash = 0x324c5041u;
    return 0;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.expected_graphicsset = 0x2a;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, fetch_wall_local_palette, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_render_walls(&viewport);
    CHECK("missing MapGraphicsStyle receipt blocks before wall fetch",
          trace.wall_fetches == 0 &&
              viewport.asset_wall_drawn_count == 0 &&
              viewport.fallback_wall_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.expected_graphicsset = 0x2a;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, fetch_wall_local_palette, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 0x2a, 0x6d324741u,
        10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    dm2_v1_render_walls(&viewport);
    CHECK("bound MapGraphicsStyle reaches every wall GDAT query",
          trace.wall_fetches > 0 &&
              trace.wrong_graphicsset_fetches == 0 &&
              viewport.asset_wall_drawn_count == trace.wall_fetches &&
              viewport.fallback_wall_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) == 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.expected_graphicsset = 0x2a;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_wall, &trace);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, fetch_wall_local_palette, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 0x2a, 0x6d324741u,
        10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    dm2_v1_render_walls(&viewport);
    {
        int local_pixel_seen = 0;
        for (size_t i = 0; i < sizeof(framebuffer); ++i) {
            if (framebuffer[i] == 0xa1u) {
                local_pixel_seen = 1;
                break;
            }
        }
        CHECK("wall pixels consume the source IMG3 local palette",
              trace.wall_fetches > 0 && local_pixel_seen &&
                  viewport.gdat_local_palette_consumed_count > 0 &&
                  viewport.blocked_material_draw_count == 0);
    }

    printf("DM2 GRAPHICSSET wall material gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
