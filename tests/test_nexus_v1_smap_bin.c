#include "nexus_v1_smap_bin.h"
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
    Nexus_V1_SmapHeader hdr;
    uint8_t bad[32];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_smap_parse_header(bad, 32, &hdr)) return 1;
    if (nexus_v1_smap_parse_header(NULL, 0, &hdr)) return 1;
    printf("  PASS synthetic\n");
    return 0;
}

static int test_real_decode(void) {
    const char *home = getenv("HOME");
    char path[512];
    int fail = 0;
    int level;

    if (!home) { printf("  SKIP real_decode (no HOME)\n"); return 0; }

    for (level = 0; level <= 15; ++level) {
        uint8_t *data;
        int size = 0;
        Nexus_V1_SmapHeader hdr;
        Nexus_V1_SmapDecodeResult result;
        uint32_t *rgba;

        snprintf(path, sizeof(path),
                 "%s/.firestaff/data/nexus/SMAP%02d.BIN", home, level);
        data = load_file(path, &size);
        if (!data) {
            printf("  SKIP SMAP%02d.BIN (not found)\n", level);
            continue;
        }
        if (!nexus_v1_smap_parse_header(data, size, &hdr)) {
            printf("  FAIL SMAP%02d.BIN: header parse failed\n", level);
            free(data);
            ++fail;
            continue;
        }

        rgba = (uint32_t *)malloc(NEXUS_SMAP_PIXEL_WIDTH *
                                   NEXUS_SMAP_PIXEL_HEIGHT * 4);
        if (!rgba) { free(data); ++fail; continue; }

        if (!nexus_v1_smap_decode(data, size, rgba,
                                   NEXUS_SMAP_PIXEL_WIDTH *
                                   NEXUS_SMAP_PIXEL_HEIGHT, &result)) {
            printf("  FAIL SMAP%02d.BIN: decode failed\n", level);
            free(rgba); free(data);
            ++fail;
            continue;
        }
        printf("  PASS SMAP%02d: %dx%d tiles=%d hash=0x%08X\n",
               level, result.rendered_width, result.rendered_height,
               hdr.tile_count, result.pixel_hash);
        free(rgba);
        free(data);
    }
    return fail;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 SMAP Automap Decoder ===\n");
    fail += test_synthetic();
    fail += test_real_decode();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
