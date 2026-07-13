/* skproject/SKULLWIN/c_gui_vp.cpp resolves GRAPHICSSET ceiling and floor
 * with their local palettes before DRAW_DUNGEON presents either plane. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int omit_floor;
    int fetches;
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_scene_plane(void *user, int gdat_index,
                             const uint8_t **out_pixels, int *out_w,
                             int *out_h, int *out_stride)
{
    static const uint8_t ceiling[4] = { 1, 2, 3, 4 };
    static const uint8_t floor[4] = { 5, 6, 7, 8 };
    FetchTrace *trace = (FetchTrace *)user;
    int graphicsset = -1;
    int field = -1;

    if (!dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 3) {
        return -1;
    }
    ++trace->fetches;
    if (field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
        trace->omit_floor) {
        return -1;
    }
    *out_pixels = field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING
        ? ceiling : floor;
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
    if (!dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 3) {
        return -1;
    }
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)i;
    *out_hash = field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING
        ? 0x4345494cu : 0x464c4f52u;
    return 0;
}

static void prepare(DM2_V1_ViewportState *viewport,
                    uint8_t *framebuffer, FetchTrace *trace)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_scene_plane, trace);
    dm2_v1_viewport_set_asset_palette_provider(viewport, fetch_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 3, 0x6d324741u,
        10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.omit_floor = 1;
    prepare(&viewport, framebuffer, &trace);
    dm2_v1_render_floor_ceiling(&viewport);
    CHECK("missing source floor blocks before ceiling or floor pixels draw",
          trace.fetches == 2 && viewport.asset_floor_ceiling_drawn_count == 0 &&
              viewport.last_floor_ceiling_material_consumed_mask == 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    prepare(&viewport, framebuffer, &trace);
    dm2_v1_render_floor_ceiling(&viewport);
    CHECK("complete source planes consume both real GDAT materials",
          trace.fetches == 2 && viewport.asset_floor_ceiling_drawn_count == 2 &&
              viewport.fallback_floor_ceiling_drawn_count == 0 &&
              viewport.last_floor_ceiling_material_required_mask == 3u &&
              viewport.last_floor_ceiling_material_consumed_mask == 3u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u);

    printf("DM2 complete GDAT floor/ceiling gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
