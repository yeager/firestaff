#include "nexus_v1_raw_bin.h"
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

static const char *type_name(int t) {
    switch (t) {
        case 1: return "VDP_DATA";
        case 2: return "SH2_CODE";
        case 3: return "TILEMAP";
        default: return "UNKNOWN";
    }
}

static int test_file(const char *name) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_RawBinDecodeResult r;

    if (!home) { printf("  SKIP %s (no HOME)\n", name); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/%s", home, name);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP %s (not found)\n", name); return 0; }

    if (!nexus_v1_raw_bin_decode(data, size, &r)) {
        printf("  FAIL %s decode\n", name);
        free(data);
        return 1;
    }

    printf("  PASS %s: type=%s size=%d nz=%d prs3=%d hash=0x%08X\n",
           name, type_name(r.content_type), r.file_size,
           r.non_zero_bytes, r.prs3_offset, r.data_hash);
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 Raw Binary Decoder ===\n");

    fail += test_file("DM.BIN");
    fail += test_file("NBG3.BIN");
    fail += test_file("STONE.BIN");
    fail += test_file("DEATH.BIN");
    fail += test_file("SWTCHR.BIN");
    fail += test_file("TM.BIN");
    fail += test_file("SDDRVS.TSK");

    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
