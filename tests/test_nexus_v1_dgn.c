#include "nexus_v1_dgn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

static int test_synthetic(void) {
    Nexus_V1_DgnDecodeResult r;
    uint8_t bad[64];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_dgn_decode(bad, 64, &r)) return 1;
    if (nexus_v1_dgn_decode(NULL, 0, &r)) return 1;
    printf("  PASS synthetic\n");
    return 0;
}

static int test_all_levels(void) {
    const char *home = getenv("HOME");
    char path[512];
    int level, fail = 0, decoded = 0;

    if (!home) { printf("  SKIP (no HOME)\n"); return 0; }

    for (level = 0; level <= 15; ++level) {
        uint8_t *data;
        int size = 0;
        Nexus_V1_DgnDecodeResult r;

        snprintf(path, sizeof(path),
                 "%s/.firestaff/data/nexus/LEV%02d.DGN", home, level);
        data = load_file(path, &size);
        if (!data) {
            printf("  SKIP LEV%02d.DGN (not found)\n", level);
            continue;
        }

        if (!nexus_v1_dgn_decode(data, size, &r)) {
            printf("  FAIL LEV%02d.DGN: decode failed\n", level);
            free(data);
            ++fail;
            continue;
        }

        printf("  PASS LEV%02d: lvl=%d models=%d tex=%d doors=%d "
               "walls=%d open=%d grid=0x%08X s2=0x%08X\n",
               level, r.level_number, r.model_ref_count,
               r.texture_count, r.door_count,
               r.wall_cell_count, r.open_cell_count,
               r.grid_hash, r.s2_hash);
        decoded++;
        free(data);
    }

    printf("  decoded %d DGN files\n", decoded);
    return fail;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 DGN Dungeon Decoder ===\n");
    fail += test_synthetic();
    fail += test_all_levels();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
