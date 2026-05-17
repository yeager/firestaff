/* DM1 V1 Dungeon DAT Decompressor — source-locked from ReDMCSB
 * DECOMPDU.C F0455_FLOPPY_DecompressDungeon: main decompression entry
 * DECOMPDU.C G0525_l_GameID, G0526_ui_DungeonID, G0527_i_Platform, G0528_i_Format
 * DECOMPDU.C G0530_B_LoadingCompressedDungeon, G0531_puc_DecompressedDungeonCurrentPosition
 * DECOMPDU.C G0532_l_DecompressedDungeonRemainingByteCount */

#include "dm1_v1_dungeon_decompressor_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

void m11_dd_init(M11_DD_State* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_DD_State));
}

/* Parse file header — mirrors G0525-G0528 assignment in DECOMPDU.C */
static bool parse_header(M11_DD_State* state, const uint8_t* data, size_t size) {
    if (size < 16) return false;

    /* DECOMPDU.C reads: GameID(4), DungeonID(2), Platform(2), Format(2) */
    memcpy(&state->header.game_id, data, 4);
    memcpy(&state->header.dungeon_id, data + 4, 2);
    memcpy(&state->header.platform, data + 6, 2);
    memcpy(&state->header.format, data + 8, 2);
    memcpy(&state->header.level_count, data + 10, 2);
    memcpy(&state->header.total_size, data + 12, 4);

    if (state->header.level_count > DM1_DD_MAX_LEVELS)
        state->header.level_count = DM1_DD_MAX_LEVELS;

    return true;
}

/* Parse level headers from data following file header */
static bool parse_level_headers(M11_DD_State* state, const uint8_t* data,
                                 size_t size, size_t offset) {
    for (uint16_t i = 0; i < state->header.level_count; i++) {
        size_t pos = offset + (size_t)i * 16;
        if (pos + 16 > size) return false;

        M11_DD_LevelHeader* lh = &state->levels[i];
        memcpy(&lh->width, data + pos, 2);
        memcpy(&lh->height, data + pos + 2, 2);
        memcpy(&lh->difficulty, data + pos + 4, 2);
        memcpy(&lh->creature_count, data + pos + 6, 2);
        memcpy(&lh->room_count, data + pos + 8, 2);
        memcpy(&lh->data_offset, data + pos + 10, 4);
        memcpy(&lh->data_size, data + pos + 14, 2);

        if (lh->width > DM1_DD_MAX_MAP_DIM) lh->width = DM1_DD_MAX_MAP_DIM;
        if (lh->height > DM1_DD_MAX_MAP_DIM) lh->height = DM1_DD_MAX_MAP_DIM;
    }
    return true;
}

bool m11_dd_load_file(M11_DD_State* state, const uint8_t* data, size_t size) {
    if (!state || !data || size < 16) return false;

    if (!parse_header(state, data, size)) return false;
    if (!parse_level_headers(state, data, size, 16)) return false;

    /* G0530 pattern: mark as compressed, store buffer pointer */
    state->compressed = true;
    state->decomp_buffer = (uint8_t*)malloc(size);
    if (!state->decomp_buffer) return false;
    memcpy(state->decomp_buffer, data, size);
    state->decomp_remaining = size;
    state->loaded = true;
    return true;
}

uint16_t m11_dd_get_level_count(const M11_DD_State* state) {
    if (!state || !state->loaded) return 0;
    return state->header.level_count;
}

/* F0455 pattern: decompress level data into tile map */
bool m11_dd_decompress_level(M11_DD_State* state, int level,
                              uint8_t* output, size_t out_size) {
    if (!state || !state->loaded) return false;
    if (level < 0 || level >= (int)state->header.level_count) return false;

    const M11_DD_LevelHeader* lh = &state->levels[level];
    uint32_t off = lh->data_offset;
    uint32_t sz = lh->data_size;

    if (off + sz > state->decomp_remaining) return false;
    if (output && out_size >= sz) {
        memcpy(output, state->decomp_buffer + off, sz);
    }

    /* Parse tiles from level data into tile_map */
    const uint8_t* ldata = state->decomp_buffer + off;
    size_t tile_offset = 0;
    for (uint16_t x = 0; x < lh->width && tile_offset + 4 <= sz; x++) {
        for (uint16_t y = 0; y < lh->height && tile_offset + 4 <= sz; y++) {
            M11_DD_Tile* t = &state->tile_map[level][y][x];
            t->type = ldata[tile_offset];
            t->attributes = ldata[tile_offset + 1];
            t->thing_index = (uint16_t)(ldata[tile_offset + 2] |
                             ((uint16_t)ldata[tile_offset + 3] << 8));
            t->wall_ornament = 0;
            t->floor_ornament = 0;
            tile_offset += 4;
        }
    }

    state->level_loaded[level] = true;
    return true;
}

const M11_DD_LevelHeader* m11_dd_get_level_header(const M11_DD_State* state, int level) {
    if (!state || !state->loaded) return NULL;
    if (level < 0 || level >= (int)state->header.level_count) return NULL;
    return &state->levels[level];
}

const M11_DD_Tile* m11_dd_get_tile(const M11_DD_State* state, int level,
                                    int16_t x, int16_t y) {
    if (!state || !state->loaded) return NULL;
    if (level < 0 || level >= (int)state->header.level_count) return NULL;
    if (!state->level_loaded[level]) return NULL;
    const M11_DD_LevelHeader* lh = &state->levels[level];
    if (x < 0 || x >= (int16_t)lh->width || y < 0 || y >= (int16_t)lh->height)
        return NULL;
    return &state->tile_map[level][y][x];
}

const M11_DD_Creature* m11_dd_get_creature(const M11_DD_State* state, int level,
                                            int16_t x, int16_t y) {
    if (!state || !state->loaded) return NULL;
    if (level < 0 || level >= (int)state->header.level_count) return NULL;
    uint16_t start = state->creature_offsets[level];
    uint16_t count = state->creature_counts[level];
    for (uint16_t i = start; i < start + count; i++) {
        if (state->creatures[i].x == x && state->creatures[i].y == y)
            return &state->creatures[i];
    }
    return NULL;
}

void m11_dd_close(M11_DD_State* state) {
    if (!state) return;
    if (state->decomp_buffer) {
        free(state->decomp_buffer);
        state->decomp_buffer = NULL;
    }
    state->loaded = false;
    state->decomp_remaining = 0;
}
