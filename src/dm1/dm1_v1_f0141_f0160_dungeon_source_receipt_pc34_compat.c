#include "dm1_v1_f0141_f0160_dungeon_source_receipt_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0141_f0160_fnv1a_pc34(const uint8_t *bytes, size_t byteCount)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes || byteCount == 0U) return 0U;
    for (i = 0U; i < byteCount; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static int source_ok(const DM1_V1_F0141F0160RawSourcePc34 *source, uint32_t *out)
{
    uint32_t hash;
    if (out) *out = 0U;
    if (!source || !source->authenticated || !source->bytes || source->byteCount == 0U) return 0;
    hash = dm1_v1_f0141_f0160_fnv1a_pc34(source->bytes, source->byteCount);
    if (hash == 0U || hash != source->fnv1a) return 0;
    if (out) *out = hash;
    return 1;
}

int dm1_v1_f0141_f0160_dungeon_source_receipt_pc34(
    const DM1_V1_F0141F0160RuntimeInputPc34 *input,
    DM1_V1_F0141F0160ReceiptPc34 *outReceipt)
{
    uint32_t a, b, c, d, fingerprint;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->suppressSyntheticRuntime = 1;
    outReceipt->sourceEvidence = dm1_v1_f0141_f0160_source_evidence_pc34();
    if (!input || input->thingType < 0 || input->thingType > 15 ||
        input->thingIndex < 0 || input->mapX < 0 || input->mapY < 0 ||
        !source_ok(&input->dungeonDat, &a) || !source_ok(&input->graphicsDat, &b) ||
        !source_ok(&input->thingTables, &c) || !source_ok(&input->mapSquares, &d)) return 0;
    fingerprint = 2166136261u;
    fingerprint ^= a; fingerprint *= 16777619u;
    fingerprint ^= b; fingerprint *= 16777619u;
    fingerprint ^= c; fingerprint *= 16777619u;
    fingerprint ^= d; fingerprint *= 16777619u;
    if (fingerprint == 0U) return 0;
    outReceipt->valid = 1;
    outReceipt->f0141ObjectInfoOwner = 1;
    outReceipt->f0142ProjectileAspectOwner = 1;
    outReceipt->f0143ToF0148GroupOwners = 1;
    outReceipt->f0149AlcoveOwner = 1;
    outReceipt->f0150ToF0155MapOwners = 1;
    outReceipt->f0156ToF0160ThingOwners = 1;
    outReceipt->sourceFingerprint = fingerprint;
    return 1;
}

const char *dm1_v1_f0141_f0160_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:1136-1228 F0141/F0142 object and projectile "
           "owners; 1230-1339 F0143-F0149 group/alcove owners; 1371-1582 "
           "F0150-F0155 map owners; 1584-1728 F0156-F0160 Thing and square "
           "owners. Existing DM1/CSB implementations remain the sole consumers.";
}
