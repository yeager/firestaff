#include "dm1_v1_action_spell_m11_host_render_lifecycle_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_m11_host_render_lifecycle_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_m11_host_render_lifecycle_original_valid_pc34(
    const DM1_V1_ActionSpellM11HostRenderReceiptPc34 *render)
{
    if (!render || !render->accepted || !render->hostRenderReady ||
        !render->suppressSyntheticFallback) return 0;
    if (render->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return render->originalGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               render->originalZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_host_render_lifecycle_rect_is_pc34(
                   &render->originalRenderRect, 233, 77, 87, 45);
    }
    if (render->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34) {
        return render->originalGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               render->originalZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_host_render_lifecycle_rect_is_pc34(
                   &render->originalRenderRect, 233, 42, 87, 25);
    }
    return 0;
}

static int
dm1_v1_action_spell_m11_host_render_lifecycle_same_pc34(
    const DM1_V1_ActionSpellM11HostRenderLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11HostRenderReceiptPc34 *render)
{
    return state->originalRouteKind == render->originalRouteKind &&
           state->originalGraphicId == render->originalGraphicId &&
           state->originalZoneId == render->originalZoneId &&
           state->sourceTick == render->sourceTick && state->serial == render->serial &&
           state->commandFingerprint == render->commandFingerprint &&
           state->orderingFingerprint == render->orderingFingerprint &&
           state->lifecycleGeneration == render->lifecycleGeneration;
}

int
dm1_v1_action_spell_m11_host_render_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11HostRenderLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellM11HostRenderReceiptPc34 *render,
    DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_m11_host_render_lifecycle_original_valid_pc34(
            render) || render->frameTick == 0 || render->sourceTick == 0 ||
        render->serial == 0 || render->commandFingerprint == 0 ||
        render->orderingFingerprint == 0 || render->lifecycleGeneration == 0) {
        return 0;
    }
    if (state->active && render->frameTick < state->frameTick) return 0;
    if (state->active && render->frameTick == state->frameTick &&
        !dm1_v1_action_spell_m11_host_render_lifecycle_same_pc34(state, render)) {
        return 0;
    }
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->hostRenderCurrent = 1;
        outReceipt->clearStaleHostRoute = state->active &&
            render->frameTick > state->frameTick;
        outReceipt->alreadyCurrent = state->active && render->frameTick == state->frameTick;
        outReceipt->originalRouteKind = render->originalRouteKind;
        outReceipt->originalGraphicId = render->originalGraphicId;
        outReceipt->originalZoneId = render->originalZoneId;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->originalRenderRect = render->originalRenderRect;
        if (outReceipt->clearStaleHostRoute) {
            outReceipt->staleOriginalRouteKind = state->originalRouteKind;
            outReceipt->staleOriginalGraphicId = state->originalGraphicId;
            outReceipt->staleOriginalZoneId = state->originalZoneId;
            outReceipt->staleClearRect = state->originalRenderRect;
        }
        outReceipt->frameTick = render->frameTick;
        outReceipt->sourceTick = render->sourceTick;
        outReceipt->serial = render->serial;
        outReceipt->commandFingerprint = render->commandFingerprint;
        outReceipt->orderingFingerprint = render->orderingFingerprint;
        outReceipt->lifecycleGeneration = render->lifecycleGeneration;
    }
    state->active = 1;
    state->originalRouteKind = render->originalRouteKind;
    state->originalGraphicId = render->originalGraphicId;
    state->originalZoneId = render->originalZoneId;
    state->originalRenderRect = render->originalRenderRect;
    state->frameTick = render->frameTick;
    state->sourceTick = render->sourceTick;
    state->serial = render->serial;
    state->commandFingerprint = render->commandFingerprint;
    state->orderingFingerprint = render->orderingFingerprint;
    state->lifecycleGeneration = render->lifecycleGeneration;
    return 1;
}
