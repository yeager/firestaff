/* Source: skproject SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE,
 * QUERY_GDAT_IMAGE_LOCALPAL.
 *
 * Provenance: introduced against the callback-provider route.  Re-anchored
 * 2026-07-21 after 5c21e5561 ("Fix DM2 scene local palette ownership"):
 * under source_materials_required the champion portrait is owned by the
 * boot-owned HUD M11 command plan, whose CHAMPION_PORTRAIT command carries
 * decoded pixels plus the local palette of its own CHAMPIONS/HeroType/0
 * lookup; the provider callback is no longer consulted.  The fixture below
 * builds a minimal synthetic plan with the same receipt discipline the
 * real-data builder proves against canonical GRAPHICS.DAT
 * (test_dm2_v1_gdat_hud_m11_command_real_data.c). */
#include "dm2_v1_gdat_hud_m11_command.h"
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
} while(0)

static uint32_t palette_fnv(const uint8_t palette[16])
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < 16; ++i) {
        hash ^= palette[i];
        hash *= 16777619u;
    }
    return hash;
}

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

static uint8_t portrait_pixels[4] = { 1, 0, 2, 3 };
static const uint8_t portrait_raw_bytes[4] = { 0x12u, 0x34u, 0x56u, 0x78u };
static uint8_t interface_font_rows[6 * 128];
static uint8_t interface_palette16[16];
static DM2_V1_InterfaceHudLayout interface_hud_layout;

/* The HUD M11 plan is only admitted complete: command_count must cover the
 * indoor HUD family minimum (DM2_V1_GDAT_HUD_M11_COMMAND_MAX minus the four
 * champion slots).  Only the first command carries material; the rest are
 * inert filler that no render query can match. */
#define FIXTURE_HUD_COMMAND_COUNT 9

static void build_hud_plan(DM2_V1_GdatHudM11CommandPlan *plan,
                           int with_palette)
{
    DM2_V1_GdatHudM11Command *command;

    memset(plan, 0, sizeof(*plan));
    plan->valid = 1;
    plan->command_count = FIXTURE_HUD_COMMAND_COUNT;
    command = &plan->commands[0];
    command->kind = DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT;
    command->gdat_category = DM2_GDAT_CATEGORY_CHAMPIONS;
    command->viewport_gdat_index =
        dm2_v1_viewport_hud_portrait_graphic_index(3);
    command->destination.x = 100;
    command->destination.y = 40;
    command->destination.w = 2;
    command->destination.h = 2;
    command->pixels = portrait_pixels;
    command->width = 2;
    command->height = 2;
    if (with_palette) {
        for (int i = 0; i < 16; ++i)
            command->palette16[i] = (uint8_t)(0x30u + i);
        command->palette_hash = palette_fnv(command->palette16);
    }
    command->raw_byte_count = sizeof(portrait_raw_bytes);
    command->material_source_bytes = portrait_raw_bytes;
    command->material_source_byte_count = sizeof(portrait_raw_bytes);
    command->material_receipt_hash = 0x52454352u;
    command->decoded_hash = dm2_v1_gdat_hud_m11_command_pixel_hash(command);
    plan->command_hash = dm2_v1_gdat_hud_m11_command_plan_hash(plan);
}

static void setup_hud(DM2_V1_ViewportState *viewport,
                      uint8_t *framebuffer,
                      FetchTrace *trace,
                      const DM2_V1_GdatHudM11CommandPlan *plan)
{
    DM2_V1_HudPartyState party;

    memset(&party, 0, sizeof(party));
    party.champion_count = 1;
    party.leader_index = 0;
    party.champions[0].occupied = 1;
    party.champions[0].leader = 1;
    party.champions[0].portrait_index = 3;
    party.champions[0].portrait_type_source_bound = 1;
    party.champions[0].state_source_bound = 1;
    party.champions[0].stat_bar_color_source_bound = 1;
    party.champions[0].stat_bar_color = 0;

    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_hud_party(viewport, &party);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_portrait, trace);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_portrait_local_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    /* skproject draws the live champion overlay only once the original
     * INTERFACE_GENERAL geometry, palette and dt07 font rows are owned. */
    dm2_v1_viewport_set_gdat_interface_hud_layout(viewport,
                                                  &interface_hud_layout);
    dm2_v1_viewport_set_gdat_interface_palette(
        viewport, 1, 0x1a7ef00du, interface_palette16);
    dm2_v1_viewport_set_gdat_interface_font(viewport, interface_font_rows,
                                            0xf07d0001u);
    if (plan) {
        dm2_v1_viewport_set_gdat_hud_material_plan(viewport, plan);
    }
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;
    DM2_V1_GdatHudM11CommandPlan hud_plan;

    memset(interface_font_rows, 0, sizeof(interface_font_rows));
    memset(interface_palette16, 0, sizeof(interface_palette16));
    memset(&interface_hud_layout, 0, sizeof(interface_hud_layout));
    interface_hud_layout.valid = 1;

    build_hud_plan(&hud_plan, 1);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_hud(&viewport, framebuffer, &trace, &hud_plan);
    dm2_v1_render_ui_chrome(&viewport);
    printf("  DEBUG: fetches=%d pal_fetches=%d has_0x31=%d portrait_drawn=%d plan_consumed=%d fallback=%d blocked=0x%x\n",
           trace.asset_fetches, trace.palette_fetches,
           framebuffer_contains(framebuffer, 0x31u),
           viewport.asset_hud_portrait_drawn_count,
           viewport.gdat_hud_material_plan_consumed_count,
           viewport.fallback_hud_portrait_drawn_count,
           viewport.blocked_material_mask);
    CHECK("HUD portrait consumes its matching source IMG3 palette",
          trace.asset_fetches == 0 && trace.palette_fetches == 0 &&
              framebuffer_contains(framebuffer, 0x31u) &&
              viewport.asset_hud_portrait_drawn_count == 1 &&
              viewport.gdat_hud_material_plan_consumed_count == 1 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) == 0u);

    /* Fail-closed re-anchor: a CHAMPION_PORTRAIT command without its local
     * palette receipt (palette_hash 0) is rejected by the plan command gate,
     * so the portrait blocks before any pixel or provider fetch. */
    build_hud_plan(&hud_plan, 0);
    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_hud(&viewport, framebuffer, &trace, &hud_plan);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("source-required HUD portrait fails closed without its palette",
          trace.asset_fetches == 0 && trace.palette_fetches == 0 &&
              viewport.asset_hud_portrait_drawn_count == 0 &&
              viewport.gdat_hud_material_plan_consumed_count == 0 &&
              viewport.fallback_hud_portrait_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT) != 0u &&
              !framebuffer_contains(framebuffer, 0x31u));

    printf("DM2 HUD portrait local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
