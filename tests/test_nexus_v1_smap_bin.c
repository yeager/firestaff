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
    uint8_t *valid;
    const int valid_size = 0x31a0 + 64;
    int i;
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_smap_parse_header(bad, 32, &hdr)) return 1;
    if (nexus_v1_smap_parse_header(NULL, 0, &hdr)) return 1;
    memcpy(bad, "LVMP", 4);
    if (nexus_v1_smap_parse_header(bad, 32, &hdr)) return 1;
    valid = (uint8_t *)calloc((size_t)valid_size, 1U);
    if (!valid) return 1;
    memcpy(valid, "LVMP", 4);
    valid[4] = (uint8_t)(valid_size >> 24);
    valid[5] = (uint8_t)(valid_size >> 16);
    valid[6] = (uint8_t)(valid_size >> 8);
    valid[7] = (uint8_t)valid_size;
    valid[8] = 0; valid[9] = 0; valid[10] = 0x00; valid[11] = 0x20;
    valid[12] = 0; valid[13] = 0x00; valid[14] = 0x2f; valid[15] = 0x80;
    valid[16] = 0; valid[17] = 0x00; valid[18] = 0x2f; valid[19] = 0xa0;
    valid[20] = 0; valid[21] = 0; valid[22] = 0x02; valid[23] = 0x00;
    valid[24] = 0; valid[25] = 0x00; valid[26] = 0x31; valid[27] = 0xa0;
    valid[28] = 0; valid[29] = 0; valid[30] = 0; valid[31] = 64;
    for (i = 0; i < 256; ++i) {
        valid[0x2fa0 + i * 2] = 0x80;
    }
    if (!nexus_v1_smap_parse_header(valid, valid_size, &hdr)) {
        free(valid); return 1;
    }
    valid[0x20 + 1] = 1;
    if (nexus_v1_smap_parse_header(valid, valid_size, &hdr)) {
        free(valid); return 1;
    }
    valid[0x20 + 1] = 0;
    valid[0x2fa0] = 0;
    if (nexus_v1_smap_parse_header(valid, valid_size, &hdr)) {
        free(valid); return 1;
    }
    valid[0x2fa0] = 0xfc;
    valid[0x2fa1] = 0x00;
    {
        uint32_t *rgba = (uint32_t *)calloc(
            (size_t)NEXUS_SMAP_PIXEL_WIDTH * NEXUS_SMAP_PIXEL_HEIGHT,
            sizeof(*rgba));
        Nexus_V1_SmapDecodeResult decoded;
        if (!rgba || !nexus_v1_smap_decode(
                valid, valid_size, rgba,
                NEXUS_SMAP_PIXEL_WIDTH * NEXUS_SMAP_PIXEL_HEIGHT,
                &decoded) || decoded.palette_rgba[0] != 0xFFF80000U) {
            free(rgba);
            free(valid);
            return 1;
        }
        free(rgba);
    }
    free(valid);
    printf("  PASS synthetic\n");
    return 0;
}

static int test_real_decode(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char path[512];
    int fail = 0;
    int level;

    if (!data_dir || !data_dir[0]) {
        if (!home || !home[0]) {
            printf("  SKIP real_decode (Nexus data root unavailable)\n");
            return 0;
        }
        data_dir = NULL;
    }

    for (level = 0; level <= 15; ++level) {
        uint8_t *data;
        int size = 0;
        Nexus_V1_SmapHeader hdr;
        Nexus_V1_SmapDecodeResult result;
        uint32_t *rgba;

        if (data_dir) {
            snprintf(path, sizeof(path), "%s/SMAP%02d.BIN", data_dir, level);
        } else {
            snprintf(path, sizeof(path),
                     "%s/.firestaff/data/nexus/SMAP%02d.BIN", home, level);
        }
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
