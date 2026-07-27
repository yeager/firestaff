/*
 * test_dm2_v1_g1_record_graph_diagnostic.c
 *
 * Validates DM2 PC G1 DUNGEON.DAT record graph completion.
 * Loads real game data and verifies record_graph_complete == 1.
 *
 * Key finding: G1 byte-square format stores game data in w0,
 * NOT next-links. The validator checks ground-stack entries
 * resolve to valid records (including G1 extension pools).
 */

#include "dm2_v1_dungeon_loader.h"

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
    if (sz <= 0 || sz > 256 * 1024) { fclose(f); return NULL; }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

int main(void) {
    const char *paths[] = {
        NULL,
        "/Users/bosse/.firestaff/data/dm2/data/dungeon.dat",
        NULL
    };
    const char *env = getenv("DM2_DUNGEON_DAT");
    uint8_t *dat = NULL;
    int size = 0;
    int failures = 0;
    DM2_V1_DungeonData d;

    paths[0] = env;

    for (int i = 0; paths[i]; ++i) {
        dat = load_file(paths[i], &size);
        if (dat) break;
    }
    if (!dat) {
        printf("SKIP: no DM2 DUNGEON.DAT available\n");
        return 0;
    }

    memset(&d, 0, sizeof(d));
    if (dm2_v1_dungeon_load(&d, dat, size) != 0) {
        printf("FAIL: dm2_v1_dungeon_load rejected data\n");
        free(dat);
        return 1;
    }

    printf("G1 record graph: levels=%d, records=%d, graph_complete=%d\n",
           d.level_count, d.square_first_thing_count,
           d.record_graph_complete);

    if (!d.record_graph_complete) {
        printf("FAIL: record_graph_complete is 0\n");
        ++failures;
    }
    if (d.partial_map_boot.incomplete) {
        printf("FAIL: partial_map_boot.incomplete is 1\n");
        ++failures;
    }
    if (d.square_bytes != 1) {
        printf("FAIL: expected G1 byte-square format (square_bytes=1)\n");
        ++failures;
    }
    if (dm2_v1_dungeon_get_next_thing(&d, 0x04A5) !=
        (int)DM2_THING_END_MARKER) {
        printf("FAIL: G1 get_next_thing should return END_MARKER\n");
        ++failures;
    }

    dm2_v1_dungeon_free(&d);
    free(dat);

    printf("%s: DM2 V1 G1 record graph diagnostic\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
