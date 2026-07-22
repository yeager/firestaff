#include "dm1_v1_action_spell_execution_receipt_pc34_compat.h"

#include <string.h>

static unsigned int
dm1_v1_action_spell_command_fingerprint_pc34(
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command =
            &commands->commands[i];
        const int values[] = {
            command->kind, command->graphicId, command->zoneId,
            command->zoneCount, command->sourceX, command->sourceY,
            command->sourceW, command->sourceH, command->sourceSurfaceIndex
        };
        int valueIndex;
        for (valueIndex = 0;
             valueIndex < (int)(sizeof(values) / sizeof(values[0]));
             ++valueIndex) {
            unsigned int value = (unsigned int)values[valueIndex];
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
dm1_v1_action_spell_effect_matches_presentation_pc34(
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    if (!effect || !presentation || !commands ||
        effect->championIndex != presentation->championIndex ||
        effect->actionIndex != presentation->actionIndex ||
        commands->presentationKind != presentation->presentationKind) {
        return 0;
    }
    switch (presentation->presentationKind) {
        case DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34:
            return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_DAMAGE_PC34 &&
                   effect->damage > 0 &&
                   presentation->damage == effect->damage &&
                   commands->commandCount == 2;
        case DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34:
            return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 &&
                   effect->remainingTicks > 0 && effect->actionIndex >= 0 &&
                   presentation->remainingTicks == effect->remainingTicks &&
                   commands->commandCount >= 3 && commands->commandCount <= 5;
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34:
            return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 &&
                   effect->combatOutcome == 1 && effect->damage > 0 &&
                   presentation->spellKind == effect->combatOutcome &&
                   presentation->spellPowerOrdinal == effect->damage &&
                   commands->commandCount == 4;
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34:
            return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 &&
                   effect->combatOutcome == 2 && effect->damage > 0 &&
                   presentation->spellKind == effect->combatOutcome &&
                   presentation->spellPowerOrdinal == effect->damage &&
                   commands->commandCount == 4;
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34:
            return effect->kind == DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 &&
                   (effect->combatOutcome == 3 || effect->combatOutcome == 4) &&
                   effect->damage > 0 &&
                   presentation->spellKind == effect->combatOutcome &&
                   presentation->spellPowerOrdinal == effect->damage &&
                   commands->commandCount == 4;
        default:
            /* F0412 failure feedback has no live-effect owner in this API. */
            return 0;
    }
}

int
dm1_v1_action_spell_execution_receipt_build_pc34(
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    DM1_V1_ActionSpellExecutionReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!effect || !presentation || !commands || !presentation->valid ||
        !presentation->drawable || !presentation->suppressSyntheticFallback ||
        !commands->accepted || !commands->drawable ||
        commands->commandCount <= 0 ||
        commands->commandCount != commands->sourceOwnedCommandCount ||
        !dm1_v1_action_spell_effect_matches_presentation_pc34(
            effect, presentation, commands)) {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->readyForPresentation = 1;
    outReceipt->sourceEffectKind = effect->kind;
    outReceipt->presentationKind = presentation->presentationKind;
    outReceipt->championIndex = effect->championIndex;
    outReceipt->actionIndex = effect->actionIndex;
    outReceipt->spellKind = presentation->spellKind;
    outReceipt->spellPowerOrdinal = presentation->spellPowerOrdinal;
    outReceipt->commandCount = commands->commandCount;
    outReceipt->sourceOwnedCommandCount = commands->sourceOwnedCommandCount;
    outReceipt->sourceTick = effect->sourceTick;
    outReceipt->serial = effect->serial;
    outReceipt->commandFingerprint =
        dm1_v1_action_spell_command_fingerprint_pc34(commands);
    return 1;
}
