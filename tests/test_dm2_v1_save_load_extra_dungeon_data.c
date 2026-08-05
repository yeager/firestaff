/* Test DM2 LOAD_EXTRA_DUNGEON_DATA — round-trip with STORE_EXTRA_DUNGEON_DATA.
 * Source: sksvgame.cpp:1108-1400 (reader), 1958-2041 (writer). */

#include "dm2_v1_save_load_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Mock dungeon: 1 map, 2x2, all floors, no records ---- */

static int mock_get_map_count(void *ctx) { (void)ctx; return 1; }
static void mock_get_map_dims(void *ctx, int *w, int *h)
{
    (void)ctx; *w = 2; *h = 2;
}
static void mock_change_map(void *ctx, int m) { (void)ctx; (void)m; }
static uint8_t mock_get_tile(void *ctx, int x, int y)
{
    (void)ctx; (void)x; (void)y;
    return 0x00;
}
static uint16_t mock_get_rec_link(void *ctx, int x, int y)
{
    (void)ctx; (void)x; (void)y;
    return 0xFFFE;
}
static int mock_get_tp(void *ctx, DM2_TeleporterDetail *out, int x, int y)
{
    (void)ctx; (void)x; (void)y;
    memset(out, 0, sizeof(*out));
    return 0;
}
static int mock_init_suppress(void *ctx)
{
    (void)ctx;
    return 0;
}

/* ---- Writer mock callbacks ---- */

static int mock_get_record(void *ctx, uint16_t link, DM2_WriteRecordData *out)
{
    (void)ctx; (void)link; (void)out;
    return -1;
}
static uint16_t mock_get_next(void *ctx, uint16_t link)
{
    (void)ctx; (void)link;
    return DM2_RECORD_LINK_END;
}
static int mock_ai_spec(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_map(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static int mock_is_moneybox(void *ctx, uint16_t link) { (void)ctx; (void)link; return 0; }
static void mock_add_poss(void *ctx, uint16_t link) { (void)ctx; (void)link; }

/* ---- Reader mock pool ---- */

typedef struct {
    int alloc_count;
} ReadPool;

static uint16_t read_alloc(void *ctx, int record_type)
{
    ReadPool *pool = (ReadPool *)ctx;
    (void)record_type;
    return (uint16_t)pool->alloc_count++;
}
static int read_set_data(void *ctx, uint16_t link,
                         const uint8_t *data, size_t size)
{
    (void)ctx; (void)link; (void)data; (void)size;
    return 0;
}
static int read_chain(void *ctx, uint16_t prev, uint16_t next)
{
    (void)ctx; (void)prev; (void)next;
    return 0;
}

/* ---- Tests ---- */

static void test_null_safety(void)
{
    assert(dm2_v1_load_extra_dungeon_data(NULL, NULL, NULL, 0, NULL) == -1);
    printf("  PASS: null_safety\n");
}

static void test_empty_dungeon_round_trip(void)
{
    uint8_t buf[1024];
    int creature_idx[4], container_idx[4];

    /* Write */
    DM2_WriteRecordSession wr;
    dm2_v1_write_record_session_init(&wr, buf, sizeof(buf),
        creature_idx, 4, container_idx, 4, NULL, 0);

    DM2_WriteRecordCallbacks wcb;
    memset(&wcb, 0, sizeof(wcb));
    wcb.get_record = mock_get_record;
    wcb.get_next_link = mock_get_next;
    wcb.query_creature_ai_spec_flags = mock_ai_spec;
    wcb.is_container_map = mock_is_map;
    wcb.is_container_moneybox = mock_is_moneybox;
    wcb.add_possession_index = mock_add_poss;

    DM2_StoreExtraDungeonCallbacks dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.get_map_count = mock_get_map_count;
    dcb.get_map_dimensions = mock_get_map_dims;
    dcb.change_current_map = mock_change_map;
    dcb.get_tile = mock_get_tile;
    dcb.get_record_link = mock_get_rec_link;
    dcb.get_teleporter_detail = mock_get_tp;
    dcb.init_suppress = mock_init_suppress;

    int wrc = dm2_v1_store_extra_dungeon_data(&wr, &wcb, &dcb, 0);
    assert(wrc == 0);

    size_t fw;
    dm2_suppress_writer_flush(&wr.writer,
        buf + wr.out_written,
        sizeof(buf) - wr.out_written, &fw);
    wr.out_written += fw;

    /* Read back */
    DM2_ReadRecordSession rd;
    ReadPool pool;
    memset(&pool, 0, sizeof(pool));
    dm2_v1_read_record_session_init(&rd, buf, wr.out_written);

    DM2_ReadRecordCallbacks rcb;
    memset(&rcb, 0, sizeof(rcb));
    rcb.alloc_record = read_alloc;
    rcb.set_data = read_set_data;
    rcb.chain_record = read_chain;
    rcb.ctx = &pool;

    DM2_LoadExtraDungeonCallbacks ldcb;
    memset(&ldcb, 0, sizeof(ldcb));
    ldcb.get_map_count = mock_get_map_count;
    ldcb.get_map_dimensions = mock_get_map_dims;
    ldcb.change_current_map = mock_change_map;
    ldcb.get_tile = mock_get_tile;
    ldcb.get_teleporter_detail = mock_get_tp;

    DM2_V1_LoadExtraDungeonReceipt receipt;
    int rrc = dm2_v1_load_extra_dungeon_data(&rd, &rcb, &ldcb, 0, &receipt);
    assert(rrc == 0);
    assert(receipt.valid == 1);
    assert(receipt.maps_loaded == 1);
    assert(receipt.tiles_loaded == 4);
    assert(receipt.error == 0);
    assert(pool.alloc_count == 0);

    printf("  PASS: empty_dungeon_round_trip\n");
}

static void test_receipt_fields(void)
{
    DM2_V1_LoadExtraDungeonReceipt r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.maps_loaded == 0);
    assert(r.tiles_loaded == 0);
    assert(r.record_chains_loaded == 0);
    assert(r.teleporter_forward_refs_skipped == 0);
    printf("  PASS: receipt_fields\n");
}

int main(void)
{
    printf("test_dm2_v1_save_load_extra_dungeon_data:\n");
    test_null_safety();
    test_empty_dungeon_round_trip();
    test_receipt_fields();
    printf("All load_extra_dungeon_data tests passed.\n");
    return 0;
}
