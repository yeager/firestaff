#include "dm1_v1_action_spell_render_consumption_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_bridge(DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *bridge)
{
    memset(bridge, 0, sizeof(*bridge));
    bridge->accepted = 1; bridge->suppressSyntheticFallback = 1;
    bridge->commandCount = 3; bridge->frameTick = 901; bridge->sourceTick = 101;
    bridge->serial = 10; bridge->commandFingerprint = 0x41u;
    bridge->orderingFingerprint = 0x42u; bridge->lifecycleGeneration = 13;
    bridge->commands[0].kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34;
    bridge->commands[0].rect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 42, 96, 33 };
    bridge->commands[1].kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34;
    bridge->commands[1].rect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 77, 96, 45 };
    bridge->commands[2].kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34;
    bridge->commands[2].rect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    bridge->commands[2].sourceGraphicId = 10; bridge->commands[2].sourceZoneId = 11;
}

int
main(void)
{
    DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 bridge;
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 consumption;

    set_bridge(&bridge);
    CHECK(dm1_v1_action_spell_render_consumption_build_pc34(&bridge, &consumption));
    CHECK(consumption.accepted && consumption.renderReadyForHost &&
          consumption.clearCount == 2 && consumption.sourceGraphicId == 10 &&
          consumption.renderRect.x == 233 && consumption.renderRect.y == 77 &&
          consumption.suppressSyntheticFallback);

    bridge.commands[1].kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34;
    CHECK(!dm1_v1_action_spell_render_consumption_build_pc34(&bridge, &consumption));
    bridge.commands[1].kind = DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34;
    bridge.commands[2].rect.w = 88;
    CHECK(!dm1_v1_action_spell_render_consumption_build_pc34(&bridge, &consumption));
    bridge.commands[2].rect.w = 87;
    bridge.commandCount = 2;
    bridge.commands[0] = bridge.commands[1];
    bridge.commands[1] = bridge.commands[2];
    bridge.commands[1].sourceGraphicId = 9; bridge.commands[1].sourceZoneId = 13;
    bridge.commands[0].rect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 42, 96, 33 };
    bridge.commands[1].rect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    CHECK(dm1_v1_action_spell_render_consumption_build_pc34(&bridge, &consumption));
    CHECK(consumption.clearCount == 1 && consumption.sourceGraphicId == 9);

    printf("%s\n", failures ? "failed" : "ok: action/spell render consumption");
    return failures ? 1 : 0;
}
