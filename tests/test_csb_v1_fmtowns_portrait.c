#include "csb_v1_fmtowns_portrait.h"
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
    ASSERT(csb_v1_fmtowns_portrait_probe(NULL, 0) == 0, "probe rejects NULL");
    uint8_t small[4] = {0};
    ASSERT(csb_v1_fmtowns_portrait_probe(small, 4) == 0, "probe rejects small");
}

static void test_real_portraits(void) {
    const char *home = getenv("HOME");
    const char *portrait_dir = getenv("FIRESTAFF_CSB_FMTOWNS_PORTRAIT_DIR");
    char path[512];
    const char *names[] = {
        "ALEX", "AZIZI", "BORIS", "CHANI", "DAROOU", "ELIJA",
        "GANDO", "GOTHMOG", "HALK", "HAWK", "HISSSSA", "IAIDO",
        "LEIF", "LEYLA", "LINFLAS", "MOPHUS", "NABI", "SONJA",
        "STAMM", "SYRA", "TIGGY", "WUTSE", "WUUF", "ZED"
    };
    int decoded = 0, i;

    if ((!portrait_dir || portrait_dir[0] == '\0') && !home) {
        printf("SKIP: HOME not set\n");
        return;
    }

    for (i = 0; i < 24; i++) {
        uint8_t *data;
        size_t size;
        uint8_t pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
        CSB_V1_FmtownsPortraitReceipt receipt;

        if (portrait_dir && portrait_dir[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s.CMP", portrait_dir, names[i]);
        } else {
            snprintf(path, sizeof(path),
                     "%s/.firestaff/data/csb/fmtowns/PORTRAIT/%s.CMP",
                     home, names[i]);
        }
        data = load_file(path, &size);
        if (!data) {
            printf("SKIP: %s not available\n", names[i]);
            continue;
        }

        ASSERT(csb_v1_fmtowns_portrait_probe(data, size) == 1,
               "probe accepts portrait");

        if (csb_v1_fmtowns_portrait_decode(data, size, pixels,
                sizeof(pixels), &receipt)) {
            ASSERT(receipt.valid == 1, "receipt valid");
            ASSERT(receipt.pixel_fnv1a != 0, "pixel hash non-zero");
            ASSERT(strlen(receipt.name) > 0, "name non-empty");

            /* Verify meaningful pixel content */
            {
                size_t nonzero = 0, j;
                for (j = 0; j < CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT; j++) {
                    if (pixels[j] != 0) nonzero++;
                    ASSERT(pixels[j] <= 15, "pixel is 4bpp");
                }
                ASSERT(nonzero > 100, "portrait has visible content");
            }
            decoded++;
            if (i < 4) {
                printf("  %s (%s): hash=%08x\n",
                       receipt.name, receipt.title, receipt.pixel_fnv1a);
            }
        }
        free(data);
    }

    printf("  Decoded %d/24 portraits\n", decoded);
    ASSERT(decoded == 24, "all 24 portraits decoded");
}

int main(void) {
    test_probe_null();
    test_real_portraits();
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
