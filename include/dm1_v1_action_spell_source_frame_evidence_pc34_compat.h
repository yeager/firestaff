#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_EVIDENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_EVIDENCE_PC34_COMPAT_H

#include "dm1_v1_action_spell_source_asset_runtime_pc34_compat.h"

/* Current source-asset proof retained for the live action/spell HUD frame. */
typedef struct {
    int active;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellSourceFrameEvidenceStatePc34;

/*
 * Live source-gated frame evidence. Clear/revoke applies only to a previous
 * source proof displaced by a later tick, never to a current route.
 */
typedef struct {
    int accepted;
    int liveSourceFrameCurrent;
    int clearStaleSourceFrame;
    int revokeStaleSourceFrame;
    int alreadyCurrent;
    int presentationKind;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    int sourceCommandCount;
    int staleOriginalGraphicId;
    int staleOriginalZoneId;
    int staleCompanionGraphicId;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34;

/*
 * Consumes a fully source-owned asset-runtime receipt into one live frame
 * proof, with monotonic per-tick stale clear/revoke behavior.
 */
int dm1_v1_action_spell_source_frame_evidence_apply_pc34(
    DM1_V1_ActionSpellSourceFrameEvidenceStatePc34 *state,
    const DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *assets,
    DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *outReceipt);

#endif
