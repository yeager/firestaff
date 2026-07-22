#include "dm1_v1_action_spell_final_hud_paint_lifecycle_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_final_hud_paint_rect_valid_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect)
{
    return rect && rect->x >= 0 && rect->y >= 0 && rect->w > 0 && rect->h > 0;
}

static int
dm1_v1_action_spell_final_hud_paint_same_pc34(
    const DM1_V1_ActionSpellFinalHudPaintLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint)
{
    return state->presentationKind == paint->presentationKind &&
           state->sourceTick == paint->sourceTick &&
           state->serial == paint->serial &&
           state->commandFingerprint == paint->commandFingerprint &&
           state->orderingFingerprint == paint->orderingFingerprint &&
           state->lifecycleGeneration == paint->lifecycleGeneration;
}

int
dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
    DM1_V1_ActionSpellFinalHudPaintLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
    DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !paint || !paint->accepted || !paint->clearBeforeRender ||
        !paint->suppressSyntheticFallback || paint->frameTick == 0 ||
        paint->sourceTick == 0 || paint->serial == 0 ||
        paint->commandFingerprint == 0 || paint->orderingFingerprint == 0 ||
        paint->lifecycleGeneration == 0 ||
        !dm1_v1_action_spell_final_hud_paint_rect_valid_pc34(&paint->clearRect) ||
        !dm1_v1_action_spell_final_hud_paint_rect_valid_pc34(&paint->renderRect)) {
        return 0;
    }
    if (state->active) {
        if (paint->frameTick < state->frameTick) return 0;
        if (paint->frameTick == state->frameTick) {
            if (!dm1_v1_action_spell_final_hud_paint_same_pc34(state, paint)) {
                return 0;
            }
            if (outReceipt) {
                outReceipt->accepted = 1;
                outReceipt->alreadyCurrent = 1;
                outReceipt->suppressSyntheticFallback = 1;
                outReceipt->renderRect = paint->renderRect;
                outReceipt->sourceGraphicId = paint->sourceGraphicId;
                outReceipt->sourceZoneId = paint->sourceZoneId;
                outReceipt->clearColor = paint->clearColor;
                outReceipt->frameTick = paint->frameTick;
                outReceipt->sourceTick = paint->sourceTick;
                outReceipt->serial = paint->serial;
                outReceipt->commandFingerprint = paint->commandFingerprint;
                outReceipt->orderingFingerprint = paint->orderingFingerprint;
                outReceipt->lifecycleGeneration = paint->lifecycleGeneration;
            }
            return 1;
        }
    }

    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->clearPreviousPaint = state->active;
        outReceipt->clearCurrentPaint = 1;
        outReceipt->renderCurrentPaint = 1;
        outReceipt->suppressSyntheticFallback = 1;
        if (state->active) outReceipt->previousClearRect = state->previousClearRect;
        outReceipt->currentClearRect = paint->clearRect;
        outReceipt->renderRect = paint->renderRect;
        outReceipt->sourceGraphicId = paint->sourceGraphicId;
        outReceipt->sourceZoneId = paint->sourceZoneId;
        outReceipt->clearColor = paint->clearColor;
        outReceipt->frameTick = paint->frameTick;
        outReceipt->sourceTick = paint->sourceTick;
        outReceipt->serial = paint->serial;
        outReceipt->commandFingerprint = paint->commandFingerprint;
        outReceipt->orderingFingerprint = paint->orderingFingerprint;
        outReceipt->lifecycleGeneration = paint->lifecycleGeneration;
    }
    state->active = 1;
    state->presentationKind = paint->presentationKind;
    state->previousClearRect = paint->clearRect;
    state->frameTick = paint->frameTick;
    state->sourceTick = paint->sourceTick;
    state->serial = paint->serial;
    state->commandFingerprint = paint->commandFingerprint;
    state->orderingFingerprint = paint->orderingFingerprint;
    state->lifecycleGeneration = paint->lifecycleGeneration;
    return 1;
}
