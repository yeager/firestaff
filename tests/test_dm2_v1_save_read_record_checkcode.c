/* Test DM2 READ_RECORD_CHECKCODE — round-trip with WRITE_RECORD_CHECKCODE.
 * Source: sksvgame.cpp:808-974 (reader), 1739-1940 (writer).
 * Only types > 3 go through RECORD_CHECKCODE; types 0-3 are bulk-serialized. */

#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Writer mock (same scheme as test_dm2_v1_save_write_record_checkcode) ---- */

#define MOCK_POOL_SIZE 8
static uint8_t g_mock_records[MOCK_POOL_SIZE][16];
static uint16_t g_mock_next[MOCK_POOL_SIZE];

static void mock_init(void)
{
    memset(g_mock_records, 0, sizeof(g_mock_records));
    for (int i = 0; i < MOCK_POOL_SIZE; i++)
        g_mock_next[i] = DM2_RECORD_LINK_END;
}

static uint16_t mock_make_link(int index, int type)
{
    return (uint16_t)((type << 10) | index);
}

static int mock_get_record(void *ctx, uint16_t link, DM2_WriteRecordData *out)
{
    (void)ctx;
    int idx = link & 0x3FF;
    if (idx >= MOCK_POOL_SIZE) return -1;
    int type = (link & 0x3C00) >> 10;
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    out->data = g_mock_records[idx];
    out->size = sizes[type];
    out->word_00 = (uint16_t)(g_mock_records[idx][0] |
                              (g_mock_records[idx][1] << 8));
    out->word_02 = (uint16_t)(g_mock_records[idx][2] |
                              (g_mock_records[idx][3] << 8));
    out->byte_04 = g_mock_records[idx][4];
    out->word_04 = (uint16_t)(g_mock_records[idx][4] |
                              (g_mock_records[idx][5] << 8));
    return 0;
}

static uint16_t mock_get_next(void *ctx, uint16_t link)
{
    (void)ctx;
    int idx = link & 0x3FF;
    if (idx >= MOCK_POOL_SIZE) return DM2_RECORD_LINK_END;
    return g_mock_next[idx];
}

static int mock_ai_spec(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_map(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_moneybox(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static void mock_add_poss(void *ctx, uint16_t link) { (void)ctx; (void)link; }

/* ---- Reader mock pool ---- */

#define MAX_READ_RECORDS 16
#define MAX_REC_SIZE 16

typedef struct {
    uint8_t data[MAX_REC_SIZE];
    size_t size;
    int type;
} ReadRecord;

typedef struct {
    ReadRecord records[MAX_READ_RECORDS];
    int count;
    int possession_count;
    uint16_t last_possession_link;
} ReadPool;

static uint16_t read_alloc(void *ctx, int record_type)
{
    ReadPool *pool = (ReadPool *)ctx;
    if (pool->count >= MAX_READ_RECORDS) return 0xFFFE;
    int idx = pool->count++;
    pool->records[idx].type = record_type;
    return (uint16_t)idx;
}

static int read_set_data(void *ctx, uint16_t link,
                         const uint8_t *data, size_t size)
{
    ReadPool *pool = (ReadPool *)ctx;
    if (link >= MAX_READ_RECORDS) return -1;
    if (size > MAX_REC_SIZE) size = MAX_REC_SIZE;
    memcpy(pool->records[link].data, data, size);
    pool->records[link].size = size;
    return 0;
}

static int read_chain(void *ctx, uint16_t prev, uint16_t next)
{
    (void)ctx; (void)prev; (void)next;
    return 0;
}

static void read_add_possession(void *ctx, uint16_t link)
{
    ReadPool *pool = (ReadPool *)ctx;
    pool->possession_count++;
    pool->last_possession_link = link;
}

/* ---- Helpers ---- */

static DM2_WriteRecordCallbacks make_writer_cb(void)
{
    DM2_WriteRecordCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_record = mock_get_record;
    cb.get_next_link = mock_get_next;
    cb.query_creature_ai_spec_flags = mock_ai_spec;
    cb.is_container_map = mock_is_map;
    cb.is_container_moneybox = mock_is_moneybox;
    cb.add_possession_index = mock_add_poss;
    return cb;
}

static DM2_ReadRecordCallbacks make_reader_cb(ReadPool *pool)
{
    DM2_ReadRecordCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.alloc_record = read_alloc;
    cb.set_data = read_set_data;
    cb.chain_record = read_chain;
    cb.add_possession_index = read_add_possession;
    cb.ctx = pool;
    return cb;
}

static size_t write_and_flush(int type, int rec_idx, int follow_chain,
                              uint8_t *buf, size_t cap)
{
    DM2_WriteRecordSession session;
    int creature_idx[4], container_idx[4];

    DM2_WriteRecordCallbacks cb = make_writer_cb();
    uint16_t link = mock_make_link(rec_idx, type);

    dm2_v1_write_record_session_init(&session, buf, cap,
        creature_idx, 4, container_idx, 4, NULL, 0);

    int rc = dm2_v1_write_record_checkcode(&session, &cb, link, 0, follow_chain);
    assert(rc == 0);

    size_t flush_w;
    dm2_suppress_writer_flush(&session.writer,
        buf + session.out_written,
        cap - session.out_written, &flush_w);
    session.out_written += flush_w;

    return session.out_written;
}

/* ---- Tests ---- */

static void test_null_safety(void)
{
    assert(dm2_v1_read_record_checkcode(NULL, NULL, 0, 0) == -1);
    printf("  PASS: null_safety\n");
}

static void test_init(void)
{
    DM2_ReadRecordSession rd;
    uint8_t buf[64];
    memset(buf, 0, sizeof(buf));
    dm2_v1_read_record_session_init(&rd, buf, sizeof(buf));
    dm2_v1_read_record_session_init(NULL, NULL, 0);
    printf("  PASS: init\n");
}

static void test_empty_chain(void)
{
    uint8_t buf[64];
    DM2_WriteRecordSession wr;
    int ci[4], co[4];
    DM2_WriteRecordCallbacks wcb = make_writer_cb();

    mock_init();
    dm2_v1_write_record_session_init(&wr, buf, sizeof(buf),
        ci, 4, co, 4, NULL, 0);
    int rc = dm2_v1_write_record_checkcode(&wr, &wcb,
        DM2_RECORD_LINK_END, 0, 1);
    assert(rc == 0);

    size_t fw;
    dm2_suppress_writer_flush(&wr.writer, buf + wr.out_written,
        sizeof(buf) - wr.out_written, &fw);
    wr.out_written += fw;

    DM2_ReadRecordSession rd;
    ReadPool pool;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, wr.out_written);
    DM2_ReadRecordCallbacks rcb = make_reader_cb(&pool);

    int rrc = dm2_v1_read_record_checkcode(&rd, &rcb, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 0);
    printf("  PASS: empty_chain\n");
}

static void test_round_trip_type5(void)
{
    mock_init();
    g_mock_records[0][0] = 0xAA;
    g_mock_records[0][1] = 0xBB;
    g_mock_records[0][2] = 0xCC;
    g_mock_records[0][3] = 0xDD;

    uint8_t buf[256];
    size_t written = write_and_flush(5, 0, 1, buf, sizeof(buf));
    assert(written > 0);

    DM2_ReadRecordSession rd;
    ReadPool pool;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, written);
    DM2_ReadRecordCallbacks rcb = make_reader_cb(&pool);

    int rrc = dm2_v1_read_record_checkcode(&rd, &rcb, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 1);
    assert(pool.records[0].type == 5);

    const uint8_t *rec_mask = dm2_v1_save_record_mask_for_type(5);
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    for (size_t i = 0; i < sizes[5]; i++)
        assert((pool.records[0].data[i] & rec_mask[i]) ==
               (g_mock_records[0][i] & rec_mask[i]));

    printf("  PASS: round_trip_type5\n");
}

static void test_round_trip_type6(void)
{
    mock_init();
    g_mock_records[0][0] = 0x11;
    g_mock_records[0][1] = 0x22;
    g_mock_records[0][2] = 0x33;
    g_mock_records[0][3] = 0x44;

    uint8_t buf[256];
    size_t written = write_and_flush(6, 0, 1, buf, sizeof(buf));
    assert(written > 0);

    DM2_ReadRecordSession rd;
    ReadPool pool;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, written);
    DM2_ReadRecordCallbacks rcb = make_reader_cb(&pool);

    int rrc = dm2_v1_read_record_checkcode(&rd, &rcb, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 1);
    assert(pool.records[0].type == 6);
    printf("  PASS: round_trip_type6\n");
}

static void test_round_trip_chain(void)
{
    mock_init();
    g_mock_records[0][0] = 0x10;
    g_mock_records[0][1] = 0x20;
    g_mock_records[0][2] = 0x30;
    g_mock_records[0][3] = 0x40;

    g_mock_records[1][0] = 0x50;
    g_mock_records[1][1] = 0x60;
    g_mock_records[1][2] = 0x70;
    g_mock_records[1][3] = 0x80;

    g_mock_next[0] = mock_make_link(1, 7);

    uint8_t buf[256];
    size_t written = write_and_flush(5, 0, 1, buf, sizeof(buf));
    assert(written > 0);

    DM2_ReadRecordSession rd;
    ReadPool pool;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, written);
    DM2_ReadRecordCallbacks rcb = make_reader_cb(&pool);

    int rrc = dm2_v1_read_record_checkcode(&rd, &rcb, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 2);
    assert(pool.records[0].type == 5);
    assert(pool.records[1].type == 7);
    printf("  PASS: round_trip_chain\n");
}

static void test_round_trip_map_container(void)
{
    uint8_t buf[256];
    DM2_ReadRecordSession rd;
    DM2_ReadRecordCallbacks rcb;
    ReadPool pool;
    size_t written;

    mock_init();
    /* SKProject DM2_IS_CONTAINER_MAP: (word@4 & 6) == 2. The source emits
     * only a possession bit for this container; it must not recurse into the
     * apparent word_02 link. */
    g_mock_records[0][2] = 0x00u;
    g_mock_records[0][3] = 0x00u;
    g_mock_records[0][4] = 0x02u;
    written = write_and_flush(9, 0, 1, buf, sizeof(buf));
    assert(written > 0u);

    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, written);
    rcb = make_reader_cb(&pool);
    assert(dm2_v1_read_record_checkcode(&rd, &rcb, 0, 1) == 0);
    assert(pool.count == 1);
    assert(pool.records[0].type == 9);
    assert((pool.records[0].data[4] & 0x06u) == 0x02u);
    assert(pool.possession_count == 1);
    assert(rd.map_containers_read == 1);
    assert(rd.possessions_read == 1);
    printf("  PASS: round_trip_map_container\n");
}

static void test_session_counters(void)
{
    DM2_ReadRecordSession rd;
    uint8_t buf[1] = {0};
    dm2_v1_read_record_session_init(&rd, buf, 1);
    assert(rd.records_read == 0);
    assert(rd.creatures_read == 0);
    assert(rd.containers_read == 0);
    assert(rd.error == 0);
    printf("  PASS: session_counters\n");
}

int main(void)
{
    printf("test_dm2_v1_save_read_record_checkcode:\n");
    test_null_safety();
    test_init();
    test_empty_chain();
    test_round_trip_type5();
    test_round_trip_type6();
    test_round_trip_chain();
    test_round_trip_map_container();
    test_session_counters();
    printf("All read_record_checkcode tests passed.\n");
    return 0;
}
