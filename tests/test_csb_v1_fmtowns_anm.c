#include "csb_v1_fmtowns_anm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static uint8_t *load_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static void test_probe_null(void) {
    ASSERT(csb_v1_fmtowns_anm_probe(NULL, 0) == 0, "probe rejects NULL");
    uint8_t small[4] = {0};
    ASSERT(csb_v1_fmtowns_anm_probe(small, 4) == 0, "probe rejects small");
}

static void test_real_anm(void) {
    const char *home = getenv("HOME");
    const char *anm_dir = getenv("FIRESTAFF_CSB_FMTOWNS_ANM_DIR");
    char path[512];
    const char *files[] = {"TITLE.ANM", "STORY.ANM", "ENDING.ANM"};
    int i;

    if ((!anm_dir || anm_dir[0] == '\0') && !home) {
        printf("SKIP: HOME not set\n");
        return;
    }

    for (i = 0; i < 3; i++) {
        uint8_t *data;
        size_t size;
        CSB_V1_FmtownsAnmReceipt receipt;

        if (anm_dir && anm_dir[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s", anm_dir, files[i]);
        } else {
            snprintf(path, sizeof(path),
                     "%s/.firestaff/data/csb/fmtowns/%s", home, files[i]);
        }
        data = load_file(path, &size);
        if (!data) {
            printf("SKIP: %s not available\n", files[i]);
            continue;
        }

        ASSERT(csb_v1_fmtowns_anm_probe(data, size) == 1,
               "probe accepts ANM");
        ASSERT(csb_v1_fmtowns_anm_parse(data, size, &receipt) == 0,
               "parse succeeds");
        ASSERT(receipt.valid == 1, "receipt valid");
        ASSERT(receipt.width == 320, "width 320");
        ASSERT(receipt.height == 200, "height 200");
        ASSERT(receipt.bpp == 4, "bpp 4");
        ASSERT(receipt.chunk_count > 0, "has chunks");

        printf("  %s: %ux%u %dbpp, %d chunks (%d frames, %d deltas, %d KF), "
               "palette=%s, BR=%s\n",
               files[i], receipt.width, receipt.height, receipt.bpp,
               receipt.chunk_count, receipt.frame_count,
               receipt.delta_count, receipt.keyframe_count,
               receipt.has_palette ? "yes" : "no",
               receipt.has_br_wrapper ? "yes" : "no");

        if (receipt.has_palette) {
            printf("    Palette[0]: R=%d G=%d B=%d\n",
                   receipt.palette[0].r, receipt.palette[0].g,
                   receipt.palette[0].b);
        }

        free(data);
    }
}

int main(void) {
    test_probe_null();
    test_real_anm();
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
