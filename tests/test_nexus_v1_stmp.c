#include "nexus_v1_stmp.h"
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

static int test_reject_invalid(void) {
    Nexus_V1_StmpDecodeResult r;
    uint8_t bad[32];
    memset(bad, 0, sizeof(bad));

    if (nexus_v1_stmp_decode(NULL, 0, &r)) return 1;
    if (nexus_v1_stmp_decode(bad, 4, &r)) return 1;
    if (nexus_v1_stmp_decode(bad, 32, &r)) return 1;
    printf("  PASS reject invalid\n");
    return 0;
}

static int test_stabg(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0, i;
    Nexus_V1_StmpDecodeResult r;

    if (!home) { printf("  SKIP (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/STABG.BIN", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP STABG.BIN (not found)\n"); return 0; }

    if (!nexus_v1_stmp_decode(data, size, &r)) {
        printf("  FAIL STABG.BIN decode\n");
        free(data);
        return 1;
    }

    printf("  PASS STABG.BIN: planes=%d palette=%d tiledata=0x%X hash=0x%08X\n",
           r.plane_count, r.palette_color_count, r.tiledata_size, r.data_hash);

    for (i = 0; i < r.plane_count; i++) {
        printf("    plane %d: %dx%d (%d tiles)\n",
               i, r.planes[i].width, r.planes[i].height, r.planes[i].tile_count);
    }

    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 STMP Decoder ===\n");
    fail += test_reject_invalid();
    fail += test_stabg();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
