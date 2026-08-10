/*
 * test_dm2_v1_img3_u4_header.c
 *
 * Regression coverage for the DM2 IMG3/U4 header variant used by querydb.
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void fixture_loader(DM2_V1_AssetLoader *loader,
                           uint8_t data[80],
                           uint32_t raw_offsets[1],
                           uint32_t raw_sizes[1],
                           DM2_V1_GdatEntry entries[1])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, 80u);

    data[16] = 2u;
    data[17] = 0u;
    data[18] = 2u;
    data[19] = 0x80u;
    data[22] = 4u;
    data[23] = 0u;
    data[26] = 0x12u;
    data[27] = 0x34u;
    memcpy(data + 28u, "\x00\x01\x02\x03\x04\x05\x06\x07"
                       "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 16u);

    raw_offsets[0] = 16u;
    raw_sizes[0] = 28u;

    entries[0].cls1 = DM2_GDAT_CATEGORY_TITLE;
    entries[0].cls2 = 1u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[0].cls4 = 0u;
    entries[0].data_index = 0u;

    loader->data = data;
    loader->data_size = 80u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 1u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 1u;
}

int main(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[80];
    uint32_t raw_offsets[1];
    uint32_t raw_sizes[1];
    DM2_V1_GdatEntry entries[1];
    DM2_V1_GdatImageEntryBuffReceipt image;
    DM2_V1_GdatImageExtractReceipt extract;

    printf("DM2 IMG3/U4 header regression\n");
    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);

    CHECK(dm2_v1_query_gdat_image_entry_buff_receipt(
              &loader, DM2_GDAT_CATEGORY_TITLE, 1, 0, &image) &&
              image.accepted &&
              image.selected_raw_index == 0u &&
              image.width == 2u &&
              image.height == 2u &&
              image.bits_per_pixel == 4u,
          "QUERY_GDAT_IMAGE_ENTRY_BUFF admits U4 bpp header word");
    CHECK(dm2_v1_extract_gdat_image_receipt(
              &loader, 0u, 0, 1, NULL, 0u, &extract) &&
              extract.valid &&
              !extract.decode_img3_underlay &&
              !extract.decode_img3_overlay &&
              !extract.decode_img9 &&
              extract.bpp == 4u &&
              extract.width == 2u &&
              extract.height == 2u &&
              extract.decoded_pixel_hash != 0u,
          "DM2_EXTRACT_GDAT_IMAGE uses the direct U4 payload route");

    printf("DM2 IMG3/U4 header regression: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
