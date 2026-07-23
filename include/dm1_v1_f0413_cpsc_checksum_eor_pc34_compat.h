#ifndef FIRESTAFF_DM1_V1_F0413_CPSC_CHECKSUM_EOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0413_CPSC_CHECKSUM_EOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0413CpscVariantPc34 {
    DM1_V1_F0413_CPSC_PAIR_SPAN_PC34 = 1,
    DM1_V1_F0413_CPSC_TERMINATED_FUNCTION_PC34 = 2,
    DM1_V1_F0413_CPSC_SKIP_TABLE_PC34 = 3
} DM1_V1_F0413CpscVariantPc34;

typedef struct DM1_V1_F0413CpscRequestPc34 {
    DM1_V1_F0413CpscVariantPc34 variant;
    const uint8_t *functionBytes;
    size_t functionByteCount;
    const uint8_t *skipTableBytes;
    size_t skipTableByteCount;
    int mediaSourceVerified;
} DM1_V1_F0413CpscRequestPc34;

typedef struct DM1_V1_F0413CpscReceiptPc34 {
    int accepted;
    DM1_V1_F0413CpscVariantPc34 variant;
    uint16_t checksum;
    size_t consumedByteCount;
    int suppressSyntheticFallback;
} DM1_V1_F0413CpscReceiptPc34;

/*
 * Raw COPYPRO8.C F0413 only. The caller must provide bytes from an original
 * verified media image; malformed spans, missing sentinels, or incomplete
 * skip tables are rejected without a calculated fallback checksum.
 */
int dm1_v1_f0413_cpsc_checksum_eor_pc34(
    const DM1_V1_F0413CpscRequestPc34 *request,
    DM1_V1_F0413CpscReceiptPc34 *outReceipt);

const char *dm1_v1_f0413_cpsc_checksum_eor_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
