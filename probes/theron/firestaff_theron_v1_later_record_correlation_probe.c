#include "asset_status_m12.h"
#include "theron_v1_later_record_correlation.h"
#include "theron_v1_stage3_manifest_evidence.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int inspect(const char *path,
                   const char *md5_hex,
                   Theron_V1LaterRecordCorrelation *out_correlation) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    uint8_t *bytes;
    size_t size;
    char actual_md5[33];
    int ok;

    bytes = read_file_bytes(path, &size);
    if (!bytes) return 0;
    ok = m12_file_md5_hex(path, actual_md5) &&
        strcmp(actual_md5, md5_hex) == 0 &&
        theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, size, md5_hex, &payload) == THERON_TRACK02_SIGNAL_OK &&
        theron_v1_stage3_manifest_evidence_from_payload(
            bytes, size, &payload, &manifest) &&
        theron_v1_later_record_correlation_from_manifest(
            &manifest, size, out_correlation);
    free(bytes);
    return ok;
}

int main(void) {
    const char *jp_path = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    Theron_V1LaterRecordCorrelation jp;
    Theron_V1LaterRecordCorrelation us;
    Theron_V1LaterRecordCorrelationComparison comparison;

    if (!jp_path || !us_path) {
        ++g_skip;
        printf("[SKIP] set FIRESTAFF_THERON_TRACK02_JP_BIN and FIRESTAFF_THERON_TRACK02_US_BIN\n");
        return 0;
    }
    check(inspect(jp_path, THERON_TRACK02_MD5_JP_BIN, &jp),
          "JP raw Track02 establishes later-record self correlation");
    check(inspect(us_path, THERON_TRACK02_MD5_US_BIN, &us),
          "US raw Track02 establishes later-record self correlation");
    check(jp.valid && jp.stage3_track02_record == 0x0004dfu &&
              jp.first_descriptor_selector == 0x000au &&
              jp.derived_record_base == 0x0004d5u &&
              jp.self_reference_proven && jp.self_resolved_record_in_bounds &&
              jp.nonzero_selector_count == 214u,
          "JP first opaque selector resolves to its proven stage-three sector");
    check(us.valid && us.stage3_track02_record == 0x0004e0u &&
              us.first_descriptor_selector == 0x000au &&
              us.derived_record_base == 0x0004d6u &&
              us.self_reference_proven && us.self_resolved_record_in_bounds &&
              us.nonzero_selector_count == 216u,
          "US first opaque selector resolves to its proven stage-three sector");
    check(theron_v1_later_record_correlation_compare(&jp, &us, &comparison) &&
              comparison.valid && comparison.shared_first_selector == 0x000au &&
              comparison.first_base == 0x0004d5u &&
              comparison.second_base == 0x0004d6u && comparison.base_delta == 1u &&
              comparison.both_self_references_proven,
          "JP/US later-record coordinate bases differ only with stage-three shift");
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
