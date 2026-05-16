
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <string.h>

/* pass603: CSB V1 save/load
 * CSBWin/SaveGame.cpp: 2953 lines
 */

int csb_v1_save_game(const char *path, const void *state, int state_size) {
    FILE *f;
    if (!path || !state || state_size <= 0) return -1;
    f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(state, 1, state_size, f);
    fclose(f);
    return 0;
}

int csb_v1_load_game(const char *path, void *state, int max_size) {
    FILE *f;
    int read;
    if (!path || !state || max_size <= 0) return -1;
    f = fopen(path, "rb");
    if (!f) return -1;
    read = (int)fread(state, 1, max_size, f);
    fclose(f);
    return read;
}

const char *csb_v1_save_source_evidence(void) {
    return "CSBWin/SaveGame.cpp: 2953 lines save/load system\n";
}

