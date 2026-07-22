#include "dm1_v1_action_spell_final_hud_paint_frame_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_common(DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
           DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *lifecycle)
{
    memset(paint, 0, sizeof(*paint));
    memset(lifecycle, 0, sizeof(*lifecycle));
    paint->accepted = 1; paint->clearBeforeRender = 1;
    paint->suppressSyntheticFallback = 1; paint->frameTick = 901;
    paint->sourceTick = 101; paint->serial = 10;
    paint->commandFingerprint = 0x41u; paint->orderingFingerprint = 0x42u;
    paint->lifecycleGeneration = 13; paint->clearColor = 0;
    lifecycle->accepted = 1; lifecycle->clearPreviousPaint = 1;
    lifecycle->clearCurrentPaint = 1; lifecycle->renderCurrentPaint = 1;
    lifecycle->suppressSyntheticFallback = 1; lifecycle->frameTick = paint->frameTick;
    lifecycle->sourceTick = paint->sourceTick; lifecycle->serial = paint->serial;
    lifecycle->commandFingerprint = paint->commandFingerprint;
    lifecycle->orderingFingerprint = paint->orderingFingerprint;
    lifecycle->lifecycleGeneration = paint->lifecycleGeneration;
    lifecycle->previousClearRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 42, 96, 33 };
    lifecycle->currentClearRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 77, 96, 45 };
    lifecycle->renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    lifecycle->sourceGraphicId = 10; lifecycle->sourceZoneId = 11;
}

int
main(void)
{
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 paint;
    DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 bridge;

    set_common(&paint, &lifecycle);
    CHECK(dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
              &paint, &lifecycle, &bridge));
    CHECK(bridge.accepted && bridge.commandCount == 3 && bridge.suppressSyntheticFallback &&
          bridge.commands[0].kind == DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34 &&
          bridge.commands[0].rect.y == 42 &&
          bridge.commands[1].kind == DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34 &&
          bridge.commands[1].rect.y == 77 &&
          bridge.commands[2].kind == DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34 &&
          bridge.commands[2].sourceGraphicId == 10 && bridge.commands[2].sourceZoneId == 11);

    lifecycle.frameTick++;
    CHECK(!dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
              &paint, &lifecycle, &bridge));
    lifecycle.frameTick--;
    lifecycle.renderCurrentPaint = 0;
    CHECK(!dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
              &paint, &lifecycle, &bridge));
    lifecycle.renderCurrentPaint = 1;
    lifecycle.alreadyCurrent = 1;
    lifecycle.clearCurrentPaint = 0;
    lifecycle.renderCurrentPaint = 0;
    CHECK(dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
              &paint, &lifecycle, &bridge));
    CHECK(bridge.accepted && bridge.commandCount == 0);

    printf("%s\n", failures ? "failed" : "ok: action/spell final HUD paint frame bridge");
    return failures ? 1 : 0;
}
