/* Focused gate for the skproject DRAW_HAND_ACTION_ICONS backdrop route. */
#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_hand_slot_backdrop(void)
{
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entry;
    uint8_t raw[27];
    uint32_t offset = 0u;
    uint32_t size = sizeof(raw);
    DM2_V1_InventoryPanelHandSlotBackdropReceipt receipt;
    DM2_V1_InventoryPanelHudBlit blit;
    DM2_V1_InventoryPanelHudSurface surface;
    DM2_V1_InventoryPanelHudConsumptionReceipt consumption;
    uint8_t pixels[6] = {0u};
    int palette;

    memset(&loader, 0, sizeof(loader));
    memset(&entry, 0, sizeof(entry));
    memset(raw, 0, sizeof(raw));
    raw[0] = 2u;
    raw[2] = 1u;
    raw[3] = 0x80u;
    raw[4] = 4u;
    raw[10] = 0x1cu;
    for (palette = 0; palette < 16; ++palette) raw[11 + palette] = (uint8_t)(0x30 + palette);
    entry.cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entry.cls2 = 0x04u;
    entry.cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entry.cls4 = 3u;
    entry.data_index = 0u;
    loader.loaded = 1;
    loader.entries = &entry;
    loader.entry_count = 1u;
    loader.raw_offsets = &offset;
    loader.raw_sizes = &size;
    loader.raw_data_count = 1u;
    loader.data = raw;
    loader.data_size = sizeof(raw);

    CHECK(dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
              &loader, 0u, 1u, 2u, 1u, &receipt) && receipt.valid &&
              receipt.image_field == 3u && receipt.expanded_rect_index == 0x4bu &&
              receipt.decoded_width == 2u && receipt.decoded_height == 1u,
          "hand-slot backdrop binds exact source field and directional rect");
    memset(&blit, 0, sizeof(blit));
    blit.rect_number = 0x4bu;
    blit.destination_x = 1;
    blit.destination_y = 1;
    blit.width = 2u;
    blit.height = 1u;
    blit.transparent_index = UINT8_MAX;
    surface.pixels = pixels;
    surface.width = 3;
    surface.height = 2;
    surface.stride = 3;
    CHECK(dm2_v1_inventory_panel_consume_hand_slot_backdrop(
              &loader, &receipt, &blit, &surface, &consumption) && consumption.valid &&
              consumption.drawn_pixel_count == 2u && consumption.transparent_pixel_count == 0u &&
              pixels[4] == 0x31u && pixels[5] == 0x3cu,
          "hand-slot backdrop consumes exact GDAT image and local palette");
    blit.rect_number = 0x4cu;
    CHECK(!dm2_v1_inventory_panel_consume_hand_slot_backdrop(
              &loader, &receipt, &blit, &surface, &consumption),
          "different source rect fails closed");
    CHECK(!dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
              &loader, 2u, 0u, 0u, 0u, &receipt),
          "unsupported possession has no fallback backdrop");
}

int main(void)
{
    test_hand_slot_backdrop();
    printf("DM2 inventory hand-slot backdrop gate: %d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
