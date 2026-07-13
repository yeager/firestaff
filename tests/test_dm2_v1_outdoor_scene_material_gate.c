/* skproject T600 resolves both active GRAPHICSSET scene planes, each with
 * QUERY_GDAT_IMAGE_LOCALPAL, before presenting the outdoor runtime scene. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int omit_ground;
    int fetches;
} SceneFetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_scene_material(void *user, int gdat_index,
                                const uint8_t **out_pixels, int *out_w,
                                int *out_h, int *out_stride)
{
    static const uint8_t sky[4] = { 1, 1, 1, 1 };
    static const uint8_t ground[4] = { 2, 2, 2, 2 };
    SceneFetchTrace *trace = (SceneFetchTrace *)user;
    int graphicsset = -1;
    int field = -1;

    if (!dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 4) {
        return -1;
    }
    ++trace->fetches;
    if (field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
        trace->omit_ground) {
        return -1;
    }
    *out_pixels = field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING
        ? sky : ground;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static int fetch_scene_palette(void *user, int gdat_index,
                               uint8_t out_palette16[16], uint32_t *out_hash)
{
    int graphicsset = -1;
    int field = -1;

    (void)user;
    if (!dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 4) {
        return -1;
    }
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x10 + i);
    out_palette16[field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING ? 1 : 2] =
        field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING ? 0x41u : 0x52u;
    *out_hash = field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING
        ? 0x534b5955u : 0x47524e44u;
    return 0;
}

static void prepare(DM2_V1_ViewportState *viewport, uint8_t *framebuffer,
                    SceneFetchTrace *trace)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_outdoor(viewport, 1);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_scene_material, trace);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_scene_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 4, 0x54363030u, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    SceneFetchTrace trace;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.omit_ground = 1;
    prepare(&viewport, framebuffer, &trace);
    dm2_v1_viewport_render(&viewport);
    CHECK("missing T600 ground material blocks before sky pixels draw",
          trace.fetches == 2 && viewport.asset_outdoor_sky_drawn_count == 0 &&
              viewport.asset_outdoor_ground_drawn_count == 0 &&
              viewport.last_outdoor_scene_material_consumed_mask == 0u &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x5a &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x5a &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    prepare(&viewport, framebuffer, &trace);
    dm2_v1_viewport_render(&viewport);
    CHECK("complete T600 scene consumes both real materials and local palettes",
          viewport.asset_outdoor_sky_drawn_count == 1 &&
              viewport.asset_outdoor_ground_drawn_count == 1 &&
              viewport.last_outdoor_scene_material_required_mask == 3u &&
              viewport.last_outdoor_scene_material_consumed_mask == 3u &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x41u &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x52u &&
              viewport.gdat_local_palette_consumed_count > 0);

    printf("DM2 atomic T600 GDAT scene gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
