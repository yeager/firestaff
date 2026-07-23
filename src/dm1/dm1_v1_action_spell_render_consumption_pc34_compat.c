#include "dm1_v1_action_spell_render_consumption_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_render_consumption_rect_is_pc34(
    const DM1_V1_ActionSpellHudPaintRectPc34 *rect,
    int x, int y, int w, int h)
{
    return rect && rect->x == x && rect->y == y && rect->w == w && rect->h == h;
}

static int
dm1_v1_action_spell_render_consumption_current_route_pc34(
    const DM1_V1_ActionSpellFinalHudFrameCommandPc34 *render,
    const DM1_V1_ActionSpellFinalHudFrameCommandPc34 *currentClear)
{
    if (!render || !currentClear ||
        render->kind != DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34 ||
        currentClear->kind != DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34) {
        return 0;
    }
    if (render->sourceGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
        render->sourceZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34) {
        return dm1_v1_action_spell_render_consumption_rect_is_pc34(
                   &currentClear->rect, 224, 77, 96, 45) &&
               dm1_v1_action_spell_render_consumption_rect_is_pc34(
                   &render->rect, 233, 77, 87, 45);
    }
    if (render->sourceGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
        render->sourceZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34) {
        return dm1_v1_action_spell_render_consumption_rect_is_pc34(
                   &currentClear->rect, 224, 42, 96, 33) &&
               dm1_v1_action_spell_render_consumption_rect_is_pc34(
                   &render->rect, 233, 42, 87, 25);
    }
    return 0;
}

static int
dm1_v1_action_spell_render_consumption_prior_clear_pc34(
    const DM1_V1_ActionSpellFinalHudFrameCommandPc34 *command)
{
    return command &&
           command->kind == DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34 &&
           (dm1_v1_action_spell_render_consumption_rect_is_pc34(
                &command->rect, 224, 77, 96, 45) ||
            dm1_v1_action_spell_render_consumption_rect_is_pc34(
                &command->rect, 224, 42, 96, 33));
}

int
dm1_v1_action_spell_render_consumption_build_pc34(
    const DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *bridge,
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *outReceipt)
{
    const DM1_V1_ActionSpellFinalHudFrameCommandPc34 *currentClear;
    const DM1_V1_ActionSpellFinalHudFrameCommandPc34 *render;
    int clearCount;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!bridge || !bridge->accepted || !bridge->suppressSyntheticFallback ||
        bridge->frameTick == 0 || bridge->sourceTick == 0 || bridge->serial == 0 ||
        bridge->commandFingerprint == 0 || bridge->orderingFingerprint == 0 ||
        bridge->lifecycleGeneration == 0 ||
        (bridge->commandCount != 2 && bridge->commandCount != 3)) {
        return 0;
    }
    clearCount = bridge->commandCount - 1;
    if (bridge->commandCount == 3 &&
        !dm1_v1_action_spell_render_consumption_prior_clear_pc34(
            &bridge->commands[0])) return 0;
    currentClear = &bridge->commands[clearCount - 1];
    render = &bridge->commands[clearCount];
    if (!dm1_v1_action_spell_render_consumption_current_route_pc34(
            render, currentClear)) return 0;

    outReceipt->accepted = 1;
    outReceipt->renderReadyForHost = 1;
    outReceipt->clearCount = clearCount;
    outReceipt->sourceGraphicId = render->sourceGraphicId;
    outReceipt->sourceZoneId = render->sourceZoneId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->clearRect = currentClear->rect;
    outReceipt->renderRect = render->rect;
    outReceipt->frameTick = bridge->frameTick;
    outReceipt->sourceTick = bridge->sourceTick;
    outReceipt->serial = bridge->serial;
    outReceipt->commandFingerprint = bridge->commandFingerprint;
    outReceipt->orderingFingerprint = bridge->orderingFingerprint;
    outReceipt->lifecycleGeneration = bridge->lifecycleGeneration;
    return 1;
}
