#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "theron_v1_srm_operator_attestation.h"

int main(int argc, char **argv) {
    Theron_V1SrmOperatorAttestationRequest request;
    Theron_V1SrmOperatorAttestationReceipt receipt;
    char *end;
    unsigned long version;

    if (argc == 1) {
        printf("SKIP: explicit SRM, Track 02 CUE, Track 02 MD5, and admission version required\n");
        return 0;
    }
    if (argc != 6 || strcmp(argv[1], "--attest")) {
        fprintf(stderr, "usage: %s --attest <srm> <track02-cue> <track02-md5> <version>\n", argv[0]);
        return 2;
    }
    version = strtoul(argv[5], &end, 10);
    if (!argv[5][0] || *end || version > UINT_MAX ||
        version != THERON_V1_SRM_OPAQUE_ADMISSION_VERSION) return 1;
    request.srm_path = argv[2];
    request.track02_cue_path = argv[3];
    request.expected_track02_md5 = argv[4];
    request.admission_format_version = (unsigned int)version;
    if (!theron_v1_srm_operator_attest(&request, &receipt)) return 1;
    if (receipt.status == THERON_V1_SRM_ATTEST_UNAVAILABLE) {
        printf("SKIP: external SRM or Track 02 provenance unavailable\n");
        return 0;
    }
    if (receipt.status != THERON_V1_SRM_ATTEST_READY) {
        fprintf(stderr, "REJECTED: SRM attestation mismatch\n");
        return 1;
    }
    printf("format=theron_srm_opaque_corpus_v1\nroot=%s\ncandidate=0:%s:%s:%zu:%u:%s\n",
           receipt.manifest_root, receipt.manifest_name, receipt.srm_md5,
           receipt.srm_size, receipt.admission_format_version, receipt.track02_md5);
    return 0;
}
