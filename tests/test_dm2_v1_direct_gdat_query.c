/*
 * test_dm2_v1_direct_gdat_query.c
 *
 * Focused SKWIN/SkWinCore2 direct GDAT query receipts.
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
                           uint32_t raw_offsets[2],
                           uint32_t raw_sizes[2],
                           DM2_V1_GdatEntry entries[3])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, 80u);
    memcpy(data + 16u, "IMGRAW", 6u);
    memcpy(data + 32u, "TEXT:OK", 7u);

    raw_offsets[0] = 16u;
    raw_offsets[1] = 32u;
    raw_sizes[0] = 6u;
    raw_sizes[1] = 7u;

    entries[0].cls1 = DM2_GDAT_CATEGORY_TITLE;
    entries[0].cls2 = 0u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[0].cls4 = 4u;
    entries[0].data_index = 0u;

    entries[1].cls1 = DM2_GDAT_CATEGORY_MESSAGES;
    entries[1].cls2 = 2u;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[1].cls4 = 7u;
    entries[1].data_index = 1u;

    entries[2].cls1 = DM2_GDAT_CATEGORY_WEAPONS;
    entries[2].cls2 = 9u;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[2].cls4 = 1u;
    entries[2].data_index = 0x1234u;

    loader->data = data;
    loader->data_size = 80u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 2u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 3u;
}

int main(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[80];
    uint32_t raw_offsets[2];
    uint32_t raw_sizes[2];
    DM2_V1_GdatEntry entries[3];
    DM2_V1_DirectGdatEntryDataBuffReceipt buff;
    DM2_V1_DirectGdatTextReceipt text;
    const uint8_t *ptr;
    size_t size = 0u;

    printf("DM2 direct GDAT query receipts\n");
    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);

    ptr = dm2_v1_direct_query_gdat_entry_data_buff_receipt(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0, DM2_GDAT_ENTRY_TYPE_IMAGE, 4,
        &size, &buff);
    CHECK(ptr == data + 16u && size == 6u && buff.accepted &&
              buff.raw_index == 0u && buff.data_index == 0u &&
              buff.raw_length == 6u && buff.raw_hash != 0u &&
              memcmp(ptr, "IMGRAW", 6u) == 0,
          "DIRECT_QUERY_GDAT_ENTRY_DATA_BUFF returns exact typed raw bytes");

    ptr = dm2_v1_direct_query_gdat_text_receipt(
        &loader, DM2_GDAT_CATEGORY_MESSAGES, 2, 7, &size, &text);
    CHECK(ptr == data + 32u && size == 7u && text.accepted &&
              text.raw_index == 1u && text.text_length == 7u &&
              text.text_hash != 0u && memcmp(ptr, "TEXT:OK", 7u) == 0,
          "DIRECT_QUERY_GDAT_TEXT returns exact dtText bytes");

    ptr = dm2_v1_direct_query_gdat_entry_data_buff_receipt(
        &loader, DM2_GDAT_CATEGORY_WEAPONS, 9,
        DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 1, &size, &buff);
    CHECK(ptr == NULL && size == 0u && !buff.accepted,
          "DIRECT_QUERY_GDAT_ENTRY_DATA_BUFF rejects scalar word entries");

    ptr = dm2_v1_direct_query_gdat_text_receipt(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0, 4, &size, &text);
    CHECK(ptr == NULL && size == 0u && !text.accepted,
          "DIRECT_QUERY_GDAT_TEXT rejects non-text typed entries");

    printf("DM2 direct GDAT query receipts: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
