/* ReDMCSB MENU/SAVEUTIL F0401-F0420 source admission. */
#ifndef FIRESTAFF_DM1_V1_F0401_F0420_RENDER_CORE_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0401_F0420_RENDER_CORE_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0401F0420RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0401F0420RawSourcePc34;
typedef struct DM1_V1_F0401F0420InputPc34 { DM1_V1_F0401F0420RawSourcePc34 dungeonDat,graphicsDat,championData,spellSymbols,saveBytes; int championIndex,actionIndex,savePart; } DM1_V1_F0401F0420InputPc34;
typedef struct DM1_V1_F0401F0420ReceiptPc34 { int valid,suppressSyntheticRuntime; int f0401ToF0412ActionSpellAdmitted,f0413CpseUnavailable,f0414ToF0420SaveAdmitted; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0401F0420ReceiptPc34;
uint32_t dm1_v1_f0401_f0420_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0401_f0420_render_core_source_receipt_pc34(const DM1_V1_F0401F0420InputPc34 *input,DM1_V1_F0401F0420ReceiptPc34 *outReceipt);
const char *dm1_v1_f0401_f0420_source_evidence_pc34(void);
#endif
