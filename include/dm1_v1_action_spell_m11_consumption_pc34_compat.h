#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CONSUMPTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CONSUMPTION_PC34_COMPAT_H

#include "dm1_v1_action_spell_host_route_lifecycle_pc34_compat.h"

enum {
    DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34 = 1,
    DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34 = 2
};

/*
 * Explicit source proof for a future M11 consumer. This module does not call
 * or alter M11; it fails closed before any host render consumption.
 */
typedef struct {
    int accepted;
    int m11ConsumptionReady;
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
} DM1_V1_ActionSpellM11ConsumptionReceiptPc34;

/*
 * Converts only an active current host image route into an explicit original
 * graphic/zone proof suitable for a later M11 consumer.
 */
int dm1_v1_action_spell_m11_consumption_build_pc34(
    const DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellM11ConsumptionReceiptPc34 *outReceipt);

#endif
