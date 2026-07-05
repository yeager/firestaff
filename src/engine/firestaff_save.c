
#include "firestaff_save.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *fs_save_game_name(int game_id) {
    /* Game IDs: 0=dm1, 1=csb, 2=dm2, 3=nexus, 4=theron */
    static const char* const game_names[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    if (game_id < 0 || game_id > 4) return NULL;
    return game_names[game_id];
}

const char *fs_save_slot_path_in_dir(const char *save_dir,
                                     int game_id,
                                     int slot,
                                     char *buf,
                                     int bufsize) {
    const char *game_name = fs_save_game_name(game_id);
    int n;
    if (!game_name || !buf || bufsize <= 0 || slot < 0) return NULL;
    if (save_dir && save_dir[0] != '\0') {
        n = snprintf(buf, (size_t)bufsize, "%s/%s_slot%d.sav",
                     save_dir, game_name, slot);
    } else {
        n = snprintf(buf, (size_t)bufsize, "saves/%s_slot%d.sav",
                     game_name, slot);
    }
    if (n <= 0 || n >= bufsize) return NULL;
    return buf;
}

const char *fs_save_slot_path(int game_id, int slot, char *buf, int bufsize) {
    return fs_save_slot_path_in_dir(NULL, game_id, slot, buf, bufsize);
}

int fs_save_game(const FS_GameState *state, int slot) {
    char path[256];
    FILE *f;
    FS_SaveHeader hdr;
    if (!state) return -1;
    if (!fs_save_slot_path_in_dir(state->config.save_dir,
                                  state->config.game,
                                  slot,
                                  path,
                                  sizeof(path))) {
        return -1;
    }
    if (state->config.save_dir && state->config.save_dir[0] != '\0') {
        if (!FSP_CreateDirectoryRecursive(state->config.save_dir)) {
            return -1;
        }
    } else {
        if (!FSP_CreateDirectoryRecursive("saves")) {
            return -1;
        }
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    memcpy(hdr.magic, FS_SAVE_MAGIC, 4);
    hdr.version = FS_SAVE_VERSION;
    hdr.game_id = state->config.game;
    hdr.game_version = state->config.version;
    hdr.level = state->current_level;
    hdr.party_x = state->party_x;
    hdr.party_y = state->party_y;
    hdr.party_dir = state->party_direction;
    hdr.timestamp = (uint32_t)time(NULL);
    fwrite(&hdr, sizeof(hdr), 1, f);
    fclose(f);
    return 0;
}

int fs_load_game(FS_GameState *state, int slot) {
    char path[256];
    FILE *f;
    FS_SaveHeader hdr;
    if (!state) return -1;
    if (!fs_save_slot_path_in_dir(state->config.save_dir,
                                  state->config.game,
                                  slot,
                                  path,
                                  sizeof(path))) {
        return -1;
    }
    f = fopen(path, "rb");
    if (!f) return -1;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return -1; }
    if (memcmp(hdr.magic, FS_SAVE_MAGIC, 4) != 0) { fclose(f); return -1; }
    state->current_level = hdr.level;
    state->party_x = hdr.party_x;
    state->party_y = hdr.party_y;
    state->party_direction = hdr.party_dir;
    fclose(f);
    return 0;
}

int fs_save_exists_in_dir(const char *save_dir, int game_id, int slot) {
    char path[256];
    FILE *f;
    if (!fs_save_slot_path_in_dir(save_dir, game_id, slot,
                                  path, sizeof(path))) {
        return 0;
    }
    f = fopen(path, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

int fs_save_exists(int game_id, int slot) {
    return fs_save_exists_in_dir(NULL, game_id, slot);
}
