/* ReDMCSB PANEL/COMMAND F0341-F0360 source admission. */
#ifndef FIRESTAFF_DM1_V1_F0341_F0360_RENDER_ACTION_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0341_F0360_RENDER_ACTION_SOURCE_RECEIPT_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct DM1_V1_F0341F0360RawSourcePc34 { int authenticated; const uint8_t *bytes; size_t byteCount; uint32_t fnv1a; } DM1_V1_F0341F0360RawSourcePc34;
typedef struct DM1_V1_F0341F0360InputPc34 { DM1_V1_F0341F0360RawSourcePc34 graphicsDat,m653Font,championData,thingTables,inputQueue; int championIndex,command; } DM1_V1_F0341F0360InputPc34;
typedef struct DM1_V1_F0341F0360ReceiptPc34 { int valid,suppressSyntheticRuntime; int f0341ToF0347ExistingPanelOwners,f0348ToF0355PanelActionAdmitted,f0356CpseUnavailable,f0357ToF0360InputAdmitted; uint32_t sourceFingerprint; const char *sourceEvidence; } DM1_V1_F0341F0360ReceiptPc34;
uint32_t dm1_v1_f0341_f0360_fnv1a_pc34(const uint8_t *bytes,size_t byteCount);
int dm1_v1_f0341_f0360_render_action_source_receipt_pc34(const DM1_V1_F0341F0360InputPc34 *input,DM1_V1_F0341F0360ReceiptPc34 *outReceipt);
const char *dm1_v1_f0341_f0360_source_evidence_pc34(void);
#endif
