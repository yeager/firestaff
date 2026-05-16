
#include "dm2_v1_game.h"
#include "firestaff_dungeon_query.h"
#include <string.h>
#include <stdio.h>

void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 15;
    state->party_y = 15;
    state->party_dir = 0;
    state->gold = 100;
    state->time_of_day = 720; /* noon (minutes) */
}

int dm2_v1_load_dungeon(DM2_V1_GameState *state) {
    char path[512];
    if (!state || !state->data_dir) return -1;
    snprintf(path, sizeof(path), "%s/dm2/DUNGEON.DAT", state->data_dir);
    printf("DM2: loading dungeon from %s (39 KB)\n", path);
    return fs_dungeon_load(path);
}

int dm2_v1_enter_shop(DM2_V1_GameState *state) {
    if (!state || !state->outdoor) return -1;
    printf("DM2: entering shop (gold: %d)\n", state->gold);
    return 0;
}

int dm2_v1_is_outdoor(const DM2_V1_GameState *state) {
    return state ? state->outdoor : 0;
}

