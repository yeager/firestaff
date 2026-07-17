#ifndef THERON_V1_SRM_OPAQUE_RUNTIME_H
#define THERON_V1_SRM_OPAQUE_RUNTIME_H

#include "theron_v1_srm_opaque_admission.h"

typedef struct {
    int opaque_save_route_ready;
    int save_restore_permitted;
    int save_semantics_decoded;
    int synthetic_fallback_used;
    char srm_md5[33];
    size_t srm_size;
    unsigned int admission_format_version;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
} Theron_V1SrmOpaqueRuntimeReceipt;

/* Retains only an already accepted source identity for later reviewed save
 * work. It does not expose SRM bytes or permit state restoration. */
int theron_v1_srm_opaque_runtime_consume(
    const Theron_V1SrmOpaqueAdmissionReceipt *admission,
    Theron_V1SrmOpaqueRuntimeReceipt *out);

#endif
