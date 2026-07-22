#include "dm1_v1_action_spell_result_feedback_pc34_compat.h"

#include <string.h>

static unsigned int
dm1_v1_action_spell_feedback_command_fingerprint_pc34(
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command =
            &commands->commands[i];
        const int values[] = { command->kind, command->graphicId,
            command->zoneId, command->zoneCount, command->sourceX,
            command->sourceY, command->sourceW, command->sourceH,
            command->sourceSurfaceIndex };
        int j;
        for (j = 0; j < (int)(sizeof(values) / sizeof(values[0])); ++j) {
            unsigned int value = (unsigned int)values[j];
            int byteIndex;
            for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
                hash ^= (value >> (byteIndex * 8)) & 0xffu;
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

static int
dm1_v1_action_spell_feedback_result_matches_route_pc34(
    const DM1_V1_ActionSpellSourceResultPc34 *result,
    const DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 *bridge)
{
    if (result->inputKind != bridge->inputKind) return 0;
    if (result->inputKind == DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34) {
        return result->resultKind == DM1_V1_ACTION_SPELL_RESULT_ACTION_SUCCESS_PC34 ||
               result->resultKind == DM1_V1_ACTION_SPELL_RESULT_ACTION_FAILURE_PC34;
    }
    if (result->inputKind == DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34) {
        return result->resultKind == DM1_V1_ACTION_SPELL_RESULT_SPELL_SUCCESS_PC34 ||
               result->resultKind == DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34;
    }
    return 0;
}

int
dm1_v1_action_spell_result_feedback_build_pc34(
    const DM1_V1_ActionSpellSourceResultPc34 *result,
    const DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 *bridge,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    DM1_V1_ActionSpellResultFeedbackReceiptPc34 *outReceipt)
{
    unsigned int fingerprint;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!result || !bridge || !execution || !commands || !result->sourceOwned ||
        !bridge->accepted || !execution->accepted ||
        !execution->readyForPresentation || !commands->accepted ||
        !commands->drawable || !dm1_v1_action_spell_feedback_result_matches_route_pc34(
            result, bridge) || result->sourceTick == 0 ||
        result->sourceTick != bridge->sourceTick ||
        result->sourceTick != execution->sourceTick ||
        result->serial != bridge->serial || result->serial != execution->serial ||
        result->commandFingerprint != bridge->commandFingerprint ||
        result->commandFingerprint != execution->commandFingerprint ||
        commands->commandCount != execution->commandCount ||
        commands->sourceOwnedCommandCount != execution->sourceOwnedCommandCount) {
        return 0;
    }
    fingerprint = dm1_v1_action_spell_feedback_command_fingerprint_pc34(commands);
    if (fingerprint != result->commandFingerprint) return 0;

    outReceipt->accepted = 1;
    outReceipt->resultKind = result->resultKind;
    outReceipt->presentationKind = execution->presentationKind;
    outReceipt->championIndex = execution->championIndex;
    outReceipt->inputZoneId = bridge->inputZoneId;
    outReceipt->requiresCommandRepaint = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->sourceTick = execution->sourceTick;
    outReceipt->serial = execution->serial;
    outReceipt->commandFingerprint = fingerprint;
    return 1;
}
