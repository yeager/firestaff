#include "dm1_v1_action_spell_source_asset_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static unsigned char c009[87 * 25], c010[87 * 45], c011[14 * 39], font[1];

static void
set_common(DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
           DM1_V1_ActionSpellHudSurfacePc34 surfaces[4],
           DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
           DM1_V1_ActionSpellPresentationFrameStatePc34 *frame,
           DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
           DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *runtime,
           int action)
{
    int commandCount = action ? 3 : 4;
    memset(surfaces, 0, 4 * sizeof(*surfaces));
    surfaces[0] = (DM1_V1_ActionSpellHudSurfacePc34){ 9, 87, 25, 87 * 25, c009, 1 };
    surfaces[1] = (DM1_V1_ActionSpellHudSurfacePc34){ 10, 87, 45, 87 * 45, c010, 1 };
    surfaces[2] = (DM1_V1_ActionSpellHudSurfacePc34){ 11, 14, 39, 14 * 39, c011, 1 };
    surfaces[3] = (DM1_V1_ActionSpellHudSurfacePc34){ 695, 1, 1, 1, font, 1 };
    materials->surfaces = surfaces; materials->surfaceCount = 4;
    memset(commands, 0, sizeof(*commands)); memset(frame, 0, sizeof(*frame));
    memset(order, 0, sizeof(*order)); memset(runtime, 0, sizeof(*runtime));
    commands->accepted = commands->drawable = 1;
    commands->commandCount = commands->sourceOwnedCommandCount = commandCount;
    frame->frameOpen = frame->hasPresentation = 1; frame->commandCount = commandCount;
    frame->frameTick = order->frameTick = runtime->frameTick = 900;
    frame->sourceTick = order->sourceTick = runtime->sourceTick = 70;
    frame->serial = order->serial = runtime->serial = 9;
    frame->commandFingerprint = order->commandFingerprint = runtime->commandFingerprint = 0x1234u;
    order->accepted = order->readyForPresentation = 1; order->commandCount = commandCount;
    order->orderingFingerprint = 0x9876u;
    runtime->accepted = runtime->runtimeFrameCurrent = 1;
    runtime->suppressSyntheticFallback = 1; runtime->orderingFingerprint = order->orderingFingerprint;
    runtime->lifecycleGeneration = 12;
    if (action) {
        frame->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
        runtime->originalRouteKind = DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34;
        runtime->sourceGraphicId = 10; runtime->sourceZoneId = 11;
        frame->commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 10, 11, 1, 0, 0, 87, 45, 1 };
        frame->commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 80, 1, 0, 0, 0, 0, 3 };
        frame->commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 85, 1, 0, 0, 0, 0, 3 };
    } else {
        frame->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
        runtime->originalRouteKind = DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
        runtime->sourceGraphicId = 9; runtime->sourceZoneId = 13;
        frame->commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 9, 13, 1, 0, 0, 87, 25, 0 };
        frame->commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 245, 6, 0, 13, 14, 12, 2 };
        frame->commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 261, 4, 0, 26, 14, 12, 2 };
        frame->commands[3] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 221, 1, 0, 0, 0, 0, 3 };
    }
    memcpy(commands->commands, frame->commands, (size_t)commandCount * sizeof(frame->commands[0]));
}

int
main(void)
{
    DM1_V1_ActionSpellHudMaterialSetPc34 materials;
    DM1_V1_ActionSpellHudSurfacePc34 surfaces[4];
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;
    DM1_V1_ActionSpellPresentationFrameStatePc34 frame;
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 order;
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 runtime;
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 receipt;

    set_common(&materials, surfaces, &commands, &frame, &order, &runtime, 0);
    CHECK(dm1_v1_action_spell_source_asset_runtime_build_pc34(
              &materials, &commands, &frame, &order, &runtime, &receipt));
    CHECK(receipt.accepted && receipt.originalGraphicId == 9 && receipt.originalZoneId == 13 &&
          receipt.companionGraphicId == 11 && receipt.sourceAssetCount == 2 &&
          receipt.sourceCommandCount == 4 && receipt.suppressSyntheticFallback);

    surfaces[2].sourceOwned = 0;
    CHECK(!dm1_v1_action_spell_source_asset_runtime_build_pc34(
              &materials, &commands, &frame, &order, &runtime, &receipt));
    surfaces[2].sourceOwned = 1;
    frame.commands[1].sourceY = 12;
    CHECK(!dm1_v1_action_spell_source_asset_runtime_build_pc34(
              &materials, &commands, &frame, &order, &runtime, &receipt));

    set_common(&materials, surfaces, &commands, &frame, &order, &runtime, 1);
    CHECK(dm1_v1_action_spell_source_asset_runtime_build_pc34(
              &materials, &commands, &frame, &order, &runtime, &receipt));
    CHECK(receipt.originalGraphicId == 10 && receipt.originalZoneId == 11 &&
          receipt.companionGraphicId == 0 && receipt.sourceAssetCount == 1);
    runtime.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_source_asset_runtime_build_pc34(
              &materials, &commands, &frame, &order, &runtime, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell source asset runtime");
    return failures ? 1 : 0;
}
