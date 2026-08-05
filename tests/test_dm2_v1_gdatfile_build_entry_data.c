/*
 * test_dm2_v1_gdatfile_build_entry_data.c
 *
 * Tests for the three new c_gdatfile functions:
 *   - dm2_v1_gdat_build_entry_data  (DM2_BUILD_GDAT_ENTRY_DATA)
 *   - dm2_v1_gdat_decode_sound_sample (DM2_47eb_00a4)
 *   - dm2_v1_gdat_resolve_deferred_sounds (DM2_482b_0684)
 */

#include "dm2_v1_gdatfile_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── test fixtures ────────────────────────────────────────────────── */

#define MAX_ENTRIES 16

typedef struct {
    int32_t values[MAX_ENTRIES][8];
    int     valid[MAX_ENTRIES];
    int     count;
    uint8_t hibigpool[0x3A0];
    uint8_t freepool[4096];
    int     freepool_offset;
} TestGdatCtx;

static bool test_is_valid(void *ctx, int16_t idx)
{
    TestGdatCtx *c = (TestGdatCtx *)ctx;
    if (idx < 0 || idx >= c->count) return false;
    return c->valid[idx] != 0;
}

static int32_t test_query_value(void *ctx, int16_t idx, int16_t field)
{
    TestGdatCtx *c = (TestGdatCtx *)ctx;
    return c->values[idx][field];
}

static int16_t *test_alloc_hibigpool(void *ctx, int32_t size)
{
    TestGdatCtx *c = (TestGdatCtx *)ctx;
    (void)size;
    memset(c->hibigpool, 0, sizeof(c->hibigpool));
    return (int16_t *)c->hibigpool;
}

static void test_dealloc_hibigpool(void *ctx, int32_t size)
{
    (void)ctx; (void)size;
}

static int16_t *test_alloc_freepool_w(void *ctx, int32_t size)
{
    TestGdatCtx *c = (TestGdatCtx *)ctx;
    int16_t *p = (int16_t *)(c->freepool + c->freepool_offset);
    c->freepool_offset += size;
    memset(p, 0, (size_t)size);
    return p;
}

static DM2_V1_GdatBBW *test_alloc_freepool_bbw(void *ctx, int32_t size)
{
    TestGdatCtx *c = (TestGdatCtx *)ctx;
    DM2_V1_GdatBBW *p = (DM2_V1_GdatBBW *)(c->freepool + c->freepool_offset);
    c->freepool_offset += size;
    memset(p, 0, (size_t)size);
    return p;
}

static void test_zero_memory(void *ctx, void *dest, int32_t len)
{
    (void)ctx;
    memset(dest, 0, (size_t)len);
}

/* ── BUILD_GDAT_ENTRY_DATA tests ─────────────────────────────────── */

static void test_build_entry_data_basic(void)
{
    TestGdatCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.count = 3;

    /* Entry 0: cat=0, sub=0, id=0x10, flags=0x20, dbidx=0x100, enc=0 */
    ctx.valid[0] = 1;
    ctx.values[0][0] = 0x00; /* cat */
    ctx.values[0][1] = 0x10; /* b_00 (id) */
    ctx.values[0][2] = 0x00; /* sub */
    ctx.values[0][3] = 0x20; /* b_01 (flags) */
    ctx.values[0][4] = 0x100; /* w_02 (dbidx) */
    ctx.values[0][6] = 0;    /* enc flag */

    /* Entry 1: cat=0, sub=1, id=0x11, flags=0x21, dbidx=0x200, enc=1 */
    ctx.valid[1] = 1;
    ctx.values[1][0] = 0x00;
    ctx.values[1][1] = 0x11;
    ctx.values[1][2] = 0x01;
    ctx.values[1][3] = 0x21;
    ctx.values[1][4] = 0x200;
    ctx.values[1][6] = 1;

    /* Entry 2: invalid */
    ctx.valid[2] = 0;

    int8_t extra[7] = {2, 2, 2, 2, 2, 0, 0};
    int8_t field_sizes[7] = {2, 2, 2, 2, 2, 0, 0};

    DM2_V1_GdatBuildCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.total_entries = 3;
    cb.is_valid_entry = test_is_valid;
    cb.query_entry_value = test_query_value;
    cb.alloc_hibigpool = test_alloc_hibigpool;
    cb.dealloc_hibigpool = test_dealloc_hibigpool;
    cb.alloc_freepool_w = test_alloc_freepool_w;
    cb.alloc_freepool_bbw = test_alloc_freepool_bbw;
    cb.zero_memory = test_zero_memory;
    cb.field_sizes = field_sizes;
    cb.field_count = 7;

    DM2_V1_GdatTable table;
    memset(&table, 0, sizeof(table));

    int rc = dm2_v1_gdat_build_entry_data(&table, extra, &cb, &ctx);
    assert(rc == 1);
    assert(table.entries == 0);
    assert(table.w_10 == 2);
    assert(table.w_table1 != NULL);
    assert(table.w_table2 != NULL);
    assert(table.u31p_08 != NULL);

    /* Check entry records */
    DM2_V1_GdatBBW *e0 = &table.u31p_08[0];
    assert(e0->b_00 == 0x10);
    assert(e0->b_01 == 0x20);
    assert(e0->w_02 == 0x100);

    DM2_V1_GdatBBW *e1 = &table.u31p_08[1];
    assert(e1->b_00 == 0x11);
    assert(e1->b_01 == 0x21);
    assert((e1->w_02 & 0x7FFF) == 0x200);
    assert((e1->w_02 & (int16_t)0x8000) != 0);

    printf("PASS: build_entry_data basic\n");
}

static void test_build_entry_data_null(void)
{
    int rc = dm2_v1_gdat_build_entry_data(NULL, NULL, NULL, NULL);
    assert(rc == 0);
    printf("PASS: build_entry_data null\n");
}

/* ── decode_sound_sample tests ───────────────────────────────────── */

static void test_decode_sound_sample_xor(void)
{
    /* 6-byte header + 4 bytes of sample data */
    uint8_t buf[10] = {0, 0, 0, 0, 0x02, 0x00, 0x00, 0x80, 0xFF, 0x7F};
    DM2_V1_GdatSampleDesc desc;
    desc.xp_00 = buf + 6;
    desc.w_04 = 4;
    desc.w_06 = 0;
    desc.next = NULL;

    DM2_V1_GdatSampleDesc *head = NULL;
    DM2_V1_GdatSampleDecodeReceipt receipt;

    dm2_v1_gdat_decode_sound_sample(&desc, &head, &receipt);

    assert(receipt.decoded == true);
    assert(head == &desc);
    assert(buf[6] == 0x80);  /* 0x00 ^ 0x80 */
    assert(buf[7] == 0x00);  /* 0x80 ^ 0x80 */
    assert(buf[8] == 0x7F);  /* 0xFF ^ 0x80 */
    assert(buf[9] == 0xFF);  /* 0x7F ^ 0x80 */
    /* control should be set to 1 */
    assert(buf[4] == 1 && buf[5] == 0);

    printf("PASS: decode_sound_sample XOR\n");
}

static void test_decode_sound_sample_already_decoded(void)
{
    uint8_t buf[10] = {0, 0, 0, 0, 0x01, 0x00, 0xAA, 0xBB, 0xCC, 0xDD};
    DM2_V1_GdatSampleDesc desc;
    desc.xp_00 = buf + 6;
    desc.w_04 = 4;
    desc.w_06 = 0;
    desc.next = NULL;

    DM2_V1_GdatSampleDesc *head = NULL;
    DM2_V1_GdatSampleDecodeReceipt receipt;

    dm2_v1_gdat_decode_sound_sample(&desc, &head, &receipt);

    assert(receipt.decoded == false);
    assert(buf[6] == 0xAA);  /* unchanged */
    assert(buf[7] == 0xBB);
    printf("PASS: decode_sound_sample already decoded\n");
}

static void test_decode_sound_sample_null(void)
{
    DM2_V1_GdatSampleDecodeReceipt receipt;
    dm2_v1_gdat_decode_sound_sample(NULL, NULL, &receipt);
    assert(receipt.decoded == false);
    printf("PASS: decode_sound_sample null\n");
}

/* ── resolve_deferred_sounds tests ───────────────────────────────── */

static int16_t mock_query_index(void *ctx, int8_t c1, int8_t c2,
                                 int8_t type, int8_t idx)
{
    (void)ctx; (void)c1; (void)c2; (void)type; (void)idx;
    return 42;
}

static uint16_t mock_sound7(void *ctx, int16_t id)
{
    (void)ctx; (void)id;
    return 0; /* not loaded */
}

static uint8_t g_mock_raw[16] = {
    0x7C, 0x15, /* sample rate LE */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0xFF, 0x7F, 0x40, 0xC0, 0, 0, 0, 0
};

static uint8_t *mock_query_ptr(void *ctx, int8_t c1, int8_t c2,
                                int8_t type, int8_t idx)
{
    (void)ctx; (void)c1; (void)c2; (void)type; (void)idx;
    return g_mock_raw;
}

static int32_t mock_query_len(void *ctx, int8_t c1, int8_t c2,
                               int8_t type, int8_t idx)
{
    (void)ctx; (void)c1; (void)c2; (void)type; (void)idx;
    return 12;
}

static void test_resolve_deferred_sounds_basic(void)
{
    DM2_V1_GdatSoundEntry queue[2];
    memset(queue, 0, sizeof(queue));
    queue[0].w_05 = -1;
    queue[0].b_02 = 1;
    queue[0].b_03 = 2;
    queue[0].b_04 = 3;
    queue[1].w_05 = 99; /* already resolved */

    /* Reset mock raw for XOR decode header */
    uint8_t raw_copy[16];
    memcpy(raw_copy, g_mock_raw, sizeof(g_mock_raw));

    uint8_t pool[64];
    memset(pool, 0, sizeof(pool));

    uint16_t active = 0;
    DM2_V1_GdatSampleDesc *head = NULL;

    DM2_V1_GdatResolveSoundCallbacks cb;
    cb.query_entry_data_index = mock_query_index;
    cb.sound7_lookup = mock_sound7;
    cb.query_entry_data_ptr = mock_query_ptr;
    cb.query_entry_data_length = mock_query_len;

    DM2_V1_GdatResolveSoundReceipt receipt;

    int rc = dm2_v1_gdat_resolve_deferred_sounds(
        queue, 2, pool, sizeof(DM2_V1_GdatSampleDesc),
        &active, 10, 1, &head, &cb, NULL, &receipt);

    assert(rc == 1);
    assert(receipt.resolved_count == 1);
    assert(active == 1);
    assert(queue[0].w_05 == 42);
    assert(queue[0].w_00 == 0);
    assert(queue[1].w_05 == 99); /* unchanged */

    printf("PASS: resolve_deferred_sounds basic\n");
}

/* ── main ─────────────────────────────────────────────────────────── */

int main(void)
{
    test_build_entry_data_null();
    test_build_entry_data_basic();
    test_decode_sound_sample_null();
    test_decode_sound_sample_xor();
    test_decode_sound_sample_already_decoded();
    test_resolve_deferred_sounds_basic();

    printf("\nAll gdatfile_build_entry_data tests passed.\n");
    return 0;
}
