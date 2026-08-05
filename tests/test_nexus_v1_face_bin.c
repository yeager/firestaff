#include "nexus_v1_face_bin.h"
#include "nexus_v1_ui_surfaces.h"
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

static int test_synthetic_header(void) {
    Nexus_V1_FaceBinHeader hdr;
    uint8_t bad[16];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_face_bin_parse_header(bad, 16, &hdr)) return 1;
    if (nexus_v1_face_bin_parse_header(NULL, 0, &hdr)) return 1;
    printf("  PASS synthetic_header\n");
    return 0;
}

static int test_real_decode(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_FaceBinHeader hdr;
    Nexus_V1_FaceBinDecodeResult result;
    int i;

    if (!home) { printf("  SKIP real_decode (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/FACE.BIN", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP real_decode (no FACE.BIN)\n"); return 0; }

    if (!nexus_v1_face_bin_parse_header(data, size, &hdr)) {
        printf("  FAIL real_decode: header parse failed\n");
        free(data); return 1;
    }
    if (hdr.portrait_count != 20) {
        printf("  FAIL real_decode: expected 20 portraits, got %d\n",
               hdr.portrait_count);
        free(data); return 1;
    }
    printf("  PASS header: %d portraits, file_size=%u\n",
           hdr.portrait_count, hdr.file_size);

    if (!nexus_v1_face_bin_decode_all(data, size, &result)) {
        printf("  FAIL real_decode: decode_all failed\n");
        free(data); return 1;
    }
    printf("  decoded=%d failed=%d\n", result.decoded_count,
           result.failed_count);

    for (i = 0; i < result.portrait_count; ++i) {
        const Nexus_V1_FaceBinPortrait *p = &result.portraits[i];
        if (!p->valid) {
            printf("  FAIL portrait %d: not valid\n", i);
            free(data); return 1;
        }
        printf("  PASS portrait %2d: hash=0x%08X pal[0]=0x%08X\n",
               i, p->pixel_hash, p->palette_rgba[0]);
    }

    if (result.decoded_count != 20) {
        printf("  FAIL real_decode: expected 20 decoded, got %d\n",
               result.decoded_count);
        free(data); return 1;
    }
    if (result.failed_count != 0) {
        printf("  FAIL real_decode: %d failures\n", result.failed_count);
        free(data); return 1;
    }

    {
        Nexus_UI_Manager ui;
        nexus_ui_manager_init(&ui);
        for (i = 0; i < 20; ++i) {
            Nexus_UI_FaceCompactRecordDescriptor descriptor;
            Nexus_UI_Surface *surface;
            if (!nexus_ui_face_compact_record_descriptor(data, size, i,
                                                         &descriptor) ||
                nexus_ui_load_face_record(
                    &ui, data + descriptor.prefix_offset,
                    (int)descriptor.prefix_size + (int)descriptor.prs3_size,
                    i, 56, 56, NULL) >= 0) {
                printf("  FAIL startup_loader admitted unverified portrait %d\n", i);
                nexus_ui_manager_free(&ui);
                free(data);
                return 1;
            }
            surface = &ui.surfaces[NEXUS_SURFACE_FACE0 + i];
            if (surface->data || surface->source_palette_loaded) {
                printf("  FAIL startup_loader retained portrait pixels %d\n", i);
                nexus_ui_manager_free(&ui);
                free(data);
                return 1;
            }
        }
        printf("  PASS startup_loader: 20 retail portraits remain blocked\n");
        nexus_ui_manager_free(&ui);
    }

    printf("  PASS real_decode: all 20 portraits decoded\n");
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 FACE.BIN Portrait Decoder ===\n");
    fail += test_synthetic_header();
    fail += test_real_decode();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
