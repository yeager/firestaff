#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_render_consumption_pc34_compat.h"

/* Previous source-owned host-consumption identity for one action/spell HUD. */
typedef struct {
    int active;
    int sourceGraphicId;
    int sourceZoneId;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRenderConsumptionLifecycleStatePc34;

/*
 * Authorizes exactly one current render-consumption receipt. This is a gate
 * before host/M11 use, not a host renderer or a fallback paint path.
 */
typedef struct {
    int accepted;
    int hostConsumptionCurrent;
    int retirePreviousHostConsumption;
    int alreadyCurrent;
    int suppressSyntheticFallback;
    int sourceGraphicId;
    int sourceZoneId;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34;

/*
 * Advances host consumption only to a newer source frame. An identical replay
 * of the current receipt is harmless; stale or same-tick divergent receipts
 * are rejected before reaching the host.
 */
int dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
    DM1_V1_ActionSpellRenderConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *consumption,
    DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *outReceipt);

#endif
