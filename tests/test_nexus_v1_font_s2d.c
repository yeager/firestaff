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

    if (r.map_offset != 0x100U || r.map_size != 0x12U ||
        r.page_offset != 0x120U || r.page_size != 0x2010U ||
        r.character_generator_offset != 0x2130U ||
        r.character_generator_size != 0x3c90U ||
        r.palette_offset != 0x5dc0U || r.palette_size != 0x210U ||
        r.attribute_offset != 0x5fd0U || r.attribute_size != 0x1e4U) {
        printf("  FAIL DMWeb FONT256 named regions\n");
        free(data);
        return 1;
    }
    printf("  PASS DMWeb named regions: map/page/CG/palette/attributes\n");
    if (r.map_horizontal_page != 1U || r.map_vertical_page != 1U ||
        r.map_page_number != 0U ||
        r.page_character_control_data != 0x00100000U ||
        r.page_pattern_name_auxiliary_data != 0x4000U) {
        printf("  FAIL DMWeb FONT256 header facts\n");
        free(data);
        return 1;
    }
    printf("  PASS DMWeb FONT256 map/page header facts\n");
    {
        uint8_t tile[64];
        if (nexus_v1_font_s2d_copy_character_generator_tile(
                data, size, &r, 0, tile) != 0 ||
            nexus_v1_font_s2d_copy_character_generator_tile(
                data, size, &r, 241, tile) != 0 ||
            nexus_v1_font_s2d_copy_character_generator_tile(
                data, size, &r, 242, tile) == 0) {
            printf("  FAIL DMWeb Character Generator tile bounds\n");
            free(data);
            return 1;
        }
        printf("  PASS DMWeb Character Generator: 242 bounded 8x8 tiles\n");
    }
    {
        uint16_t tilemap_word = 0;
        uint16_t palette_word = 0;
        uint16_t attribute_word = 0;
        if (nexus_v1_font_s2d_page_tilemap_word(
                data, size, &r, 1, &tilemap_word) != 0 ||
            tilemap_word != 2U ||
            nexus_v1_font_s2d_palette_word(
                data, size, &r, 0, &palette_word) != 0 ||
            palette_word != 0x8000U ||
            nexus_v1_font_s2d_attribute_word(
                data, size, &r, 0, &attribute_word) != 0 ||
            nexus_v1_font_s2d_page_tilemap_word(
                data, size, &r, 4096, &tilemap_word) == 0) {
            printf("  FAIL DMWeb tilemap/palette source words\n");
            free(data);
            return 1;
        }
        printf("  PASS DMWeb Page tilemap + palette + attribute words\n");
    }

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
