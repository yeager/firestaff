#include "dm1_v1_action_spell_input_execution_bridge_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static unsigned int
dm1_v1_action_spell_input_execution_command_fingerprint_pc34(
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

static unsigned int
dm1_v1_action_spell_input_execution_bridge_fingerprint_pc34(
    const DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *input,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution)
{
    unsigned int hash = 2166136261u;
    const unsigned int values[] = {
        (unsigned int)input->kind, (unsigned int)input->championIndex,
        (unsigned int)input->actionIndex, (unsigned int)input->commandZoneId,
        (unsigned int)input->runeValue, input->sourceTick,
        (unsigned int)execution->sourceEffectKind,
        (unsigned int)execution->presentationKind, execution->sourceTick,
        execution->serial, execution->commandFingerprint
    };
    int i;
    for (i = 0; i < (int)(sizeof(values) / sizeof(values[0])); ++i) {
        int byteIndex;
        for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
            hash ^= (values[i] >> (byteIndex * 8)) & 0xffu;
            hash *= 16777619u;
        }
    }
    return hash;
}

static int
dm1_v1_action_spell_input_execution_action_valid_pc34(
    const DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *input,
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution)
{
    int i;
    int selectedZonePresent = 0;
    if (input->presentationKind !=
            DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 ||
        effect->kind != DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 ||
        execution->presentationKind !=
            DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 ||
        input->commandGraphicId != DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 ||
        input->fontGraphicId != 695 || commands->commandCount < 3) {
        return 0;
    }
    for (i = 0; i < commands->commandCount; ++i) {
        if (commands->commands[i].zoneId == input->commandZoneId) {
            selectedZonePresent = 1;
        }
    }
    return selectedZonePresent;
}

static int
dm1_v1_action_spell_input_execution_spell_valid_pc34(
    const DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *input,
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution)
{
    int isSpellPresentation =
        execution->presentationKind ==
            DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 ||
        execution->presentationKind ==
            DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 ||
        execution->presentationKind ==
            DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34;
    return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 &&
           isSpellPresentation && input->commandGraphicId ==
               DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
           input->secondaryGraphicId ==
               DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
           input->commandZoneId >=
               DM1_V1_SPELL_AVAILABLE_SYMBOL_ZONE_ID_BASE_PC34 &&
           input->commandZoneId <
               DM1_V1_SPELL_AVAILABLE_SYMBOL_ZONE_ID_BASE_PC34 +
               DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34 &&
           input->runeValue >= DM1_V1_SPELL_RUNE_VALUE_BASE_PC34 &&
           input->runeValue < DM1_V1_SPELL_RUNE_VALUE_BASE_PC34 + 24 &&
           commands->commandCount == 4;
}

int
dm1_v1_action_spell_input_execution_bridge_build_pc34(
    const DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *input,
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 *outReceipt)
{
    unsigned int fingerprint;
    int routeValid;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!input || !effect || !presentation || !commands || !execution ||
        !input->accepted || !presentation->valid || !presentation->drawable ||
        !presentation->suppressSyntheticFallback || !commands->accepted ||
        !commands->drawable || !execution->accepted ||
        !execution->readyForPresentation || input->sourceTick == 0 ||
        input->sourceTick != effect->sourceTick ||
        input->sourceTick != execution->sourceTick ||
        input->championIndex != effect->championIndex ||
        input->championIndex != execution->championIndex ||
        presentation->presentationKind != execution->presentationKind ||
        commands->presentationKind != execution->presentationKind ||
        commands->commandCount != execution->commandCount ||
        commands->sourceOwnedCommandCount != execution->sourceOwnedCommandCount ||
        commands->commandCount <= 0) {
        return 0;
    }
    fingerprint = dm1_v1_action_spell_input_execution_command_fingerprint_pc34(
        commands);
    if (fingerprint != execution->commandFingerprint) return 0;
    if (input->kind == DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34) {
        routeValid = input->actionIndex == effect->actionIndex &&
                     input->actionIndex == execution->actionIndex &&
                     dm1_v1_action_spell_input_execution_action_valid_pc34(
                         input, effect, commands, execution);
    } else if (input->kind == DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34) {
        routeValid = dm1_v1_action_spell_input_execution_spell_valid_pc34(
            input, effect, commands, execution);
    } else {
        routeValid = 0;
    }
    if (!routeValid) return 0;

    outReceipt->accepted = 1;
    outReceipt->inputKind = input->kind;
    outReceipt->presentationKind = execution->presentationKind;
    outReceipt->championIndex = execution->championIndex;
    outReceipt->actionIndex = execution->actionIndex;
    outReceipt->inputZoneId = input->commandZoneId;
    outReceipt->runeValue = input->runeValue;
    outReceipt->commandCount = execution->commandCount;
    outReceipt->sourceTick = execution->sourceTick;
    outReceipt->serial = execution->serial;
    outReceipt->commandFingerprint = execution->commandFingerprint;
    outReceipt->bridgeFingerprint =
        dm1_v1_action_spell_input_execution_bridge_fingerprint_pc34(
            input, execution);
    return 1;
}
