#include "csb_v1_game.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <string.h>

/* Known CSB DUNGEON.DAT MD5 hashes (all verified platforms share the same hash) */
static const char *const csb_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",  /* CSB Atari ST + Amiga (EN) */
    NULL
};

void csb_v1_init(CSB_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 5;
    state->party_y = 5;
    state->party_dir = 0;
    state->difficulty = 1;
}

int csb_v1_load_dungeon(CSB_V1_GameState *state) {
    char path[ASSET_PATH_MAX];
    if (!state || !state->data_dir) return -1;

    /* Hash-based search: find any file matching known CSB dungeon hashes,
     * regardless of filename or directory layout */
    if (!asset_find_by_md5_list(state->data_dir, csb_dungeon_hashes,
                                path, sizeof(path), NULL, 4)) {
        printf("CSB: no dungeon file found matching known hashes in %s\n",
               state->data_dir);
        return -1;
    }
    printf("CSB: found dungeon at %s (hash-verified)\n", path);
    /* TODO: parse CSB DUNGEON.DAT */
    return 0;
}

int csb_v1_import_dm1_save(CSB_V1_GameState *state, const char *dm1_save_path) {
    if (!state || !dm1_save_path) return -1;
    printf("CSB: importing DM1 champions from %s\n", dm1_save_path);
    state->dm1_import_done = 1;
    return 0;
}
