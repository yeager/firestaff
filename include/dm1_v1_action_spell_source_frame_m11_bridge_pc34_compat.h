#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_M11_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_action_spell_source_frame_evidence_pc34_compat.h"

/*
 * M11-facing source asset evidence. This bridge preserves only original asset
 * facts and retirement facts; it neither calls nor changes M11.
 */
typedef struct {
    int accepted;
    int m11SourceFrameReady;
    int presentationKind;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    int sourceCommandCount;
    int clearStaleSourceFrame;
    int revokeStaleSourceFrame;
    int staleOriginalGraphicId;
    int staleOriginalZoneId;
    int staleCompanionGraphicId;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34;

/*
 * Converts current live source evidence to an M11-facing receipt while
 * retaining only independently valid original asset clear/revoke facts.
 */
int dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(
    const DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *evidence,
    DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *outReceipt);

#endif
