#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_HOST_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_HOST_RENDER_PC34_COMPAT_H

#include "dm1_v1_action_spell_m11_consumption_lifecycle_pc34_compat.h"

/*
 * Exact source-owned image route for host rendering after M11-consumption
 * lifecycle admission. This is a receipt only and does not modify M11.
 */
typedef struct {
    int accepted;
    int hostRenderReady;
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
} DM1_V1_ActionSpellM11HostRenderReceiptPc34;

/*
 * Publishes a host-render route only from current M11 consumption and only
 * when its C010/C011 or C009/C013 proof still has exact PC34 geometry.
 */
int dm1_v1_action_spell_m11_host_render_build_pc34(
    const DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellM11HostRenderReceiptPc34 *outReceipt);

#endif
