#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_HOST_M11_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_HOST_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_action_spell_runtime_frame_lifecycle_pc34_compat.h"

/*
 * Source-owned M11-facing frame route. This bridge preserves original proof
 * and stale-output retirement facts but does not invoke or modify M11.
 */
typedef struct {
    int accepted;
    int m11HostOutputReady;
    int originalRouteKind;
    int originalGraphicId;
    int originalZoneId;
    int clearStaleHostOutput;
    int revokeStaleHostOutput;
    int staleOriginalRouteKind;
    int staleOriginalGraphicId;
    int staleOriginalZoneId;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 originalRenderRect;
    DM1_V1_ActionSpellHudPaintRectPc34 staleClearRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRuntimeHostM11BridgeReceiptPc34;

/*
 * Converts current runtime output to a M11-facing source route, retaining a
 * clear/revoke only for an explicitly proven stale original output.
 */
int dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(
    const DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 *runtime,
    DM1_V1_ActionSpellRuntimeHostM11BridgeReceiptPc34 *outReceipt);

#endif
