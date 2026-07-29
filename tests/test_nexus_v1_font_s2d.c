#include "nexus_v1_font_s2d.h"
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
    Nexus_V1_FontS2dDecodeResult r;
    uint8_t bad[32];
    memset(bad, 0, sizeof(bad));

    if (nexus_v1_font_s2d_decode(NULL, 0, &r)) return 1;
    if (nexus_v1_font_s2d_decode(bad, 4, &r)) return 1;
    if (nexus_v1_font_s2d_decode(bad, 32, &r)) return 1;
    printf("  PASS reject invalid\n");
    return 0;
}

static int test_font256(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0, i;
    Nexus_V1_FontS2dDecodeResult r;

    if (!home) { printf("  SKIP (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/FONT256.S2D", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP FONT256.S2D (not found)\n"); return 0; }

    if (!nexus_v1_font_s2d_decode(data, size, &r)) {
        printf("  FAIL FONT256.S2D decode\n");
        free(data);
        return 1;
    }

    printf("  PASS FONT256.S2D: sections=%d tilemap=%dx%d tiles=%d palette=%d hash=0x%08X\n",
           r.section_count, r.tilemap_width, r.tilemap_height,
           r.tile_count, r.palette_color_count, r.data_hash);

    for (i = 0; i < r.section_count; i++) {
        printf("    section %d: offset=0x%X size=0x%X\n",
               i, r.sections[i].offset, r.sections[i].size);
    }

    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 FONT S2D Decoder ===\n");
    fail += test_reject_invalid();
    fail += test_font256();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
