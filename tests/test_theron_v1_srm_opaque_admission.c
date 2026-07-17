#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_srm_opaque_admission.h"
#include "theron_v1_srm_opaque_runtime.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

int main(void) {
    const char *path = "/tmp/firestaff-theron-opaque-save.srm";
    const unsigned char gzip_header[] = {0x1f, 0x8b, 0x08, 0x00};
    Theron_V1SrmOpaqueAdmissionRequest request = {0};
    Theron_V1SrmOpaqueAdmissionReceipt receipt;
    Theron_V1SrmOpaqueRuntimeReceipt runtime;
    char md5[33];
    FILE *file;

    CHECK(theron_v1_srm_opaque_admit(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_UNAVAILABLE);
    file = fopen(path, "wb");
    CHECK(file != NULL);
    if (file) { fwrite(gzip_header, 1, sizeof(gzip_header), file); fclose(file); }
    CHECK(m12_file_md5_hex(path, md5));
    request.srm_path = path;
    request.expected_srm_md5 = md5;
    request.expected_srm_size = sizeof(gzip_header);
    request.admission_format_version = THERON_V1_SRM_OPAQUE_ADMISSION_VERSION;
    request.expected_shape = THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1;
    request.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    request.expected_track02_md5 = THERON_TRACK02_MD5_US_BIN;
    CHECK(theron_v1_srm_opaque_admit(&request, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_READY);
    CHECK(receipt.source_md5_verified && receipt.source_size_verified &&
          receipt.admission_version_verified && receipt.source_shape_verified &&
          receipt.track02_identity_verified && receipt.opaque_save_route_ready);
    CHECK(!receipt.save_semantics_decoded && !receipt.synthetic_fallback_used);
    CHECK(theron_v1_srm_opaque_runtime_consume(&receipt, &runtime));
    CHECK(runtime.opaque_save_route_ready && !runtime.save_restore_permitted &&
          !runtime.save_semantics_decoded && !runtime.synthetic_fallback_used);
    receipt.source_shape_verified = 0;
    CHECK(!theron_v1_srm_opaque_runtime_consume(&receipt, &runtime));
    receipt.source_shape_verified = 1;
    request.expected_srm_size++;
    CHECK(theron_v1_srm_opaque_admit(&request, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_REJECTED);
    request.expected_srm_size--;
    request.expected_track02_md5 = THERON_TRACK02_MD5_JP_BIN;
    CHECK(theron_v1_srm_opaque_admit(&request, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_REJECTED);
    request.expected_track02_md5 = THERON_TRACK02_MD5_US_BIN;
    request.admission_format_version++;
    CHECK(theron_v1_srm_opaque_admit(&request, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_REJECTED);
    remove(path);
    CHECK(theron_v1_srm_opaque_admit(&request, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_OPAQUE_UNAVAILABLE);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
