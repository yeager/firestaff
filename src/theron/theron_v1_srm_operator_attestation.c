#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "asset_status_m12.h"
#include "theron_v1_srm_operator_attestation.h"
#include "theron_v1_track02_raw_media_intake.h"

static int direct_regular_path(const char *path) {
    struct stat status;
#if defined(_WIN32)
    DWORD attributes;
    if (!path || !path[0]) return 0;
    attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
        !(attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) &&
        stat(path, &status) == 0 && S_ISREG(status.st_mode);
#else
    struct stat link_status;
    return path && path[0] && lstat(path, &link_status) == 0 &&
        !S_ISLNK(link_status.st_mode) && stat(path, &status) == 0 &&
        S_ISREG(status.st_mode);
#endif
}

static int path_exists(const char *path) {
#if defined(_WIN32)
    return path && path[0] && GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat status;
    return path && path[0] && lstat(path, &status) == 0;
#endif
}

static int canonical_path(const char *path, char *out, size_t capacity)
{
#if defined(_WIN32)
    return path && out && capacity && _fullpath(out, path, capacity) != NULL;
#else
    (void)capacity;
    return path && out && realpath(path, out) != NULL;
#endif
}

static void manifest_parts(const char *path, char *root, size_t root_capacity,
                           char *name, size_t name_capacity)
{
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    const char *separator = backslash && (!slash || backslash > slash) ? backslash : slash;
    if (!separator) {
        snprintf(root, root_capacity, ".");
        snprintf(name, name_capacity, "%s", path);
        return;
    }
    snprintf(root, root_capacity, "%.*s", (int)(separator - path), path);
    snprintf(name, name_capacity, "%s", separator + 1);
}

int theron_v1_srm_operator_attest(
    const Theron_V1SrmOperatorAttestationRequest *request,
    Theron_V1SrmOperatorAttestationReceipt *out) {
    Theron_V1SrmOperatorAttestationReceipt receipt = {0};
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1SrmOpaqueAdmissionRequest admission_request = {0};
    Theron_V1SrmOpaqueAdmissionReceipt admission;
    struct stat srm_status;
    char srm_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char cue_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char root_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char name_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    const char *resolved_srm;
    const char *resolved_cue;

    if (!out) return 0;
    *out = receipt;
    if (!request || !request->srm_path || !request->track02_cue_path ||
        !request->expected_track02_md5 || !request->srm_path[0] ||
        !request->track02_cue_path[0] || !request->expected_track02_md5[0]) {
        receipt.status = THERON_V1_SRM_ATTEST_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (!direct_regular_path(request->srm_path)) {
        receipt.status = path_exists(request->srm_path) ? THERON_V1_SRM_ATTEST_REJECTED :
            THERON_V1_SRM_ATTEST_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (!direct_regular_path(request->track02_cue_path)) {
        receipt.status = path_exists(request->track02_cue_path) ? THERON_V1_SRM_ATTEST_REJECTED :
            THERON_V1_SRM_ATTEST_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (request->admission_format_version != THERON_V1_SRM_OPAQUE_ADMISSION_VERSION ||
        !(resolved_srm = canonical_path(request->srm_path, srm_path, sizeof(srm_path)) ? srm_path : NULL) ||
        !(resolved_cue = canonical_path(request->track02_cue_path, cue_path, sizeof(cue_path)) ? cue_path : NULL) ||
        stat(resolved_srm, &srm_status) != 0 || srm_status.st_size < 3 ||
        !theron_v1_track02_raw_media_intake_discover(resolved_cue, &media) ||
        media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        strcmp(media.track02_md5, request->expected_track02_md5) ||
        media.variant != theron_v1_track02_variant_for_md5(request->expected_track02_md5) ||
        !m12_file_md5_hex(resolved_srm, receipt.srm_md5)) {
        receipt.status = THERON_V1_SRM_ATTEST_REJECTED;
        *out = receipt;
        return 1;
    }
    admission_request.srm_path = resolved_srm;
    admission_request.expected_srm_md5 = receipt.srm_md5;
    admission_request.expected_srm_size = (size_t)srm_status.st_size;
    admission_request.admission_format_version = request->admission_format_version;
    admission_request.expected_shape = THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1;
    admission_request.track02_variant = media.variant;
    admission_request.expected_track02_md5 = media.track02_md5;
    if (!theron_v1_srm_opaque_admit(&admission_request, &admission) ||
        admission.status != THERON_V1_SRM_OPAQUE_READY) {
        receipt.status = THERON_V1_SRM_ATTEST_REJECTED;
        *out = receipt;
        return 1;
    }
    manifest_parts(resolved_srm, root_path, sizeof(root_path), name_path,
                   sizeof(name_path));
    snprintf(receipt.manifest_root, sizeof(receipt.manifest_root), "%s", root_path);
    snprintf(receipt.manifest_name, sizeof(receipt.manifest_name), "%s", name_path);
    receipt.status = THERON_V1_SRM_ATTEST_READY;
    receipt.srm_path_canonical = 1;
    receipt.track02_path_canonical = 1;
    receipt.source_regular_file_verified = 1;
    receipt.track02_provenance_verified = 1;
    receipt.opaque_admission_verified = 1;
    receipt.srm_size = (size_t)srm_status.st_size;
    receipt.admission_format_version = request->admission_format_version;
    receipt.track02_variant = media.variant;
    snprintf(receipt.canonical_srm_path, sizeof(receipt.canonical_srm_path), "%s", resolved_srm);
    snprintf(receipt.canonical_track02_cue_path, sizeof(receipt.canonical_track02_cue_path), "%s", resolved_cue);
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", media.track02_md5);
    *out = receipt;
    return 1;
}
