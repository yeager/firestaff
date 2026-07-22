#include "dm1_v1_action_spell_result_feedback_pc34_compat.h"

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
    DM1_V1_ActionSpellSourceResultPc34 result;
    DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 bridge;
    DM1_V1_ActionSpellExecutionReceiptPc34 execution;
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;
    DM1_V1_ActionSpellResultFeedbackReceiptPc34 feedback;
    unsigned int commandFingerprint;

    memset(&result, 0, sizeof(result)); memset(&bridge, 0, sizeof(bridge));
    memset(&execution, 0, sizeof(execution)); memset(&commands, 0, sizeof(commands));
    commands.accepted = 1; commands.drawable = 1;
    commands.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    commands.commandCount = 4; commands.sourceOwnedCommandCount = 4;
    commands.commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 9, 13, 1, 0, 0, 87, 25, 0 };
    commands.commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 245, 6, 0, 13, 14, 12, 1 };
    commands.commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 11, 261, 4, 0, 26, 14, 12, 1 };
    commands.commands[3] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 221, 1, 0, 0, 0, 0, 2 };
    commandFingerprint = fingerprint(&commands);
    bridge.accepted = 1; bridge.inputKind = DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34;
    bridge.presentationKind = commands.presentationKind; bridge.championIndex = 1;
    bridge.inputZoneId = 258; bridge.sourceTick = 70; bridge.serial = 9;
    bridge.commandFingerprint = commandFingerprint;
    execution.accepted = 1; execution.readyForPresentation = 1;
    execution.presentationKind = commands.presentationKind; execution.championIndex = 1;
    execution.commandCount = 4; execution.sourceOwnedCommandCount = 4;
    execution.sourceTick = 70; execution.serial = 9; execution.commandFingerprint = commandFingerprint;
    result.sourceOwned = 1; result.resultKind = DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34;
    result.inputKind = bridge.inputKind; result.sourceTick = 70; result.serial = 9;
    result.commandFingerprint = commandFingerprint;
    CHECK(dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));
    CHECK(feedback.accepted && feedback.resultKind ==
              DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34 &&
          feedback.requiresCommandRepaint && feedback.suppressSyntheticFallback);
    result.sourceOwned = 0;
    CHECK(!dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));
    result.sourceOwned = 1;
    result.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));
    result.commandFingerprint = commandFingerprint;
    commands.commands[1].zoneId = 246;
    CHECK(!dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));

    memset(&result, 0, sizeof(result)); memset(&bridge, 0, sizeof(bridge));
    memset(&execution, 0, sizeof(execution)); memset(&commands, 0, sizeof(commands));
    commands.accepted = 1; commands.drawable = 1;
    commands.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    commands.commandCount = 3; commands.sourceOwnedCommandCount = 3;
    commands.commands[0] = (DM1_V1_ActionSpellRenderCommandPc34){ 1, 10, 11, 1, 0, 0, 87, 45, 0 };
    commands.commands[1] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 80, 1, 0, 0, 0, 0, 1 };
    commands.commands[2] = (DM1_V1_ActionSpellRenderCommandPc34){ 2, 695, 86, 1, 0, 0, 0, 0, 1 };
    commandFingerprint = fingerprint(&commands);
    bridge.accepted = 1; bridge.inputKind = DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34;
    bridge.presentationKind = commands.presentationKind; bridge.championIndex = 2;
    bridge.inputZoneId = 86; bridge.sourceTick = 71; bridge.serial = 10;
    bridge.commandFingerprint = commandFingerprint;
    execution.accepted = 1; execution.readyForPresentation = 1;
    execution.presentationKind = commands.presentationKind; execution.championIndex = 2;
    execution.commandCount = 3; execution.sourceOwnedCommandCount = 3;
    execution.sourceTick = 71; execution.serial = 10; execution.commandFingerprint = commandFingerprint;
    result.sourceOwned = 1; result.resultKind = DM1_V1_ACTION_SPELL_RESULT_ACTION_SUCCESS_PC34;
    result.inputKind = bridge.inputKind; result.sourceTick = 71; result.serial = 10;
    result.commandFingerprint = commandFingerprint;
    CHECK(dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));
    CHECK(feedback.accepted && feedback.resultKind ==
              DM1_V1_ACTION_SPELL_RESULT_ACTION_SUCCESS_PC34 &&
          feedback.presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34);
    result.resultKind = DM1_V1_ACTION_SPELL_RESULT_SPELL_SUCCESS_PC34;
    CHECK(!dm1_v1_action_spell_result_feedback_build_pc34(
              &result, &bridge, &execution, &commands, &feedback));

    printf("%s\n", failures ? "failed" : "ok: action/spell result feedback");
    return failures ? 1 : 0;
}
