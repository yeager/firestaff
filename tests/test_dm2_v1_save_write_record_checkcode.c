/* DM2 WRITE_RECORD_CHECKCODE unit test with mock record data.
 * Source: sksvgame.cpp:1739-1940. */
#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock record pool: 4 records. */
#define MOCK_POOL_SIZE 4
static uint8_t g_mock_records[MOCK_POOL_SIZE][16];
static uint16_t g_mock_next[MOCK_POOL_SIZE];
static int g_mock_type[MOCK_POOL_SIZE];

static void mock_init(void) {
    memset(g_mock_records, 0, sizeof(g_mock_records));
    for (int i = 0; i < MOCK_POOL_SIZE; i++) {
        g_mock_next[i] = DM2_RECORD_LINK_END;
        g_mock_type[i] = 0;
    }
}

static uint16_t mock_make_link(int index, int type) {
    g_mock_type[index] = type;
    return (uint16_t)((type << 10) | index);
}

static int mock_get_record(void *ctx, uint16_t link, DM2_WriteRecordData *out) {
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

static uint16_t mock_get_next(void *ctx, uint16_t link) {
    (void)ctx;
    int idx = link & 0x3FF;
    if (idx >= MOCK_POOL_SIZE) return DM2_RECORD_LINK_END;
    return g_mock_next[idx];
}

static int mock_ai_spec(void *ctx, uint16_t link) {
    (void)ctx; (void)link;
    return 0;
}

static int mock_is_map(void *ctx, uint16_t link) {
    (void)ctx; (void)link;
    return 0;
}

static int mock_is_moneybox(void *ctx, uint16_t link) {
    (void)ctx; (void)link;
    return 0;
}

static void mock_add_poss(void *ctx, uint16_t link) {
    (void)ctx; (void)link;
}

int main(void) {
    DM2_WriteRecordSession session;
    DM2_WriteRecordCallbacks cb;
    uint8_t buf[256];
    int creature_idx[16], container_idx[16];
    int rc;

    memset(&cb, 0, sizeof(cb));
    cb.get_record = mock_get_record;
    cb.get_next_link = mock_get_next;
    cb.query_creature_ai_spec_flags = mock_ai_spec;
    cb.is_container_map = mock_is_map;
    cb.is_container_moneybox = mock_is_moneybox;
    cb.add_possession_index = mock_add_poss;

    /* Test 1: empty chain (link = END) should just write terminator 0-bit. */
    mock_init();
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_record_checkcode(&session, &cb,
        DM2_RECORD_LINK_END, 0, 1);
    assert(rc == 0);
    /* Should have flushed at least the terminator bit (partial byte). */

    /* Test 2: single type-0 record (4 bytes, no chain). */
    mock_init();
    g_mock_records[0][0] = 0x12;
    g_mock_records[0][1] = 0x34;
    g_mock_records[0][2] = 0x56;
    g_mock_records[0][3] = 0x78;
    uint16_t link0 = mock_make_link(0, 0);

    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_record_checkcode(&session, &cb, link0, 0, 0);
    assert(rc == 0);
    {
        size_t flush_w;
        dm2_suppress_writer_flush(&session.writer,
            buf + session.out_written,
            sizeof(buf) - session.out_written, &flush_w);
        session.out_written += flush_w;
    }
    assert(session.out_written > 0);

    /* Test 3: type-5 record (triggers 1-bit + 4-bit type write). */
    mock_init();
    g_mock_records[0][0] = 0xAA;
    g_mock_records[0][1] = 0xBB;
    g_mock_records[0][2] = 0xCC;
    g_mock_records[0][3] = 0xDD;
    uint16_t link5 = mock_make_link(0, 5);

    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_record_checkcode(&session, &cb, link5, 0, 0);
    assert(rc == 0);

    /* Test 4: two-record chain (type 0 -> type 0). */
    mock_init();
    g_mock_records[0][0] = 0x01;
    g_mock_records[0][1] = 0x02;
    g_mock_records[0][2] = 0x03;
    g_mock_records[0][3] = 0x04;
    uint16_t link_a = mock_make_link(0, 0);
    g_mock_records[1][0] = 0x05;
    g_mock_records[1][1] = 0x06;
    g_mock_records[1][2] = 0x07;
    g_mock_records[1][3] = 0x08;
    uint16_t link_b = mock_make_link(1, 0);
    g_mock_next[0] = link_b;

    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_record_checkcode(&session, &cb, link_a, 0, 1);
    assert(rc == 0);

    /* Test 5: creature record (type 4) with empty possession chain. */
    mock_init();
    memset(g_mock_records[0], 0, 16);
    g_mock_records[0][2] = 0xFF;
    g_mock_records[0][3] = 0xFF;
    g_mock_records[0][4] = 0x42;
    uint16_t link_creature = mock_make_link(0, 4);

    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_record_checkcode(&session, &cb, link_creature, 0, 0);
    assert(rc == 0);
    assert(session.creature_count == 1);
    assert(creature_idx[0] == 0);

    /* Test 6: NULL callbacks should return -1. */
    rc = dm2_v1_write_record_checkcode(NULL, &cb, link0, 0, 0);
    assert(rc == -1);

    printf("PASS: dm2_v1_save_write_record_checkcode\n");
    return 0;
}
