#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_HOST_RENDER_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_HOST_RENDER_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_m11_host_render_pc34_compat.h"

/* Current original host-render identity retained across source ticks. */
typedef struct {
    int active;
    int originalRouteKind;
    int originalGraphicId;
    int originalZoneId;
    DM1_V1_ActionSpellHudPaintRectPc34 originalRenderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellM11HostRenderLifecycleStatePc34;

/*
 * Current host-render admission. A clear is published only when a previously
 * active original route becomes stale because a later source tick replaces it.
 */
typedef struct {
    int accepted;
    int hostRenderCurrent;
    int clearStaleHostRoute;
    int alreadyCurrent;
    int originalRouteKind;
    int originalGraphicId;
    int originalZoneId;
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
} DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34;

/*
 * Advances original host rendering by source tick. It retains the exact route
 * proof and never clears a current route or accepts stale input.
 */
int dm1_v1_action_spell_m11_host_render_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11HostRenderLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11HostRenderReceiptPc34 *render,
    DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *outReceipt);

#endif
