/* Test DM2 save orchestrator — full pipeline with mock callbacks. */
#include "dm2_v1_save_orchestrator_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock state. */
static uint8_t g_raw_output[4096];
static size_t g_raw_pos;
static uint8_t g_hero_data[4][263];
static uint8_t g_globalb[64];
static uint8_t g_globalw[128];
static uint8_t g_v1e0104[8];
static uint8_t g_save_state[6];
static uint8_t g_timer_data[32];

static int mock_write_raw(void *ctx, const uint8_t *data, size_t size)
{
    (void)ctx;
    if (g_raw_pos + size > sizeof(g_raw_output)) return -1;
    memcpy(g_raw_output + g_raw_pos, data, size);
    g_raw_pos += size;
    return 0;
}

static int mock_get_header(void *ctx, uint8_t *out)
{
    (void)ctx;
    memset(out, 0x42, DM2_SAVE_HEADER_SIZE);
    out[0] = 1; /* version */
    return 0;
}

static int mock_get_sgwords(void *ctx, uint8_t *out)
{
    (void)ctx;
    memset(out, 0, DM2_SAVE_SGWORDS_SIZE);
    return 0;
}

static const uint8_t *mock_get_raw_block(void *ctx, int block_id, size_t *size)
{
    (void)ctx; (void)block_id;
    *size = 0;
    return NULL;
}

static const uint8_t *mock_get_record_array(void *ctx, int type, size_t *count)
{
    (void)ctx; (void)type;
    *count = 0;
    return NULL;
}

static const uint8_t *mock_get_map_data(void *ctx, size_t *size)
{
    (void)ctx;
    *size = 0;
    return NULL;
}

static int mock_fill_sgbuf(void *ctx, DM2_SaveGameBuffer *buf)
{
    (void)ctx;
    memset(buf, 0, sizeof(*buf));
    buf->gametick = 12345;
    buf->heros_in_party = 2;
    return 0;
}

static const uint8_t *mock_get_globalb(void *ctx)
{
    (void)ctx;
    return g_globalb;
}

static const uint8_t *mock_get_v1e0104(void *ctx)
{
    (void)ctx;
    return g_v1e0104;
}

static const uint8_t *mock_get_globalw(void *ctx)
{
    (void)ctx;
    return g_globalw;
}

static const uint8_t *mock_get_hero_data(void *ctx, int hero_idx)
{
    (void)ctx;
    if (hero_idx < 0 || hero_idx >= 4) return NULL;
    return g_hero_data[hero_idx];
}

static int mock_get_hero_count(void *ctx)
{
    (void)ctx;
    return 2;
}

static const uint8_t *mock_get_save_state(void *ctx)
{
    (void)ctx;
    return g_save_state;
}

static const uint8_t *mock_get_timer_array(void *ctx, int *count)
{
    (void)ctx;
    *count = 2;
    return g_timer_data;
}

static size_t mock_get_timer_entry_size(void *ctx)
{
    (void)ctx;
    return 16;
}

static uint16_t mock_get_hero_item_link(void *ctx, int hero_idx, int slot)
{
    (void)ctx; (void)hero_idx; (void)slot;
    return DM2_RECORD_LINK_END;
}

static uint16_t mock_get_wpc_link(void *ctx)
{
    (void)ctx;
    return DM2_RECORD_LINK_END;
}

static uint16_t mock_get_sgwords_field(void *ctx, int idx)
{
    (void)ctx; (void)idx;
    return 0;
}

/* Write record callbacks (no-op since all links are END). */
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

/* Dungeon callbacks (minimal — 0 maps). */
static int mock_get_map_count(void *ctx) { (void)ctx; return 0; }
static void mock_get_map_dims(void *ctx, int *w, int *h)
{
    (void)ctx; *w = 0; *h = 0;
}
static void mock_change_map(void *ctx, int m) { (void)ctx; (void)m; }
static uint8_t mock_get_tile(void *ctx, int x, int y) { (void)ctx; (void)x; (void)y; return 0; }
static uint16_t mock_get_rec_link(void *ctx, int x, int y)
{
    (void)ctx; (void)x; (void)y;
    return DM2_RECORD_LINK_END;
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

/* Possession callbacks (no-op). */
static int mock_resolve_poss(void *ctx, uint16_t link)
{
    (void)ctx; (void)link;
    return 0;
}

static DM2_SaveOrchestratorCallbacks make_callbacks(void)
{
    DM2_SaveOrchestratorCallbacks cb;
    memset(&cb, 0, sizeof(cb));

    cb.write_raw = mock_write_raw;
    cb.get_header = mock_get_header;
    cb.get_sgwords = mock_get_sgwords;
    cb.get_raw_block = mock_get_raw_block;
    cb.get_record_array = mock_get_record_array;
    cb.get_map_data = mock_get_map_data;
    cb.fill_savegame_buffer = mock_fill_sgbuf;
    cb.get_globalb = mock_get_globalb;
    cb.get_v1e0104 = mock_get_v1e0104;
    cb.get_globalw = mock_get_globalw;
    cb.get_hero_data = mock_get_hero_data;
    cb.get_hero_count = mock_get_hero_count;
    cb.get_save_state = mock_get_save_state;
    cb.get_timer_array = mock_get_timer_array;
    cb.get_timer_entry_size = mock_get_timer_entry_size;
    cb.get_hero_item_link = mock_get_hero_item_link;
    cb.get_wpc_link = mock_get_wpc_link;
    cb.get_sgwords_field = mock_get_sgwords_field;

    cb.write_record_cb.get_record = mock_get_record;
    cb.write_record_cb.get_next_link = mock_get_next;

    cb.dungeon_cb.get_map_count = mock_get_map_count;
    cb.dungeon_cb.get_map_dimensions = mock_get_map_dims;
    cb.dungeon_cb.change_current_map = mock_change_map;
    cb.dungeon_cb.get_tile = mock_get_tile;
    cb.dungeon_cb.get_record_link = mock_get_rec_link;
    cb.dungeon_cb.get_teleporter_detail = mock_get_tp;
    cb.dungeon_cb.init_suppress = mock_init_suppress;

    cb.possession_cb.resolve_possession_index = mock_resolve_poss;

    return cb;
}

static void test_null_safety(void)
{
    assert(dm2_v1_save_orchestrate(NULL, NULL, 0, NULL) == -1);
    printf("  PASS: null_safety\n");
}

static void test_empty_save(void)
{
    uint8_t suppress_buf[4096];
    DM2_SaveOrchestratorCallbacks cb = make_callbacks();
    DM2_SaveOrchestratorResult result;

    g_raw_pos = 0;
    memset(g_globalb, 0x11, sizeof(g_globalb));
    memset(g_globalw, 0x22, sizeof(g_globalw));
    memset(g_v1e0104, 0x33, sizeof(g_v1e0104));
    memset(g_hero_data, 0, sizeof(g_hero_data));
    memset(g_save_state, 0, sizeof(g_save_state));
    memset(g_timer_data, 0, sizeof(g_timer_data));

    int rc = dm2_v1_save_orchestrate(&cb, suppress_buf, sizeof(suppress_buf), &result);
    assert(rc == 0);
    assert(result.error == 0);

    /* Raw section: header (0x2a) + sgwords (0x2c) = 0x56 bytes minimum. */
    assert(g_raw_pos == DM2_SAVE_HEADER_SIZE + DM2_SAVE_SGWORDS_SIZE);

    /* Verify header starts with version=1. */
    assert(g_raw_output[0] == 1);

    /* SUPPRESS section should have some output. */
    assert(result.suppress_bits_written > 0);

    printf("  PASS: empty_save\n");
}

static void test_raw_header_contents(void)
{
    uint8_t suppress_buf[4096];
    DM2_SaveOrchestratorCallbacks cb = make_callbacks();
    DM2_SaveOrchestratorResult result;

    g_raw_pos = 0;
    memset(g_hero_data, 0, sizeof(g_hero_data));
    memset(g_timer_data, 0, sizeof(g_timer_data));

    int rc = dm2_v1_save_orchestrate(&cb, suppress_buf, sizeof(suppress_buf), &result);
    assert(rc == 0);

    /* Header byte 1 should be 0x42 (our mock fill). */
    assert(g_raw_output[1] == 0x42);

    printf("  PASS: raw_header_contents\n");
}

static void test_suppress_output_nonzero(void)
{
    uint8_t suppress_buf[8192];
    DM2_SaveOrchestratorCallbacks cb = make_callbacks();
    DM2_SaveOrchestratorResult result;

    g_raw_pos = 0;
    for (int i = 0; i < 64; i++) g_globalb[i] = (uint8_t)(i * 3);
    for (int i = 0; i < 128; i++) g_globalw[i] = (uint8_t)(i * 5);
    memset(g_hero_data[0], 0xAA, 263);
    memset(g_hero_data[1], 0x55, 263);
    memset(g_timer_data, 0, sizeof(g_timer_data));

    int rc = dm2_v1_save_orchestrate(&cb, suppress_buf, sizeof(suppress_buf), &result);
    assert(rc == 0);

    /* With nonzero data, SUPPRESS output should be substantial. */
    assert(result.suppress_bits_written > 50);

    printf("  PASS: suppress_output_nonzero\n");
}

int main(void) {
    test_null_safety();
    test_empty_save();
    test_raw_header_contents();
    test_suppress_output_nonzero();

    printf("PASS: dm2_v1_save_orchestrator (4 tests)\n");
    return 0;
}
