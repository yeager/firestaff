#include "dm1_v1_action_spell_runtime_frame_lifecycle_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_runtime_frame_lifecycle_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_runtime_frame_lifecycle_original_valid_pc34(
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *admission)
{
    if (!admission || !admission->accepted || !admission->runtimeFrameCurrent ||
        !admission->suppressSyntheticFallback) return 0;
    if (admission->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return admission->sourceGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               admission->sourceZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_runtime_frame_lifecycle_rect_is_pc34(
                   &admission->renderRect, 233, 77, 87, 45);
    }
    if (admission->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34) {
        return admission->sourceGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               admission->sourceZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               dm1_v1_action_spell_runtime_frame_lifecycle_rect_is_pc34(
                   &admission->renderRect, 233, 42, 87, 25);
    }
    return 0;
}

static int
dm1_v1_action_spell_runtime_frame_lifecycle_same_pc34(
    const DM1_V1_ActionSpellRuntimeFrameLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *admission)
{
    return state->originalRouteKind == admission->originalRouteKind &&
           state->sourceGraphicId == admission->sourceGraphicId &&
           state->sourceZoneId == admission->sourceZoneId &&
           state->sourceTick == admission->sourceTick && state->serial == admission->serial &&
           state->commandFingerprint == admission->commandFingerprint &&
           state->orderingFingerprint == admission->orderingFingerprint &&
           state->lifecycleGeneration == admission->lifecycleGeneration;
}

int
dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(
    DM1_V1_ActionSpellRuntimeFrameLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *admission,
    DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_runtime_frame_lifecycle_original_valid_pc34(
            admission) || admission->frameTick == 0 || admission->sourceTick == 0 ||
        admission->serial == 0 || admission->commandFingerprint == 0 ||
        admission->orderingFingerprint == 0 || admission->lifecycleGeneration == 0) {
        return 0;
    }
    if (state->active && admission->frameTick < state->frameTick) return 0;
    if (state->active && admission->frameTick == state->frameTick &&
        !dm1_v1_action_spell_runtime_frame_lifecycle_same_pc34(state, admission)) return 0;
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->hostOutputCurrent = 1;
        outReceipt->clearStaleHostOutput = state->active &&
            admission->frameTick > state->frameTick;
        outReceipt->revokeStaleHostOutput = outReceipt->clearStaleHostOutput;
        outReceipt->alreadyCurrent = state->active && admission->frameTick == state->frameTick;
        outReceipt->originalRouteKind = admission->originalRouteKind;
        outReceipt->sourceGraphicId = admission->sourceGraphicId;
        outReceipt->sourceZoneId = admission->sourceZoneId;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->renderRect = admission->renderRect;
        if (outReceipt->clearStaleHostOutput) {
            outReceipt->staleOriginalRouteKind = state->originalRouteKind;
            outReceipt->staleSourceGraphicId = state->sourceGraphicId;
            outReceipt->staleSourceZoneId = state->sourceZoneId;
            outReceipt->staleClearRect = state->renderRect;
        }
        outReceipt->frameTick = admission->frameTick;
        outReceipt->sourceTick = admission->sourceTick;
        outReceipt->serial = admission->serial;
        outReceipt->commandFingerprint = admission->commandFingerprint;
        outReceipt->orderingFingerprint = admission->orderingFingerprint;
        outReceipt->lifecycleGeneration = admission->lifecycleGeneration;
    }
    state->active = 1;
    state->originalRouteKind = admission->originalRouteKind;
    state->sourceGraphicId = admission->sourceGraphicId;
    state->sourceZoneId = admission->sourceZoneId;
    state->renderRect = admission->renderRect;
    state->frameTick = admission->frameTick;
    state->sourceTick = admission->sourceTick;
    state->serial = admission->serial;
    state->commandFingerprint = admission->commandFingerprint;
    state->orderingFingerprint = admission->orderingFingerprint;
    state->lifecycleGeneration = admission->lifecycleGeneration;
    return 1;
}
