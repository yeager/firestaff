/* ReDMCSB MEMORY/LZW F0481-F0500 source admission. */
#ifndef FIRESTAFF_DM1_V1_F0481_F0500_GRAPHICS_CACHE_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0481_F0500_GRAPHICS_CACHE_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0481F0500RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0481F0500RawSourcePc34;
typedef struct DM1_V1_F0481F0500InputPc34 { DM1_V1_F0481F0500RawSourcePc34 graphicsDat,graphicHeader,compressedGraphic,cacheState,lzwStream; int graphicIndex; } DM1_V1_F0481F0500InputPc34;
typedef struct DM1_V1_F0481F0500ReceiptPc34 { int valid,suppressSyntheticGraphics; int f0481ToF0494CacheGraphicsAdmitted,f0495ToF0497LzwAdmitted,f0496ExistingOwner,f0500AmigaUnavailable; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0481F0500ReceiptPc34;
uint32_t dm1_v1_f0481_f0500_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0481_f0500_graphics_cache_source_receipt_pc34(const DM1_V1_F0481F0500InputPc34 *input,DM1_V1_F0481F0500ReceiptPc34 *outReceipt);
const char *dm1_v1_f0481_f0500_source_evidence_pc34(void);
#endif
