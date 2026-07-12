/* Source: skproject SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE 12866-12880.
 * HeroType selects CHAMPIONS[type] dtImage field 0; an unbound Firestaff
 * session ordinal must not authorize that real-data fetch. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int portrait_fetch(void *user,
                          int gdat_index,
                          const uint8_t **out_pixels,
                          int *out_w,
                          int *out_h,
                          int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 2, 3, 4 };
    int *portrait_fetches = (int *)user;
    if (gdat_index == dm2_v1_viewport_hud_portrait_graphic_index(3)) {
        ++*portrait_fetches;
    }
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

int main(void)
{
    DM2_V1_HudPartyState party;
    DM2_V1_HudChromeRenderPlan plan;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT] = { 0 };
    int portrait_fetches = 0;

    CHECK("legacy portrait ordinal stays bounded",
          dm2_v1_viewport_hud_portrait_graphic_index(3) != 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(-1) == 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(8) == 0);

    memset(&party, 0, sizeof(party));
    party.champion_count = 1;
    party.leader_index = 0;
    party.champions[0].occupied = 1;
    party.champions[0].leader = 1;
    party.champions[0].portrait_index = 3;
    CHECK("HUD plan retains an explicitly unbound portrait type",
          dm2_v1_viewport_build_hud_chrome_plan_for_party(0, &party, &plan) &&
              !plan.champion_slots[0].portrait_type_source_bound);

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_hud_party(&viewport, &party);
    dm2_v1_viewport_set_asset_provider(&viewport, portrait_fetch,
                                       &portrait_fetches);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("unbound portrait type blocks before a CHAMPIONS fetch",
          portrait_fetches == 0 &&
              viewport.asset_hud_portrait_drawn_count == 0 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) != 0u);

    printf("DM2 HUD HeroType GDAT gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
