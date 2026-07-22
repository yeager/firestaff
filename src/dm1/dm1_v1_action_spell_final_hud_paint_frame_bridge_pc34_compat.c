#include "dm1_v1_action_spell_final_hud_paint_frame_bridge_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_final_hud_paint_frame_bridge_matches_pc34(
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
    const DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *lifecycle)
{
    return paint && lifecycle && paint->accepted && lifecycle->accepted &&
           paint->suppressSyntheticFallback &&
           lifecycle->suppressSyntheticFallback &&
           paint->frameTick == lifecycle->frameTick &&
           paint->sourceTick == lifecycle->sourceTick &&
           paint->serial == lifecycle->serial &&
           paint->commandFingerprint == lifecycle->commandFingerprint &&
           paint->orderingFingerprint == lifecycle->orderingFingerprint &&
           paint->lifecycleGeneration == lifecycle->lifecycleGeneration;
}

static int
dm1_v1_action_spell_final_hud_paint_frame_bridge_append_clear_pc34(
    DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *outReceipt,
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int clearColor)
{
    DM1_V1_ActionSpellFinalHudFrameCommandPc34 *command;
    if (outReceipt->commandCount >= DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_MAX_PC34 ||
        !rect || rect->w <= 0 || rect->h <= 0) return 0;
    command = &outReceipt->commands[outReceipt->commandCount++];
    command->kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34;
    command->rect = *rect;
    command->clearColor = clearColor;
    return 1;
}

int
dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
    const DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *outReceipt)
{
    DM1_V1_ActionSpellFinalHudFrameCommandPc34 *command;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_action_spell_final_hud_paint_frame_bridge_matches_pc34(
            paint, lifecycle)) return 0;

    if (lifecycle->alreadyCurrent) {
        outReceipt->accepted = 1;
        outReceipt->suppressSyntheticFallback = 1;
    } else {
        if (!lifecycle->renderCurrentPaint || !lifecycle->clearCurrentPaint ||
            !paint->clearBeforeRender) return 0;
        if (lifecycle->clearPreviousPaint &&
            !dm1_v1_action_spell_final_hud_paint_frame_bridge_append_clear_pc34(
                outReceipt, &lifecycle->previousClearRect, paint->clearColor)) {
            return 0;
        }
        if (!dm1_v1_action_spell_final_hud_paint_frame_bridge_append_clear_pc34(
                outReceipt, &lifecycle->currentClearRect, paint->clearColor)) {
            return 0;
        }
        command = &outReceipt->commands[outReceipt->commandCount++];
        command->kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34;
        command->rect = lifecycle->renderRect;
        command->sourceGraphicId = lifecycle->sourceGraphicId;
        command->sourceZoneId = lifecycle->sourceZoneId;
        outReceipt->accepted = 1;
        outReceipt->suppressSyntheticFallback = 1;
    }
    outReceipt->frameTick = lifecycle->frameTick;
    outReceipt->sourceTick = lifecycle->sourceTick;
    outReceipt->serial = lifecycle->serial;
    outReceipt->commandFingerprint = lifecycle->commandFingerprint;
    outReceipt->orderingFingerprint = lifecycle->orderingFingerprint;
    outReceipt->lifecycleGeneration = lifecycle->lifecycleGeneration;
    return 1;
}
