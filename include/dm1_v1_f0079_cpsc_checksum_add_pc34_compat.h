#ifndef FIRESTAFF_DM1_V1_F0079_CPSC_CHECKSUM_ADD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0079_CPSC_CHECKSUM_ADD_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0079CpscVariantPc34 {
    DM1_V1_F0079_CPSC_TERMINATED_ATARI_ST_PC34 = 1,
    DM1_V1_F0079_CPSC_TERMINATED_AMIGA_PC34 = 2,
    DM1_V1_F0079_CPSC_SKIP_TABLE_PC34 = 3
} DM1_V1_F0079CpscVariantPc34;

typedef struct DM1_V1_F0079CpscRequestPc34 {
    DM1_V1_F0079CpscVariantPc34 variant;
    const uint8_t *codeBytes;
    size_t codeByteCount;
    uint32_t codeImageAddress;
    uint32_t functionAddress;
    const uint8_t *skipTableBytes;
    size_t skipTableByteCount;
    int mediaSourceVerified;
} DM1_V1_F0079CpscRequestPc34;

typedef struct DM1_V1_F0079CpscReceiptPc34 {
    int accepted;
    DM1_V1_F0079CpscVariantPc34 variant;
    uint16_t checksum;
    size_t consumedByteCount;
    int suppressSyntheticFallback;
} DM1_V1_F0079CpscReceiptPc34;

/*
 * Bounded F0079_CPSC_GetChecksumAdd adapter. Code-image callers must supply
 * an authenticated executable span that contains the function entry and, if
 * present, its absolute JMP target. The skip-table variant consumes the
 * source length and offset/count pairs. Invalid or truncated spans fail
 * closed instead of scanning outside the caller-owned bytes.
 */
int dm1_v1_f0079_cpsc_checksum_add_pc34(
    const DM1_V1_F0079CpscRequestPc34 *request,
    DM1_V1_F0079CpscReceiptPc34 *outReceipt);

const char *dm1_v1_f0079_cpsc_checksum_add_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
