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
                           uint8_t data[96],
                           uint32_t raw_offsets[3],
                           uint32_t raw_sizes[3],
                           DM2_V1_GdatEntry entries[4])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, 96u);
    memcpy(data + 16u, "IMGRAW", 6u);
    memcpy(data + 32u, "TEXT", 4u);
    memcpy(data + 48u, "PAL0123456789ABC", 16u);
    raw_offsets[0] = 16u;
    raw_offsets[1] = 32u;
    raw_offsets[2] = 48u;
    raw_sizes[0] = 6u;
    raw_sizes[1] = 4u;
    raw_sizes[2] = 16u;
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
    loader->data = data;
    loader->data_size = 96u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 3u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 4u;
}

static void test_fixture_entry_queries(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[96];
    uint32_t raw_offsets[3];
    uint32_t raw_sizes[3];
    DM2_V1_GdatEntry entries[4];
    DM2_V1_GdatEntryQueryReceipt receipt;
    const uint8_t *ptr;
    size_t size = 0u;
    uint32_t u32 = 0u;
    uint16_t u16 = 0u;

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
    test_fixture_pict_allocation_receipts();
    test_real_graphics_census();
    printf("Results: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
