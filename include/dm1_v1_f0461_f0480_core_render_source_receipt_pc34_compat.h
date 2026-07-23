/* ReDMCSB START/MEMORY/CACHE F0461-F0480 source admission. */
#ifndef FIRESTAFF_DM1_V1_F0461_F0480_CORE_RENDER_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0461_F0480_CORE_RENDER_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0461F0480RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0461F0480RawSourcePc34;
typedef struct DM1_V1_F0461F0480InputPc34 { DM1_V1_F0461F0480RawSourcePc34 graphicsDat,graphicsHeader,wallMaterial,cacheState,allocationState; int graphicIndex; } DM1_V1_F0461F0480InputPc34;
typedef struct DM1_V1_F0461F0480ReceiptPc34 { int valid,suppressSyntheticRendering; int f0461ExistingWallOwner,f0462ToF0463StartAdmitted,f0464CpseUnavailable,f0466ToF0480MemoryGraphicsAdmitted; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0461F0480ReceiptPc34;
uint32_t dm1_v1_f0461_f0480_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0461_f0480_core_render_source_receipt_pc34(const DM1_V1_F0461F0480InputPc34 *input,DM1_V1_F0461F0480ReceiptPc34 *outReceipt);
const char *dm1_v1_f0461_f0480_source_evidence_pc34(void);
#endif
