
#include "csb_v1_game.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>

void csb_v1_init(CSB_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 5;  /* CSB start position (different from DM1) */
    state->party_y = 5;
    state->party_dir = 0;
    state->difficulty = 1; /* 1 = normal CSB difficulty (1.5x vs DM1) */
}

int csb_v1_load_dungeon(CSB_V1_GameState *state) {
    char path[512];
    if (!state || !state->data_dir) return -1;
    snprintf(path, sizeof(path), "%s/csb/DUNGEON.DAT", state->data_dir);
    printf("CSB: loading dungeon from %s\n", path);
    /* CSB DUNGEON.DAT is only 2098 bytes — likely single level or compressed */
    /* TODO: parse CSB DUNGEON.DAT */
    printf("CSB: would load %s\n", path);
    return 0;
}

int csb_v1_import_dm1_save(CSB_V1_GameState *state, const char *dm1_save_path) {
    if (!state || !dm1_save_path) return -1;
    printf("CSB: importing DM1 champions from %s\n", dm1_save_path);
    state->dm1_import_done = 1;
    return 0;
}

