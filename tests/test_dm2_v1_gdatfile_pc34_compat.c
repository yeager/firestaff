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

static int16_t mock_file_open_fail_first(void *ctx, const char *name)
{
    MockCtx *m = (MockCtx *)ctx;
    m->open_count++;
    (void)name;
    return -1;
}

static int16_t mock_file_open_fail_second(void *ctx, const char *name)
{
    MockCtx *m = (MockCtx *)ctx;
    m->open_count++;
    (void)name;
    return m->open_count == 2 ? -1 : 42;
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

typedef struct RealGdatCtx {
    FILE *file;
    char path[1024];
    uint8_t *allocation;
} RealGdatCtx;

static int16_t real_gdat_open(void *ctx, const char *name)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)name;
    real->file = fopen(real->path, "rb");
    return real->file ? 7 : -1;
}

static void real_gdat_close(void *ctx, int16_t handle)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)handle;
    if (real->file) {
        fclose(real->file);
        real->file = NULL;
    }
}

static bool real_gdat_read(void *ctx, int16_t handle, void *buf, int32_t len)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)handle;
    return real->file && len >= 0 && fread(buf, 1u, (size_t)len, real->file) ==
        (size_t)len;
}

static bool real_gdat_seek(void *ctx, int16_t handle, int32_t pos)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)handle;
    return real->file && fseek(real->file, (long)pos, SEEK_SET) == 0;
}

static int32_t real_gdat_size(void *ctx, int16_t handle)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    long saved;
    long end;
    (void)handle;
    if (!real->file) return 0;
    saved = ftell(real->file);
    if (fseek(real->file, 0L, SEEK_END) != 0) return 0;
    end = ftell(real->file);
    (void)fseek(real->file, saved, SEEK_SET);
    return end > 0L ? (int32_t)end : 0;
}

static uint8_t *real_gdat_alloc(void *ctx, int32_t size, int16_t pool)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)pool;
    real->allocation = size > 0 ? (uint8_t *)malloc((size_t)size) : NULL;
    return real->allocation;
}

static void real_gdat_dealloc(void *ctx, int32_t size)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)size;
    free(real->allocation);
    real->allocation = NULL;
}

static const char *real_gdat_path(void *ctx, const char *name)
{
    RealGdatCtx *real = (RealGdatCtx *)ctx;
    (void)name;
    return real->path;
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

static int test_read_graphics_structure_binds_header_and_ulp(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    memset(&mctx, 0, sizeof(mctx));

    dm2_v1_gdat_file_init(&state, NULL);
    /* version=5|0x8000, entries=2, first ENT1 size=28, ULP offsets={0,0}. */
    mctx.file_data[0] = 0x05;
    mctx.file_data[1] = 0x80;
    mctx.file_data[2] = 0x02;
    mctx.file_data[3] = 0x00;
    mctx.file_data[4] = 0x1c;
    mctx.file_data[5] = 0x00;
    mctx.file_data[6] = 0x00;
    mctx.file_data[7] = 0x00;
    mctx.file_data[8] = 0x00;
    mctx.file_data[9] = 0x00;
    mctx.file_data[10] = 0x01; mctx.file_data[11] = 0x80;
    mctx.file_data[12] = 0x01; mctx.file_data[13] = 0x00;
    mctx.file_data[14] = 0x07; mctx.file_data[15] = 0x00;
    mctx.file_data[16] = 'T'; mctx.file_data[17] = 1;
    mctx.file_data[18] = 'I'; mctx.file_data[19] = 1;
    mctx.file_data[20] = 'D'; mctx.file_data[21] = 1;
    mctx.file_data[22] = 'S'; mctx.file_data[23] = 1;
    mctx.file_data[24] = 'F'; mctx.file_data[25] = 1;
    mctx.file_data[26] = 'G'; mctx.file_data[27] = 1;
    mctx.file_data[28] = 'P'; mctx.file_data[29] = 2;
    mctx.file_data_len = 38;

    DM2_V1_GdatReadStructureReceipt out;
    int r = dm2_v1_gdat_read_graphics_structure(&state, &cb, &mctx, &out);
    if (!(r == 1 && out.valid && out.header_validated && out.ulp_validated &&
           out.entries == 2 && out.versionlo == 5 && out.ulp_length == 4 &&
           out.ulp_table_end == 10 && out.first_entry_size == 28 &&
           out.source_data_offset == 10 && out.first_raw_offset == 10 &&
           out.raw_data_end == 38 && out.ent1_validated &&
           out.ent1_entry_count == 1 && out.ent1_group_count == 7 &&
           out.ent1_stride == 8 &&
           mctx.close_count == 1 && state.ulp_table != NULL &&
           state.ulp_count == 2 && state.ulp_length == 4)) {
        return 0;
    }
    if (!dm2_v1_gdat_release_graphics_structure(&state, &cb, &mctx) ||
        state.ulp_table != NULL) return 0;
    return 1;
}

static int test_read_graphics_structure_accepts_big_endian_header_and_ulp(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    DM2_V1_GdatReadStructureReceipt out;

    memset(&mctx, 0, sizeof(mctx));
    dm2_v1_gdat_file_init(&state, NULL);
    /* 68k order: version=5|0x8000, entries=2, first ENT1 size=28,
     * ULP offsets={0,0}. Keep the same source boundary as the LE fixture. */
    mctx.file_data[0] = 0x80;
    mctx.file_data[1] = 0x05;
    mctx.file_data[2] = 0x00;
    mctx.file_data[3] = 0x02;
    mctx.file_data[4] = 0x00;
    mctx.file_data[5] = 0x00;
    mctx.file_data[6] = 0x00;
    mctx.file_data[7] = 0x1c;
    mctx.file_data[8] = 0x00;
    mctx.file_data[9] = 0x00;
    mctx.file_data[10] = 0x80; mctx.file_data[11] = 0x01;
    mctx.file_data[12] = 0x00; mctx.file_data[13] = 0x01;
    mctx.file_data[14] = 0x00; mctx.file_data[15] = 0x07;
    mctx.file_data[16] = 'T'; mctx.file_data[17] = 1;
    mctx.file_data[18] = 'I'; mctx.file_data[19] = 1;
    mctx.file_data[20] = 'D'; mctx.file_data[21] = 1;
    mctx.file_data[22] = 'S'; mctx.file_data[23] = 1;
    /* Source accepts descriptor order per platform; exercise TIDSPFG,
     * while the real PC-DOS file below uses TIDSFGP. */
    mctx.file_data[24] = 'P'; mctx.file_data[25] = 2;
    mctx.file_data[26] = 'F'; mctx.file_data[27] = 1;
    mctx.file_data[28] = 'G'; mctx.file_data[29] = 1;
    mctx.file_data_len = 38;

    if (!dm2_v1_gdat_read_graphics_structure(&state, &cb, &mctx, &out)) {
        return 0;
    }
    if (!(out.valid && out.endian_swapped && out.entries == 2u &&
           out.versionlo == 5 && out.first_entry_size == 28u &&
           out.source_data_offset == 10u && out.first_raw_offset == 10u &&
           out.raw_data_end == 38u && out.ent1_validated &&
           out.ent1_entry_count == 1u && out.ent1_group_count == 7u &&
           out.ent1_stride == 8u && mctx.close_count == 1 &&
           state.ent1_field_offset[6] == 4u && state.ent1_field_size[6] == 2u &&
           state.ent1_field_offset[4] == 6u && state.ent1_field_offset[5] == 7u)) {
        return 0;
    }
    if (!state.ulp_table || state.ulp_count != 2u ||
        !dm2_v1_gdat_release_graphics_structure(&state, &cb, &mctx) ||
        state.ulp_table != NULL) return 0;
    return 1;
}

static int test_read_graphics_structure_real_dm2_data(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    RealGdatCtx real;
    DM2_V1_GdatFileCallbacks cb;
    DM2_V1_GdatFileState state;
    DM2_V1_GdatReadStructureReceipt receipt;
    FILE *probe;

    if (!root || !root[0]) return 1;
    memset(&real, 0, sizeof(real));
    if (snprintf(real.path, sizeof(real.path), "%s/GRAPHICS.DAT", root) >=
            (int)sizeof(real.path)) return 0;
    probe = fopen(real.path, "rb");
    if (!probe) {
        /* The supplied DOS extraction uses the original lowercase filename;
         * keep this real-data regression valid on case-sensitive hosts. */
        if (snprintf(real.path, sizeof(real.path), "%s/graphics.dat", root) >=
                (int)sizeof(real.path)) return 0;
    } else {
        fclose(probe);
    }
    memset(&cb, 0, sizeof(cb));
    cb.file_open = real_gdat_open;
    cb.file_close = real_gdat_close;
    cb.file_read = real_gdat_read;
    cb.file_seek = real_gdat_seek;
    cb.get_file_size = real_gdat_size;
    cb.alloc_memory = real_gdat_alloc;
    cb.dealloc_memory = real_gdat_dealloc;
    cb.format_skstr = real_gdat_path;
    dm2_v1_gdat_file_init(&state, NULL);
    if (!dm2_v1_gdat_read_graphics_structure(&state, &cb, &real, &receipt)) {
        return 0;
    }
    if (!(receipt.valid && receipt.header_validated && receipt.ulp_validated &&
           receipt.entries == 0x15f8u && receipt.versionlo == 5 &&
           receipt.first_entry_size == 0x17284u &&
           receipt.ulp_table_end == 11254u &&
           receipt.first_raw_offset == 11254u &&
           receipt.raw_data_end == 8639757u &&
           receipt.allocator_table_length == (uint32_t)receipt.entries * 2u &&
           receipt.allocator_table_initialized &&
           receipt.ent1_validated && receipt.ent1_entry_count == 0x2e4eu &&
           receipt.ent1_group_count == 7u && receipt.ent1_stride == 8u &&
           state.allocator_table != NULL &&
           state.allocator_table[0] == 0xffu &&
           state.allocator_table[state.allocator_table_length - 1u] == 0xffu &&
           state.ulp_table != NULL && state.ulp_count == receipt.entries &&
           state.ent1_data != NULL && state.ent1_length == receipt.first_entry_size &&
           state.ent1_entry_count == receipt.ent1_entry_count &&
           state.ent1_group_count == receipt.ent1_group_count &&
           state.ent1_stride == receipt.ent1_stride &&
           state.ent1_field_offset[0] == 0u && state.ent1_field_offset[1] == 1u &&
           state.ent1_field_offset[2] == 2u && state.ent1_field_offset[3] == 3u &&
           state.ent1_field_offset[4] == 4u && state.ent1_field_offset[5] == 5u &&
           state.ent1_field_offset[6] == 6u &&
           state.ent1_field_size[0] == 1u && state.ent1_field_size[1] == 1u &&
           state.ent1_field_size[2] == 1u && state.ent1_field_size[3] == 1u &&
           state.ent1_field_size[4] == 1u && state.ent1_field_size[5] == 1u &&
           state.ent1_field_size[6] == 2u)) {
        return 0;
    }
    {
        DM2_V1_GdatEnt1Row *rows = (DM2_V1_GdatEnt1Row *)calloc(
            receipt.ent1_entry_count, sizeof(*rows));
        DM2_V1_GdatEnt1RowsReceipt rows_receipt;
        int rows_ok = rows != NULL &&
            dm2_v1_gdat_materialize_ent1_rows(
                &state, rows, receipt.ent1_entry_count, &rows_receipt) &&
            rows_receipt.valid &&
            rows_receipt.entry_count == receipt.ent1_entry_count &&
            rows_receipt.row_stride == receipt.ent1_stride &&
            rows_receipt.rows_hash != 0u &&
            rows[0].cls1 == 0u && rows[0].cls2 == 0u &&
            rows[0].cls3 == 0x0bu && rows[0].cls4 == 0u &&
            rows[0].cls5 == 0u && rows[0].cls6 == 0u &&
            rows[0].data_index == 0x007bu &&
            rows[1].cls3 == 0x0bu && rows[1].cls4 == 1u &&
            rows[1].data_index == 0x0006u;
        free(rows);
        if (!rows_ok) return 0;
    }
    return dm2_v1_gdat_release_graphics_structure(&state, &cb, &real) == 1 &&
           state.ulp_table == NULL && state.allocator_table == NULL &&
           state.ent1_data == NULL;
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

static int test_open_failure_rolls_back_transaction(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    MockCtx mctx;
    DM2_V1_GdatOpenReceipt out;

    memset(&mctx, 0, sizeof(mctx));
    dm2_v1_gdat_file_init(&state, NULL);
    cb.file_open = mock_file_open_fail_first;
    if (dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, &out) != 0 ||
        out.opened || out.open_count != 0 || state.fileopencounter != 0 ||
        mctx.syserr_code != 0x29) {
        return 0;
    }

    memset(&mctx, 0, sizeof(mctx));
    dm2_v1_gdat_file_init(&state, NULL);
    state.filetype1 = false;
    state.filetype2 = true;
    cb = make_mock_callbacks();
    cb.file_open = mock_file_open_fail_second;
    if (dm2_v1_gdat_graphics_data_open(&state, &cb, &mctx, &out) != 0 ||
        out.opened || out.open_count != 0 || state.fileopencounter != 0 ||
        state.filehandle != -1 || state.xfilehandle != -1 ||
        mctx.open_count != 2 || mctx.close_count != 1 ||
        mctx.syserr_code != 0x1f) {
        return 0;
    }
    return 1;
}

static int test_close_rejects_counter_underflow(void)
{
    DM2_V1_GdatFileState state;
    DM2_V1_GdatFileCallbacks cb = make_mock_callbacks();
    DM2_V1_GdatCloseReceipt out;
    MockCtx mctx;

    memset(&state, 0, sizeof(state));
    memset(&mctx, 0, sizeof(mctx));
    if (dm2_v1_gdat_graphics_data_close(&state, &cb, &mctx, &out) != 0 ||
        out.closed || out.open_count != 0 || mctx.close_count != 0) {
        return 0;
    }
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
    TEST(open_failure_rolls_back_transaction);
    TEST(close_rejects_counter_underflow);
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
    TEST(read_graphics_structure_binds_header_and_ulp);
    TEST(read_graphics_structure_accepts_big_endian_header_and_ulp);
    TEST(read_graphics_structure_real_dm2_data);

    printf("----------------------------------\n");
    printf("Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
