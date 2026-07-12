/* Source: skproject SKWIN/SkWinCore.cpp DRAW_DEFAULT_DOOR_BUTTON and
 * GET_WALL_DECORATION_OF_ACTUATOR. A custom button is WALL_GFX field 1 only
 * after a direct DB2 Text or DB3 Actuator material receipt owns its index. */
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int passed;
static int custom_button_fetches;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_asset(void *user,
                       int gdat_index,
                       const uint8_t **out_pixels,
                       int *out_w,
                       int *out_h,
                       int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 2, 3, 4 };
    int expected = dm2_v1_viewport_wall_button_graphic_index(0x2a, 1);
    (void)user;
    if (gdat_index == expected) ++custom_button_fetches;
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static void setup_custom_button(DM2_V1_ViewportState *viewport,
                                uint8_t *framebuffer)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    viewport->squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport->squares[DM2_SQ_D0C].door_wall_button = 1;
    viewport->squares[DM2_SQ_D0C].door_wall_button_index = 0x2a;
    viewport->squares[DM2_SQ_D0C].door_wall_button_field = 1;
    dm2_v1_viewport_set_asset_provider(viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_G1TextWallGfxRuntimeReceipt text_receipt;
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt actuator_receipt;

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("unbound custom WALL_GFX button blocks before asset fetch",
          custom_button_fetches == 0 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(&text_receipt, 0, sizeof(text_receipt));
    text_receipt.valid = 1;
    text_receipt.map = 5;
    text_receipt.material_count = 1;
    text_receipt.materials[0].wall_gfx_index = 0x2a;
    CHECK("text receipt accepts source WALL_GFX field one",
          dm2_v1_g1_text_wall_gfx_allows_button_material(
              &text_receipt, 0x2a, 1) &&
              !dm2_v1_g1_text_wall_gfx_allows_button_material(
                  &text_receipt, 0x2a, 2));

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("other-map text receipt cannot authorize a WALL_GFX button",
          custom_button_fetches == 0 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("verified text WALL_GFX receipt reaches the button asset consumer",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 1);

    memset(&actuator_receipt, 0, sizeof(actuator_receipt));
    actuator_receipt.valid = 1;
    actuator_receipt.material_count = 1;
    actuator_receipt.materials[0].wall_gfx_index = 0x2a;
    CHECK("actuator receipt accepts only its exact source graphic",
          dm2_v1_g1_actuator_wall_gfx_allows_button_material(
              &actuator_receipt, 0x2a, 1) &&
              !dm2_v1_g1_actuator_wall_gfx_allows_button_material(
                  &actuator_receipt, 0x2b, 1));

    printf("DM2 G1 WALL_GFX button gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
