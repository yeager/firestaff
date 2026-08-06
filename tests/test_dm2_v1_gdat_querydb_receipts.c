/*
 * test_dm2_v1_gdat_querydb_receipts.c
 *
 * Focused skproject c_gdatfile.cpp/c_querydb.cpp named-symbol receipts.
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void fixture_loader(DM2_V1_AssetLoader *loader,
                           uint8_t data[256],
                           uint32_t raw_offsets[8],
                           uint32_t raw_sizes[8],
                           DM2_V1_GdatEntry entries[9])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, 256u);
    memcpy(data + 16u, "IMGRAW", 6u);
    memcpy(data + 32u, "TEXT", 4u);
    memcpy(data + 48u, "PAL0123456789ABC", 16u);
    memcpy(data + 80u, "\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa", 10u);
    data[96u] = 2u;
    data[98u] = 2u;
    data[99u] = 0x80u;
    data[102u] = 4u;
    data[106u] = 0x12u;
    data[107u] = 0x34u;
    memcpy(data + 108u, "\x00\x01\x02\x03\x04\x05\x06\x07"
                         "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 16u);
    data[140u] = 4u;
    data[141u] = 0u;
    data[142u] = 3u;
    data[143u] = 0u;
    data[150u] = 3u;
    data[151u] = 0u;
    data[152u] = 2u;
    data[153u] = 0u;
    data[154u] = 4u;
    data[155u] = 0u;
    data[156u] = 4u;
    data[157u] = 0u;
    memcpy(data + 160u, "\x10\x11\x12\x13\x14\x15"
                         "\x00\x01\x02\x03\x04\x05\x06\x07"
                         "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 22u);
    data[190u] = 8u;
    data[191u] = 0u;
    data[192u] = 4u;
    data[193u] = 0u;
    data[194u] = 4u;
    data[195u] = 0u;
    data[196u] = 4u;
    data[197u] = 0u;
    memcpy(data + 200u, "\x20\x21\x22\x23\x24\x25\x26\x27"
                         "\x00\x03\x06\x09\x0c\x0f\x12\x15"
                         "\x18\x1b\x1e\x21\x24\x27\x2a\x2d", 24u);
    raw_offsets[0] = 16u;
    raw_offsets[1] = 32u;
    raw_offsets[2] = 48u;
    raw_offsets[3] = 80u;
    raw_offsets[4] = 96u;
    raw_offsets[5] = 140u;
    raw_offsets[6] = 150u;
    raw_offsets[7] = 190u;
    raw_sizes[0] = 6u;
    raw_sizes[1] = 4u;
    raw_sizes[2] = 16u;
    raw_sizes[3] = 10u;
    raw_sizes[4] = 28u;
    raw_sizes[5] = 4u;
    raw_sizes[6] = 32u;
    raw_sizes[7] = 34u;
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
    entries[3].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[3].cls2 = 3u;
    entries[3].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    entries[3].cls4 = 0xfeu;
    entries[3].data_index = 0xfe02u;
    entries[4].cls1 = DM2_GDAT_CATEGORY_MUSICS;
    entries[4].cls2 = 1u;
    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_SOUND;
    entries[4].cls4 = 2u;
    entries[4].data_index = 3u;
    entries[5].cls1 = DM2_GDAT_CATEGORY_TITLE;
    entries[5].cls2 = 1u;
    entries[5].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[5].cls4 = 0u;
    entries[5].data_index = 4u;
    entries[6].cls1 = 0u;
    entries[6].cls2 = 0u;
    entries[6].cls3 = DM2_GDAT_ENTRY_TYPE_RAW8;
    entries[6].cls4 = 0u;
    entries[6].data_index = 5u;
    entries[7].cls1 = DM2_GDAT_CATEGORY_MISCELLANEOUS;
    entries[7].cls2 = 0xfeu;
    entries[7].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[7].cls4 = 0xfeu;
    entries[7].data_index = 6u;
    entries[8].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[8].cls2 = 2u;
    entries[8].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[8].cls4 = DM2_GDAT_IMG_MAP_CHIP;
    entries[8].data_index = 7u;
    loader->data = data;
    loader->data_size = 256u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 8u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 9u;
}

static void test_fixture_entry_queries(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[256];
    uint32_t raw_offsets[8];
    uint32_t raw_sizes[8];
    DM2_V1_GdatEntry entries[9];
    DM2_V1_GdatEntryQueryReceipt receipt;
    DM2_V1_GdatImageEntryBuffReceipt image_receipt;
    DM2_V1_QueryPictBitsReceipt pict_bits;
    DM2_V1_Query4BppPictBuffAndPalReceipt pict4;
    DM2_V1_QueryPicstImageReceipt picst;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    const uint8_t *ptr;
    size_t size = 0u;
    uint32_t u32 = 0u;
    uint32_t value = 0u;
    uint16_t u16 = 0u;
    uint16_t width = 0u;
    uint16_t height = 0u;
    uint8_t copy[16];

    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);

    CHECK(dm2_v1_query_gdat_raw_data_file_pos(&loader, 0u, &u32) &&
              u32 == 16u,
          "QUERY_GDAT_RAW_DATA_FILE_POS resolves raw-table offset");
    CHECK(dm2_v1_query_gdat_raw_data_length(&loader, 2u, &u32) &&
              u32 == 16u,
          "QUERY_GDAT_RAW_DATA_LENGTH resolves raw-table byte count");
    ptr = dm2_v1_load_gdat_raw_data(&loader, 1u, &size);
    CHECK(ptr == data + 32u && size == 4u && memcmp(ptr, "TEXT", 4u) == 0,
          "LOAD_GDAT_RAW_DATA returns the bounded original payload");

    CHECK(dm2_v1_query_gdat_entry(
              &loader, DM2_GDAT_CATEGORY_TITLE, 0,
              DM2_GDAT_ENTRY_TYPE_IMAGE, 4, &receipt) &&
              receipt.present && receipt.loadable_raw &&
              receipt.raw_index == 0u && receipt.raw_file_pos == 16u &&
              receipt.raw_length == 6u && receipt.receipt_hash != 0u,
          "QUERY_GDAT_ENTRYPTR receipt binds class tuple to raw payload");
    CHECK(dm2_v1_query_gdat_entry_value(&loader, 0u, 0u, &value) &&
              value == DM2_GDAT_CATEGORY_TITLE &&
          dm2_v1_query_gdat_entry_value(&loader, 0u, 1u, &value) &&
              value == 0u &&
          dm2_v1_query_gdat_entry_value(&loader, 0u, 2u, &value) &&
              value == DM2_GDAT_ENTRY_TYPE_IMAGE &&
          dm2_v1_query_gdat_entry_value(&loader, 0u, 3u, &value) &&
              value == 4u &&
          dm2_v1_query_gdat_entry_value(&loader, 0u, 4u, &value) &&
              value == 0u,
          "DM2_QUERY_GDAT_ENTRY_VALUE returns source ENT1 group values");
    CHECK(!dm2_v1_query_gdat_entry_value(&loader, 99u, 0u, &value) &&
              value == 0u,
          "DM2_QUERY_GDAT_ENTRY_VALUE rejects out-of-range entry ordinals");
    ptr = dm2_v1_query_gdat_entry_data_ptr(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0,
        DM2_GDAT_ENTRY_TYPE_IMAGE, 4, &size);
    CHECK(ptr == data + 16u && size == 6u && memcmp(ptr, "IMGRAW", 6u) == 0,
          "QUERY_GDAT_ENTRY_DATA_PTR returns exact typed payload");
    CHECK(dm2_v1_query_gdat_entry_data_length(
              &loader, DM2_GDAT_CATEGORY_TITLE, 0,
              DM2_GDAT_ENTRY_TYPE_IMAGE, 4, &u32) &&
              u32 == 6u,
          "QUERY_GDAT_ENTRY_DATA_LENGTH mirrors payload length");
    CHECK(dm2_v1_query_gdat_entry_if_loadable(
              &loader, DM2_GDAT_CATEGORY_MESSAGES, 2,
              DM2_GDAT_ENTRY_TYPE_TEXT, 7, &receipt) &&
              receipt.loadable_raw && receipt.raw_index == 1u,
          "QUERY_GDAT_ENTRY_IF_LOADABLE accepts loadable dtText");
    memset(copy, 0, sizeof(copy));
    CHECK(dm2_v1_load_gdat_entry_data_to(
              &loader, DM2_GDAT_CATEGORY_MESSAGES, 2,
              DM2_GDAT_ENTRY_TYPE_TEXT, 7, copy, sizeof(copy), &receipt) &&
              receipt.copied_to_destination &&
              receipt.copied_length == 4u &&
              memcmp(copy, "TEXT", 4u) == 0,
          "DM2_LOAD_GDAT_ENTRY_DATA_TO copies only the exact raw payload");
    memset(copy, 0, sizeof(copy));
    CHECK(!dm2_v1_load_gdat_entry_data_to(
              &loader, DM2_GDAT_CATEGORY_MESSAGES, 2,
              DM2_GDAT_ENTRY_TYPE_TEXT, 7, copy, 2u, &receipt) &&
              !receipt.copied_to_destination,
          "DM2_LOAD_GDAT_ENTRY_DATA_TO rejects undersized destinations");

    CHECK(dm2_v1_query_gdat_entry_data_index(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 9,
              DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 1, &u16) &&
              u16 == 0x1234u,
          "QUERY_GDAT_ENTRY_DATA_INDEX returns scalar dtWordValue");
    CHECK(dm2_v1_query_gdat_entry(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 3,
              DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET, 0xfe, &receipt) &&
              receipt.present && !receipt.loadable_raw &&
              receipt.data_index == 0xfe02u,
          "QUERY_GDAT_ENTRY_DATA_INDEX preserves dtImageOffset scalar");
    CHECK(dm2_v1_query_gdat_entry_data_ptr(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 9,
              DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 1, &size) == NULL &&
              size == 0u,
          "scalar GDAT entries are not exposed as synthetic buffers");
    CHECK(!dm2_v1_load_gdat_entry_data_to(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 9,
              DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 1, copy, sizeof(copy),
              &receipt) &&
              !receipt.copied_to_destination,
          "DM2_LOAD_GDAT_ENTRY_DATA_TO rejects scalar entries");

    CHECK(dm2_v1_query_gdat_image_entry_buff_receipt(
              &loader, DM2_GDAT_CATEGORY_TITLE, 1, 0, &image_receipt) &&
              image_receipt.accepted &&
              !image_receipt.used_default_image &&
              image_receipt.selected_raw_index == 4u &&
              image_receipt.width == 2u &&
              image_receipt.height == 2u &&
              image_receipt.bits_per_pixel == 4u &&
              image_receipt.raw_hash != 0u,
          "QUERY_GDAT_IMAGE_ENTRY_BUFF admits the exact real dtImage raw payload");
    CHECK(dm2_v1_query_gdat_image_entry_buff_receipt(
              &loader, DM2_GDAT_CATEGORY_TITLE, 99, 0, &image_receipt) &&
              image_receipt.accepted &&
              image_receipt.used_default_image &&
              image_receipt.requested_data_index == 0xffffu &&
              image_receipt.selected_raw_index == 6u &&
              image_receipt.width == 3u &&
              image_receipt.height == 2u,
          "QUERY_GDAT_IMAGE_ENTRY_BUFF falls back only to the real MISC FE/FE image");
    CHECK(dm2_v1_query_gdat_image_metrics_receipt(
              &loader, DM2_GDAT_CATEGORY_TITLE, 1, 0,
              &width, &height, &image_receipt) &&
              width == 2u && height == 2u &&
              image_receipt.selected_data_index == 4u,
          "QUERY_GDAT_IMAGE_METRICS returns source IMG3 dimensions");
    CHECK(dm2_v1_query_pict_bits_receipt(
              &loader, 0x04u, 0, 0, DM2_GDAT_CATEGORY_TITLE, 1, 0,
              &pict_bits) &&
              pict_bits.accepted &&
              pict_bits.queried_gdat_image &&
              pict_bits.selected_raw_index == 4u &&
              pict_bits.width == 2u &&
              pict_bits.height == 2u,
          "QUERY_PICT_BITS mode bit 2 routes through real GDAT image data");
    CHECK(dm2_v1_query_pict_bits_receipt(
              &loader, 0x08u, 0, 1, DM2_GDAT_CATEGORY_TITLE, 1, 0,
              &pict_bits) &&
              pict_bits.used_cached_bitmap &&
              !pict_bits.queried_gdat_image,
          "QUERY_PICT_BITS mode bit 3 requires an existing cached bitmap route");
    CHECK(!dm2_v1_query_pict_bits_receipt(
              &loader, 0x00u, 0, 0, DM2_GDAT_CATEGORY_TITLE, 1, 0,
              &pict_bits),
          "QUERY_PICT_BITS rejects missing current bitmap without fallback");
    CHECK(dm2_v1_query_4bpp_pict_buff_and_pal_receipt(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2, 4u, &pict4) &&
              pict4.accepted &&
              pict4.field == DM2_GDAT_IMG_MAP_CHIP &&
              pict4.selected_raw_index == 7u &&
              pict4.width == 8u &&
              pict4.height == 4u &&
              pict4.width_units == 2u &&
              pict4.palette16[0] == 0x00u &&
              pict4.palette16[15] == 0x2du &&
              pict4.palette_hash != 0u,
          "QUERY_4BPP_PICT_BUFF_AND_PAL admits real map-chip pixels and local palette");
    CHECK(!dm2_v1_query_4bpp_pict_buff_and_pal_receipt(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 99, 4u, &pict4),
          "QUERY_4BPP_PICT_BUFF_AND_PAL rejects absent map-chip rows");
    CHECK(dm2_v1_query_picst_image_receipt(
              &loader, DM2_GDAT_CATEGORY_TITLE, 1, 0, &picst) &&
              picst.accepted &&
              picst.mode == 4u &&
              picst.selected_raw_index == 4u &&
              picst.width == 2u &&
              picst.height == 2u &&
              picst.image_hash != 0u,
          "QUERY_PICST_IMAGE binds source image descriptor to real GDAT pixels");
    CHECK(dm2_v1_query_gdat_summary_image_receipt(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2,
              DM2_GDAT_IMG_MAP_CHIP, &summary) &&
              summary.accepted &&
              !summary.gdat_bypassed_for_ff &&
              summary.metadata.width == 8u &&
              summary.metadata.height == 4u &&
              summary.metadata.bits_per_pixel == 4u &&
              summary.colors == 16u &&
              summary.palette16[15] == 0x2du &&
              summary.palette_hash != 0u,
          "QUERY_GDAT_SUMMARY_IMAGE binds metadata offsets and local palette");
    CHECK(dm2_v1_query_gdat_summary_image_receipt(
              &loader, 0xff, 0, 0, &summary) &&
              summary.accepted &&
              summary.gdat_bypassed_for_ff &&
              summary.colors == 0xffu,
          "QUERY_GDAT_SUMMARY_IMAGE cls FF bypasses GDAT lookup");
}

static void test_fixture_gdat_entry_iteration_and_sound(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[256];
    uint8_t sample[4] = {0x00u, 0x7fu, 0x80u, 0xffu};
    uint32_t raw_offsets[8];
    uint32_t raw_sizes[8];
    DM2_V1_GdatEntry entries[9];
    DM2_V1_GdatLoadEntriesReceipt load_receipt;
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry_receipt;
    DM2_V1_GdatSoundToggleReceipt toggle;
    DM2_V1_GdatSoundEntryReceipt sound_receipt;
    DM2_V1_DballocSoundCensusReceipt census;
    DM2_V1_DballocEntryFilterReceipt filter;
    DM2_V1_LoadDyn4AdmissionReceipt dyn4;
    DM2_V1_GdatMaxRawLengthReceipt max_raw;

    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);

    CHECK(dm2_v1_load_gdat_entries_receipt(&loader, &load_receipt) &&
              load_receipt.valid &&
              load_receipt.entry_count == 9u &&
              load_receipt.loadable_entry_count == 7u &&
              load_receipt.scalar_entry_count == 2u &&
              load_receipt.payload_bytes == 118u &&
              load_receipt.allocated_bytes_with_length_words == 132u,
          "LOAD_GDAT_ENTRIES receipt preloads only non-scalar raw payloads");

    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = DM2_GDAT_CATEGORY_MESSAGES;
    iterator.category_last = DM2_GDAT_CATEGORY_MUSICS;
    iterator.index_filter = -1;
    iterator.type_filter = -1;
    iterator.field_filter = -1;
    CHECK(dm2_v1_query_next_gdat_entry(&loader, &iterator, &entry_receipt) &&
              entry_receipt.category == DM2_GDAT_CATEGORY_MESSAGES &&
              entry_receipt.type == DM2_GDAT_ENTRY_TYPE_TEXT,
          "QUERY_NEXT_GDAT_ENTRY walks the first filtered category hit");
    CHECK(dm2_v1_query_next_gdat_entry(&loader, &iterator, &entry_receipt) &&
              entry_receipt.category == DM2_GDAT_CATEGORY_MUSICS &&
              entry_receipt.type == DM2_GDAT_ENTRY_TYPE_SOUND,
          "QUERY_NEXT_GDAT_ENTRY resumes after the prior raw-table hit");
    CHECK(!dm2_v1_query_next_gdat_entry(&loader, &iterator, &entry_receipt),
          "QUERY_NEXT_GDAT_ENTRY stops at the filtered range boundary");

    CHECK(dm2_v1_gdat_sound_toggle_payload(sample, 4u, 0u, 1u, &toggle) &&
              toggle.accepted &&
              toggle.flags_before == 1u &&
              toggle.flags_after == 0u &&
              toggle.toggled_bytes == 4u &&
              sample[0] == 0x80u && sample[1] == 0xffu &&
              sample[2] == 0x00u && sample[3] == 0x7fu,
          "DM2_47eb_00a4 clears the pending flag and XORs sample high bits");
    CHECK(!dm2_v1_gdat_sound_toggle_payload(sample, 4u, 1u, 1u, &toggle) &&
              !toggle.accepted &&
              toggle.payload_hash_before == toggle.payload_hash_after,
          "DM2_47eb_00a4 rejects already-converted sample payloads");

    CHECK(dm2_v1_gdat_sound_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_MUSICS, 1, 2,
              0, 0, &sound_receipt) &&
              sound_receipt.accepted &&
              sound_receipt.data_index == 3u &&
              sound_receipt.header_skip_bytes == 2u &&
              sound_receipt.payload_offset == 82u &&
              sound_receipt.payload_length == 8u,
          "DM2_482b_0684 binds a sound row to dt02 after the short header");
    CHECK(dm2_v1_gdat_sound_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_MUSICS, 1, 2,
              0, 1, &sound_receipt) &&
              sound_receipt.header_skip_bytes == 6u &&
              sound_receipt.payload_offset == 86u &&
              sound_receipt.payload_length == 4u,
          "DM2_482b_0684 preserves the alternate six-byte sound header");
    CHECK(!dm2_v1_gdat_sound_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_MUSICS, 1, 2,
              7, 0, &sound_receipt),
          "DM2_482b_0684 does not admit payloads rejected by SOUND7");

    CHECK(dm2_v1_r_2bad4_swap_word(0x1234u) == 0x3412u &&
              dm2_v1_r_2bad4_swap_word(0x00ffu) == 0xff00u,
          "R_2BAD4 byte-swaps source 16-bit words");
    entries[0].cls4 = 3u;
    entries[0].data_index = 2u;
    CHECK(dm2_v1_r_2d07d_max_raw_length_receipt(
              &loader, DM2_GDAT_ENTRY_TYPE_IMAGE, 3u, &max_raw) &&
              max_raw.accepted &&
              max_raw.type_filter == DM2_GDAT_ENTRY_TYPE_IMAGE &&
              max_raw.field_filter == 3u &&
              max_raw.scanned_entry_count == 1u &&
              max_raw.max_raw_length == 16u &&
              max_raw.receipt_hash != 0u,
          "R_2D07D scans GDAT field-3 loadable image entries for max raw length");
    entries[0].cls4 = 4u;
    entries[0].data_index = 0u;

    CHECK(dm2_v1_dballoc_3e74_24b8_receipt(&loader, &census) &&
              census.accepted &&
              census.sound_entry_count == 1u &&
              census.unique_raw_index_count == 1u &&
              census.max_raw_length == 10u &&
              census.scratch_allocation_bytes == 2u,
          "DM2_dballoc_3e74_24b8 counts sound GDAT entries and max raw length");
    CHECK(dm2_v1_dballoc_3e74_2162_receipt(&loader, 0u, 0x10u, &filter) &&
              filter.accepted && filter.allowed && filter.cls5_mask == 0u,
          "DM2_dballoc_3e74_2162 accepts zero cls5 high-nibble mask");
    entries[0].cls5 = 0x20u;
    CHECK(dm2_v1_dballoc_3e74_2162_receipt(&loader, 0u, 0x10u, &filter) &&
              filter.accepted && !filter.allowed &&
              filter.cls5_mask == 0x20u,
          "DM2_dballoc_3e74_2162 rejects nonmatching cls5 high-nibble mask");
    entries[0].cls5 = 0x10u;
    CHECK(dm2_v1_dballoc_3e74_2162_receipt(&loader, 0u, 0x10u, &filter) &&
              filter.accepted && filter.allowed &&
              filter.cls5_mask == 0x10u,
          "DM2_dballoc_3e74_2162 accepts active cls5 high-nibble mask");
    entries[0].cls5 = 0u;
    CHECK(dm2_v1_load_dyn4_admission_receipt(&loader, 2u, 0, &dyn4) &&
              dyn4.accepted &&
              dyn4.entry_count == 9u &&
              dyn4.descriptor_count == 2u &&
              dyn4.marker_allocation_bytes == 9u &&
              dyn4.requested_sound_cleanup,
          "DM2_LOAD_DYN4 admission allocates clean low-pool marker bytes and requests SOUND5");
    CHECK(dm2_v1_load_dyn4_admission_receipt(&loader, 2u, 1, &dyn4) &&
              dyn4.accepted &&
              dyn4.early_dealloc_when_locked &&
              !dyn4.requested_sound_cleanup,
          "DM2_LOAD_DYN4 admission records cache-locked early deallocation route");
}

static void test_graphics_structure_and_image_extract(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[256];
    uint32_t raw_offsets[8];
    uint32_t raw_sizes[8];
    DM2_V1_GdatEntry entries[9];
    DM2_V1_GraphicsStructureReceipt structure;
    DM2_V1_GdatImageExtractReceipt image;
    DM2_V1_GdatUnderlayPair underlays[1];
    DM2_V1_GdatEntry startup_entry;
    int16_t underlay = -1;

    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);
    underlays[0].image_raw_index = 4u;
    underlays[0].underlay_raw_index = 3;
    /* c_gdatfile.cpp::DM2_READ_GRAPHICS_STRUCTURE queries this scalar after
     * ENT1. The receipt must retain it and derive the image-cache route. */
    startup_entry = entries[2];
    entries[2].cls1 = 0u;
    entries[2].cls2 = 0u;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[2].cls4 = 0u;
    entries[2].data_index = 0x0060u;

    CHECK(dm2_v1_read_graphics_structure_receipt(&loader, &structure) &&
              structure.valid &&
              structure.entries == 9u &&
              structure.raw_data_count == 8u &&
              structure.raw0_length == 6u &&
              structure.calculated_payload_end == 224u &&
              structure.max_raw_payload_length == 34u &&
              structure.has_underlay_table &&
              structure.underlay_pair_count == 1u &&
              structure.source_startup_word == 0x0060u &&
              structure.source_sound_mode &&
              structure.source_image_allocator_mode &&
              structure.source_image_cache_limit == 0x001fu,
          "DM2_READ_GRAPHICS_STRUCTURE retains GDAT table, underlay and setup word");
    entries[2] = startup_entry;
    CHECK(dm2_v1_gdat_track_underlay(underlays, 1u, 4u, &underlay) &&
              underlay == 3,
          "DM2_TRACK_UNDERLAY binary-searches the loaded image underlay table");
    CHECK(!dm2_v1_gdat_track_underlay(underlays, 1u, 2u, &underlay) &&
              underlay == -1,
          "DM2_TRACK_UNDERLAY rejects absent raw indexes without substitute");
    CHECK(dm2_v1_extract_gdat_image_receipt(
              &loader, 4u, 0, 1, NULL, 0u, &image) &&
              image.valid &&
              !image.decode_img3_underlay &&
              !image.uses_underlay &&
              image.width == 2u &&
              image.height == 2u &&
              image.bpp == 4u &&
              image.pixel_payload_bytes == 18u &&
              image.allocation_bytes == 32u &&
              image.decoded_pixel_hash != 0u,
          "DM2_EXTRACT_GDAT_IMAGE decodes source OffsetY=-32 U4 payload bytes");
    CHECK(dm2_v1_extract_gdat_image_receipt(
              &loader, 4u, 1, 0, underlays, 1u, &image) &&
              image.valid &&
              image.uses_underlay &&
              image.decode_img3_overlay &&
              image.underlay_raw_index == 3u &&
              image.pixel_payload_bytes == 2u &&
              image.allocation_bytes == 0x18u &&
              image.decoded_pixel_hash == 0u,
          "DM2_EXTRACT_GDAT_IMAGE records underlay overlay route without fabricated pixels");
}

static void test_gfx_material_ownership_routes(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[256];
    uint32_t raw_offsets[8];
    uint32_t raw_sizes[8];
    DM2_V1_GdatEntry entries[9];
    DM2_V1_GdatGfxMaterialReceipt material;

    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);
    CHECK(dm2_v1_gdat_allocate_gfx256_material_receipt(
              &loader, 4u, 1, &material) && material.accepted &&
              material.raw_index == 4u && material.source_bytes == data + 96u &&
              material.source_byte_count == 28u && material.image.valid &&
              !material.image.decode_img3_underlay,
          "DM2_ALLOCATE_GFX256 exposes source OffsetY=-32 image material");
    CHECK(dm2_v1_gdat_allocate_gfx16_material_receipt(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2,
              DM2_GDAT_IMG_MAP_CHIP, 0, &material) && material.accepted &&
              !material.used_gfx16_default && material.raw_index == 7u &&
              material.source_bytes == data + 190u,
          "DM2_ALLOCATE_GFX16 resolves the requested real GDAT image tuple");
    CHECK(dm2_v1_gdat_allocate_gfx16_material_receipt(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 99, 1, 0, &material) &&
              material.accepted && material.used_gfx16_default &&
              material.selected_category == DM2_GDAT_CATEGORY_MISCELLANEOUS &&
              material.raw_index == 6u && material.source_bytes == data + 150u,
          "DM2_ALLOCATE_GFX16 uses only SKULLWIN's real default image route");
    CHECK(!dm2_v1_gdat_allocate_gfx256_material_receipt(
               &loader, 1u, 0, &material) && !material.accepted &&
              material.source_bytes == NULL,
          "GFX material route rejects non-image GDAT bytes without substitute art");
}

static void test_graphics_data_file_lifecycle(void)
{
    DM2_V1_GraphicsDataFileState state;
    DM2_V1_GraphicsDataOpenReceipt open_receipt;
    DM2_V1_GraphicsDataCloseReceipt close_receipt;
    DM2_V1_GraphicsDataReadReceipt read_receipt;

    memset(&state, 0, sizeof(state));
    state.filetype2 = 1u;
    state.primary_file_size = 100u;

    CHECK(dm2_v1_graphics_data_open_receipt(
              &state, 1, 11, 1, 22, &open_receipt) &&
              open_receipt.valid &&
              open_receipt.opened_primary &&
              open_receipt.opened_secondary &&
              state.file_open_counter == 1 &&
              state.file_handle == 11 &&
              state.xfile_handle == 22,
          "GRAPHICS_DATA_OPEN opens primary and secondary files at counter one");
    CHECK(dm2_v1_graphics_data_open_receipt(
              &state, 0, -1, 0, -1, &open_receipt) &&
              open_receipt.valid &&
              !open_receipt.opened_primary &&
              !open_receipt.opened_secondary &&
              state.file_open_counter == 2,
          "GRAPHICS_DATA_OPEN nested calls only increment the open counter");

    CHECK(dm2_v1_graphics_data_read_receipt(
              &state, 90u, 30u, &read_receipt) &&
              read_receipt.valid &&
              read_receipt.uses_primary &&
              read_receipt.uses_secondary &&
              read_receipt.crosses_secondary_split &&
              read_receipt.primary_offset == 90u &&
              read_receipt.primary_length == 10u &&
              read_receipt.secondary_offset == 0u &&
              read_receipt.secondary_length == 20u,
          "GRAPHICS_DATA_READ splits reads crossing the secondary file boundary");
    CHECK(dm2_v1_graphics_data_read_receipt(
              &state, 120u, 8u, &read_receipt) &&
              !read_receipt.uses_primary &&
              read_receipt.uses_secondary &&
              read_receipt.secondary_offset == 20u &&
              read_receipt.secondary_length == 8u,
          "GRAPHICS_DATA_READ routes wholly secondary reads to xfilehandle");
    CHECK(dm2_v1_graphics_data_read_receipt(
              &state, 20u, 8u, &read_receipt) &&
              read_receipt.uses_primary &&
              !read_receipt.uses_secondary &&
              read_receipt.primary_offset == 20u &&
              read_receipt.primary_length == 8u,
          "GRAPHICS_DATA_READ keeps primary-only reads on filehandle");

    CHECK(dm2_v1_graphics_data_close_receipt(
              &state, &close_receipt) &&
              close_receipt.valid &&
              !close_receipt.closed_primary &&
              !close_receipt.closed_secondary &&
              state.file_open_counter == 1,
          "GRAPHICS_DATA_CLOSE nested close decrements without closing handles");
    CHECK(dm2_v1_graphics_data_close_receipt(
              &state, &close_receipt) &&
              close_receipt.valid &&
              close_receipt.closed_primary &&
              close_receipt.closed_secondary &&
              state.file_open_counter == 0,
          "GRAPHICS_DATA_CLOSE final close closes primary and secondary handles");
    CHECK(!dm2_v1_graphics_data_close_receipt(
              &state, &close_receipt) &&
              close_receipt.blocked_underflow,
          "GRAPHICS_DATA_CLOSE rejects counter underflow");

    memset(&state, 0, sizeof(state));
    CHECK(!dm2_v1_graphics_data_open_receipt(
              &state, 0, -1, 1, 22, &open_receipt) &&
              open_receipt.blocked_primary_open &&
              open_receipt.syserr_code == 0x29u,
          "GRAPHICS_DATA_OPEN reports source sys error 0x29 for primary failure");
    memset(&state, 0, sizeof(state));
    state.filetype2 = 1u;
    CHECK(!dm2_v1_graphics_data_open_receipt(
              &state, 1, 11, 0, -1, &open_receipt) &&
              open_receipt.blocked_secondary_open &&
              open_receipt.syserr_code == 0x1fu,
          "GRAPHICS_DATA_OPEN reports source sys error 0x1f for secondary failure");
}

static void test_fixture_pict_allocation_receipts(void)
{
    DM2_V1_GdatPictAllocationReceipt alloc;
    DM2_V1_GdatPictFreeReceipt free_receipt;
    DM2_V1_GdatBigpoolMemoryReceipt pool;
    DM2_V1_GdatCpxReserveReceipt reserve;
    DM2_V1_GdatCpxCopyReceipt copy;
    DM2_V1_GdatCpxCompactReceipt compact;
    const uint8_t source_payload[10] = {
        0x04u, 0x00u, 0xaau, 0xbbu, 0xccu,
        0xddu, 0xeeu, 0xffu, 0x11u, 0x22u
    };
    const DM2_V1_GdatCpxBlockInput cpx_blocks[] = {
        { 7u, 6u, 91u, 0u },
        { 0x8008u, 4u, 87u, 1u },
        { 9u, 5u, 82u, 0u }
    };

    CHECK(dm2_v1_gdat_bigpool_memory_receipt(
              5u, DM2_V1_GDAT_PICT_POOL_LOBIG, 1, 0, &pool) &&
              pool.accepted && pool.clean &&
              !pool.deallocate &&
              pool.aligned_bytes == 6u,
          "DM2_ALLOC_LOBIGPOOL_MEMORY aligns odd low-pool allocation bytes");
    CHECK(dm2_v1_gdat_bigpool_memory_receipt(
              7u, DM2_V1_GDAT_PICT_POOL_HIBIG, 0, 1, &pool) &&
              pool.accepted && pool.deallocate &&
              pool.aligned_bytes == 8u,
          "DM2_DEALLOC_HIBIGPOOL aligns odd high-pool deallocation bytes");
    CHECK(dm2_v1_gdat_alloc_pict_buff_receipt(
              13u, 5u, 4u, DM2_V1_GDAT_PICT_POOL_LOBIG, &alloc) &&
              alloc.accepted && !alloc.is_cpx_heap &&
              alloc.row_bytes == 7u && alloc.payload_bytes == 35u &&
              alloc.header_bytes == 6u &&
              alloc.allocation_bytes == 41u &&
              alloc.free_bytes == 41u && alloc.receipt_hash != 0u,
          "DM2_ALLOC_PICT_BUFF uses 4bpp even-width row bytes plus six-byte header");
    CHECK(dm2_v1_gdat_free_pict_buff_receipt(&alloc, &free_receipt) &&
              free_receipt.accepted &&
              free_receipt.freed_pool == DM2_V1_GDAT_PICT_POOL_LOBIG &&
              free_receipt.row_bytes == 7u &&
              free_receipt.free_bytes == 41u,
          "DM2_FREE_PICT_BUFF frees the recomputed low-bigpool byte count");

    CHECK(dm2_v1_gdat_alloc_pict_buff_receipt(
              13u, 5u, 8u, DM2_V1_GDAT_PICT_POOL_FREE, &alloc) &&
              alloc.row_bytes == 13u && alloc.payload_bytes == 65u &&
              alloc.allocation_bytes == 71u,
          "DM2_ALLOC_PICT_BUFF keeps 8bpp row bytes unrounded");
    CHECK(!dm2_v1_gdat_alloc_pict_buff_receipt(
              13u, 5u, 3u, DM2_V1_GDAT_PICT_POOL_LOBIG, &alloc),
          "bitmap allocation receipt rejects unsupported resolutions");

    CHECK(dm2_v1_gdat_alloc_new_bmp_receipt(42u, 13u, 5u, 4u, &alloc) &&
              alloc.accepted && alloc.is_cpx_heap &&
              alloc.pool == DM2_V1_GDAT_PICT_POOL_CPXHEAP &&
              alloc.raw_index == 42u &&
              alloc.row_bytes == 7u &&
              alloc.payload_bytes == 35u &&
              alloc.allocation_bytes == 35u &&
              alloc.free_bytes == 65u,
          "DM2_ALLOC_NEW_BMP allocates CPX payload bytes and preserves GDAT raw index");
    CHECK(dm2_v1_gdat_free_pict_entry_receipt(
              &alloc, 1, 1, 0, &free_receipt) &&
              free_receipt.accepted &&
              free_receipt.used_bigpool_struct_before &&
              free_receipt.freed_pool == DM2_V1_GDAT_PICT_POOL_CPXHEAP &&
              free_receipt.free_bytes == 65u,
          "DM2_FREE_PICT_ENTRY admits matching CPX bitmap header and struct-before route");
    CHECK(dm2_v1_gdat_free_pict_entry_receipt(
              &alloc, 1, 0, 1, &free_receipt) &&
              free_receipt.accepted &&
              free_receipt.removed_from_preserved_list &&
              free_receipt.freed_pool == DM2_V1_GDAT_PICT_POOL_LOBIG,
          "DM2_FREE_PICT_ENTRY records preserved-list unlink before pool free");
    CHECK(!dm2_v1_gdat_free_pict_entry_receipt(
              &alloc, 0, 1, 0, &free_receipt),
          "DM2_FREE_PICT_ENTRY rejects mismatched bitmap headers");

    CHECK(dm2_v1_gdat_cpx_reserve_receipt(100u, 8u, &reserve) &&
              reserve.accepted && reserve.old_wp08_word == 100u &&
              reserve.reserved_words == 4u &&
              reserve.new_wp08_word == 96u &&
              reserve.returned_word == 96u &&
              reserve.receipt_hash != 0u,
          "R_2D8AD reserves CPX bytes by moving wp_08 downward");
    CHECK(!dm2_v1_gdat_cpx_reserve_receipt(3u, 8u, &reserve),
          "R_2D8AD rejects reservations that would underflow wp_08");
    CHECK(dm2_v1_gdat_cpx_copy_receipt(
              100u, 8u, source_payload, sizeof(source_payload), &copy) &&
              copy.accepted && copy.source_header_included &&
              copy.reserve.new_wp08_word == 96u &&
              copy.returned_payload_word == 97u &&
              copy.copied_bytes == 8u &&
              copy.receipt_hash != 0u,
          "R_2D8BA copies the source header span and returns payload word");
    CHECK(!dm2_v1_gdat_cpx_copy_receipt(
              100u, 12u, source_payload, sizeof(source_payload), &copy),
          "R_2D8BA rejects undersized caller-owned source spans");
    CHECK(dm2_v1_gdat_cpx_compact_receipt(
              100u, 80u, cpx_blocks,
              (uint16_t)(sizeof(cpx_blocks) / sizeof(cpx_blocks[0])),
              &compact) &&
              compact.accepted &&
              compact.input_block_count == 3u &&
              compact.preserved_block_count == 2u &&
              compact.skipped_free_block_count == 1u &&
              compact.moved_block_count == 2u &&
              compact.new_wp08_word == 90u &&
              compact.blocks[0].preserved &&
              compact.blocks[0].word_count == 5u &&
              compact.blocks[0].new_start_word == 95u &&
              compact.blocks[1].skipped_free &&
              compact.blocks[2].preserved &&
              compact.blocks[2].word_count == 5u &&
              compact.blocks[2].new_start_word == 90u &&
              compact.receipt_hash != 0u,
          "R_2D802 compacts active CPX GDAT blocks and skips high-bit frees");
    CHECK(!dm2_v1_gdat_cpx_compact_receipt(
              100u, 90u, cpx_blocks,
              (uint16_t)(sizeof(cpx_blocks) / sizeof(cpx_blocks[0])),
              &compact),
          "R_2D802 rejects blocks outside the current CPX pool window");
}

static void test_querydb_word_and_ornate_receipts(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[320];
    uint32_t raw_offsets[6];
    uint32_t raw_sizes[6];
    DM2_V1_GdatEntry entries[21];
    DM2_V1_QueryOrnateAnimFrameReceipt frame;
    DM2_V1_GetOrnateAnimLenReceipt len;
    DM2_V1_GdatWordQueryReceipt word;
    DM2_V1_DoorStrengthReceipt strength;
    DM2_V1_CreaturesItemMaskReceipt mask;
    DM2_V1_ItemFitForEquipReceipt fit;
    DM2_V1_GdatNameReceipt name;
    DM2_V1_CmdstrEntryReceipt cmdstr;
    DM2_V1_CurCmdstrContext curcmd;
    DM2_V1_ItemOrderInContainerReceipt order;
    uint8_t cache3[3] = {0x21u, 0xffu, 0x44u};
    uint16_t money_ids[6] = {0x0100u, 0x0101u, 0x0102u,
                             0x0103u, 0x0104u, 0x0105u};

    memset(&loader, 0, sizeof(loader));
    memset(data, 0, sizeof(data));
    memcpy(data, "2A4", 4u);
    memcpy(data + 8u, "A1-2J0P3S0C1W4", 16u);
    memcpy(data + 32u, "AXE:SK=7LV-2CM=11", 18u);
    data[50u] = 0u;
    memcpy(data + 64u, "BLADE:SK=4SK=8DM=-5HN=12", 26u);
    data[90u] = 0u;
    memcpy(data + 96u, "J0-5", 5u);
    memset(data + 160u, 'A', 128u);
    data[288u] = 0u;
    raw_offsets[0] = 0u;
    raw_sizes[0] = 4u;
    raw_offsets[1] = 8u;
    raw_sizes[1] = 16u;
    raw_offsets[2] = 32u;
    raw_sizes[2] = 19u;
    raw_offsets[3] = 64u;
    raw_sizes[3] = 27u;
    raw_offsets[4] = 96u;
    raw_sizes[4] = 5u;
    raw_offsets[5] = 160u;
    raw_sizes[5] = 129u;

    memset(entries, 0, sizeof(entries));
    entries[0] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WALL_GFX, 7u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x0du, 0u, 0u, 0x8003u};
    entries[1] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_FLOOR_GFX, 8u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x0du, 0u, 0u, 0u};
    entries[2] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 2u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x0fu, 0u, 0u, 0x002au};
    entries[3] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 2u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x11u, 0u, 0u, 0x0033u};
    entries[4] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 3u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x10u, 0u, 0u, 0x0000u};
    entries[5] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 4u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x10u, 0u, 0u, 0x0009u};
    entries[6] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_CREATURES, 5u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x01u, 0u, 0u, 0x0044u};
    entries[7] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_CREATURES, 5u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x05u, 0u, 0u, 0x0055u};
    entries[8] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_MISCELLANEOUS, 6u,
        DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x03u, 0u, 0u, 0x0066u};
    entries[9] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 5u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x11u, 0u, 0u, 0x0000u};
    entries[10] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_CREATURES, 9u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x12u, 0u, 0u, 1u};
    entries[11] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 9u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x04u, 0u, 0u, 0x8444u};
    entries[12] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 10u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x04u, 0u, 0u, 0x0040u};
    entries[13] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 6u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x0du, 0u, 0u, 0x0007u};
    entries[14] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 6u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x0eu, 0u, 0u, 0x0008u};
    entries[15] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 6u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x10u, 0u, 0u, 0x0009u};
    entries[16] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 1u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x18u, 0u, 0u, 2u};
    entries[17] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 2u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x08u, 0u, 0u, 3u};
    entries[18] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 3u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x08u, 0u, 0u, 0x8003u};
    entries[19] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_CONTAINERS, 7u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x40u, 0u, 0u, 4u};
    entries[20] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_WEAPONS, 4u, DM2_GDAT_ENTRY_TYPE_TEXT,
        0x18u, 0u, 0u, 5u};

    loader.data = data;
    loader.data_size = sizeof(data);
    loader.loaded = 1;
    loader.category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader.raw_data_count = 6u;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 21u;

    CHECK(dm2_v1_query_ornate_anim_frame_receipt(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 7, 4u, 1u, &frame) &&
              frame.accepted && frame.used_word_value &&
              frame.length == 3u && frame.frame_base == 1u &&
              frame.frame == 3u,
          "DM2_QUERY_ORNATE_ANIM_FRAME uses source dtWordValue high-bit frame base");
    CHECK(dm2_v1_query_ornate_anim_frame_receipt(
              &loader, DM2_GDAT_CATEGORY_FLOOR_GFX, 8, 2u, 1u, &frame) &&
              frame.accepted && frame.used_text_sequence &&
              frame.length == 3u && frame.frame == 2u,
          "DM2_QUERY_ORNATE_ANIM_FRAME uses source dtText base36 sequence");
    CHECK(dm2_v1_get_ornate_anim_len_receipt(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 7, 0, &len) &&
              len.accepted && len.used_word_value && len.length == 3u,
          "DM2_GET_ORNATE_ANIM_LEN masks the source word high bit");
    CHECK(dm2_v1_get_ornate_anim_len_receipt(
              &loader, DM2_GDAT_CATEGORY_FLOOR_GFX, 8, 0, &len) &&
              len.accepted && len.used_text_sequence && len.length == 3u,
          "DM2_GET_ORNATE_ANIM_LEN counts source dtText frames");
    CHECK(dm2_v1_get_ornate_anim_len_receipt(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 99, 1, &len) &&
              len.accepted && len.decoration_absent && len.length == 1u,
          "DM2_GET_ORNATE_ANIM_LEN preserves decoration-absent length one");

    CHECK(dm2_v1_query_door_damage_resist_receipt(&loader, 6, &word) &&
              word.accepted && word.category == DM2_GDAT_CATEGORY_DOORS &&
              word.field == 0x0eu && word.value == 0x0008u,
          "DM2_QUERY_DOOR_DAMAGE_RESIST binds DOORS GDAT_DOOR_DEFENSE field 0x0e");
    CHECK(dm2_v1_query_door_strength_receipt(&loader, 2, &strength) &&
              strength.accepted && strength.used_explicit_strength &&
              !strength.used_resistance_fallback &&
              strength.strength == 0x002au,
          "DM2_QUERY_DOOR_STRENGTH reads DOORS GDAT_DOOR_STRENGTH field 0x0f");
    CHECK(!dm2_v1_query_door_strength_receipt(&loader, 3, &strength) &&
              !dm2_v1_query_door_strength_receipt(&loader, 4, &strength),
          "DM2_QUERY_DOOR_STRENGTH rejects entries missing source field 0x0f");
    CHECK(!dm2_v1_query_door_strength_receipt(&loader, 5, &strength),
          "DM2_QUERY_DOOR_STRENGTH rejects a missing source strength field");
    CHECK(dm2_v1_get_graphics_for_door_receipt(&loader, 6, &word) &&
              word.accepted && word.field == 0x0du && word.value == 0x0007u,
          "DM2_GET_GRAPHICS_FOR_DOOR binds DOORS dtWordValue field 0x0d");
    CHECK(dm2_v1_get_door_stat_0x10_receipt(&loader, 6, &word) &&
              word.accepted && word.field == 0x10u && word.value == 0x0009u,
          "DM2_GET_DOOR_STAT_0X10 binds DOORS GDAT_DOOR_X10 field 0x10");
    CHECK(dm2_v1_query_0cee_3275_receipt(&loader, 6, &word) &&
              word.accepted && word.field == 0x0du && word.value == 0x0007u,
          "DM2_query_0cee_3275 binds DOORS graphics field 0x0d");
    CHECK(!dm2_v1_get_graphics_for_door_receipt(&loader, 99, &word),
          "DM2_GET_GRAPHICS_FOR_DOOR rejects absent door rows without fallback");
    CHECK(dm2_v1_query_gdat_item_name_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 1, &name) &&
              name.accepted && name.field == 0x18u &&
              strcmp(name.text, "AXE") == 0 &&
              name.text_hash != 0u,
          "DM2_QUERY_GDAT_ITEM_NAME binds real dtText field 0x18 and stops at colon");
    CHECK(!dm2_v1_query_gdat_item_name_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 4, &name) &&
              !name.accepted && name.truncated && name.text[0] == '\0',
          "DM2_QUERY_GDAT_ITEM_NAME rejects over-cap source text without a partial name");
    CHECK(dm2_v1_query_cmdstr_name_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 2, 0x08u, &name) &&
              name.accepted && strcmp(name.text, "BLADE") == 0,
          "DM2_QUERY_CMDSTR_NAME returns the source command string name prefix");
    CHECK(dm2_v1_query_cmdstr_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 2, 0x08u, 0, &cmdstr) &&
              cmdstr.accepted && cmdstr.found &&
              strcmp(cmdstr.key, "SK") == 0 &&
              cmdstr.value == 48,
          "DM2_QUERY_CMDSTR_ENTRY preserves source repeated-key digit accumulation");
    CHECK(dm2_v1_query_cmdstr_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 2, 0x08u, 11, &cmdstr) &&
              cmdstr.accepted && cmdstr.found &&
              strcmp(cmdstr.key, "DM") == 0 &&
              cmdstr.value == -5,
          "DM2_QUERY_CMDSTR_ENTRY parses signed decimal values");
    CHECK(dm2_v1_query_cmdstr_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 2, 0x08u, 17, &cmdstr) &&
              cmdstr.accepted && !cmdstr.found && cmdstr.value == 0,
          "DM2_QUERY_CMDSTR_ENTRY preserves source zero result for absent keys");
    CHECK(!dm2_v1_query_cmdstr_entry_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 2, 0x08u, 18, &cmdstr),
          "DM2_QUERY_CMDSTR_ENTRY rejects key indexes outside table1d6912");
    curcmd.category = DM2_GDAT_CATEGORY_WEAPONS;
    curcmd.index = 2u;
    curcmd.field = 0x08u;
    CHECK(dm2_v1_query_cur_cmdstr_entry_receipt(
              &loader, &curcmd, 15, &cmdstr) &&
              cmdstr.accepted && cmdstr.found &&
              strcmp(cmdstr.key, "HN") == 0 &&
              cmdstr.value == 12,
          "DM2_QUERY_CUR_CMDSTR_ENTRY reuses the caller-owned current command string context");
    CHECK(dm2_v1_get_item_order_in_container_receipt(
              &loader, 7, 5, money_ids, 6u, &order) &&
              order.accepted && order.field == 0x40u &&
              order.enumerated_order == 5u &&
              order.resolved_item_type == 0x0105u &&
              order.money_item_index == 5,
          "DM2_GET_ITEM_ORDER_IN_CONTAINER parses real CONTAINERS text order");
    CHECK(!dm2_v1_get_item_order_in_container_receipt(
              &loader, 7, 6, money_ids, 6u, &order),
          "DM2_GET_ITEM_ORDER_IN_CONTAINER rejects out-of-range order without fallback");
    CHECK(dm2_v1_query_gdat_creature_word_value_receipt(
              &loader, 5, 1, NULL, 0u, &word) &&
              word.accepted && !word.used_cache_byte && word.value == 0x0044u,
          "DM2_QUERY_GDAT_CREATURE_WORD_VALUE reads source creature word field");
    CHECK(dm2_v1_query_gdat_creature_word_value_receipt(
              &loader, 5, 5, cache3, sizeof(cache3), &word) &&
              word.accepted && word.used_cache_byte && word.value == 0x0044u,
          "DM2_QUERY_GDAT_CREATURE_WORD_VALUE admits populated source cache slot");
    CHECK(dm2_v1_query_gdat_food_value_from_record_receipt(
              &loader, DM2_GDAT_CATEGORY_MISCELLANEOUS, 6, &word) &&
              word.accepted && word.field == 0x03u && word.value == 0x0066u,
          "DM2_QUERY_GDAT_FOOD_VALUE_FROM_RECORD binds DB spec word field 0x03");
    CHECK(dm2_v1_query_creatures_item_mask_receipt(
              &loader, 9, 2, 0, &mask) &&
              mask.accepted && mask.field == 0x12u &&
              mask.set_bits == 7u &&
              (mask.mask[129u / 8u] & (1u << (129u & 7u))) &&
              (mask.mask[130u / 8u] & (1u << (130u & 7u))) &&
              (mask.mask[256u / 8u] & (1u << (256u & 7u))) &&
              (mask.mask[387u / 8u] & (1u << (387u & 7u))) &&
              (mask.mask[508u / 8u] & (1u << (508u & 7u))) &&
              (mask.mask[481u / 8u] & (1u << (481u & 7u))) &&
              (mask.mask[4u / 8u] & (1u << (4u & 7u))),
          "DM2_QUERY_CREATURES_ITEM_MASK parses source dtText equipment ranges");
    CHECK(dm2_v1_query_creatures_item_mask_receipt(
              &loader, 9, 2, 1, &mask) &&
              mask.accepted && mask.creature_route &&
              (mask.mask[1u / 8u] & (1u << (1u & 7u))),
          "DM2_QUERY_CREATURES_ITEM_MASK maps C ranges to creature-local base");
    CHECK(dm2_v1_is_item_fit_for_equip_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 9, 3, 1, -1, &fit) &&
              fit.accepted && fit.only_body_part &&
              fit.tested_mask == 0x0004u && fit.result == 0x0004u,
          "DM2_IS_ITEM_FIT_FOR_EQUIP tests source body-part mask");
    CHECK(dm2_v1_is_item_fit_for_equip_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 9, 30, 0, -1, &fit) &&
              fit.accepted && fit.result == 0u,
          "DM2_IS_ITEM_FIT_FOR_EQUIP blocks over slots when high flag is set");
    CHECK(dm2_v1_is_item_fit_for_equip_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 10, 30, 0, 0, &fit) &&
              fit.accepted && fit.used_active_hand_result &&
              fit.result == 1u,
          "DM2_IS_ITEM_FIT_FOR_EQUIP over slot admits empty active-hand result");
    CHECK(dm2_v1_is_item_fit_for_equip_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 10, 30, 0, 1, &fit) &&
              fit.accepted && fit.used_active_hand_result &&
              fit.result == 0x0040u,
          "DM2_IS_ITEM_FIT_FOR_EQUIP over slot tests source flag 0x40 after hand fit");
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *dm2_data = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    const char *leaf;

    if (!out || out_size == 0u || !suffix) return 0;
    /* FIRESTAFF_DM2_DATA_DIR is the direct selected data directory, unlike
     * FIRESTAFF_DATA which is the shared root containing dm2/.  Keep the
     * real-media census usable for an installed archive without copying or
     * unpacking GRAPHICS.DAT for a test. */
    if (dm2_data && dm2_data[0]) {
        leaf = strrchr(suffix, '/');
        snprintf(out, out_size, "%s/%s", dm2_data, leaf ? leaf + 1 : suffix);
        return 1;
    }
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size)
{
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    char path[1024];
    size_t i;

    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, sizeof(path), suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_real_graphics_census(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEnt1Receipt ent1_receipt;
    DM2_V1_GdatLoadEntriesReceipt load_entries_receipt;
    DM2_V1_GraphicsStructureReceipt structure_receipt;
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt iter_receipt;
    DM2_V1_GdatGfxMaterialReceipt material;
    DM2_V1_GdatNameReceipt item_name;
    unsigned int loadable_count = 0u;
    unsigned int scalar_count = 0u;
    unsigned int gfx_material_count = 0u;
    uint32_t receipt_hash = 2166136261u;

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&graphics, &graphics_size)) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return;
    }
    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real GRAPHICS.DAT initializes querydb receipt loader");
    if (!loader.loaded) {
        free(graphics);
        return;
    }
    CHECK(dm2_v1_read_graphics_structure_receipt(&loader, &structure_receipt) &&
              structure_receipt.valid &&
              (structure_receipt.source_image_cache_limit == 0x001fu ||
               structure_receipt.source_image_cache_limit == 0x03e8u),
          "real GRAPHICS.DAT retains its source startup allocator mode");
    CHECK(dm2_v1_query_gdat_item_name_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 0, &item_name) &&
              item_name.accepted && !item_name.truncated &&
              strcmp(item_name.text, "EYE OF TIME") == 0,
          "real GDAT item-name receipt preserves WEAPONS/0 dtText/0x18 in full");
    CHECK(dm2_v1_query_gdat_item_name_receipt(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 3, &item_name) &&
              item_name.accepted && !item_name.truncated &&
              strcmp(item_name.text, "KALAN GAUNTLET") == 0,
          "real GDAT item-name receipt preserves the complete longest PC weapon name");

    for (uint16_t i = 0u; i < loader.entry_count; ++i) {
        DM2_V1_GdatEntryQueryReceipt receipt;
        const DM2_V1_GdatEntry *entry = &loader.entries[i];
        if (!dm2_v1_query_gdat_entry(&loader, entry->cls1, entry->cls2,
                                     entry->cls3, entry->cls4, &receipt)) {
            continue;
        }
        if (receipt.loadable_raw) ++loadable_count;
        else ++scalar_count;
        receipt_hash = (receipt_hash ^ receipt.receipt_hash) * 16777619u;
    }

    for (uint16_t i = 0u; i < loader.entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader.entries[i];
        const uint8_t *source_bytes;
        size_t source_byte_count = 0u;
        if (entry->cls3 != DM2_GDAT_ENTRY_TYPE_IMAGE ||
            !dm2_v1_gdat_allocate_gfx16_material_receipt(
                &loader, entry->cls1, entry->cls2, entry->cls4, 1,
                &material)) {
            continue;
        }
        source_bytes = dm2_v1_load_gdat_raw_data(&loader, material.raw_index,
                                                  &source_byte_count);
        CHECK(material.accepted && !material.used_gfx16_default &&
                  source_bytes == material.source_bytes &&
                  source_byte_count == material.source_byte_count &&
                  dm2_v1_gdat_allocate_gfx256_material_receipt(
                      &loader, material.raw_index, 1, &material) &&
                  material.accepted && material.source_bytes == source_bytes,
              "real GDAT GFX16/GFX256 material routes retain loaded source bytes");
        ++gfx_material_count;
        break;
    }

    CHECK(dm2_v1_load_ent1_receipt(&loader, &ent1_receipt) &&
              ent1_receipt.valid &&
              ent1_receipt.entry_count == loader.entry_count &&
              ent1_receipt.ep_present[0] &&
              ent1_receipt.ep_present[1] &&
              ent1_receipt.ep_present[2] &&
              ent1_receipt.ep_present[3] &&
              ent1_receipt.ep_present[4] &&
              ent1_receipt.receipt_hash != 0u,
          "real GDAT LOAD_ENT1 receipt binds raw0 tag layout and entry count");
    CHECK(dm2_v1_load_gdat_entries_receipt(&loader, &load_entries_receipt) &&
              load_entries_receipt.valid &&
              load_entries_receipt.entry_count == loader.entry_count &&
              load_entries_receipt.loadable_entry_count > 100u &&
              load_entries_receipt.payload_bytes > 100000u &&
              load_entries_receipt.allocated_bytes_with_length_words >
                  load_entries_receipt.payload_bytes,
          "real GDAT LOAD_GDAT_ENTRIES receipt accounts raw payload preload bytes");
    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    iterator.category_last = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    iterator.index_filter = -1;
    iterator.type_filter = DM2_GDAT_ENTRY_TYPE_PAL_IRGB;
    iterator.field_filter = DM2_GDAT_INTERFACE_PALETTE_FIELD;
    CHECK(dm2_v1_query_next_gdat_entry(&loader, &iterator, &iter_receipt) &&
              iter_receipt.category == DM2_GDAT_CATEGORY_INTERFACE_GENERAL &&
              iter_receipt.type == DM2_GDAT_ENTRY_TYPE_PAL_IRGB &&
              iter_receipt.field == DM2_GDAT_INTERFACE_PALETTE_FIELD &&
              iter_receipt.loadable_raw,
          "real GDAT QUERY_NEXT_GDAT_ENTRY finds the interface IRGB palette row");

    CHECK(loadable_count > 100u,
          "real GDAT exposes many loadable querydb raw entries");
    CHECK(scalar_count > 20u,
          "real GDAT exposes scalar data-index entries without buffers");
    CHECK(receipt_hash != 0u,
          "real GDAT querydb census has nonzero receipt hash");
    CHECK(gfx_material_count == 1u,
          "real GDAT exposes a source-owned GFX16/GFX256 material route");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

int main(void)
{
    printf("DM2 V1 GDAT querydb receipts\n");
    test_fixture_entry_queries();
    test_fixture_gdat_entry_iteration_and_sound();
    test_graphics_structure_and_image_extract();
    test_gfx_material_ownership_routes();
    test_graphics_data_file_lifecycle();
    test_fixture_pict_allocation_receipts();
    test_querydb_word_and_ornate_receipts();
    test_real_graphics_census();
    printf("Results: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
