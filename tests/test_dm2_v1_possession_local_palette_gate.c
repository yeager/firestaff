/* Source: skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP,
 * QUERY_DUNGEON_MAP_CHIP_PICT, QUERY_GDAT_IMAGE_LOCALPAL. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int asset_fetches;
    int palette_fetches;
    int last_asset_index;
    int last_palette_index;
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_possession_map_chip(void *user,
                                     int gdat_index,
                                     const uint8_t **out_pixels,
                                     int *out_w,
                                     int *out_h,
                                     int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 0, 2, 3 };
    FetchTrace *trace = (FetchTrace *)user;

    ++trace->asset_fetches;
    trace->last_asset_index = gdat_index;
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static int fetch_possession_local_palette(void *user,
                                          int gdat_index,
                                          uint8_t out_palette16[16],
                                          uint32_t *out_hash)
{
    FetchTrace *trace = (FetchTrace *)user;

    ++trace->palette_fetches;
    trace->last_palette_index = gdat_index;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x40 + i);
    if (out_hash) *out_hash = 0x504f5353u;
    return 0;
}

static int framebuffer_contains(const uint8_t *framebuffer, uint8_t value)
{
    for (size_t i = 0; i < (size_t)DM2_VP_WIDTH * DM2_VP_HEIGHT; ++i) {
        if (framebuffer[i] == value) return 1;
    }
    return 0;
}

static void setup_possession(DM2_V1_ViewportState *viewport,
                             uint8_t *framebuffer,
                             FetchTrace *trace,
                             int bind_palette)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    viewport->creature_possession_item_count = 1;
    viewport->creature_possession_items[0].item_category = 0x10;
    viewport->creature_possession_items[0].item_type = 0x22;
    viewport->creature_possession_items[0].frame_index = 0;
    viewport->creature_possession_items[0].screen_x = 96;
    viewport->creature_possession_items[0].screen_y = 88;
    dm2_v1_viewport_set_asset_provider(
        viewport, fetch_possession_map_chip, trace);
    if (bind_palette) {
        dm2_v1_viewport_set_asset_palette_provider(
            viewport, fetch_possession_local_palette, trace);
    }
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;
    int expected_index = dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_possession(&viewport, framebuffer, &trace, 1);
    dm2_v1_render_creature_possession_items(&viewport);
    CHECK("possession overlay consumes its matching source IMG3 palette",
          trace.asset_fetches == 1 && trace.palette_fetches == 1 &&
              trace.last_asset_index == expected_index &&
              trace.last_palette_index == expected_index &&
              framebuffer_contains(framebuffer, 0x41u) &&
              viewport.asset_creature_possession_item_drawn_count == 1 &&
              viewport.fallback_creature_possession_item_drawn_count == 0 &&
              viewport.gdat_local_palette_consumed_count > 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_POSSESSION) == 0u);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_possession(&viewport, framebuffer, &trace, 0);
    dm2_v1_render_creature_possession_items(&viewport);
    CHECK("source-required possession overlay fails closed without its palette",
          trace.asset_fetches == 1 && trace.palette_fetches == 0 &&
              viewport.asset_creature_possession_item_drawn_count == 0 &&
              viewport.fallback_creature_possession_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_POSSESSION) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);

    printf("DM2 possession local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
