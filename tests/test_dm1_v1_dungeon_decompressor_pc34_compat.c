#include "dm1_v1_dungeon_decompressor_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    assert(state.loaded == false);
    assert(state.compressed == false);
}

static void test_load_null(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    bool ok = DM1_V1_DungeonDecompressor_LoadFilePc34Compat(&state, NULL, 0);
    (void)ok;
    assert(ok == false);
}

static void test_load_too_small(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    uint8_t buf[4] = {0};
    bool ok = DM1_V1_DungeonDecompressor_LoadFilePc34Compat(&state, buf, 4);
    (void)ok;
    assert(ok == false);
}

static void test_get_level_count_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    uint16_t count = DM1_V1_DungeonDecompressor_GetLevelCountPc34Compat(&state);
    (void)count;
    assert(count == 0);
}

static void test_get_level_header_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    const DM1_V1_DungeonDecompressorLevelHeaderPc34 *hdr =
        DM1_V1_DungeonDecompressor_GetLevelHeaderPc34Compat(&state, 0);
    (void)hdr;
    assert(hdr == NULL);
}

static void test_get_tile_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    const DM1_V1_DungeonDecompressorTilePc34 *t =
        DM1_V1_DungeonDecompressor_GetTilePc34Compat(&state, 0, 0, 0);
    (void)t;
    assert(t == NULL);
}

static void test_get_creature_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    const DM1_V1_DungeonDecompressorCreaturePc34 *c =
        DM1_V1_DungeonDecompressor_GetCreaturePc34Compat(&state, 0, 0, 0);
    (void)c;
    assert(c == NULL);
}

static void test_close_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    DM1_V1_DungeonDecompressor_ClosePc34Compat(&state);
    assert(state.loaded == false);
}

static void test_close_double(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    DM1_V1_DungeonDecompressor_ClosePc34Compat(&state);
    DM1_V1_DungeonDecompressor_ClosePc34Compat(&state);
}

static void test_decompress_unloaded(void)
{
    DM1_V1_DungeonDecompressorStatePc34 state;
    DM1_V1_DungeonDecompressor_InitPc34Compat(&state);
    uint8_t out[256];
    bool ok = DM1_V1_DungeonDecompressor_DecompressLevelPc34Compat(&state, 0, out, sizeof(out));
    (void)ok;
    assert(ok == false);
}

static void test_tile_type_constants(void)
{
    assert(DM1_DD_TILE_WALL == 0);
    assert(DM1_DD_TILE_OPEN == 1);
    assert(DM1_DD_TILE_PIT == 2);
    assert(DM1_DD_TILE_STAIRS == 3);
    assert(DM1_DD_TILE_DOOR == 4);
    assert(DM1_DD_TILE_TELEPORTER == 5);
    assert(DM1_DD_TILE_TRICK_WALL == 6);
}

static void test_max_constants(void)
{
    assert(DM1_DD_MAX_LEVELS == 16);
    assert(DM1_DD_MAX_MAP_DIM == 32);
}

int main(void)
{
    test_init();
    test_load_null();
    test_load_too_small();
    test_get_level_count_unloaded();
    test_get_level_header_unloaded();
    test_get_tile_unloaded();
    test_get_creature_unloaded();
    test_close_unloaded();
    test_close_double();
    test_decompress_unloaded();
    test_tile_type_constants();
    test_max_constants();

    puts("ok: DM1 dungeon decompressor (Q-DM1-04) 12 tests passed");
    return 0;
}
