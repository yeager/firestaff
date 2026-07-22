#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_FRAME_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_FRAME_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_runtime_frame_admission_pc34_compat.h"

/* Current source-owned runtime frame proof whose host output is active. */
typedef struct {
    int active;
    int originalRouteKind;
    int sourceGraphicId;
    int sourceZoneId;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRuntimeFrameLifecycleStatePc34;

/*
 * Runtime frame output admission. Clear/revoke fields describe only a prior
 * proof displaced by a later tick; no current or synthetic route is cleared.
 */
typedef struct {
    int accepted;
    int hostOutputCurrent;
    int clearStaleHostOutput;
    int revokeStaleHostOutput;
    int alreadyCurrent;
    int originalRouteKind;
    int sourceGraphicId;
    int sourceZoneId;
    int staleOriginalRouteKind;
    int staleSourceGraphicId;
    int staleSourceZoneId;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    DM1_V1_ActionSpellHudPaintRectPc34 staleClearRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34;

/*
 * Advances current runtime-frame host output to a later original proof and
 * emits clear/revoke only for the proof that became stale.
 */
int dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(
    DM1_V1_ActionSpellRuntimeFrameLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *admission,
    DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 *outReceipt);

#endif
