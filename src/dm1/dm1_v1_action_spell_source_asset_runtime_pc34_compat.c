#include "dm1_v1_action_spell_source_asset_runtime_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static const DM1_V1_ActionSpellHudSurfacePc34 *
dm1_v1_action_spell_source_asset_runtime_find_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials, int graphicId)
{
    int i;
    if (!materials || !materials->surfaces || materials->surfaceCount <= 0) return 0;
    for (i = 0; i < materials->surfaceCount; ++i) {
        const DM1_V1_ActionSpellHudSurfacePc34 *surface = &materials->surfaces[i];
        if (surface->graphicId == graphicId) return surface;
    }
    return 0;
}

static int
dm1_v1_action_spell_source_asset_runtime_surface_pc34(
    const DM1_V1_ActionSpellHudSurfacePc34 *surface, int width, int height)
{
    return surface && surface->sourceOwned && surface->pixels &&
           surface->width == width && surface->height == height &&
           surface->pixelCount == width * height;
}

static int
dm1_v1_action_spell_source_asset_runtime_command_matches_pc34(
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState)
{
    int i;
    if (!commands || !frameState || commands->commandCount != frameState->commandCount ||
        commands->commandCount <= 0 ||
        commands->commandCount > DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34) return 0;
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *a = &commands->commands[i];
        const DM1_V1_ActionSpellRenderCommandPc34 *b = &frameState->commands[i];
        if (a->kind != b->kind || a->graphicId != b->graphicId ||
            a->zoneId != b->zoneId || a->zoneCount != b->zoneCount ||
            a->sourceX != b->sourceX || a->sourceY != b->sourceY ||
            a->sourceW != b->sourceW || a->sourceH != b->sourceH ||
            a->sourceSurfaceIndex != b->sourceSurfaceIndex) return 0;
    }
    return 1;
}

static int
dm1_v1_action_spell_source_asset_runtime_presentation_matches_pc34(
    int presentationKind, int originalRouteKind)
{
    if (originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        return presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    }
    return originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34 &&
           presentationKind >= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 &&
           presentationKind <= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34;
}

static int
dm1_v1_action_spell_source_asset_runtime_route_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *runtime,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    int *outCompanionGraphicId, int *outSourceAssetCount)
{
    const DM1_V1_ActionSpellHudSurfacePc34 *c009;
    const DM1_V1_ActionSpellHudSurfacePc34 *c010;
    const DM1_V1_ActionSpellHudSurfacePc34 *c011;
    int i;
    c009 = dm1_v1_action_spell_source_asset_runtime_find_pc34(materials, 9);
    c010 = dm1_v1_action_spell_source_asset_runtime_find_pc34(materials, 10);
    c011 = dm1_v1_action_spell_source_asset_runtime_find_pc34(materials, 11);
    if (runtime->originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        if (runtime->sourceGraphicId != DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 ||
            runtime->sourceZoneId != DM1_V1_ACTION_AREA_ZONE_ID_PC34 ||
            frameState->presentationKind != DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 ||
            !dm1_v1_action_spell_source_asset_runtime_surface_pc34(c010, 87, 45)) return 0;
        for (i = 0; i < frameState->commandCount; ++i) {
            const DM1_V1_ActionSpellRenderCommandPc34 *command = &frameState->commands[i];
            if (command->graphicId == 10 && command->zoneId == 11 &&
                command->sourceX == 0 && command->sourceY == 0 &&
                command->sourceW == 87 && command->sourceH == 45) {
                *outCompanionGraphicId = 0;
                *outSourceAssetCount = 1;
                return 1;
            }
        }
        return 0;
    }
    if (runtime->originalRouteKind != DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34 ||
        runtime->sourceGraphicId != DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 ||
        runtime->sourceZoneId != DM1_V1_SPELL_AREA_ZONE_ID_PC34 ||
        (frameState->presentationKind != DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 &&
         frameState->presentationKind != DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 &&
         frameState->presentationKind != DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34) ||
        !dm1_v1_action_spell_source_asset_runtime_surface_pc34(c009, 87, 25) ||
        !dm1_v1_action_spell_source_asset_runtime_surface_pc34(c011, 14, 39)) return 0;
    if (c010 && !c010->sourceOwned) return 0;
    for (i = 0; i < frameState->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command = &frameState->commands[i];
        if (command->graphicId == 9 && command->zoneId == 13 &&
            command->sourceX == 0 && command->sourceY == 0 &&
            command->sourceW == 87 && command->sourceH == 25) {
            int j;
            for (j = 0; j < frameState->commandCount; ++j) {
                const DM1_V1_ActionSpellRenderCommandPc34 *line = &frameState->commands[j];
                if (line->graphicId == 11 &&
                    (line->zoneId == DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34 ||
                     line->zoneId == DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34) &&
                    line->sourceX == 0 && (line->sourceY == 13 || line->sourceY == 26) &&
                    line->sourceW == 14 && line->sourceH == 12) {
                    *outCompanionGraphicId = 11;
                    *outSourceAssetCount = 2;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int
dm1_v1_action_spell_source_asset_runtime_build_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *runtime,
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *outReceipt)
{
    int companionGraphicId;
    int sourceAssetCount;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!materials || !commands || !frameState || !order || !runtime ||
        !commands->accepted || !commands->drawable ||
        commands->sourceOwnedCommandCount != commands->commandCount ||
        !frameState->frameOpen || !frameState->hasPresentation || !order->accepted ||
        !order->readyForPresentation || !runtime->accepted ||
        !runtime->runtimeFrameCurrent || !runtime->suppressSyntheticFallback ||
        !dm1_v1_action_spell_source_asset_runtime_presentation_matches_pc34(
            frameState->presentationKind, runtime->originalRouteKind) ||
        frameState->frameTick != order->frameTick ||
        frameState->frameTick != runtime->frameTick ||
        frameState->sourceTick != order->sourceTick ||
        frameState->sourceTick != runtime->sourceTick ||
        frameState->serial != order->serial || frameState->serial != runtime->serial ||
        frameState->commandFingerprint != order->commandFingerprint ||
        frameState->commandFingerprint != runtime->commandFingerprint ||
        order->commandCount != frameState->commandCount ||
        !dm1_v1_action_spell_source_asset_runtime_command_matches_pc34(commands, frameState) ||
        !dm1_v1_action_spell_source_asset_runtime_route_pc34(
            frameState, runtime, materials, &companionGraphicId, &sourceAssetCount)) {
        return 0;
    }
    outReceipt->accepted = 1;
    outReceipt->presentationKind = frameState->presentationKind;
    outReceipt->originalGraphicId = runtime->sourceGraphicId;
    outReceipt->originalZoneId = runtime->sourceZoneId;
    outReceipt->companionGraphicId = companionGraphicId;
    outReceipt->sourceAssetCount = sourceAssetCount;
    outReceipt->sourceCommandCount = frameState->commandCount;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->frameTick = frameState->frameTick;
    outReceipt->sourceTick = frameState->sourceTick;
    outReceipt->serial = frameState->serial;
    outReceipt->commandFingerprint = frameState->commandFingerprint;
    outReceipt->orderingFingerprint = order->orderingFingerprint;
    return 1;
}
