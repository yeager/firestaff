#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_HOST_ROUTE_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_HOST_ROUTE_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_host_route_bridge_pc34_compat.h"

/* Current source-owned action/spell host image route identity. */
typedef struct {
    int active;
    int hostImageRouteKind;
    int sourceGraphicId;
    int sourceZoneId;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellHostRouteLifecycleStatePc34;

/*
 * Fence receipt for the active original image route. It authorizes no image
 * itself and cannot introduce a generated or fallback route.
 */
typedef struct {
    int accepted;
    int hostImageRouteCurrent;
    int retirePreviousHostImageRoute;
    int alreadyCurrent;
    int hostImageRouteKind;
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
} DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34;

/*
 * Advances the active host image route only to a newer source frame. Exact
 * replay is idempotent; stale and divergent same-tick routes are rejected.
 */
int dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
    DM1_V1_ActionSpellHostRouteLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *route,
    DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 *outReceipt);

#endif
