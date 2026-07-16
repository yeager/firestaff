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
                           uint8_t data[128],
                           uint32_t raw_offsets[4],
                           uint32_t raw_sizes[4],
                           DM2_V1_GdatEntry entries[5])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, 128u);
    memcpy(data + 16u, "IMGRAW", 6u);
    memcpy(data + 32u, "TEXT", 4u);
    memcpy(data + 48u, "PAL0123456789ABC", 16u);
    memcpy(data + 80u, "\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa", 10u);
    raw_offsets[0] = 16u;
    raw_offsets[1] = 32u;
    raw_offsets[2] = 48u;
    raw_offsets[3] = 80u;
    raw_sizes[0] = 6u;
    raw_sizes[1] = 4u;
    raw_sizes[2] = 16u;
    raw_sizes[3] = 10u;
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
    loader->data = data;
    loader->data_size = 128u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 4u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 5u;
}

static void test_fixture_entry_queries(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[128];
    uint32_t raw_offsets[4];
    uint32_t raw_sizes[4];
    DM2_V1_GdatEntry entries[5];
    DM2_V1_GdatEntryQueryReceipt receipt;
    const uint8_t *ptr;
    size_t size = 0u;
    uint32_t u32 = 0u;
    uint32_t value = 0u;
    uint16_t u16 = 0u;
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
}

static void test_fixture_gdat_entry_iteration_and_sound(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[128];
    uint8_t sample[4] = {0x00u, 0x7fu, 0x80u, 0xffu};
    uint32_t raw_offsets[4];
    uint32_t raw_sizes[4];
    DM2_V1_GdatEntry entries[5];
    DM2_V1_GdatLoadEntriesReceipt load_receipt;
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry_receipt;
    DM2_V1_GdatSoundToggleReceipt toggle;
    DM2_V1_GdatSoundEntryReceipt sound_receipt;

    fixture_loader(&loader, data, raw_offsets, raw_sizes, entries);

    CHECK(dm2_v1_load_gdat_entries_receipt(&loader, &load_receipt) &&
              load_receipt.valid &&
              load_receipt.entry_count == 5u &&
              load_receipt.loadable_entry_count == 3u &&
              load_receipt.scalar_entry_count == 2u &&
              load_receipt.payload_bytes == 20u &&
              load_receipt.allocated_bytes_with_length_words == 26u,
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
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
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
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt iter_receipt;
    unsigned int loadable_count = 0u;
    unsigned int scalar_count = 0u;
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

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

int main(void)
{
    printf("DM2 V1 GDAT querydb receipts\n");
    test_fixture_entry_queries();
    test_fixture_gdat_entry_iteration_and_sound();
    test_graphics_data_file_lifecycle();
    test_fixture_pict_allocation_receipts();
    test_real_graphics_census();
    printf("Results: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
