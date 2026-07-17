#ifndef THERON_V1_SRM_OPAQUE_ADMISSION_H
#define THERON_V1_SRM_OPAQUE_ADMISSION_H

#include <stddef.h>

#include "theron_v1_track02.h"

#define THERON_V1_SRM_OPAQUE_PATH_CAPACITY 512
#define THERON_V1_SRM_OPAQUE_ADMISSION_VERSION 1u

typedef enum {
    THERON_V1_SRM_OPAQUE_UNAVAILABLE = 0,
    THERON_V1_SRM_OPAQUE_REJECTED,
    THERON_V1_SRM_OPAQUE_READY
} Theron_V1SrmOpaqueAdmissionStatus;

/* This is a container admission version, not a claim about fields inside the
 * original Save Disk payload. The only accepted V1 shape is gzip/DEFLATE. */
typedef enum {
    THERON_V1_SRM_OPAQUE_SHAPE_UNKNOWN = 0,
    THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1
} Theron_V1SrmOpaqueShape;

typedef struct {
    const char *srm_path;
    const char *expected_srm_md5;
    size_t expected_srm_size;
    unsigned int admission_format_version;
    Theron_V1SrmOpaqueShape expected_shape;
    Theron_Track02Variant track02_variant;
    const char *expected_track02_md5;
} Theron_V1SrmOpaqueAdmissionRequest;

typedef struct {
    Theron_V1SrmOpaqueAdmissionStatus status;
    int source_regular_file_verified;
    int source_md5_verified;
    int source_size_verified;
    int admission_version_verified;
    int source_shape_verified;
    int track02_identity_verified;
    int opaque_save_route_ready;
    int save_semantics_decoded;
    int synthetic_fallback_used;
    char srm_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char srm_md5[33];
    size_t srm_size;
    unsigned int admission_format_version;
    Theron_V1SrmOpaqueShape shape;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
} Theron_V1SrmOpaqueAdmissionReceipt;

/* Verifies only source identity and a closed container shape. It never
 * inflates or decodes an SRM body, reconstructs save state, or opens a
 * fallback route. Missing corpus data is an honest UNAVAILABLE result. */
int theron_v1_srm_opaque_admit(
    const Theron_V1SrmOpaqueAdmissionRequest *request,
    Theron_V1SrmOpaqueAdmissionReceipt *out);

#endif
