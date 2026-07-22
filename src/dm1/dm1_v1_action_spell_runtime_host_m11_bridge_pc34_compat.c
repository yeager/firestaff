#include "dm1_v1_action_spell_runtime_host_m11_bridge_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_runtime_host_m11_bridge_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_runtime_host_m11_bridge_proof_valid_pc34(
    int routeKind, int graphicId, int zoneId,
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect)
{
    if (routeKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return graphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               zoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_runtime_host_m11_bridge_rect_is_pc34(
                   rect, 233, 77, 87, 45);
    }
    if (routeKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34) {
        return graphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               zoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_runtime_host_m11_bridge_rect_is_pc34(
                   rect, 233, 42, 87, 25);
    }
    return 0;
}

int
dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(
    const DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 *runtime,
    DM1_V1_ActionSpellRuntimeHostM11BridgeReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!runtime || !runtime->accepted || !runtime->hostOutputCurrent ||
        !runtime->suppressSyntheticFallback || runtime->frameTick == 0 ||
        runtime->sourceTick == 0 || runtime->serial == 0 ||
        runtime->commandFingerprint == 0 || runtime->orderingFingerprint == 0 ||
        runtime->lifecycleGeneration == 0 ||
        !dm1_v1_action_spell_runtime_host_m11_bridge_proof_valid_pc34(
            runtime->originalRouteKind, runtime->sourceGraphicId,
            runtime->sourceZoneId, &runtime->renderRect) ||
        runtime->clearStaleHostOutput != runtime->revokeStaleHostOutput) {
        return 0;
    }
    if (runtime->clearStaleHostOutput &&
        !dm1_v1_action_spell_runtime_host_m11_bridge_proof_valid_pc34(
            runtime->staleOriginalRouteKind, runtime->staleSourceGraphicId,
            runtime->staleSourceZoneId, &runtime->staleClearRect)) {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->m11HostOutputReady = 1;
    outReceipt->originalRouteKind = runtime->originalRouteKind;
    outReceipt->originalGraphicId = runtime->sourceGraphicId;
    outReceipt->originalZoneId = runtime->sourceZoneId;
    outReceipt->clearStaleHostOutput = runtime->clearStaleHostOutput;
    outReceipt->revokeStaleHostOutput = runtime->revokeStaleHostOutput;
    outReceipt->staleOriginalRouteKind = runtime->staleOriginalRouteKind;
    outReceipt->staleOriginalGraphicId = runtime->staleSourceGraphicId;
    outReceipt->staleOriginalZoneId = runtime->staleSourceZoneId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->originalRenderRect = runtime->renderRect;
    outReceipt->staleClearRect = runtime->staleClearRect;
    outReceipt->frameTick = runtime->frameTick;
    outReceipt->sourceTick = runtime->sourceTick;
    outReceipt->serial = runtime->serial;
    outReceipt->commandFingerprint = runtime->commandFingerprint;
    outReceipt->orderingFingerprint = runtime->orderingFingerprint;
    outReceipt->lifecycleGeneration = runtime->lifecycleGeneration;
    return 1;
}
