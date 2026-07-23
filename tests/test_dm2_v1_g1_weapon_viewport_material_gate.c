#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else fprintf(stderr, "FAIL: %s\\n", label); \
} while (0)

static uint8_t g_pixels[16] = {
    0, 0x35, 0, 0,
    0x35, 0x35, 0x35, 0,
    0, 0x35, 0, 0,
    0, 0, 0, 0
};

static uint32_t fnv1a(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int fetch_material(void *user, int gdat_index, const uint8_t **pixels,
                          int *width, int *height, int *stride)
{
    int expected = *(const int *)user;
    if (gdat_index != expected) return -1;
    *pixels = g_pixels;
    *width = 4;
    *height = 4;
    *stride = 4;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    int expected = *(const int *)user;
    if (gdat_index != expected) return -1;
    for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
    *hash = fnv1a(palette, 16);
    return 0;
}

static void setup(DM2_V1_ViewportState *viewport, uint8_t *framebuffer,
                  DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt,
                  int *expected_index)
{
    DM2_V1_G1WeaponMapChipMaterial *material;

    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->map = 7;
    receipt->material_count = 1;
    material = &receipt->materials[0];
    material->object_id = 0x1401u;
    material->x = 11;
    material->y = 13;
    material->direction = 2;
    material->item_type = 0x22;
    material->raw_hash = 1u;
    material->raw_byte_count = 16u;
    material->image_width = 4;
    material->image_height = 4;
    material->image_format = 3;
    {
        uint8_t palette[16];
        for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
        material->local_palette_hash = fnv1a(palette, 16);
    }
    material->decoded_pixel_hash = fnv1a(g_pixels, sizeof(g_pixels));

    *expected_index = dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0xf9);
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    viewport->item_count = 1;
    viewport->items[0].item_category = 0x10;
    viewport->items[0].item_type = 0x22;
    viewport->items[0].screen_x = 96;
    viewport->items[0].screen_y = 88;
    viewport->items[0].object_id = material->object_id;
    viewport->items[0].map_x = material->x;
    viewport->items[0].map_y = material->y;
    viewport->items[0].direction = material->direction;
    viewport->items[0].source_gdat_field = 0xf9;
    viewport->items[0].source_g1_weapon = 1;
    viewport->items[0].source_static_object_admitted = 1;
    viewport->items[0].source_static_object_cell = 3;
    viewport->items[0].source_static_object_pass = 17;
    dm2_v1_viewport_set_g1_weapon_map_chip_materials(viewport, receipt);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_material, expected_index);
    dm2_v1_viewport_set_asset_palette_provider(viewport, fetch_palette,
                                                expected_index);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_G1WeaponMapChipRuntimeReceipt receipt;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int expected_index;

    memset(framebuffer, 0, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    {
        uint8_t palette[16];
        for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
        dm2_v1_viewport_set_g1_scene_item_material_direct(
            &viewport, 1, 0x10, 0x22, expected_index, 0x1401u, 11, 13,
            g_pixels, 4, 4, 4, palette, fnv1a(palette, sizeof(palette)),
            fnv1a(g_pixels, sizeof(g_pixels)));
        /* The direct M11 handoff owns the decoded F9 image. A second
         * provider lookup would allow a changed asset cache to substitute it. */
        dm2_v1_viewport_set_asset_provider(&viewport, NULL, NULL);
        dm2_v1_viewport_set_asset_palette_provider(&viewport, NULL, NULL);
    }
    dm2_v1_render_items(&viewport);
    CHECK("G1 DB5 F9 map-chip receipt cannot impersonate DRAW_ITEM material",
          viewport.asset_item_drawn_count == 0 &&
              viewport.g1_scene_item_material_consumed_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    viewport.items[0].object_id ^= 1u;
    dm2_v1_render_items(&viewport);
    CHECK("altered G1 DB5 object identity blocks its viewport material",
          viewport.asset_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    g_pixels[1] ^= 1u;

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    viewport.items[0].source_static_object_admitted = 0;
    dm2_v1_render_items(&viewport);
    CHECK("unproven G1 DB5 cell blocks instead of using generic projection",
          viewport.asset_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);

    {
        int cell = -1;
        int pass = -1;
        DM2_V1_StaticObjectSourcePlan static_plan;
        CHECK("D1 center maps to exact static-object source pass",
              dm2_v1_viewport_static_object_cell_for_map(
                  10, 8, 0, 10, 10, &cell, &pass) == 1 &&
                  cell == 3 && pass == 17);
        CHECK("D0 center has no generic static-object pass",
              dm2_v1_viewport_static_object_cell_for_map(
                  10, 9, 0, 10, 10, &cell, &pass) == 0 &&
                  cell == -1 && pass == -1);
        CHECK("DRAW_ITEM DB5 D1 north derives F0 rect, scale and slot zero",
              dm2_v1_viewport_static_object_source_plan(
                  3, 17, 0x10, 0, 0, 0, 0, 1u, 1u << 6, &static_plan) == 1 &&
                  static_plan.position_5x5 == 6 &&
                  static_plan.clip_rect_id == (0x8000 | 5081) &&
                  static_plan.y_distance == 1 &&
                  static_plan.stretch_factor64 == 0x40 &&
                  static_plan.image_field == 0 &&
                  static_plan.flip_mirror == 0 &&
                  static_plan.slot_x_offset == 2 &&
                  static_plan.slot_y_offset == -3);
        CHECK("DRAW_ITEM DB9 D2 east open derives F4 and mirror",
              dm2_v1_viewport_static_object_source_plan(
                  6, 14, 0x14, 1, 1, 15, 0, 1u, 1u << 8, &static_plan) == 1 &&
                  static_plan.position_5x5 == 8 &&
                  static_plan.clip_rect_id == (0x8000 | 5158) &&
                  static_plan.y_distance == 2 &&
                  static_plan.stretch_factor64 == 0x2b &&
                  static_plan.image_field == 4 &&
                  static_plan.flip_mirror == 1 &&
                  static_plan.slot_x_offset == -3 &&
                  static_plan.slot_y_offset == 3);
        CHECK("unsupported static cell remains no-draw",
              dm2_v1_viewport_static_object_source_plan(
                  4, 16, 0x10, 0, 0, 0, 0, 1u, 0u, &static_plan) == 0);
    }
    dm2_v1_render_items(&viewport);
    CHECK("changed decoded G1 DB5 pixels block their source material",
          viewport.asset_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);
    g_pixels[1] ^= 1u;

    printf("DM2 G1 DB5 viewport material gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
