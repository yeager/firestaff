/* skproject DM2_DRAW_DOOR requires every selected door IMG3 and its local
 * palette before the first panel/frame pixel reaches the dungeon viewport. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int missing_gdat_index;
    int fetches;
} DoorFetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_door_material(void *user, int gdat_index,
                               const uint8_t **out_pixels, int *out_w,
                               int *out_h, int *out_stride)
{
    static const uint8_t pixels[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 1
    };
    DoorFetchTrace *trace = (DoorFetchTrace *)user;

    ++trace->fetches;
    if (gdat_index == trace->missing_gdat_index) return -1;
    *out_pixels = pixels;
    *out_w = 8;
    *out_h = 2;
    *out_stride = 8;
    return 0;
}

static int fetch_door_palette(void *user, int gdat_index,
                              uint8_t out_palette16[16], uint32_t *out_hash)
{
    (void)user;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x20 + i);
    *out_hash = 0x444f4f52u ^ (uint32_t)gdat_index;
    return 0;
}

static void prepare(DM2_V1_ViewportState *viewport, uint8_t *framebuffer,
                    DoorFetchTrace *trace)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    viewport->squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    dm2_v1_viewport_set_asset_provider(viewport, fetch_door_material, trace);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_door_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_DoorRenderPlan plan;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DoorFetchTrace trace;

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    prepare(&viewport, framebuffer, &trace);
    CHECK("front door plan supplies panel and frame source material",
          dm2_v1_viewport_build_door_render_plan(&viewport, &plan) == 1 &&
              plan.door_count == 1 && plan.doors[0].panel_gdat_index != 0 &&
              plan.doors[0].frame_gdat_index != 0);
    trace.missing_gdat_index = plan.doors[0].frame_gdat_index;
    dm2_v1_render_doors(&viewport);
    CHECK("missing source frame blocks before panel pixels are painted",
          trace.fetches >= 2 && viewport.asset_door_panel_drawn_count == 0 &&
              viewport.asset_door_frame_drawn_count == 0 &&
              viewport.fallback_door_drawn_count == 0 &&
              viewport.last_door_material_consumed_mask == 0u &&
              framebuffer[70 * DM2_VP_WIDTH + 112] == 0x5a &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    prepare(&viewport, framebuffer, &trace);
    dm2_v1_render_doors(&viewport);
    CHECK("complete source door consumes panel and frame local palettes",
          viewport.asset_door_panel_drawn_count == 1 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.last_door_material_required_mask ==
                  ((1u << 0) | (1u << 3)) &&
              viewport.last_door_material_consumed_mask ==
                  viewport.last_door_material_required_mask &&
              viewport.gdat_local_palette_consumed_count > 0 &&
              viewport.fallback_door_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) == 0u);

    printf("DM2 complete GDAT door material gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
