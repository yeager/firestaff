#include "dm1_v1_action_spell_host_route_bridge_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_host_route_bridge_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_host_route_bridge_kind_pc34(
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *lifecycle)
{
    if (lifecycle->sourceGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
        lifecycle->sourceZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
        dm1_v1_action_spell_host_route_bridge_rect_is_pc34(
            &lifecycle->renderRect, 233, 77, 87, 45)) {
        return DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34;
    }
    if (lifecycle->sourceGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
        lifecycle->sourceZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
        dm1_v1_action_spell_host_route_bridge_rect_is_pc34(
            &lifecycle->renderRect, 233, 42, 87, 25)) {
        return DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34;
    }
    return 0;
}

int
dm1_v1_action_spell_host_route_bridge_build_pc34(
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *outReceipt)
{
    int routeKind;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!lifecycle || !lifecycle->accepted || !lifecycle->hostConsumptionCurrent ||
        !lifecycle->suppressSyntheticFallback || lifecycle->frameTick == 0 ||
        lifecycle->sourceTick == 0 || lifecycle->serial == 0 ||
        lifecycle->commandFingerprint == 0 || lifecycle->orderingFingerprint == 0 ||
        lifecycle->lifecycleGeneration == 0) {
        return 0;
    }
    routeKind = dm1_v1_action_spell_host_route_bridge_kind_pc34(lifecycle);
    if (!routeKind) return 0;

    outReceipt->accepted = 1;
    outReceipt->hostImageRouteActive = 1;
    outReceipt->hostImageRouteKind = routeKind;
    outReceipt->sourceGraphicId = lifecycle->sourceGraphicId;
    outReceipt->sourceZoneId = lifecycle->sourceZoneId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->renderRect = lifecycle->renderRect;
    outReceipt->frameTick = lifecycle->frameTick;
    outReceipt->sourceTick = lifecycle->sourceTick;
    outReceipt->serial = lifecycle->serial;
    outReceipt->commandFingerprint = lifecycle->commandFingerprint;
    outReceipt->orderingFingerprint = lifecycle->orderingFingerprint;
    outReceipt->lifecycleGeneration = lifecycle->lifecycleGeneration;
    return 1;
}
