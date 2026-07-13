/* Focused gate for skproject DRAW_ITEM_SURVEY's static description frame. */
#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_survey_frame(void)
{
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entry;
    uint8_t raw[27];
    uint32_t offset = 0u;
    uint32_t size = sizeof(raw);
    DM2_V1_InventoryPanelSurveyFrameReceipt frame;
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
    raw[10] = 0x2du;
    for (palette = 0; palette < 16; ++palette) raw[11 + palette] = (uint8_t)(0x50 + palette);
    entry.cls1 = DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET;
    entry.cls2 = 0u;
    entry.cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entry.cls4 = 1u;
    entry.data_index = 0u;
    loader.loaded = 1;
    loader.entries = &entry;
    loader.entry_count = 1u;
    loader.raw_offsets = &offset;
    loader.raw_sizes = &size;
    loader.raw_data_count = 1u;
    loader.data = raw;
    loader.data_size = sizeof(raw);

    CHECK(dm2_v1_inventory_panel_survey_frame_receipt(&loader, &frame) &&
              frame.valid && frame.image_field == 1u &&
              frame.expanded_rect_index == DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT &&
              frame.decoded_width == 2u && frame.decoded_height == 1u,
          "survey frame binds source image field and original rect");
    memset(&blit, 0, sizeof(blit));
    blit.rect_number = DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT;
    blit.destination_x = 1;
    blit.destination_y = 1;
    blit.width = 2u;
    blit.height = 1u;
    blit.transparent_index = UINT8_MAX;
    surface.pixels = pixels;
    surface.width = 3;
    surface.height = 2;
    surface.stride = 3;
    CHECK(dm2_v1_inventory_panel_consume_survey_frame(
              &loader, &frame, &blit, &surface, &consumption) && consumption.valid &&
              consumption.drawn_pixel_count == 2u && pixels[4] == 0x52u &&
              pixels[5] == 0x5du,
          "survey frame consumes exact local-palette GDAT pixels");
    blit.transparent_index = 12u;
    CHECK(!dm2_v1_inventory_panel_consume_survey_frame(
              &loader, &frame, &blit, &surface, &consumption),
          "survey frame rejects preview transparency mode");
    raw[10] = 0x21u;
    blit.transparent_index = UINT8_MAX;
    CHECK(!dm2_v1_inventory_panel_consume_survey_frame(
              &loader, &frame, &blit, &surface, &consumption),
          "changed source pixels cannot produce a replacement frame");
}

int main(void)
{
    test_survey_frame();
    printf("DM2 inventory survey frame gate: %d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
