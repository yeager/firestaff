#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "asset_status_m12.h"
#include "theron_v1_srm_opaque_admission.h"

static int theron_v1_srm_opaque_is_symlink(const char *path) {
#if defined(_WIN32)
    (void)path;
    return 0;
#else
    struct stat status;
    return lstat(path, &status) == 0 && S_ISLNK(status.st_mode);
#endif
}

int theron_v1_srm_opaque_admit(
    const Theron_V1SrmOpaqueAdmissionRequest *request,
    Theron_V1SrmOpaqueAdmissionReceipt *out) {
    Theron_V1SrmOpaqueAdmissionReceipt receipt = {0};
    FILE *file;
    struct stat status;
    unsigned char header[3];
    char initial_md5[33];
    char final_md5[33];

    if (!out) return 0;
    *out = receipt;
    if (!request || !request->srm_path || !request->expected_srm_md5 ||
        !request->expected_track02_md5 || !request->srm_path[0] ||
        !request->expected_srm_md5[0] || !request->expected_track02_md5[0] ||
        !(file = fopen(request->srm_path, "rb"))) {
        receipt.status = THERON_V1_SRM_OPAQUE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (theron_v1_srm_opaque_is_symlink(request->srm_path) ||
        fstat(fileno(file), &status) != 0 || !S_ISREG(status.st_mode) ||
        status.st_size < 3 || (uintmax_t)status.st_size != request->expected_srm_size ||
        request->admission_format_version != THERON_V1_SRM_OPAQUE_ADMISSION_VERSION ||
        request->expected_shape != THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1 ||
        request->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        theron_v1_track02_variant_for_md5(request->expected_track02_md5) !=
            request->track02_variant || !m12_file_md5_hex(request->srm_path, initial_md5) ||
        strcmp(initial_md5, request->expected_srm_md5) ||
        fread(header, 1, sizeof(header), file) != sizeof(header) ||
        header[0] != 0x1f || header[1] != 0x8b || header[2] != 0x08 ||
        !m12_file_md5_hex(request->srm_path, final_md5) ||
        strcmp(initial_md5, final_md5)) {
        fclose(file);
        receipt.status = THERON_V1_SRM_OPAQUE_REJECTED;
        *out = receipt;
        return 1;
    }
    fclose(file);
    receipt.status = THERON_V1_SRM_OPAQUE_READY;
    receipt.source_regular_file_verified = 1;
    receipt.source_md5_verified = 1;
    receipt.source_size_verified = 1;
    receipt.admission_version_verified = 1;
    receipt.source_shape_verified = 1;
    receipt.track02_identity_verified = 1;
    receipt.opaque_save_route_ready = 1;
    receipt.srm_size = request->expected_srm_size;
    receipt.admission_format_version = request->admission_format_version;
    receipt.shape = request->expected_shape;
    receipt.track02_variant = request->track02_variant;
    snprintf(receipt.srm_path, sizeof(receipt.srm_path), "%s", request->srm_path);
    snprintf(receipt.srm_md5, sizeof(receipt.srm_md5), "%s", initial_md5);
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             request->expected_track02_md5);
    *out = receipt;
    return 1;
}
