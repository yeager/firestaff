#include "nexus_v1_bppk.h"
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

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 BPPK Decoder ===\n");
    fail += test_menu_bpk();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
