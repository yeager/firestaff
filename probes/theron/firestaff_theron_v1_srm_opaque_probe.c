#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "theron_v1_srm_opaque_runtime.h"

int main(int argc, char **argv) {
    Theron_V1SrmOpaqueAdmissionRequest request = {0};
    Theron_V1SrmOpaqueAdmissionReceipt admission;
    Theron_V1SrmOpaqueRuntimeReceipt runtime;
    char *end;
    unsigned long long value;

    if (argc == 1) {
        printf("SKIP: explicit SRM path, MD5, size, admission version, and Track 02 MD5 required\n");
        return 0;
    }
    if (argc != 7 || strcmp(argv[1], "--admit")) {
        fprintf(stderr, "usage: %s --admit <srm> <srm-md5> <size> <version> <track02-md5>\n",
                argv[0]);
        return 2;
    }
    value = strtoull(argv[4], &end, 10);
    if (!argv[4][0] || *end || value > SIZE_MAX) {
        fprintf(stderr, "REJECTED: invalid SRM size\n");
        return 1;
    }
    request.srm_path = argv[2];
    request.expected_srm_md5 = argv[3];
    request.expected_srm_size = (size_t)value;
    value = strtoull(argv[5], &end, 10);
    if (!argv[5][0] || *end || value > UINT_MAX) {
        fprintf(stderr, "REJECTED: invalid admission version\n");
        return 1;
    }
    request.admission_format_version = (unsigned int)value;
    request.expected_shape = THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1;
    request.expected_track02_md5 = argv[6];
    request.track02_variant = theron_v1_track02_variant_for_md5(argv[6]);
    if (!theron_v1_srm_opaque_admit(&request, &admission)) return 1;
    if (admission.status == THERON_V1_SRM_OPAQUE_UNAVAILABLE) {
        printf("SKIP: external SRM corpus unavailable\n");
        return 0;
    }
    if (admission.status != THERON_V1_SRM_OPAQUE_READY ||
        !theron_v1_srm_opaque_runtime_consume(&admission, &runtime)) {
        fprintf(stderr, "REJECTED: SRM provenance or Track 02 binding mismatch\n");
        return 1;
    }
    printf("READY: opaque SRM provenance retained; save restore disabled\n");
    return 0;
}
