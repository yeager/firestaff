#include "csb_v1_f0726_f0745_media_filename_raw_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures, assertions;
static void check(int value, const char *expr, int line) { ++assertions; if (!value) { ++failures; fprintf(stderr, "FAIL:%d:%s\n", line, expr); } }
#define CHECK(value) check((value), #value, __LINE__)
static CSB_V1_MediaFilenameRawMaterialPc34 raw(uint8_t *data) {
    CSB_V1_MediaFilenameRawMaterialPc34 r; memset(&r, 0, sizeof(r));
    r.graphics = r.palette = r.zone = r.music = r.package = r.file_names = data;
    r.graphics_size = r.palette_size = r.zone_size = r.music_size = r.package_size = r.file_names_size = 8;
    r.graphics_identity = 1; r.palette_identity = 2; r.zone_identity = 3; r.music_identity = 4; r.package_identity = 5; r.file_names_identity = 6; r.authenticated_pc34 = 1; return r;
}
int main(void) {
    uint8_t bytes[8] = { 1 }, before[8]; int id;
    CSB_V1_MediaFilenameRawMaterialPc34 material = raw(bytes);
    CSB_V1_MediaFilenameAuditReceiptPc34 receipt;
    memcpy(before, bytes, sizeof(bytes));
    for (id = 731; id <= 745; ++id) {
        if (id == 736 || id == 737) continue;
        CHECK(csb_v1_f0726_f0745_media_filename_audit_pc34(&material, (CSB_V1_MediaFilenameFunctionPc34)id, &receipt) == 1);
        CHECK(receipt.source_evidence != NULL && receipt.runtime_execution_blocked && receipt.platform_behavior_fail_closed);
    }
    CHECK(memcmp(bytes, before, sizeof(bytes)) == 0);
    material.music_identity = 0;
    CHECK(csb_v1_f0726_f0745_media_filename_audit_pc34(&material, CSB_V1_MEDIA_FILENAME_F0738, &receipt) == 0);
    material = raw(bytes);
    CHECK(csb_v1_f0726_f0745_media_filename_audit_pc34(&material, CSB_V1_MEDIA_FILENAME_F0726, &receipt) == 0);
    CHECK(csb_v1_f0726_f0745_media_filename_audit_pc34(&material, CSB_V1_MEDIA_FILENAME_F0736, &receipt) == 0);
    printf("csb_v1_f0726_f0745_media_filename_raw: %d/%d assertions passed\n", assertions - failures, assertions);
    return failures != 0;
}
