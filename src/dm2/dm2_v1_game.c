#include "dm2_v1_game.h"
/* dungeon query stub */
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

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

/* DM2 dungeon file varies: DUNGEON.DAT, DM2DUNGEON.DAT, or inside zip */
static int dm2_find_dungeon(const char *data_dir, char *path, int pathlen) {
    struct stat st;
    static const char *fmts[] = {
        "%s/dm2/DUNGEON.DAT", "%s/dm2/DM2DUNGEON.DAT",
        "%s/DUNGEON.DAT", "%s/DM2DUNGEON.DAT", NULL
    };
    for (int i = 0; fmts[i]; i++) {
        snprintf(path, pathlen, fmts[i], data_dir);
        if (stat(path, &st) == 0) return 1;
    }
    snprintf(path, pathlen, "%s/dm2/DUNGEON.DAT", data_dir);
    return 0;
}

int dm2_v1_load_dungeon(DM2_V1_GameState *state) {
    char path[512];
    if (!state || !state->data_dir) return -1;
    int found = dm2_find_dungeon(state->data_dir, path, sizeof(path));
    if (!found) {
        printf("DM2: dungeon not found (searched dm2/DUNGEON.DAT, dm2/DM2DUNGEON.DAT, DUNGEON.DAT, DM2DUNGEON.DAT)\n");
        return -1;
    }
    printf("DM2: loading dungeon from %s (39 KB)\n", path);
    /* TODO: parse DM2 DUNGEON.DAT */
    return 0;
}

int dm2_v1_enter_shop(DM2_V1_GameState *state) {
    if (!state || !state->outdoor) return -1;
    printf("DM2: entering shop (gold: %d)\n", state->gold);
    return 0;
}

int dm2_v1_is_outdoor(const DM2_V1_GameState *state) {
    return state ? state->outdoor : 0;
}
