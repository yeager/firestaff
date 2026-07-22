#include "dm1_v1_action_spell_m11_host_render_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_m11_host_render_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_m11_host_render_original_valid_pc34(
    const DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *lifecycle)
{
    if (lifecycle->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return lifecycle->originalGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               lifecycle->originalZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_host_render_rect_is_pc34(
                   &lifecycle->originalRenderRect, 233, 77, 87, 45);
    }
    if (lifecycle->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34) {
        return lifecycle->originalGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               lifecycle->originalZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_m11_host_render_rect_is_pc34(
                   &lifecycle->originalRenderRect, 233, 42, 87, 25);
    }
    return 0;
}

int
dm1_v1_action_spell_m11_host_render_build_pc34(
    const DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellM11HostRenderReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!lifecycle || !lifecycle->accepted || !lifecycle->m11ConsumptionCurrent ||
        !lifecycle->suppressSyntheticFallback || lifecycle->frameTick == 0 ||
        lifecycle->sourceTick == 0 || lifecycle->serial == 0 ||
        lifecycle->commandFingerprint == 0 || lifecycle->orderingFingerprint == 0 ||
        lifecycle->lifecycleGeneration == 0 ||
        !dm1_v1_action_spell_m11_host_render_original_valid_pc34(lifecycle)) {
        return 0;
    }
    outReceipt->accepted = 1;
    outReceipt->hostRenderReady = 1;
    outReceipt->originalRouteKind = lifecycle->originalRouteKind;
    outReceipt->originalGraphicId = lifecycle->originalGraphicId;
    outReceipt->originalZoneId = lifecycle->originalZoneId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->originalRenderRect = lifecycle->originalRenderRect;
    outReceipt->frameTick = lifecycle->frameTick;
    outReceipt->sourceTick = lifecycle->sourceTick;
    outReceipt->serial = lifecycle->serial;
    outReceipt->commandFingerprint = lifecycle->commandFingerprint;
    outReceipt->orderingFingerprint = lifecycle->orderingFingerprint;
    outReceipt->lifecycleGeneration = lifecycle->lifecycleGeneration;
    return 1;
}
