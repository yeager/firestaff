/* Source: skproject SKWIN/SkWinCore.cpp T600 GRAPHICSSET scene material
 * lookup and QUERY_GDAT_IMAGE_LOCALPAL. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int asset_fetches;
    int palette_fetches;
    int sky_index;
    int ground_index;
    int palette_seen[2];
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_outdoor_scene(void *user,
                               int gdat_index,
                               const uint8_t **out_pixels,
                               int *out_w,
                               int *out_h,
                               int *out_stride)
{
    static const uint8_t sky[1] = { 1 };
    static const uint8_t ground[1] = { 2 };
    FetchTrace *trace = (FetchTrace *)user;

    if (gdat_index == trace->sky_index) {
        *out_pixels = sky;
    } else if (gdat_index == trace->ground_index) {
        *out_pixels = ground;
    } else {
        return -1;
    }
    ++trace->asset_fetches;
    *out_w = 1;
    *out_h = 1;
    *out_stride = 1;
    return 0;
}

static int fetch_outdoor_local_palette(void *user,
                                       int gdat_index,
                                       uint8_t out_palette16[16],
                                       uint32_t *out_hash)
{
    FetchTrace *trace = (FetchTrace *)user;

    ++trace->palette_fetches;
    memset(out_palette16, 0, 16);
    if (gdat_index == trace->sky_index) {
        out_palette16[1] = 0x21u;
        trace->palette_seen[0] = 1;
    } else if (gdat_index == trace->ground_index) {
        out_palette16[2] = 0x42u;
        trace->palette_seen[1] = 1;
    } else {
        return -1;
    }
    if (out_hash) *out_hash = (uint32_t)gdat_index ^ 0x4f555444u;
    return 0;
}

static void setup_outdoor(DM2_V1_ViewportState *viewport,
                          uint8_t *framebuffer,
                          FetchTrace *trace,
                          int bind_palette)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_outdoor(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 0, 0, 0x53434e45u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_outdoor_scene, trace);
    if (bind_palette) {
        dm2_v1_viewport_set_asset_palette_provider(
            viewport, fetch_outdoor_local_palette, trace);
    }
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;

    memset(&trace, 0, sizeof(trace));
    trace.sky_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    trace.ground_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_outdoor(&viewport, framebuffer, &trace, 1);
    dm2_v1_viewport_render(&viewport);
    CHECK("outdoor scene consumes each source IMG3 local palette",
          trace.asset_fetches == 2 && trace.palette_fetches == 2 &&
              trace.palette_seen[0] && trace.palette_seen[1] &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x21u &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x42u &&
              viewport.asset_outdoor_sky_drawn_count == 1 &&
              viewport.asset_outdoor_ground_drawn_count == 1 &&
              viewport.gdat_local_palette_consumed_count > 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.sky_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    trace.ground_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    setup_outdoor(&viewport, framebuffer, &trace, 0);
    dm2_v1_viewport_render(&viewport);
    CHECK("source-required outdoor scene fails closed without local palettes",
          trace.asset_fetches == 2 && trace.palette_fetches == 0 &&
              viewport.asset_outdoor_sky_drawn_count == 0 &&
              viewport.asset_outdoor_ground_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x7eu &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x7eu);

    printf("DM2 outdoor scene local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
