#include "dm1_v1_action_spell_host_route_lifecycle_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_host_route_lifecycle_valid_pc34(
    const DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *route)
{
    return route && route->accepted && route->hostImageRouteActive &&
           route->suppressSyntheticFallback &&
           (route->hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34 ||
            route->hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34) &&
           route->sourceGraphicId > 0 && route->sourceZoneId > 0 &&
           route->renderRect.w > 0 && route->renderRect.h > 0 &&
           route->frameTick > 0 && route->sourceTick > 0 && route->serial > 0 &&
           route->commandFingerprint > 0 && route->orderingFingerprint > 0 &&
           route->lifecycleGeneration > 0;
}

static int
dm1_v1_action_spell_host_route_lifecycle_same_pc34(
    const DM1_V1_ActionSpellHostRouteLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *route)
{
    return state->hostImageRouteKind == route->hostImageRouteKind &&
           state->sourceGraphicId == route->sourceGraphicId &&
           state->sourceZoneId == route->sourceZoneId &&
           state->sourceTick == route->sourceTick && state->serial == route->serial &&
           state->commandFingerprint == route->commandFingerprint &&
           state->orderingFingerprint == route->orderingFingerprint &&
           state->lifecycleGeneration == route->lifecycleGeneration;
}

int
dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
    DM1_V1_ActionSpellHostRouteLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 *route,
    DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_host_route_lifecycle_valid_pc34(route)) {
        return 0;
    }
    if (state->active) {
        if (route->frameTick < state->frameTick) return 0;
        if (route->frameTick == state->frameTick &&
            !dm1_v1_action_spell_host_route_lifecycle_same_pc34(state, route)) {
            return 0;
        }
    }
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->hostImageRouteCurrent = 1;
        outReceipt->retirePreviousHostImageRoute = state->active &&
            route->frameTick > state->frameTick;
        outReceipt->alreadyCurrent = state->active &&
            route->frameTick == state->frameTick;
        outReceipt->hostImageRouteKind = route->hostImageRouteKind;
        outReceipt->sourceGraphicId = route->sourceGraphicId;
        outReceipt->sourceZoneId = route->sourceZoneId;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->renderRect = route->renderRect;
        outReceipt->frameTick = route->frameTick;
        outReceipt->sourceTick = route->sourceTick;
        outReceipt->serial = route->serial;
        outReceipt->commandFingerprint = route->commandFingerprint;
        outReceipt->orderingFingerprint = route->orderingFingerprint;
        outReceipt->lifecycleGeneration = route->lifecycleGeneration;
    }
    state->active = 1;
    state->hostImageRouteKind = route->hostImageRouteKind;
    state->sourceGraphicId = route->sourceGraphicId;
    state->sourceZoneId = route->sourceZoneId;
    state->frameTick = route->frameTick;
    state->sourceTick = route->sourceTick;
    state->serial = route->serial;
    state->commandFingerprint = route->commandFingerprint;
    state->orderingFingerprint = route->orderingFingerprint;
    state->lifecycleGeneration = route->lifecycleGeneration;
    return 1;
}
