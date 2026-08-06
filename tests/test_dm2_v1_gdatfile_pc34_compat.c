/*
 * test_dm2_v1_gdatfile_pc34_compat.c — Tests for DM2 GDAT file handling.
 *
 * Tests the callback-based GDAT file operations ported from skproject
 * c_gdatfile.cpp.
 */

#include "dm2_v1_gdatfile_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  %-60s ", #name); \
    if (test_##name()) { tests_passed++; printf("PASS\n"); } \
    else { printf("FAIL\n"); } \
} while (0)

/* ======================================================================== */
/* Mock callbacks                                                           */
/* ======================================================================== */

typedef struct MockCtx {
    int open_count;
    int close_count;
    int read_count;
    int seek_count;
    int alloc_count;
    int dealloc_count;
    int syserr_code;
    int16_t last_handle;
    int32_t last_seek_pos;
    uint8_t file_data[256];
    int32_t file_data_len;
    uint8_t alloc_buf[1024];
} MockCtx;

static int16_t mock_file_open(void *ctx, const char *name)
{
    MockCtx *m = (MockCtx *)ctx;
    m->open_count++;
    (void)name;
    return 42;
}

static void mock_file_close(void *ctx, int16_t handle)
{
    MockCtx *m = (MockCtx *)ctx;
    m->close_count++;
    (void)handle;
}

static bool mock_file_read(void *ctx, int16_t handle, void *buf, int32_t len)
{
    MockCtx *m = (MockCtx *)ctx;
    m->read_count++;
    (void)handle;
    if (len > m->file_data_len) return false;
    memcpy(buf, m->file_data + m->last_seek_pos, (size_t)len);
    return true;
}

static bool mock_file_seek(void *ctx, int16_t handle, int32_t pos)
{
    MockCtx *m = (MockCtx *)ctx;
    m->seek_count++;
    m->last_seek_pos = pos;
    (void)handle;
    return true;
}

static int32_t mock_get_file_size(void *ctx, int16_t handle)
{
    MockCtx *m = (MockCtx *)ctx;
    (void)handle;
    return m->file_data_len;
}

static uint8_t *mock_alloc_memory(void *ctx, int32_t size, int16_t pool)
{
    MockCtx *m = (MockCtx *)ctx;
    m->alloc_count++;
    (void)pool;
    if (size > (int32_t)sizeof(m->alloc_buf)) return NULL;
    return m->alloc_buf;
}

static void mock_dealloc_memory(void *ctx, int32_t size)
{
    MockCtx *m = (MockCtx *)ctx;
    m->dealloc_count++;
    (void)size;
}

static void mock_raise_syserr(void *ctx, int32_t code)
{
    MockCtx *m = (MockCtx *)ctx;
    m->syserr_code = code;
}

static int32_t mock_query_raw_data_length(void *ctx, int16_t dbidx)
{
    (void)ctx;
    /* Each entry is 10 bytes for testing */
    (void)dbidx;
    return 10;
}

static DM2_V1_GdatFileCallbacks make_mock_callbacks(void)
{
    DM2_V1_GdatFileCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.file_open = mock_file_open;
    cb.file_close = mock_file_close;
    cb.file_read = mock_file_read;
    cb.file_seek = mock_file_seek;
    cb.get_file_size = mock_get_file_size;
    cb.alloc_memory = mock_alloc_memory;
    cb.dealloc_memory = mock_dealloc_memory;
    cb.raise_syserr = mock_raise_syserr;
    cb.query_raw_data_length = mock_query_raw_data_length;
    return cb;
}

/* ======================================================================== */
/* Tests                                                                    */
/* ======================================================================== */

static int test_file_init_sets_defaults(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileInitReceipt receipt;

    int r = dm2_v1_gdat_file_init(&state, &receipt);
    if (!r) return 0;
    if (!receipt.valid) return 0;
    if (state.fileopencounter != 0) return 0;
    if (state.filehandle != 0) return 0;
    if (state.filesize != 0) return 0;
    if (state.versionlo != 0) return 0;
    if (state.entries != 0) return 0;
    if (state.filetype1 != false) return 0;
    if (state.filetype2 != false) return 0;
    return 1;
}

static int test_file_init_sets_filenames(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileInitReceipt receipt;

    dm2_v1_gdat_file_init(&state, &receipt);
    if (strcmp(state.filename1, ".Z020GRAPHICS.DAT") != 0) return 0;
    if (strcmp(state.filename2, ".Z026GRAPHIC2.DAT") != 0) return 0;
    if (strcmp(state.filename3, ".Z020DUNGENB.DAT") != 0) return 0;
    if (strcmp(state.filename4, ".Z020DUNGEON.DAT") != 0) return 0;
    if (strcmp(state.filename5, ".Z022SKSAVE.Z023.DAT") != 0) return 0;
    if (strcmp(state.filename6, ".Z022SKSAVE.Z023.BAK") != 0) return 0;
    if (strcmp(state.filename7, ".Z020DUNGEON.FTL") != 0) return 0;
    return 1;
}

static int test_file_init_null_state_fails(void)
{
    DM2_V1_GdatFileInitReceipt receipt;
    return dm2_v1_gdat_file_init(NULL, &receipt) == 0;
}

static int test_file_init_null_receipt_ok(void)
{
    DM2_V1_GdatFileState state;
    return dm2_v1_gdat_file_init(&state, NULL) == 1;
}

static int test_open_increments_counter(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);
    DM2_V1_GdatOpenReceipt receipt;

    dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, &receipt);
    if (receipt.open_count != 1) return 0;
    if (!receipt.opened) return 0;
    if (mctx.open_count != 1) return 0;
    if (receipt.filehandle != 42) return 0;

    /* Second open should not call file_open again */
    dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, &receipt);
    if (receipt.open_count != 2) return 0;
    if (mctx.open_count != 1) return 0;
    return 1;
}

static int test_close_decrements_counter(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);

    /* Open twice, then close twice */
    dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, NULL);
    dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, NULL);

    DM2_V1_GdatCloseReceipt cr;
    dm2_v1_gdat_graphics_data_close(&state, &cb, &mctx, &cr);
    if (cr.closed) return 0; /* counter=1, not yet zero */
    if (mctx.close_count != 0) return 0;

    dm2_v1_gdat_graphics_data_close(&state, &cb, &mctx, &cr);
    if (!cr.closed) return 0;
    if (mctx.close_count != 1) return 0;
    return 1;
}

static int test_read_single_file(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);
    state.filehandle = 42;

    /* Set up file data */
    for (int i = 0; i < 64; i++) mctx.file_data[i] = (uint8_t)(i + 1);
    mctx.file_data_len = 64;

    uint8_t buf[32];
    memset(buf, 0, sizeof(buf));

    DM2_V1_GdatReadReceipt rr;
    int r = dm2_v1_gdat_graphics_data_read(&state, 0, 16, buf, &cb, &mctx, &rr);
    if (!r) return 0;
    if (!rr.success) return 0;
    if (rr.bytes_read != 16) return 0;
    /* Verify data was read from offset 0 */
    if (buf[0] != 1 || buf[15] != 16) return 0;
    return 1;
}

static int test_byte_swap_16(void)
{
    if (dm2_v1_gdat_byte_swap_16(0x0102) != 0x0201) return 0;
    if (dm2_v1_gdat_byte_swap_16(0x00FF) != (int16_t)0xFF00) return 0;
    if (dm2_v1_gdat_byte_swap_16(0) != 0) return 0;
    return 1;
}

static int test_entry_value_single_byte(void)
{
    /* entry_data: 3 records of 4 bytes each, 1 field of 1 byte at offset 2 */
    uint8_t data[12] = { 0,0,0xAA,0, 0,0,0xBB,0, 0,0,0xCC,0 };
    int16_t offsets[1] = { 2 };
    uint8_t sizes[1] = { 1 };

    DM2_V1_GdatEntryValueReceipt out;
    int r = dm2_v1_gdat_query_entry_value(data, 0, 0, offsets, sizes, 4, &out);
    if (!r || !out.valid) return 0;
    if (out.value != 0xAA) return 0;

    r = dm2_v1_gdat_query_entry_value(data, 1, 0, offsets, sizes, 4, &out);
    if (!r || out.value != 0xBB) return 0;

    r = dm2_v1_gdat_query_entry_value(data, 2, 0, offsets, sizes, 4, &out);
    if (!r || out.value != 0xCC) return 0;
    return 1;
}

static int test_entry_value_two_bytes(void)
{
    /* 2-byte big-endian field at offset 0 */
    uint8_t data[4] = { 0x12, 0x34, 0, 0 };
    int16_t offsets[1] = { 0 };
    uint8_t sizes[1] = { 2 };

    DM2_V1_GdatEntryValueReceipt out;
    dm2_v1_gdat_query_entry_value(data, 0, 0, offsets, sizes, 4, &out);
    if (out.value != 0x1234) return 0;
    return 1;
}

static int test_raw_data_file_pos(void)
{
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    DM2_V1_GdatRawDataFilePosReceipt out;
    /* base=100, dbidx=3, each entry=10 bytes, no cache */
    int r = dm2_v1_gdat_raw_data_file_pos(3, 100, 0, 0, &cb, &mctx, &out);
    if (!r) return 0;
    /* Position should be 100 + 10 + 10 + 10 = 130 */
    if (out.position != 130) return 0;
    if (out.cached_idx != 3) return 0;
    if (out.cached_offset != 30) return 0;
    return 1;
}

static int test_raw_data_file_pos_with_cache(void)
{
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    DM2_V1_GdatRawDataFilePosReceipt out;
    /* base=100, dbidx=5, cached at idx=3 offset=30 */
    int r = dm2_v1_gdat_raw_data_file_pos(5, 100, 3, 30, &cb, &mctx, &out);
    if (!r) return 0;
    /* Position should be 100 + 30 + 10 + 10 = 150 */
    if (out.position != 150) return 0;
    return 1;
}

static int test_alloc_pict_buff_8bpp(void)
{
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    DM2_V1_GdatAllocPictReceipt out;
    int r = dm2_v1_gdat_alloc_pict_buff(32, 16, 0, 8, &cb, &mctx, &out);
    if (!r) return 0;
    if (!out.bmp) return 0;
    if (out.width != 32) return 0;
    if (out.height != 16) return 0;
    if (out.bpp != 8) return 0;
    /* Buffer = 32*16 + 6 = 518 */
    if (out.buffer_size != 518) return 0;
    /* Verify header at bmp-6 */
    uint8_t *hdr = (uint8_t *)out.bmp - 6;
    if (hdr[0] != 8) return 0;  /* bpp */
    if (hdr[1] != 0) return 0;  /* unused */
    return 1;
}

static int test_alloc_pict_buff_4bpp(void)
{
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    DM2_V1_GdatAllocPictReceipt out;
    /* 4bpp: row_bytes = (33+1 & 0xfffe) >> 1 = 17 */
    int r = dm2_v1_gdat_alloc_pict_buff(33, 10, 0, 4, &cb, &mctx, &out);
    if (!r) return 0;
    if (out.bpp != 4) return 0;
    /* row = ((33+1)&0xfffe)>>1 = 17, total = 17*10 + 6 = 176 */
    if (out.buffer_size != 176) return 0;
    return 1;
}

static int test_free_pict_buff(void)
{
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    int r = dm2_v1_gdat_free_pict_buff(32, 16, 8, &cb, &mctx);
    if (!r) return 0;
    if (mctx.dealloc_count != 1) return 0;
    return 1;
}

static int test_query_next_entry_simple(void)
{
    /* Build a minimal GDAT table with one category, one subcategory, one entry */
    int16_t t1[2] = { 0, 1 };    /* category 0: subcats [0..1) */
    int16_t t2[2] = { 0, 1 };    /* subcat 0: entries [0..1) */
    DM2_V1_GdatBBW entries[1] = { { 0x05, 0x03, 0x0042 } };

    DM2_V1_GdatTable table;
    memset(&table, 0, sizeof(table));
    table.w_table1 = t1;
    table.w_table2 = t2;
    table.u31p_08 = entries;
    table.entries = 0; /* max category index */

    DM2_V1_GdatQueryState qs;
    memset(&qs, 0, sizeof(qs));
    qs.l_00 = 1;
    qs.s19_04.w_00 = 0;
    qs.s19_04.b_02 = (int8_t)-1;  /* all categories */
    qs.s19_04.b_03 = (int8_t)-1;  /* all types */
    qs.s19_04.b_04 = (int8_t)-1;  /* all subcategories */
    qs.s19_04.b_05 = (int8_t)-1;  /* all subtypes */

    DM2_V1_GdatQueryNextEntryReceipt out;
    int r = dm2_v1_gdat_query_next_entry(&qs, &table, &out);
    if (!r) return 0;
    if (!out.found) return 0;
    if (!out.entry) return 0;
    if (out.entry->b_00 != 0x05) return 0;
    if (out.entry->b_01 != 0x03) return 0;
    if (out.entry->w_02 != 0x0042) return 0;
    return 1;
}

static int test_query_next_entry_no_match(void)
{
    /* Empty table */
    int16_t t1[2] = { 0, 0 };
    int16_t t2[1] = { 0 };

    DM2_V1_GdatTable table;
    memset(&table, 0, sizeof(table));
    table.w_table1 = t1;
    table.w_table2 = t2;
    table.u31p_08 = NULL;
    table.entries = 0;

    DM2_V1_GdatQueryState qs;
    memset(&qs, 0, sizeof(qs));
    qs.l_00 = 1;
    qs.s19_04.b_02 = (int8_t)-1;
    qs.s19_04.b_03 = (int8_t)-1;
    qs.s19_04.b_04 = (int8_t)-1;
    qs.s19_04.b_05 = (int8_t)-1;

    DM2_V1_GdatQueryNextEntryReceipt out;
    dm2_v1_gdat_query_next_entry(&qs, &table, &out);
    if (out.found) return 0;
    if (out.entry != NULL) return 0;
    return 1;
}

static int test_query_next_entry_filtered(void)
{
    /* Two entries: (type=3,sub=1) and (type=5,sub=2) */
    int16_t t1[2] = { 0, 1 };
    int16_t t2[2] = { 0, 2 };
    DM2_V1_GdatBBW entries[2] = {
        { 0x03, 0x01, 0x0010 },
        { 0x05, 0x02, 0x0020 }
    };

    DM2_V1_GdatTable table;
    memset(&table, 0, sizeof(table));
    table.w_table1 = t1;
    table.w_table2 = t2;
    table.u31p_08 = entries;
    table.entries = 0;

    /* Query for type=5 only */
    DM2_V1_GdatQueryState qs;
    memset(&qs, 0, sizeof(qs));
    qs.l_00 = 1;
    qs.s19_04.w_00 = 0;
    qs.s19_04.b_02 = (int8_t)-1;
    qs.s19_04.b_03 = 0x05;        /* match type 5 */
    qs.s19_04.b_04 = (int8_t)-1;
    qs.s19_04.b_05 = (int8_t)-1;

    DM2_V1_GdatQueryNextEntryReceipt out;
    dm2_v1_gdat_query_next_entry(&qs, &table, &out);
    if (!out.found) return 0;
    if (out.entry->w_02 != 0x0020) return 0;
    return 1;
}

static int test_load_raw_data(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);

    /* Fill file data */
    for (int i = 0; i < 32; i++) mctx.file_data[i] = (uint8_t)(0xA0 + i);
    mctx.file_data_len = 32;

    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));

    DM2_V1_GdatLoadRawDataReceipt out;
    int r = dm2_v1_gdat_load_raw_data(&state, 1, buf, 16, 0, &cb, &mctx, &out);
    if (!r) return 0;
    if (!out.success) return 0;
    if (out.bytes_loaded != 16) return 0;
    if (buf[0] != 0xA0) return 0;
    if (buf[15] != 0xAF) return 0;
    return 1;
}

static int test_read_graphics_structure_unimplemented_fails_closed(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);
    state.entries = 42;
    state.versionlo = 5;

    DM2_V1_GdatReadStructureReceipt out;
    int r = dm2_v1_gdat_read_graphics_structure(&state, &cb, &mctx, &out);
    return r == 0 && !out.valid && out.entries == 0 && out.versionlo == 0;
}

static int test_struct_sizes(void)
{
    /* Verify packed struct sizes match skproject */
    if (sizeof(DM2_V1_GdatHex6) != 6) return 0;
    if (sizeof(DM2_V1_GdatBBW) != 4) return 0;
    return 1;
}

static int test_open_dual_file(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);
    state.filetype1 = false;
    state.filetype2 = true;

    DM2_V1_GdatOpenReceipt out;
    dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, &out);
    if (!out.opened) return 0;
    /* Should have opened both files */
    if (mctx.open_count != 2) return 0;
    return 1;
}

/* ======================================================================== */
/* Main                                                                     */
/* ======================================================================== */

int main(void)
{
    printf("dm2_v1_gdatfile_pc34_compat tests\n");
    printf("==================================\n");

    TEST(file_init_sets_defaults);
    TEST(file_init_sets_filenames);
    TEST(file_init_null_state_fails);
    TEST(file_init_null_receipt_ok);
    TEST(struct_sizes);
    TEST(byte_swap_16);
    TEST(open_increments_counter);
    TEST(close_decrements_counter);
    TEST(open_dual_file);
    TEST(read_single_file);
    TEST(entry_value_single_byte);
    TEST(entry_value_two_bytes);
    TEST(raw_data_file_pos);
    TEST(raw_data_file_pos_with_cache);
    TEST(alloc_pict_buff_8bpp);
    TEST(alloc_pict_buff_4bpp);
    TEST(free_pict_buff);
    TEST(query_next_entry_simple);
    TEST(query_next_entry_no_match);
    TEST(query_next_entry_filtered);
    TEST(load_raw_data);
    TEST(read_graphics_structure_unimplemented_fails_closed);

    printf("----------------------------------\n");
    printf("Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
