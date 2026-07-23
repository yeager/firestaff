/* ReDMCSB GROUP.C F0201-F0209 and PROJEXPL.C F0212-F0220 ownership receipt. */
#ifndef FIRESTAFF_DM1_V1_F0201_F0220_ACTION_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0201_F0220_ACTION_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0201F0220RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0201F0220RawSourcePc34;
typedef struct DM1_V1_F0201F0220InputPc34 {
    DM1_V1_F0201F0220RawSourcePc34 dungeonDat, groupC04, timelineC38, projectileC14, explosionC15;
    int eventType;
    int eventIndex;
} DM1_V1_F0201F0220InputPc34;
typedef struct DM1_V1_F0201F0220ReceiptPc34 {
    int valid, suppressSyntheticRuntime;
    int f0201ToF0208ExistingGroupOwners, f0209GroupEventAdmitted;
    int f0210F0211CopyProtectionUnavailable, f0212ToF0220ExistingProjectileOwners;
    uint32_t sourceFingerprint;
    const char *sourceEvidence;
} DM1_V1_F0201F0220ReceiptPc34;
uint32_t dm1_v1_f0201_f0220_fnv1a_pc34(const uint8_t *bytes, size_t byteCount);
int dm1_v1_f0201_f0220_action_source_receipt_pc34(const DM1_V1_F0201F0220InputPc34 *input, DM1_V1_F0201F0220ReceiptPc34 *outReceipt);
const char *dm1_v1_f0201_f0220_source_evidence_pc34(void);
#endif
