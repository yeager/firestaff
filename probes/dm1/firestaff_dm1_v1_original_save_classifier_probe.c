#include "dm1_v1_original_save_classifier.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        g_fail++;
    } else {
        g_pass++;
    }
}

static void probe_default_root(void) {
    DM1OriginalSaveManifest manifest;
    char root[DM1_ORIGINAL_SAVE_PATH_MAX];

    check_int("default root resolves",
              dm1_v1_original_save_default_root(root), 1);
    printf("DM1 original save classifier root: %s\n", root);

    check_int("default root classifies",
              dm1_v1_original_save_classify_root(root, &manifest), 1);
    check_int("candidate count",
              manifest.candidate_count,
              (int)DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT);

    for (int i = 0; i < manifest.candidate_count; i++) {
        const DM1OriginalSaveClassifyResult *r = &manifest.results[i];
        printf("candidate[%d] path=%s shape=%s readiness=%s size=%llu "
               "format=%u checksum_ok=%d blocked=%d reason=%s\n",
               i,
               manifest.paths[i],
               dm1_v1_original_save_shape_name(r->shape),
               dm1_v1_original_save_readiness_name(r->readiness),
               (unsigned long long)r->size_bytes,
               (unsigned)r->format_id,
               r->header_checksum_ok,
               r->import_blocked_until_roundtrip,
               r->reason);

        if (r->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1) {
            check_int("original save remains importer-blocked",
                      r->import_blocked_until_roundtrip, 1);
            check_int("original save only header-classified",
                      r->readiness,
                      DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY);
        }
    }

    if (manifest.present_count == 0) {
        printf("SKIP no user-staged DM1 original saves found under %s\n", root);
        g_skip++;
    } else {
        check_int("classified count bounded",
                  manifest.classified_count <= manifest.present_count ? 1 : 0,
                  1);
        if (manifest.original_dm1_count == 0) {
            printf("SKIP present save candidates found, but no DM1 original "
                   "header shape was recognized\n");
            g_skip++;
        } else {
            g_pass++;
        }
    }
}

int main(void) {
    const char *evidence = dm1_v1_original_save_source_evidence();
    check_int("source evidence names ReDMCSB",
              evidence && strstr(evidence, "ReDMCSB") != NULL ? 1 : 0,
              1);
    check_int("source evidence names SAVEHEAD",
              evidence && strstr(evidence, "SAVEHEAD.C") != NULL ? 1 : 0,
              1);

    probe_default_root();

    printf("DM1 original save classifier probe: %d passed, %d skipped, %d failed\n",
           g_pass, g_skip, g_fail);
    return g_fail == 0 ? 0 : 1;
}
