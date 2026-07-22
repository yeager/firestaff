#include "dm1_v1_action_spell_m11_consumption_lifecycle_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_m11_consumption_lifecycle_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_m11_consumption_lifecycle_original_valid_pc34(
    const DM1_V1_ActionSpellM11ConsumptionReceiptPc34 *consumption)
{
    if (!consumption || !consumption->accepted || !consumption->m11ConsumptionReady ||
        !consumption->suppressSyntheticFallback) return 0;
    if (consumption->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return consumption->originalGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               consumption->originalZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_consumption_lifecycle_rect_is_pc34(
                   &consumption->originalRenderRect, 233, 77, 87, 45);
    }
    if (consumption->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34) {
        return consumption->originalGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               consumption->originalZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_consumption_lifecycle_rect_is_pc34(
                   &consumption->originalRenderRect, 233, 42, 87, 25);
    }
    return 0;
}

static int
dm1_v1_action_spell_m11_consumption_lifecycle_same_pc34(
    const DM1_V1_ActionSpellM11ConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11ConsumptionReceiptPc34 *consumption)
{
    return state->originalRouteKind == consumption->originalRouteKind &&
           state->originalGraphicId == consumption->originalGraphicId &&
           state->originalZoneId == consumption->originalZoneId &&
           state->sourceTick == consumption->sourceTick &&
           state->serial == consumption->serial &&
           state->commandFingerprint == consumption->commandFingerprint &&
           state->orderingFingerprint == consumption->orderingFingerprint &&
           state->lifecycleGeneration == consumption->lifecycleGeneration;
}

int
dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11ConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11ConsumptionReceiptPc34 *consumption,
    DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_m11_consumption_lifecycle_original_valid_pc34(
            consumption) || consumption->frameTick == 0 || consumption->sourceTick == 0 ||
        consumption->serial == 0 || consumption->commandFingerprint == 0 ||
        consumption->orderingFingerprint == 0 || consumption->lifecycleGeneration == 0) {
        return 0;
    }
    if (state->active && consumption->frameTick < state->frameTick) return 0;
    if (state->active && consumption->frameTick == state->frameTick &&
        !dm1_v1_action_spell_m11_consumption_lifecycle_same_pc34(state, consumption)) {
        return 0;
    }
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->m11ConsumptionCurrent = 1;
        outReceipt->retirePreviousM11Consumption = state->active &&
            consumption->frameTick > state->frameTick;
        outReceipt->alreadyCurrent = state->active &&
            consumption->frameTick == state->frameTick;
        outReceipt->originalRouteKind = consumption->originalRouteKind;
        outReceipt->originalGraphicId = consumption->originalGraphicId;
        outReceipt->originalZoneId = consumption->originalZoneId;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->originalRenderRect = consumption->originalRenderRect;
        outReceipt->frameTick = consumption->frameTick;
        outReceipt->sourceTick = consumption->sourceTick;
        outReceipt->serial = consumption->serial;
        outReceipt->commandFingerprint = consumption->commandFingerprint;
        outReceipt->orderingFingerprint = consumption->orderingFingerprint;
        outReceipt->lifecycleGeneration = consumption->lifecycleGeneration;
    }
    state->active = 1;
    state->originalRouteKind = consumption->originalRouteKind;
    state->originalGraphicId = consumption->originalGraphicId;
    state->originalZoneId = consumption->originalZoneId;
    state->frameTick = consumption->frameTick;
    state->sourceTick = consumption->sourceTick;
    state->serial = consumption->serial;
    state->commandFingerprint = consumption->commandFingerprint;
    state->orderingFingerprint = consumption->orderingFingerprint;
    state->lifecycleGeneration = consumption->lifecycleGeneration;
    return 1;
}
