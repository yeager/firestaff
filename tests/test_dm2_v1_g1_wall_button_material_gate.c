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
static int wrong_sized_custom_button;
static const uint8_t source_bytes[4] = { 0x44, 0x4d, 0x32, 0x00 };

static uint32_t indexed_pixel_hash(const uint8_t *pixels, int width,
                                   int height, int stride)
{
    uint32_t hash = 2166136261u;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash ? hash : 1u;
}

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
    *out_w = wrong_sized_custom_button ? 1 : 2;
    *out_h = wrong_sized_custom_button ? 4 : 2;
    *out_stride = wrong_sized_custom_button ? 1 : 2;
    return 0;
}

static int fetch_local_palette(void *user,
                               int gdat_index,
                               uint8_t out_palette16[16],
                               uint32_t *out_hash)
{
    (void)user;
    (void)gdat_index;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x90 + i);
    if (out_hash) *out_hash = 0x47443150u;
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
    viewport->squares[DM2_SQ_D0C].door_wall_button_x = 6;
    viewport->squares[DM2_SQ_D0C].door_wall_button_y = 7;
    viewport->squares[DM2_SQ_D0C].door_wall_button_object_id = 0x8abcu;
    dm2_v1_viewport_set_asset_provider(viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_local_palette, NULL);
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
    CHECK("unbound custom WALL_GFX button blocks after exact image lookup",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(&text_receipt, 0, sizeof(text_receipt));
    text_receipt.valid = 1;
    text_receipt.map = 5;
    text_receipt.material_count = 1;
    text_receipt.materials[0].x = 6;
    text_receipt.materials[0].y = 7;
    text_receipt.materials[0].object_id = 0x8abcu;
    text_receipt.materials[0].wall_gfx_index = 0x2a;
    text_receipt.materials[0].front_image_ready = 1;
    text_receipt.materials[0].front_image_width = 2;
    text_receipt.materials[0].front_image_height = 2;
    text_receipt.materials[0].local_palette_hash = 0x47443150u;
    text_receipt.materials[0].raw_material_index = 0x123u;
    text_receipt.materials[0].raw_material_bytes = source_bytes;
    text_receipt.materials[0].raw_material_byte_count = sizeof(source_bytes);
    text_receipt.materials[0].raw_material_hash =
        indexed_pixel_hash(source_bytes, sizeof(source_bytes), 1,
                           sizeof(source_bytes));
    text_receipt.materials[0].raw_material_receipt_hash = 0x52454331u;

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    wrong_sized_custom_button = 1;
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("same-palette WALL_GFX with receipt-mismatched dimensions blocks",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);
    wrong_sized_custom_button = 0;
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
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    text_receipt.materials[0].object_id = 0x8abdu;
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("same-index receipt from another record cannot authorize a button",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    text_receipt.materials[0].object_id = 0x8abcu;
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("verified text WALL_GFX receipt reaches the button asset consumer",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 1 &&
              viewport.source_click_target_count == 1u &&
              viewport.source_click_targets[0].object_id == -1 &&
              viewport.source_click_targets[0].view_slot == DM2_SQ_D0C &&
              viewport.source_click_targets[0].target_kind == 4u &&
              viewport.source_click_targets[0].w > 0 &&
              viewport.source_click_targets[0].h > 0);

    {
        static const uint8_t direct_pixels[4] = { 1, 2, 3, 4 };
        uint8_t palette16[16];
        for (int i = 0; i < 16; ++i) palette16[i] = (uint8_t)(0x90 + i);
        dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
            &viewport, 1,
            dm2_v1_viewport_wall_button_graphic_index(0x2a, 1),
            0x2a, 1, 6, 7, 0x8abcu, direct_pixels, 2, 2, 2,
            palette16, 0x47443150u,
            indexed_pixel_hash(direct_pixels, 2, 2, 2), 0x123u,
            source_bytes, sizeof(source_bytes),
            indexed_pixel_hash(source_bytes, sizeof(source_bytes), 1,
                               sizeof(source_bytes)), 0x52454331u);
        CHECK("direct G1 WALL_GFX receipt retains exact M11 source bytes",
              viewport.g1_scene_wall_button_material_ready &&
                  viewport.g1_scene_wall_button_material_pixels == direct_pixels &&
                  viewport.g1_scene_wall_button_material_pixel_hash ==
                      indexed_pixel_hash(direct_pixels, 2, 2, 2));
        memset(framebuffer, 0, sizeof(framebuffer));
        setup_custom_button(&viewport, framebuffer);
        dm2_v1_viewport_set_level(&viewport, 5);
        dm2_v1_viewport_set_source_materials_required(&viewport, 0);
        dm2_v1_viewport_set_g1_wall_gfx_materials(
            &viewport, &text_receipt, NULL);
        dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
            &viewport, 1,
            dm2_v1_viewport_wall_button_graphic_index(0x2a, 1),
            0x2a, 1, 6, 7, 0x8abcu, direct_pixels, 2, 2, 2,
            palette16, 0x47443150u,
            indexed_pixel_hash(direct_pixels, 2, 2, 2), 0x123u,
            source_bytes, sizeof(source_bytes),
            indexed_pixel_hash(source_bytes, sizeof(source_bytes), 1,
                               sizeof(source_bytes)), 0x52454331u);
        custom_button_fetches = 0;
        dm2_v1_render_doors(&viewport);
        CHECK("direct G1 WALL_GFX bytes avoid a second provider lookup",
              custom_button_fetches == 0 &&
                  viewport.asset_door_button_drawn_count == 1 &&
                  viewport.g1_scene_wall_button_material_consumed_count == 1);
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    text_receipt.materials[0].local_palette_hash = 0x12345678u;
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("mismatched WALL_GFX local palette blocks the custom button",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);
    text_receipt.materials[0].local_palette_hash = 0x47443150u;

    text_receipt.materials[0].raw_material_receipt_hash = 0u;
    memset(framebuffer, 0, sizeof(framebuffer));
    setup_custom_button(&viewport, framebuffer);
    dm2_v1_viewport_set_level(&viewport, 5);
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport, &text_receipt, NULL);
    custom_button_fetches = 0;
    dm2_v1_render_doors(&viewport);
    CHECK("missing raw WALL_GFX receipt preserves no-draw",
          custom_button_fetches == 1 &&
              viewport.asset_door_button_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u);
    text_receipt.materials[0].raw_material_receipt_hash = 0x52454331u;

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
