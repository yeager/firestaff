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
static int mock_ai_spec_alt(void *ctx, uint16_t link) { (void)ctx; (void)link; return 1; }
static int mock_is_map(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_moneybox(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_moneybox_true(void *ctx, uint16_t link) { (void)ctx; (void)link; return 1; }
static void mock_add_poss(void *ctx, uint16_t link) { (void)ctx; (void)link; }

/* ---- Reader mock pool ---- */

#define MAX_READ_RECORDS 16
#define MAX_REC_SIZE 16

typedef struct {
    uint8_t data[MAX_REC_SIZE];
    size_t size;
    int type;
    uint16_t next_link;
    uint16_t child_link;
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
    pool->records[idx].next_link = DM2_RECORD_LINK_END;
    pool->records[idx].child_link = DM2_RECORD_LINK_END;
    return (uint16_t)(((uint16_t)record_type << 10) | (uint16_t)idx);
}

static int read_set_data(void *ctx, uint16_t link,
                         const uint8_t *data, size_t size)
{
    ReadPool *pool = (ReadPool *)ctx;
    const uint16_t index = (uint16_t)(link & 0x03ffu);
    if (index >= MAX_READ_RECORDS) return -1;
    if (size > MAX_REC_SIZE) size = MAX_REC_SIZE;
    memcpy(pool->records[index].data, data, size);
    pool->records[index].size = size;
    return 0;
}

static int read_append(void *ctx, uint16_t next, uint16_t *owner,
                       int map_x, int map_y)
{
    ReadPool *pool = (ReadPool *)ctx;
    uint16_t *tail;
    const uint16_t next_index = (uint16_t)(next & 0x03ffu);

    (void)map_x;
    (void)map_y;
    if (!pool || !owner || next_index >= MAX_READ_RECORDS) return -1;
    if (*owner == 0xfffeu) {
        *owner = next;
        return 0;
    }
    tail = owner;
    while (*tail != DM2_RECORD_LINK_END) {
        const uint16_t tail_index = (uint16_t)(*tail & 0x03ffu);
        if (tail_index >= MAX_READ_RECORDS) return -1;
        tail = &pool->records[tail_index].next_link;
    }
    *tail = next;
    return 0;
}

static int read_child_owner(void *ctx, uint16_t link, uint16_t **out)
{
    ReadPool *pool = (ReadPool *)ctx;
    const uint16_t index = (uint16_t)(link & 0x03ffu);
    if (!pool || !out || index >= MAX_READ_RECORDS) return -1;
    *out = &pool->records[index].child_link;
    return 0;
}

static void read_add_possession(void *ctx, uint16_t link)
{
    ReadPool *pool = (ReadPool *)ctx;
    pool->possession_count++;
    pool->last_possession_link = link;
}

static int read_creature_ai_flags(void *ctx, uint16_t link,
                                  uint8_t creature_type,
                                  uint16_t *out_flags)
{
    (void)ctx;
    (void)link;
    (void)creature_type;
    if (!out_flags) return -1;
    *out_flags = 0u;
    return 0;
}

static int read_creature_ai_flags_alt(void *ctx, uint16_t link,
                                      uint8_t creature_type,
                                      uint16_t *out_flags)
{
    (void)ctx;
    (void)link;
    (void)creature_type;
    if (!out_flags) return -1;
    *out_flags = 1u;
    return 0;
}

typedef struct {
    int count;
    uint16_t links[4];
    uint16_t continuations[4];
} ContinuationMock;

typedef struct {
    int count;
    uint16_t record_link[4];
    uint16_t timer_index[4];
    uint8_t slot[4];
} TimerBindingMock;
static TimerBindingMock *g_timer_binding_target;

static int read_set_continuation(void *ctx, uint16_t record_link,
                                 uint16_t continuation)
{
    ContinuationMock *mock = (ContinuationMock *)ctx;
    if (!mock || mock->count >= 4) return -1;
    mock->links[mock->count] = record_link;
    mock->continuations[mock->count] = continuation;
    mock->count++;
    return 0;
}

static int read_bind_timer(void *ctx, uint16_t record_link,
                           uint16_t timer_index, uint8_t slot)
{
    TimerBindingMock *mock = g_timer_binding_target;
    (void)ctx;
    if (!mock || mock->count >= 4) return -1;
    mock->record_link[mock->count] = record_link;
    mock->timer_index[mock->count] = timer_index;
    mock->slot[mock->count] = slot;
    mock->count++;
    return 0;
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
    cb.append_record = read_append;
    cb.child_owner = read_child_owner;
    cb.add_possession_index = read_add_possession;
    cb.is_container_moneybox = mock_is_moneybox;
    cb.query_creature_ai_flags = read_creature_ai_flags;
    cb.ctx = pool;
    return cb;
}

static int read_from_root(DM2_ReadRecordSession *session,
                          const DM2_ReadRecordCallbacks *cb,
                          uint16_t *out_root, int read_sub_chain_info,
                          int follow_chain)
{
    uint16_t root = DM2_RECORD_LINK_END;
    const int rc = dm2_v1_read_record_checkcode(session, cb, &root, -1, 0,
                                                 read_sub_chain_info,
                                                 follow_chain);
    if (out_root) *out_root = root;
    return rc;
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

static size_t write_timer_record(uint8_t *buf, size_t cap,
                                 uint16_t record_link)
{
    DM2_WriteRecordTimer timer = {0x19u, record_link};
    DM2_WriteRecordSession session;
    DM2_WriteRecordCallbacks cb = make_writer_cb();
    int creature_idx[4], container_idx[4];
    size_t flushed;

    dm2_v1_write_record_session_init(&session, buf, cap,
        creature_idx, 4, container_idx, 4, &timer, 1);
    assert(dm2_v1_write_record_checkcode(&session, &cb,
        record_link, 0, 1) == 0);
    assert(dm2_suppress_writer_flush(&session.writer,
        buf + session.out_written, cap - session.out_written,
        &flushed) == 0);
    return session.out_written + flushed;
}

/* ---- Tests ---- */

static void test_null_safety(void)
{
    assert(dm2_v1_read_record_checkcode(NULL, NULL, NULL, -1, 0, 0, 0) == -1);
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

    int rrc = read_from_root(&rd, &rcb, NULL, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 0);
    printf("  PASS: empty_chain\n");
}

static void test_special_timer_record_chains(void)
{
    uint8_t stream[128];
    DM2_V1_SaveTimerRecord timer;
    DM2_ReadRecordSession session;
    DM2_ReadRecordCallbacks cb;
    ReadPool pool;
    uint16_t decoded = 0u;
    size_t stream_size;

    mock_init();
    memset(&pool, 0, sizeof(pool));
    cb = make_reader_cb(&pool);
    stream_size = write_and_flush(5, 0, 0, stream, sizeof(stream));
    dm2_v1_read_record_session_init(&session, stream, stream_size);
    memset(&timer, 0, sizeof(timer));
    timer.bytes[4] = 0x3cu;
    dm2_v1_save_timer_set_b(&timer, 0x1400);

    assert(dm2_v1_read_special_timer_record_chains(
               &session, &cb, &timer, 1u, 1u, &decoded) == 0);
    assert(decoded == 1u);
    assert(dm2_v1_save_timer_get_b(&timer) == 0x1400);
    assert(pool.count == 1 && pool.records[0].type == 5);

    /* Source DM2_2066_197c rejects the whole phase when savegamew7 is zero. */
    dm2_v1_read_record_session_init(&session, stream, stream_size);
    dm2_v1_save_timer_set_b(&timer, 0x1400);
    assert(dm2_v1_read_special_timer_record_chains(
               &session, &cb, &timer, 1u, 0u, &decoded) != 0);
    assert(dm2_v1_save_timer_get_b(&timer) == 0x1400);

    /* A truncated chain must not publish the temporary end marker. */
    dm2_v1_read_record_session_init(&session, stream, 1u);
    assert(dm2_v1_read_special_timer_record_chains(
               &session, &cb, &timer, 1u, 1u, &decoded) != 0);
    assert(dm2_v1_save_timer_get_b(&timer) == 0x1400);

    printf("  PASS: special_timer_record_chains_source_owner\n");
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

    uint16_t root;
    int rrc = read_from_root(&rd, &rcb, &root, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 1);
    assert(root == mock_make_link(0, 5));
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

    int rrc = read_from_root(&rd, &rcb, NULL, 0, 1);
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

    uint16_t root;
    int rrc = read_from_root(&rd, &rcb, &root, 0, 1);
    assert(rrc == 0);
    assert(pool.count == 2);
    assert(pool.records[0].type == 5);
    assert(pool.records[1].type == 7);
    assert(root == mock_make_link(0, 5));
    assert(pool.records[0].next_link == mock_make_link(1, 7));
    assert(pool.records[1].next_link == DM2_RECORD_LINK_END);
    printf("  PASS: round_trip_chain\n");
}

static void test_round_trip_sub_chain_bits(void)
{
    uint8_t buf[256];
    DM2_WriteRecordSession wr;
    DM2_ReadRecordSession rd;
    DM2_WriteRecordCallbacks wcb = make_writer_cb();
    DM2_ReadRecordCallbacks rcb;
    ReadPool pool;
    int ci[4], co[4];
    size_t flush_size;
    uint16_t root;

    mock_init();
    dm2_v1_write_record_session_init(&wr, buf, sizeof(buf), ci, 4, co, 4,
                                     NULL, 0);
    assert(dm2_v1_write_record_checkcode(&wr, &wcb,
        (uint16_t)(mock_make_link(0, 5) | 0x8000u), 1, 0) == 0);
    assert(dm2_suppress_writer_flush(&wr.writer, buf + wr.out_written,
        sizeof(buf) - wr.out_written, &flush_size) == 0);
    wr.out_written += flush_size;

    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, wr.out_written);
    rcb = make_reader_cb(&pool);
    assert(read_from_root(&rd, &rcb, &root, 1, 0) == 0);
    assert((root & 0xc000u) == 0x8000u);
    assert((root & 0x3fffu) == mock_make_link(0, 5));
    printf("  PASS: round_trip_sub_chain_bits\n");
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
    assert(read_from_root(&rd, &rcb, NULL, 0, 1) == 0);
    assert(pool.count == 1);
    assert(pool.records[0].type == 9);
    assert((pool.records[0].data[4] & 0x06u) == 0x02u);
    assert(pool.possession_count == 1);
    assert(rd.map_containers_read == 1);
    assert(rd.possessions_read == 1);
    printf("  PASS: round_trip_map_container\n");
}

static void test_round_trip_moneybox_misc_mask(void)
{
    uint8_t buf[256];
    DM2_ReadRecordSession rd;
    DM2_ReadRecordCallbacks rcb;
    DM2_WriteRecordCallbacks wcb;
    ReadPool pool;
    int ci[4], co[4];
    size_t flushed;
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    const uint8_t *default_mask = dm2_v1_save_record_mask_misc_default();
    const uint8_t *moneybox_mask = dm2_v1_save_record_mask_misc_moneybox();
    int masks_differ = 0;

    mock_init();
    /* DB9 normal container owns a DB10 child through w2. */
    g_mock_records[0][2] = (uint8_t)mock_make_link(1, 10);
    g_mock_records[0][3] = (uint8_t)(mock_make_link(1, 10) >> 8);
    g_mock_records[1][0] = 0x5au;
    g_mock_records[1][1] = 0xa5u;
    g_mock_records[1][2] = 0xc3u;
    g_mock_records[1][3] = 0x3cu;
    g_mock_next[1] = DM2_RECORD_LINK_END;
    wcb = make_writer_cb();
    wcb.is_container_moneybox = mock_is_moneybox_true;
    {
        DM2_WriteRecordSession wr;
        dm2_v1_write_record_session_init(&wr, buf, sizeof(buf), ci, 4,
                                         co, 4, NULL, 0);
        assert(dm2_v1_write_record_checkcode(
                   &wr, &wcb, mock_make_link(0, 9), 0, 1) == 0);
        assert(dm2_suppress_writer_flush(&wr.writer, buf + wr.out_written,
                                          sizeof(buf) - wr.out_written,
                                          &flushed) == 0);
        wr.out_written += flushed;
        memset(&pool, 0, sizeof(pool));
        dm2_v1_read_record_session_init(&rd, buf, wr.out_written);
        rcb = make_reader_cb(&pool);
        rcb.is_container_moneybox = mock_is_moneybox_true;
        assert(read_from_root(&rd, &rcb, NULL, 0, 1) == 0);
    }
    for (size_t i = 0; i < sizes[10]; ++i) {
        if (default_mask[i] != moneybox_mask[i]) masks_differ = 1;
        assert((pool.records[1].data[i] & moneybox_mask[i]) ==
               (g_mock_records[1][i] & moneybox_mask[i]));
    }
    assert(masks_differ);
    assert(pool.count == 2);
    printf("  PASS: round_trip_moneybox_misc_mask\n");
}

static void test_creature_requires_source_ai_mask(void)
{
    uint8_t buf[256];
    DM2_ReadRecordSession rd;
    DM2_ReadRecordCallbacks rcb;
    ReadPool pool;
    size_t written;

    mock_init();
    g_mock_records[0][4] = 7u;
    written = write_and_flush(4, 0, 1, buf, sizeof(buf));
    assert(written > 0u);
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, written);
    rcb = make_reader_cb(&pool);
    rcb.query_creature_ai_flags = NULL;
    assert(read_from_root(&rd, &rcb, NULL, 0, 1) == -1);
    assert(rd.error == 1);
    printf("  PASS: creature_requires_source_ai_mask\n");
}

static void test_creature_uses_source_ai_mask(void)
{
    uint8_t buf[256];
    DM2_WriteRecordSession wr;
    DM2_ReadRecordSession rd;
    DM2_WriteRecordCallbacks wcb;
    DM2_ReadRecordCallbacks rcb;
    ReadPool pool;
    int ci[4], co[4];
    size_t flushed;

    mock_init();
    g_mock_records[0][2] = 0xfeu;
    g_mock_records[0][3] = 0xffu;
    g_mock_records[0][4] = 7u;
    g_mock_records[0][5] = 0x5au;
    wcb = make_writer_cb();
    wcb.query_creature_ai_spec_flags = mock_ai_spec_alt;
    dm2_v1_write_record_session_init(&wr, buf, sizeof(buf), ci, 4, co, 4,
                                     NULL, 0);
    assert(dm2_v1_write_record_checkcode(&wr, &wcb,
                                         mock_make_link(0, 4), 0, 1) == 0);
    assert(dm2_suppress_writer_flush(&wr.writer, buf + wr.out_written,
                                     sizeof(buf) - wr.out_written,
                                     &flushed) == 0);
    wr.out_written += flushed;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, wr.out_written);
    rcb = make_reader_cb(&pool);
    rcb.query_creature_ai_flags = read_creature_ai_flags_alt;
    assert(read_from_root(&rd, &rcb, NULL, 0, 1) == 0);
    assert(pool.count == 1);
    assert(pool.records[0].type == 4);
    assert(pool.records[0].data[4] == 7u);
    printf("  PASS: creature_uses_source_ai_mask\n");
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

static void test_possession_continuations(void)
{
    const uint16_t links[] = {
        mock_make_link(2, 5),
        mock_make_link(0, 9),
        mock_make_link(1, 0x0e),
        mock_make_link(3, 0)
    };
    const uint8_t values[] = {
        0x23, 0x01, /* source continuation payload 0x0123 */
        0xab, 0x02  /* source continuation payload 0x02ab */
    };
    const uint8_t mask[] = { 0xff, 0x03, 0xff, 0x03 };
    uint8_t encoded[8] = {0};
    DM2_SuppressReader reader;
    DM2_ReadPossessionContinuationCallbacks cb;
    ContinuationMock mock;
    int encoded_size;

    encoded_size = dm2_suppress_encode(values, mask, sizeof(values),
                                       encoded, sizeof(encoded));
    assert(encoded_size > 0);
    memset(&mock, 0, sizeof(mock));
    cb.set_continuation = read_set_continuation;
    cb.ctx = &mock;
    dm2_suppress_reader_init(&reader, encoded, (size_t)encoded_size);
    assert(dm2_v1_read_possession_continuations(&reader, links,
                                                sizeof(links) / sizeof(links[0]),
                                                &cb) == 0);
    assert(mock.count == 2);
    assert(mock.links[0] == links[1]);
    assert(mock.continuations[0] == 0x1123);
    assert(mock.links[1] == links[2]);
    assert(mock.continuations[1] == 0x26ab);
    /* SKProject's `if (type < 9) ; else if (type <= 9)` deliberately
     * consumes no continuation bits for the 0..8 record classes. Placing
     * type 5 first makes an accidental read shift both asserted payloads. */
    printf("  PASS: possession_continuations_source_type_gate\n");
}

static void test_possession_continuations_underflow(void)
{
    const uint16_t link = mock_make_link(0, 9);
    const uint8_t encoded[] = {0};
    DM2_SuppressReader reader;
    DM2_ReadPossessionContinuationCallbacks cb;
    ContinuationMock mock;

    memset(&mock, 0, sizeof(mock));
    cb.set_continuation = read_set_continuation;
    cb.ctx = &mock;
    dm2_suppress_reader_init(&reader, encoded, sizeof(encoded));
    assert(dm2_v1_read_possession_continuations(&reader, &link, 1, &cb) == -1);
    assert(mock.count == 0);
    printf("  PASS: possession_continuations_underflow_is_closed\n");
}

static void test_timer_record_bindings(void)
{
    uint8_t buf[256];
    DM2_ReadRecordSession rd;
    DM2_ReadRecordCallbacks cb;
    ReadPool pool;
    TimerBindingMock bindings;
    size_t written;

    mock_init();
    g_mock_records[0][6] = 3u;
    g_mock_records[0][7] = 0u;
    written = write_and_flush(0x0e, 0, 1, buf, sizeof(buf));
    memset(&pool, 0, sizeof(pool));
    memset(&bindings, 0, sizeof(bindings));
    dm2_v1_read_record_session_init(&rd, buf, written);
    cb = make_reader_cb(&pool);
    cb.bind_timer_record = read_bind_timer;
    g_timer_binding_target = &bindings;
    assert(read_from_root(&rd, &cb, NULL, 0, 1) == 0);
    assert(bindings.count == 1 &&
           bindings.record_link[0] == mock_make_link(0, 0x0e) &&
           bindings.timer_index[0] == 3u && bindings.slot[0] == 0u);

    mock_init();
    written = write_timer_record(buf, sizeof(buf), mock_make_link(0, 0x0f));
    memset(&pool, 0, sizeof(pool));
    memset(&bindings, 0, sizeof(bindings));
    dm2_v1_read_record_session_init(&rd, buf, written);
    cb = make_reader_cb(&pool);
    cb.bind_timer_record = read_bind_timer;
    g_timer_binding_target = &bindings;
    assert(read_from_root(&rd, &cb, NULL, 0, 1) == 0);
    assert(bindings.count == 1 &&
           bindings.record_link[0] == mock_make_link(0, 0x0f) &&
           bindings.timer_index[0] == 0u && bindings.slot[0] == 1u);
    g_timer_binding_target = NULL;
    printf("  PASS: timer_record_bindings_source_slots\n");
}

int main(void)
{
    printf("test_dm2_v1_save_read_record_checkcode:\n");
    test_null_safety();
    test_init();
    test_empty_chain();
    test_special_timer_record_chains();
    test_round_trip_type5();
    test_round_trip_type6();
    test_round_trip_chain();
    test_round_trip_sub_chain_bits();
    test_round_trip_map_container();
    test_round_trip_moneybox_misc_mask();
    test_creature_requires_source_ai_mask();
    test_creature_uses_source_ai_mask();
    test_session_counters();
    test_possession_continuations();
    test_possession_continuations_underflow();
    test_timer_record_bindings();
    printf("All read_record_checkcode tests passed.\n");
    return 0;
}
