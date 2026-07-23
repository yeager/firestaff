/* ReDMCSB DUNGEON.C F0141-F0160 ownership gate.  This produces no synthetic
 * dungeon records or graphics: all existing function owners are admitted only
 * after their raw PC34 DUNGEON.DAT/GRAPHICS.DAT and Thing-table receipts agree. */
#ifndef FIRESTAFF_DM1_V1_F0141_F0160_DUNGEON_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0141_F0160_DUNGEON_SOURCE_RECEIPT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM1_V1_F0141F0160RawSourcePc34 {
    int authenticated;
    const uint8_t *bytes;
    size_t byteCount;
    uint32_t fnv1a;
} DM1_V1_F0141F0160RawSourcePc34;

typedef struct DM1_V1_F0141F0160RuntimeInputPc34 {
    DM1_V1_F0141F0160RawSourcePc34 dungeonDat;
    DM1_V1_F0141F0160RawSourcePc34 graphicsDat;
    DM1_V1_F0141F0160RawSourcePc34 thingTables;
    DM1_V1_F0141F0160RawSourcePc34 mapSquares;
    int thingType;
    int thingIndex;
    int mapX;
    int mapY;
} DM1_V1_F0141F0160RuntimeInputPc34;

typedef struct DM1_V1_F0141F0160ReceiptPc34 {
    int valid;
    int suppressSyntheticRuntime;
    int f0141ObjectInfoOwner;
    int f0142ProjectileAspectOwner;
    int f0143ToF0148GroupOwners;
    int f0149AlcoveOwner;
    int f0150ToF0155MapOwners;
    int f0156ToF0160ThingOwners;
    uint32_t sourceFingerprint;
    const char *sourceEvidence;
} DM1_V1_F0141F0160ReceiptPc34;

uint32_t dm1_v1_f0141_f0160_fnv1a_pc34(const uint8_t *bytes, size_t byteCount);
int dm1_v1_f0141_f0160_dungeon_source_receipt_pc34(
    const DM1_V1_F0141F0160RuntimeInputPc34 *input,
    DM1_V1_F0141F0160ReceiptPc34 *outReceipt);
const char *dm1_v1_f0141_f0160_source_evidence_pc34(void);

#endif
