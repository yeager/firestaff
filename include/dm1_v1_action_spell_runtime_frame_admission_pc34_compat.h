#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_action_spell_m11_host_render_lifecycle_pc34_compat.h"
#include "dm1_v1_action_spell_render_consumption_lifecycle_pc34_compat.h"

/* Current source-owned runtime frame identity after both render chains agree. */
typedef struct {
    int active;
    int originalRouteKind;
    int sourceGraphicId;
    int sourceZoneId;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRuntimeFrameAdmissionStatePc34;

/*
 * Runtime admission proof spanning final paint consumption and M11 host-route
 * lifecycle. It is source-owned and does not invoke or alter M11.
 */
typedef struct {
    int accepted;
    int runtimeFrameCurrent;
    int clearStaleRuntimeFrame;
    int alreadyCurrent;
    int originalRouteKind;
    int sourceGraphicId;
    int sourceZoneId;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34;

/*
 * Admits one runtime frame only where final paint and M11 host-route receipts
 * prove the same original graphic, zone, geometry, and tick identity.
 */
int dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
    DM1_V1_ActionSpellRuntimeFrameAdmissionStatePc34 *state,
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *paint,
    const DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *hostRoute,
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *outReceipt);

#endif
