#ifndef FIRESTAFF_DM1_V1_F0433_SAVE_COMMAND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0433_SAVE_COMMAND_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_original_save_pc34_handoff.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_F0433_PART_COUNT 5u

typedef struct DM1_V1_F0433SavePartPc34 {
    const uint8_t *plainBytes;
    size_t byteCount;
    uint16_t key;
    int sourceReceiptValid;
} DM1_V1_F0433SavePartPc34;

typedef struct DM1_V1_F0433SaveCommandRequestPc34 {
    /* Plain DM_SAVE_HEADER metadata from an authenticated PC34 source. F0430
     * replaces its first half and restores its second half after writing. */
    uint8_t headerPlain[DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES];
    int sourceHeaderReceiptValid;
    const uint16_t *sourceRandomWords;
    size_t sourceRandomWordCount;
    DM1_V1_F0433SavePartPc34 parts[DM1_V1_F0433_PART_COUNT];
} DM1_V1_F0433SaveCommandRequestPc34;

typedef struct DM1_V1_F0433SaveCommandReceiptPc34 {
    int accepted;
    size_t bytesWritten;
    size_t partOffsets[DM1_V1_F0433_PART_COUNT];
    uint16_t partChecksums[DM1_V1_F0433_PART_COUNT];
    uint32_t headerFingerprint;
    uint32_t partFingerprints[DM1_V1_F0433_PART_COUNT];
    int suppressSyntheticFallback;
} DM1_V1_F0433SaveCommandReceiptPc34;

/* ReDMCSB LOADSAVE.C F0433: source-bound PC34 prefix writer only. It writes
 * the raw 512-byte F0430 header plus GLOBAL_DATA, ACTIVE_GROUP, PARTY, EVENT,
 * and TIMELINE F0420 parts. Portrait and dungeon-tail/F0434 work, filesystem
 * I/O, menu flow, and any synthetic state construction are intentionally out
 * of scope. */
int dm1_v1_f0433_save_command_write_pc34(
    const DM1_V1_F0433SaveCommandRequestPc34 *request,
    uint8_t *destination,
    size_t destination_size,
    DM1_V1_F0433SaveCommandReceiptPc34 *out_receipt);

const char *dm1_v1_f0433_save_command_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
