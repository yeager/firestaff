#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_HOST_ROUTE_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_HOST_ROUTE_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_action_spell_render_consumption_lifecycle_pc34_compat.h"

enum {
    DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34 = 1,
    DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34 = 2
};

/*
 * Final source-owned image route that a host renderer may consume. It does
 * not implement the host renderer and carries no substitute image route.
 */
typedef struct {
    int accepted;
    int hostImageRouteActive;
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
} DM1_V1_ActionSpellHostRouteBridgeReceiptPc34;

/*
 * Converts only a current action/spell consumption lifecycle receipt into an
 * active original image route for host consumption.
 */
int dm1_v1_action_spell_host_route_bridge_build_pc34(
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *outReceipt);

#endif
