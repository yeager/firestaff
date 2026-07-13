/* Source: skproject SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE,
 * QUERY_GDAT_IMAGE_LOCALPAL. */
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

static int fetch_portrait(void *user,
                          int gdat_index,
                          const uint8_t **out_pixels,
                          int *out_w,
                          int *out_h,
                          int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 0, 2, 3 };
    FetchTrace *trace = (FetchTrace *)user;
    int expected = dm2_v1_viewport_hud_portrait_graphic_index(3);

    if (gdat_index != expected) return -1;
    ++trace->asset_fetches;
    trace->last_asset_index = gdat_index;
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static int fetch_portrait_local_palette(void *user,
                                        int gdat_index,
                                        uint8_t out_palette16[16],
                                        uint32_t *out_hash)
{
    FetchTrace *trace = (FetchTrace *)user;

    ++trace->palette_fetches;
    trace->last_palette_index = gdat_index;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x30 + i);
    if (out_hash) *out_hash = 0x504f5254u;
    return 0;
}

static int framebuffer_contains(const uint8_t *framebuffer, uint8_t value)
{
    for (size_t i = 0; i < (size_t)DM2_VP_WIDTH * DM2_VP_HEIGHT; ++i) {
        if (framebuffer[i] == value) return 1;
    }
    return 0;
}

static void setup_hud(DM2_V1_ViewportState *viewport,
                      uint8_t *framebuffer,
                      FetchTrace *trace,
                      int bind_palette)
{
    DM2_V1_HudPartyState party;

    memset(&party, 0, sizeof(party));
    party.champion_count = 1;
    party.leader_index = 0;
    party.champions[0].occupied = 1;
    party.champions[0].leader = 1;
    party.champions[0].portrait_index = 3;
    party.champions[0].portrait_type_source_bound = 1;

    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_hud_party(viewport, &party);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_portrait, trace);
    if (bind_palette) {
        dm2_v1_viewport_set_asset_palette_provider(
            viewport, fetch_portrait_local_palette, trace);
    }
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;
    int expected = dm2_v1_viewport_hud_portrait_graphic_index(3);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_hud(&viewport, framebuffer, &trace, 1);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("HUD portrait consumes its matching source IMG3 palette",
          trace.asset_fetches == 1 && trace.palette_fetches == 1 &&
              trace.last_asset_index == expected &&
              trace.last_palette_index == expected &&
              framebuffer_contains(framebuffer, 0x31u) &&
              viewport.asset_hud_portrait_drawn_count == 1 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              viewport.gdat_local_palette_consumed_count > 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) == 0u);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_hud(&viewport, framebuffer, &trace, 0);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("source-required HUD portrait fails closed without its palette",
          trace.asset_fetches == 1 && trace.palette_fetches == 0 &&
              viewport.asset_hud_portrait_drawn_count == 0 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) != 0u &&
              !framebuffer_contains(framebuffer, 0x31u));

    printf("DM2 HUD portrait local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
