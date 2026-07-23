/* ReDMCSB TIMELINE.C F0241-F0260 raw event admission. */
#ifndef FIRESTAFF_DM1_V1_F0241_F0260_TIMELINE_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0241_F0260_TIMELINE_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0241F0260RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0241F0260RawSourcePc34;
typedef struct DM1_V1_F0241F0260InputPc34 { DM1_V1_F0241F0260RawSourcePc34 dungeonDat,timelineEvent,thingTables,championData,graphicsDat; int eventType,eventIndex; } DM1_V1_F0241F0260InputPc34;
typedef struct DM1_V1_F0241F0260ReceiptPc34 { int valid,suppressSyntheticRuntime; int f0241ToF0246SquareAdmitted,f0247F0248ExistingOwners,f0249ExistingOwner,f0250ToF0255TransitionAdmitted,f0256CpseUnavailable,f0257LightAdmitted,f0258F0259ExistingOwner,f0260StatusAdmitted; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0241F0260ReceiptPc34;
uint32_t dm1_v1_f0241_f0260_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0241_f0260_timeline_source_receipt_pc34(const DM1_V1_F0241F0260InputPc34 *input,DM1_V1_F0241F0260ReceiptPc34 *outReceipt);
const char *dm1_v1_f0241_f0260_source_evidence_pc34(void);
#endif
