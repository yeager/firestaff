#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_M11_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_FRAME_M11_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_source_frame_m11_bridge_pc34_compat.h"

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
} DM1_V1_ActionSpellSourceFrameM11LifecycleStatePc34;

/* Current M11-facing source asset evidence, with stale output retirement. */
typedef struct {
    int accepted;
    int m11SourceFrameCurrent;
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
} DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34;

/* Advances M11-facing source assets only to a newer frame; stale input fails. */
int dm1_v1_action_spell_source_frame_m11_lifecycle_apply_pc34(
    DM1_V1_ActionSpellSourceFrameM11LifecycleStatePc34 *state,
    const DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *bridge,
    DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *outReceipt);

#endif
