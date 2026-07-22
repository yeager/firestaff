#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_m11_consumption_pc34_compat.h"

/* Persistent original graphic/zone proof for current M11 consumption. */
typedef struct {
    int active;
    int originalRouteKind;
    int originalGraphicId;
    int originalZoneId;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellM11ConsumptionLifecycleStatePc34;

/*
 * Source-owned tick fence for M11-ready action/spell material. It retains the
 * exact C010/C011 or C009/C013 proof and does not invoke M11.
 */
typedef struct {
    int accepted;
    int m11ConsumptionCurrent;
    int retirePreviousM11Consumption;
    int alreadyCurrent;
    int originalRouteKind;
    int originalGraphicId;
    int originalZoneId;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 originalRenderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34;

/*
 * Advances only to a newer explicit original route proof. Exact current replay
 * is idempotent; stale and same-tick divergent M11 consumption are rejected.
 */
int dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11ConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11ConsumptionReceiptPc34 *consumption,
    DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *outReceipt);

#endif
