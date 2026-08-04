/*
 * test_dm2_v1_sfx_pc34_compat.c -- unit tests for DM2 sound effects.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_sfx_pc34_compat.h"

/* ── Test volume/pan calculation ────────────────────────────────────── */

static void test_volume_pan_calc(void)
{
    int16_t vol, pan;

    /* Max volume, no distance offset */
    dm2_v1_sfx_calc_volume_pan(7, 0, 0, &vol, &pan);
    assert(vol == 255);
    assert(pan == 0x80); /* center */

    /* Positive dx shifts pan right */
    dm2_v1_sfx_calc_volume_pan(7, 10, 0, &vol, &pan);
    assert(pan > 0x80);

    /* Negative dx shifts pan left */
    dm2_v1_sfx_calc_volume_pan(7, -10, 0, &vol, &pan);
    assert(pan < 0x80);

    /* Lower vol_level reduces volume */
    dm2_v1_sfx_calc_volume_pan(3, 0, 0, &vol, &pan);
    assert(vol < 255);
    assert(vol > 0);

    /* Pan clamped to [0, 255] */
    dm2_v1_sfx_calc_volume_pan(7, 127, 0, &vol, &pan);
    assert(pan >= 0 && pan <= 255);

    dm2_v1_sfx_calc_volume_pan(7, -128, 0, &vol, &pan);
    assert(pan >= 0 && pan <= 255);

    printf("  PASS: volume_pan_calc\n");
}

/* ── Test state init ────────────────────────────────────────────────── */

static void test_state_init(void)
{
    DM2_V1_SfxState state;
    memset(&state, 0xFF, sizeof(state));

    dm2_v1_sfx_state_init(&state);
    assert(state.sample_index == 0);
    assert(state.queued_count == 0);
    assert(state.queued_neg_count == 0);

    printf("  PASS: state_init\n");
}

/* ── Sound distance mock callbacks ──────────────────────────────────── */

static uint8_t mock_view_data[32 * 32];
static int16_t mock_map_height = 10;

static int16_t mock_get_current_map(void *ctx)  { (void)ctx; return 1; }
static int16_t mock_get_view_map1(void *ctx)     { (void)ctx; return 1; }
static int16_t mock_get_view_map2(void *ctx)     { (void)ctx; return 2; }
static uint8_t *mock_get_view_data1(void *ctx)   { (void)ctx; return mock_view_data; }
static uint8_t *mock_get_view_data2(void *ctx)   { (void)ctx; return NULL; }
static int16_t mock_get_view_width1(void *ctx)   { (void)ctx; return 10; }
static int16_t mock_get_view_width2(void *ctx)   { (void)ctx; return 10; }
static int16_t mock_get_map_height(void *ctx)    { (void)ctx; return mock_map_height; }

static void test_sound_distance(void)
{
    DM2_V1_SfxCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_current_map = mock_get_current_map;
    cb.get_view_map1   = mock_get_view_map1;
    cb.get_view_map2   = mock_get_view_map2;
    cb.get_view_data1  = mock_get_view_data1;
    cb.get_view_data2  = mock_get_view_data2;
    cb.get_view_width1 = mock_get_view_width1;
    cb.get_view_width2 = mock_get_view_width2;
    cb.get_map_height  = mock_get_map_height;
    cb.ctx = NULL;

    /* Set up view data: distance 5 at (3,3) */
    memset(mock_view_data, 0, sizeof(mock_view_data));
    mock_view_data[(3 << 5) + 3] = 5;

    DM2_V1_SfxSoundDistanceReceipt r =
        dm2_v1_sfx_calc_sound_distance(&cb, 3, 3);
    assert(r.valid);
    assert(r.distance == 4); /* val - 1 */

    /* Out of range */
    r = dm2_v1_sfx_calc_sound_distance(&cb, 15, 3);
    assert(!r.valid);

    /* Wrong map */
    cb.get_current_map = mock_get_view_map2; /* map 2, but data1 is map 1 */
    /* Need to set view_data2 */
    cb.get_view_data2 = mock_get_view_data1;
    r = dm2_v1_sfx_calc_sound_distance(&cb, 3, 3);
    assert(r.valid);

    printf("  PASS: sound_distance\n");
}

/* ── Test queue noise gen2 routing ──────────────────────────────────── */

static int16_t mock_query_snd_found(void *ctx, int8_t a, int8_t b, int8_t c)
{
    (void)ctx; (void)a; (void)b; (void)c;
    return 1; /* found */
}

static int16_t mock_get_zero(void *ctx) { (void)ctx; return 0; }
static int16_t mock_get_party_map(void *ctx) { (void)ctx; return 1; }
static int32_t mock_get_game_tick(void *ctx) { (void)ctx; return 100; }

static void test_queue_noise_gen2(void)
{
    DM2_V1_SfxCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_snd_entry_index = mock_query_snd_found;
    cb.get_current_map       = mock_get_current_map;
    cb.get_party_map         = mock_get_party_map;
    cb.get_party_alt_map     = mock_get_zero;
    cb.get_party_x           = mock_get_zero;
    cb.get_party_y           = mock_get_zero;
    cb.get_party_dir         = mock_get_zero;
    cb.get_distance_halve_flag = mock_get_zero;
    cb.get_view_map1         = mock_get_view_map1;
    cb.get_view_map2         = mock_get_view_map2;
    cb.get_view_data1        = mock_get_view_data1;
    cb.get_view_data2        = mock_get_view_data2;
    cb.get_view_width1       = mock_get_view_width1;
    cb.get_view_width2       = mock_get_view_width2;
    cb.get_map_height        = mock_get_map_height;
    cb.get_game_tick         = mock_get_game_tick;
    cb.ctx = NULL;

    DM2_V1_SfxState state;
    dm2_v1_sfx_state_init(&state);

    /* When snd entry found, should use original type */
    DM2_V1_SfxQueueNoiseReceipt r = dm2_v1_sfx_queue_noise_gen2(
        &cb, &state, 0x0d, 0x10, 0x01, (int8_t)0xfe,
        5, 5, 1, 0x6c, 0xc8);
    /* Should have queued (or at least tried) */
    assert(r.queued);

    printf("  PASS: queue_noise_gen2\n");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_sfx_pc34_compat:\n");
    test_volume_pan_calc();
    test_state_init();
    test_sound_distance();
    test_queue_noise_gen2();
    printf("All SFX tests passed.\n");
    return 0;
}
