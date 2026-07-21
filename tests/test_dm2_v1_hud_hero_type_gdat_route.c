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

    CHECK("source HeroType keeps the full 8-bit record range",
          dm2_v1_viewport_hud_portrait_graphic_index(3) != 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(-1) == 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(255) != 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(256) == 0);

    memset(&party, 0, sizeof(party));
    party.champion_count = 1;
    party.leader_index = 0;
    party.champions[0].occupied = 1;
    party.champions[0].leader = 1;
    party.champions[0].portrait_index = 3;
    CHECK("HUD plan retains an explicitly unbound portrait type",
          dm2_v1_viewport_build_hud_chrome_plan_for_party(0, &party, &plan) &&
              !plan.champion_slots[0].portrait_type_source_bound &&
              plan.champion_slots[0].stat_bar_color_source_bound &&
              plan.champion_slots[0].stat_bar_color == 7u);

    party.champion_count = 4;
    for (int slot = 0; slot < party.champion_count; ++slot) {
        party.champions[slot].occupied = 1;
    }
    CHECK("HUD plan consumes SKProject's per-player default bar colors",
          dm2_v1_viewport_build_hud_chrome_plan_for_party(0, &party, &plan) &&
              plan.champion_slots[0].stat_bar_color == 7u &&
              plan.champion_slots[1].stat_bar_color == 11u &&
              plan.champion_slots[2].stat_bar_color == 8u &&
              plan.champion_slots[3].stat_bar_color == 14u);

    party.champion_count = 1;
    memset(&party.champions[1], 0,
           sizeof(party.champions) - sizeof(party.champions[0]));

    /* d6adbe2e2 ("gate dynamic HUD on GDAT ownership") blocks the whole
     * champion slot at the dynamic-overlay gate unless the source
     * dt04/dt07/palette contract is complete.  Bind that contract here so
     * this focused gate test isolates the HeroType portrait gate below:
     * the slot then reaches DRAW_CHAMPION_PICTURE with only the portrait
     * type unbound. */
    {
        static const uint8_t font_rows[6 * 128] = { 0 };
        uint8_t palette16[16];
        DM2_V1_InterfaceHudLayout hud_layout;
        for (int i = 0; i < 16; ++i) palette16[i] = (uint8_t)i;
        memset(&hud_layout, 0, sizeof(hud_layout));
        hud_layout.valid = 1;
        hud_layout.table_hash = 0x64743034u;
        party.champions[0].state_source_bound = 1;

        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_hud_party(&viewport, &party);
        dm2_v1_viewport_set_asset_provider(&viewport, portrait_fetch,
                                           &portrait_fetches);
        dm2_v1_viewport_set_gdat_interface_hud_layout(&viewport,
                                                      &hud_layout);
        dm2_v1_viewport_set_gdat_interface_palette(&viewport, 1,
                                                   0x70616c31u, palette16);
        dm2_v1_viewport_set_gdat_interface_font(&viewport, font_rows,
                                                0x64743037u);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_render_ui_chrome(&viewport);
    }
    CHECK("unbound portrait type blocks before a CHAMPIONS fetch",
          portrait_fetches == 0 &&
              viewport.asset_hud_portrait_drawn_count == 0 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) != 0u);

    printf("DM2 HUD HeroType GDAT gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
