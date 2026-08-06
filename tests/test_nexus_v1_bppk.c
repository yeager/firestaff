#include "nexus_v1_bppk.h"
#include "nexus_v1_bpk_archive.h"
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

static int test_menu_bpk(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_BppkDecodeResult r;

    if (!home) { printf("  SKIP (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP MENU.BPK (not found)\n"); return 0; }

    if (!nexus_v1_bppk_decode(data, size, &r)) {
        printf("  FAIL MENU.BPK decode\n");
        free(data);
        return 1;
    }

    printf("  PASS MENU.BPK: entries=%d prs3=%d size=%u hash=0x%08X\n",
           r.entry_count, r.prs3_count, r.file_size, r.data_hash);
    free(data);
    return 0;
}

static int test_real_menu_surface_decode(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_BpkArchiveInfo archive;
    uint16_t palette[NEXUS_V1_BPK_PALT_ENTRY_COUNT];
    uint64_t palette_hash = 0U;
    int decoded = 0;
    int fail = 0;
    uint32_t index;

    if (!data_dir || !data_dir[0]) return 0;
    if (snprintf(path, sizeof(path), "%s/MENU.BPK", data_dir) >=
        (int)sizeof(path) || !(data = load_file(path, &size))) {
        printf("  SKIP real MENU.BPK surface decode (not found)\n");
        return 0;
    }
    if (nexus_v1_bpk_archive_parse(data, (size_t)size, &archive) != 0) {
        printf("  FAIL real MENU.BPK archive parse\n");
        free(data);
        return 1;
    }
    if (nexus_v1_bpk_archive_copy_palette_words_be16(
            data, (size_t)size, palette, &palette_hash) != 0 ||
        palette[0] != 0xffffU || palette[14] != 0U ||
        palette_hash != UINT64_C(0x0ec4e98ca3a18f85)) {
        printf("  FAIL real MENU.BPK raw PALT words\n");
        free(data);
        return 1;
    }
    for (index = 0U; index < archive.entry_count_hint; ++index) {
        Nexus_V1_BpkPrs3Info info;
        Nexus_V1_BpkSurfaceEntry surface;
        uint8_t *pixels;
        size_t written = 0U;
        size_t nonzero = 0U;
        uint32_t i;

        if (nexus_v1_bpk_archive_inspect_prs3(
                data, (size_t)size, index, &info) != 0 || !info.has_prs3)
            continue;
        pixels = (uint8_t *)calloc(1U, info.prs3_pixel_count);
        if (!pixels || nexus_v1_bpk_archive_decode_surface(
                data, (size_t)size, index, pixels, info.prs3_pixel_count,
                &surface, &written) != NEXUS_V1_BPK_DECODE_OK ||
            written != info.prs3_pixel_count ||
            surface.layout.surface_class != NEXUS_V1_BPK_SURFACE_INDEXED_8BPP ||
            surface.pixel_count != info.prs3_pixel_count) {
            ++fail;
        } else {
            for (i = 0U; i < info.prs3_pixel_count; ++i) {
                if (pixels[i] != 0U) ++nonzero;
            }
            if (!nonzero) ++fail;
        }
        free(pixels);
        ++decoded;
    }
    printf("  %s real MENU.BPK indexed surface decode: %d/162\n",
           fail == 0 && decoded == 162 ? "PASS" : "FAIL", decoded);
    free(data);
    return fail == 0 && decoded == 162 ? 0 : 1;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 BPPK Decoder ===\n");
    fail += test_menu_bpk();
    fail += test_real_menu_surface_decode();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
