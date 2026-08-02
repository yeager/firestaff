/* Test DM2 STORE_EXTRA_DUNGEON_DATA with mock dungeon. */
#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock dungeon: 2 maps, each 2x2 tiles. */
#define MOCK_MAPS 2
#define MOCK_W 2
#define MOCK_H 2

static uint8_t g_tiles[MOCK_MAPS][MOCK_W][MOCK_H];
static int g_current_map;
static int g_map_change_count;

static void mock_change_map(void *ctx, int idx) {
    (void)ctx;
    g_current_map = idx;
    g_map_change_count++;
}

static uint8_t mock_get_tile(void *ctx, int x, int y) {
    (void)ctx;
    return g_tiles[g_current_map][x][y];
}

static uint16_t mock_get_record_link(void *ctx, int x, int y) {
    (void)ctx; (void)x; (void)y;
    return DM2_RECORD_LINK_NONE;
}

static int mock_get_teleporter_detail(void *ctx, DM2_TeleporterDetail *out,
                                       int x, int y) {
    (void)ctx; (void)x; (void)y;
    memset(out, 0, sizeof(*out));
    out->bytes[4] = 0;
    return 1;
}

static int mock_get_map_count(void *ctx) {
    (void)ctx;
    return MOCK_MAPS;
}

static void mock_get_map_dims(void *ctx, int *w, int *h) {
    (void)ctx;
    *w = MOCK_W;
    *h = MOCK_H;
}

static int mock_init_suppress(void *ctx) {
    (void)ctx;
    return 0;
}

/* Minimal record callbacks (no records in tiles). */
static int mock_get_record(void *ctx, uint16_t link, DM2_WriteRecordData *out) {
    (void)ctx; (void)link; (void)out;
    return -1;
}

static uint16_t mock_get_next(void *ctx, uint16_t link) {
    (void)ctx; (void)link;
    return DM2_RECORD_LINK_END;
}

int main(void) {
    DM2_WriteRecordSession session;
    DM2_WriteRecordCallbacks rec_cb;
    DM2_StoreExtraDungeonCallbacks dung_cb;
    uint8_t buf[512];
    int creature_idx[16], container_idx[16];

    memset(&rec_cb, 0, sizeof(rec_cb));
    rec_cb.get_record = mock_get_record;
    rec_cb.get_next_link = mock_get_next;

    memset(&dung_cb, 0, sizeof(dung_cb));
    dung_cb.change_current_map = mock_change_map;
    dung_cb.get_tile = mock_get_tile;
    dung_cb.get_record_link = mock_get_record_link;
    dung_cb.get_teleporter_detail = mock_get_teleporter_detail;
    dung_cb.get_map_count = mock_get_map_count;
    dung_cb.get_map_dimensions = mock_get_map_dims;
    dung_cb.init_suppress = mock_init_suppress;

    /* Test 1: all tiles type 0 (wall), no records. Should just write terminators. */
    memset(g_tiles, 0, sizeof(g_tiles));
    g_map_change_count = 0;
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    int rc = dm2_v1_store_extra_dungeon_data(&session, &rec_cb, &dung_cb, 0);
    assert(rc == 0);
    assert(g_map_change_count == MOCK_MAPS + 1);

    /* Test 2: tile type 2 (pit, mask=0x08) should write SUPPRESS data. */
    memset(g_tiles, 0, sizeof(g_tiles));
    g_tiles[0][0][0] = (2 << 5);
    g_map_change_count = 0;
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_store_extra_dungeon_data(&session, &rec_cb, &dung_cb, 0);
    assert(rc == 0);

    /* Test 3: tile type 5 (teleporter) with backward ref should skip chain. */
    memset(g_tiles, 0, sizeof(g_tiles));
    g_tiles[1][0][0] = (5 << 5);
    g_map_change_count = 0;
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_store_extra_dungeon_data(&session, &rec_cb, &dung_cb, 0);
    assert(rc == 0);

    /* Test 4: NULL params. */
    rc = dm2_v1_store_extra_dungeon_data(NULL, &rec_cb, &dung_cb, 0);
    assert(rc == -1);

    printf("PASS: dm2_v1_save_store_extra_dungeon_data\n");
    return 0;
}
