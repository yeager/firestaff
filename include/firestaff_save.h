
#ifndef FIRESTAFF_SAVE_H
#define FIRESTAFF_SAVE_H

#include "firestaff_game_loop.h"

/* Firestaff Save System — unified save/load for all games. */

#define FS_SAVE_MAGIC "FSSV"
#define FS_SAVE_VERSION 1
#define FS_MAX_SAVE_SLOTS 10

typedef struct {
    char magic[4];
    uint32_t version;
    uint32_t game_id;
    uint32_t game_version;
    uint32_t play_time_seconds;
    int level;
    int party_x, party_y, party_dir;
    uint32_t timestamp;
    /* Game-specific data follows */
} FS_SaveHeader;

int fs_save_game(const FS_GameState *state, int slot);
int fs_load_game(FS_GameState *state, int slot);
int fs_save_exists(int game_id, int slot);
const char *fs_save_slot_path(int game_id, int slot, char *buf, int bufsize);

#endif

