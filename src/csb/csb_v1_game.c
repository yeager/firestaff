#include "csb_v1_game.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void csb_v1_init(CSB_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 5;  /* CSB start position (different from DM1) */
    state->party_y = 5;
    state->party_dir = 0;
    state->difficulty = 1; /* 1 = normal CSB difficulty (1.5x vs DM1) */
}

/* Try multiple dungeon file candidates; CSB files vary by platform:
 * Atari ST: DUNGEON.DAT, Amiga: Dungeon.DAT, some: CSB.DAT */
static int csb_find_dungeon(const char *data_dir, char *path, int pathlen) {
    struct stat st;
    static const char *fmts[] = {
        "%s/csb/DUNGEON.DAT", "%s/csb/Dungeon.DAT", "%s/csb/CSB.DAT",
        "%s/DUNGEON.DAT", "%s/CSB.DAT", NULL
    };
    for (int i = 0; fmts[i]; i++) {
        snprintf(path, pathlen, fmts[i], data_dir);
        if (stat(path, &st) == 0) return 1;
    }
    /* Fallback to standard path for error message */
    snprintf(path, pathlen, "%s/csb/DUNGEON.DAT", data_dir);
    return 0;
}

int csb_v1_load_dungeon(CSB_V1_GameState *state) {
    char path[512];
    if (!state || !state->data_dir) return -1;
    int found = csb_find_dungeon(state->data_dir, path, sizeof(path));
    if (!found) {
        printf("CSB: dungeon not found (searched csb/DUNGEON.DAT, csb/Dungeon.DAT, csb/CSB.DAT, DUNGEON.DAT, CSB.DAT)\n");
        return -1;
    }
    printf("CSB: loading dungeon from %s\n", path);
    /* CSB DUNGEON.DAT is only 2098 bytes — likely single level or compressed */
    /* TODO: parse CSB DUNGEON.DAT */
    return 0;
}

int csb_v1_import_dm1_save(CSB_V1_GameState *state, const char *dm1_save_path) {
    if (!state || !dm1_save_path) return -1;
    printf("CSB: importing DM1 champions from %s\n", dm1_save_path);
    state->dm1_import_done = 1;
    return 0;
}
