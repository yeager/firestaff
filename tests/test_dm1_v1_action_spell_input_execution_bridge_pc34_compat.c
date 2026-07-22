#include "dm1_v1_action_spell_input_execution_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static unsigned int fingerprint(const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *c = &commands->commands[i];
        const int v[] = { c->kind, c->graphicId, c->zoneId, c->zoneCount,
            c->sourceX, c->sourceY, c->sourceW, c->sourceH, c->sourceSurfaceIndex };
        int j;
        for (j = 0; j < 9; ++j) {
            int b;
            for (b = 0; b < 4; ++b) { hash ^= ((unsigned int)v[j] >> (b * 8)) & 0xffu; hash *= 16777619u; }
        }
    }
    return hash;
}

int main(void)
{
    DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 input;
    DM1_V1_LiveActionEffectPc34 effect;
    DM1_V1_ActionSpellHudPresentationReceiptPc34 presentation;
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;
    DM1_V1_ActionSpellExecutionReceiptPc34 execution;
    DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 bridge;

    memset(&input, 0, sizeof(input)); memset(&effect, 0, sizeof(effect));
    memset(&presentation, 0, sizeof(presentation)); memset(&commands, 0, sizeof(commands));
    memset(&execution, 0, sizeof(execution));
    input.accepted = 1; input.kind = DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34;
    input.championIndex = 1; input.commandZoneId = 258; input.commandGraphicId = 9;
    input.secondaryGraphicId = 11; input.runeValue = 0x69; input.sourceTick = 60;
    effect.kind = DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34; effect.championIndex = 1;
    effect.damage = 3; effect.combatOutcome = 2; effect.sourceTick = 60; effect.serial = 7;
    presentation.valid = 1; presentation.drawable = 1; presentation.suppressSyntheticFallback = 1;
    presentation.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    commands.accepted = 1; commands.drawable = 1; commands.presentationKind = presentation.presentationKind;
    commands.commandCount = 4; commands.sourceOwnedCommandCount = 4;
    commands.commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 9, 13, 1, 0, 0, 87, 25, 0 };
    commands.commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 245, 6, 0, 13, 14, 12, 1 };
    commands.commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 261, 4, 0, 26, 14, 12, 1 };
    commands.commands[3] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 221, 1, 0, 0, 0, 0, 2 };
    execution.accepted = 1; execution.readyForPresentation = 1;
    execution.sourceEffectKind = effect.kind; execution.presentationKind = presentation.presentationKind;
    execution.championIndex = 1; execution.spellKind = 2; execution.spellPowerOrdinal = 3;
    execution.commandCount = 4; execution.sourceOwnedCommandCount = 4;
    execution.sourceTick = 60; execution.serial = 7; execution.commandFingerprint = fingerprint(&commands);

    CHECK(dm1_v1_action_spell_input_execution_bridge_build_pc34(
              &input, &effect, &presentation, &commands, &execution, &bridge));
    CHECK(bridge.accepted && bridge.inputKind ==
              DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34 &&
          bridge.runeValue == 0x69 && bridge.bridgeFingerprint != 0);
    input.sourceTick = 59;
    CHECK(!dm1_v1_action_spell_input_execution_bridge_build_pc34(
              &input, &effect, &presentation, &commands, &execution, &bridge));
    input.sourceTick = 60;
    commands.commands[1].zoneId = 246;
    CHECK(!dm1_v1_action_spell_input_execution_bridge_build_pc34(
              &input, &effect, &presentation, &commands, &execution, &bridge));

    memset(&input, 0, sizeof(input)); memset(&effect, 0, sizeof(effect));
    memset(&presentation, 0, sizeof(presentation)); memset(&commands, 0, sizeof(commands));
    memset(&execution, 0, sizeof(execution));
    input.accepted = 1; input.kind = DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34;
    input.championIndex = 3; input.actionIndex = 12; input.commandZoneId = 86;
    input.commandGraphicId = 10; input.fontGraphicId = 695;
    input.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    input.sourceTick = 61;
    effect.kind = DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34; effect.championIndex = 3;
    effect.actionIndex = 12; effect.remainingTicks = 2; effect.sourceTick = 61; effect.serial = 8;
    presentation.valid = 1; presentation.drawable = 1; presentation.suppressSyntheticFallback = 1;
    presentation.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    commands.accepted = 1; commands.drawable = 1; commands.presentationKind = presentation.presentationKind;
    commands.commandCount = 3; commands.sourceOwnedCommandCount = 3;
    commands.commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 10, 11, 1, 0, 0, 87, 45, 0 };
    commands.commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 80, 1, 0, 0, 0, 0, 1 };
    commands.commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 86, 1, 0, 0, 0, 0, 1 };
    execution.accepted = 1; execution.readyForPresentation = 1;
    execution.sourceEffectKind = effect.kind; execution.presentationKind = presentation.presentationKind;
    execution.championIndex = 3; execution.actionIndex = 12;
    execution.commandCount = 3; execution.sourceOwnedCommandCount = 3;
    execution.sourceTick = 61; execution.serial = 8; execution.commandFingerprint = fingerprint(&commands);
    CHECK(dm1_v1_action_spell_input_execution_bridge_build_pc34(
              &input, &effect, &presentation, &commands, &execution, &bridge));
    CHECK(bridge.accepted && bridge.inputKind ==
              DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34 &&
          bridge.inputZoneId == 86 && bridge.actionIndex == 12);
    input.actionIndex = 13;
    CHECK(!dm1_v1_action_spell_input_execution_bridge_build_pc34(
              &input, &effect, &presentation, &commands, &execution, &bridge));

    printf("%s\n", failures ? "failed" : "ok: action/spell input execution bridge");
    return failures ? 1 : 0;
}
