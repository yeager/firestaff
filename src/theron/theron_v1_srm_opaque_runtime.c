#include <stdio.h>
#include <string.h>

#include "theron_v1_srm_opaque_runtime.h"

int theron_v1_srm_opaque_runtime_consume(
    const Theron_V1SrmOpaqueAdmissionReceipt *admission,
    Theron_V1SrmOpaqueRuntimeReceipt *out) {
    Theron_V1SrmOpaqueRuntimeReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!admission || admission->status != THERON_V1_SRM_OPAQUE_READY ||
        !admission->source_regular_file_verified || !admission->source_md5_verified ||
        !admission->source_size_verified || !admission->admission_version_verified ||
        !admission->source_shape_verified || !admission->track02_identity_verified ||
        !admission->opaque_save_route_ready || !admission->srm_md5[0] ||
        !admission->srm_size || !admission->track02_md5[0]) return 0;
    receipt.opaque_save_route_ready = 1;
    receipt.srm_size = admission->srm_size;
    receipt.admission_format_version = admission->admission_format_version;
    receipt.track02_variant = admission->track02_variant;
    snprintf(receipt.srm_md5, sizeof(receipt.srm_md5), "%s", admission->srm_md5);
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             admission->track02_md5);
    *out = receipt;
    return 1;
}
