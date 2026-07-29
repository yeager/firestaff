#include "nexus_v1_title_cg.h"
#include "nexus_v1_res.h"
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

static int test_title_cg(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_TitleCgDecodeResult r;

    if (!home) { printf("  SKIP title_cg (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/TITLE.CG", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP title_cg (no file)\n"); return 0; }

    if (!nexus_v1_title_cg_decode(data, size, &r)) {
        printf("  FAIL title_cg decode\n");
        free(data);
        return 1;
    }

    if (r.tile_count != NEXUS_TITLE_CG_TILE_COUNT) {
        printf("  FAIL tile_count=%d expected=%d\n",
               r.tile_count, NEXUS_TITLE_CG_TILE_COUNT);
        free(data);
        return 1;
    }

    printf("  PASS title_cg: tiles=%d hash=0x%08X\n",
           r.tile_count, r.tile_hash);
    free(data);
    return 0;
}

static int test_res_file(const char *name) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0, i;
    Nexus_V1_ResDecodeResult r;

    if (!home) return 0;
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/%s", home, name);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP %s (not found)\n", name); return 0; }

    if (!nexus_v1_res_decode(data, size, &r)) {
        printf("  FAIL %s: RES* decode failed\n", name);
        free(data);
        return 1;
    }

    printf("  PASS %-14s entries=%d size=%u\n", name, r.entry_count, r.file_size);
    for (i = 0; i < r.entry_count && i < 8; ++i) {
        printf("    [%d] %s#%u off=0x%X size=%u\n",
               i, r.entries[i].tag, r.entries[i].index,
               r.entries[i].offset, r.entries[i].size);
    }
    if (r.entry_count > 8) printf("    ... +%d more\n", r.entry_count - 8);

    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 TITLE.CG & RES* Decoder ===\n");
    fail += test_title_cg();
    fail += test_res_file("TITLE.BIN");
    fail += test_res_file("RLOWFIX.BIN");
    fail += test_res_file("RHIFIX.BIN");
    fail += test_res_file("POTEFT.BIN");
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
