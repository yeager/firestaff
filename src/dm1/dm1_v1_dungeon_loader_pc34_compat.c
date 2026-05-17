#include "dm1_v1_dungeon_loader_pc34_compat.h"
#include <stdio.h>
#include <string.h>

void m11_dl_init(M11_DL_DungeonState *state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_DL_DungeonState));
    state->step_east[0] = 0; state->step_east[1] = 1; state->step_east[2] = 0; state->step_east[3] = -1;
    state->step_north[0] = -1; state->step_north[1] = 0; state->step_north[2] = 1; state->step_north[3] = 0;
    state->thing_byte_count[0] = 4; state->thing_byte_count[1] = 6; state->thing_byte_count[2] = 4;
    state->thing_byte_count[3] = 8; state->thing_byte_count[4] = 16; state->thing_byte_count[5] = 4;
    state->thing_byte_count[6] = 4; state->thing_byte_count[7] = 4; state->thing_byte_count[8] = 4;
    state->thing_byte_count[9] = 8; state->thing_byte_count[10] = 4; state->thing_byte_count[11] = 0;
    state->thing_byte_count[12] = 0; state->thing_byte_count[13] = 0; state->thing_byte_count[14] = 8;
    state->thing_byte_count[15] = 0;
    state->loaded = false;
}

bool m11_dl_load_from_file(M11_DL_DungeonState *state, const char *path) {
    if (!state || !path) return false;
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    if (fread(&state->header, sizeof(M11_DL_DungeonHeader), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (fseek(f, state->header.map_data_offset, SEEK_SET) != 0) {
        fclose(f);
        return false;
    }

    for (uint8_t lvl = 0; lvl < state->header.level_count && lvl < DM1_MAX_LEVELS; lvl++) {
        uint8_t w = state->header.levels[lvl].width;
        uint8_t h = state->header.levels[lvl].height;
        if (w > DM1_MAX_MAP_W || h > DM1_MAX_MAP_H) {
            fclose(f);
            return false;
        }
        size_t tile_count = (size_t)w * h;
        if (fread(state->tiles[lvl], sizeof(M11_DL_Tile), tile_count, f) != tile_count) {
            fclose(f);
            return false;
        }
    }

    /* Tile layout: column-major per ReDMCSB DUNGEON.C F0151.
     * tiles[level][x][y] — C stores tiles[x] as contiguous column of height elements.
     * Collision system uses dm1_tile_index(x, y, height) = x * height + y. */
    state->loaded = true;
    fclose(f);
    return true;
}

const M11_DL_Tile *m11_dl_get_tile(const M11_DL_DungeonState *state, uint8_t level, uint8_t x, uint8_t y) {
    if (!state || !state->loaded || level >= DM1_MAX_LEVELS || x >= DM1_MAX_MAP_W || y >= DM1_MAX_MAP_H) {
        return NULL;
    }
    return (const M11_DL_Tile *)&state->tiles[level][x][y];
}

void m11_dl_step_forward(int *x, int *y, uint8_t dir) {
    if (!x || !y) return;
    if (dir > 3) return;
    static const int8_t step_east[4] = {0, 1, 0, -1};
    static const int8_t step_north[4] = {-1, 0, 1, 0};
    *x += step_east[dir];
    *y += step_north[dir];
}

void m11_dl_cleanup(M11_DL_DungeonState *state) {
    if (state) {
        state->loaded = false;
        memset(&state->header, 0, sizeof(M11_DL_DungeonHeader));
        memset(state->tiles, 0, sizeof(state->tiles));
    }
}
