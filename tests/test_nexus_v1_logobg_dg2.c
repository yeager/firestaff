#include "nexus_v1_logobg_dg2.h"
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
    Nexus_V1_LogobgDg2DecodeResult r;
    uint8_t bad[32];
    memset(bad, 0, sizeof(bad));

    if (nexus_v1_logobg_dg2_decode(NULL, 0, &r)) return 1;
    if (nexus_v1_logobg_dg2_decode(bad, 4, &r)) return 1;
    printf("  PASS reject invalid\n");
    return 0;
}

static int test_logobg(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_LogobgDg2DecodeResult r;

    if (!home) { printf("  SKIP (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/LOGOBG.DG2", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP LOGOBG.DG2 (not found)\n"); return 0; }

    if (!nexus_v1_logobg_dg2_decode(data, size, &r)) {
        printf("  FAIL LOGOBG.DG2 decode\n");
        free(data);
        return 1;
    }

    printf("  PASS LOGOBG.DG2: %dx%d fmt=0x%04X palette=%d pixels=%d hash=0x%08X\n",
           r.width, r.height, r.format, r.palette_color_count,
           r.pixel_count, r.data_hash);
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 LOGOBG DG2 Decoder ===\n");
    fail += test_reject_invalid();
    fail += test_logobg();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
