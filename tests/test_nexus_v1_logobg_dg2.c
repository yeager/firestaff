#include "nexus_v1_logobg_dg2.h"
#include "nexus_v1_engine.h"
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
    Nexus_V1_LogobgDg2DecodeResult r;
    uint8_t bad[32];
    memset(bad, 0, sizeof(bad));

    if (nexus_v1_logobg_dg2_decode(NULL, 0, &r)) return 1;
    if (nexus_v1_logobg_dg2_decode(bad, 4, &r)) return 1;
    printf("  PASS reject invalid\n");
    return 0;
}

static int test_logobg(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_Engine engine;
    Nexus_V1_LevelAuxSourceReceipt receipt;
    Nexus_V1_LogobgDg2DecodeResult r;

    if (data_dir && data_dir[0]) {
        snprintf(path, sizeof(path), "%s/LOGOBG.DG2", data_dir);
    } else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/LOGOBG.DG2", home);
    } else {
        printf("  SKIP (Nexus data root is unset)\n");
        return 0;
    }
    data = load_file(path, &size);
    memset(&engine, 0, sizeof(engine));
    memset(&receipt, 0, sizeof(receipt));
    if (!data) {
        /* A CUE/BIN-only installation is a first-class retail source. Read
         * its member in memory through the native engine; never extract it. */
        if (nexus_v1_init(&engine, data_dir && data_dir[0] ? data_dir : path) != 0 ||
            engine.source != NEXUS_SRC_ISO ||
            nexus_v1_named_asset_source_receipt(&engine, "LOGOBG.DG2", &receipt) != 0 ||
            !receipt.exact_source_entry_observed || !receipt.canonical_hash_verified) {
            nexus_v1_shutdown(&engine);
            printf("  SKIP LOGOBG.DG2 (no authenticated native source)\n");
            return 0;
        }
        data = nexus_v1_read_file(&engine, "LOGOBG.DG2", &size);
    }
    if (!data) { nexus_v1_shutdown(&engine); return 1; }

    if (!nexus_v1_logobg_dg2_decode(data, size, &r)) {
        printf("  FAIL LOGOBG.DG2 decode\n");
        free(data);
        nexus_v1_shutdown(&engine);
        return 1;
    }

    printf("  PASS LOGOBG.DG2: %dx%d fmt=0x%04X palette=%d pixels=%d hash=0x%08X\n",
           r.width, r.height, r.format, r.palette_color_count,
           r.pixel_count, r.data_hash);
    free(data);
    nexus_v1_shutdown(&engine);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 LOGOBG DG2 Decoder ===\n");
    fail += test_reject_invalid();
    fail += test_logobg();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
