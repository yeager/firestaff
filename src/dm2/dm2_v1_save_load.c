
#include "dm2_v1_save_load.h"
#include <stdio.h>

int dm2_v1_save_game(const char *path, const void *state, int size) {
    FILE *f;
    if (!path || !state || size <= 0) return -1;
    f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(state, 1, size, f);
    fclose(f);
    return 0;
}

int dm2_v1_load_game(const char *path, void *state, int max_size) {
    FILE *f; int r;
    if (!path || !state || max_size <= 0) return -1;
    f = fopen(path, "rb");
    if (!f) return -1;
    r = (int)fread(state, 1, max_size, f);
    fclose(f);
    return r;
}

const char *dm2_v1_save_source_evidence(void) {
    return "SKULL.ASM: DM2 save/load format\n";
}

