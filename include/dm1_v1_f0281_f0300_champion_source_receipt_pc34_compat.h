/* ReDMCSB REVIVE.C/CHAMPION.C/CHAMDRAW.C F0281-F0300 admission gate. */
#ifndef FIRESTAFF_DM1_V1_F0281_F0300_CHAMPION_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0281_F0300_CHAMPION_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0281F0300RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0281F0300RawSourcePc34;
typedef struct DM1_V1_F0281F0300InputPc34 { DM1_V1_F0281F0300RawSourcePc34 dungeonDat,graphicsDat,championData,inputData,m653Font; int championIndex,slotIndex,command; } DM1_V1_F0281F0300InputPc34;
typedef struct DM1_V1_F0281F0300ReceiptPc34 { int valid,suppressSyntheticUi; int f0281RenameAdmitted,f0282F0283ReviveAdmitted,f0284ToF0286ExistingOwners,f0287ToF0296PresentationAdmitted,f0297ToF0300HandSlotAdmitted; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0281F0300ReceiptPc34;
uint32_t dm1_v1_f0281_f0300_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0281_f0300_champion_source_receipt_pc34(const DM1_V1_F0281F0300InputPc34 *input,DM1_V1_F0281F0300ReceiptPc34 *outReceipt);
const char *dm1_v1_f0281_f0300_source_evidence_pc34(void);
#endif
