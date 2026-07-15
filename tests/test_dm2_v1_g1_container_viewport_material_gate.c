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
    if (gdat_index != *(const int *)user) return -1;
    *pixels = g_pixels;
    *width = 4;
    *height = 4;
    *stride = 4;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    if (gdat_index != *(const int *)user) return -1;
    for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
    *hash = fnv1a(palette, 16);
    return 0;
}

static void setup(DM2_V1_ViewportState *viewport, uint8_t *framebuffer,
                  DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt,
                  int *expected_index)
{
    DM2_V1_G1ContainerMapChipMaterial *material;
    uint8_t palette[16];

    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->map = 9;
    receipt->material_count = 1;
    material = &receipt->materials[0];
    material->object_id = 0xe408u;
    material->x = 11;
    material->y = 19;
    material->direction = 3;
    material->container_type = 0;
    material->raw_hash = 1u;
    material->raw_byte_count = 16u;
    material->image_width = 4;
    material->image_height = 4;
    material->image_format = 3;
    for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
    material->local_palette_hash = fnv1a(palette, 16);
    material->decoded_pixel_hash = fnv1a(g_pixels, sizeof(g_pixels));

    *expected_index = dm2_v1_viewport_item_graphic_index(0x14, 0, 0xf9);
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    viewport->item_count = 1;
    viewport->items[0].item_category = 0x14;
    viewport->items[0].screen_x = 96;
    viewport->items[0].screen_y = 88;
    viewport->items[0].object_id = material->object_id;
    viewport->items[0].map_x = material->x;
    viewport->items[0].map_y = material->y;
    viewport->items[0].direction = material->direction;
    viewport->items[0].source_gdat_field = 0xf9;
    viewport->items[0].source_g1_container = 1;
    viewport->items[0].source_static_object_admitted = 1;
    viewport->items[0].source_static_object_cell = 6;
    viewport->items[0].source_static_object_pass = 14;
    dm2_v1_viewport_set_g1_container_map_chip_materials(viewport, receipt);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_material, expected_index);
    dm2_v1_viewport_set_asset_palette_provider(viewport, fetch_palette,
                                                expected_index);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    DM2_V1_G1ContainerMapChipRuntimeReceipt receipt;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int expected_index;

    memset(framebuffer, 0, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    {
        uint8_t palette[16];
        for (int i = 0; i < 16; ++i) palette[i] = (uint8_t)(0x30 + i);
        dm2_v1_viewport_set_g1_scene_item_material_direct(
            &viewport, 1, 0x14, 0, expected_index, 0xe408u, 11, 19,
            g_pixels, 4, 4, 4, palette, fnv1a(palette, sizeof(palette)),
            fnv1a(g_pixels, sizeof(g_pixels)));
        dm2_v1_viewport_set_asset_provider(&viewport, NULL, NULL);
        dm2_v1_viewport_set_asset_palette_provider(&viewport, NULL, NULL);
    }
    dm2_v1_render_items(&viewport);
    CHECK("G1 DB9 container consumes direct exact CONTAINERS/type/F9 material",
          viewport.asset_item_drawn_count == 1 &&
              viewport.g1_scene_item_material_consumed_count == 1 &&
              viewport.last_item_render.gdat_index == expected_index &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) == 0u);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    viewport.items[0].map_y++;
    dm2_v1_render_items(&viewport);
    CHECK("changed G1 DB9 tile blocks container material",
          viewport.asset_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    setup(&viewport, framebuffer, &receipt, &expected_index);
    g_pixels[1] ^= 1u;
    dm2_v1_render_items(&viewport);
    CHECK("changed decoded G1 DB9 pixels block container material",
          viewport.asset_item_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM) != 0u &&
              framebuffer[88 * DM2_VP_WIDTH + 96] == 0x7eu);
    g_pixels[1] ^= 1u;

    printf("DM2 G1 DB9 viewport material gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
